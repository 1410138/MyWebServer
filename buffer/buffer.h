#ifndef BUFFER_H
#define BUFFER_H

#include <string>   
#include <string.h>
#include <vector> 
#include <assert.h>
#include <unistd.h>  
#include <sys/uio.h> 

class Buffer 
{
public:
    Buffer(size_t initialSize = 1024);
    ~Buffer() = default;

    size_t readableBytes() const;
    size_t writableBytes() const;
    size_t prependableBytes() const;

    const char* peek() const;
    char* beginWrite();
    const char* beginWrite() const;
    void ensureWriteableBytes(size_t len);
    void hasWritten(size_t len);
    void unwrite(size_t len);
    
    void retrieve(size_t len);
    void retrieveUntil(const char* end);
    void retrieveAll();
    std::string retrieveAllAsString();
    std::string retrieveAsString(size_t len);

    void append(const std::string& str);
    void append(const char* str, size_t len);
    void append(const void* data, size_t len);

    ssize_t readFd(int fd, int* Errno);
private:
    char* begin() {
        return &*buffer.begin();
    }

    //常量默认调用此重载函数
    const char* begin() const {
        return &*buffer.begin();
    }

    void makeSpace(size_t len)
    {
        //剩余空间不足len，则直接扩大buffer大小
        if (writableBytes() + prependableBytes() < len)
        {
            buffer.resize(writerIndex + len + 1);
        }
        else//剩余空间大于等于len，则移动可读数据至最前面，腾出空间
        {
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex, begin() + writerIndex, begin());
            readerIndex = 0;
            writerIndex = readerIndex + readable;
            assert(readable == readableBytes());
        }
    }

    std::vector<char> buffer;
    size_t readerIndex;
    size_t writerIndex;
};

#endif //BUFFER_H