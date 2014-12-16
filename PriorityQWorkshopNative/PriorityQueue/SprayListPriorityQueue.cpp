#include "SprayListPriorityQueue.h"
#include <climits>
#include <stdlib.h>
#include <cmath>

SprayListPriorityQueue::SprayListPriorityQueue(int maxAllowedHeight)
{
	_maxAllowedHeight = maxAllowedHeight;
	_head = new SprayListNode(INT_MIN, maxAllowedHeight);
	_tail = new SprayListNode(INT_MAX, maxAllowedHeight);
	for(int i=0;i<=_maxAllowedHeight;i++)
	{
		_head->next[i] = _tail;
		_tail->next[i] = NULL;
	}
}

SprayListPriorityQueue::~SprayListPriorityQueue()
{
	// TODO: Delete all middle nodes
	delete _head;
	delete _tail;
}

void SprayListPriorityQueue::Insert(int value)
{
	StartInsert();
	// long insert logic
	int topLevel = RandomLevel();
	SprayListNode* preds[_maxAllowedHeight+1];
	SprayListNode* succs[_maxAllowedHeight+1];
	bool success = false;

	while(!success)
	{
		int lFound = Find(value, preds, succs);

		//if (lfound!=-1) it means that the item is already exist
		//then return; TODO: decide whether to change to boolean/execption

		if(lFound != -1)
		{
			SprayListNode* nodeFound = succs[lFound];
			//if the item is marked due to deletion
			if(!nodeFound->IsMarked())
			{
				//wait for prev insertion to finish?
				//						while(!nodeFound.isFullyLinked()){}
				return;// false;
			}
			continue;
		}
		int highestLocked = -1;
		//		try
		{
			SprayListNode *pred, *succ;
			bool valid = true;
			// Try to connect the new element, bottom up
			for(int level = 0; valid && level<=topLevel; level++)
			{
				pred = preds[level];
				succ = succs[level];
				LockNode(pred); // note: not necessarily locks anything, implementation dependant
				highestLocked = level;
				// valid == false means a different item was inserted after the same pred, or pred/succ were removed
				valid = CanInsertBetween(pred, succ, level);
			}

			// TODO: convert the lock & unlock loops to a method of this kind:
			// valid = aquireAllLevels(preds, succs, level, out highestLocked);

			if(!valid)
			{
				// TODO: unlock nodes here? depends on 'finally' block
				for(int level = 0; level<=highestLocked; level++)
				{
					UnlockNode(preds[level]);
				}
				continue;
			}

			// At this point, all preds in all levels are exclusively locked for this insertion
			SprayListNode* newNode = new SprayListNode(value, topLevel);
			for(int level=0; level<=topLevel; level++)
			{
				newNode->next[level] = succs[level];
			}

			for(int level=0; level<=topLevel; level++)
			{
				preds[level]->next[level] = newNode;
			}

			newNode->SetFullyLinked(); // Successful add linearization point
			success = true;
		}
		//		finally
		//		{
		//			for(int level = 0; level<=highestLocked; level++)
		//			{
		//				unlockNode(preds[level]);
		//			}
		//		}
		for(int level = 0; level<=highestLocked; level++)
		{
			UnlockNode(preds[level]);
		}
	}

	EndInsert();
}

int SprayListPriorityQueue::Find(int value, SprayListNode** preds, SprayListNode** succs)
{
	int lFound = -1;
	SprayListNode* pred = _head;
	for(int level = _maxAllowedHeight;level>=0;level--)
	{
		SprayListNode* curr = pred->next[level];
		while(value > curr->value)
		{
			pred = curr;
			curr = pred->next[level];
		}
		if(lFound == -1 && value == curr->value) {
			lFound = level;
		}
		preds[level] = pred;
		succs[level] = curr;
	}
	return lFound;
}

int SprayListPriorityQueue::DeleteMin()
{
	StartDeleteMin();
	bool retry = false;
	int result;
	do
	{
		int p = GetNumberOfThreads();
		int H = (int) log(p)/*+K*/;
		int L = (int) (/*M * */ pow(log(p),3));
		int D = 1; /* Math.max(1, log(log(p))) */
		result = Spray(H,L,D);
		retry = !Remove(result);
	} while(retry && !IsEmpty());
	EndDeleteMin();
	return result;
}

bool SprayListPriorityQueue::Remove(int value)
{
	SprayListNode* victim = NULL;
	bool isMarked = false;
	int topLevel = -1;
	SprayListNode* preds[_maxAllowedHeight+1];
	SprayListNode* succs[_maxAllowedHeight+1];

	while(true)
	{
		int lFound = Find(value, preds, succs);
		if(lFound !=-1)
		{
			victim = succs[lFound];
		}

		// testing topLevel==lFound is not necessarily required, due to the otehr two checks
		if (isMarked ||  (lFound != -1 && ReadyToBeDeleted(victim)))// (victim.isFullyLinked()  && victim.topLevel() == lFound  && !victim.isMarked())))
		{
			if(!isMarked)
			{
				topLevel = victim->TopLevel();
				LockNode(victim);
				if(victim->IsMarked())
				{
					// Item was marked by another thread, and is being removed.
					UnlockNode(victim);
					return false;
				}
				victim->SetMark();
				isMarked = true;
			}

			int highestLocked = -1;
//			try
//			{
				SprayListNode *pred, *succ;
				bool valid = true;
				for (int level = 0; valid && (level <= topLevel); level++)
				{
					pred = preds[level];
					LockNode(pred);
					highestLocked = level;
					valid = !pred->IsMarked() && pred->next[level]==victim;
				}

				// valid == false if pred is pending deletion, or a new item was inserted after it
				if (!valid)
				{
					for (int i = 0; i <= highestLocked; i++)
					{
						UnlockNode(preds[i]);
					}
					continue;
				}

				for (int level = topLevel; level >= 0; level--)
				{
					preds[level]->next[level]=victim->next[level];
				}

				UnlockNode(victim);

				delete victim;

				for (int i = 0; i <= highestLocked; i++)
				{
					UnlockNode(preds[i]);
				}
				return true;
//			}
//			finally
//			{
//				for (int i = 0; i <= highestLocked; i++)
//				{
//					unlockNode(preds[i]);
//				}
//			}
		}
		else
		{
			// Item was either not found, or not ready for deletion (in the middle of insert/remove)
			return false;
		}
	}
}

int SprayListPriorityQueue::Spray(int H, int L, int D)
{
	SprayListNode* x = _head;
	int level = H;
	while(level>=0)
	{
		int j = RandomStep(L);
		for(;j>0 || x==_head;j--)
		{
			x = x->next[level];
		}
		level-=D;
	}

	return x->value;
}




bool SprayListPriorityQueue::IsEmpty() {
	return _head->next[0] == _tail;
}
