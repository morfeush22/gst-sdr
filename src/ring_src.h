/*
 * ring_src.h
 *
 *  Created on: May 17, 2015
 *      Author: morfeush22
 */

#ifndef SRC_RING_SRC_H_
#define SRC_RING_SRC_H_

#define RING_SRC_CAST(X) (reinterpret_cast<RingSrc *>(X))
#define RING_SRC_DATA_CAST(X) (reinterpret_cast<RingSrcHelpers::Data *>(X))

#include "abstract_src.h"
#include "blocking_ring_buffer.h"
#include <gst/gst.h>
#include <stdint.h>

namespace RingSrcHelpers {

struct Data {
	void *abstract_src;

	BlockingRingBuffer *ring_buffer;
	const size_t buff_size;
	const size_t buff_chunk_size;

	guint source_id;
};

}

class RingSrc: public AbstractSrc {
public:
	RingSrc(float, size_t);
	virtual ~RingSrc();

	void InitSrc(void *);
	const char *name() const;

	float DecrementRatio(void *);
	float IncrementRatio(void *);

	size_t ParseThreshold(float);
	void ProcessThreshold(AbstractSrcHelpers::Data *);

	void Write(float *, size_t);
	void set_last_frame(bool);

private:
	AbstractSrcHelpers::Data *data_;

	float threshold_;	//relative to 0.5
	float current_ratio_;

};

#endif /* SRC_RING_SRC_H_ */
