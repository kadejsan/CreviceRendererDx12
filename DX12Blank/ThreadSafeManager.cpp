#include "stdafx.h"
#include "ThreadSafeManager.h"

using namespace std;

mutex ThreadSafeManager::STATICMUTEX;

ThreadSafeManager::ThreadSafeManager()
{
}

ThreadSafeManager::~ThreadSafeManager()
{
}

void ThreadSafeManager::LOCK()
{
	m_spinlock.lock();
}
bool ThreadSafeManager::TRY_LOCK()
{
	return m_spinlock.try_lock();
}
void ThreadSafeManager::UNLOCK()
{
	m_spinlock.unlock();
}

void ThreadSafeManager::LOCK_STATIC()
{
	STATICMUTEX.lock();
}
void ThreadSafeManager::UNLOCK_STATIC()
{
	STATICMUTEX.unlock();
}