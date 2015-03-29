#include "TestBench.h"
#include "TestBenches.h"
#include "../PriorityQueue/news/IPriorityQueue.h"
#include "Workers.h"
#include "../PriorityQueue/news/serviceClass.h"
#include "LockFreeRandom.h"

	class InterleavingStepGenerator : public INumberGenerator
	{
		const int _step;
		int _seed;

	public:
		InterleavingStepGenerator(int seed, int step) : _step(step)
		{
			_seed = seed;
		}

		virtual int getNext() {
			int tmp = _seed;
			_seed += _step;
			return tmp;
		}
	};

	class DecreasingStepGenerator : public INumberGenerator
	{
		int _value;

	public:
		DecreasingStepGenerator(int highest)
		{
			_value = highest;
		}

		virtual int getNext() {

			if(_value==0) return 0;

			return _value--;
		}
	};

	class IncreasingStepGenerator : public INumberGenerator
	{
		int _value;
		int _highest;
	public:
		IncreasingStepGenerator(int highest){
			_highest = highest;
			_value = 0;
		}

		IncreasingStepGenerator() {
			_highest = -1;
			_value = 0;
		}

		virtual int getNext() {
			//if value reached the highest number, return the highest
			if(_value==_highest) return _highest;
			return _value++;
		}
	};

	class RandomStepGenerator : public INumberGenerator
	{
		LockFreeRandom _random;

	public:
		RandomStepGenerator(int seed) :
			_random(seed)
		{
		}


		virtual int getNext() {
			return	_random.nextInt();
//			return serviceClass::randomInt();
		}
	};


	class : public TestBench {
		virtual void run() {
			bool doneWorkers = false;
			volatile bool memFence = false;

			_insertWorkerThreads = new InsertWorker*[_numInsertWorkers];
			for(int i=0; i < _numInsertWorkers; i++)
			{
				_insertWorkerThreads[i] = new SimpleInsertWorker(_queue, getItemsPerThread()*i ,getItemsPerThread());
			}

//			_deleteWorkerThreads = new CCP::Thread*[_numDeleteWorkers];
			_deleteWorkerThreads = new GradedWorkerBase*[_numDeleteWorkers];

			for(int i=0;i<_numDeleteWorkers; i++)
			{
				_deleteWorkerThreads[i] = new AdvancedDeleteWorker(&doneWorkers, _queue);
			}

			startAllWorkers();

			joinInsertWorkers();

			// Stop delete Workers - they are responsible for leaving the queue empty
			doneWorkers = true;
			memFence = true; // memFence is a 'volatile' forcing a memory fence
			// which means that done.value is visible to the workers

			joinDeleteWorkers();


			//get grade of each worker
			int* grade = new int[_numDeleteWorkers];
			for(int i=0; i<_numDeleteWorkers;i++){
				grade[i]=_deleteWorkerThreads[i]->getGrade();
			}
			
			long totalCount = 0;
			for(int i=0;i<_numDeleteWorkers;i++)
			{
				totalCount+= _deleteWorkerThreads[i]->totalPackets();
			}
			
			saveResult(getItemsPerThread()*_numInsertWorkers, totalCount, grade);

			cleanWorkers();
		}
	} test2;

	class : public TestBench {
		virtual void run() {
			_insertWorkerThreads = new InsertWorker*[_numInsertWorkers];

			for(int i=0;i<_numInsertWorkers; i++)
			{
				_insertWorkerThreads[i] = new SimpleInsertWorker(_queue, getItemsPerThread()*i ,getItemsPerThread());
			}

			_deleteWorkerThreads = new GradedWorkerBase*[_numDeleteWorkers];

			for(int i=0;i<_numDeleteWorkers; i++)
			{
				_deleteWorkerThreads[i] = new SimpleDeleteWorker(_queue);
			}


			startInsertWorkers();

			joinInsertWorkers();

			startDeleteWorkers();

			joinDeleteWorkers();
			
			//get grade of each worker
			int* grade = new int[_numDeleteWorkers];
			for(int i=0; i<_numDeleteWorkers;i++){
				grade[i]=_deleteWorkerThreads[i]->getGrade();
			}
			
			// Output the statistics
			
			long totalCount = 0;
			for(int i=0;i<_numDeleteWorkers;i++)
			{
				totalCount+= _deleteWorkerThreads[i]->totalPackets();
			}

			saveResult(getItemsPerThread()*_numInsertWorkers, totalCount, grade);

			cleanWorkers();
		}
	} test3;

	/**
	 * All workers run for a constant amount of time, interleaved values inserted
	 */
	class : public TestBench{
		virtual void run() {

			// Allocate and initialize locks and any signals used to marshal threads (eg. done signals)
			bool doneDispatcher = false;
			bool doneWorkers = false;
			volatile bool memFence = false;

			_insertWorkerThreads = new InsertWorker*[_numInsertWorkers];

			for(int i=0;i<_numInsertWorkers; i++)
			{
				// Initialize insert workers with interleaving number generators
				_insertWorkerThreads[i] = new AdvancedInsertWorker(&doneDispatcher, new InterleavingStepGenerator(i,  _numInsertWorkers), _queue);
			}

			_deleteWorkerThreads = new GradedWorkerBase*[_numDeleteWorkers];

			for(int i=0;i<_numDeleteWorkers; i++)
			{
				_deleteWorkerThreads[i] = new AdvancedDeleteWorker(&doneWorkers, _queue);
			}


			// Start all the workers
			startAllWorkers();


			CCP::Thread::sleep(_timeOutMilliseconds);


			// stop insert workers
			doneDispatcher = true;
			memFence = true; // memFence is a 'volatile' forcing a memory fence
			// which means that done.value is visible to the workers


			joinInsertWorkers();
			
			// Stop delete Workers - they are responsible for leaving the queue empty
			doneWorkers = true;
			memFence = true; // memFence is a 'volatile' forcing a memory fence
			// which means that done.value is visible to the workers

			joinDeleteWorkers();
			
			//get grade of each worker
			int* grade = new int[_numDeleteWorkers];
			for(int i=0; i<_numDeleteWorkers;i++){
				grade[i]=_deleteWorkerThreads[i]->getGrade();
			}
			
			// Output the statistics

			long totalCount = 0;
			for(int i=0;i<_numDeleteWorkers;i++)
			{
				totalCount+= _deleteWorkerThreads[i]->totalPackets();
			}

			saveResult(totalCount, totalCount, grade);

			cleanWorkers();
		}
	} test5;

	class : public TestBench {
		virtual void run() {
			// Allocate and initialize locks and any signals used to marshal threads (eg. done signals)
			bool doneDispatcher = false;
			bool doneWorkers = false;
			volatile bool memFence = false;

			_insertWorkerThreads = new InsertWorker*[_numInsertWorkers];

			for(int i=0;i<_numInsertWorkers; i++)
			{
				// Initialize insert workers with interleaving number generators
				_insertWorkerThreads[i] = new AdvancedInsertWorker(&doneDispatcher, new InterleavingStepGenerator(i,  _numInsertWorkers), _queue);
			}

			_deleteWorkerThreads = new GradedWorkerBase*[_numDeleteWorkers];

			for(int i=0;i<_numDeleteWorkers; i++)
			{
				_deleteWorkerThreads[i] = new AdvancedDeleteWorker(&doneWorkers, _queue);
			}

			/**********		 Insertion part	**********/

			// Start insert workers
			startInsertWorkers();


			CCP::Thread::sleep(_timeOutMilliseconds);


			// stop insert workers
			doneDispatcher = true;
			memFence = true; // memFence is a 'volatile' forcing a memory fence
			// which means that done.value is visible to the workers

			joinInsertWorkers();

			long totalInsertCount = 0;
			for(int i=0;i<_numInsertWorkers;i++)
			{
				totalInsertCount+= _insertWorkerThreads[i]->totalPackets();
			}
			
			/**********		 Deletion part  	**********/

			// Start delete workers
			startDeleteWorkers();

			
			// Stop delete Workers - they are responsible for leaving the queue empty
			doneWorkers = true;
			memFence = true; // memFence is a 'volatile' forcing a memory fence
			// which means that done.value is visible to the workers

			joinDeleteWorkers();
			
			// Output the statistics for delete only
			long totalDeleteCount = 0;
			for(int i=0;i<_numDeleteWorkers;i++)
			{
				totalDeleteCount+= _deleteWorkerThreads[i]->totalPackets();
			}

			//get grade of each worker
			int* grade = new int[_numDeleteWorkers];
			for(int i=0; i<_numDeleteWorkers;i++){
				grade[i]=_deleteWorkerThreads[i]->getGrade();
			}
			
			saveResult(totalInsertCount, totalDeleteCount, grade);

			cleanWorkers();
		}
	} test6;

	class : public TestBench {
		virtual void run() {
			// Allocate and initialize locks and any signals used to marshal threads (eg. done signals)
			bool doneDispatcher = false;
			bool doneWorkers = false;
			volatile bool memFence = false;

			_insertWorkerThreads = new InsertWorker*[_numInsertWorkers];

			for(int i=0;i<_numInsertWorkers; i++)
			{
				// Initialize insert workers with interleaving number generators
				_insertWorkerThreads[i] = new AdvancedInsertWorker(&doneDispatcher, new InterleavingStepGenerator(i,  _numInsertWorkers), _queue);
			}

			_deleteWorkerThreads = new GradedWorkerBase*[_numDeleteWorkers];

			for(int i=0;i<_numDeleteWorkers; i++)
			{
				_deleteWorkerThreads[i] = new AdvancedDeleteWorkerWithoutEmptying(&doneWorkers, _queue);
			}


			// Start all the workers
			startAllWorkers();


			CCP::Thread::sleep(_timeOutMilliseconds);


			// Stop delete Workers - they are responsible for leaving the queue empty
			doneWorkers = true;
			memFence = true; // memFence is a 'volatile' forcing a memory fence
			// which means that done.value is visible to the workers

			joinDeleteWorkers();

			// stop insert workers
			doneDispatcher = true;
			memFence = true; // memFence is a 'volatile' forcing a memory fence
			// which means that done.value is visible to the workers

			joinInsertWorkers();
			
			// Output the statistics for the delete min

			long totalInsertCount = 0;
			for(int i=0;i<_numInsertWorkers;i++)
			{
				totalInsertCount+= _insertWorkerThreads[i]->totalPackets();
			}

			long totalDeleteCount = 0;
			for(int i=0;i<_numDeleteWorkers;i++)
			{
				totalDeleteCount+= _deleteWorkerThreads[i]->totalPackets();
			}

			//get grade of each worker
			int* grade = new int[_numDeleteWorkers];
			for(int i=0; i<_numDeleteWorkers;i++){
				grade[i]=_deleteWorkerThreads[i]->getGrade();
			}
			
			saveResult(totalInsertCount, totalDeleteCount, grade);

			cleanWorkers();
		}
	} test7;

	class : public TestBench {
		virtual void run() {
			// Allocate and initialize locks and any signals used to marshal threads (eg. done signals)
			bool doneDispatcher = false;
			bool doneWorkers = false;
			volatile bool memFence = false;

			_insertWorkerThreads = new InsertWorker*[_numInsertWorkers];
			DecreasingStepGenerator* decreasingStepGenerator = new DecreasingStepGenerator(_highest);
			for(int i=0;i<_numInsertWorkers; i++)
			{
				// Initialize insert workers with interleaving number generators
				_insertWorkerThreads[i] = new AdvancedInsertWorker(&doneDispatcher,decreasingStepGenerator , _queue);
			}

			_deleteWorkerThreads = new GradedWorkerBase*[_numDeleteWorkers];

			for(int i=0;i<_numDeleteWorkers; i++)
			{
				_deleteWorkerThreads[i] = new AdvancedDeleteWorker(&doneWorkers, _queue);
			}


			// Start all the workers
			startAllWorkers();
			
			CCP::Thread::sleep(_timeOutMilliseconds);


			// stop insert workers
			doneDispatcher = true;
			memFence = true; // memFence is a 'volatile' forcing a memory fence
			// which means that done.value is visible to the workers

			
			joinInsertWorkers();

			
			// Stop delete Workers - they are responsible for leaving the queue empty
			doneWorkers = true;
			memFence = true; // memFence is a 'volatile' forcing a memory fence
			// which means that done.value is visible to the workers


			joinDeleteWorkers();


			// Output the statistics for the delete min

			long totalCount = 0;
			for(int i=0;i<_numDeleteWorkers;i++)
			{
				totalCount+= _deleteWorkerThreads[i]->totalPackets();
			}
			
			// grade is irrelevant due to decreasing insertion order
			saveResult(totalCount, totalCount, null);

			cleanWorkers(false);

			delete decreasingStepGenerator;
		}
	} test8;


	class : public TestBench {
		virtual void run() {
			// Allocate and initialize locks and any signals used to marshal threads (eg. done signals)
			bool doneWorkers = false;
			volatile bool memFence = false;

			_insertWorkerThreads = new InsertWorker*[_numInsertWorkers];
			DecreasingStepGenerator* decreasingStepGenerator = new DecreasingStepGenerator(_highest);
			for(int i=0;i<_numInsertWorkers; i++)
			{
				// Initialize insert workers with interleaving number generators
				_insertWorkerThreads[i] = new AdvancedInsertWorkerUntilValue(decreasingStepGenerator , _queue, 0);
			}

			_deleteWorkerThreads = new GradedWorkerBase*[_numDeleteWorkers];

			for(int i=0;i<_numDeleteWorkers; i++)
			{
				_deleteWorkerThreads[i] = new AdvancedDeleteWorker(&doneWorkers, _queue);
			}


			// Start all the workers
			startAllWorkers();


			// no need to stop insert workers - will finish when reaching to value=0
			joinInsertWorkers();

			// Stop delete Workers - they are responsible for leaving the queue empty
			doneWorkers = true;
			memFence = true; // memFence is a 'volatile' forcing a memory fence
			// which means that done.value is visible to the workers

			joinDeleteWorkers();

			// Output the statistics for the delete min
			long totalCount = 0;
			for(int i=0;i<_numDeleteWorkers;i++)
			{
				totalCount+= _deleteWorkerThreads[i]->totalPackets();
			}

			//get grade of each worker
			int* grade = new int[_numDeleteWorkers];
			for(int i=0; i<_numDeleteWorkers;i++){
				grade[i]=_deleteWorkerThreads[i]->getGrade();
			}
			
			saveResult(_highest, totalCount, grade);

			cleanWorkers(false);

			delete decreasingStepGenerator;
		}
	} test9;


	class : public TestBench {
		virtual void run() {

			// Allocate and initialize locks and any signals used to marshal threads (eg. done signals)
			bool doneWorkers = false;
			volatile bool memFence = false;

			_insertWorkerThreads = new InsertWorker*[_numInsertWorkers];
			DecreasingStepGenerator* decreasingStepGenerator = new DecreasingStepGenerator(_highest);
			for(int i=0;i<_numInsertWorkers; i++)
			{
				// Initialize insert workers with interleaving number generators
				_insertWorkerThreads[i] = new AdvancedInsertWorkerUntilValue(decreasingStepGenerator , _queue, 0);
			}

			_deleteWorkerThreads = new GradedWorkerBase*[_numDeleteWorkers];

			for(int i=0;i<_numDeleteWorkers; i++)
			{
				_deleteWorkerThreads[i] = new AdvancedDeleteWorker(&doneWorkers, _queue);
			}


			/**********		 Deal with insert only		**********/

			// Start insert workers
			startInsertWorkers();


			joinInsertWorkers();
			
			// Output the statistics for insert only

			long totalInsertCount = 0;
			for(int i=0;i<_numInsertWorkers;i++)
			{
				totalInsertCount+= _insertWorkerThreads[i]->totalPackets();
			}


			/**********		 Deal with delete only		**********/

			// Start delete workers
			startDeleteWorkers();


			// Stop delete Workers - they are responsible for leaving the queue empty
			doneWorkers = true;
			memFence = true; // memFence is a 'volatile' forcing a memory fence
			// which means that done.value is visible to the workers

			joinDeleteWorkers();
			
			// Output the statistics for delete only

			long totalDeleteCount = 0;
			for(int i=0;i<_numDeleteWorkers;i++)
			{
				totalDeleteCount+= _deleteWorkerThreads[i]->totalPackets();
			}
			
			//get grade of each worker
			int* grade = new int[_numDeleteWorkers];
			for(int i=0; i<_numDeleteWorkers;i++){
				grade[i]=_deleteWorkerThreads[i]->getGrade();
			}
			
			saveResult(totalInsertCount, totalDeleteCount, grade);

			cleanWorkers(false);

			delete decreasingStepGenerator;
		}
	} test10;

	class : public TestBench {
		virtual void run() {
			// Allocate and initialize locks and any signals used to marshal threads (eg. done signals)
			bool doneDispatcher = false;
			bool doneWorkers = false;
			volatile bool memFence = false;

			_insertWorkerThreads = new InsertWorker*[_numInsertWorkers];
			IncreasingStepGenerator* increasingStepGenerator = new IncreasingStepGenerator();
			for(int i=0;i<_numInsertWorkers; i++)
			{
				// Initialize insert workers with interleaving number generators
				_insertWorkerThreads[i] = new AdvancedInsertWorker(&doneDispatcher,increasingStepGenerator , _queue);
			}

			_deleteWorkerThreads = new GradedWorkerBase*[_numDeleteWorkers];

			for(int i=0;i<_numDeleteWorkers; i++)
			{
				_deleteWorkerThreads[i] = new AdvancedDeleteWorker(&doneWorkers, _queue);
			}


			// Start all the workers
			startAllWorkers();

			CCP::Thread::sleep(_timeOutMilliseconds);


			// stop insert workers
			doneDispatcher = true;
			memFence = true; // memFence is a 'volatile' forcing a memory fence
			// which means that done.value is visible to the workers


			joinInsertWorkers();


			// Stop delete Workers - they are responsible for leaving the queue empty
			doneWorkers = true;
			memFence = true; // memFence is a 'volatile' forcing a memory fence
			// which means that done.value is visible to the workers


			joinDeleteWorkers();


			// Output the statistics for the delete min

			long totalCount = 0;
			for(int i=0;i<_numDeleteWorkers;i++)
			{
				totalCount+= _deleteWorkerThreads[i]->totalPackets();
			}
			
			//get grade of each worker
			int* grade = new int[_numDeleteWorkers];
			for(int i=0; i<_numDeleteWorkers;i++){
				grade[i]=_deleteWorkerThreads[i]->getGrade();
			}
			
			saveResult(totalCount, totalCount, grade);

			cleanWorkers(false);

			delete increasingStepGenerator;
		}
	} test11;

	class : public TestBench {
		virtual void run() {
			// Allocate and initialize locks and any signals used to marshal threads (eg. done signals)
			bool doneWorkers = false;
			volatile bool memFence = false;

			_insertWorkerThreads = new InsertWorker*[_numInsertWorkers];
			IncreasingStepGenerator* increasingStepGenerator = new IncreasingStepGenerator(_highest);
			for(int i=0;i<_numInsertWorkers; i++)
			{
				// Initialize insert workers with interleaving number generators
				_insertWorkerThreads[i] = new AdvancedInsertWorkerUntilValue(increasingStepGenerator , _queue, _highest);
			}

			_deleteWorkerThreads = new GradedWorkerBase*[_numDeleteWorkers];

			for(int i=0;i<_numDeleteWorkers; i++)
			{
				_deleteWorkerThreads[i] = new AdvancedDeleteWorker(&doneWorkers, _queue);
			}


			// Start all the workers
			startAllWorkers();


			// no need to stop insert workers - will finish when reaching to value=0
			joinInsertWorkers();

			// Stop delete Workers - they are responsible for leaving the queue empty
			doneWorkers= true;
			memFence = true; // memFence is a 'volatile' forcing a memory fence
			// which means that done.value is visible to the workers

			joinDeleteWorkers();

			// Output the statistics for the delete min
			long totalCount = 0;
			for(int i=0;i<_numDeleteWorkers;i++)
			{
				totalCount+= _deleteWorkerThreads[i]->totalPackets();
			}
			
			//get grade of each worker
			int* grade = new int[_numDeleteWorkers];
			for(int i=0; i<_numDeleteWorkers;i++){
				grade[i]=_deleteWorkerThreads[i]->getGrade();
			}
			
			saveResult(_highest, totalCount, grade);

			cleanWorkers(false);

			delete increasingStepGenerator;
		}
	} test12;

	class : public TestBench {
		virtual void run() {

			// Allocate and initialize locks and any signals used to marshal threads (eg. done signals)
			bool doneWorkers = false;
			volatile bool memFence = false;

			_insertWorkerThreads = new InsertWorker*[_numInsertWorkers];
			IncreasingStepGenerator* increasingStepGenerator = new IncreasingStepGenerator(_highest);
			for(int i=0;i<_numInsertWorkers; i++)
			{
				// Initialize insert workers with interleaving number generators
				_insertWorkerThreads[i] = new AdvancedInsertWorkerUntilValue(increasingStepGenerator , _queue, _highest);
			}

			_deleteWorkerThreads = new GradedWorkerBase*[_numDeleteWorkers];

			for(int i=0;i<_numDeleteWorkers; i++)
			{
				_deleteWorkerThreads[i] = new AdvancedDeleteWorker(&doneWorkers, _queue);
			}


			/**********		 Deal with insert only		**********/

			// Start insert workers
			startInsertWorkers();


			joinInsertWorkers();
			
			// Output the statistics for insert only

			long totalInsertCount = 0;
			for(int i=0;i<_numInsertWorkers;i++)
			{
				totalInsertCount+= _insertWorkerThreads[i]->totalPackets();
			}


			/**********		 Deal with delete only		**********/

			// Start delete workers
			startDeleteWorkers();


			// Stop delete Workers - they are responsible for leaving the queue empty
			doneWorkers = true;
			memFence = true; // memFence is a 'volatile' forcing a memory fence
			// which means that done.value is visible to the workers

			joinDeleteWorkers();
			
			// Output the statistics for delete only

			long totalDeleteCount = 0;
			for(int i=0;i<_numDeleteWorkers;i++)
			{
				totalDeleteCount+= _deleteWorkerThreads[i]->totalPackets();
			}
			
			//get grade of each worker
			int* grade = new int[_numDeleteWorkers];
			for(int i=0; i<_numDeleteWorkers;i++){
				grade[i]=_deleteWorkerThreads[i]->getGrade();
			}

			saveResult(totalInsertCount, totalDeleteCount, grade);

			cleanWorkers(false);

			delete increasingStepGenerator;
		}
	} test13;

	class : public TestBench {
		virtual void run() {
			// Allocate and initialize locks and any signals used to marshal threads (eg. done signals)
			bool doneDispatcher = false;
			bool doneWorkers = false;
			volatile bool memFence = false;

			_insertWorkerThreads = new InsertWorker* [_numInsertWorkers];

			for(int i=0;i<_numInsertWorkers; i++)
			{
				// Initialize insert workers with interleaving number generators
				_insertWorkerThreads[i] = new AdvancedInsertWorker(&doneDispatcher, new RandomStepGenerator(i), _queue);
			}

			_deleteWorkerThreads = new GradedWorkerBase*[_numDeleteWorkers];

			for(int i=0;i<_numDeleteWorkers; i++)
			{
				_deleteWorkerThreads[i] = new AdvancedDeleteWorker(&doneWorkers, _queue);
			}


			// Start all the workers
			startAllWorkers();


			CCP::Thread::sleep(_timeOutMilliseconds);


			// stop insert workers
			doneDispatcher = true;
			memFence = true; // memFence is a 'volatile' forcing a memory fence
			// which means that done.value is visible to the workers


			joinInsertWorkers();

			
			// Stop delete Workers - they are responsible for leaving the queue empty
			doneWorkers = true;
			memFence = true; // memFence is a 'volatile' forcing a memory fence
			// which means that done.value is visible to the workers

			
			joinDeleteWorkers();
			
			
			// Output the statistics

			long totalCount = 0;
			for(int i=0;i<_numDeleteWorkers;i++)
			{
				totalCount+= _deleteWorkerThreads[i]->totalPackets();
			}
			
			// grade is not relevant because generating random values
			saveResult(totalCount, totalCount, null);

			cleanWorkers();
		}
	} test14;

	class : public TestBench {
		virtual void run() {
			// Allocate and initialize locks and any signals used to marshal threads (eg. done signals)
			bool doneDispatcher = false;
			bool doneWorkers = false;
			volatile bool memFence = false;

			_insertWorkerThreads = new InsertWorker*[_numInsertWorkers];

			for(int i=0;i<_numInsertWorkers; i++)
			{
				// Initialize insert workers with interleaving number generators
				_insertWorkerThreads[i] = new AdvancedInsertWorker(&doneDispatcher, new RandomStepGenerator(i), _queue);
			}

			_deleteWorkerThreads = new GradedWorkerBase*[_numDeleteWorkers];

			for(int i=0;i<_numDeleteWorkers; i++)
			{
				_deleteWorkerThreads[i] = new AdvancedDeleteWorker(&doneWorkers, _queue);
			}

			/**********		 Insertion part	**********/

			// Start insert workers
			startInsertWorkers();


			CCP::Thread::sleep(_timeOutMilliseconds);


			// stop insert workers
			doneDispatcher = true;
			memFence = true; // memFence is a 'volatile' forcing a memory fence
			// which means that done.value is visible to the workers


			joinInsertWorkers();

			
			long totalInsertCount = 0;
			for(int i=0;i<_numInsertWorkers;i++)
			{
				totalInsertCount+= _insertWorkerThreads[i]->totalPackets();
			}
			
			/**********		 Deletion part  	**********/

			// Start delete workers
			startDeleteWorkers();


			// Stop delete Workers - they are responsible for leaving the queue empty
			doneWorkers = true;
			memFence = true; // memFence is a 'volatile' forcing a memory fence
			// which means that done.value is visible to the workers

			joinDeleteWorkers();
			
			// Output the statistics for delete only
			long totalDeleteCount = 0;
			for(int i=0;i<_numDeleteWorkers;i++)
			{
				totalDeleteCount+= _deleteWorkerThreads[i]->totalPackets();
			}
			
			//get grade of each worker
			int* grade = new int[_numDeleteWorkers];
			for(int i=0; i<_numDeleteWorkers;i++){
				grade[i]=_deleteWorkerThreads[i]->getGrade();
			}
			
			saveResult(totalInsertCount, totalDeleteCount, grade);

			cleanWorkers();
		}
	} test15;

	class : public TestBench {
		virtual void run() {
			{
				// Allocate and initialize locks and any signals used to marshal threads (eg. done signals)
				bool doneDispatcher = false;
				bool doneWorkers = false;
				volatile bool memFence = false;

				_insertWorkerThreads = new InsertWorker*[_numInsertWorkers];

				for(int i=0;i<_numInsertWorkers; i++)
				{
					// Initialize insert workers with interleaving number generators
					_insertWorkerThreads[i] = new AdvancedInsertWorker(&doneDispatcher, new RandomStepGenerator(i), _queue);
				}

				_deleteWorkerThreads = new GradedWorkerBase*[_numDeleteWorkers];

				for(int i=0;i<_numDeleteWorkers; i++)
				{
					_deleteWorkerThreads[i] = new AdvancedDeleteWorkerWithoutEmptying(&doneWorkers, _queue);
				}


				// Start all the workers
				startAllWorkers();


				CCP::Thread::sleep(_timeOutMilliseconds);


				// Stop delete Workers - they are responsible for leaving the queue empty
				doneWorkers = true;
				memFence = true; // memFence is a 'volatile' forcing a memory fence
				// which means that done.value is visible to the workers


				joinDeleteWorkers();

				
				// stop insert workers
				doneDispatcher = true;
				memFence = true; // memFence is a 'volatile' forcing a memory fence
				// which means that done.value is visible to the workers

				
				joinInsertWorkers();

				
				// Output the statistics for the delete min

				long totalCount = 0;
				for(int i=0;i<_numDeleteWorkers;i++)
				{
					totalCount+= _deleteWorkerThreads[i]->totalPackets();
				}
				
				// grade is not relevant because generating random values
				saveResult(0, totalCount, null);

				cleanWorkers();
			}
		}
	} test16;

	class : public TestBench {
		virtual void run() {
	
			// Allocate and initialize locks and any signals used to marshal threads (eg. done signals)
	
			_insertWorkerThreads = new InsertWorker*[_numInsertWorkers];
			IncreasingStepGenerator* increasingStepGenerator = new IncreasingStepGenerator(_highest);
			int runs = _highest/_numDeleteWorkers;
			for(int i=0;i<_numInsertWorkers; i++)
			{
				// Initialize insert workers with interleaving number generators
				_insertWorkerThreads[i] = new AdvancedInsertWorkerUntilValue(increasingStepGenerator , _queue, _highest);
			}
	
			_deleteWorkerThreads = new GradedWorkerBase*[_numDeleteWorkers];
	
			for(int i=0;i<_numDeleteWorkers; i++)
			{
				_deleteWorkerThreads[i] = new AdvancedInsertAndDelete(_queue, runs, _highest);
			}
	
	
			/**********		 Deal with insert only		**********/
	
			// Start insert workers
			startInsertWorkers();
	
	
			joinInsertWorkers();
			
			// Output the statistics for insert only
	
			long totalInsertCount = 0;
			for(int i=0;i<_numInsertWorkers;i++)
			{
				totalInsertCount+= _insertWorkerThreads[i]->totalPackets();
			}
	
	
			/**********		 Deal with delete only		**********/
	
			// Start delete workers
			startDeleteWorkers();
	
			joinDeleteWorkers();
	
			//get grade of each worker
			int* grade = new int[_numDeleteWorkers];
			for(int i=0; i<_numDeleteWorkers;i++){
				grade[i]=_deleteWorkerThreads[i]->getGrade();
			}
	
	
			// Output the statistics for delete only
			long totalDeleteCount = _highest;
	
			saveResult(totalInsertCount, totalDeleteCount, grade);

			cleanWorkers(false);

			delete increasingStepGenerator;
		}
	} test17;


TestBench* testBench2 = &test2;
TestBench* testBench3 = &test3;
TestBench* testBench5 = &test5;
TestBench* testBench6 = &test6;
TestBench* testBench7 = &test7;
TestBench* testBench8 = &test8;
TestBench* testBench9 = &test9;
TestBench* testBench10 = &test10;
TestBench* testBench11 = &test11;
TestBench* testBench12 = &test12;
TestBench* testBench13 = &test13;
TestBench* testBench14 = &test14;
TestBench* testBench15 = &test15;
TestBench* testBench16 = &test16;
TestBench* testBench17 = &test17;
