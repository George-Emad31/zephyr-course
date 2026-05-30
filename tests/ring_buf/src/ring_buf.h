/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * ring_buf — minimal circular FIFO byte buffer.
 */

#ifndef RING_BUF_H_
#define RING_BUF_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <errno.h>

/* ENODATA (61) is POSIX but absent from some libc variants; guard it. */
#ifndef ENODATA
#define ENODATA 61
#endif

struct app_rb {
	uint8_t *data;
	size_t   capacity;
	size_t   head;   /* index of the next byte to read  */
	size_t   tail;   /* index of the next byte to write */
	size_t   count;  /* number of bytes currently held  */
};

/**
 * @brief Initialise (or re-initialise) the ring buffer.
 *
 * Resets head, tail and count to 0.  Safe to call on an already-used
 * buffer — this is the "reinit clears state" guarantee the tests check.
 *
 * @param buf      Buffer descriptor.
 * @param capacity Maximum number of bytes the storage array can hold.
 * @param data     Backing storage array of at least @p capacity bytes.
 */
void app_rb_init(struct app_rb *buf, size_t capacity, uint8_t *data);

/**
 * @brief Push one byte onto the back of the buffer.
 *
 * @return 0 on success, -ENOMEM if the buffer is full.
 */
int app_rb_push(struct app_rb *buf, uint8_t item);

/**
 * @brief Pop one byte from the front of the buffer.
 *
 * @param item  Destination for the byte.  Pass NULL to discard without
 *              storing the value — the byte is still consumed.
 * @return 0 on success, -ENODATA if the buffer is empty.
 */
int app_rb_pop(struct app_rb *buf, uint8_t *item);

/**
 * @brief Peek at the next byte without consuming it.
 *
 * @return 0 on success, -ENODATA if the buffer is empty.
 */
int app_rb_peek(struct app_rb *buf, uint8_t *item);

/** @return true when the buffer holds no bytes. */
bool app_rb_is_empty(struct app_rb *buf);

/** @return true when the buffer is at capacity. */
bool app_rb_is_full(struct app_rb *buf);

#endif /* RING_BUF_H_ */
