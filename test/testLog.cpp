#include "../log/log.h"

void TestLog() 
{
    int cnt = 0;
    Log::getInstance()->init(0,"./testlog1");//同步日志
    for(int j = 0; j < 15000; j++ )
    {
        LOG_DEBUG("%s 111111111 %d ============= ", "Test", cnt++);
        LOG_INFO ("%s 111111111 %d ============= ", "Test", cnt++);
        LOG_WARN ("%s 111111111 %d ============= ", "Test", cnt++);
        LOG_ERROR("%s 111111111 %d ============= ", "Test", cnt++);
    }

    cnt = 0;
    Log::getInstance()->init(1024,"./testlog2");//异步日志
    for(int j = 0; j < 15000; j++ )
    {
        LOG_DEBUG("%s 222222222 %d ============= ", "Test", cnt++);
        LOG_INFO ("%s 222222222 %d ============= ", "Test", cnt++);
        LOG_WARN ("%s 222222222 %d ============= ", "Test", cnt++);
        LOG_ERROR("%s 222222222 %d ============= ", "Test", cnt++);
    }
}

int main() 
{
    TestLog();
}