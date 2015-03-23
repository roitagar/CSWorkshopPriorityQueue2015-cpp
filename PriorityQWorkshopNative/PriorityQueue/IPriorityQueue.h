#ifndef PRIORITYQUEUE_OLD_H_
#define PRIORITYQUEUE_OLD_H_

class IPriorityQueueOld
{
public:
	virtual void Insert(int value) = 0;
	virtual int DeleteMin() = 0;
	virtual bool IsEmpty() = 0;
//	virtual int size() = 0;
};

#endif /* PRIORITYQUEUE_OLD_H_ */
