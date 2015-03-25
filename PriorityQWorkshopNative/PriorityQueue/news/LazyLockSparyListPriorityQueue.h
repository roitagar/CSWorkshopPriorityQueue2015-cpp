#ifndef LAZYLOCKSPRAYLISTPRIORITYQUEUE_H_
#define LAZYLOCKSPRAYLISTPRIORITYQUEUE_H_

#include <memory>
#include "IPriorityQueue.h"
#include "../../framework/cpp_framework.h"

/*This is a lazy fine-grained locked-base version */


class LazyLockSprayListNode {
public:
	int value;
	std::shared_ptr<LazyLockSprayListNode>* next;

	/* Each Node has a lock and also has two flags:
	 * fullyLinked - true if the node is pointed by all its predecessors, and points to all it successors
	 * marked - true if the node was logically deleted
	 */
private:
	CCP::ReentrantLock _lock;
	volatile bool _fullyLinked;
	volatile bool _marked;
	int _height;

public:
	LazyLockSprayListNode(int value, int height);
	~LazyLockSprayListNode();

	inline int topLevel()
	{
		return _height;
	}

	inline void mark() {
		_marked = true;
	}

	inline bool isMarked() {
		return _marked;
	}

	inline bool isFullyLinked() {
		return _fullyLinked;
	}

	inline void setFullyLinked() {
		_fullyLinked = true;
	}

	inline void lock() {
		_lock.lock();
	}

	inline void unlock() {
		_lock.unlock();
	}
};


class LazyLockSparyListPriorityQueue : public IPriorityQueue {
private:
	int _maxAllowedHeight;
	CCP::AtomicInteger _threads;
	std::shared_ptr<LazyLockSprayListNode> _head;
	std::shared_ptr<LazyLockSprayListNode> _tail;

public:
	LazyLockSparyListPriorityQueue(int maxAllowedHeight);
	virtual ~LazyLockSparyListPriorityQueue();

private:
	/*This implementation of find is identical to the serial one, and it is wait-free */
	int find(int value, std::shared_ptr<LazyLockSprayListNode>* preds, std::shared_ptr<LazyLockSprayListNode>* succs);
	boolean remove(int value);

	/* spray finds a candidate for deleteMin */
	int spray(int H, int L, int D);

public:
	bool insert(int value);

	int deleteMin();

	bool isEmpty();
};

#endif /* LAZYLOCKSPRAYLISTPRIORITYQUEUE_H_ */
