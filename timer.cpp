#include"timer.h"
#include<iostream>
std::shared_ptr<Timer> TimerManager::addTimer(const int& time, TimeOutFuction timeOutFun) {
	std::shared_ptr<Timer> timer = std::make_shared<Timer>(nowTime_ + MS(time), timeOutFun);
	{
		std::unique_lock<std::mutex> lock(lock_);
		updateTime();
		mangerQueue_.push(timer);
	}
	return timer;
}
void TimerManager::delTimer(std::shared_ptr<Timer> timer) {
	if (timer == nullptr) {
		return;
	}
	{
		//std::unique_lock<std::mutex> lock(lock_);	//上锁会卡死
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
			std::shared_ptr<Timer> delTimer = mangerQueue_.top();
			if (!mangerQueue_.top()->isUsed()) {
				mangerQueue_.pop();
				continue;
			}
			if (std::chrono::duration_cast<MS>(mangerQueue_.top()->getExpireTime() - nowTime_).count() > 0){   //没有超时
				return;
			}
			mangerQueue_.top()->runTimeOutFunction();
			mangerQueue_.pop();
		}
	}
}
int TimerManager::getExpireTime() {
	{
		std::unique_lock<std::mutex> lock(lock_);
		updateTime();
		int expireTime = 0;
		while (!mangerQueue_.empty() && !mangerQueue_.top()->isUsed()) {
			std::shared_ptr<Timer> delTimer = mangerQueue_.top();
			mangerQueue_.pop();
		}
		if (mangerQueue_.empty())
			return expireTime;

		expireTime = std::chrono::duration_cast<MS>(mangerQueue_.top()->getExpireTime() - nowTime_).count();
		expireTime < 0 ? 0 : expireTime;
		return expireTime;

	}
}

