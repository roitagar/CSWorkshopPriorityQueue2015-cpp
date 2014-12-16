/*
 * ThreadRandom.cpp
 *
 *  Created on: Dec 17, 2014
 *      Author: student
 */

#include "ThreadRandom.h"

ThreadRandom::ThreadRandom() {
	// TODO Auto-generated constructor stub

}

ThreadRandom::~ThreadRandom() {
	// TODO Auto-generated destructor stub
}

CCP::Random* ThreadRandom::initialValue()
{
	// TODO: is this the way it's meant to be?
	return new CCP::Random();
}
