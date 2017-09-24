#pragma once

#define DLL_DECALRE

class DLL_DECALRE CCriticalSectionEx
{
public:
	//���캯��
	CCriticalSectionEx();
	//��������
	~CCriticalSectionEx();

	//�����ٽ���
	void Enter();
	//�뿪�ٽ���
	void Leave();
private:
	CRITICAL_SECTION moSection;
};

class DLL_DECALRE CAutoLock
{
public:
	//���캯��
	CAutoLock(CCriticalSectionEx& aoSection);
	//��������
	~CAutoLock();
private:
	CCriticalSectionEx& moSection;
};



class DLL_DECALRE CEventEx
{
public:
	//���캯��
	CEventEx();
	//��������
	~CEventEx();

public:
	//�����¼�
	BOOL Create(BOOL bManualReset, BOOL bInitialState);
	//�ȴ��¼�
	int WaitForEvent(DWORD dwMilliseconds);
	//�����¼�Ϊ���ź�
	void SetEvent();
	//���������¼�Ϊ���ź�
	void ResetEvent();
	//�ر��¼�
	void Close();

private:
#ifdef WIN32
	//�¼����
	HANDLE				mhEventHandle;
#else
	//Ϊ�˷�ֹ����������������ʹ�����Ǻ�һ�������������һ��
	//Linuxƽ̨����ṹ�����
	pthread_mutex_t		mhMutex;
	//Linux���������ṹ�����
	pthread_cond_t		mhCond_t;
#endif
};