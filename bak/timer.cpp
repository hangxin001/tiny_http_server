#include"timer.h"
Timer* TimerManager::addTimer(const int& time, TimeOutFuction timeOutFun) {
	Timer* timer;
	{
		std::unique_lock<std::mutex> lock(lock_);
		updateTime();
		timer = new Timer(nowTime_ + MS(time), timeOutFun);
		mangerQueue_.push(timer);
	}
	return timer;
}
void TimerManager::delTimer(Timer* timer) {	
	if (timer == nullptr) {
		return;
	}
	{
		std::unique_lock<std::mutex> lock(lock_);	//应该可以不用上锁，不过先上个锁看看
		timer->setUsed(false);
	}
}

void TimerManager::tick() {
	{
		std::unique_lock<std::mutex> lock(lock_);
		updateTime();
		if (mangerQueue_.empty())
			return;
		while (!mangerQueue_.empty()) {
			Timer* delTimer = mangerQueue_.top();
			if (!mangerQueue_.top()->isUsed()) {
				mangerQueue_.pop();
				delete delTimer;
				continue;
			}
			if (std::chrono::duration_cast<MS>(mangerQueue_.top()->getExpireTime() - nowTime_).count() > 0)   //没有超时
				return;

			mangerQueue_.top()->runTimeOutFunction();
			mangerQueue_.pop();
			delete delTimer;
		}
	}
}
int TimerManager::getExpireTime() {
	{
		std::unique_lock<std::mutex> lock(lock_);
		updateTime();
		int expireTime = 0;
		while (!mangerQueue_.empty() && !mangerQueue_.top()->isUsed()) {
			Timer* delTimer = mangerQueue_.top();
			mangerQueue_.pop();
			delete delTimer;
		}
		if (mangerQueue_.empty())
			return expireTime;

		expireTime = std::chrono::duration_cast<MS>(mangerQueue_.top()->getExpireTime() - nowTime_).count();
		expireTime < 0 ? 0 : expireTime;
		return expireTime;

	}
}
TimerManager::~TimerManager() {
	{
		std::unique_lock<std::mutex> lock(lock_);
		while (!mangerQueue_.empty()) {
			Timer* delTimer = mangerQueue_.top();
			mangerQueue_.pop();
		}
	}
}
