/*
 * fake_sink.h
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#ifndef SRC_FAKE_SINK_H_
#define SRC_FAKE_SINK_H_

#define FAKE_SINK_CAST(X) ((FakeSink *)X)
#define FAKE_SINK_DATA_CAST(X) ((FakeSinkHelpers::Data *)X)

#include "abstract_sink.h"
#include <gst/gst.h>
#include <stdint.h>

namespace FakeSinkHelpers {

struct Data {
	void *abstract_sink_;

	GstElement *queue_;
	GstElement *sink_;
	GstPad *teepad_;

	gboolean removing_;
};

}

class FakeSink: public AbstractSink {
public:
	FakeSink();
	virtual ~FakeSink();

	uint32_t GetBytesReturned();
	void AddBytes(uint32_t);

	void InitSink(void *);
	const char *GetName() const;
	void Finish();
	bool IsLinked() const;

private:
	AbstractSinkHelpers::Data *data_;

	uint32_t bytes_returned_;
	bool linked_;

};

#endif /* SRC_FAKE_SINK_H_ */
