#include "CoolSprayListPriorityQueue.h"
#include <climits>
#include <cmath>

extern "C" void threadscan_collect (void *ptr);

CoolSprayListPriorityQueue::CoolSprayListPriorityQueue(int maxAllowedHeight, bool fair) :
	_maxAllowedHeight(maxAllowedHeight)
{
	_head = new CoolSprayListNode(INT_MIN, maxAllowedHeight);
	_tail = new CoolSprayListNode(INT_MAX, maxAllowedHeight);
	_elimArray = new NodesEliminationArray(0, _tail);
//	_lock1 = new CCP::ReentrantLock();
//	_lock2 = new ReentrantReadWriteLock(fair);
//	_lock3 = new ReentrantReadWriteLock(fair);
	highestNodeKey = INT_MIN; // null;
	for(int i=0;i<=_maxAllowedHeight;i++)
	{
//		_head.next[i] = new AtomicMarkableReference<CoolSprayListNode>(_tail, false);
//		_tail.next[i] = new AtomicMarkableReference<CoolSprayListNode>(null, false);
		_head->next[i].set(_tail, false);
		_tail->next[i].set(NULL, false);
	}
}

CoolSprayListPriorityQueue::~CoolSprayListPriorityQueue()
{
	// delete the list
	CoolSprayListNode* node;
	CoolSprayListNode* next = _head;

	do
	{
		node = next;
		next = node->next[0].getReference();
		delete node;
	} while (node != _tail);

	// or while not empty remove head.next?

	delete _elimArray;
}

/* find can find also marked nodes */
NodeStatus CoolSprayListPriorityQueue::find(int value, CoolSprayListNode** preds, CoolSprayListNode** succs)
{
	CoolSprayListNode* pred =_head;
	CoolSprayListNode* curr = NULL;
	/* Traverse each level up do bottom */
	for (int level = _maxAllowedHeight; level >= 0; level--) {
		curr = pred->next[level].getReference();

		while (curr->value  < value){
			pred = curr;
			curr = pred->next[level].getReference();

		}
		/* update preds and succss */
		preds[level] = pred;
		succs[level] = curr;
	}
	if (curr->value == value) {
		if (curr->isDeleted()){
			return NodeStatus::DELETED;
		}
		return NodeStatus::FOUND;
	}
	return NodeStatus::NOT_FOUND;
}

bool CoolSprayListPriorityQueue::insert(int value) {
	_threads.getAndIncrement();
	int topLevel = serviceClass::randomLevel(_maxAllowedHeight);
	bool shouldReleaseLock3 = false;
	bool reinsert = false; // determine insertion phase
	CoolSprayListNode* preds[_maxAllowedHeight+1];
	CoolSprayListNode* succs[_maxAllowedHeight+1];

	bool result;

	// Don't interfere with deciding a delete-group
	_lock2.readerLock();
//	try {
		/*in this case we have to wait */
		int temp = highestNodeKey; // local copy to avoid a race condition with assignment of null value
		if (/*temp != NULL && */value <temp){
			shouldReleaseLock3 = true;
			// Don't interfere with disconnecting and building a delete-group
			_lock3.readerLock();

			// TODO: maybe instead of waiting, join the elimination array being built right now?
			//		 not sure it's a good idea, since it requires complex synchronization with the cleaner thread
		}

		/* create a new node */
		CoolSprayListNode* newNode = new CoolSprayListNode(value, topLevel);

		// Insertion might have two phases:
		// 		1. insert the requested value
		//		2. reinsert an item from the elimination array back to the skiplist
		//
		// if there are elimination items smaller than my value,
		// I need to eliminate them back to the list, to preserve linearizability
		//
		// every insert thread reinserts at most one item. the elimination array is depleted only if enough insert operations
		// are performed before deleteMin operations, otherwise deleteMin operations will deplete the elimination array before
		// linearizability is harmed

		while(true)
		{
			if (!reinsert && _elimArray->contains(value))
			{
				// Node exists, and is pending deletion in the elimination array
				result = false;
				delete newNode; // The new allocated node will not be used
				break;
			}

			NodeStatus status = find(value, preds, succs);
			if(!reinsert && status == NodeStatus::FOUND) {
				/* linearization point of unsuccessful insertion */
				result = false;
				delete newNode; // The new allocated node will not be used
				break;
			}

			if (!reinsert && status == NodeStatus::DELETED) {
				/* Node physically exists, and only logically deleted - unmark it */
				bool IRevivedIt = succs[0]->revive();

				if(IRevivedIt)
				{
					logInsertion(true);
				}

				result = IRevivedIt;
				delete newNode; // The new allocated node will not be used
				break;
			}

			if (reinsert && newNode->isDeleted())
			{
				// some deleteMin successfully eliminated this node, no need to reinsert it
				// original insertion was successful
				result = true;
				// TODO: verify collect responsibility?
				break;
			}

			/* The item is not in the set (both physically and logically - so add it */

			/* connect level 0 of newNode to its successor */
			newNode->next[0].set(succs[0], false);
			CoolSprayListNode* pred = preds[0];
			CoolSprayListNode* succ = succs[0];

			/* try to connect the bottom level - this is a linearization point of successful insertion*/
			if(!pred->next[0].compareAndSet(succ, newNode, false, false)) {
				continue;
			}

			if(reinsert)
			{
				logReinsert();
			}
			else
			{
				logInsertion(false);
			}

			/* now when level 0 is connected - connect the other predecessors from the other levels to the new node.
			 * If you fail to connect a specific level - find again - means - prepare new arrays of preds and succs,
			 * and continue from the level you failed.
			 * If you failed because of the mark - that means you try to insert the node next to the last node in the
			 * deletion list - and it will work just after this node will be removed.
			 */
			for (int level= 1; level <= topLevel; level++) {
				while (true) {
					pred = preds[level];
					succ = succs[level];

					// connect the new node to the next successor
					// Note: This action is repeated until connecting pred to newNode succeeds, unlike in the lock-free implementation
					//		 Although it seems to increase accesses due to retries, this implementation heavily reduces retries,
					//		 due to grouped removals, thus allowing us to maintain better correctness of the skiplist structure without
					//		 harming performance.
					newNode->next[level].set(succ, false);

					if (pred->next[level].compareAndSet(succ, newNode, false, false)){
						break;
					}
					find(value, preds, succs);
				}
			}
			if (reinsert) {
				// change node status from ALIVE_IN_ELIMINATION to ALIVE
				//linearization point for reinsert
				newNode->reinsert();
			}
			else if(false) /* TODO: reinsert is DISABLED due to incompatibility with threadscan*/
			{
				// successful insertion completed, now check if we need to help fix linearization by
				// reinserting a high-valued node from the elimination array to the skiplist
				newNode = _elimArray->getNodeForReinsert(value);
				reinsert = (newNode != null);
				if(reinsert)
				{
					value = newNode->value;
					topLevel = newNode->topLevel();
					// repeat insertion with the reinsert node
					continue;
				}
			}

			// success
			result = true;
			break;
		}

//	}
//	finally {s
		if(shouldReleaseLock3)
		{
			_lock3.readerUnlock();
		}
		_lock2.readerUnlock();

		_threads.getAndDecrement();
//	}

	return result;
}

bool CoolSprayListPriorityQueue::clean() {

	// Allow only a single cleaner, but don't block
	if (!_lock1.tryLock())
	{
		return false;
	}
	int foundHealthyNodes;
	int maxLevelFound;
	int len;
	int actualLen;
	CoolSprayListNode* firstNode;
	CoolSprayListNode* curr;
	CoolSprayListNode* highest;
	NodesEliminationArray* newElimArray;
	NodesEliminationArray* tmpElimArray;
//	try {

		// Coherence test:
		if(_elimArray->hasNodes())
		{
			_lock1.unlock();
			// Someone else performed cleanup and I missed it, go back to empty the elimination array
			return false;
		}

		// Block inserters
		_lock2.writerLock();
		_lock3.writerLock();
//		try { //for _lock3
//			try{ //for _lock2
				/* Determine the max number of Healthy element you want to traverse */
				int p = _threads.get();
				p = p*(int)(log(p)/log(2)) + 1;
				int numOfHealtyNodes = std::min(p, MAX_ELIMINATION_ARRAY);
				/* Create an Elimination Array in this size */
				newElimArray = new NodesEliminationArray(numOfHealtyNodes, _tail);

				/* Traverse the list in the bottom level look for healthy element, and find an optimal group */
				foundHealthyNodes = 0;
				maxLevelFound = 0;
				len = 0;
				actualLen = 0;
				firstNode = _head->next[0].getReference();
				curr = firstNode;
				highest = curr;
				while (foundHealthyNodes < numOfHealtyNodes && curr != _tail) {
					len++;
					if (!curr->isDeleted()) {
						foundHealthyNodes++;
					}
					/* find the last highest node in the ragne */
					if (maxLevelFound <= curr->topLevel()) {
						highest = curr;
						maxLevelFound = curr->topLevel();
						actualLen = len;

						// TODO: Compare live-dead element ratios?
					}
					curr = curr->next[0].getReference();
				}

				highestNodeKey = highest->value;

				if(firstNode == _tail)
				{
					// No nodes to remove
					highestNodeKey = INT_MIN; // null;
					_lock2.writerUnlock();
					_lock3.writerUnlock();
					_lock1.unlock();
					return false;
				}
//			}
//			finally {
//				_lock2.writeLock().unlock(); // high-valued inserts can go on
//			}

			_lock2.writerUnlock();


			// Now you have a range that you want to delete. mark the highest node's markable reference in all levels,
			// so other threads cannot add a node after it.
			// Starting the marking process from the bottom, blocks new inserts from interrupting.
			for (int level=0; level <= highest->topLevel(); level++) {
				while (true) {
					CoolSprayListNode* succ = highest->next[level].getReference();
					if (highest->next[level].attemptMark(succ, true)) {
						break;
					}
				}
			}

			/* Now - nobody can connect a node after the highest node - in the deletion list - connect the head*/
			for (int level=0; level <= highest->topLevel(); level++) {
				_head->next[level].set(highest->next[level].getReference(), false);
			}
			/* Now  - mark each alive node in the group as belong to elimination array and add it to the elimination array */
			curr = firstNode;
			bool done = false;
			while (!done){
				if(curr == highest) // last node to process
					done = true;

				// clean next references, to prevent accessing collected nodes
				CoolSprayListNode* next = curr->next[0].getReference();

				for(int level=curr->topLevel(); level>=0; level--)
				{
					curr->next[level].set(_tail, false);
				}

				if (!curr->isDeleted()) {
					// Try to mark it as node of the eliminataion array.
					if (curr->markAsEliminationNode()) {
						newElimArray->addNode(curr);
					}
				}

				// if the node is deleted, it did not enter the elimination array, and can be safely collected
				// not using "else" here to avoid a race between the "if" and the CAS (markAsEliminationNode)
				if(curr->isDeleted())
				{
					// TODO: collect
					threadscan_collect(curr);
				}

				curr = next;
			}

			logCleanup(actualLen);

			// Spin until ongoing eliminations are done
			while(!_elimArray->completed()) { }

			tmpElimArray = _elimArray;
			// publish the ready elimination array
			_elimArray = newElimArray;

//			delete tmpElimArray; // TODO: collect
			threadscan_collect(tmpElimArray);

			highestNodeKey = INT_MIN; // null;
//		}
//		finally {
			// now after the elimination array is ready, also the lower inserters can go.
			_lock3.writerUnlock();
//		}
//	}
//	finally{

		_lock1.unlock();
//	}

	return true;
}

/* This remove is wait-free and only logically removes the item */
bool CoolSprayListPriorityQueue::remove(int value) {
	CoolSprayListNode* preds[_maxAllowedHeight+1];
	CoolSprayListNode* succs[_maxAllowedHeight+1];
	NodeStatus status = find(value, preds, succs);
	if (status == NodeStatus::NOT_FOUND || status == NodeStatus::DELETED) {
		return false;
	}

	/*The node exists in the set */

	else {
		/*remove it logically and check if it was marked before*/
		CoolSprayListNode* nodeToRemove = succs[0];
		bool iMarkedIt = nodeToRemove->logicallyDeleteFromList();
		return iMarkedIt;
	}
}

// Finds a candidate for deleteMin
int CoolSprayListPriorityQueue::spray(int H, int L, int D)
{
	CoolSprayListNode* x = _head;
	int level = H;
	while(level>=0)
	{
		int j = serviceClass::randomStep(L);
		/*
		 * Don't stay on head
		 * Don't advance beyond tail
		 * Usually don't advance to tail
		 * Advance to tail when list is empty
		 */
		for(;(j>0 || x==_head) && x!=_tail && (x->next[level].getReference() != _tail || isSkipListEmpty());j--)
		{
			x = x->next[level].getReference();

			// TODO: skip longer over deleted nodes?
		}
		level-=D;
	}

	return x->value;
}

int CoolSprayListPriorityQueue::deleteMin() {
	_threads.getAndIncrement();

	int result;

	while(true)
	{
		// First try to eliminate
		if(_elimArray->hasNodes())
		{
			CoolSprayListNode* node = _elimArray->getNode();
			if(node != NULL)
			{
				// Successful elimination
				result = node->value;
				// TODO: collect here or inside?
				// TODO: is it correct to do it here?
				threadscan_collect(node);
				break;
			}
		}

		// Attempt cleanup
		int tmp = serviceClass::randomStep(100);
		if(tmp < 3)
		{
			if(clean())
			{
				// Successful cleanup - now try to eliminate an item
				continue;
			}
		}

		// Normal spray
		int p = _threads.get();
		int K = 0; // TODO: K=2?
		int H =  std::min((int) (log(p)/log(2))+K, _maxAllowedHeight);
		int L = (int) (pow((log(p)/log(2)),3));
		int D = 1; /* Math.max(1, log(log(p))) */
		result = spray(H,L,D);

		if(result == INT_MAX)
		{
			// if we got tail's value, the list might be empty
			if(isEmpty())
				break;
		}
		else if(remove(result))
		{
			// Successful spray+remove
			break;
		}
	}

	_threads.getAndDecrement();

	if(result != INT_MAX)
	{
		logRemoval();
	}

	return result;
}

//bool CoolSprayListPriorityQueue::isEmpty() {
//	return _liveItems.get() == 0;
//}

/**
 * for internal use - due to items in elimination array, the list might be empty
 * even when the queue is not empty
 */
//bool CoolSprayListPriorityQueue::isSkipListEmpty()
//{
//	return _itemsInSkipList.get() == 0;
//}

///**
// * inform a successful insertion
// */
//protected void logInsertion(boolean revive)
//{
//
//	_liveItems.getAndIncrement();
//	if(!revive)
//	{
//		_itemsInSkipList.getAndIncrement();
//	}
//}
//
//protected void logReinsert()
//{
//	_itemsInSkipList.getAndIncrement();
//}
//
///**
// * inform a successful logical removal
// */
//protected void logRemoval()
//{
//	_liveItems.getAndDecrement();
//}
//
///**
// * inform a successful batch cleanup
// */
//protected void logCleanup(int amount)
//{
//	_itemsInSkipList.getAndAdd(-amount);
//}

//CoolSprayListNode::CoolSprayListNode(int value, int height) {
//	this.value = value;
//	this.next =  (AtomicMarkableReference<CoolSprayListNode>[]) new AtomicMarkableReference[height+1];
//	for (int i = 0; i < next.length; i++) {
//		next[i] = new AtomicMarkableReference<CoolSprayListNode>(null,false);
//	}
//
//	_status = new AtomicInteger(ALIVE);
//}
//
//public int topLevel()
//{
//	return next.length-1;
//}
//
///* try to mark and return if succeed */
//public boolean markAsEliminationNode() {
//	return _status.compareAndSet(ALIVE, ALIVE_IN_ELIMINATION);
//}
//
//public boolean logicallyDeleteFromList() {
//	return _status.compareAndSet(ALIVE, DELETED);
//}
//
///* try to revive the node and return true if succeed */
//public boolean revive() {
//	return (_status.compareAndSet(DELETED, ALIVE) || _status.compareAndSet(ELIMINATED, ALIVE));
//}
//
//public boolean reinsert() {
//	return _status.compareAndSet(ALIVE_IN_ELIMINATION, ALIVE);
//}
//
//public boolean eliminate() {
//	return _status.compareAndSet(ALIVE_IN_ELIMINATION, ELIMINATED);
//}
//
//public boolean isDeleted() {
//	int status = _status.get();
//	return status == DELETED || status == ELIMINATED;
//}
//}

//protected class NodesEliminationArray {
//public NodesEliminationArray(int size) {
//	arr = new CoolSprayListNode[size];
//	deleteMinCounter = new AtomicInteger(0);
//	reInsertCounter = new AtomicInteger(0);
//	pendingCompletion = new AtomicInteger(0);
//
//}
//public void addNode(CoolSprayListNode node) {
//	int i = deleteMinCounter.getAndIncrement();
//	arr[i] = node;
//	pendingCompletion.getAndIncrement();
//	reInsertCounter.getAndIncrement();
//	numOfNodes++;
//}
//
////Traverse the array from lowest to highest
//public CoolSprayListNode getNode() {
//	// get token
//	int i = numOfNodes -  deleteMinCounter.getAndDecrement();
//
//	if (i >= numOfNodes) {
//		// nothing left to do here
//		return null;
//	}
//
//	// get value
//	CoolSprayListNode result = arr[i];
//
//	// inform completion, "release" the array
//	pendingCompletion.getAndDecrement();
//
//	//Try to mark the item as deleted - means that no re-insertion was done (or got the linearization point)
//	if (result.eliminate()) {
//		// successful elimination
//		return result;
//	}
//	else {
//		//otherwise - someone else (re-insertion) succeeded to insert and mark it as ready so try the next node.
//		return getNode();
//	}
//}
//
////Traverse the array from highest to lowest
//public CoolSprayListNode getNodeForReinsert(int insertedValue) {
//	int i = reInsertCounter.get() - 1; // speculated reinsert token for value test
//	int nextEliminatedIndex = numOfNodes - deleteMinCounter.get();
//	if(i < nextEliminatedIndex || arr[i].value < insertedValue)
//	{
//		// no relevant reinsert node exists
//		return null;
//	}
//
//	// get token
//	i = reInsertCounter.decrementAndGet();
//	if(i < 0)
//	{
//		return null;
//	}
//
//	// Now get value and check if node is still valid for reinsert
//	CoolSprayListNode result = arr[i];
//	if (result.isDeleted()) {
//		// the node was already deleted by some deleteMin
//		// no need to retry - lower nodes were also already eliminated
//		return null;
//	}
//
//	//return result
//	return result;
//}
//
//public boolean hasNodes(){
//	return deleteMinCounter.get() > 0;
//}
//
//public boolean completed()
//{
//	return pendingCompletion.get() == 0;
//}
//
//public boolean contains(int value)
//{
//	// go over values that were not removed yet
//	for(int i=1; i<= deleteMinCounter.get(); i++)
//	{
//		// if value found and not deleted (linearization point) return true
//		if(arr[numOfNodes - i].value == value && !arr[numOfNodes - i].isDeleted())
//		{
//			return true;
//		}
//	}
//
//	return false;
//}
//}
//
//private enum NodeStatus {
//	FOUND,
//	DELETED,
//	NOT_FOUND
//}
//
//}
