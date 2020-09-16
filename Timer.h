#pragma once
#include<chrono>
#include<queue>
#include<functional>
#include<mutex>

using TimeOutFuction = std::function<void()>;
using MS = std::chrono::milliseconds;
using Clock = std::chrono::high_resolution_clock;
using TimePoint = Clock::time_point;

class Timer {
public:
	Timer(const TimePoint &t , TimeOutFuction timeOutFun) : _expireTime(t), _timeOutFuction(timeOutFun),
			_used(true){};
	void setUsed(bool used) { _used = used; };
	bool isUsed() const { return _used; };
	void runTimeOutFunction() const { _timeOutFuction(); };
	TimePoint getExpireTime() const { return _expireTime; };
	bool operator<(const Timer& a) {
		return (_expireTime < a.getExpireTime());
	}
private:
	TimePoint _expireTime;
	TimeOutFuction _timeOutFuction;
	bool _used;  
};
struct TimerCmp {
	bool operator()(Timer *a, Timer *b) {
		return (a->getExpireTime() > b->getExpireTime());
	}
};
class TimerManager {
public:
	TimerManager() :_nowTime(Clock::now()) {};
	~TimerManager();
	void updateTime() { _nowTime = Clock::now(); };
	Timer* addTimer(const int &time , TimeOutFuction timeOutFun);   //返回引用 还是指针 是个问题
	void delTimer(Timer *timer); //因为优先队列只能删除顶部，使用惰性删除，减少开销,真正删除在tick()和getExpireTime()
	void tick();		//心跳函数
	int getExpireTime();  //获取超时时间

	/////debug/////
	Timer* getTop() { 
		if (!_mangerQueue.empty())
			return _mangerQueue.top();
		else
			return nullptr;
	};

private:	
	std::priority_queue <Timer *, std::vector<Timer *>,TimerCmp> _mangerQueue;  //Timer重载<,生成最小堆
	TimePoint _nowTime;
	std::mutex _lock;		
};