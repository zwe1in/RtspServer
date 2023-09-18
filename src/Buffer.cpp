#include "Buffer.h"
#include "SocketsOps.h"
#include <unistd.h>

const int Buffer::initialSize = 1024;
const char* Buffer::kCRLF = "\r\n";

int Buffer::read(int fd)
{
    char extraBuf[65536];
    struct iovec vec[2];
    const int writable = writableBytes();
    vec[0].iov_base = begin() + mWriteIndex;
    vec[0].iov_len = writable;
    vec[1].iov_base = extraBuf;
    vec[1].iov_len = sizeof(extraBuf);

    // 如果目前空间不够，就动用extraBuf
    const int iovcnt = (writable < sizeof(extraBuf)) ? 2 : 1;
    const int n = sockets::readv(fd, vec, iovcnt);

    if(n < 0)
        return -1;
    else if(n <= writable)
        mWriteIndex += n;
    else
    {
        mWriteIndex = mBufferSize;
        append(extraBuf, n - writable);
    }

    return n;
}

int Buffer::write(int fd)
{
    return sockets::write(fd, peek(), readableBytes());
}