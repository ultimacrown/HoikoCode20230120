#include "Mutex.h"

#include "framework.h"

Mutex::Mutex() {
	this->mutexHandle = CreateMutexW(nullptr, false, nullptr);
}
Mutex::~Mutex() {
	CloseHandle(this->mutexHandle);
}
void Mutex::Lock() {
	WaitForSingleObject(this->mutexHandle, INFINITE);
}
void Mutex::Unlock() {
	ReleaseMutex(this->mutexHandle);
}

LockGuard::LockGuard(Mutex *mtx) : myMutex(mtx) {
	this->myMutex->Lock();
}
LockGuard::~LockGuard() {
	this->myMutex->Unlock();
}
