CPPSRCS		= mytest.cpp util/ThreadRandom.cpp framework/cpp_framework.cpp PriorityQueue/news/serviceClass.cpp PriorityQueue/news/SeqSprayListPriorityQueue.cpp PriorityQueue/news/GlobalLockSprayListPriorityQueue.cpp PriorityQueue/news/LazyLockSparyListPriorityQueue.cpp Test/TestBenches.cpp PriorityQueue/news/CoolSprayListPriorityQueue.cpp
TARGET		= myrun

CPP		= g++

CPPFLAGS	= -O3 -DNDEBUG -DINTEL64 -D_REENTRANT -lrt -pthread -std=c++0x -m64 -L. -lthreadscan  
LFLAGS		= -O3 -DNDEBUG -DINTEL64 -D_REENTRANT -lrt -pthread -std=c++0x -m64 -L. -lthreadscan

OBJS		= $(CPPSRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CPP) $(LFLAGS) $(OBJS) -o $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

depend:
	mkdep $(SRCS)
