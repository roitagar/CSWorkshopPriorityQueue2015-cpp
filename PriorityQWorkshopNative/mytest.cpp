#include <stdio.h>
#include "PriorityQueue/IPriorityQueue.h"
#include "PriorityQueue/NaiveLockSprayListPriorityQueue.h"

void printList(SprayListNode* head)
{
	SprayListNode* curr;
	for(int i=0;i<5;i++)
	{
		curr = head;
		printf("level %d: ", i);
		while(curr)
		{
			printf("%d->", curr->value);
			curr = curr->next[i];
		}
		printf("\n");
	}
}

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
	printList(q->head());

	// deletemin all
	while(!q->IsEmpty())
	{
		printf("Got %d\n", q->DeleteMin());
		printList(q->head());
	}

	delete q;
}
