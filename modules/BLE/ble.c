/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "ble.h"
#include "shared_zbus_definition.h"

#include <zephyr/kernel.h>

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>

#include <bluetooth/services/nus.h>

#include <zephyr/zbus/zbus.h>
#include <zephyr/zbus/multidomain/zbus_multidomain.h>

#include <zephyr/settings/settings.h>

#ifdef CONFIG_BLE_MODULE_DK_SUPPORT
#include <dk_buttons_and_leds.h>
#endif

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ble_module, CONFIG_BLE_MODULE_LOG_LEVEL);

#define BLE_TX_BUFFER_SIZE     64
#define BLE_TX_TIMEOUT_MS      1000
#define BLE_ATT_PRIME_DELAY_MS 200
#define BLE_MAX_PRINT_LEN      256

/* Internal callback types */
typedef void (*ble_data_received_cb_t)(struct bt_conn *conn, const uint8_t *data, uint16_t len);
typedef void (*ble_connection_status_cb_t)(struct bt_conn *conn, bool connected);
typedef void (*ble_ready_cb_t)(struct bt_conn *conn, bool ready);

/* Internal configuration structure */
struct ble_module_config {
	ble_data_received_cb_t data_received_cb;
	ble_connection_status_cb_t connection_status_cb;
	ble_ready_cb_t ready_cb;
};

/* Module states */
static struct bt_conn *current_conn;
static struct bt_conn *auth_conn;
static struct k_work adv_work;
static K_SEM_DEFINE(nus_tx_sem, 1, 1);
static bool module_enabled;
static bool nus_notifications_enabled;
static ble_data_received_cb_t user_data_cb;
static ble_connection_status_cb_t user_connection_status_cb;
static ble_ready_cb_t user_ready_cb;

/* Advertising data */
#define DEVICE_NAME     CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_NUS_VAL),
};

static int publish_ble_data(const uint8_t *data, uint16_t len)
{
	int ret;

	struct ble_module_message msg = {0};

	msg.type = BLE_RECV;
	msg.len = len;
	memcpy(msg.data, data, len);
	msg.timestamp = k_uptime_get_32();

	ret = zbus_chan_pub(&BLE_CHAN, &msg, K_FOREVER);
	if (ret != 0) {
		LOG_ERR("Failed to publish BLE data: %d", ret);
	}
	return ret;
}

/* BLE callbacks */
static void connected(struct bt_conn *conn, uint8_t err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	if (err) {
		LOG_ERR("Connection failed, err 0x%02x %s", err, bt_hci_err_to_str(err));
		return;
	}

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_DBG("Connected %s", addr);

	current_conn = bt_conn_ref(conn);

	if (user_connection_status_cb) {
		user_connection_status_cb(conn, true);
	}

#ifdef CONFIG_BLE_MODULE_DK_SUPPORT
	dk_set_led_on(DK_LED1);
#endif
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_DBG("Disconnected: %s, reason 0x%02x %s", addr, reason, bt_hci_err_to_str(reason));

	if (auth_conn) {
		bt_conn_unref(auth_conn);
		auth_conn = NULL;
	}

	if (current_conn) {
		if (user_connection_status_cb) {
			user_connection_status_cb(conn, false);
		}

		if (user_ready_cb) {
			user_ready_cb(conn, false);
		}

		bt_conn_unref(current_conn);
		current_conn = NULL;
		nus_notifications_enabled = false;
#ifdef CONFIG_BLE_MODULE_DK_SUPPORT
		dk_set_led_off(DK_LED1);
#endif
	}
}

static void recycled_cb(void)
{
	LOG_DBG("Connection object available. Disconnect complete!");
	if (module_enabled) {
		k_work_submit(&adv_work);
	}
}

#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (!err) {
		LOG_DBG("Security changed: %s level %u", addr, level);
	} else {
		LOG_WRN("Security failed: %s level %u err %d %s", addr, level, err,
			bt_security_err_to_str(err));
	}
}
#endif

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
	.recycled = recycled_cb,
#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
	.security_changed = security_changed,
#endif
};

#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_DBG("Passkey for %s: %06u", addr, passkey);
}

static void auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];

	if (auth_conn) {
		bt_conn_unref(auth_conn);
	}
	auth_conn = bt_conn_ref(conn);

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_DBG("Passkey for %s: %06u", addr, passkey);
	LOG_DBG("Press Button to confirm, Button 2 to reject.");
}

static void auth_cancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_DBG("Pairing cancelled: %s", addr);
}

static void pairing_complete(struct bt_conn *conn, bool bonded)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_DBG("Pairing completed: %s, bonded: %d", addr, bonded);
}

static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_DBG("Pairing failed conn: %s, reason %d %s", addr, reason,
		bt_security_err_to_str(reason));
}

static struct bt_conn_auth_cb conn_auth_callbacks = {
	.passkey_display = auth_passkey_display,
	.passkey_confirm = auth_passkey_confirm,
	.cancel = auth_cancel,
};

static struct bt_conn_auth_info_cb conn_auth_info_callbacks = {.pairing_complete = pairing_complete,
							       .pairing_failed = pairing_failed};
#endif

static void bt_receive_cb(struct bt_conn *conn, const uint8_t *const data, uint16_t len)
{
	publish_ble_data(data, len);

	if (user_data_cb) {
		user_data_cb(conn, data, len);
	}
}

static struct k_work_delayable ready_work;

static void ready_work_handler(struct k_work *work)
{
	const char *msg = "\r\n";
	int ret = bt_nus_send(NULL, (const uint8_t *)msg, strlen(msg));

	if (ret == 0) {
		LOG_DBG("ATT channel primed and ready");

		if (user_ready_cb && current_conn) {
			user_ready_cb(current_conn, true);
		}
	}
}

static void nus_send_enabled_cb(enum bt_nus_send_status status)
{
	nus_notifications_enabled = (status == BT_NUS_SEND_STATUS_ENABLED);
	LOG_DBG("NUS notifications %s", nus_notifications_enabled ? "enabled" : "disabled");

	if (nus_notifications_enabled) {
		k_work_schedule(&ready_work, K_MSEC(BLE_ATT_PRIME_DELAY_MS));
	}
}

static void nus_sent_cb(struct bt_conn *conn)
{
	LOG_DBG("Data sent successfully - releasing semaphore");
	k_sem_give(&nus_tx_sem);
}

static struct bt_nus_cb nus_cb = {
	.received = bt_receive_cb,
	.send_enabled = nus_send_enabled_cb,
	.sent = nus_sent_cb,
};

static void adv_work_handler(struct k_work *work)
{
	int err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_2, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));

	if (err) {
		LOG_ERR("Advertising failed to start (err %d)", err);
		return;
	}

	LOG_DBG("Advertising successfully started");
}

static int ble_module_init(const struct ble_module_config *config)
{
	int err;

	if (config) {
		user_data_cb = config->data_received_cb;
		user_connection_status_cb = config->connection_status_cb;
		user_ready_cb = config->ready_cb;
	}

#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
	err = bt_conn_auth_cb_register(&conn_auth_callbacks);
	if (err) {
		LOG_ERR("Failed to register authorization callbacks. (err: %d)", err);
		return err;
	}

	err = bt_conn_auth_info_cb_register(&conn_auth_info_callbacks);
	if (err) {
		LOG_ERR("Failed to register authorization info callbacks. (err: %d)", err);
		return err;
	}
#endif

	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return err;
	}

	LOG_INF("Bluetooth initialized");

#ifdef CONFIG_SETTINGS
	err = settings_load();
	if (err) {
		LOG_ERR("Failed to load settings (err: %d)", err);
		return err;
	}
#endif

	err = bt_nus_init(&nus_cb);
	if (err) {
		LOG_ERR("Failed to initialize nus service (err: %d)", err);
		return err;
	}

	k_work_init(&adv_work, adv_work_handler);
	k_work_init_delayable(&ready_work, ready_work_handler);

	LOG_INF("BLE module initialized");
	return 0;
}

static int ble_module_enable(void)
{
	if (module_enabled) {
		LOG_WRN("BLE module already enabled");
		return -EALREADY;
	}

	module_enabled = true;
	k_work_submit(&adv_work);

	LOG_DBG("BLE module enabled");
	return 0;
}

int ble_module_send(const uint8_t *data, uint16_t len)
{
	if (!data || len == 0) {
		return -EINVAL;
	}

	if (!current_conn) {
		return -ENOTCONN;
	}

	if (!nus_notifications_enabled) {
		return -EACCES;
	}

	if (k_sem_take(&nus_tx_sem, K_MSEC(BLE_TX_TIMEOUT_MS)) != 0) {
		LOG_WRN("TX semaphore timeout - previous send may have failed");
		return -ETIMEDOUT;
	}

	int ret = bt_nus_send(NULL, data, len);

	if (ret != 0) {
		k_sem_give(&nus_tx_sem);
	}

	return ret;
}

bool ble_module_is_connected(void)
{
	return current_conn != NULL;
}

struct bt_conn *ble_module_get_connection(void)
{
	return current_conn;
}

bool ble_module_is_ready(void)
{
	return current_conn != NULL && nus_notifications_enabled;
}

static struct k_work send_work;
static uint8_t tx_buffer[BLE_TX_BUFFER_SIZE];
static size_t tx_len;
static K_MUTEX_DEFINE(tx_buf_mutex);

static void send_work_handler(struct k_work *work)
{
	uint8_t local_buf[BLE_TX_BUFFER_SIZE];
	size_t local_len;

	k_mutex_lock(&tx_buf_mutex, K_FOREVER);
	if (tx_len == 0) {
		k_mutex_unlock(&tx_buf_mutex);
		return;
	}
	local_len = tx_len;
	memcpy(local_buf, tx_buffer, tx_len);
	tx_len = 0;
	k_mutex_unlock(&tx_buf_mutex);

	int err = ble_module_send(local_buf, local_len);

	if (err == 0) {
		LOG_DBG("Data sent successfully (%zu bytes)", local_len);
	} else {
		LOG_ERR("Failed to send data: %d", err);
	}
}

static void queue_ble_send(const uint8_t *data, size_t len)
{
	if (len > sizeof(tx_buffer)) {
		LOG_ERR("Data too large (%zu bytes, max %zu)", len, sizeof(tx_buffer));
		return;
	}

	k_mutex_lock(&tx_buf_mutex, K_FOREVER);
	if (tx_len) {
		LOG_WRN("Previous message not sent yet, overwriting");
	}
	memcpy(tx_buffer, data, len);
	tx_len = len;
	k_mutex_unlock(&tx_buf_mutex);

	k_work_submit(&send_work);
}

static void default_data_received_cb(struct bt_conn *conn, const uint8_t *data, uint16_t len)
{
	if (!data || len == 0) {
		return;
	}

	LOG_INF("RX: %d bytes", len);
	LOG_HEXDUMP_DBG(data, len, "RX Data");

	bool printable = true;

	for (uint16_t i = 0; i < len; i++) {
		if (data[i] < 0x20 && data[i] != '\r' && data[i] != '\n' && data[i] != '\t') {
			printable = false;
			break;
		}
	}

	if (printable) {
		if (len < BLE_MAX_PRINT_LEN) {
			char str_buf[BLE_MAX_PRINT_LEN];

			memcpy(str_buf, data, len);
			str_buf[len] = '\0';
			LOG_INF("  As String: \"%s\"", str_buf);
		} else {
			LOG_WRN("Data too large to print as string (%d bytes, max %d)", len,
				BLE_MAX_PRINT_LEN - 1);
		}
	}
}

static void default_connection_status_cb(struct bt_conn *conn, bool connected)
{
	if (!conn) {
		LOG_ERR("Connection callback called with NULL conn");
		return;
	}

	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (connected) {
		LOG_INF("BLE Connected to %s", addr);
	} else {
		LOG_INF("BLE Disconnected from %s", addr);
	}
}

static void default_ready_cb(struct bt_conn *conn, bool ready)
{
	if (!conn) {
		return;
	}

	if (ready) {
		LOG_DBG("Notifications enabled - can send data to phone");

		const char *msg = "Device ready\r\n";

		queue_ble_send((uint8_t *)msg, strlen(msg));
	}
}

static int ble_module_auto_init(void)
{
	int err;

	LOG_INF("=================================");
	LOG_INF("  BLE NUS Application");
	LOG_INF("=================================");

	k_work_init(&send_work, send_work_handler);

	struct ble_module_config ble_config = {
		.data_received_cb = default_data_received_cb,
		.connection_status_cb = default_connection_status_cb,
		.ready_cb = default_ready_cb,
	};

	err = ble_module_init(&ble_config);
	if (err) {
		LOG_ERR("BLE init failed: %d", err);
		return err;
	}

	err = ble_module_enable();
	if (err) {
		LOG_ERR("BLE enable failed: %d", err);
		return err;
	}

	LOG_INF("Advertising as: %s", CONFIG_BT_DEVICE_NAME);

	return 0;
}

SYS_INIT(ble_module_auto_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

#if IS_ENABLED(CONFIG_MDM_BLE_ZBUS_LOGGING)

static const char *ble_message_type_to_string(enum ble_msg_type type)
{
	switch (type) {
	case BLE_RECV:
		return "BLE_RECV";
	default:
		return "UNKNOWN";
	}
}

/* Add a message subscriber to the BLE_CHAN channel to log received messages */
ZBUS_MSG_SUBSCRIBER_DEFINE(test_msg_subscriber);
ZBUS_CHAN_ADD_OBS(BLE_CHAN, test_msg_subscriber, 0);

static void test_subscriber_thread(void *unused1, void *unused2, void *unused3)
{
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);

	const struct zbus_channel *chan;
	struct ble_module_message msg;

	while (true) {
		if (zbus_sub_wait_msg(&test_msg_subscriber, &chan, &msg, K_FOREVER) == 0) {
			if (chan == &BLE_CHAN) {
				LOG_INF("=== ZBUS Message Received ===");
				LOG_INF("Type: %s", ble_message_type_to_string(msg.type));
				LOG_INF("Timestamp: %u ms", msg.timestamp);
				LOG_INF("Length: %u bytes", msg.len);

				/* Try to print as string if printable */
				bool printable = true;

				for (uint16_t i = 0; i < msg.len; i++) {
					if (msg.data[i] < 0x20 && msg.data[i] != '\r' &&
					    msg.data[i] != '\n' && msg.data[i] != '\t') {
						printable = false;
						break;
					}
				}

				if (printable && msg.len < BLE_MAX_PRINT_LEN) {
					char str_buf[BLE_MAX_PRINT_LEN];

					memcpy(str_buf, msg.data, msg.len);
					str_buf[msg.len] = '\0';
					LOG_INF("As String: \"%s\"", str_buf);
				}
				LOG_INF("=============================");
			}
		}
	}
}

K_THREAD_DEFINE(test_subscriber_tid, CONFIG_MDM_BLE_ZBUS_LOGGING_STACK_SIZE, test_subscriber_thread,
		NULL, NULL, NULL, CONFIG_MDM_BLE_ZBUS_LOGGING_PRIORITY, 0, 0);

#endif /* CONFIG_MDM_BLE_ZBUS_LOGGING */
