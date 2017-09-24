// $_FILEHEADER_BEGIN ****************************
// 线程管理类, 简单线程池处理, 线程常用检测宏
// $_FILEHEADER_END ******************************

#pragma once

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#include <Windows.h>
#include <process.h>

#define THREAD_CHECK_EXIT(pThreadInfo) \
	if (pThreadInfo->bExit) return 0;

#define THREAD_LOOP_BEGIN(pThreadInfo) \
	for( ; pThreadInfo->bExit != true ; ) \
	{ \

#define THREAD_LOOP_END(pThreadInfo) \
	} \

#define THREAD_WAIT(pThreadInfo) \
	::WaitForSingleObject(pThreadInfo->hEvent, INFINITE); \
	if (pThreadInfo->bExit) return 0;

#define THREAD_CONTINUE(pThreadInfo) \
	if (pThreadInfo != NULL && pThreadInfo->hEvent != NULL) { ::SetEvent(pThreadInfo->hEvent); Sleep(1); }

#define THREAD_STOP(pThreadInfo) \
	if (pThreadInfo != NULL && pThreadInfo->hEvent != NULL) { pThreadInfo->bExit = true; ::SetEvent(pThreadInfo->hEvent); Sleep(1); }

namespace NSTHREADPOOL
{
    const long CONST_THREAD_POOL_INC_STEP = 10; // pool buff inc step, not the thread inc step, thread is inc one by one

	typedef struct _st_thread_info
	{
		bool	bExit;
		HANDLE	hEvent;
		WPARAM	wParam;
		LPARAM	lParam;
		void *	pFn;
	} STTHREADINFO, *LPTHREADINFO;

	typedef struct _st_thread_ctrl_info
	{
#ifdef _DEBUG
		unsigned short iIndex;
#endif
		bool	bExitPool;
		HANDLE	hEventPool;
		HANDLE  hThread;
		char *  pszDesc;
		STTHREADINFO threadInfo;
	} STTHREADCTRLINFO, *LPSTTHREADCTRLINFO;

	typedef long (__stdcall *PFN_ThreadCallBack)(STTHREADINFO * const pThreadInfo, WPARAM wParam, LPARAM lParam);
    inline unsigned int __stdcall ThreadProc( void * p );

    class CThreadPool
    {
        // static
    private:
        static CThreadPool * m_pThreadPool;

    public:
        static CThreadPool * const GetInstance()
        {
            if (m_pThreadPool == NULL)
            {
                m_pThreadPool = new CThreadPool;
                m_pThreadPool->_Init();
            }
            return m_pThreadPool;
        }

        static void Destory()
        {
            if (m_pThreadPool != NULL) { delete m_pThreadPool; m_pThreadPool = NULL; }
        }

        // members
    private:
        CThreadPool(void): m_lThreadCount(0L), m_hAryThreadPool(NULL), m_ppAryThreadCtrlInfo(NULL), m_pCS(NULL) {}
        ~CThreadPool(void) { _FreeMemory(); }

    private:

        long	  m_lThreadCount;
        HANDLE	* m_hAryThreadPool;
        STTHREADCTRLINFO ** m_ppAryThreadCtrlInfo;  // ** : increase buf, first copy then free
        CRITICAL_SECTION *	m_pCS;

    private:
        bool _Init()
        {
            if (m_pCS == NULL)
            {
                m_pCS = new CRITICAL_SECTION;
                if (!InitializeCriticalSectionAndSpinCount(m_pCS, 0x80000400))
                {
                    delete m_pCS;
                    m_pCS = NULL;
                    return false;
                }
            }

            if (m_ppAryThreadCtrlInfo == NULL)
            {
                m_ppAryThreadCtrlInfo = (STTHREADCTRLINFO **)malloc(sizeof(STTHREADCTRLINFO*) * CONST_THREAD_POOL_INC_STEP);
                memset(m_ppAryThreadCtrlInfo, 0x00, sizeof(STTHREADCTRLINFO*) * CONST_THREAD_POOL_INC_STEP);
            }

            if (m_hAryThreadPool == NULL)	
            {
                m_hAryThreadPool = (HANDLE*)malloc(sizeof(HANDLE) * CONST_THREAD_POOL_INC_STEP);
                memset(m_hAryThreadPool, 0x00, sizeof(HANDLE) * CONST_THREAD_POOL_INC_STEP);
            }
            return true;
        }

        void _FreeMemory()
        {
            if (m_ppAryThreadCtrlInfo == NULL || m_hAryThreadPool == NULL) return;

            STTHREADCTRLINFO * pCtrlInfo = NULL;
            for (long l = 0; l < m_lThreadCount; l++)
            {
                pCtrlInfo = m_ppAryThreadCtrlInfo[l];
                pCtrlInfo->bExitPool = true;
                pCtrlInfo->threadInfo.bExit = true;
                ::SetEvent(pCtrlInfo->threadInfo.hEvent);
                ::SetEvent(pCtrlInfo->hEventPool);
            }

            ::Sleep(1);
            ::WaitForMultipleObjects(m_lThreadCount, m_hAryThreadPool, TRUE, INFINITE);

            if (m_ppAryThreadCtrlInfo != NULL)
            {
                for (long l = 0; l < m_lThreadCount; l++)
                {
                    STTHREADCTRLINFO * pCtrlInfo = NULL;
                    pCtrlInfo = m_ppAryThreadCtrlInfo[l];;
                    if (pCtrlInfo->hThread != NULL) { ::CloseHandle(pCtrlInfo->hThread); }
                    if (pCtrlInfo->hEventPool != NULL) { ::CloseHandle(pCtrlInfo->hEventPool); }
                    if (pCtrlInfo->threadInfo.hEvent != NULL) { ::CloseHandle(pCtrlInfo->threadInfo.hEvent); }
                    if (pCtrlInfo->pszDesc != NULL) { free(pCtrlInfo->pszDesc); }
                    free(pCtrlInfo);
                }
                free(m_ppAryThreadCtrlInfo);
            }

            if (m_hAryThreadPool != NULL) { free(m_hAryThreadPool); }
            if (m_pCS != NULL) { ::DeleteCriticalSection(m_pCS); delete m_pCS; m_pCS = NULL; }
        }

        void _SetDescription(STTHREADCTRLINFO * const pThreadCtrlInfo, const char * const pszDescription)
        {
            if (pThreadCtrlInfo == NULL || pszDescription == NULL) return;
            if (pThreadCtrlInfo->pszDesc != NULL) free(pThreadCtrlInfo->pszDesc);
            long lLength = (long)strlen(pszDescription);
            pThreadCtrlInfo->pszDesc = (char*)malloc(lLength + 1);
            memcpy(pThreadCtrlInfo->pszDesc, pszDescription, lLength);
            pThreadCtrlInfo->pszDesc[lLength] = '\0';
        }

        void _IncreasePool()
        {
            if ( (m_lThreadCount > 0) && (m_lThreadCount % CONST_THREAD_POOL_INC_STEP == 0) )
            {
                unsigned char ucSizeofHandle = sizeof(HANDLE);
                long lNewCount = CONST_THREAD_POOL_INC_STEP * (m_lThreadCount / CONST_THREAD_POOL_INC_STEP + 1);

                // increase thread handle ary
                HANDLE * hAry = (HANDLE*)malloc(ucSizeofHandle * lNewCount);
                memcpy(hAry, m_hAryThreadPool, ucSizeofHandle * m_lThreadCount);
                memset(hAry + m_lThreadCount, 0x00, ucSizeofHandle * (lNewCount - m_lThreadCount));
                free(m_hAryThreadPool);
                m_hAryThreadPool = hAry;

                // increase thread ctrl info ary
                unsigned char ucSizeofPoint = sizeof(STTHREADCTRLINFO *);
                STTHREADCTRLINFO ** ppAry = (STTHREADCTRLINFO**)malloc(ucSizeofPoint * lNewCount);
                memcpy(ppAry, m_ppAryThreadCtrlInfo, ucSizeofPoint * m_lThreadCount);
                memset(ppAry + m_lThreadCount, 0x00, ucSizeofPoint * (lNewCount - m_lThreadCount));
                free(m_ppAryThreadCtrlInfo);
                m_ppAryThreadCtrlInfo = ppAry;		
            }
        }

        STTHREADCTRLINFO * const _GetIdleThread()
        {
            STTHREADCTRLINFO * pThreadCtrlInfo = NULL;
            STTHREADCTRLINFO * pCtrlInfo = NULL;
            ::EnterCriticalSection(m_pCS);
            for (long l = 0; l < m_lThreadCount; l++)
            {
                pCtrlInfo = m_ppAryThreadCtrlInfo[l];
                if (pCtrlInfo->threadInfo.pFn == NULL)
                {
                    pThreadCtrlInfo = pCtrlInfo;
                    break;
                }
            }

            if (pThreadCtrlInfo == NULL)
            {
                _IncreasePool();
                pThreadCtrlInfo = (STTHREADCTRLINFO*)malloc(sizeof(STTHREADCTRLINFO));
                memset(pThreadCtrlInfo, 0x00, sizeof(STTHREADCTRLINFO));
                m_ppAryThreadCtrlInfo[m_lThreadCount++] = pThreadCtrlInfo;
            }

            ::LeaveCriticalSection(m_pCS);
            return pThreadCtrlInfo;
        }

        long _CreateThread( STTHREADCTRLINFO * const pThreadCtrlInfo)
        {
            //assert(pThreadCtrlInfo != NULL);

            HANDLE hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
            if (hEvent == NULL) return GetLastError();

            HANDLE hEventPool = ::CreateEvent(NULL, FALSE, FALSE, NULL);
            if (hEventPool == NULL)
            {
                if (hEvent != NULL) { ::CloseHandle(hEvent); }
                return GetLastError();
            }

            DWORD dwThreadID = 0L;
            HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, NSTHREADPOOL::ThreadProc, pThreadCtrlInfo, CREATE_SUSPENDED, (unsigned int *)&dwThreadID);
            if (hThread == NULL)
            {
                if (hEvent != NULL) { ::CloseHandle(hEvent); }
                if (hEventPool != NULL) { ::CloseHandle(hEventPool); }
                return GetLastError();
            }

            pThreadCtrlInfo->hThread = hThread;
            pThreadCtrlInfo->hEventPool = hEventPool;
            pThreadCtrlInfo->threadInfo.hEvent = hEvent;
            m_hAryThreadPool[m_lThreadCount - 1] = hThread;

#ifdef _DEBUG
            pThreadCtrlInfo->iIndex = (unsigned short)(m_lThreadCount - 1);
#pragma warning(push)
#pragma warning(disable:4996)
            char szBuf[10];
            _snprintf(szBuf, 10, "num %d\r\n", m_lThreadCount);
            OutputDebugStringA(szBuf);
#pragma warning(pop)
#endif
            return 0L;
        }

    public:

        STTHREADINFO * const CreateThread(const char * const pszDescription, PFN_ThreadCallBack pFn, WPARAM wParam = NULL, LPARAM lParam = NULL, long nThreadLevel = THREAD_PRIORITY_NORMAL)
        {
            //assert(pszDescription != NULL && pFn != NULL);

            long lRet = 0L;
            STTHREADINFO * pRet = NULL;
            STTHREADCTRLINFO * const pThreadCtrlInfo = _GetIdleThread();
            _SetDescription(pThreadCtrlInfo, pszDescription);
            if (pThreadCtrlInfo->hThread == NULL)
            {
                lRet = _CreateThread(pThreadCtrlInfo);
            }

            if (lRet == 0L)
            {
                STTHREADINFO * const pThreadInfo = &pThreadCtrlInfo->threadInfo;
                pThreadInfo->pFn = pFn;
                pThreadInfo->wParam = wParam;
                pThreadInfo->lParam = lParam;
                pRet = pThreadInfo;
                SetThreadPriority( pThreadCtrlInfo->hThread, nThreadLevel );
                ::ResumeThread(pThreadCtrlInfo->hThread);
                ::SetEvent(pThreadCtrlInfo->hEventPool);
                Sleep(1);
            }
            return pRet;
        }

        void RecycleThread(STTHREADCTRLINFO * const pThreadCtrlInfo)
        {
            if (pThreadCtrlInfo == NULL) return;
            STTHREADINFO * const pThreadInfo = (STTHREADINFO * const)&pThreadCtrlInfo->threadInfo;
            ::EnterCriticalSection(m_pCS);
            // clear data for recycle
            if (pThreadCtrlInfo->pszDesc != NULL) { free(pThreadCtrlInfo->pszDesc); pThreadCtrlInfo->pszDesc = NULL; }
            pThreadInfo->pFn = NULL;
            pThreadInfo->wParam = NULL;
            pThreadInfo->lParam = NULL;
            pThreadInfo->bExit = false;
            ::LeaveCriticalSection(m_pCS);
        }

        CRITICAL_SECTION * const GetCriticalSection() { return m_pCS; }
    };

    inline unsigned int __stdcall ThreadProc( void * p )
    {
        STTHREADCTRLINFO * const pThreadCtrlInfo = (STTHREADCTRLINFO * const)p;
        STTHREADINFO * const pThreadInfo = (STTHREADINFO * const)&pThreadCtrlInfo->threadInfo;
        CThreadPool * const pThManager = CThreadPool::GetInstance();
        while(true)
        {
            WaitForSingleObject(pThreadCtrlInfo->hEventPool, INFINITE);
            if (pThreadCtrlInfo->bExitPool) break;

            if (pThreadInfo->pFn != NULL)
            {
                long lRet = ((PFN_ThreadCallBack)pThreadInfo->pFn)(pThreadInfo, pThreadInfo->wParam, pThreadInfo->lParam);
            }

            // clear data for recycle
            pThManager->RecycleThread(pThreadCtrlInfo);
        }

        //_endthreadex(0);
        return 0;
    }
    
    __declspec(selectany) CThreadPool * CThreadPool::m_pThreadPool = NULL;
};

