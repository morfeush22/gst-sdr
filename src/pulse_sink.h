/*
 * pulse_sink.h
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#ifndef SRC_PULSE_SINK_H_
#define SRC_PULSE_SINK_H_

#include "abstract_sink.h"

#include <gst/gst.h>

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

namespace PulseSinkHelpers {

struct Data {
	PulseSink *abstract_sink_;

	GstElement *queue_;
	GstElement *sink_;
	GstPad *teepad_;

	gboolean removing_;
};

}

#endif /* SRC_PULSE_SINK_H_ */
