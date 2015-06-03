/*
 * pulse_sink.h
 *
 *  Created on: May 13, 2015
 *      Author: Kacper Patro patro.kacper@gmail.com
 */

#ifndef SRC_PULSE_SINK_H_
#define SRC_PULSE_SINK_H_

#define PULSE_SINK_CAST(X) (reinterpret_cast<PulseSink *>(X))
#define PULSE_SINK_DATA_CAST(X) (reinterpret_cast<PulseSinkHelpers::Data *>(X))

#include "abstract_sink.h"
#include <gst/gst.h>

namespace PulseSinkHelpers {

/**
 * @struct Data
 * @biref This struct contains specific for PulseSink class elements
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

class PulseSink: public AbstractSink {
public:
	PulseSink();
	virtual ~PulseSink();

	void InitSink(void *);
	const char *name() const;
	void Finish();
	bool linked() const;

private:
	AbstractSinkHelpers::Data *data_;

};

#endif /* SRC_PULSE_SINK_H_ */
