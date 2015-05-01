#include <memory>
#include <stdio.h>
#include "PriorityQueue/IPriorityQueue.h"
#include "PriorityQueue/news/IPriorityQueue.h"
#include "PriorityQueue/news/GlobalLockSprayListPriorityQueue.h"
#include "PriorityQueue/news/LazyLockSparyListPriorityQueue.h"
#include "PriorityQueue/news/CoolSprayListPriorityQueue.h"
#include "PriorityQueue/NaiveLockSprayListPriorityQueue.h"
#include "PriorityQueue/GrainedLockSprayListPriorityQueue.h"
#include "Test/Workers.h"
#include "Test/StopWatch.h"
#include "Test/TestBench.h"
#include "Test/TestBenches.h"
#include "Test/TestUtils.h"
#include "Test/PriorityQueueFactory.h"
#include <string>

#define FACTORIES 1

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

int main()
{
	const int skiplistHeight = 10;
	const int highestOnQueue = 100000;
	const int timeOutMillisecond = 500;

	int insertWorkerCount;
	int deleteWorkerCount;
	IPriorityQueue* pq = NULL;

	PriorityQueueFactory* factories[FACTORIES] = {
//			new GlobalLockSprayListPriorityQueueFactory(),
			new CoolSprayListPriorityQueueFactory(),
//			new OptimisticCoolSprayListPriorityQueueFactory(),
//			new LazyLockSparyListPriorityQueueFactory()
	};

	TestBench* simultaneousTests[] = {
			testBench2,
			testBench5,
			testBench8,
			testBench11,
			testBench14,
			testBench7,
			testBench16,
	};

	TestBench* serialTests[] = {
			testBench3,
			testBench6,
			testBench10,
			testBench13,
			testBench15,
			testBench17,
	};

	TestBench** tests[] = {simultaneousTests, serialTests};
	int testCount[] = {7, 6};

	string result[21];

	for(int fact=0;fact<FACTORIES; fact++)
	{
		PriorityQueueFactory& factory = *(factories[fact]);
		int inserters[2][8]	= {{1, 2, 3, 4, 1, 7, 1, 3}, {1, 2, 3, 4, 5, 6, 7, 8}};
		int deleters[2][8]	= {{1, 2, 3, 4, 7, 1, 3, 1}, {1, 2, 3, 4, 5, 6, 7, 8}};
		for(int i=0;i<2/*inserters.length*/;i++)
		{
			// Print headers // TODO: Remove?
			result[0] = "Queue type";
			result[1] = "Set";
			result[2] = "Threads";
			result[3] = "Test";
			// TODO: Test name?
			result[4] = "Iwrkr";
			result[5] = "Dwrkr";
			result[6] = "Icnt";
			result[7] = "Itime";
			result[8] = "Dcnt";
			result[9] = "Dtime";
			result[10] = "t-out";
			result[11] = "highest";
			result[12] = "height";
			result[13] = "Grades";
			saveResult(result, 14);

			for(int j=0;j<8/*inserters[i].length*/;j++)
			{
				insertWorkerCount = inserters[i][j];
				deleteWorkerCount = deleters[i][j];

				for(int k=0;k<testCount[i];k++)
				{
					TestBench& tb = *(tests[i][k]);
					pq = factory.Create(skiplistHeight);
					tb.setQueue(pq);
					tb.setNumDeleteWorkers(deleteWorkerCount);
					tb.setNumInsertWorkers(insertWorkerCount);
					tb.setHighestOnQueue(highestOnQueue);
					tb.setTimeOutMillisecond(timeOutMillisecond);

					tb.runTest();

					// prepare result output
//					result = new ArrayList<String>();

					int fields = 13;
					result[0] = factory.getQueueType(); // queue type
					result[1] = to_string(i); // Test series index
					result[2] = to_string(j); // Thread composition index
					result[3] = to_string(k); // Test index
					// TODO: Test name?
					result[4] = to_string(insertWorkerCount); // Insert worker count
					result[5] = to_string(deleteWorkerCount); // Delete worker count
					result[6] = to_string(tb.getResult().insertCount); // Insert count
					result[7] = to_string(tb.getResult().insertTime); // Insert time
					result[8] = to_string(tb.getResult().deleteCount); // Delete count
					result[9] = to_string(tb.getResult().deleteTime); // Delete time
					result[10] = to_string(timeOutMillisecond); // Set timeout
					result[11] = to_string(highestOnQueue); // Highest setting
					result[12] = to_string(skiplistHeight); // Skiplist height

					// Grade
					if(tb.getResult().grade != null) {
						for(int l = 0;l<deleteWorkerCount;l++){
							result[13+l] = to_string(tb.getResult().grade[l]);
							fields++;
						}
					}

					// Write result to output
					saveResult(result, fields);

					delete pq;
				}
			}
		}
	}

	for(int fact=0;fact<FACTORIES;fact++)
	{
		delete factories[fact];
	}
}

