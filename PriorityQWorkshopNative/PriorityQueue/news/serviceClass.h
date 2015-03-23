#ifndef SERVICECLASS_H_
#define SERVICECLASS_H_

#include "../../util/ThreadRandom.h"

class serviceClass
{
public:
	int randomStep(int max);

	int randomLevel(int maxAllowedHeight);

private:
	ThreadRandom _random;
};

#endif /* SERVICECLASS_H_ */
