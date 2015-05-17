/*
 * AbstractSink.h
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#ifndef SRC_ABSTRACTSINK_H_
#define SRC_ABSTRACTSINK_H_

class AbstractSink {
public:
	virtual ~AbstractSink();
	AbstractSink();

	virtual void InitSink(void *data) = 0;
	virtual const char *GetName() = 0;

};

#endif /* SRC_ABSTRACTSINK_H_ */
