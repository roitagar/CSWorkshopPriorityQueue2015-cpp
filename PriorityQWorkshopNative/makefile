CPPSRCS		= mytest.cpp PriorityQueue/SprayListNode.cpp PriorityQueue/SprayListPriorityQueue.cpp PriorityQueue/NaiveLockSprayListPriorityQueue.cpp # cpp_framework.cpp
TARGET		= myrun

CPP		= g++

CPPFLAGS	= #-O3 -m64 -DNDEBUG -DINTEL64 -D_REENTRANT -lrt -pthread  
LFLAGS		= #-O3 -m64 -DNDEBUG -DINTEL64 -D_REENTRANT -lrt -pthread

OBJS		= $(CPPSRCS:.cpp=.o)

all: $(TARGET)

cpp_framework.o:
	$(CPP) $(CPPFLAGS) -c ./framework/cpp_framework.cpp
	
mytest.o:
	$(CPP) $(CPPFLAGS) -c ./mytest.cpp

$(TARGET): $(OBJS)
	$(CPP) $(LFLAGS) $(OBJS) -o $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

depend:
	mkdep $(SRCS)
