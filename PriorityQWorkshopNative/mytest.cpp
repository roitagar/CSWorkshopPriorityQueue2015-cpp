#include <stdio.h>
#include "PriorityQueue/IPriorityQueue.h"
#include "PriorityQueue/NaiveLockSprayListPriorityQueue.h"

int main()
{
//	IPriorityQueue* q;
	NaiveLockSprayListPriorityQueue* q;
	printf("Hi there\n");
	q = new NaiveLockSprayListPriorityQueue(5);
	printf("Hi there\n");
	q->Insert(5);
	printf("Hi there\n");
	q->Insert(3);
	printf("Hi there\n");
	q->Insert(18);
	q->Insert(2);
	q->Insert(105);
	q->Insert(45);
	printf("Hi there\n");

	// print the skiplist:
	SprayListNode* curr = q->head();
	while(curr)
	{
		printf("Node %d\n", curr->value);
		curr = curr->next[0];
	}
	// deletemin all
	while(!q->IsEmpty())
	{
		printf("Got %d\n", q->DeleteMin());
	}
}
