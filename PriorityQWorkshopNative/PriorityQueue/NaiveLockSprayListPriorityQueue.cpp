#include "NaiveLockSprayListPriorityQueue.h"

NaiveLockSprayListPriorityQueue::NaiveLockSprayListPriorityQueue(int maxHeight)
: SprayListPriorityQueue(maxHeight)
{
	_threads = 0;
}

NaiveLockSprayListPriorityQueue::~NaiveLockSprayListPriorityQueue() {
}

void NaiveLockSprayListPriorityQueue::StartInsert()
{
	_lock.lock();
	_threads++;
}

void NaiveLockSprayListPriorityQueue::EndInsert()
{
	_threads--;
	_lock.unlock();
}

int NaiveLockSprayListPriorityQueue::RandomLevel()
{
	return RandomStep(_maxAllowedHeight);
}

void NaiveLockSprayListPriorityQueue::StartDeleteMin()
{
	_lock.lock();
	_threads++;
}

void NaiveLockSprayListPriorityQueue::EndDeleteMin()
{
	_threads--;
	_lock.unlock();
}

// Number of threads currently calling deleteMin
int NaiveLockSprayListPriorityQueue::GetNumberOfThreads()
{
	return _threads;
}

int NaiveLockSprayListPriorityQueue::RandomStep(int max)
{
	return _random.get()->nextInt(max+1);
}

bool NaiveLockSprayListPriorityQueue::CanInsertBetween(SprayListNode* pred, SprayListNode* succ, int level)
{
	return true;
}

void NaiveLockSprayListPriorityQueue::LockNode(SprayListNode* node)
{

}

void NaiveLockSprayListPriorityQueue::UnlockNode(SprayListNode* node)
{

}

bool NaiveLockSprayListPriorityQueue::ReadyToBeDeleted(SprayListNode* victim)
{
	return true;
}
