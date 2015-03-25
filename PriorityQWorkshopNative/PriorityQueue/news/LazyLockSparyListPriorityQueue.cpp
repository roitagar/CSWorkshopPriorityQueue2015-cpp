#include "LazyLockSparyListPriorityQueue.h"
#include <climits>
#include <cmath>
#include "serviceClass.h"


LazyLockSparyListPriorityQueue::LazyLockSparyListPriorityQueue(int maxAllowedHeight)
{
	_maxAllowedHeight = maxAllowedHeight;
	_head = std::shared_ptr<LazyLockSprayListNode>(new LazyLockSprayListNode(INT_MIN, maxAllowedHeight));
	_tail = std::shared_ptr<LazyLockSprayListNode>(new LazyLockSprayListNode(INT_MAX, maxAllowedHeight));
	for(int i=0;i<=_maxAllowedHeight;i++)
	{
		_head->next[i] = _tail;
	}
}

LazyLockSparyListPriorityQueue::~LazyLockSparyListPriorityQueue()
{
}

/*This implementation of find is identical to the serial one, and it is wait-free */
int LazyLockSparyListPriorityQueue::find(int value, std::shared_ptr<LazyLockSprayListNode>* preds, std::shared_ptr<LazyLockSprayListNode>* succs)
{
	int lFound = -1;
	std::shared_ptr<LazyLockSprayListNode> pred = _head;
	std::shared_ptr<LazyLockSprayListNode> curr;

	/* traverse each level and update preds and succss,
	 * if node was found - set lFound with the level it was initially found
	 * Note: if a node was found - the succs array cells,
	 * from lFound down to 0, should all point to it.
	 */
	for(int level = _maxAllowedHeight;level>=0;level--)
	{
		curr = pred->next[level];
		while(value > curr->value)
		{
			pred = curr;
			curr = pred->next[level];
		}
		if(lFound == -1 && value == curr->value) {
			lFound = level;
		}

		/*update the preds and succss arrays for the current level */
		preds[level] = pred;
		succs[level] = curr;
	}
	return lFound;
}

bool LazyLockSparyListPriorityQueue::insert(int value) {
	/* Increment the number of threads that are writing to the SprayList by 1,
	 * in order to use this number in DeleteMin
	 */
	_threads.incrementAndGet();

	bool done = false;

	int topLevel = serviceClass::randomLevel(_maxAllowedHeight);
	std::shared_ptr<LazyLockSprayListNode>* preds = new std::shared_ptr<LazyLockSprayListNode>[_maxAllowedHeight+1];
	std::shared_ptr<LazyLockSprayListNode>* succs = new std::shared_ptr<LazyLockSprayListNode>[_maxAllowedHeight+1];

	while(!done)
	{
		/* Find the location for the new item with predecessors and successors arrays
		 * if find returns an integer other then -1, it means that the key is already exist (physically!).
		 * In this implementation we also have to check if the key is logically exists(by checking node.isMark = false).
		 * If it also logically exist - wait until it will be fully linked (in a case when other thread is inserting the same key)
		 * and then return false - because we don't want to add it.
		 * otherwise - it is logically deleted - and it is about to be physically deleted, so try again.
		 * Note that if the node is not fully linked - it cannot be marked (see: Remove method),
		 * and therefore we will just wait until it will be fully linked and just after that we will return,
		 * in order to be consistent with the linearization point.
		 */
		int lFound = find(value, preds, succs);
		if(lFound != -1)
		{
			std::shared_ptr<LazyLockSprayListNode> nodeFound = succs[lFound];

			/*Check if the node is not deleted */
			if(!nodeFound->isMarked())
			{
				/* If node is in an insertion process - it is not fully linked
				 * wait until it will be fully linked
				 */
				while(!nodeFound->isFullyLinked()) {
					//do nothing
				}
				/*Linearization Point of unsuccessful insertion */
				delete[] preds;
				delete[] succs;
				_threads.decrementAndGet();
				return false;
			}
			/*If you arrived here, it means that node is logically deleted, but not yet physically,
			 * so continue with the loop and try again - maybe next time it will be deleted
			 */
			continue;
		}

		/* this variable will indicate the level I arrived while locking, in order to release all the lock until this level */
		int highestLocked = -1;

		std::shared_ptr<LazyLockSprayListNode> pred;
		std::shared_ptr<LazyLockSprayListNode> succ;
		bool valid = true;

		/* Try to acquire the locks of all the predecessor bottom up - in order to avoid deadlock
		 * (Remove behaves the same way when acquiring locks).
		 * Note that from the moment we found the appropriate location for insertion,
		 * and until we finished to lock all the predecessors,
		 * the structure can be changed in a way we have to start over:
		 * - Either one or more of the predecessors or the successors can be deleted,
		 * - Or one or more nodes were inserted between one or more of the predecessors and the successors.
		 * Therefore we must validate it was not occurred, and if it was - start over.
		 */
		for(int level = 0; valid && level<=topLevel; level++)
		{
			pred = preds[level];
			succ = succs[level];
			pred->lock();
			highestLocked = level;
			valid = !pred->isMarked() && !succ->isMarked() && (pred->next[level] == succ);
		}

		/* If one or more of the validation test failed - start over. */
		if(valid) {

			/* At this point, all predecessor in all levels are exclusively locked for the insertion,
			 * and we are ready to insert the new node:
			 * Create an new element and connect it, bottom up to its successors
			 */

			std::shared_ptr<LazyLockSprayListNode> newNode(new LazyLockSprayListNode(value, topLevel));

			for(int level=0; level<=topLevel; level++)
			{
				newNode->next[level] = succs[level];
			}

			for(int level=0; level<=topLevel; level++)
			{
				preds[level]->next[level] = newNode;
			}

			/* Now the node is fully linked - mark it as fully linked, and exit from the loop.
			 * Note: This is a linearization point - where the node was successfully added*/
			newNode->setFullyLinked();
			done = true;
		}
		//			finally
		//			{
		// TODO: Treat the finally thing
		/* Release all the predecessors locks *that I locked* (highestLocked) in order to not release someone's else lock*/
		for(int level = 0; level<=highestLocked; level++)
		{
			preds[level]->unlock();
		}
		//			}
	}

	delete[] preds;
	delete[] succs;
	_threads.decrementAndGet();
	return true;
}

bool LazyLockSparyListPriorityQueue::remove(int value)
{
	std::shared_ptr<LazyLockSprayListNode> victim;
	int topLevel = -1;
	bool done = false;
	bool result;

	std::shared_ptr<LazyLockSprayListNode>* preds = new std::shared_ptr<LazyLockSprayListNode>[_maxAllowedHeight+1];
	std::shared_ptr<LazyLockSprayListNode>* succs = new std::shared_ptr<LazyLockSprayListNode>[_maxAllowedHeight+1];

	/* This local variable isMarked is set to true
	 * when a thread succeed to lock a node and mark it as marked(i.e logically deleted).
	 * After isMarked to true - the node will eventually deleted by *this* thread
	 */
	bool isMarked = false;

	while(!done)
	{
		/* if node were found -assign succs[lFound]
		 * (which contains the node we want to remove) to victim */
		int lFound = find(value, preds, succs);
		if(lFound !=-1)
		{
			victim = succs[lFound];
		}

		/* Try to delete if (isMarked == true) - means the current thread has already started to delete this node,
		 * failed because of validation and now it retries OR:
		 *
		 * This is the first time thread tries do delete and all the following condition are satisfied:
		 * 1. A victim was found
		 * 2. The victim is fully linked - means that it is not being inserted now.
		 *    Note that a non fully linked node is not considered as exist - therefore it cannot be deleted
		 * 3. The victim's top level equal to lFound - means that the victim is not in a process of insertion
		 *    or other deletion.
		 * 4. The victim was not not logically removed and is not being removed by other thread
		 */
		if (isMarked ||  (lFound != -1 &&
				victim->isFullyLinked()  &&
				victim->topLevel() == lFound &&
				!victim->isMarked())) {

			/*Check if the node was already marked by me.
			 * If it wasn't, try to lock it and mark it, and then set the local isMarked to true.
			 */
			if(!isMarked)
			{
				topLevel = victim->topLevel();
				/* Grab a Lock on the victim, and check if it is already marked.
				 * If it is, release the lock and return, because victim was marked by another thread, and is being removed.
				 * If victim is not marked - mark it, and continue.
				 * In this case, victim will be release only after the current thread will delete it.
				 */
				victim->lock();

				if(victim->isMarked())
				{
					/*	This is a linearization point of unsuccessful remove.*/
					victim->unlock();
					delete [] preds;
					delete [] succs;
					return false;
				}

				/*This is a linearization point of successful remove */
				victim->mark();
				isMarked = true;
			}

			/*Now the victim is mine - locked and marked, and I have to remove it */

			/* this variable will indicate the level I arrived while locking, in order to release all the lock until this level */
			int highestLocked = -1;

			std::shared_ptr<LazyLockSprayListNode> pred;
			bool valid = true;

			/* Try to acquire the locks of all the predecessor bottom up - in order to avoid deadlock
			 * (Insert behaves the same way when acquiring locks).
			 * Note that from the moment we found the victim and its predecessors,
			 * and until we finished lock all the predecessors,
			 * the structure can changed in a way we have to start over:
			 * - Either one or more of the predecessors can be deleted,
			 * - Or one or more nodes were inserted between one or more of the predecessors and the victim.
			 * Therefore we must validate it was not occurred, and if it was - start over.
			 */
			for (int level = 0; valid && (level <= topLevel); level++)
			{
				pred = preds[level];
				pred->lock();
				highestLocked = level;
				valid = !pred->isMarked() && pred->next[level]==victim;
			}

			/* If one or more of the validation test failed - start over,
			 * but this time - isMarked = true*/
			if (valid)
			{

				/* At this point, all predecessor in all levels are exclusively locked,
				 * and we are ready to physically remove the victim.
				 */
				for (int level = topLevel; level >= 0; level--)
				{
					preds[level]->next[level] = victim->next[level];
				}

				/* Release the lock of the victim - and return true*/
				victim->unlock();
				done = true;
				result = true;
				//				finally
				//				{
				// TODO: Fix the finally thing
				/* Release all the predecessors locks *that I locked* (highestLocked) in order to not release someone's else lock*/
				for (int i = 0; i <= highestLocked; i++)
				{
					preds[i]->unlock();
				}
			}
			//				}
		}
		else
		{
			/* Node was either not found, or not ready for deletion */
			done = true;
			result = false;
		}
	}

	delete[] preds;
	delete[] succs;

	return result;
}

/* spray finds a candidate for deleteMin */
int LazyLockSparyListPriorityQueue::spray(int H, int L, int D)
{
	std::shared_ptr<LazyLockSprayListNode> x = _head;
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
		for(;(j>0 || x==_head) && x!=_tail && (x->next[level] != _tail || isEmpty());j--)
		{
			x = x->next[level];
		}
		level-=D;
	}

	return x->value;
}

//TODO: Do we really need the test here?
int LazyLockSparyListPriorityQueue::deleteMin() {
	_threads.incrementAndGet();
	bool retry = false;
	int result;

	do
	{
		int p = _threads.get();
		int H = (int) log(p)/*+K*/;
		int L = (int) (/*M * */ pow(log(p),3));
		int D = 1; /* Math.max(1, log(log(p))) */
		result = spray(H,L,D);
		if(result == INT_MAX)
		{
			// if we got tail's value, the list might be empty
			retry = !isEmpty();
		}
		else
		{
			retry = !remove(result);
			//(retry = true) means that another thread performed an action that affect the remove
		}
	} while(retry);
	_threads.decrementAndGet();
	return result;
}

bool LazyLockSparyListPriorityQueue::isEmpty() {
	return _head->next[0] == _tail;
}



LazyLockSprayListNode::LazyLockSprayListNode(int value, int height)
{
	this->value = value;
	_fullyLinked = false;
	_marked = false;
	next =  new std::shared_ptr<LazyLockSprayListNode>[height+1];
	_height = height;
}

LazyLockSprayListNode::~LazyLockSprayListNode()
{
	delete[] next;
}
