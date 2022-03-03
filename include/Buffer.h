#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <vector>
#include <string>
#include <assert.h>

class Buffer {
public:
    explicit Buffer();

    Buffer(const Buffer &rhs);

    ~Buffer() = default;

    Buffer &operator=(const Buffer &rhs);

    ssize_t read(int fd);
    
    std::size_t writableBytes() const { return data_.size() - writeIndex_; }

    std::size_t readableBytes() const { return writeIndex_ - readIndex_; }

    std::size_t prependableBytes() const { return readIndex_ - kInitialIndex; }

    const char *first() const { return data_.data() + readIndex_; }

    const char *last() const { return data_.data() + writeIndex_; }

    void retrieveAll() { readIndex_ = writeIndex_ = kInitialIndex; }

    void retrieve(std::size_t n) {
        assert(readIndex_ + n <= writeIndex_);
        readIndex_ += n;
        if (readIndex_ == writeIndex_) {
            retrieveAll();
        }
    }

    std::string retrieveAsString() {
        std::string s(first(), last());
        retrieveAll();
        return s;
    }

    void append(const char *buf, std::size_t len);
private:
    void movingDataForward() {
        auto readable = readableBytes();
        std::copy(first(), last(), data_.data() + kInitialIndex);
        readIndex_ = kInitialIndex;
        writeIndex_ = readIndex_ + readable;
    }

    void emplaceBack(const char *first, const char *last) {
        while (first != last) {
            data_.emplace_back(*first);
            ++first;
        }
        writeIndex_ = data_.size();
    }

    std::size_t readIndex_;
    std::size_t writeIndex_;
    std::vector<char> data_;

    static const std::size_t kInitialIndex;
    static const std::size_t kInitialSize;
};
#endif // !__BUFFER_H__