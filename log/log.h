 #ifndef LOG_H
 #define LOG_H

#include <mutex>
#include <thread>
#include <string>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <assert.h>

#include "blockqueue.h"
#include "../buffer/buffer.h"

 class Log
 {
 public:
    static Log* getInstance()
    {
        static Log instance;
        return &instance;
    }

    void init(int maxQueueCapacity = 1024,
            const char* path_="./logfile",
            const char* suffix_=".log");

    //异步写日志公有方法，调用私有方法asyncWrite
    static void flushLogThread()
    {
        Log::getInstance()->asyncWrite();
    }

    //将输出内容按照标准格式整理
   void writeLog(int level, const char* format, ...);

private:
    Log();
    ~Log();
    //异步写日志方法
    void asyncWrite();

private:
    const int LOG_NAME_LEN=256;   //日志文件最长文件名
    const int MAX_LOG_LINES=50000;//日志文件内的最长日志条数

    const char* path;      //路径名
    const char* suffix;    //后缀名  
    int lineCount;   //日志行数记录
    int today;             //按当天日期区分文件
    FILE* fp;              //打开log的文件指针
    Buffer buff;           //输出的内容
    std::unique_ptr<BlockQueue<std::string>> deque;  //阻塞队列
    std::unique_ptr<std::thread> writeThread;        //写线程
    bool isAsync;          //是否开启异步日志
    std::mutex mtx;        //同步日志必需的互斥量
};


//四个宏定义，主要用于不同类型的日志输出
#define LOG_DEBUG(format, ...) Log::getInstance()->writeLog(0, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...)  Log::getInstance()->writeLog(1, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...)  Log::getInstance()->writeLog(2, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) Log::getInstance()->writeLog(3, format, ##__VA_ARGS__)

#endif // !LOG_H