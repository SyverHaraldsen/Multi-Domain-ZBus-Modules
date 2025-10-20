/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

#include "shared_zbus.h"
#include "shared_zbus_definition.h"
#include "module_common.h"
#include "led.h"

/* Use GPIO LEDs available on nRF54L15DK */
#define LED1 DT_ALIAS(led1)
#define LED2 DT_ALIAS(led2)
#define LED3 DT_ALIAS(led3)

#if !DT_NODE_HAS_STATUS(LED1, okay) || !DT_NODE_HAS_STATUS(LED2, okay) || \
        !DT_NODE_HAS_STATUS(LED3, okay)
#error "Unsupported board: led1, led2, led3 devicetree alias is not defined"
#endif

static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3, gpios);

/* Register log module */
LOG_MODULE_REGISTER(led, CONFIG_MDM_LED_LOG_LEVEL);

static void led_callback(const struct zbus_channel *chan);

/* Register listener - led_callback will be called everytime a channel that the module listens on
 * receives a new message.
 */
ZBUS_LISTENER_DEFINE(led, led_callback);

/* Observe channels */
ZBUS_CHAN_ADD_OBS(LED_CHAN, led, 0);

static struct k_work_delayable blink_work;

/* Structure to hold all LED state variables */
struct led_state {
	struct led_msg current_state;
	bool is_on;
	int repetitions;
};

static struct led_state led_state;
static void blink_timer_handler(struct k_work *work);

static int gpio_led_out(const struct led_msg *led_msg, bool force_off)
{
	int err;

	/* If force_off is true, turn off all LEDs regardless of led_msg values */
	/* Map RGB values to the available LEDs on nRF54L15DK */
	bool led1_on = !force_off && (led_msg->red > 0);     /* Red -> LED1 */
	bool led2_on = !force_off && (led_msg->green > 0);   /* Green -> LED2 */
	bool led3_on = !force_off && (led_msg->blue > 0);    /* Blue -> LED3 */

	err = gpio_pin_set_dt(&led1, led1_on);
	if (err) {
		LOG_ERR("gpio_pin_set_dt LED1, error: %d", err);
		return err;
	}

	err = gpio_pin_set_dt(&led2, led2_on);
	if (err) {
		LOG_ERR("gpio_pin_set_dt LED2, error: %d", err);
		return err;
	}

	err = gpio_pin_set_dt(&led3, led3_on);
	if (err) {
		LOG_ERR("gpio_pin_set_dt LED3, error: %d", err);
		return err;
	}

	return 0;
}

/* Timer work handler for LED blinking */
static void blink_timer_handler(struct k_work *work)
{
	int err;

	ARG_UNUSED(work);

	led_state.is_on = !led_state.is_on;

	/* Update LED state */
	err = gpio_led_out(&led_state.current_state, !led_state.is_on);
	if (err) {
		LOG_ERR("gpio_led_out, error: %d", err);
		SEND_FATAL_ERROR();
	}

	/* If LED just turned off, we completed one cycle */
	if (!led_state.is_on && led_state.repetitions > 0) {
		led_state.repetitions--;
		if (led_state.repetitions == 0) {
			/* We're done, don't schedule next toggle */
			return;
		}
	}

	/* Schedule next toggle */
	uint32_t next_delay = led_state.is_on ? led_state.current_state.duration_on_msec
					      : led_state.current_state.duration_off_msec;

	err = k_work_schedule(&blink_work, K_MSEC(next_delay));
	if (err < 0) {
		LOG_ERR("k_work_schedule, error: %d", err);
		SEND_FATAL_ERROR();
	}
}

/* Function called when there is a message received on a channel that the module listens to */
static void led_callback(const struct zbus_channel *chan)
{
	if (&LED_CHAN == chan) {
		int err;
		const struct led_msg *led_msg = zbus_chan_const_msg(chan);

		/* Print received LED message */
		LOG_DBG("LED message received: type=%d, R=%d, G=%d, B=%d, on=%dms, off=%dms, "
			"reps=%d",
			led_msg->type, led_msg->red, led_msg->green, led_msg->blue,
			led_msg->duration_on_msec, led_msg->duration_off_msec,
			led_msg->repetitions);

		/* Cancel any existing blink timer */
		(void)k_work_cancel_delayable(&blink_work);

		/* Store the new LED state */
		memcpy(&led_state.current_state, led_msg, sizeof(struct led_msg));

		/* Set up repetitions */
		led_state.repetitions = led_msg->repetitions;

		/* If repetitions is 0, turn LED off. Otherwise LED on */
		led_state.is_on = (led_state.repetitions != 0);

		err = gpio_led_out(led_msg, !led_state.is_on);
		if (err) {
			LOG_ERR("gpio_led_out, error: %d", err);
			SEND_FATAL_ERROR();
		}

		/* Schedule first toggle if LED should be blinking */
		if (led_state.is_on) {
			err = k_work_schedule(&blink_work, K_MSEC(led_msg->duration_on_msec));
			if (err < 0) {
				LOG_ERR("k_work_schedule, error: %d", err);
				SEND_FATAL_ERROR();
			}
		}
	}
}

static int led_init(void)
{
	int err;

	k_work_init_delayable(&blink_work, blink_timer_handler);

	/* Configure GPIO LEDs as outputs */
	if (!gpio_is_ready_dt(&led1) || !gpio_is_ready_dt(&led2) || !gpio_is_ready_dt(&led3)) {
		LOG_ERR("GPIO LED device not ready");
		return -ENODEV;
	}

	err = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
	if (err) {
		LOG_ERR("Cannot configure LED1 GPIO, error: %d", err);
		return err;
	}

	err = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
	if (err) {
		LOG_ERR("Cannot configure LED2 GPIO, error: %d", err);
		return err;
	}

	err = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);
	if (err) {
		LOG_ERR("Cannot configure LED3 GPIO, error: %d", err);
		return err;
	}

	return 0;
}

/* Initialize module at SYS_INIT() */
SYS_INIT(led_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
