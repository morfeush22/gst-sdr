/*
 * abstract_sink.h
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#ifndef SRC_ABSTRACT_SINK_H_
#define SRC_ABSTRACT_SINK_H_

#define ABSTRACT_SINK_CAST(x) ((AbstractSink *)x)
#define ABSTRACT_SINK_DATA_CAST(x) ((AbstractSinkHelpers::Data *)x)

namespace AbstractSinkHelpers {

struct Data {
	void *sink_data_;
	void *other_data_;
};

}

class AbstractSink {
public:
	virtual ~AbstractSink();
	AbstractSink();

	virtual void InitSink(AbstractSinkHelpers::Data *data) = 0;
	virtual const char *GetName() const = 0;
	virtual void Finish(AbstractSinkHelpers::Data *data) = 0;
	virtual bool IsLinked() const = 0;

	bool operator ==(const AbstractSink &other) const;

};

#endif /* SRC_ABSTRACT_SINK_H_ */
