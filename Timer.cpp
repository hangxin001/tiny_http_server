#include"Timer.h"
Timer* TimerManager::addTimer(const int& time, TimeOutFuction timeOutFun) {
	Timer* timer;
	{
		std::unique_lock<std::mutex> lock(_lock);
		updateTime();
		timer = new Timer(_nowTime + MS(time), timeOutFun);
		_mangerQueue.push(timer);
	}
	return timer;
}
void TimerManager::delTimer(Timer* timer) {	
	if (timer == nullptr) {
		return;
	}
	{
		std::unique_lock<std::mutex> lock(_lock);	//应该可以不用上锁，不过先上个锁看看
		timer->setUsed(false);
	}
}

void TimerManager::tick() {
	{
		std::unique_lock<std::mutex> lock(_lock);
		updateTime();
		if (_mangerQueue.empty())
			return;
		while (!_mangerQueue.empty()) {
			Timer* delTimer = _mangerQueue.top();
			if (!_mangerQueue.top()->isUsed()) {
				_mangerQueue.pop();
				delete delTimer;
				continue;
			}
			if (std::chrono::duration_cast<MS>(_mangerQueue.top()->getExpireTime() - _nowTime).count() > 0)   //没有超时
				return;

			_mangerQueue.top()->runTimeOutFunction();
			_mangerQueue.pop();
			delete delTimer;
		}
	}
}
int TimerManager::getExpireTime() {
	{
		std::unique_lock<std::mutex> lock(_lock);
		updateTime();
		int expireTime = 0;
		while (!_mangerQueue.empty() && !_mangerQueue.top()->isUsed()) {
			Timer* delTimer = _mangerQueue.top();
			_mangerQueue.pop();
			delete delTimer;
		}
		if (_mangerQueue.empty())
			return expireTime;

		expireTime = std::chrono::duration_cast<MS>(_mangerQueue.top()->getExpireTime() - _nowTime).count();
		expireTime < 0 ? 0 : expireTime;
		return expireTime;

	}
}
TimerManager::~TimerManager() {
	{
		std::unique_lock<std::mutex> lock(_lock);
		while (!_mangerQueue.empty()) {
			Timer* delTimer = _mangerQueue.top();
			_mangerQueue.pop();
		}
	}
}
