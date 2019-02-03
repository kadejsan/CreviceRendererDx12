#pragma once

#include <mutex>
#include "SpinLock.h"

class ThreadSafeManager
{
protected:
	//mutex MUTEX;
	static std::mutex STATICMUTEX;
	SpinLock m_spinlock;
public:
	ThreadSafeManager();
	~ThreadSafeManager();

	void LOCK();
	bool TRY_LOCK();
	void UNLOCK();

	static void LOCK_STATIC();
	static void UNLOCK_STATIC();
};

