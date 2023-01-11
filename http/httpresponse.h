#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <unordered_map>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "../log/log.h"
#include "../buffer/buffer.h"

class HttpResponse
{
public:
    HttpResponse();
    ~HttpResponse();
    void init(const std::string& srcDir_,const std::string& path_,bool isKeepAlive_, int code_);
    void makeResponse(Buffer& buffer);
    void unmapFile();
    char* getfile();
    size_t getFileLen();

private:
    void addStateLine(Buffer& buffer);
    void addHeader(Buffer& buffer);
    void addContent(Buffer& buffer);
    
    void errorHtml();
    std::string getFileType();

    int code;
    bool isKeepAlive;
    std::string path;
    std::string srcDir;
    char* mmFile;
    struct stat mmFileStat;

    static const std::unordered_map<std::string,std::string> SUFFIX_TYPE;
    static const std::unordered_map<int,std::string> CODE_STATUS;
    static const std::unordered_map<int,std::string> CODE_PATH;
};


#endif // !HTTPRESPONSE_H