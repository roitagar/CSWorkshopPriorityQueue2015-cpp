CPPSRCS		= mytest.cpp PriorityQueue/SprayListNode.cpp PriorityQueue/SprayListPriorityQueue.cpp PriorityQueue/NaiveLockSprayListPriorityQueue.cpp PriorityQueue/GrainedLockSprayListPriorityQueue.cpp util/ThreadRandom.cpp framework/cpp_framework.cpp PriorityQueue/news/serviceClass.cpp PriorityQueue/news/SeqSprayListPriorityQueue.cpp PriorityQueue/news/GlobalLockSprayListPriorityQueue.cpp PriorityQueue/news/LazyLockSparyListPriorityQueue.cpp
TARGET		= myrun

CPP		= g++

CPPFLAGS	= -O3 -DNDEBUG -DINTEL64 -D_REENTRANT -lrt -pthread -std=c++0x #-m64  
LFLAGS		= -O3 -DNDEBUG -DINTEL64 -D_REENTRANT -lrt -pthread -std=c++0x #-m64

OBJS		= $(CPPSRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CPP) $(LFLAGS) $(OBJS) -o $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

depend:
	mkdep $(SRCS)
