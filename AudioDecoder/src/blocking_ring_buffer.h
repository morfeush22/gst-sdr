/*
 * blocking_ring_buffer.h
 *
 *  Created on: May 24, 2015
 *      Author: morfeush22
 */

#ifndef SRC_BLOCKING_RING_BUFFER_H_
#define SRC_BLOCKING_RING_BUFFER_H_

#include "RingBuffer/ring_buffer.h"
#include <stdint.h>
#include <pthread.h>

class BlockingRingBuffer {
public:
	BlockingRingBuffer(size_t);
	virtual ~BlockingRingBuffer();

	size_t ReadFrom(uint8_t *, size_t);
	size_t WriteInto(uint8_t *, size_t);

	size_t FreeSpace();
	size_t DataStored();

	const bool last_frame() const;
	void set_last_frame(bool);

private:
	RingBuffer<uint8_t> buffer_;

	pthread_mutex_t count_mutex_;
	pthread_cond_t count_condition_not_empty_;

	bool last_frame_;

};

#endif /* SRC_BLOCKING_RING_BUFFER_H_ */
