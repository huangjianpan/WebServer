#ifndef __NON_COPYABLE_H__
#define __NON_COPYABLE_H__

class NonCopyable {
public:
    NonCopyable(const NonCopyable &rhs) = delete;
    NonCopyable &operator=(const NonCopyable &rhs) = delete;

protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
};
#endif // !__NON_COPYABLE_H__