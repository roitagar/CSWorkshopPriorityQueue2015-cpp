#include "NaiveLockSprayListPriorityQueue.h"
#include <stdlib.h>

NaiveLockSprayListPriorityQueue::NaiveLockSprayListPriorityQueue(int maxHeight)
: SprayListPriorityQueue(maxHeight)
{
	_threads = 0;
	_lock =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
}

NaiveLockSprayListPriorityQueue::~NaiveLockSprayListPriorityQueue() {
	pthread_mutex_destroy(&_lock);
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
	pthread_mutex_lock(&_lock);
	_threads++;
}
void NaiveLockSprayListPriorityQueue::EndInsert()
{
	_threads--;
	pthread_mutex_unlock(&_lock);
}
int NaiveLockSprayListPriorityQueue::RandomLevel()
{
	return RandomStep(_maxAllowedHeight);
}
void NaiveLockSprayListPriorityQueue::StartDeleteMin()
{
	pthread_mutex_lock(&_lock);
	_threads++;
}
void NaiveLockSprayListPriorityQueue::EndDeleteMin()
{
	_threads--;
	pthread_mutex_unlock(&_lock);
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
