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
    std::vector<std::thread> worker_;
    std::deque<Function> jobs_;
    std::mutex lock_;
    std::condition_variable cond_;
    bool stop_;


};

