#include "sqlconn.h"

SqlConn::SqlConn()
{
    conn=mysql_init(nullptr);
    mysql_set_character_set(conn, "utf8");
}

SqlConn::~SqlConn()
{
    if(conn!=nullptr)
    {
        mysql_close(conn);
    }
}

bool SqlConn::connect(const std::string& ip,const uint16_t port,const std::string& user,const std::string& pwd,const std::string& dbName)
{
    MYSQL* ptr=mysql_real_connect(conn,ip.c_str(),user.c_str(),pwd.c_str(),dbName.c_str(),port,nullptr,0);
    return ptr!=nullptr;
}

bool SqlConn::update(const std::string& sql)
{
    if(mysql_query(conn,sql.c_str()))
    {
        return false;       
    }
    return true;
}

MYSQL_RES* SqlConn::query(const std::string& sql) 
{
    if(mysql_query(conn, sql.c_str())) 
    {
        return nullptr;
    }
    return mysql_use_result(conn);
}

