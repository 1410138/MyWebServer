#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <memory>
#include <string>

#include "sqlconn.h"
#include "../log/blockqueue.h"
#include "../log/log.h"

class SqlConnPool
{
public:
    static SqlConnPool* getInstance(int maxCapacity=1024)
    {
        static SqlConnPool single(maxCapacity);
        return &single;
    }
    std::shared_ptr<SqlConn> getConn();
    
private:
    SqlConnPool(int maxCapacity);
    ~SqlConnPool(){};
    SqlConnPool(const SqlConnPool& other)=delete;
    SqlConnPool& operator =(const SqlConnPool& other)=delete;

    bool loadConfigFile();
    //专门用于产生新连接的线程
    void produceConnTask();
    //扫描超过最大空闲时间maxIdleTime的连接，对其进行回收
    void scannerConnTask();
    //添加新连接
    void addConn();

    std::string ip;
    uint16_t port;
    std::string user;
    std::string pwd;
    std::string dbName;

    size_t initSize;//初始连接数
    size_t maxIdleTime;//最大空闲时间
    size_t connTimeout;//超时时间

    BlockQueue<SqlConn*> connQue;
};

#endif // !SQLCONNPOOL_H