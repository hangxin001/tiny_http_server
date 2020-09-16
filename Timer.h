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
	bool operator()(Timer a, Timer b) {
		return (a.getExpireTime() > b.getExpireTime());
	}
};
class TimerManager {
public:
	TimerManager() :_nowTime(Clock::now()) {};
	~TimerManager();
	void updateTime() { _nowTime = Clock::now(); };
	void addTimer(const int &time , TimeOutFuction timeOutFun);
	void delTimer(Timer &timer); //��Ϊ���ȶ���ֻ��ɾ��������ʹ�ö���ɾ�������ٿ���
	void tick();		//��������
	int getExpireTime();  //��ȡ��ʱʱ�䣬������ɾ������,���ص�λΪms

private:	
	std::priority_queue <Timer, std::vector<Timer>,TimerCmp> _mangerQueue;  //Timer����<,������С��
	TimePoint _nowTime;
	std::mutex _lock;		
};