#pragma once

#define DLL_DECALRE

class DLL_DECALRE CCriticalSectionEx
{
public:
	//构造函数
	CCriticalSectionEx();
	//析构函数
	~CCriticalSectionEx();

	//进入临界区
	void Enter();
	//离开临界区
	void Leave();
private:
	CRITICAL_SECTION moSection;
};

class DLL_DECALRE CAutoLock
{
public:
	//构造函数
	CAutoLock(CCriticalSectionEx& aoSection);
	//析构函数
	~CAutoLock();
private:
	CCriticalSectionEx& moSection;
};



class DLL_DECALRE CEventEx
{
public:
	//构造函数
	CEventEx();
	//析构函数
	~CEventEx();

public:
	//创建事件
	BOOL Create(BOOL bManualReset, BOOL bInitialState);
	//等待事件
	int WaitForEvent(DWORD dwMilliseconds);
	//设置事件为有信号
	void SetEvent();
	//重新设置事件为无信号
	void ResetEvent();
	//关闭事件
	void Close();

private:
#ifdef WIN32
	//事件句柄
	HANDLE				mhEventHandle;
#else
	//为了防止竞争，条件变量的使用总是和一个互斥锁结合在一起。
	//Linux平台互斥结构体对象
	pthread_mutex_t		mhMutex;
	//Linux条件变量结构体对象
	pthread_cond_t		mhCond_t;
#endif
};