/*
 * FileSink.h
 *
 *  Created on: May 21, 2015
 *      Author: morfeush22
 */

#ifndef SRC_FILESINK_H_
#define SRC_FILESINK_H_

#include "AbstractSink.h"

#include <gst/gst.h>

class FileSink: public AbstractSink {
public:
	FileSink(const char *);
	virtual ~FileSink();

	void InitSink(void *);
	const char *GetName() const;
	void FinishEarly(void *);
	bool IsLinked() const;
	void UnlinkFinished();

	GstElement *queue_;
	GstElement *sink_;
	GstPad *teepad_;

	gboolean removing_;

protected:
	const char *path_;
	bool linked_;

};

#endif /* SRC_FILESINK_H_ */
