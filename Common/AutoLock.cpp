#include "stdafx.h"
#include "AutoLock.h"


CCriticalSectionEx::CCriticalSectionEx()
{
	InitializeCriticalSection(&moSection);
}


//��������
CCriticalSectionEx::~CCriticalSectionEx()
{
	DeleteCriticalSection(&moSection);
}
//�����ٽ���
void CCriticalSectionEx::Enter()
{
	EnterCriticalSection(&moSection);
}

//�뿪�ٽ���
void CCriticalSectionEx::Leave()
{
	LeaveCriticalSection(&moSection);
}

//���캯��
CAutoLock::CAutoLock(CCriticalSectionEx& aoSection):moSection(aoSection)
{
	moSection.Enter();
}

//��������
CAutoLock::~CAutoLock()
{
	moSection.Leave();
}

//////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////








//���캯��
CEventEx::CEventEx()
{
#ifdef WIN32
	mhEventHandle = NULL;
#else

#endif
}

//��������
CEventEx::~CEventEx()
{
	this->Close();
}

//�����¼�
//Linux�²�֧���ֶ����÷�ʽ�ͳ�ʼ���¼�״̬��������������Ч
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

//�ȴ��¼�
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
		//ʹ�߳�������һ�����������Ļ������ϣ��������ȴ�
		pthread_mutex_lock(&mhMutex);
		pthread_cond_wait(&mhCond_t, &mhMutex);
		pthread_mutex_unlock(&mhMutex);
		return 0;
	}

	struct timeval now;      /*time when we started waiting*/ 
	struct timespec timeout; /*timeout value for the wait function */ 

	pthread_mutex_lock(&mhMutex);		//Lock
	//ȡ��ǰʱ��
	gettimeofday(&now, NULL); 
	//׼��ʱ����ֵ        
	// tv_usec -- ΢��(10E-6 second)��	// tv_nsec -- nano second(10E-9 second)��
	timeout.tv_sec = now.tv_sec + (( dwMilliseconds + now.tv_usec / 1000 ) / 1000 ); 
	timeout.tv_nsec = ((now.tv_usec / 1000 + dwMilliseconds) % 1000) * 1000000;        

	//ʹ�߳�������һ�����������Ļ������ϣ���ʱ�ȴ�
	int ldwResult = pthread_cond_timedwait(&mhCond_t, &mhMutex, &timeout);
	pthread_mutex_unlock(&mhMutex);		//UnLock
	if(ldwResult == ETIMEDOUT)
	{
		return -1;
	}
	return 0;
#endif    
}

//�����¼�Ϊ���ź�
void CEventEx::SetEvent()
{
#ifdef WIN32
	if (mhEventHandle)
	{
		::SetEvent(mhEventHandle);
	}
#else
	//�������б���������������mhCond_t�ϵ��̡߳�
	pthread_cond_broadcast(&mhCond_t);
#endif
}

//���������¼�Ϊ���ź�
void CEventEx::ResetEvent()
{
#ifdef WIN32
	if (mhEventHandle)
	{
		::ResetEvent(mhEventHandle);
	}
#endif
}

//�ر��¼�
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