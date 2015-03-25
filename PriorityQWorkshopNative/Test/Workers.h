#include "../PriorityQueue/IPriorityQueue.h"
#include "../PriorityQueue/news/IPriorityQueue.h"
#include "../framework/cpp_framework.h"
#include <stdio.h>

#include "../PriorityQueue/news/serviceClass.h"

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
			_queue->insert(i);
		}

		serviceClass::cleanup();
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
		while(!_queue->isEmpty())
		{
			int result;
			result = _queue->deleteMin();
			printf("%d\n",result); //TODO modify
		}

		serviceClass::cleanup();
	}
};
