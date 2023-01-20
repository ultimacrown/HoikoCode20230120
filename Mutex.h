#pragma once

class Mutex {
public:
	Mutex();
	~Mutex();
	Mutex(const Mutex &) = delete;
	Mutex &operator=(const Mutex &) = delete;

	void Lock();
	void Unlock();
private:
	void *mutexHandle;
};

class LockGuard {
public:
	LockGuard(Mutex *mtx);
	~LockGuard();
	LockGuard(const LockGuard &) = delete;
	LockGuard &operator=(const LockGuard &) = delete;
private:
	Mutex *myMutex;
};