/*
 *	@class DataFeeder
 * 	@brief implements basic ring buffer capabilities
 *
 *
 *  @date Created on: May 13, 2015
 *	@author: hipek <pawel_szulc@onet.pl>
 *
 */
#include <cstdlib>
#include <sys/types.h>
#include <cstdio>
#include <iostream>
#include <cstddef>
#include <cstring>
#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

class RingBuffer{
public:
	RingBuffer(size_t size);

	virtual ~RingBuffer();

	float ReadNext ();
	void WriteNext (float f);

	// safe methods
	float sReadNext ();
	bool sWriteNext (float f);

	size_t ReadFrom (float *dest_buffer, size_t number_to_write);
	size_t WriteInto (float *source_buffer, size_t number_to_write);

	// safe methods (not overwriting/reading)
	size_t sReadFrom (float *dest_buffer, size_t number_to_write);
	size_t sWriteInto (float *source_buffer, size_t number_to_write);

	size_t DataStored();
	size_t FreeSpace();

protected:

	size_t HeadToRightEnd();
	size_t TailToRightEnd();
	float *buffer;
	size_t head, tail;
	bool last_op_write;
	size_t length;

};



#endif /* RINGBUFFER_H_ */
