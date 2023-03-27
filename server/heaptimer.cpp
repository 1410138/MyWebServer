#include "heaptimer.h"
 
void HeapTimer::swim(size_t i)//上浮
{
	assert(i>=0&&i<heap.size());
    while (i > 0)
    {
        size_t parent = (i - 1) / 2;
        if (heap[i] >= heap[parent])
            break;
        swapNode(i, parent);
        i = parent;
    }
}
 
void HeapTimer::swapNode(size_t i, size_t j)
{
    assert(i >= 0 && i < heap.size());
    assert(j >= 0 && j < heap.size());
    std::swap(heap[i], heap[j]);
    ref[heap[i].id] = i;
    ref[heap[j].id] = j;
}
 
bool HeapTimer::sink(size_t index,size_t n)//下沉
{
    assert(index >= 0 && index < heap.size());
    assert(n >= 0 && n <= heap.size());
    size_t i = index;
    while (i * 2 + 1<n)
    {
        size_t left = i * 2 + 1;
        size_t right = i * 2 + 2;
        size_t older = left;
        if (right<n && heap[older]>heap[right])
        {
            older = right;
        }
        if (heap[older]>=heap[i])
            break;
        swapNode(i, older);
        i = older;
    }
    return i > index;
}
 
void HeapTimer::add(int id, int timeout, const TimeoutCallBack& func)//添加新的时间节点
{
    assert(id >= 0);
    size_t i;
    if (ref.count(id) == 0)
    {
        i = heap.size();
        ref[id] = i;
        heap.push_back({ id,Clock::now() + MS(timeout),func });
        swim(i);
    }
    else//若已存在，则更新阻塞时间
    {
        i = ref[id];
        heap[i].expires = Clock::now() + MS(timeout);
        heap[i].func = func;
        if (!sink(i, heap.size()))
        {
            swim(i);
        }
    }
}
 
void HeapTimer::del(size_t i)//删除指定位置的时间节点
{
    assert(!heap.empty() && i >= 0 && i < heap.size());
    size_t n = heap.size() - 1;
    if (i < n)
    {
        swapNode(i, n);
        if (!sink(i, n))
        {
            swim(i);
        }
    }
    ref.erase(heap.back().id);
    heap.pop_back();
}
 
void HeapTimer::doWork(int id)//删除指定序号的时间节点
{
    if (heap.empty() || ref.count(id) == 0)
    {
        return;
    }
    size_t i = ref[id];
    TimerNode cur = heap[i];
    cur.func();
    del(i);
}
 
void HeapTimer::adjust(int id, int timeout)//更新阻塞时间
{
    assert(!heap.empty() && ref.count(id) > 0);
    heap[ref[id]].expires = Clock::now() + MS(timeout);
    sink(ref[id], heap.size());
}
 
void HeapTimer::tick()//清除超时节点
{
    if (heap.empty())
    {
        return;
    }
    while (!heap.empty())
    {
        TimerNode cur = heap.front();
        if (std::chrono::duration_cast<MS>(cur.expires - Clock::now()).count() > 0)
        {
            break;
        }
        cur.func();
        pop();
    }
}
 
void HeapTimer::pop()//删除堆顶节点
{
    assert(!heap.empty());
    del(0);
}
 
void HeapTimer::clear()
{
    ref.clear();
    heap.clear();
}
 
int HeapTimer::getNextTick()//得到堆顶节点剩余的阻塞时间（心搏函数）
{
    tick();
    size_t res = -1;
    if (!heap.empty())
    {
        res = std::chrono::duration_cast<MS>(heap.front().expires - Clock::now()).count();
        if (res < 0)
            res = 0;
    }
    return res;
}
