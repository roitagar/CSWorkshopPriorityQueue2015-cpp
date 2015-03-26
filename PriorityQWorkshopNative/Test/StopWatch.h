#ifndef STOPWATCH_H_
#define STOPWATCH_H_

#include "../framework/cpp_framework.h"

class StopWatch
{
private:
	tick_t startTime;
	tick_t stopTime;
public:
	void startTimer()
	{
		startTime = CCP::System::currentTimeMillis();
	}

	void stopTimer()
	{
		stopTime = CCP::System::currentTimeMillis();
	}

	tick_t getElapsedTime()
	{
		return stopTime - startTime;
	}
};

#endif /* STOPWATCH_H_ */
