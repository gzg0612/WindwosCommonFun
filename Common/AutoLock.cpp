#include "stdafx.h"
#include "AutoLock.h"


CCriticalSectionEx::CCriticalSectionEx()
{
	InitializeCriticalSection(&moSection);
}


//析构函数
CCriticalSectionEx::~CCriticalSectionEx()
{
	DeleteCriticalSection(&moSection);
}
//进入临界区
void CCriticalSectionEx::Enter()
{
	EnterCriticalSection(&moSection);
}

//离开临界区
void CCriticalSectionEx::Leave()
{
	LeaveCriticalSection(&moSection);
}

//构造函数
CAutoLock::CAutoLock(CCriticalSectionEx& aoSection):moSection(aoSection)
{
	moSection.Enter();
}

//析构函数
CAutoLock::~CAutoLock()
{
	moSection.Leave();
}

//////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////








//构造函数
CEventEx::CEventEx()
{
#ifdef WIN32
	mhEventHandle = NULL;
#else

#endif
}

//析构函数
CEventEx::~CEventEx()
{
	this->Close();
}

//创建事件
//Linux下不支持手动设置方式和初始化事件状态，这两个参数无效
BOOL CEventEx::Create(BOOL abManualReset, BOOL abInitialState)
{
#ifdef WIN32
	mhEventHandle = CreateEvent(NULL, abManualReset, abInitialState, NULL);
#else    
	mhMutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_init(&mhMutex, NULL);
	pthread_cond_init(&mhCond_t, NULL); 
#endif
	return TRUE;
}

//等待事件
int CEventEx::WaitForEvent(DWORD dwMilliseconds)
{
#ifdef WIN32
	DWORD ldwResult = WaitForSingleObject(mhEventHandle, dwMilliseconds);

	if (ldwResult == WAIT_OBJECT_0)
	{
		return 0;
	}
	else if (ldwResult == WAIT_TIMEOUT)
	{
		return -1;
	}

	return -2;

#else
	if (dwMilliseconds == (DWORD)-1)
	{
		//使线程阻塞在一个条件变量的互斥锁上，无条件等待
		pthread_mutex_lock(&mhMutex);
		pthread_cond_wait(&mhCond_t, &mhMutex);
		pthread_mutex_unlock(&mhMutex);
		return 0;
	}

	struct timeval now;      /*time when we started waiting*/ 
	struct timespec timeout; /*timeout value for the wait function */ 

	pthread_mutex_lock(&mhMutex);		//Lock
	//取当前时间
	gettimeofday(&now, NULL); 
	//准备时间间隔值        
	// tv_usec -- 微秒(10E-6 second)，	// tv_nsec -- nano second(10E-9 second)，
	timeout.tv_sec = now.tv_sec + (( dwMilliseconds + now.tv_usec / 1000 ) / 1000 ); 
	timeout.tv_nsec = ((now.tv_usec / 1000 + dwMilliseconds) % 1000) * 1000000;        

	//使线程阻塞在一个条件变量的互斥锁上，计时等待
	int ldwResult = pthread_cond_timedwait(&mhCond_t, &mhMutex, &timeout);
	pthread_mutex_unlock(&mhMutex);		//UnLock
	if(ldwResult == ETIMEDOUT)
	{
		return -1;
	}
	return 0;
#endif    
}

//设置事件为有信号
void CEventEx::SetEvent()
{
#ifdef WIN32
	if (mhEventHandle)
	{
		::SetEvent(mhEventHandle);
	}
#else
	//唤醒所有被阻塞在条件变量mhCond_t上的线程。
	pthread_cond_broadcast(&mhCond_t);
#endif
}

//重新设置事件为无信号
void CEventEx::ResetEvent()
{
#ifdef WIN32
	if (mhEventHandle)
	{
		::ResetEvent(mhEventHandle);
	}
#endif
}

//关闭事件
void CEventEx::Close()
{
#ifdef WIN32
	if (mhEventHandle != NULL)
	{
		CloseHandle(mhEventHandle);
		mhEventHandle = NULL;
	}
#else
	pthread_mutex_destroy(&mhMutex);
	pthread_cond_destroy(&mhCond_t);
#endif
}