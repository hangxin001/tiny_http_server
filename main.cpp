#include<iostream>
#include<queue>
#include<unistd.h>
#include"Timer.h"
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
int main() {
	using namespace std;
	TimerManager T;
	T.addTimer(1000, testFun1);
	T.addTimer(3000, testFun2);
	T.addTimer(5000, testFun3);
	T.addTimer(7000, testFun4);
	TimePoint nowTime = Clock::now();
	while (1) {
		T.getExpireTime();
		T.tick();
		if (chrono::duration_cast<MS>(Clock::now() - nowTime).count() > 10000)
			break;

	}
	cout << "10s gone" << endl;
}