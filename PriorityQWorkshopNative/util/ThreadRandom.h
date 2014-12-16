/*
 * ThreadRandom.h
 *
 *  Created on: Dec 17, 2014
 *      Author: student
 */

#ifndef THREADRANDOM_H_
#define THREADRANDOM_H_

#include "../framework/cpp_framework.h"

class ThreadRandom: public CCP::ThreadLocal<CCP::Random*> {
public:
	ThreadRandom();
	virtual ~ThreadRandom();
	virtual CCP::Random* initialValue();
};

#endif /* THREADRANDOM_H_ */
