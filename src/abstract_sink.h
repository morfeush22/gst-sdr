/*
 * abstract_sink.h
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#ifndef SRC_ABSTRACT_SINK_H_
#define SRC_ABSTRACT_SINK_H_

#define ABSTRACT_SINK_CAST(X) (reinterpret_cast<AbstractSink *>(X))
#define ABSTRACT_SINK_DATA_CAST(X) (reinterpret_cast<AbstractSinkHelpers::Data *>(X))

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

	virtual void InitSink(void *other_data) = 0;
	virtual const char *GetName() const = 0;
	virtual void Finish() = 0;
	virtual bool IsLinked() const = 0;

	bool operator ==(const AbstractSink &other) const;

};

#endif /* SRC_ABSTRACT_SINK_H_ */
