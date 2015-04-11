#pragma once

#include "IPriorityQueue.h"
#include "../../framework/cpp_framework.h"
#include "serviceClass.h"

	class CoolSprayListNode {
	public:
		int value;
		/*The mark in the AtomicMarkableReference is used to the phyiscal deleteion of the last node in the deletion group */ 
		CCP::AtomicMarkableReference<CoolSprayListNode>* next;
	private:
		CCP::AtomicInteger _status;
		const int _height;

		// Status codes:
	private:
		static const int ALIVE = 0; // alive in the skiplist
		static const int DELETED = 1; // logically deleted in the skiplist
		static const int ALIVE_IN_ELIMINATION = 2; // alive in the elimination array (but not alive in the skiplist)
		static const int ELIMINATED = 3; // logically deleted in the elimination array

	public:
		CoolSprayListNode(int value, int height) :
			_height(height)
		{
			this->value = value;
			this->next =  new CCP::AtomicMarkableReference<CoolSprayListNode>[height+1];
//			for (int i = 0; i < next.length; i++) {
//				next[i] = new AtomicMarkableReference<CoolSprayListNode>(null,false);
//			}

//			_status = new AtomicInteger(ALIVE);
		}

		~CoolSprayListNode()
		{
			delete[] this->next;
		}

		inline int topLevel()
		{
			return _height;
		}

		/* try to mark and return if succeed */
		bool markAsEliminationNode() {
			return _status.compareAndSet(ALIVE, ALIVE_IN_ELIMINATION);
		}

		bool logicallyDeleteFromList() {
			return _status.compareAndSet(ALIVE, DELETED);
		}

		/* try to revive the node and return true if succeed */
		bool revive() {
			return (_status.compareAndSet(DELETED, ALIVE) || _status.compareAndSet(ELIMINATED, ALIVE));
		}

		bool reinsert() {
			return _status.compareAndSet(ALIVE_IN_ELIMINATION, ALIVE);
		}

		bool eliminate() {
			return _status.compareAndSet(ALIVE_IN_ELIMINATION, ELIMINATED);
		}

		bool isDeleted() {
			int status = _status.get();
			return status == DELETED || status == ELIMINATED;
		}
	};

	class NodesEliminationArray {
	private:
		CoolSprayListNode** arr;
		CCP::AtomicInteger deleteMinCounter; // token allocator
		CCP::AtomicInteger reInsertCounter; // token allocator
		CCP::AtomicInteger pendingCompletion; // prevents overriding before array access is done
		int numOfNodes; //number of initial total nodes after all insertions
	public:
		NodesEliminationArray(int size) {
			arr = new CoolSprayListNode*[size];
			numOfNodes = 0;

		}

		~NodesEliminationArray()
		{
			delete[] arr;
		}

		void addNode(CoolSprayListNode* node) {
			int i = deleteMinCounter.getAndIncrement();
			arr[i] = node;
			pendingCompletion.getAndIncrement();
			reInsertCounter.getAndIncrement();
			numOfNodes++;
		}

		//Traverse the array from lowest to highest
		CoolSprayListNode* getNode() {
			// get token
			int i = numOfNodes -  deleteMinCounter.getAndDecrement();

			if (i >= numOfNodes) {
				// nothing left to do here
				return NULL;
			}

			// get value
			CoolSprayListNode* result = arr[i];

			// inform completion, "release" the array
			pendingCompletion.getAndDecrement();

			//Try to mark the item as deleted - means that no re-insertion was done (or got the linearization point)
			if (result->eliminate()) {
				// successful elimination
				return result;
			}
			else {
				//otherwise - someone else (re-insertion) succeeded to insert and mark it as ready so try the next node.
				return NULL;//getNode();
			}
		}

		//Traverse the array from highest to lowest
		CoolSprayListNode* getNodeForReinsert(int insertedValue) {
			int i = reInsertCounter.get() - 1; // speculated reinsert token for value test
			int nextEliminatedIndex = numOfNodes - deleteMinCounter.get(); 
			if(i < nextEliminatedIndex || arr[i]->value < insertedValue)
			{
				// no relevant reinsert node exists
				return null;
			}

			// get token
			i = reInsertCounter.decrementAndGet();
			if(i < 0)
			{
				return null;
			}

			// Now get value and check if node is still valid for reinsert
			CoolSprayListNode* result = arr[i];
			if (result->isDeleted()) {
				// the node was already deleted by some deleteMin
				// no need to retry - lower nodes were also already eliminated
				return null;
			}

			//return result
			return result;
		}

		bool hasNodes(){
			return deleteMinCounter.get() > 0;
		}

		bool completed()
		{
			return pendingCompletion.get() == 0;
		}

		bool contains(int value)
		{
			// go over values that were not removed yet
			for(int i=1; i<= deleteMinCounter.get(); i++)
			{
				// if value found and not deleted (linearization point) return true
				if(arr[numOfNodes - i]->value == value && !arr[numOfNodes - i]->isDeleted())
				{
					return true;
				}
			}

			return false;
		}
	};

	enum NodeStatus {
		FOUND,
		DELETED,
		NOT_FOUND
	};


class CoolSprayListPriorityQueue : public IPriorityQueue {

protected:
	const int _maxAllowedHeight;
	CCP::AtomicInteger _threads;
	CCP::AtomicInteger _liveItems;
	CCP::AtomicInteger _itemsInSkipList;
	CoolSprayListNode* _head;
	CoolSprayListNode* _tail;
	/*volatile*/ NodesEliminationArray* _elimArray; // TODO: is volatile required?
	CCP::ReentrantLock _lock1; // during the entire cleanup - allows only one cleaner
	CCP::ReentrantReadWriteLock _lock2; // during delete-group selection - blocks all inserters
	CCP::ReentrantReadWriteLock _lock3; // during delete-group disconnection and construction - blocks low inserters
//	volatile CCP::Integer highestNodeKey;
	/*volatile*/ int highestNodeKey; // TODO: volatile?

public:
	CoolSprayListPriorityQueue(int maxAllowedHeight, bool fair);
	virtual ~CoolSprayListPriorityQueue();

	/* find can find also marked nodes */
protected:
	NodeStatus find(int value, CoolSprayListNode** preds, CoolSprayListNode** succs);

public:
	virtual bool insert(int value);

protected:
	virtual bool clean();

	/* This remove is wait-free and only logically removes the item */
	bool remove(int value);

	// Finds a candidate for deleteMin
	int spray(int H, int L, int D);

public:
	virtual int deleteMin();

	virtual bool isEmpty() {
			return _liveItems.get() == 0;
			}

	/**
	 * for internal use - due to items in elimination array, the list might be empty
	 * even when the queue is not empty
	 */
protected:
	inline bool isSkipListEmpty()
	{
		return _itemsInSkipList.get() == 0;
	}

	/**
	 * inform a successful insertion
	 */
	inline void logInsertion(bool revive)
	{

		_liveItems.getAndIncrement();
		if(!revive)
		{
			_itemsInSkipList.getAndIncrement();
		}
	}

	inline void logReinsert()
	{
		_itemsInSkipList.getAndIncrement();
	}

	/**
	 * inform a successful logical removal
	 */
	inline void logRemoval()
	{
		_liveItems.getAndDecrement();
	}

	/**
	 * inform a successful batch cleanup
	 */
	inline void logCleanup(int amount)
	{
		_itemsInSkipList.getAndAdd(-amount);
	}
};
