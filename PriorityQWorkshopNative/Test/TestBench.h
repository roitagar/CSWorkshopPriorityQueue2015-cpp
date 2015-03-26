#include "../PriorityQueue/news/IPriorityQueue.h"
#include "../framework/cpp_framework.h"
#include <stdlib.h>


class TestBenchResult {
public:
	~TestBenchResult() {
		delete [] grade;
	}
	long insertTime;
	long deleteTime;
	long insertCount;
	long deleteCount;
	int* grade;

};

class TestBench {
private:
	TestBenchResult* _result;
protected:
	IPriorityQueue* _queue;
	int _numInsertWorkers;
	int _numDeleteWorkers;
	int _highest;
	int _timeOutMilliseconds;

	CCP::Thread** _insertWorkerThreads;
	CCP::Thread** _deleteWorkerThreads;
	StopWatch _insertTimer;
	StopWatch _deleteTimer;

public:

	inline void setNumInsertWorkers(int numInsertWorkers){
		_numInsertWorkers = numInsertWorkers;
	}
	inline void setNumDeleteWorkers(int numDeleteWorkers){
		_numDeleteWorkers= numDeleteWorkers;
	}

	inline void setHighestOnQueue(int highest){
		_highest = highest;
	}
	
	inline int getItemsPerThread()
	{
		return _highest/_numInsertWorkers;
	}

	inline void setTimeOutMillisecond(int timeOutMillisecond){
		_timeOutMilliseconds = timeOutMillisecond;
	}
	
protected:
	inline void startAllWorkers()
	{
		startInsertWorkers();
		startDeleteWorkers();
	}
	
	inline void startInsertWorkers()
	{
		_insertTimer.startTimer();
		
		for(int i=0;i<_numInsertWorkers;i++)
		{
			_insertWorkerThreads[i]->start();
		}
	}
	
	inline void startDeleteWorkers()
	{
		_deleteTimer.startTimer();
		
		for(int i=0;i<_numDeleteWorkers;i++)
		{
			_deleteWorkerThreads[i]->start();
		}
	}
	
	inline void joinInsertWorkers()
	{
//		bool kicked = false;
		for(int i=0;i<_numInsertWorkers;i++)
		{
			_insertWorkerThreads[i]->join();
//			_insertWorkerThreads[i]->join(kicked ? 1000 : 50000);
//			if(_insertWorkerThreads[i]->isAlive())
//			{
//				_insertWorkerThreads[i]->interrupt();
//				_insertWorkerThreads[i]->join();
//				kicked = true;
//			}
		}
		
		_insertTimer.stopTimer();
	}
	
	inline void joinDeleteWorkers()
	{
//		bool kicked = false;
		for(int i=0;i<_numDeleteWorkers;i++)
		{
			_deleteWorkerThreads[i]->join();
//			_deleteWorkerThreads[i]->join(kicked ? 1000 : 50000);
//			if(_deleteWorkerThreads[i]->isAlive())
//			{
//				_deleteWorkerThreads[i]->interrupt();
//				_deleteWorkerThreads[i]->join();
//				kicked = true;
//			}
		}
		
		_deleteTimer.stopTimer();
	}

public:
	inline void runTest() {
		// TODO: Initialize signals?
		
		// TODO: Initialize arrays
		
		// TODO: Initialize workers/threads
		
		run();
		
		// TODO: Calculate statistics/results
		
		// TODO: get grades
		
		// TODO: save result
	}

protected:
	virtual void run() = 0;

public:
	inline void setQueue(IPriorityQueue* queue){

		_queue = queue;
	}

	inline TestBenchResult* getResult(){
		return _result;
	}

protected:
	inline void saveResult(long insertCount, long deleteCount,	int* grade){

		TestBenchResult* result= new TestBenchResult();

		result->deleteCount=deleteCount;
		result->deleteTime=_deleteTimer.getElapsedTime();
		result->insertCount=insertCount;
		result->insertTime=_insertTimer.getElapsedTime();
		result->grade=grade;

		if(_result != NULL)
			delete _result;

		_result=result;
	}

};
