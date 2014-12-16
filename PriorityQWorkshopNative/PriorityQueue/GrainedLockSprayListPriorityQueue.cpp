#include "GrainedLockSprayListPriorityQueue.h"

GrainedLockSprayListPriorityQueue::GrainedLockSprayListPriorityQueue(int maxHeight)
: SprayListPriorityQueue(maxHeight), _threads(0)
{
	// TODO Auto-generated constructor stub
}

GrainedLockSprayListPriorityQueue::~GrainedLockSprayListPriorityQueue() {
	// TODO Auto-generated destructor stub
}

void GrainedLockSprayListPriorityQueue::StartInsert()
{
	_threads.incrementAndGet();
}

void GrainedLockSprayListPriorityQueue::EndInsert()
{
	_threads.decrementAndGet();
}

int GrainedLockSprayListPriorityQueue::RandomLevel()
{
	return RandomStep(_maxAllowedHeight);
}

void GrainedLockSprayListPriorityQueue::StartDeleteMin()
{
	_threads.incrementAndGet();
}

void GrainedLockSprayListPriorityQueue::EndDeleteMin()
{
	_threads.decrementAndGet();
}

// Number of threads currently calling deleteMin
int GrainedLockSprayListPriorityQueue::GetNumberOfThreads()
{
	return _threads.get();
}

int GrainedLockSprayListPriorityQueue::RandomStep(int max)
{
	return _random.get()->nextInt(max+1);
}

bool GrainedLockSprayListPriorityQueue::CanInsertBetween(SprayListNode* pred, SprayListNode* succ, int level)
{
	return !pred->IsMarked() && !succ->IsMarked() && pred->next[level]==succ;
}

void GrainedLockSprayListPriorityQueue::LockNode(SprayListNode* node)
{
	node->lock.lock();
}

void GrainedLockSprayListPriorityQueue::UnlockNode(SprayListNode* node)
{
	node->lock.unlock();
}

bool GrainedLockSprayListPriorityQueue::ReadyToBeDeleted(SprayListNode* node)
{
	return node->IsFullyLinked() && !node->IsMarked();
}
