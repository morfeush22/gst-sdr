/*
 * blocking_ring_buffer.h
 *
 *  Created on: May 24, 2015
 *      Author: morfeush22
 */

#ifndef SRC_BLOCKING_RING_BUFFER_H_
#define SRC_BLOCKING_RING_BUFFER_H_

#include "ring_buffer.h"
#include <pthread.h>

class BlockingRingBuffer {
public:
	BlockingRingBuffer(size_t);
	virtual ~BlockingRingBuffer();

	size_t ReadFrom(char *, size_t);
	size_t WriteInto(char *, size_t);

	size_t FreeSpace();
	size_t DataStored();

	void LastFrame();

private:
	RingBuffer<char> buffer_;

	pthread_mutex_t count_mutex_;
	pthread_cond_t count_condition_not_empty_;

	bool last_frame_;

};

#endif /* SRC_BLOCKING_RING_BUFFER_H_ */
