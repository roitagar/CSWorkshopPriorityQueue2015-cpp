#ifndef PRIORITYQUEUE_H_
#define PRIORITYQUEUE_H_

class IPriorityQueue
{
public:
	virtual bool insert(int value) = 0;
	virtual int deleteMin() = 0;
	virtual bool isEmpty() = 0;
};

#endif /* PRIORITYQUEUE_H_ */
