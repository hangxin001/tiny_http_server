#include "threadpool.h"

ThreadPool::ThreadPool(int maxWork):stop_(false){
    maxWork <=0 ? maxWork = 1 : maxWork;
    for(int i=0 ; i < maxWork ; ++i){
        worker_.emplace_back([this](){
            for(;;){
                Function job;
                {
                    std::unique_lock<std::mutex> lock(lock_);  //构造加锁，系构解锁 RAII
                    while (!stop_ && jobs_.empty())
                        cond_.wait(lock);              //不满足则解锁，阻塞。防止假唤醒需要while判断
                    if(stop_ && jobs_.empty())
                        return;
                    job = jobs_.front();
                    jobs_.pop_front();
                }
                job();
            }
        });
    }
}
ThreadPool::~ThreadPool(){
    {
        std::unique_lock<std::mutex> lock(lock_);   //上锁
        stop_ = true;
    }
    cond_.notify_all();
    for(auto &job : worker_)
        job.join();

}
void ThreadPool::joinJob(Function job){
    {
        std::unique_lock<std::mutex> lock(lock_);
        jobs_.push_back(job);
    }
    cond_.notify_one();
}