#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <regex>
#include <algorithm>
#include <memory>
#include <mysql/mysql.h>

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../pool/sqlconnpool.h"
using std::string;

class HttpRequest
{
public:
    enum PARSE_STATE
    {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH
    };
    HttpRequest();
    ~HttpRequest()=default;

    void init();
    bool parse(Buffer& buffer);
    const string& getMethod() const;
    const string& getPath() const;
    const string& getVersion() const;
    bool isKeepAlive() const;

private:
    void parseRequestLine(const string& line);
    void parseHeader(const string& line);
    void parseBody(const string& line);

    void parsePath();
    void parsePost();
    void parseUrlencoded();
    bool userVertify(const string& username,const string& password,int tag);
    PARSE_STATE state;
    string method;
    string path;
    string version;
    string body;
    std::unordered_map<string,string> header;
    std::unordered_map<string,string> post; 
    static const std::unordered_set<string> DEFAULT_HTML;
    static const std::unordered_map<string,int> DEFAULT_HTML_TAG;
};

#endif // !HTTPREQUEST_H