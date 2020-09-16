#include"Timer.h"

void TimerManager::addTimer(const int& time, TimeOutFuction timeOutFun) {
	{
		std::unique_lock<std::mutex> lock(_lock);
		updateTime();
		_mangerQueue.push(Timer(_nowTime + MS(time), timeOutFun));

	}
}
void TimerManager::delTimer(Timer& timer) {	
	{
		std::unique_lock<std::mutex> lock(_lock);	//应该可以不用上锁，不过先上个锁看看
		timer.setUsed(false);
	}
}

void TimerManager::tick() {
	{
		std::unique_lock<std::mutex> lock(_lock);
		updateTime();
		if (_mangerQueue.empty())
			return;
		while (!_mangerQueue.empty()) {
			if (!_mangerQueue.top().isUsed()) {
				_mangerQueue.pop();
				continue;
			}
			if (std::chrono::duration_cast<MS>(_mangerQueue.top().getExpireTime() - _nowTime).count() > 0)   //没有超时
				return;

			_mangerQueue.top().runTimeOutFunction();
			_mangerQueue.pop();
		}
	}
}
int TimerManager::getExpireTime() {
	{
		std::unique_lock<std::mutex> lock(_lock);
		updateTime();
		int expireTime = 0;
		while (!_mangerQueue.empty() && !_mangerQueue.top().isUsed()) {
			_mangerQueue.pop();
		}
		if (_mangerQueue.empty())
			return expireTime;

		expireTime = std::chrono::duration_cast<MS>(_mangerQueue.top().getExpireTime() - _nowTime).count();
		expireTime < 0 ? 0 : expireTime;
		return expireTime;

	}
}
TimerManager::~TimerManager() {
	while (!_mangerQueue.empty())
		_mangerQueue.pop();
}
