/*
 * blocking_ring_buffer.h
 *
 *  Created on: May 24, 2015
 *      Author: Kacper Patro patro.kacper@gmail.com
 */

#ifndef SRC_BLOCKING_RING_BUFFER_H_
#define SRC_BLOCKING_RING_BUFFER_H_

#include "RingBuffer/ring_buffer.h"
#include <stdint.h>
#include <pthread.h>

/**
 * @class BlockingRingBuffer
 * @brief Based on Paweł Szulc implementation, this buffer blocks when empty and overrides oldest data when overflowed
 *
 * @author Kacper Patro patro.kacper@gmail.com
 * @copyright Public domain
 * @pre
 */
class BlockingRingBuffer {
public:
	/**
	 * Constructor of BlockingRingBuffer
	 * @param length Length of internal ring buffer
	 */
	BlockingRingBuffer(size_t);
	virtual ~BlockingRingBuffer();

	/**
	 * Reads from internal ring buffer, could block when empty
	 * @param dest_buffer Pointer to destination buffer
	 * @param how_many Number of elements to save in destination buffer
	 * @return Number of elements written to destination buffer
	 */
	size_t ReadFrom(uint8_t *, size_t);

	/**
	 * Writes into internal ring buffer, could override oldest data when full
	 * @param source_buffer Pointer to source buffer
	 * @param how_many Number of elements to read from source buffer
	 * @return Number of elements written into internal buffer
	 */
	size_t WriteInto(uint8_t *, size_t);

	/**
	 * Checks free space
	 * @return Free space of internal buffer
	 */
	size_t FreeSpace();

	/**
	 * Checks data stored
	 * @return Number of items stored
	 */
	size_t DataStored();

	/**
	 * Checks, if last frame property is set
	 * @return True when set, false otherwise
	 */
	const bool last_frame() const;

	/**
	 * Sets last frame property
	 * @param to
	 */
	void set_last_frame(bool);

private:
	RingBuffer<uint8_t> buffer_;

	pthread_mutex_t count_mutex_;
	pthread_cond_t count_condition_not_empty_;

	bool last_frame_;

};

#endif /* SRC_BLOCKING_RING_BUFFER_H_ */
