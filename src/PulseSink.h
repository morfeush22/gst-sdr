/*
 * PulseSink.h
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#ifndef SRC_PULSESINK_H_
#define SRC_PULSESINK_H_

#include "AbstractSink.h"

#include <gst/gst.h>

class PulseSink: public AbstractSink {
public:
	PulseSink();
	virtual ~PulseSink();

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
	bool linked_;

};

#endif /* SRC_PULSESINK_H_ */
