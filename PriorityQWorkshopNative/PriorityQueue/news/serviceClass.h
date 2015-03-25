#ifndef SERVICECLASS_H_
#define SERVICECLASS_H_

#include "../../util/ThreadRandom.h"

class serviceClass
{
public:
	static int randomStep(int max);

	static int randomLevel(int maxAllowedHeight);

	// Deletes the ThreadRandom object of the calling thread
	// Only call before thread completion
	static void cleanup();

private:
	static ThreadRandom _random;
};

#endif /* SERVICECLASS_H_ */
