#ifndef STOPWATCH_H_
#define STOPWATCH_H_

#include "../framework/cpp_framework.h"

class StopWatch
{
private:
	tick_t startTime;
	tick_t stopTime;
public:
	void StartTimer()
	{
		startTime = CCP::System::currentTimeMillis();
	}

	void StopTimer()
	{
		stopTime = CCP::System::currentTimeMillis();
	}

	tick_t GetElapsedTime()
	{
		return stopTime - startTime;
	}
};

#endif /* STOPWATCH_H_ */
