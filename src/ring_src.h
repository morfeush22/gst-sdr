/*
 * ring_src.h
 *
 *  Created on: May 17, 2015
 *      Author: morfeush22
 */

#ifndef SRC_RING_SRC_H_
#define SRC_RING_SRC_H_

#define RING_DATA_CAST(X) ((RingSrc *)X)
#define RING_SRC_DATA_CAST(X) ((RingSrcHelpers::Data *)X)

#include "abstract_src.h"
#include "blocking_ring_buffer.h"
#include <gst/gst.h>
#include <stdint.h>

namespace RingSrcHelpers {

struct Data {
	void *abstract_src_;

	BlockingRingBuffer *ring_buffer_;

	guint source_id_;
};

}

class RingSrc: public AbstractSrc {
public:
	RingSrc(float threshold);
	virtual ~RingSrc();

	void InitSrc(void *);
	const char *GetName();

	float DecrementRatio(void *);
	float IncrementRatio(void *);

	size_t ParseThreshold(float);
	void ProcessThreshold(AbstractSrcHelpers::Data *);

	void Write(float *, size_t);
	void LastFrame();

private:
	AbstractSrcHelpers::Data *data_;

	float threshold_;	//relative to 1.0
	float current_ratio_;

};

#endif /* SRC_RING_SRC_H_ */
