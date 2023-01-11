#include "buffer.h"

Buffer::Buffer(size_t initialSize):buffer(initialSize),readerIndex(0),writerIndex(0)
{
    assert(readableBytes() == 0);
    assert(writableBytes() == initialSize);
    assert(prependableBytes() == 0);
}

size_t Buffer::readableBytes() const
{
    return writerIndex - readerIndex;
}

size_t Buffer::writableBytes() const
{
    return buffer.size() - writerIndex;
}

size_t Buffer::prependableBytes() const
{
    return readerIndex;
}

const char* Buffer::peek() const
{
    return begin() + readerIndex;
}

char* Buffer::beginWrite()
{
    return begin() + writerIndex;
}

const char* Buffer::beginWrite() const
{
    return begin() + writerIndex;
}

void Buffer::ensureWriteableBytes(size_t len)
{
    if (writableBytes() < len)
    {
        makeSpace(len);
    }
    assert(writableBytes() >= len);
}

void Buffer::hasWritten(size_t len)
{
    writerIndex += len;
}

void Buffer::unwrite(size_t len)
{
    assert(len <= readableBytes());
    writerIndex -= len;
}

void Buffer::retrieve(size_t len)
{
    if(len<readableBytes())
    {
        readerIndex += len;
    }
    else
    {
        retrieveAll();
    }
}

void Buffer::retrieveUntil(const char* end)
{
    assert(peek() <= end);
    assert(end <= beginWrite());
    retrieve(end - peek());
}

void Buffer::retrieveAll()
{
    bzero(&buffer[0], buffer.size());
    readerIndex = 0;
    writerIndex = 0;
}

std::string Buffer::retrieveAllAsString()
{
    return retrieveAsString(readableBytes());
}

std::string Buffer::retrieveAsString(size_t len)
{
    assert(len <= readableBytes());
    std::string res(peek(), len);
    retrieve(len);
    return res;
}

void Buffer::append(const std::string& str)
{
    append(str.data(), str.length());
}

void Buffer::append(const char* str, size_t len)
{
    ensureWriteableBytes(len);
    std::copy(str, str + len, beginWrite());
    hasWritten(len);
}

void Buffer::append(const void* data, size_t len)
{
    append(static_cast<const char*>(data), len);
}

ssize_t Buffer::readFd(int fd, int* Errno)
{
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();
    //分散读，buffer内的 writable字节（堆区）+ 固定的 extrabuf（栈区）
    vec[0].iov_base = begin() + writerIndex;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    //如果writable已经很大了，就无需将第二块内存分配出去
    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    const ssize_t len = readv(fd, vec, iovcnt);
    if (len < 0)
    {
        *Errno = errno;
    }
    else if (static_cast<size_t>(len) <= writable)
    {
        writerIndex += len;
    }
    else
    {
        writerIndex = buffer.size();
        append(extrabuf, len - writable);
    }
    return len;
}