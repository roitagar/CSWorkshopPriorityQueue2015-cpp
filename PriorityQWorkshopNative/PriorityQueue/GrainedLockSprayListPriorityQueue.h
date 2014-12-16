/*
 * GrainedLockSprayListPriorityQueue.h
 *
 *  Created on: Dec 16, 2014
 *      Author: student
 */

#ifndef GRAINEDLOCKSPRAYLISTPRIORITYQUEUE_H_
#define GRAINEDLOCKSPRAYLISTPRIORITYQUEUE_H_

#include "SprayListPriorityQueue.h"
#include "../framework/cpp_framework.h"

class GrainedLockSprayListPriorityQueue: public SprayListPriorityQueue {
public:
	GrainedLockSprayListPriorityQueue(int maxHeight);
	virtual ~GrainedLockSprayListPriorityQueue();
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
	CCP::AtomicInteger _threads;
};

#endif /* GRAINEDLOCKSPRAYLISTPRIORITYQUEUE_H_ */
