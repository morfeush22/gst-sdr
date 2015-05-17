/*
 * RingSrc.h
 *
 *  Created on: May 17, 2015
 *      Author: morfeush22
 */

#ifndef SRC_RINGSRC_H_
#define SRC_RINGSRC_H_

#include "AbstractSrc.h"
#include <stdint.h>
#include <gst/gst.h>
#include "ring_buffer.h"
#include "file_wrapper.h"	//should not be

class RingSrc: public AbstractSrc {
public:
	RingSrc(const char *, float threshold);
	virtual ~RingSrc();

	void InitSrc(void *);
	const char *GetName();

	float DecrementRatio(void *);
	float IncrementRatio(void *);

	guint *GetSourceId();

	size_t ReadFromFile();	//should not be in final code
	size_t ParseThreshold(float);
	void ProcessThreshold(void *ptr);
	RingBuffer<char> *GetRingBuffer();

private:
	guint source_id_;
	float threshold_;	//in percent
	const char *path_;	//should not be in final code
	FileWrapper *file_wrapper_;	//should not be
	RingBuffer<char> *ring_buffer_;
	float current_ratio_;

};

#endif /* SRC_RINGSRC_H_ */
