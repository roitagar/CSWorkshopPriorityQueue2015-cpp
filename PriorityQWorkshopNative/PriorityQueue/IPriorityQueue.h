#ifndef PRIORITYQUEUE_H_
#define PRIORITYQUEUE_H_

class IPriorityQueue
{
public:
	virtual void Insert(int value) = 0;
	virtual int DeleteMin() = 0;
	virtual bool IsEmpty() = 0;
//	virtual int size() = 0;
};

#endif /* PRIORITYQUEUE_H_ */
