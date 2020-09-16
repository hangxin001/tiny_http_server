#include<iostream>
#include<functional>
#include<queue>
#include"Timer.h"
#include"ThreadPool.h"
#include<unistd.h>
///////////////////This is test file/////////////////
///////////////////Test git and vs2019//////////////
void testFun1() {
	std::cout << "fun1" << std::endl;
}
void testFun2() {
	std::cout << "fun2" << std::endl;
}
void testFun3() {
	std::cout << "fun3" << std::endl;
}
void testFun4() {
	std::cout << "fun4" << std::endl;
}
void timerAddThreadPool(TimerManager* T,int time ,std::function<void()> fun) {
	T->addTimer(time, fun);
}
int main() {
	using namespace std;
	
	TimerManager T ;
	ThreadPool pool(4);
	pool.joinJob(bind(& TimerManager::addTimer, &T ,1000,testFun1));
	pool.joinJob(bind(&TimerManager::addTimer, &T, 3000, testFun2));
	pool.joinJob(bind(&TimerManager::addTimer, &T, 5000, testFun3));
	//while(1)
	//	T.tick();
	//pool.joinJob(testFun1);
	cout << "now start" << endl;
	sleep(5000);
}