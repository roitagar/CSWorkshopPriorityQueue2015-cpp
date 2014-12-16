/*
 * NaiveLockSprayListPriorityQueue.h
 *
 *  Created on: Dec 16, 2014
 *      Author: student
 */

#ifndef NAIVELOCKSPRAYLISTPRIORITYQUEUE_H_
#define NAIVELOCKSPRAYLISTPRIORITYQUEUE_H_

#include "SprayListPriorityQueue.h"
#include <pthread.h>

class NaiveLockSprayListPriorityQueue: public SprayListPriorityQueue {
public:
	NaiveLockSprayListPriorityQueue(int height);
	virtual ~NaiveLockSprayListPriorityQueue();
	SprayListNode* head() { return _head;} // TODO: Remove
protected:
	virtual bool CanInsertBetween(SprayListNode* pred, SprayListNode* succ, int level);
	virtual void LockNode(SprayListNode* node);
	virtual void UnlockNode(SprayListNode* node);
	virtual void StartInsert();
	virtual void EndInsert();
	virtual int RandomLevel();
	virtual void StartDeleteMin();
	virtual void EndDeleteMin();

	// Number of threads currently calling deleteMin
	virtual int GetNumberOfThreads();

	virtual int RandomStep(int max);

	virtual bool ReadyToBeDeleted(SprayListNode* victim);

private:
	int _threads;
	pthread_mutex_t _lock;
};

#endif /* NAIVELOCKSPRAYLISTPRIORITYQUEUE_H_ */
