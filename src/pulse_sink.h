/*
 * pulse_sink.h
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#ifndef SRC_PULSE_SINK_H_
#define SRC_PULSE_SINK_H_

#define PULSE_SINK_CAST(x) ((PulseSink *)x)
#define PULSE_SINK_DATA_CAST(x) ((PulseSinkHelpers::Data *)(x))

#include "abstract_sink.h"
#include <gst/gst.h>

namespace PulseSinkHelpers {

struct Data {
	void *abstract_sink_;

	GstElement *queue_;
	GstElement *sink_;
	GstPad *teepad_;

	gboolean removing_;
};

}

class PulseSink: public AbstractSink {
public:
	PulseSink();
	virtual ~PulseSink();

	void InitSink(AbstractSinkHelpers::Data *);
	const char *GetName() const;
	void Finish(AbstractSinkHelpers::Data *);
	bool IsLinked() const;

private:
	PulseSinkHelpers::Data data_;

	bool linked_;

};

#endif /* SRC_PULSE_SINK_H_ */
