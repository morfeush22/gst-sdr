/*
 * ring_src.h
 *
 *  Created on: May 17, 2015
 *      Author: Kacper Patro patro.kacper@gmail.com
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

/**
 * @struct Data
 * @biref This struct contains specific for RingSrc class elements
 */
struct Data {
	void *abstract_src;	/**< Pointer to "this" src element */

	GstElement *src;	/**< Src element for GStreamer */

	guint source_id;	/**< SID, needed by GStreamer callbacks*/
};

extern "C" {
/**
 * GStreamer callback called when feed started. Check GStreamer documentation for more
 */
gboolean ReadData(gpointer);

/**
 * GStreamer callback called when need more data. Check GStreamer documentation for more
 */
void StartFeed(GstElement *, guint, gpointer);

/**
 * GStreamer callback called when feeding should be stopped. Check GStreamer documentation for more
 */
void StopFeed(GstElement *, gpointer);

}

}

/**
 * @class RingSrc
 * @brief Class used to read from buffer. Introduces internal ring buffer, threshold for audio time-stretching
 *
 * @author Kacper Patro patro.kacper@gmail.com
 * @copyright Public domain
 * @pre
 */
class RingSrc: public AbstractSrc {
public:
	/**
	 * Constructor of RingSrc
	 * @param threshold Relative to 0.5, indicates values for which audio time-stretching will take place
	 * @param buffer_length Length of internal blocking ring buffer
	 */
	RingSrc(float, size_t);
	virtual ~RingSrc();

	void SetSrc(void *);
	void LinkSrc();
	const char *name() const;

	/**
	 * Decrements playback speed
	 * @param player Pointer to Player
	 * @return Current playback speed
	 */
	float DecrementRatio(Player *);

	/**
	 * Increments playback speed
	 * @param player Pointer to Player
	 * @return Current playback speed
	 */
	float IncrementRatio(Player *);

	/**
	 * Parses threshold
	 * @param fraction Fraction to calculate number of probes corresponding internal buffer size
	 * @return Number of probes corresponding to given fraction
	 */
	size_t ParseThreshold(float);

	/**
	 * Process audio time-stretching depending upon current internal buffer fullness
	 */
	void ProcessThreshold();

	/**
	 * Writes into internal ring buffer, could override oldest data when full
	 * @param source_buffer Pointer to source buffer
	 * @param how_many Number of elements to read from source buffer
	 */
	void Write(uint8_t *, size_t);

	/**
	 * Sets last frame property. Needed to properly invoke cleaning procedures
	 * @param to
	 */
	void set_last_frame(bool);

	friend gboolean RingSrcHelpers::ReadData(gpointer);
	friend void RingSrcHelpers::StartFeed(GstElement *, guint, gpointer);
	friend void RingSrcHelpers::StopFeed(GstElement *, gpointer);

private:
	AbstractSrcHelpers::Data *data_;

	float threshold_;	//relative to 0.5
	float current_ratio_;

	const size_t buff_size_;
	const size_t buff_chunk_size_;

	BlockingRingBuffer *ring_buffer_;

};

#endif /* SRC_RING_SRC_H_ */
