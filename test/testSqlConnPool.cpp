#include <time.h>
#include <iostream>

#include "../pool/sqlconn.h"
#include "../pool/sqlconnpool.h"
using namespace std;

void SigleWithConnection()
{
	clock_t begin = clock();
	for (int i = 0; i < 10000; ++i)
	{
		SqlConnPool* pool = SqlConnPool::getInstance();

		shared_ptr<SqlConn> sp = pool->getConn();
		char sql[1024] = {0};
		sprintf(sql, "insert into user(username,password) values('%s','%s')","zhangsan","123456");
		sp->update(sql);
	}
	clock_t end = clock();
	cout << (end - begin)/1000 << " ms" << endl;
}

void SigleNoConnection()
{
	clock_t begin = clock();
	for (int i = 0; i < 10000; ++i)
	{
		SqlConn conn;
		char sql[1024] = {0};
		sprintf(sql, "insert into user(username,password) values('%s','%s')","zhangsan","123456");
		conn.connect("127.0.0.1", 3306, "root", "123456", "yourdb");
		conn.update(sql);
	}
	clock_t end = clock();
	cout << (end - begin)/1000 << " ms" << endl;
}


void runInThreadNoConn()
{   
    for (int i = 0; i < 2500; ++i)
		{
			SqlConn conn;
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(username,password) values('%s','%s')","zhangsan","123456");
			conn.connect("127.0.0.1", 3306, "root", "123456", "yourdb");
			conn.update(sql);
		}
}

void MutiNoConnection()
{
	SqlConn conn;
	conn.connect("127.0.0.1", 3306, "root", "123456", "yourdb");
	clock_t begin = clock();
	for(int i=0;i<4;i++)
    {
        thread t=thread(runInThreadNoConn);
        t.join();     
    }
	clock_t end = clock();
	cout << (end - begin)/1000 << " ms" << endl;
}

void runInThreadByConn()
{   
    for (int i = 0; i < 2500; i++)
	{
		SqlConnPool* pool = SqlConnPool::getInstance();
        shared_ptr<SqlConn> sp = pool->getConn();
		char sql[1024] = {0};
		sprintf(sql, "insert into user(username,password) values('%s','%s')","zhangsan","123456");
		if (sp == nullptr)
		{
			cout << "sp is empty" << endl;
			continue;
		}
		sp->update(sql);
	}
}

void MutiWithConnection()
{
	clock_t begin = clock();
	for(int i=0;i<4;i++)
    {
        thread t=thread(runInThreadByConn);
        t.join();     
    }
	clock_t end = clock();
	cout << (end - begin)/1000 << " ms" << endl;
}

int main()
{ 
	MutiNoConnection();
	MutiWithConnection();
	SigleNoConnection();
	SigleWithConnection();
	return 0;
}
