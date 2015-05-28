/*
 * fake_sink.h
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#ifndef SRC_FAKE_SINK_H_
#define SRC_FAKE_SINK_H_

#define FAKE_SINK_CAST(X) (reinterpret_cast<FakeSink *>(X))
#define FAKE_SINK_DATA_CAST(X) (reinterpret_cast<FakeSinkHelpers::Data *>(X))

#include "abstract_sink.h"
#include <gst/gst.h>
#include <stdint.h>

namespace FakeSinkHelpers {

struct Data {
	void *abstract_sink;

	GstElement *queue;
	GstElement *sink;
	GstPad *teepad;

	gboolean removing;
};

}

class FakeSink: public AbstractSink {
public:
	FakeSink();
	virtual ~FakeSink();

	uint32_t bytes_returned();
	void AddBytes(uint32_t);

	void InitSink(void *);
	const char *get_name() const;
	void Finish();
	bool linked() const;

private:
	AbstractSinkHelpers::Data *data_;

	uint32_t bytes_returned_;
	bool linked_;

	static uint16_t count_;

};

#endif /* SRC_FAKE_SINK_H_ */
