#pragma once

#include "../PriorityQueue/IPriorityQueue.h"
#include "../PriorityQueue/news/IPriorityQueue.h"
#include "../framework/cpp_framework.h"
#include <stdio.h>

#include "../PriorityQueue/news/serviceClass.h"
#include <vector>

class InsertWorker : public CCP::Thread {
protected:
	int _totalPackets;
public:
	int totalPackets()
	{
		return _totalPackets;
	}

};

class GradedWorkerBase : public CCP::Thread {
	IPriorityQueue* _queue;
protected:
	std::vector<int> _values;

public:
	GradedWorkerBase(IPriorityQueue* queue) :
		_queue(queue)
	{
	}

	int deleteMin(){
		int result = _queue->deleteMin();
		if(result!=INT_MAX){
			_values.push_back(result);
		}
		return result;
	}

	/**
	 * Counting #inversions of the _value array
	 *
	 * Each delete worker creates an array, grade = 0 -> optimum
	 * @return
	 */
	virtual int getGrade(){
		int grade=0;

		//check with bubble sort the goodness of the del. min
//		for(int i=0;i<_values.size()-1;i++){
//			for(int j=i;j<_values.size();j++){
//				if(_values.get(i)>_values.get(j)){
//					grade++;
//				}
//			}
//		}

		int old = INT_MIN;

		for(int i=0;i<_values.size();i++)
		{
			int value = _values[i];
			if(old>value)
			{
				// out of order, count a bad point
				grade++;
			}

			old = value;
		}

		return grade;
	}

	int totalPackets()
	{
		return _values.size();
	}
};

class SimpleInsertWorker : public InsertWorker
{
private:
	IPriorityQueue* _queue;
	int _amount;
	int _from;
public:
	SimpleInsertWorker(IPriorityQueue* queue, int from, int amount)
	{
		this->_queue = queue;
		this->_from = from;
		this->_amount = amount;
		_totalPackets = 0;
	}

	virtual void run()
	{
		for(int i=_from;i<_from+_amount;i++){
			if(_queue->insert(i))
				_totalPackets++;
		}

		serviceClass::cleanup();
	}
};

class SimpleDeleteWorker : public GradedWorkerBase
{
private:
	IPriorityQueue* _queue;

public:
	SimpleDeleteWorker(IPriorityQueue* queue) :
		GradedWorkerBase(queue)
	{
		this->_queue = queue;
	}

	void run()
	{
		while(!_queue->isEmpty())
		{
			int result;
			result = _queue->deleteMin();
		}

		serviceClass::cleanup();
	}
};


class INumberGenerator
{
public:
	virtual int getNext() = 0;
};



class AdvancedInsertAndDelete : public GradedWorkerBase {
	IPriorityQueue* _queue;
	int _runs;
	int _highest;

public:
	AdvancedInsertAndDelete(
			IPriorityQueue* queue, int runs, int highest) :
			GradedWorkerBase(queue)
	{
		_queue = queue;
		_runs=runs;
		_highest = highest;
	}

	virtual void run()
	{
		int counter=0;
		int result;
		int value;
		while( _runs > counter)
		{
			result = deleteMin();
			value = result + _highest;
			_queue->insert(value);
			counter++;
		}

		serviceClass::cleanup();
	}

	/**
	 * Ranking the worker - grade = 0 -> optimum
	 * Each delete min with result which is higher then the _highest increasing grade by 1 (bad point)
	 * @return
	 */
	virtual int getGrade(){
		int grade=0;
		for(int i=0; i<_values.size(); i++){
			if(_values[i] > _highest){
				grade++;
			}
		}
		return grade;

	}

};


class AdvancedInsertWorker : public InsertWorker {
	bool* _done;
	INumberGenerator* _generator;
	IPriorityQueue* _queue;

public:
	AdvancedInsertWorker(
		bool* done,
		INumberGenerator* generator,
		IPriorityQueue* queue)
	{
		_done = done;
		_generator = generator;
		_queue = queue;
		_totalPackets = 0;
	}

	~AdvancedInsertWorker()
	{
		delete _generator;
	}

	virtual void run()
	{
		while( !(*_done) )
		{
			int value = _generator->getNext();
			if(value>0){
				if(_queue->insert(value))
					_totalPackets++;
			}
		}
	}
};


class AdvancedInsertWorkerUntilValue : public InsertWorker {
	INumberGenerator* _generator;
	IPriorityQueue* _queue;
	int _finishWithValue;

public:
	AdvancedInsertWorkerUntilValue(
			INumberGenerator* generator,
			IPriorityQueue* queue,
			int finishWithValue)
	{
		_generator = generator;
		_queue = queue;
		_totalPackets = 0;
		_finishWithValue = finishWithValue;
	}

	virtual void run()
	{

		bool reallyDone = false;
		while( !reallyDone ) {
			reallyDone = true; // if we got the last value we need to finish
			int value = _generator->getNext();

			if(value!=_finishWithValue)
			{
				// if the value is not as expected, need to keep going
				reallyDone = false;
				if(_queue->insert(value))
					_totalPackets++;
			}

		}

	}
};

class AdvancedDeleteWorker : public GradedWorkerBase
{
	bool* _done;
	IPriorityQueue* _queue;

public:
	AdvancedDeleteWorker(
			bool* done,
			IPriorityQueue* queue) :
			GradedWorkerBase(queue)
	{
		_done = done;
		_queue = queue;
	}

	virtual void run()
	{
		// if done is triggered and queue is not empty, reallyDone would keep the loop running.
		// if done is triggered and queue is empty, the loop will not set reallyDone back to false, and exit.
		bool reallyDone = false;
		while( !reallyDone ) {
			reallyDone = *_done; // if done is marked, we might need to finish
			int result;

			result = deleteMin();

			if(!_queue->isEmpty())
			{
				// if the queue is not empty, need to keep going
				reallyDone = false;
			}
		}
	}
};

class AdvancedDeleteWorkerWithoutEmptying : public GradedWorkerBase
{
	bool* _done;
	IPriorityQueue* _queue;

public:
	AdvancedDeleteWorkerWithoutEmptying(
			bool* done,
			IPriorityQueue* queue) :
			GradedWorkerBase(queue)
	{
		_done = done;
		_queue = queue;
	}

	virtual void run()
	{
		// if done is triggered and queue is not empty, reallyDone would keep the loop running.
		// if done is triggered and queue is empty, the loop will not set reallyDone back to false, and exit.
		boolean done = false;
		while( !done ) {
			done = *_done; // if done is marked, we might need to finish
			int result;

			result = deleteMin();
		}
	}
};
