#ifndef MBED_COMPAT_H
#define MBED_COMPAT_H

#ifdef NATIVE_TEST_MODE
    // Native模式下的Mbed兼容层
    #include <cstdio>
    #include <cstdlib>
    #include <cmath>
    #include <unistd.h>
    #include <sys/time.h>
    
    // 模拟Timer类
    class Timer {
    private:
        struct timeval startTime;
        bool started;
        
    public:
        Timer() : started(false) {}
        
        void start() {
            gettimeofday(&startTime, nullptr);
            started = true;
        }
        
        int read_ms() {
            if (!started) return 0;
            struct timeval currentTime;
            gettimeofday(&currentTime, nullptr);
            return (currentTime.tv_sec - startTime.tv_sec) * 1000 + 
                   (currentTime.tv_usec - startTime.tv_usec) / 1000;
        }
    };
    
    // 模拟thread_sleep_for
    inline void thread_sleep_for(int ms) {
        usleep(ms * 1000);
    }
    
    // 重定义printf以兼容（实际上不需要，但保持一致性）
    #define printf printf
    
#else
    // Mbed模式 - 使用真正的Mbed库
    #include <mbed.h>
#endif

#endif // MBED_COMPAT_H

