/*
 * fake_sink.h
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#ifndef SRC_FAKE_SINK_H_
#define SRC_FAKE_SINK_H_

#include "abstract_sink.h"

#include <gst/gst.h>

#include <stdint.h>


class FakeSink: public AbstractSink {
public:
	FakeSink();
	virtual ~FakeSink();

	uint32_t GetBytesReturned();
	void AddBytes(uint32_t);

	void InitSink(void *);
	const char *GetName() const;
	void FinishEarly(void *);
	bool IsLinked() const;
	void UnlinkFinished();

	GstElement *queue_;
	GstElement *sink_;
	GstPad *teepad_;

	gboolean removing_;

private:
	uint32_t bytes_returned_;
	bool linked_;

};

#endif /* SRC_FAKE_SINK_H_ */
