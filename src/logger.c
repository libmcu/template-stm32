/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "logger.h"
#include "libmcu/board.h"

#define STDOUT_RTT
#if defined(STDOUT_RTT)
#include "SEGGER_RTT.h"
#else
#include "libmcu/uart.h"

static struct uart *uart2;
#endif

static size_t stdout_writer(const void *data, size_t size)
{
	unused(size);

	static char buf[LOGGING_MESSAGE_MAXLEN];
	size_t len = logging_stringify(buf, sizeof(buf)-2, data);

	buf[len++] = '\n';
	buf[len] = '\0';

#if defined(STDOUT_RTT)
	return (size_t)SEGGER_RTT_Write(0, buf, len);
#else
	return (size_t)uart_write(uart2, buf, len);
#endif
}

static void initialize_backend_stdout(void)
{
#if defined(STDOUT_RTT)
	volatile uint32_t *dhcsr = (uint32_t*)0xE000EDF0;
	if (*dhcsr & 1) {
		SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0,
				SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
	} else {
		SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0,
				SEGGER_RTT_MODE_NO_BLOCK_SKIP);
	}
#else
	uart2 = uart_create(2);
	uart_enable(uart2, 115200);
#endif

	static struct logging_backend log_console = {
		.write = stdout_writer,
	};

	logging_add_backend(&log_console);
}

void logger_init(void)
{
	logging_init(board_get_time_since_boot_ms);
	initialize_backend_stdout();
}
