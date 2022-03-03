#ifndef __SPIN_MUTEX_H__
#define __SPIN_MUTEX_H__

#include <atomic>
#include "NonCopyable.h"

class SpinMutex : NonCopyable {
public:
    SpinMutex() : flag_(ATOMIC_FLAG_INIT) {}

    void lock() {
        while (flag_.test_and_set(std::memory_order_acquire))
            ;
    }

    void unlock() {
        flag_.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag_;
};
#endif // !__SPIN_MUTEX_H__