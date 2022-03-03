#ifndef __THIS_THREAD_H__
#define __THIS_THREAD_H__

#include <unistd.h>
#include <sys/syscall.h>

class ThisThread {
public:
    ThisThread() = delete;
    static pid_t tid();
    
private:
    static __thread pid_t tid_;
};
#endif // !__THIS_THREAD_H__