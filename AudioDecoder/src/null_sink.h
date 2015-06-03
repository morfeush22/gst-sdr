/*
 * null_sink.h
 *
 *  Created on: Jun 3, 2015
 *      Author: Kacper Patro patro.kacper@gmail.com
 */

#ifndef AUDIODECODER_SRC_NULL_SINK_H_
#define AUDIODECODER_SRC_NULL_SINK_H_

#define NULL_SINK_CAST(X) (reinterpret_cast<NullSink *>(X))
#define NULL_SINK_DATA_CAST(X) (reinterpret_cast<NullSinkHelpers::Data *>(X))

#include "abstract_sink.h"
#include <gst/gst.h>

namespace NullSinkHelpers {

/**
 * @struct Data
 * @biref This struct contains specific for NullSink class elements
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
 * @class NullSink
 * @brief Base class to keep input buffer empty when not playing
 *
 * @author Kacper Patro patro.kacper@gmail.com
 * @copyright Public domain
 * @pre
 */
class NullSink: public AbstractSink {
public:
	NullSink();
	virtual ~NullSink();

	void InitSink(void *);
	const char *name() const;
	void Finish();
	bool linked() const;

	friend GstPadProbeReturn NullSinkHelpers::UnlinkCall(GstPad *, GstPadProbeInfo *, gpointer);

private:
	AbstractSinkHelpers::Data *data_;

};

#endif /* AUDIODECODER_SRC_NULL_SINK_H_ */
