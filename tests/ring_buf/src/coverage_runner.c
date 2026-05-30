/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Standalone coverage driver for ring_buf.
 *
 * Mirrors every ZTEST case from test_ring_buf.c but uses only the C
 * standard library (assert, stdio) so it compiles without Zephyr.
 * Build with:
 *   clang --coverage -I. -o rb_runner ring_buf.c coverage_runner.c
 * Then run ./rb_runner and invoke gcovr / llvm-cov gcov.
 */

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include "ring_buf.h"

#define BUF_SIZE 4U

static uint8_t      storage[BUF_SIZE];
static struct app_rb rb;

#define RUN(fn) \
	do { fn(); printf("PASS  %s\n", #fn); } while (0)

/* ------------------------------------------------------------------ */
/* Suite: ring_buf_init                                                 */
/* ------------------------------------------------------------------ */

static void test_init_basic(void)
{
	app_rb_init(&rb, BUF_SIZE, storage);
	assert(app_rb_is_empty(&rb));
	assert(!app_rb_is_full(&rb));
}

static void test_reinit_clears_state(void)
{
	app_rb_init(&rb, BUF_SIZE, storage);
	app_rb_push(&rb, 0xAAU);
	app_rb_push(&rb, 0xBBU);

	app_rb_init(&rb, BUF_SIZE, storage);
	assert(rb.count    == 0U);
	assert(rb.head     == 0U);
	assert(rb.tail     == 0U);
	assert(app_rb_is_empty(&rb));
	assert(!app_rb_is_full(&rb));
}

/* ------------------------------------------------------------------ */
/* Suite: ring_buf_push_pop                                             */
/* ------------------------------------------------------------------ */

static void test_push_pop_single(void)
{
	app_rb_init(&rb, BUF_SIZE, storage);
	assert(app_rb_push(&rb, 0x42U) == 0);
	uint8_t out = 0U;
	assert(app_rb_pop(&rb, &out) == 0);
	assert(out == 0x42U);
	assert(app_rb_is_empty(&rb));
}

static void test_fifo_order(void)
{
	app_rb_init(&rb, BUF_SIZE, storage);
	app_rb_push(&rb, 1U);
	app_rb_push(&rb, 2U);
	app_rb_push(&rb, 3U);
	uint8_t a = 0, b = 0, c = 0;
	app_rb_pop(&rb, &a);
	app_rb_pop(&rb, &b);
	app_rb_pop(&rb, &c);
	assert(a == 1U);
	assert(b == 2U);
	assert(c == 3U);
}

static void test_full_buf_push_fails(void)
{
	app_rb_init(&rb, BUF_SIZE, storage);
	for (size_t i = 0; i < BUF_SIZE; i++) {
		assert(app_rb_push(&rb, (uint8_t)i) == 0);
	}
	assert(app_rb_push(&rb, 0xFFU) == -ENOMEM);
	assert(app_rb_is_full(&rb));
}

/* ------------------------------------------------------------------ */
/* Suite: ring_buf_boundaries                                           */
/* ------------------------------------------------------------------ */

static void test_peek_non_consuming(void)
{
	app_rb_init(&rb, BUF_SIZE, storage);
	app_rb_push(&rb, 0x55U);
	uint8_t peeked = 0U;
	assert(app_rb_peek(&rb, &peeked) == 0);
	assert(peeked == 0x55U);
	assert(rb.count == 1U);          /* still in buffer */
	uint8_t popped = 0U;
	assert(app_rb_pop(&rb, &popped) == 0);
	assert(popped == 0x55U);
	assert(app_rb_is_empty(&rb));
}

static void test_pop_null_discards(void)
{
	app_rb_init(&rb, BUF_SIZE, storage);
	app_rb_push(&rb, 0xDEU);
	assert(app_rb_pop(&rb, NULL) == 0);
	assert(app_rb_is_empty(&rb));
}

static void test_is_full_after_fill(void)
{
	app_rb_init(&rb, BUF_SIZE, storage);
	assert(!app_rb_is_full(&rb));
	for (size_t i = 0; i < BUF_SIZE - 1U; i++) {
		app_rb_push(&rb, (uint8_t)i);
		assert(!app_rb_is_full(&rb));
	}
	app_rb_push(&rb, 0xFFU);
	assert(app_rb_is_full(&rb));
	assert(!app_rb_is_empty(&rb));
}

/* ------------------------------------------------------------------ */
/* Error-path coverage (pop/peek on empty)                             */
/* ------------------------------------------------------------------ */

static void test_empty_errors(void)
{
	app_rb_init(&rb, BUF_SIZE, storage);
	uint8_t dummy = 0U;
	assert(app_rb_pop(&rb, &dummy)  == -ENODATA);
	assert(app_rb_peek(&rb, &dummy) == -ENODATA);
}

/* ------------------------------------------------------------------ */

int main(void)
{
	printf("=== ring_buf coverage runner ===\n");
	RUN(test_init_basic);
	RUN(test_reinit_clears_state);
	RUN(test_push_pop_single);
	RUN(test_fifo_order);
	RUN(test_full_buf_push_fails);
	RUN(test_peek_non_consuming);
	RUN(test_pop_null_discards);
	RUN(test_is_full_after_fill);
	RUN(test_empty_errors);
	printf("=== All 9 cases passed ===\n");
	return 0;
}
