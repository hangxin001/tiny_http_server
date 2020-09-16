#pragma once
#include<vector>
#include<deque>
#include<mutex>
#include<condition_variable>
#include<functional>
#include<thread>
class ThreadPool{
public:
    using Function = std::function<void()>;
    ThreadPool(int maxWork);
    ~ThreadPool();
    void joinJob(Function job);   //bind封装函数参数

private:
    std::vector<std::thread> _worker;
    std::deque<Function> _jobs;
    std::mutex _lock;
    std::condition_variable _cond;
    bool _stop;


};

