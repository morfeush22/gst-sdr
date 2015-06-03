/*
 * fake_sink.h
 *
 *  Created on: May 13, 2015
 *      Author: Kacper Patro patro.kacper@gmail.com
 */

#ifndef SRC_FAKE_SINK_H_
#define SRC_FAKE_SINK_H_

#define FAKE_SINK_CAST(X) (reinterpret_cast<FakeSink *>(X))
#define FAKE_SINK_DATA_CAST(X) (reinterpret_cast<FakeSinkHelpers::Data *>(X))

#include "abstract_sink.h"
#include <gst/gst.h>
#include <stdint.h>

namespace FakeSinkHelpers {

/**
 * @struct Data
 * @biref This struct contains specific for FakeSink class elements
 */
struct Data {
	void *abstract_sink;	/**< Pointer to "this" sink element */

	GstElement *queue;	/**< Queue element for GStreamer */
	GstElement *sink;	/**< Sink element for GStreamer */
	GstPad *teepad;	/**< TeePad element for GStreamer */

	gboolean removing;	/**< True, when sink is being removed from pipeline */
	bool linked;	/**< True, when sink is linked in pipeline */
};

/**
 * GStreamer callback called when unlinking sink from pipeline. Check GStreamer documentation for more
 */
GstPadProbeReturn UnlinkCall(GstPad *, GstPadProbeInfo *, gpointer);

}

/**
 * @class FakeSink
 * @brief This class is used in unittests, provides specific necessary information about stream. Could be used multiple times in one pipeline
 *
 * @author Kacper Patro patro.kacper@gmail.com
 * @copyright Public domain
 * @pre
 */
class FakeSink: public AbstractSink {
public:
	FakeSink();
	virtual ~FakeSink();

	/**
	 * Get number of bytes received at the end of pipeline
	 * @return Number of bytes received
	 */
	uint32_t bytes_returned() const;

	/**
	 * Adds bytes to sum number of bytes received
	 * @param bytes Number of bytes, which should be added
	 */
	void AddBytes(uint32_t);

	void InitSink(void *);
	const char *name() const;
	void Finish();
	bool linked() const;

	/**
	 * Gets number of linked sink elements
	 * @return Number of current linked sink elements
	 */
	uint32_t num_src_pads() const;

	/**
	 * Gets current playback speed, relative to 1.0
	 * @return Current playback speed
	 */
	float playback_speed() const;

	friend GstPadProbeReturn FakeSinkHelpers::UnlinkCall(GstPad *, GstPadProbeInfo *, gpointer);

private:
	AbstractSinkHelpers::Data *data_;

	uint32_t bytes_returned_;

	static uint16_t count_;

};

#endif /* SRC_FAKE_SINK_H_ */
