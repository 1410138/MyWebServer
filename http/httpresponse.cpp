#include "httpresponse.h"

const std::unordered_map<std::string,std::string> HttpResponse::SUFFIX_TYPE=
{
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".mp4",   "video/mp4" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "}
};
const std::unordered_map<int,std::string> HttpResponse::CODE_STATUS=
{
    { 200, "OK" },
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
};
const std::unordered_map<int,std::string> HttpResponse::CODE_PATH=
{
    { 400, "/400.html" },
    { 403, "/403.html" },
    { 404, "/404.html" },
};

HttpResponse::HttpResponse()
{
    code=-1;
    isKeepAlive=false;
    mmFile=nullptr;
    mmFileStat={0};
    Log::getInstance()->init();
}

HttpResponse::~HttpResponse()
{
    unmapFile();
}

void HttpResponse::unmapFile()
{
    if(mmFile)
    {
        munmap(mmFile,mmFileStat.st_size);
        mmFile=nullptr;
    }
}

void HttpResponse::init(const std::string& srcDir_,const std::string& path_, bool isKeepAlive_, int code_)
{
    assert(srcDir_!="");
    if(mmFile)
    {
        unmapFile();
    }
    srcDir=srcDir_;
    code=code_;
    path=path_;
    isKeepAlive=isKeepAlive_;
}

void HttpResponse::makeResponse(Buffer& buffer)
{
    std::string location=srcDir+path;
    //按location路径无法搜到文件，或搜到的是目录文件(404找不到)
    if(stat(location.c_str(),&mmFileStat)<0||S_ISDIR(mmFileStat.st_mode))
    {
        code=404;
    }
    else if (!(mmFileStat.st_mode&S_IROTH))//其他用户不可读(403禁止访问)
    {
        code=403;
    }
    else if(code==-1)
    {
        code=400;
    }
    errorHtml();
    addStateLine(buffer);
    addHeader(buffer);
    addContent(buffer);
}

void HttpResponse::errorHtml()
{
    if(CODE_PATH.count(code))
    {
        path=CODE_PATH.find(code)->second;
        std::string location=srcDir+path;
        stat(location.c_str(),&mmFileStat);
    }
}

void HttpResponse::addStateLine(Buffer& buffer)
{
    std::string status;
    if(CODE_STATUS.count(code))
    {
        status=CODE_STATUS.find(code)->second;
    }
    else
    {
        code=400;
        status=CODE_STATUS.find(400)->second;
    }
    buffer.append("HTTP/1.1 "+std::to_string(code)+" "+status+"\r\n");
}

void HttpResponse::addHeader(Buffer& buffer)
{
    buffer.append("Connection: ");
    if(isKeepAlive)
    {
        buffer.append("keep-alive\r\n");
    }
    else
    {
        buffer.append("close\r\n");
    }
    buffer.append("Content-type: "+getFileType()+"\r\n");
}

void HttpResponse::addContent(Buffer& buffer)
{
    std::string location=srcDir+path;
    int srcFd=open(location.c_str(),O_RDONLY);
    if(srcFd<0)
    {
        LOG_ERROR("%s","Response File NotFind!");
        return ;
    }
    void* mmRet=mmap(0,mmFileStat.st_size,PROT_READ,MAP_PRIVATE,srcFd,0);
    if(mmRet==MAP_FAILED)
    {
        LOG_ERROR("%s","Response File NotFind!");
        return ;
    }
    mmFile=(char*)mmRet;
    close(srcFd);
    buffer.append("Content-length: "+std::to_string(mmFileStat.st_size)+"\r\n\r\n");
}

std::string HttpResponse::getFileType()
{
    size_t idx=path.find_last_of('.');
    if(idx==std::string::npos)
    {
        return "text/plain";
    }
    std::string suffix=path.substr(idx);
    if(SUFFIX_TYPE.count(suffix))
    {
        return SUFFIX_TYPE.find(suffix)->second;
    }
    return "text/plain";
} 

char* HttpResponse::getfile()
{
    return mmFile;
}

size_t HttpResponse::getFileLen()
{
    return mmFileStat.st_size;
}