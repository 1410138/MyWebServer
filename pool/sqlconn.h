#ifndef SQLCONN_H
#define SQLCONN_H

#include <mysql/mysql.h>
#include <chrono>
#include <string>

class SqlConn
{
public:
	SqlConn();
	~SqlConn();
	
	bool connect(const std::string& ip,
				 const uint16_t port, 
				 const std::string& user, 
				 const std::string& pwd,
                 const std::string& dbName);

	// 更新操作 insert、delete、update
	bool update(const std::string& sql);
	// 查询操作 select
	MYSQL_RES* query(const std::string& sql);

	void refreshAliveTime()
	{
		aliveTime = std::chrono::steady_clock::now();
	}
	size_t getAliveTime()
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - aliveTime).count();
	}
private:
	MYSQL* conn;
	std::chrono::time_point<std::chrono::steady_clock> aliveTime; //存活时间
};

#endif // !SQLCONN_H