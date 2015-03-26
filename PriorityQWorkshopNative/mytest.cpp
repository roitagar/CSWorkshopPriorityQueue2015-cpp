#include <stdio.h>
#include "PriorityQueue/IPriorityQueue.h"
#include "PriorityQueue/news/IPriorityQueue.h"
#include "PriorityQueue/news/GlobalLockSprayListPriorityQueue.h"
#include "PriorityQueue/news/LazyLockSparyListPriorityQueue.h"
#include "PriorityQueue/NaiveLockSprayListPriorityQueue.h"
#include "PriorityQueue/GrainedLockSprayListPriorityQueue.h"
#include "Test/Workers.h"
#include "Test/StopWatch.h"
#include "Test/TestBench.h"
#include "Test/TestUtils.h"
#include <string>

#define TOP_LEVEL 10

//void printList(SprayListNode* head)
//{
//	SprayListNode* curr;
//	for(int i=0;i<TOP_LEVEL;i++)
//	{
//		curr = head;
//		printf("level %d: ", i);
//		while(curr)
//		{
//			printf("%d->", curr->value);
//			curr = curr->next[i];
//		}
//		printf("\n");
//	}
//}

//void printForHistogram(SprayListNode* head)
//{
//	SprayListNode* curr = head;
//	while(curr)
//	{
//		printf("%d\t%d\n", curr->value, curr->TopLevel());
//		curr = curr->next[0];
//	}
//}

void testBench3(IPriorityQueue* queue);

int main()
{
//	//IPriorityQueue* q;
//	SprayListPriorityQueue* q;
//	printf("Hi there\n");
////	q = new NaiveLockSprayListPriorityQueue(TOP_LEVEL);
//	q = new GrainedLockSprayListPriorityQueue(TOP_LEVEL);
//	printf("Hi there\n");
//	q->Insert(5);
//	printf("Hi there\n");
//	q->Insert(3);
//	printf("Hi there\n");
//	q->Insert(18);
//	q->Insert(2);
//	q->Insert(105);
//	q->Insert(45);
//	printf("Hi there\n");
//
//	// print the skiplist:
//	printList(q->head());
//
//	// deletemin all
//	while(!q->IsEmpty())
//	{
//		printf("Got %d\n", q->DeleteMin());
//		printList(q->head());
//	}
//
//	delete q;

//	IPriorityQueue* pq = new GrainedLockSprayListPriorityQueue(TOP_LEVEL);
//	SprayListPriorityQueue* pq = new GrainedLockSprayListPriorityQueue(TOP_LEVEL);
//	SprayListPriorityQueue* pq = new NaiveLockSprayListPriorityQueue(TOP_LEVEL);
//	IPriorityQueue* pq = new GlobalLockSprayListPriorityQueue(TOP_LEVEL);
	IPriorityQueue* pq = new LazyLockSparyListPriorityQueue(TOP_LEVEL);
//	testBench3(pq);

	delete pq;

	std::string res[3];
	res[0] = "ba";
	res[1] = "ga";
	res[2] = "da";

	saveResult(res, 3);

	printf("Done!\n");
//	InsertWorker* w = new InsertWorker(pq, 5,100);
//
//	w->start();
//	w->join();
//	printf("done");
//	printf("%d", pq->DeleteMin());
//
//	delete w;
//	DeleteWorker* w2 = new DeleteWorker(pq);
//	w2->start();
//	w2->join();
//	delete w2;
}

	void testBench3(IPriorityQueue* queue) {
		StopWatch* timer1 = new StopWatch();
		StopWatch* timer2 = new StopWatch();


		int numWorkers = 8;

		InsertWorker** insertWorkers = new  InsertWorker*[numWorkers];
//		CCP::Thread* insertWorkerThreads[] = new CCP::Thread*[numWorkers];

		for(int i=0;i<numWorkers; i++)
		{
			insertWorkers[i] = new InsertWorker(queue, 100*i ,100);
//			insertWorkerThreads[i] = new CCP::Thread(insertWorkers[i]);
		}

		DeleteWorker** deleteWorkers = new  DeleteWorker*[numWorkers];
//		Thread[] deleteWorkerThreads = new Thread[numWorkers];

		for(int i=0;i<numWorkers; i++)
		{
			deleteWorkers[i] = new DeleteWorker(queue);
//			deleteWorkerThreads[i] = new Thread(deleteWorkers[i]);
		}


		for(int i=0;i<numWorkers;i++)
		{
			insertWorkers[i]->start();
		}



		timer1->startTimer();

		//    try {
			//         Thread.sleep(numMilliseconds);
		//    } catch (InterruptedException ignore) {;}


		for(int i=0;i<numWorkers;i++)
		{
//			try {
				insertWorkers[i]->join();
				delete insertWorkers[i];
//			} catch (InterruptedException ignore) {;}
		}

		delete [] insertWorkers;

		timer1->stopTimer();
//		printf("List after all insertions:\n");
//		printForHistogram(queue->head());

		printf("Starting delete\n");
		timer2->startTimer();
		for(int i=0;i<numWorkers;i++)
		{
			deleteWorkers[i]->start();
		}

//		printf("Mid state:\n");
//		usleep(3000);
//		printForHistogram(queue->head());

		for(int i=0;i<numWorkers;i++)
		{
//			try {
				deleteWorkers[i]->join();
				delete deleteWorkers[i];
//			} catch (InterruptedException ignore) {;}
		}

		delete [] deleteWorkers;

		timer2->stopTimer();
		// Output the statistics

		printf("insert time %u\n", (unsigned int)timer1->getElapsedTime());
		printf("delete time %u\n", (unsigned int)timer2->getElapsedTime());
		delete timer1;
		delete timer2;
	}
