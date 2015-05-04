#include "../PriorityQueue/news/IPriorityQueue.h"
#include "../PriorityQueue/news/GlobalLockSprayListPriorityQueue.h"
#include "../PriorityQueue/news/LazyLockSparyListPriorityQueue.h"

class PriorityQueueFactory {
public:
	virtual IPriorityQueue* Create(int skiplistHeight) = 0;
	virtual const char* getQueueType() = 0;
};

class GlobalLockSprayListPriorityQueueFactory : public PriorityQueueFactory {
	virtual IPriorityQueue* Create(int skiplistHeight) {
		return new GlobalLockSprayListPriorityQueue(skiplistHeight);
	}
	virtual const char* getQueueType() {
		return "GlobalLockSprayListPriorityQueue_CPP";
	}
};

class CoolSprayListPriorityQueueFactory : public PriorityQueueFactory {
	virtual IPriorityQueue* Create(int skiplistHeight) {
		return new CoolSprayListPriorityQueue(skiplistHeight, false);
	}
	virtual const char* getQueueType() {
		return "CoolSprayListPriorityQueue_CPP";
	}
};

class CoolSprayListPriorityQueueFairLockFactory : public PriorityQueueFactory {
	virtual IPriorityQueue* Create(int skiplistHeight) {
		return new CoolSprayListPriorityQueue(skiplistHeight, true);
	}
	virtual const char* getQueueType() {
		return "CoolSprayListPriorityQueueFairLock_CPP";
	}
};

//class CoolSprayListPriorityQueueWithImpreciseIsEmptyFactory extends PriorityQueueFactory {
//	@Override
//	IPriorityQueue Create(int skiplistHeight) {
//		return new CoolSprayListPriorityQueue(skiplistHeight, false);
//	}
//}

class LazyLockSparyListPriorityQueueFactory : public PriorityQueueFactory {
	virtual IPriorityQueue* Create(int skiplistHeight) {
		return new LazyLockSparyListPriorityQueue(skiplistHeight);
	}
	virtual const char* getQueueType() {
		return "LazyLockSparyListPriorityQueue_CPP";
	}
};
