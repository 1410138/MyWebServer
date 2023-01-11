CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = myWebServer
OBJS =  buffer/buffer.cpp log/log.cpp log/blockqueue.h\
		pool/*.cpp pool/threadpool.h\
		http/*.cpp server/*.cpp\
        main.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $(TARGET)  -pthread -L/usr/lib64/mysql -lmysqlclient

clean:
	rm -rf $(OBJS) $(TARGET)