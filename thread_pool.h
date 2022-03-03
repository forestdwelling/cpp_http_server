#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <pthread.h>
#include <vector>
#include <queue>

#define MAX_THREAD 64

class Task {
public:
    void (*func)(void *);
    void *args;
    Task(void (*func)(void *), void *args);
};

class ThreadPool {
public:
    pthread_mutex_t mutex;
    pthread_cond_t notify;
    bool on;
    std::vector<pthread_t> threads;
    std::queue<Task> tasks;
    int threadCount;
    int taskCount;
    int taskMax;
    ThreadPool(int threadNum, int taskMax);
    ~ThreadPool();
    void addTask(void (*func)(void *), void *args);
    static void *runThread(void *threadPool);
};

#endif