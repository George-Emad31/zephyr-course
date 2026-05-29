/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * ring_buf — circular FIFO byte buffer implementation.
 */

#include "ring_buf.h"

void app_rb_init(struct app_rb *buf, size_t capacity, uint8_t *data)
{
	buf->data     = data;
	buf->capacity = capacity;
	buf->head     = 0;
	buf->tail     = 0;
	buf->count    = 0;
}

int app_rb_push(struct app_rb *buf, uint8_t item)
{
	if (buf->count == buf->capacity) {
		return -ENOMEM;
	}

	buf->data[buf->tail] = item;
	buf->tail = (buf->tail + 1) % buf->capacity;
	buf->count++;
	return 0;
}

int app_rb_pop(struct app_rb *buf, uint8_t *item)
{
	if (buf->count == 0) {
		return -ENODATA;
	}

	if (item != NULL) {
		*item = buf->data[buf->head];
	}

	buf->head = (buf->head + 1) % buf->capacity;
	buf->count--;
	return 0;
}

int app_rb_peek(struct app_rb *buf, uint8_t *item)
{
	if (buf->count == 0) {
		return -ENODATA;
	}

	*item = buf->data[buf->head];
	return 0;
}

bool app_rb_is_empty(struct app_rb *buf)
{
	return buf->count == 0;
}

bool app_rb_is_full(struct app_rb *buf)
{
	return buf->count == buf->capacity;
}
