/*
 * file_sink.h
 *
 *  Created on: May 21, 2015
 *      Author: morfeush22
 */

#ifndef SRC_FILE_SINK_H_
#define SRC_FILE_SINK_H_

#define FILE_SINK_CAST(X) (reinterpret_cast<FileSink *>(X))
#define FILE_SINK_DATA_CAST(X) (reinterpret_cast<FileSinkHelpers::Data *>(X))

#include "abstract_sink.h"
#include <gst/gst.h>

namespace FileSinkHelpers {

struct Data {
	void *abstract_sink;

	GstElement *queue;
	GstElement *sink;
	GstPad *teepad;

	gboolean removing;
	bool linked;
};

}

class FileSink: public AbstractSink {
public:
	FileSink(const char *);
	virtual ~FileSink();

	void InitSink(void *);
	const char *name() const;
	void Finish();
	bool linked() const;

private:
	const char *path_;
	AbstractSinkHelpers::Data *data_;

};

#endif /* SRC_FILE_SINK_H_ */
