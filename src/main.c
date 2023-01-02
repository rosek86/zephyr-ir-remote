/*
 * Copyright (c) 2016 Intel Corporation
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file Sample app to demonstrate PWM.
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "ir_thread.h"

void main(void)
{
	printk("Start main thread\n");

	while (1) {
		ir_thread_push(BENQ_KEY_POWER_ON);
		ir_thread_push(BENQ_KEY_POWER_ON);
		ir_thread_push(BENQ_KEY_POWER_ON);

		k_sleep(K_SECONDS(2U));
	}
}
