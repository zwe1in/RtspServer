#ifndef BUFFER_H
#define BUFFER_H
#include <stdlib.h>
#include <algorithm>
#include <assert.h>

class Buffer{
public:
    static const int initialSize;

    explicit Buffer() 
        : mBufferSize(initialSize), mReadIndex(0), mWriteIndex(0)
    {
        mBuffer = (char*)malloc(mBufferSize);
    }

    ~Buffer()
    {
        free(mBuffer);
    }

    int readableBytes() const { return mWriteIndex - mReadIndex; }
    int writableBytes() const { return mBufferSize - mWriteIndex; }
    int prependableBytes() const { return mReadIndex; }
    char* peek() { return begin() + mReadIndex; }
    const char* peek() const { return begin() + mReadIndex; }

    const char* findCRLF() const
    {
        const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
        return crlf == beginWrite() ? NULL : crlf;
    }

    const char* findCRLF(const char* start) const
    {
        assert(peek() <= start);
        assert(start < beginWrite());
        const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
        return crlf == beginWrite() ? NULL : crlf;
    }

    const char* findLastCRLF() const
    {
        const char* crlf = std::find_end(peek(), beginWrite(), kCRLF, kCRLF + 2);
        return crlf == beginWrite() ? NULL : crlf;
    }

    void retrieve(int len)
    {
        assert(len <= readableBytes());
        if(len < readableBytes())
            mReadIndex += len;
        else
            retrieveAll();
    }

    void retrieveAll()
    {
        mReadIndex = 0;
        mWriteIndex = 0;
    }

    void retrieveUntil(const char* end)
    {
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(end - peek());
    }

    char* beginWrite() { return begin() + mWriteIndex; }
    const char* beginWrite() const { return begin() + mWriteIndex; }

    void hasWritten(int len)
    {
        assert(len <= writableBytes());
        mWriteIndex += len;
    }

    void unwrite(int len)
    {
        assert(len <= readableBytes());
        mWriteIndex -= len;
    }

    void ensureWritableBytes(int len)
    {
        if(writableBytes() < len)
            makeSpace(len);
        assert(writableBytes() >= len);
    }

    void makeSpace(int len)
    {
        if(writableBytes() + prependableBytes() < len)
        {
            // 往后开拓空间
            mBufferSize = mWriteIndex + len;
            mBuffer = (char*)realloc(mBuffer, mBufferSize);
        }
        else
        {
            // 复用前面的空间
            int readable = readableBytes();
            std::copy(begin() + mReadIndex, begin() + mWriteIndex, begin());
            mReadIndex = 0;
            mWriteIndex = readable;
            assert(readable == readableBytes());
        }
    }

    void append(const char* data, int len)
    {
        ensureWritableBytes(len);
        std::copy(data, data + len , beginWrite());
        hasWritten(len);
    }

    void append(const void* data, int len)
    {
        append((const char*)(data), len);
    }

    int read(int fd);
    int write(int fd);
private:
    char* begin(){ return mBuffer; }
    const char* begin() const { return mBuffer; }
private:
    char* mBuffer;
    int mBufferSize;
    int mReadIndex;
    int mWriteIndex;

    static const char* kCRLF;
};

#endif