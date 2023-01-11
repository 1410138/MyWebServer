#include <iostream>
#include "../http/httprequest.h"
using namespace std;

void testPost()
{
    HttpRequest request;
    Buffer input;
    input.append("POST /register HTTP/1.1\r\n"
            "Host: 127.0.0.1:8888\r\n"
            "User-Agent: Mozilla/5.0\r\n" 
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n"
            "Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6\r\n"
            "Accept-Encoding: gzip, deflate\r\n"
            "Content-Type: application/x-www-form-urlencoded\r\n"
            "Content-Length: 29\r\n"
            "Connection: keep-alive\r\n"
            "\r\n"
            "username=emm&password=123456");
    request.parse(input);
    cout<<"method:"<<request.getMethod()<<endl;
    cout<<"path:"<<request.getPath()<<endl;
    cout<<"version:"<<request.getVersion()<<endl;
    if(request.isKeepAlive())   
        cout<<"isKeepAlive"<<endl;
}

void testGet()
{
    HttpRequest request;
    Buffer input;
    input.append("GET /signin?next=%2F HTTP/2\r\n"
            "Host: www.zhihu.com\r\n"
            "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:91.0) Gecko/20100101 Firefox/91.0\r\n"
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
            "Accept-Language: zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2\r\n"
            "Accept-Encoding: gzip, deflate, br\r\n"
            "Connection: keep-alive\r\n"
            "Upgrade-Insecure-Requests: 1\r\n"
            "Cache-Control: max-age=0\r\n"
            "TE: trailers\r\n"
            "\r\n");
    request.parse(input);
    cout<<"method:"<<request.getMethod()<<endl;
    cout<<"path:"<<request.getPath()<<endl;
    cout<<"version:"<<request.getVersion()<<endl;
    if(request.isKeepAlive())   
        cout<<"isKeepAlive"<<endl;
}

int main()
{
    cout<<"POST------------------------------------------"<<endl;
    testPost();
    cout<<"GET-------------------------------------------"<<endl;
    testGet();
}