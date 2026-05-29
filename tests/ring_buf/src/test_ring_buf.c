/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * ZTest suite for the ring_buf circular FIFO buffer module.
 *
 * Suites and tests
 * ----------------
 *  app_rb_init     (2 tests)
 *    test_init_basic           – pre-filled example: empty/not-full after init
 *    test_reinit_clears_state  – push items, reinit, verify all state reset
 *
 *  app_rb_push_pop (3 tests)
 *    test_push_pop_single      – push one byte, pop it, value matches
 *    test_fifo_order           – push 3 bytes, pop 3, verify FIFO order
 *    test_full_buf_push_fails  – fill buffer, next push returns -ENOMEM
 *
 *  ring_buf_boundaries (3 tests)
 *    test_peek_non_consuming   – peek does not consume the byte
 *    test_pop_null_discards    – pop(NULL) consumes without storing
 *    test_is_full_after_fill   – is_full true only after capacity reached
 */

#include <zephyr/ztest.h>
#include "ring_buf.h"

#define BUF_SIZE 4U

static uint8_t        storage[BUF_SIZE];
static struct app_rb rb;

/* ======================================================================
 * Suite: app_rb_init
 * ====================================================================*/
ZTEST_SUITE(app_rb_init, NULL, NULL, NULL, NULL, NULL);

/**
 * Pre-filled example test.
 * After app_rb_init() the buffer must be empty and not full.
 */
ZTEST(app_rb_init, test_init_basic)
{
	app_rb_init(&rb, BUF_SIZE, storage);

	zassert_true(app_rb_is_empty(&rb),
		     "buffer must be empty immediately after init");
	zassert_false(app_rb_is_full(&rb),
		      "buffer must not be full immediately after init");
}

/**
 * Re-initialising an already-used buffer must reset all internal state:
 * head, tail and count must all return to 0, and the buffer must report
 * empty / not-full.
 */
ZTEST(app_rb_init, test_reinit_clears_state)
{
	app_rb_init(&rb, BUF_SIZE, storage);
	app_rb_push(&rb, 0xAA);
	app_rb_push(&rb, 0xBB);

	/* Re-initialise — must wipe all state */
	app_rb_init(&rb, BUF_SIZE, storage);

	zassert_equal(rb.count, 0U, "count must be 0 after reinit");
	zassert_equal(rb.head,  0U, "head must be 0 after reinit");
	zassert_equal(rb.tail,  0U, "tail must be 0 after reinit");
	zassert_true(app_rb_is_empty(&rb), "buffer must be empty after reinit");
	zassert_false(app_rb_is_full(&rb), "buffer must not be full after reinit");
}

/* ======================================================================
 * Suite: app_rb_push_pop
 * ====================================================================*/
ZTEST_SUITE(app_rb_push_pop, NULL, NULL, NULL, NULL, NULL);

/**
 * A single push followed by a single pop must return the same byte and
 * leave the buffer empty.
 */
ZTEST(app_rb_push_pop, test_push_pop_single)
{
	app_rb_init(&rb, BUF_SIZE, storage);

	zassert_equal(app_rb_push(&rb, 0x42), 0, "push must return 0");

	uint8_t out = 0;

	zassert_equal(app_rb_pop(&rb, &out), 0, "pop must return 0");
	zassert_equal(out, 0x42, "popped value must match pushed value");
	zassert_true(app_rb_is_empty(&rb),
		     "buffer must be empty after single push/pop");
}

/**
 * Items must come out in the same order they were pushed in (FIFO).
 */
ZTEST(app_rb_push_pop, test_fifo_order)
{
	app_rb_init(&rb, BUF_SIZE, storage);

	app_rb_push(&rb, 1U);
	app_rb_push(&rb, 2U);
	app_rb_push(&rb, 3U);

	uint8_t a = 0, b = 0, c = 0;

	app_rb_pop(&rb, &a);
	app_rb_pop(&rb, &b);
	app_rb_pop(&rb, &c);

	zassert_equal(a, 1U, "first pop must return first pushed byte");
	zassert_equal(b, 2U, "second pop must return second pushed byte");
	zassert_equal(c, 3U, "third pop must return third pushed byte");
}

/**
 * Pushing into a full buffer must fail with -ENOMEM; the buffer must
 * still report full.
 */
ZTEST(app_rb_push_pop, test_full_buf_push_fails)
{
	app_rb_init(&rb, BUF_SIZE, storage);

	for (size_t i = 0; i < BUF_SIZE; i++) {
		zassert_equal(app_rb_push(&rb, (uint8_t)i), 0,
			      "push into non-full buffer must succeed");
	}

	zassert_equal(app_rb_push(&rb, 0xFF), -ENOMEM,
		      "push into full buffer must return -ENOMEM");
	zassert_true(app_rb_is_full(&rb),
		     "buffer must still be full after failed push");
}

/* ======================================================================
 * Suite: ring_buf_boundaries
 * ====================================================================*/
ZTEST_SUITE(ring_buf_boundaries, NULL, NULL, NULL, NULL, NULL);

/**
 * peek() must return the front byte without consuming it; a subsequent
 * pop() must return the same byte and leave the buffer empty.
 */
ZTEST(ring_buf_boundaries, test_peek_non_consuming)
{
	app_rb_init(&rb, BUF_SIZE, storage);
	app_rb_push(&rb, 0x55);

	uint8_t peeked = 0;

	zassert_equal(app_rb_peek(&rb, &peeked), 0, "peek must return 0");
	zassert_equal(peeked, 0x55, "peeked value must match pushed value");

	/* The byte must still be in the buffer */
	zassert_equal(rb.count, 1U, "count must be 1 after peek");

	uint8_t popped = 0;

	zassert_equal(app_rb_pop(&rb, &popped), 0,
		      "pop after peek must succeed");
	zassert_equal(popped, 0x55,
		      "pop after peek must return the same byte");
	zassert_true(app_rb_is_empty(&rb),
		     "buffer must be empty after pop");
}

/**
 * pop(NULL) must consume the front byte (decrement count) without
 * storing it anywhere — no crash, buffer empty afterwards.
 */
ZTEST(ring_buf_boundaries, test_pop_null_discards)
{
	app_rb_init(&rb, BUF_SIZE, storage);
	app_rb_push(&rb, 0xDE);

	zassert_equal(app_rb_pop(&rb, NULL), 0,
		      "pop(NULL) must return 0");
	zassert_true(app_rb_is_empty(&rb),
		     "buffer must be empty after pop(NULL)");
}

/**
 * is_full() must return false for any partially-filled buffer and true
 * only once exactly BUF_SIZE bytes have been pushed.
 */
ZTEST(ring_buf_boundaries, test_is_full_after_fill)
{
	app_rb_init(&rb, BUF_SIZE, storage);

	zassert_false(app_rb_is_full(&rb),
		      "fresh buffer must not be full");

	for (size_t i = 0; i < BUF_SIZE - 1U; i++) {
		app_rb_push(&rb, (uint8_t)i);
		zassert_false(app_rb_is_full(&rb),
			      "partially-filled buffer must not be full");
	}

	app_rb_push(&rb, 0xFFU);
	zassert_true(app_rb_is_full(&rb),
		     "buffer must be full after BUF_SIZE pushes");
	zassert_false(app_rb_is_empty(&rb),
		      "full buffer must not report empty");
}
