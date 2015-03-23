#ifndef SEQSPRAYLISTPRIORITYQUEUE_H_
#define SEQSPRAYLISTPRIORITYQUEUE_H_

#include "IPriorityQueue.h"
#include "../../framework/cpp_framework.h"
#include "../../util/ThreadRandom.h"
#include "serviceClass.h"

class SeqSprayListNode
{
public:
	int value;
	SeqSprayListNode** next;
	SeqSprayListNode(int value, int height);
	~SeqSprayListNode();
	int topLevel();

private:
	int _height;
};

class SeqSprayListPriorityQueue : public IPriorityQueue
{
public:
	SeqSprayListPriorityQueue(int maxAllowedHeight);
	virtual ~SeqSprayListPriorityQueue();
	virtual bool insert(int value);
	virtual int deleteMin();
	virtual bool isEmpty();
	SeqSprayListNode* head() { return _head;} // For printing purposes // TODO: Remove
protected:
	SeqSprayListNode* _head;
	SeqSprayListNode* _tail;
	int _maxAllowedHeight;
	ThreadRandom _random;
	serviceClass _service;

	/**
	 * Returns the number of active threads. Used for spray height.
	 * concurrent implementations should override this for correctness.
	 */
	virtual int getThreadCount();

	int find(int value, SeqSprayListNode** preds, SeqSprayListNode** succs);

	bool remove(int value);

	int spray(int H, int L, int D);
};

#endif /* SEQSPRAYLISTPRIORITYQUEUE_H_ */
