/*
 * pulse_sink.h
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#ifndef SRC_PULSE_SINK_H_
#define SRC_PULSE_SINK_H_

#define PULSE_SINK_CAST(X) (reinterpret_cast<PulseSink *>(X))
#define PULSE_SINK_DATA_CAST(X) (reinterpret_cast<PulseSinkHelpers::Data *>(X))

#include "abstract_sink.h"
#include <gst/gst.h>

namespace PulseSinkHelpers {

struct Data {
	void *abstract_sink;

	GstElement *queue;
	GstElement *sink;
	GstPad *teepad;

	gboolean removing;
	bool linked;
};

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
