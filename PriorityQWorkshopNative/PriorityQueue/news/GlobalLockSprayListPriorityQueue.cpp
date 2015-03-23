#include "GlobalLockSprayListPriorityQueue.h"

GlobalLockSprayListPriorityQueue::GlobalLockSprayListPriorityQueue(int maxAllowedHeight)
	: SeqSprayListPriorityQueue(maxAllowedHeight) {
}

GlobalLockSprayListPriorityQueue::~GlobalLockSprayListPriorityQueue()
{

}
	
bool GlobalLockSprayListPriorityQueue::insert(int value) {
	bool ret = false;
	_lock.lock();

	ret = SeqSprayListPriorityQueue::insert(value);

	_lock.unlock();

	return ret;
}

int GlobalLockSprayListPriorityQueue::deleteMin() {
	int ret;
	_lock.lock();

	ret = SeqSprayListPriorityQueue::deleteMin();

	_lock.unlock();

	return ret;	
}
