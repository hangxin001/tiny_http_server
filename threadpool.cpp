#include "threadpool.h"

ThreadPool::ThreadPool(int maxWork):_stop(false){
    maxWork <=0 ? maxWork = 1 : maxWork;
    for(int i=0 ; i < maxWork ; ++i){
        _worker.emplace_back([this](){
            for(;;){
                Function job;
                {
                    std::unique_lock<std::mutex> lock(_lock);  //构造加锁，系构解锁 RAII
                    while(!_stop && _jobs.empty())
                        _cond.wait(lock);                   //不满足则解锁，阻塞。防止假唤醒需要while判断
                    if(_stop && _jobs.empty())
                        return;
                    job = _jobs.front();
                    _jobs.pop_front();
                }
                job();
            }
        });
    }
}
ThreadPool::~ThreadPool(){
    {
        std::unique_lock<std::mutex> lock(_lock);   //上锁
        _stop = true;
    }
    _cond.notify_all();
    for(auto &job : _worker)
        job.join();

}
void ThreadPool::joinJob(Function job){
    {
        std::unique_lock<std::mutex> lock(_lock);
        _jobs.push_back(job);
    }
    _cond.notify_one();
}