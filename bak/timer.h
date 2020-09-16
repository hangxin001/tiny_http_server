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
	Timer(const TimePoint &t , TimeOutFuction timeOutFun) : expireTime_(t), timeOutFuction_(timeOutFun),
			_used(true){};
	void setUsed(bool used) { _used = used; };
	bool isUsed() const { return _used; };
	void runTimeOutFunction() const { timeOutFuction_(); };
	TimePoint getExpireTime() const { return expireTime_; };
	bool operator<(const Timer& a) {
		return (expireTime_ < a.getExpireTime());
	}
private:
	TimePoint expireTime_;
	TimeOutFuction timeOutFuction_;
	bool _used;  
};
struct TimerCmp {
	bool operator()(Timer *a, Timer *b) {
		return (a->getExpireTime() > b->getExpireTime());
	}
};
class TimerManager {
public:
	TimerManager() :nowTime_(Clock::now()) {};
	~TimerManager();
	void updateTime() { nowTime_ = Clock::now(); };
	Timer* addTimer(const int &time , TimeOutFuction timeOutFun);   //�������� ����ָ�� �Ǹ�����
	void delTimer(Timer *timer); //��Ϊ���ȶ���ֻ��ɾ��������ʹ�ö���ɾ�������ٿ���,����ɾ����tick()��getExpireTime()
	void tick();		//��������
	int getExpireTime();  //��ȡ��ʱʱ��



private:	
	std::priority_queue <Timer *, std::vector<Timer *>,TimerCmp> mangerQueue_;  //Timer����<,������С��
	TimePoint nowTime_;
	std::mutex lock_;		
};