#include "ThisThread.h"

__thread pid_t ThisThread::tid_ = 0;

pid_t ThisThread::tid() {
    if (tid_ == 0) {
        tid_ = syscall(SYS_gettid);
    }
    return tid_;
}