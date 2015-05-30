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
#include "player.h"
#include <gst/gst.h>
#include <stdint.h>

namespace RingSrcHelpers {

struct Data {
	void *abstract_src;

	const size_t buff_size;
	const size_t buff_chunk_size;

	guint source_id;
};

gboolean ReadData(gpointer);
void StartFeed(GstElement *, guint, gpointer);
void StopFeed(GstElement *, gpointer);

}

class RingSrc: public AbstractSrc {
public:
	RingSrc(float, size_t);
	virtual ~RingSrc();

	void InitSrc(void *);
	const char *name() const;

	float DecrementRatio(Player *);
	float IncrementRatio(Player *);

	size_t ParseThreshold(float);
	void ProcessThreshold(Player *);

	void Write(float *, size_t);
	void set_last_frame(bool);

	friend gboolean RingSrcHelpers::ReadData(gpointer);
	friend void RingSrcHelpers::StartFeed(GstElement *, guint, gpointer);
	friend void RingSrcHelpers::StopFeed(GstElement *, gpointer);

private:
	BlockingRingBuffer *ring_buffer_;

	float threshold_;	//relative to 0.5
	float current_ratio_;

	AbstractSrcHelpers::Data *data_;

};

#endif /* SRC_RING_SRC_H_ */
