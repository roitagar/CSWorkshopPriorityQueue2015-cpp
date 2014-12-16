#ifndef SPRAYLISTPRIORITYQUEUE_H_
#define SPRAYLISTPRIORITYQUEUE_H_

#include "IPriorityQueue.h"

class SprayListNode
{
public:
	int value;
	SprayListNode** next;
	SprayListNode(int value, int height);
	~SprayListNode();
	bool IsMarked();
	void SetMark();
	bool IsFullyLinked();
	void SetFullyLinked();
	int TopLevel();

private:
	int _height;
	bool _fullyLinked;
	bool _marked;
	// TODO: Add mutex,
};

class SprayListPriorityQueue : public IPriorityQueue
{
public:
	SprayListPriorityQueue(int maxAllowedHeight);
	virtual ~SprayListPriorityQueue();
	virtual void Insert(int value);
	virtual int DeleteMin();
	virtual bool IsEmpty();
protected:
	SprayListNode* _head;
	SprayListNode* _tail;
	int _maxAllowedHeight;

	virtual bool CanInsertBetween(SprayListNode* pred, SprayListNode* succ, int level) = 0;
	virtual void LockNode(SprayListNode* node) = 0;
	virtual void UnlockNode(SprayListNode* node) = 0;
	virtual void StartInsert() = 0;
	virtual void EndInsert() = 0;
	virtual int RandomLevel() = 0;
	virtual void StartDeleteMin() = 0;
	virtual void EndDeleteMin() = 0;

	// Number of threads currently calling deleteMin
	virtual int GetNumberOfThreads() = 0;

	virtual int RandomStep(int max) = 0;

	virtual bool ReadyToBeDeleted(SprayListNode* victim) = 0;

	int Find(int value, SprayListNode** preds, SprayListNode** succs);

private:
	int Spray(int H, int L, int D);
	bool Remove(int value);
};

#endif /* SPRAYLISTPRIORITYQUEUE_H_ */
