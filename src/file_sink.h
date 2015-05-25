/*
 * file_sink.h
 *
 *  Created on: May 21, 2015
 *      Author: morfeush22
 */

#ifndef SRC_FILE_SINK_H_
#define SRC_FILE_SINK_H_

#include "abstract_sink.h"

#include <gst/gst.h>

class FileSink: public AbstractSink {
public:
	FileSink(const char *);
	virtual ~FileSink();

	virtual void InitSink(AbstractSinkHelpers::Data *);
	const char *GetName() const;
	void Finish(AbstractSinkHelpers::Data *);
	bool IsLinked() const;

protected:
	const char *path_;
	bool linked_;

private:
	FileSinkHelpers::Data data_;

};

namespace FileSinkHelpers {

struct Data {
	FileSink *abstract_sink_;

	GstElement *queue_;
	GstElement *sink_;
	GstPad *teepad_;

	gboolean removing_;
};

}

#endif /* SRC_FILE_SINK_H_ */
