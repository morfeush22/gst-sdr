/*
 * blocking_ring_buffer.cpp
 *
 *  Created on: May 24, 2015
 *      Author: morfeush22
 */

#include "blocking_ring_buffer.h"

BlockingRingBuffer::BlockingRingBuffer(size_t length):
buffer_(length),
last_frame_(false) {
	pthread_mutex_init(&count_mutex_, NULL);
	pthread_cond_init(&count_condition_not_empty_, NULL);
}

BlockingRingBuffer::~BlockingRingBuffer() {
	pthread_cond_destroy(&count_condition_not_empty_);
	pthread_mutex_destroy(&count_mutex_);
}

size_t BlockingRingBuffer::ReadFrom(float *dest_buffer, size_t number_to_write) {
	pthread_mutex_lock(&count_mutex_);

	while(!FreeSpace() && !last_frame_)
		pthread_cond_wait(&count_condition_not_empty_, &count_mutex_);

	size_t result = buffer_.ReadFrom(dest_buffer, number_to_write);

	pthread_mutex_unlock(&count_mutex_);

	return result;
}

size_t BlockingRingBuffer::WriteInto(float *source_buffer, size_t number_to_write) {
	pthread_mutex_lock(&count_mutex_);

	size_t result = buffer_.WriteInto(source_buffer, number_to_write);

	pthread_cond_signal(&count_condition_not_empty_);
	pthread_mutex_unlock(&count_mutex_);

	return result;
}

size_t BlockingRingBuffer::FreeSpace() {
	return buffer_.FreeSpace();
}

size_t BlockingRingBuffer::DataStored() {
	return buffer_.DataStored();
}

void BlockingRingBuffer::LastFrame() {
	last_frame_ = true;
}
