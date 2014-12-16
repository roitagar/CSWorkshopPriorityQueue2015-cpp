#include "NaiveLockSprayListPriorityQueue.h"
#include <stdlib.h>

NaiveLockSprayListPriorityQueue::NaiveLockSprayListPriorityQueue(int maxHeight)
: SprayListPriorityQueue(maxHeight)
{
	_threads = 0;
}

NaiveLockSprayListPriorityQueue::~NaiveLockSprayListPriorityQueue() {
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
void NaiveLockSprayListPriorityQueue::StartInsert()
{
	// TODO: Lock
	_threads++;
}
void NaiveLockSprayListPriorityQueue::EndInsert()
{
	// TODO: unlock
	_threads--;
}
int NaiveLockSprayListPriorityQueue::RandomLevel()
{
	return RandomStep(_maxAllowedHeight);
}
void NaiveLockSprayListPriorityQueue::StartDeleteMin()
{
	// TODO: Lock
	_threads++;
}
void NaiveLockSprayListPriorityQueue::EndDeleteMin()
{
	// TODO: Unlock
	_threads--;
}

// Number of threads currently calling deleteMin
int NaiveLockSprayListPriorityQueue::GetNumberOfThreads()
{
	return _threads;
}

int NaiveLockSprayListPriorityQueue::RandomStep(int max)
{
	return rand()%(max+1);
}

bool NaiveLockSprayListPriorityQueue::ReadyToBeDeleted(SprayListNode* victim)
{
	return true;
}
