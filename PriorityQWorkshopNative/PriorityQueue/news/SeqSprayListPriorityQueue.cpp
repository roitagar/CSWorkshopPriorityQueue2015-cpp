#include <climits>
#include <stdlib.h>
#include <cmath>
#include "SeqSprayListPriorityQueue.h"
#include "serviceClass.h"

SeqSprayListNode::SeqSprayListNode(int value, int height)
{
	this->value = value;
	next =  new SeqSprayListNode*[height+1];
	_height = height;
}

SeqSprayListNode::~SeqSprayListNode()
{
	delete [] next;
}

int SeqSprayListNode::topLevel()
{
	return _height;
}

SeqSprayListPriorityQueue::SeqSprayListPriorityQueue(int maxAllowedHeight)
{
	_maxAllowedHeight = maxAllowedHeight;
	_head = new SeqSprayListNode(INT_MIN, maxAllowedHeight);
	_tail = new SeqSprayListNode(INT_MAX, maxAllowedHeight);
	/* connect the head sentinel to the tail sentinel */
	for(int i=0;i<=_maxAllowedHeight;i++)
	{
		_head->next[i] = _tail;
	}
}

SeqSprayListPriorityQueue::~SeqSprayListPriorityQueue()
{
	// delete the list
	SeqSprayListNode* node;
	SeqSprayListNode* next = _head;

	do
	{
		node = next;
		next = node->next[0];
		delete node;
	} while (node != _tail);

	// or while not empty remove head.next?
}

int SeqSprayListPriorityQueue::find(int value, SeqSprayListNode** preds, SeqSprayListNode** succs)
{
	int lFound = -1;
	SeqSprayListNode* pred = _head;

	/* traverse each level and update preds and succss,
	 * if node was found - set lFound with the level it was initially found
	 * Note: if a node was found - the succs array cells,
	 * from lFound down to 0, should all point to it.
	 * !
	 */
	for(int level = _maxAllowedHeight;level>=0;level--)
	{
		SeqSprayListNode* curr = pred->next[level];
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
	
	

bool SeqSprayListPriorityQueue::insert(int value)
{
	int topLevel = _service.randomLevel(_maxAllowedHeight);
	bool result;

	SeqSprayListNode** preds = new SeqSprayListNode*[_maxAllowedHeight+1];
	SeqSprayListNode** succs = new SeqSprayListNode*[_maxAllowedHeight+1];

	/* find the location for the new item with preds and succss arrays
	 * if find returns an integer other then -1, it means that
	 * the key is already exist - and we don't want to insert it again
	 */
	int lFound = find(value, preds, succs);
	if(lFound != -1)
	{
		result = false;
	}
	else
	{
		/*Item is not on the list - so insert it */

		/*Create an new element and connect it, bottom up to its successors */
		SeqSprayListNode* newNode = new SeqSprayListNode(value, topLevel);

		for(int level=0; level<=topLevel; level++)
		{
			newNode->next[level] = succs[level];
		}

		/*Connect the new item's predecessors to it */
		for(int level=0; level<=topLevel; level++)
		{
			preds[level]->next[level] = newNode;
		}

		result = true;
	}

	delete [] preds;
	delete [] succs;

	return result;
}
	
	
boolean SeqSprayListPriorityQueue::remove(int value)
{
	SeqSprayListNode* victim = NULL;
	int topLevel = -1;
	bool result;
	SeqSprayListNode** preds = new SeqSprayListNode*[_maxAllowedHeight+1];
	SeqSprayListNode** succs = new SeqSprayListNode*[_maxAllowedHeight+1];

	int lFound = find(value, preds, succs);

	/* if node were found -assign succs[lFound]
	 * (which contains the node we want to remove) to victim and remove it. */
	if(lFound !=-1)
	{
		victim = succs[lFound];
		topLevel = victim->topLevel();

		if(victim == _tail)
		{
			// Don't remove tail
			// We got here because the list was empty during deleteMin
			// this lets deleteMin retry only if the list is not empty
			return isEmpty();
		}

		for (int level = topLevel; level >= 0; level--)
		{
			preds[level]->next[level] = victim->next[level];
		}

		result = true;
	}

	/*Item was not found*/
	else
	{
		result = false;
	}

	delete [] preds;
	delete [] succs;

	return result;
}

// Finds a candidate for deleteMin
int SeqSprayListPriorityQueue::spray(int H, int L, int D)
{
	SeqSprayListNode* x = _head;
	int level = H;
	while(level>=0)
	{
		int j = _service.randomStep(L);
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
	
/**
 * Returns the number of active threads. Used for spray height.
 * concurrent implementations should override this for correctness.
 */
int SeqSprayListPriorityQueue::getThreadCount() {
	return 1; // There is always one active thread each time
}
	
int SeqSprayListPriorityQueue::deleteMin() {
	bool retry = false;
	int result;
	do
	{
		int p = getThreadCount();
		int H = (int) log(p)/*+K*/;
		int L = (int) (/*M * */ pow(log(p),3));
		int D = 1; /* Math.max(1, log(log(p))) */
		result = spray(H,L,D);
		retry = !remove(result);
	} while(retry);
	return result;
}

boolean SeqSprayListPriorityQueue::isEmpty() {
	return _head->next[0] == _tail;
}

