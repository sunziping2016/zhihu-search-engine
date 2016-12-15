//
// Created by sun on 11/28/16.
//

#ifndef HTML_PARSER_THREADPOOL_H
#define HTML_PARSER_THREADPOOL_H

// containers
#include "myvector.h"
#include "mylist.h"
// threading
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
// utility wrappers
#include <memory>
#include <functional>
// assert
#include <cassert>

class thread_pool {
public:
    // the constructor just launches some amount of workers
    thread_pool(size_t threads_n = std::thread::hardware_concurrency()) : stop(false)
    {
        assert(threads_n);
        for(; threads_n; --threads_n)
            this->workers.push_back(std::thread(
                    [this]
                    {
                        while(true)
                        {
                            std::function<void()> task;
                            {
                                std::unique_lock<std::mutex> lock(this->queue_mutex);
                                this->condition.wait(lock,
                                                     [this]{ return this->stop || !this->tasks.empty(); });
                                if(this->stop && this->tasks.empty())
                                    return;
                                task = std::move(this->tasks.front());
                                this->tasks.pop_front();
                            }

                            task();
                        }
                    }
            ));
    }
    // deleted copy&move ctors&assignments
    thread_pool(const thread_pool&) = delete;
    thread_pool& operator=(const thread_pool&) = delete;
    thread_pool(thread_pool&&) = delete;
    thread_pool& operator=(thread_pool&&) = delete;
    // add new work item to the pool
    template<class F, class... Args>
    std::future<typename std::result_of<F(Args...)>::type> enqueue(F&& f, Args&&... args)
    {
        using packaged_task_t = std::packaged_task<typename std::result_of<F(Args...)>::type ()>;

        std::shared_ptr<packaged_task_t> task(new packaged_task_t(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        ));
        auto res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(this->queue_mutex);
            this->tasks.push_back([task](){ (*task)(); });
        }
        this->condition.notify_one();
        return res;
    }
    // the destructor joins all threads
    virtual ~thread_pool()
    {
        this->stop = true;
        this->condition.notify_all();
        for(std::thread &worker : this->workers)
            worker.join();
    }
private:
    // need to keep track of threads so we can join them
    myvector<std::thread> workers;
    // the task queue
    mylist<std::function<void()> > tasks;

    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    // workers finalization flag
    std::atomic_bool stop;
};

#endif //HTML_PARSER_THREADPOOL_H
