/*
 * PulseSink.h
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#ifndef SRC_PULSESINK_H_
#define SRC_PULSESINK_H_

#include "AbstractSink.h"

class PulseSink: public AbstractSink {
public:
	PulseSink();
	virtual ~PulseSink();

	void InitSink(void *);
	const char *GetName();
};

#endif /* SRC_PULSESINK_H_ */
