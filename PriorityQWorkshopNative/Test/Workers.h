#include "../PriorityQueue/IPriorityQueue.h"
#include "../framework/cpp_framework.h"
#include <stdio.h>

class InsertWorker : public CCP::Thread
{
private:
	IPriorityQueue* _queue;
	int _amount;
	int _from;
public:
	InsertWorker(IPriorityQueue* queue, int from, int amount)
	{
		this->_queue = queue;
		this->_from = from;
		this->_amount = amount;
	}

	virtual void run()
	{
		for(int i=_from;i<_from+_amount;i++){
			_queue->Insert(i);
		}
	}
};

class DeleteWorker : public CCP::Thread
{
private:
	IPriorityQueue* _queue;

public:
	DeleteWorker(IPriorityQueue* queue)
	{
		this->_queue = queue;
	}

	void run()
	{
		while(!_queue->IsEmpty())
		{
			int result;
			result = _queue->DeleteMin();
			printf("%d\n",result); //TODO modify
		}
	}
};
