/*
 * AbstractSink.cpp
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#include "AbstractSink.h"
#include <string.h>

AbstractSink::~AbstractSink() {
}

AbstractSink::AbstractSink() {
}

bool AbstractSink::operator ==(const AbstractSink &other) const {
	return !strcmp(GetName(), other.GetName());
}
