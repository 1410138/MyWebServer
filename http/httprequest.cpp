#include "httprequest.h"

const std::unordered_set<string> HttpRequest::DEFAULT_HTML=
{"/home","/register","/login","/video","/picture"};

const std::unordered_map<string,int> HttpRequest::DEFAULT_HTML_TAG=
{{"/register.html", 0},{"/login.html", 1}};

HttpRequest::HttpRequest()
{
    Log::getInstance()->init();
    init();
}

void HttpRequest::init()
{
    method="";
    path="";
    version="";
    body="";
    state = REQUEST_LINE;
    header.clear();
    post.clear();
}

bool HttpRequest::parse(Buffer& buffer)
{
    if(buffer.readableBytes()<=0)
        return false;
    while(buffer.readableBytes()&&state!=FINISH)
    {
        const char CRLF[3]="\r\n";
        const char* lineEnd=std::search(buffer.peek(),static_cast<const char*>(buffer.beginWrite()),CRLF,CRLF+2);
        string line(buffer.peek(),lineEnd);
        switch (state)
        {
        case REQUEST_LINE:
            parseRequestLine(line);
            parsePath();
            break;
        case HEADERS:
            parseHeader(line);
            break;
        case BODY:
            parseBody(line);
            break;
        default:
            break;
        }
        if(lineEnd==buffer.beginWrite())//解析完请求体(不由"\r\n"结尾)
            break;
        buffer.retrieveUntil(lineEnd+2);//解析完一行Headers(由"\r\n"结尾)
    }
    return true;
}

void HttpRequest::parsePath()
{
    if(path=="/") 
        path="/home.html";
    else
        if(DEFAULT_HTML.count(path))
            path+=".html";
}

void HttpRequest::parseRequestLine(const string& line)
{
    std::regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch match;
    if(!std::regex_match(line,match,patten))
    {
        LOG_ERROR("%s","Parse RequestLine Error");
    }
    method=match[1];
    path=match[2];
    version=match[3];
    state=HEADERS;
}

void HttpRequest::parseHeader(const string& line)
{
    std::regex patten("^([^:]*): ?(.*)$");
    std::smatch match;
    if(std::regex_match(line,match,patten))
    {
        header[match[1]]=match[2];
    }        
    else
    {
        state=BODY;
    }
}

void HttpRequest::parseBody(const string& line)
{
    body=line;
    parsePost();
    state=FINISH;
}

void HttpRequest::parsePost()
{
    if(method=="POST"&&header["Content-Type"]=="application/x-www-form-urlencoded")
    {
        parseUrlencoded();
        if(DEFAULT_HTML_TAG.count(path))
        {
            int tag=DEFAULT_HTML_TAG.find(path)->second;
            if(userVertify(post["username"],post["password"],tag))
            {
                path="/home.html";
            }
            else
            {
                path="/error.html";
            }
        }
    }
}

void HttpRequest::parseUrlencoded()
{
	std::regex patten("(?!&)(.*?)=(.*?)(?=&|$)");
    std::smatch match;
    string::const_iterator begin=body.begin();
    string::const_iterator end=body.end();
    while(std::regex_search(begin,end,match,patten))
    {
        post[match[1]]=match[2];
        begin=match[0].second;
    }
}

bool HttpRequest::userVertify(const string& username,const string& password,int tag)
{
    SqlConnPool* pool = SqlConnPool::getInstance();
    std::shared_ptr<SqlConn> conn=pool->getConn();
    string order1="SELECT username,password FROM user WHERE username='"+username+"' LIMIT 1";
    string order2="INSERT INTO user(username, password) VALUES('"+username+"','"+password+"')";
    MYSQL_RES* res=conn->query(order1);
    string user;
    string pwd;
    MYSQL_ROW row=nullptr;
    while((row=mysql_fetch_row(res))!=nullptr) 
    {
        user=row[0];
        pwd=row[1];
    }
    if(tag)//登录
    {
        if(pwd!=password)//密码错误
        {
            LOG_ERROR("%s","Password Error");
            return false;
        }
        LOG_INFO("%s Login Success",username);
    }
    else//注册
    {
        if(!user.empty())//用户名已被使用
        {
            LOG_ERROR("%s","Username Used");
            return false;
        }
        if(!conn->update(order2))//数据库插入失败
        {
            LOG_ERROR("%s","Insert Error");
            return false;
        }
        LOG_INFO("%s Register Success",username);
    }
    mysql_free_result(res);
    return true;
}

const string& HttpRequest::getMethod() const
{
    return method;
}

const string& HttpRequest::getPath() const
{
    return path;
}

const string& HttpRequest::getVersion() const
{
    return version;
}

bool HttpRequest::isKeepAlive() const
{
    if(header.count("Connection"))
    {
        return header.find("Connection")->second=="keep-alive";
    }
    return false;
}