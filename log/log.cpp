#include "log.h"

Log::Log():lineCount(0),
            today(0),
            fp(nullptr),
            deque(nullptr),
            writeThread(nullptr),
            isAsync(false){}

Log::~Log()
{
    if(writeThread&&writeThread->joinable())
    {
        while(!deque->empty())//清空阻塞队列中的全部任务
        {
            deque->flush();
        }
        deque->close();
        writeThread->join();//等待当前线程完成手中的任务
    }
    if(fp)//冲洗文件缓冲区，关闭文件描述符
    {
        std::lock_guard<std::mutex> lock(mtx);
        fflush(fp);
        fclose(fp);
    }
}

void Log::init(int maxQueueCapacity,const char* path_,const char* suffix_)
{
    if(maxQueueCapacity>0)//异步方式
    {
        isAsync=true;
        if(!deque)
        {
            std::unique_ptr<BlockQueue<std::string>> newDeque(new BlockQueue<std::string>(maxQueueCapacity));
            deque=std::move(newDeque);
            std::unique_ptr<std::thread> newThread(new std::thread(flushLogThread));
            writeThread=std::move(newThread);
        }
    }
    else//同步方式
    {
        isAsync=false;
    }
    lineCount=0;
    //生成日志文件名
    time_t timer=time(nullptr);
    struct tm* sysTime=localtime(&timer);
    struct tm t=*sysTime;
    path=path_;
    suffix=suffix_;
    char filename[LOG_NAME_LEN]={0};
    snprintf(filename,LOG_NAME_LEN-1,"%s/%04d_%02d_%02d%s",
            path,t.tm_year+1900,t.tm_mon+1,t.tm_mday,suffix);
    today=t.tm_mday;
    {
        std::lock_guard<std::mutex> lock(mtx);
        buff.retrieveAll();
        if(fp)
        {
            fflush(fp);
            fclose(fp);
        }
        fp=fopen(filename,"a");
        if(fp==nullptr)
        {
            mkdir(path,0777);//先生成目录文件（最大权限）
            fp=fopen(filename,"a");
        }
        assert(fp!=nullptr);
    }
}

void Log::writeLog(int level, const char* format, ...)
{
    struct timeval now={0,0};
    gettimeofday(&now,nullptr);
    time_t tSec=now.tv_sec;
    struct tm* sysTime=localtime(&tSec);
    struct tm t=*sysTime;
    va_list vaList;

    if(today!=t.tm_mday||(lineCount&&(lineCount%MAX_LOG_LINES==0)))
    {
        //生成最新的日志文件名
        char newFile[LOG_NAME_LEN];
        char tail[36]={0};
        snprintf(tail,35,"%04d_%02d_%02d",t.tm_year+1900,t.tm_mon+1,t.tm_mday);
        if(today!=t.tm_mday)//时间不匹配，则替换为最新的日志文件名
        {
            snprintf(newFile,LOG_NAME_LEN-1,"%s/%s%s",path,tail,suffix);
            today=t.tm_mday;
            lineCount=0;
        }
        else//长度超过日志最长行数，则生成xxx-1、xxx-2文件
        {
            snprintf(newFile,LOG_NAME_LEN-1,"%s/%s-%d%s",path,tail,(lineCount/MAX_LOG_LINES),suffix);
        }

        if(fp)
        {
            std::lock_guard<std::mutex> lock(mtx);
            fflush(fp);
            fclose(fp);
        }
        fp=fopen(newFile,"a");
        assert(fp!=nullptr);
    }
    
    //在buffer内生成一条对应的日志信息
    {
        std::lock_guard<std::mutex> lock(mtx);
        lineCount++;
        //添加年月日时分秒微秒———"2022-12-29 19:08:23.406500"
        int n=snprintf(buff.beginWrite(),128,"%04d-%02d-%02d %02d:%02d:%02d.%06ld ",
                        t.tm_year+1900,t.tm_mon+1,t.tm_mday,
                        t.tm_hour,t.tm_min,t.tm_sec,now.tv_usec);
        buff.hasWritten(n);
        //添加日志等级———"2022-12-29 19:08:23.406539 [debug]: "
        switch(level) 
        {
        case 0:
            buff.append("[debug]: ", 9);
            break;
        case 1:
            buff.append("[info] : ", 9);
            break;
        case 2:
            buff.append("[warn] : ", 9);
            break;
        case 3:
            buff.append("[error]: ", 9);
            break;
        default:
            buff.append("[info] : ", 9);
            break;
        }
        //添加使用日志时的格式化输入———"2022-12-29 19:08:23.535531 [debug]: Test 222222222 8 ============= "
        va_start(vaList, format);
        int m = vsnprintf(buff.beginWrite(), buff.writableBytes(), format, vaList);
        va_end(vaList);
        buff.hasWritten(m);
        //添加换行符与字符串结尾
        buff.append("\n\0", 2);
    }
    
    if(isAsync&&deque&&!deque->full())//异步方式（加入阻塞队列中，等待写线程读取日志信息）
    {
        deque->push_back(buff.retrieveAllAsString());
    }
    else//同步方式（直接向文件中写入日志信息）
    {
        std::lock_guard<std::mutex> lock(mtx);
        fputs(buff.peek(),fp);
    }
    {//清理buffer缓冲区
        std::lock_guard<std::mutex> lock(mtx);
        buff.retrieveAll();
    }
}

void Log::asyncWrite()
{
    std::string str="";
    while (deque->pop(str))
    {
        std::lock_guard<std::mutex> lock(mtx);
        fputs(str.c_str(),fp);
    }
}