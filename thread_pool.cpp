#include <pthread.h>
#include <vector>
#include <queue>
#include "base.h"
#include "thread_pool.h"

using namespace std;

Task::Task(void (*routine)(void *), void *args) {
    func = routine;
    this->args = args;
}

ThreadPool::ThreadPool(int threadNum, int taskMax) {
    if (threadNum > MAX_THREAD)
        errorExit("Create thread pool.");
    
    this->threadCount = threadNum;
    this->taskCount = 0;
    this->taskMax = taskMax;
    this->on = true;
    
    if (pthread_mutex_init(&this->mutex, NULL) != 0 || pthread_cond_init(&this->notify, NULL) != 0)
        errorExit("Create mutex.");
    
    for (int i = 0; i < threadNum; i++) {
        this->threads.emplace_back();
        if (pthread_create(&this->threads.back(), NULL, ThreadPool::runThread, (void *)this) != 0)
            errorExit("Create thread.");
    }
}

ThreadPool::~ThreadPool() {
    pthread_mutex_lock(&this->mutex);
    this->on = false;
    pthread_cond_broadcast(&this->notify);
    pthread_mutex_unlock(&this->mutex);
    for (int i = 0; i < this->threadCount; i++) {
        pthread_join(this->threads[i], NULL);
    }
    pthread_mutex_destroy(&this->mutex);
    pthread_cond_destroy(&this->notify);
}

void ThreadPool::addTask(void (*func)(void *), void *args) {
    pthread_mutex_lock(&this->mutex);
    if (this->taskCount == this->taskMax)
        errorExit("Tasks full.");
    this->tasks.emplace(func, args);
    this->taskCount++;
    if(pthread_cond_signal(&(this->notify)) != 0)
        errorExit("Cond signal.");
    pthread_mutex_unlock(&this->mutex);
}

void *ThreadPool::runThread(void *threadPool) {
    ThreadPool *pool = (ThreadPool *)threadPool;
    while (true) {
        pthread_mutex_lock(&pool->mutex);
        while (pool->taskCount == 0) {
            if (pool->on == false) {
                pthread_mutex_unlock(&(pool->mutex));
                pthread_exit(NULL);
            }
            pthread_cond_wait(&pool->notify, &pool->mutex);
        }
        Task task = pool->tasks.front();
        pool->tasks.pop();
        pool->taskCount--;
        pthread_mutex_unlock(&pool->mutex);
        task.func(task.args);
    }
}

// void test(void *threadPool) {
//     ThreadPool *pool = (ThreadPool *)threadPool;
//     cout << "======================\n";
// }

// #include <unistd.h>

// int main()
// {
//     ThreadPool pool(6, 5);
//     pool.addTask(test, &pool);
//     pool.addTask(test, &pool);
//     pool.addTask(test, &pool);
//     pool.addTask(test, &pool);
//     pool.addTask(test, &pool);
//     return 0;
// }