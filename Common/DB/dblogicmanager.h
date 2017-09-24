#pragma once
#include <vector>
#include <map>
#include "..\..\Common\Pool\ThreadPool.h"
#include "..\..\zthird\SQLiteKey\CppSQLite3.h"
using namespace NSTHREADPOOL;

static const BYTE INDEX_BIND_START = 1;

#ifdef _DEBUG
#define BEGINTICKCOUNT long lTickCount = ::GetTickCount();
#define ENDTICKCOUNT ::GetTickCount() - lTickCount
#define MSGBOX(title, msg) ::MessageBoxA(NULL, msg, title, NULL);
#else
#define BEGINTICKCOUNT ;
#define ENDTICKCOUNT 0
#define MSGBOX(title, msg) //WriteLog1(msg);
#endif

// 数据库操作宏
#define SQLBEGIN(pDB, lRet)\
	long lRet = SQLITE_OK;	\
	BEGINTICKCOUNT \
	try { pDB->execDML(" begin transaction; ");\


#define SQLEND(pDB)\
	pDB->execDML(" commit transaction; "); } catch (CppSQLite3Exception& e) { \
	pDB->execDML(" ROLLBACK; "); lRet = -1L; \
	OutputDebugStringA(__FUNCTION__); OutputDebugStringA(e.errorMessage()); MSGBOX(__FUNCTION__, e.errorMessage()); } \
	ATLTRACE("function: %s, sql time: %d\r\n", __FUNCTION__, ENDTICKCOUNT);\

	
#define SQLQUERY_BEGIN(lRet)\
	long lRet = SQLITE_OK;	\
	BEGINTICKCOUNT \
	try { \


#define SQLQUERY_END()\
	} catch (CppSQLite3Exception& e) { \
	lRet = -1L; \
	OutputDebugStringA(__FUNCTION__); OutputDebugStringA(e.errorMessage()); MSGBOX(__FUNCTION__, e.errorMessage()); } \
	ATLTRACE("function: %s, sql time: %d\r\n", __FUNCTION__, ENDTICKCOUNT);\



// 数据库线程处理函数
inline long __stdcall OnThreadDB(STTHREADINFO * const pThreadInfo, WPARAM wParam, LPARAM lParam);

// ADDTASK宏, 检测是否调用AddTask, AddTask参数 wParam, lParam 等来自调用该宏的函数申明
#define ADDTASK(pfn) if (bAddThreadTask) { AddTask((PFN_OnDBCmd)pfn, wParam, lParam, lPriority, pSyncRetData, pSyncRetDataLen); return; }

// 数据库处理函数指针
class CDBLogicManager;
typedef long (CDBLogicManager::*PFN_OnDBCmd)(const bool bAddThreadTask, WPARAM wParam, LPARAM lParam, long lPriority, OUT void ** pSyncRetData, OUT long * pSyncRetDataLen);

class CDBLogicManager
{
public:
    CDBLogicManager() : m_bDBEnable(true), m_hEventSync(NULL), m_pCS(NULL), m_pVecTask(NULL), m_pThreadInfo(NULL), m_pDB(NULL) {}
    ~CDBLogicManager(void) { _FreeMem(); }

private:

    bool    m_bDBEnable;
    HANDLE  m_hEventSync;

    typedef std::vector<void*> VecTask;
    VecTask             *	m_pVecTask;
	CRITICAL_SECTION    *	m_pCS;
	NSTHREADPOOL::STTHREADINFO	*	m_pThreadInfo;

protected:
    CppSQLite3DB        *   m_pDB;

private:

    typedef struct _stru_db_task
    {
        PFN_OnDBCmd pfn;
        long	    lPriority;
        WPARAM	    wParam;
        LPARAM	    lParam;
        void    *   pSyncRetData;
        long        lSyncRetDataLen;
    } STDBTASK, *LPSTDBTASK;

private:

    void _FreeMem()
    {
        if ( m_hEventSync != NULL )
        {
            ::SetEvent(m_hEventSync);
            ::CloseHandle(m_hEventSync);
        }

        if ( m_pCS != NULL )
        {
            DeleteCriticalSection(m_pCS);
            delete m_pCS;
        }

        if ( m_pVecTask != NULL )
        {
            _FreeAllTask();
            std::vector<void*>().swap(*m_pVecTask);
            delete m_pVecTask;
        }

        if (m_pDB != NULL) delete m_pDB;
    }

    void _FreeAllTask()
    {
        VecTask & vec = *m_pVecTask;
        for ( long l = 0; l < (long)vec.size(); l++ ) free((STDBTASK*)vec[l]);
        m_pVecTask->clear();
    }

public:

    enum ENUMTASKPRIORITY{ EN_TASK_SYNC, EN_TASK_ASYNC_NORMAL };

    bool IsEnable() { return m_bDBEnable; }
    void SetEnable(const bool bEnable) { m_bDBEnable = bEnable; }

	bool Init()
    {
        if ( m_pCS == NULL )
        {
            m_pCS = new CRITICAL_SECTION;
            if ( !InitializeCriticalSectionAndSpinCount(m_pCS, 0x80000400) )
            {
                delete m_pCS;
                m_pCS = NULL;
                return false;
            }
        }

        if ( m_pVecTask == NULL ) m_pVecTask = new VecTask;
        if ( m_pDB == NULL ) m_pDB = new CppSQLite3DB;
        m_hEventSync = ::CreateEvent(NULL, FALSE, FALSE, NULL);
        m_pThreadInfo = CThreadPool::GetInstance()->CreateThread("sqlite therad", OnThreadDB, (WPARAM)this);

        AddTask(&CDBLogicManager::_OpenDB);
        return true;
    }

	long OnDBCmd_RunOnThread(NSTHREADPOOL::STTHREADINFO * const pThreadInfo)
    {
        const long CONST_BUFFER_COUNT = 100;
        long lBuffCount = 0;
        STDBTASK ** pAllTask = NULL;
        STDBTASK * p = NULL;
        while ( !pThreadInfo->bExit )
        {
            // 等待
            if ( m_pVecTask->size() <= 0L )
            {
                WaitForSingleObject(pThreadInfo->hEvent, INFINITE);
            }

            if (pThreadInfo->bExit) break;

            // 复制数据
            EnterCriticalSection(m_pCS);
            long lCount = (long)m_pVecTask->size();
            if ( lCount > 0L )
            {
                if ( lCount > lBuffCount )
                {
                    free(pAllTask);
                    lBuffCount = (lCount / CONST_BUFFER_COUNT + 1) * CONST_BUFFER_COUNT;
                    pAllTask = (STDBTASK**)malloc(sizeof(STDBTASK**)* lBuffCount);
                }

                memcpy(pAllTask, &((*m_pVecTask)[0]), sizeof(STDBTASK*) * lCount);
                m_pVecTask->clear();
            }
            LeaveCriticalSection(m_pCS);

            // 处理分发
            for ( long l = 0L; l < lCount; l++ )
            {
                p = (STDBTASK*)pAllTask[l];
                (this->*p->pfn)(false, p->wParam, p->lParam, p->lPriority, &p->pSyncRetData, &p->lSyncRetDataLen);
                if ( p->lPriority == EN_TASK_SYNC )
                {
                    ::SetEvent(m_hEventSync);
                    ::Sleep(1);
                }
                else
                {
                    free(p);
                }
            }
        }

        free(pAllTask);
        return 0L;
    }

    long AddTask(PFN_OnDBCmd pfn, WPARAM wParam = NULL, LPARAM lParam = NULL, long lPriority = EN_TASK_ASYNC_NORMAL, OUT void ** pSyncRetData = NULL, OUT long * pSyncRetDataLen = NULL)
    {
        if (!m_bDBEnable) return -1L;

        STDBTASK * pTask = (STDBTASK*)malloc(sizeof(STDBTASK));
        //pTask->lID = lID;
        pTask->pfn = pfn;
        pTask->lPriority = lPriority;
        pTask->wParam = wParam;
        pTask->lParam = lParam;
        pTask->pSyncRetData = NULL;
        pTask->lSyncRetDataLen = 0L;

        // 异步(可加排序处理)
        EnterCriticalSection(m_pCS);
        m_pVecTask->push_back(pTask);
        ::LeaveCriticalSection(m_pCS);
        THREAD_CONTINUE(m_pThreadInfo);

        if ( lPriority == 0 )
        {
            ::WaitForSingleObject(m_hEventSync, INFINITE);

            if ( pSyncRetData != NULL ) *pSyncRetData = pTask->pSyncRetData;
            if ( pSyncRetDataLen != NULL ) *pSyncRetDataLen = pTask->lSyncRetDataLen;
            free(pTask);
        }
        return NULL;
    }

    // ------------- OpenDB ------------------
protected:
    virtual long _OpenDB(const bool bAddThreadTask, WPARAM wParam = NULL, LPARAM lParam = NULL, long lPriority = EN_TASK_ASYNC_NORMAL, OUT void ** pSyncRetData = NULL, OUT long * pSyncRetDataLen = NULL)
    {
        // parent do nothing
        return 0L;
    }

    void _EscapeSqlite(CStringW & wstr)
    {
        wstr.Replace(L"/", L"//");
        wstr.Replace(L"'", L"''");
        wstr.Replace(L"[", L"/[");
        wstr.Replace(L"]", L"/]");
        wstr.Replace(L"%", L"/%");
        wstr.Replace(L"&", L"/&");
        wstr.Replace(L"_", L"/_");
        wstr.Replace(L"(", L"/(");
        wstr.Replace(L")", L"/)");
    }
};

long __stdcall OnThreadDB(STTHREADINFO * const pThreadInfo, WPARAM wParam, LPARAM lParam)
{
    CDBLogicManager * const pDBManager = (CDBLogicManager * const)(wParam);
    _ASSERT(pDBManager != NULL);
    pDBManager->OnDBCmd_RunOnThread(pThreadInfo);
    return 0L;
}
