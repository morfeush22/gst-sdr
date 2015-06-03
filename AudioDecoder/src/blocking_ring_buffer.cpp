/*
 * blocking_ring_buffer.cpp
 *
 *  Created on: May 24, 2015
 *      Author: Kacper Patro patro.kacper@gmail.com
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

size_t BlockingRingBuffer::ReadFrom(uint8_t *dest_buffer, size_t how_many) {
	pthread_mutex_lock(&count_mutex_);

	while(!DataStored() && !last_frame_)
		pthread_cond_wait(&count_condition_not_empty_, &count_mutex_);

	size_t result = buffer_.sReadFrom(dest_buffer, how_many);

	pthread_mutex_unlock(&count_mutex_);

	return result;
}

size_t BlockingRingBuffer::WriteInto(uint8_t *source_buffer, size_t how_many) {
	pthread_mutex_lock(&count_mutex_);

	size_t result = buffer_.WriteInto(source_buffer, how_many);

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

void BlockingRingBuffer::set_last_frame(bool to) {
	last_frame_ = to;
}

bool BlockingRingBuffer::last_frame() const {
	return last_frame_;
}
