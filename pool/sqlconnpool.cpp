#include "sqlconnpool.h"
#include <functional>
using std::string;

bool SqlConnPool::loadConfigFile()//加载mysql.ini配置文件
{
    FILE* fp = fopen("./mysql.ini", "r");
    if (fp == nullptr)
    {
        LOG_ERROR("%s","mysql.ini file dose not exsit!");
        return false;
    }    
    while(!feof(fp)) 
    {
    	char line[1024] = {0};
        fgets(line, 1024, fp);
        string str = line;
        int idx = str.find('=', 0);
        if (idx == -1)
        {
            continue;
        }
        int endidx = str.find('\n', idx);
        string key = str.substr(0, idx);
        string value = str.substr(idx+1,endidx-idx-1);

        if (key == "ip")    
            ip = value;
        else if (key == "port")
            port = stoi(value);
        else if (key == "username")
            user = value;
        else if (key == "password") 
            pwd = value;
        else if (key == "dbName")
            dbName = value;
        else if (key == "initSize")
            initSize = stoi(value);
        else if (key == "maxIdleTime")
			maxIdleTime = stoi(value);
		else if (key == "connTimeOut")
			connTimeout = stoi(value);
    }
    fclose(fp);
    return true;
}

SqlConnPool::SqlConnPool(int maxCapacity):connQue(maxCapacity)
{
    Log::getInstance()->init();
    if (!loadConfigFile())//配置失败
    {
        return ;
    }
    for (size_t i = 0; i < initSize; i++)
    {
        addConn();
    }

    std::thread produce(std::bind(&SqlConnPool::produceConnTask, this));
    produce.detach();
    
    std::thread scanner(std::bind(&SqlConnPool::scannerConnTask, this));
    scanner.detach();
}

void SqlConnPool::produceConnTask()
{
    for(;;)
    {
        addConn();
    }
}

void SqlConnPool::scannerConnTask()
{
    for (;;)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(maxIdleTime));
        while (connQue.get_size() > initSize)
        {
            SqlConn* p = connQue.front();
            if(p->getAliveTime() >= maxIdleTime)
            {
                SqlConn* tmp=nullptr;
                connQue.pop(tmp);
                delete tmp;
            }
            else 
                break;
        }
    }
}

std::shared_ptr<SqlConn> SqlConnPool::getConn()
{
    SqlConn* conn;
    if (!connQue.pop(conn,connTimeout))
    {
        LOG_ERROR("%s","get connection timeout");
        return nullptr;
    }
   std::shared_ptr<SqlConn> sp(conn,
          [&](SqlConn* p) 
          {
              p->refreshAliveTime();
              connQue.push_back(p);
          });
    return sp;
}

 void SqlConnPool::addConn()
 {
    SqlConn* conn=new SqlConn();
    conn->connect(ip,port,user,pwd,dbName);
    conn->refreshAliveTime();
    connQue.push_back(conn);
 }