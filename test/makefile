CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = testLog
OBJS =  ../buffer/buffer.cpp\
		../log/blockqueue.h ../log/log.cpp\
        testLog.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $(TARGET)  -pthread -L/usr/lib64/mysql -lmysqlclient

clean:
	rm -rf $(OBJS) $(TARGET)