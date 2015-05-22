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
	virtual const char *GetName() const = 0;
	virtual void FinishEarly(void *data) = 0;

	bool operator ==(const AbstractSink &other) const;

};

namespace AbstractSinkHelpers {

struct Data {
	AbstractSink *sink_;
	void *other_data_;
};

}

#endif /* SRC_ABSTRACTSINK_H_ */
