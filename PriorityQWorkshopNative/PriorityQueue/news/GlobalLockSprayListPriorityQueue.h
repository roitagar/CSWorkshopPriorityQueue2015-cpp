#include "SeqSprayListPriorityQueue.h"
#include "../../framework/cpp_framework.h"

class GlobalLockSprayListPriorityQueue : public SeqSprayListPriorityQueue{
	/*This is a global Lock that used in insert and deleteMin methods
	 * isEmpty() and getSize() are wait-free*/
	
	CCP::ReentrantLock _lock;
public:
	GlobalLockSprayListPriorityQueue(int maxAllowedHeight);
	virtual ~GlobalLockSprayListPriorityQueue();
	
	virtual bool insert(int value);
	virtual int deleteMin();
};
