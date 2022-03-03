#include <sys/uio.h>
#include <stdio.h>
#include <errno.h>
#include <algorithm>

#include "Buffer.h"

const std::size_t Buffer::kInitialIndex = 8;
const std::size_t Buffer::kInitialSize = 1024;

Buffer::Buffer() :
    readIndex_(kInitialIndex),
    writeIndex_(kInitialIndex),
    data_(kInitialIndex + kInitialSize)
{}

Buffer::Buffer(const Buffer &rhs) : 
    readIndex_(rhs.readIndex_),
    writeIndex_(rhs.writeIndex_),
    data_(rhs.data_)
{}

Buffer &Buffer::operator=(const Buffer &rhs) {
    readIndex_ = rhs.readIndex_;
    writeIndex_ = rhs.writeIndex_;
    data_ = rhs.data_;
    return *this;
}

ssize_t Buffer::read(int fd) {
    char extraBuffer[65536];
    std::size_t writable = writableBytes();
    struct iovec iov[2];
    iov[0].iov_base = data_.data() + writeIndex_;
    iov[0].iov_len = writable;
    iov[1].iov_base = extraBuffer;
    iov[1].iov_len = sizeof extraBuffer;
    ssize_t n = readv(fd, iov, 2);
    if (n < 0) {
        printf("%s, errno = %d\n", __func__, errno);
    } else if (static_cast<std::size_t>(n) <= writable) {
        writeIndex_ += n;
    } else {
        writeIndex_ = data_.size();
        append(extraBuffer, n - writable);
    }
    return n;
}

void Buffer::append(const char *buf, std::size_t len) {
    std::size_t remaining = data_.capacity() - data_.size();
    if (remaining >= len) {
        emplaceBack(buf, buf + len);
    } else if (prependableBytes() + remaining >= len) {
        movingDataForward();
        if (writableBytes() >= len) {
            std::copy(buf, buf + len, data_.data() + writeIndex_);
            writeIndex_ += len;
        } else {
            std::copy(buf, buf + writableBytes(), data_.data() + writeIndex_);
            emplaceBack(buf + writableBytes(), buf + len);
        }
    } else {
        auto readable = readableBytes();
        std::vector<char> newData(kInitialIndex + readable + len);
        std::copy(first(), last(), newData.data() + kInitialIndex);
        std::copy(buf, buf + len, newData.data() + kInitialIndex + readable);
        std::swap(data_, newData);
        readIndex_ = kInitialIndex;
        writeIndex_ = data_.size();
    }
}