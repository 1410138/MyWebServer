#ifndef HEAP_TIMER_H
#define HEAP_TIMER_H

#include<functional>
#include<chrono>
#include<assert.h>
#include<unordered_map>
#include<vector>

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

struct TimerNode
{
	int id;
	TimeStamp expires;
	TimeoutCallBack func;
	bool operator<(const TimerNode& other)
	{
		return this->expires < other.expires;
	}
};

//基于最小堆的定时器
class HeapTimer
{
public:
	HeapTimer() { heap.reserve(64); }

	~HeapTimer() { clear(); }

	void add(int id, int timeOut, const TimeoutCallBack& func);

	void adjust(int id, int newExpires);

	void doWork(int id);

	void tick();

	void pop();
	
	void clear();
	
	int getNextTick();

private:
	std::vector<TimerNode> heap;

	std::unordered_map<int, size_t> ref;

	void swim(size_t i);

	bool sink(size_t index, size_t n);

	void del(size_t i);

	void swapNode(size_t i, size_t j);
};

#endif // !HEAP_TIMER_H