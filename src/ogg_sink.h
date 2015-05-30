/*
 * ogg_sink.h
 *
 *  Created on: May 29, 2015
 *      Author: morfeush22
 */

#ifndef SRC_OGG_SINK_H_
#define SRC_OGG_SINK_H_

#define OGG_SINK_CAST(X) (reinterpret_cast<OggSink *>(X))
#define OGG_SINK_DATA_CAST(X) (reinterpret_cast<OggSinkHelpers::Data *>(X))

#include "abstract_sink.h"
#include <gst/gst.h>

namespace OggSinkHelpers {

struct Data {
	void *abstract_sink;

	GstElement *queue;
	GstElement *encoder;
	GstElement *muxer;
	GstElement *sink;
	GstPad *teepad;

	gboolean removing;
	bool linked;
};

}

class OggSink: public AbstractSink {
public:
	OggSink(const char *);
	virtual ~OggSink();

	void InitSink(void *);
	const char *name() const;
	void Finish();
	bool linked() const;

private:
	const char *path_;
	AbstractSinkHelpers::Data *data_;

};

#endif /* SRC_OGG_SINK_H_ */
