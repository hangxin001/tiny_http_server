#pragma once
#include<chrono>
#include<queue>
#include<functional>
#include<mutex>
#include<memory>
using TimeOutFuction = std::function<void()>;
using MS = std::chrono::milliseconds;
using Clock = std::chrono::high_resolution_clock;
using TimePoint = Clock::time_point;

class Timer {
public:
	Timer(const TimePoint &t , TimeOutFuction timeOutFun) : expireTime_(t), timeOutFuction_(timeOutFun),
			used_(true){};
	void setUsed(bool used) { used_ = used; };
	bool isUsed() const { return used_; };
	void runTimeOutFunction() const { timeOutFuction_(); };
	TimePoint getExpireTime() const { return expireTime_; };
	bool operator<(const Timer& a) {
		return (expireTime_ < a.getExpireTime());
	}
private:
	TimePoint expireTime_;
	TimeOutFuction timeOutFuction_;
	bool used_;  
};
struct TimerCmp {
	bool operator()(std::shared_ptr<Timer> a, std::shared_ptr<Timer> b) {
		return (a->getExpireTime() > b->getExpireTime());
	}
};
class TimerManager {
public:
	TimerManager() :nowTime_(Clock::now()) {};
	~TimerManager() = default;
	void updateTime() { nowTime_ = Clock::now(); };
	std::shared_ptr<Timer> addTimer(const int &time , TimeOutFuction timeOutFun);   //返回引用 还是指针 是个问题
	void delTimer(std::shared_ptr<Timer> timer); //因为优先队列只能删除顶部，使用惰性删除，减少开销,真正删除在tick()和getExpireTime()
	void tick();		//心跳函数
	int getExpireTime();  //获取超时时间

	//debug
	std::shared_ptr<Timer> getTop() { 
		if (mangerQueue_.empty())
			return nullptr;
		return mangerQueue_.top(); };

private:	
	std::priority_queue <std::shared_ptr<Timer>, std::vector<std::shared_ptr<Timer>>,TimerCmp> mangerQueue_;  //Timer重载<,生成最小堆
	TimePoint nowTime_;
	std::mutex lock_;		
};
