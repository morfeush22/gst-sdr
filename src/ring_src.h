/*
 * ring_src.h
 *
 *  Created on: May 17, 2015
 *      Author: morfeush22
 */

#ifndef SRC_RING_SRC_H_
#define SRC_RING_SRC_H_

#include <gst/gst.h>

#include <stdint.h>
#include "abstract_src.h"
#include "blocking_ring_buffer.h"

class RingSrc: public AbstractSrc {
public:
	RingSrc(float threshold);
	virtual ~RingSrc();

	void InitSrc(void *);
	const char *GetName();

	float DecrementRatio(void *);
	float IncrementRatio(void *);

	size_t ParseThreshold(float);
	void ProcessThreshold(void *ptr);

	BlockingRingBuffer *GetRingBuffer();

	guint source_id_;

private:
	float threshold_;	//relative to 1.0
	float current_ratio_;

	BlockingRingBuffer *ring_buffer_;

};

#endif /* SRC_RING_SRC_H_ */
