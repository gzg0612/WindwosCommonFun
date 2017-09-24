#pragma once
#include "IPC.h"
#include <vector>
#include <process.h>
#include "..\spinlock.h"
#include "..\Encode\hash.h"
#include "..\wnd.h"
#include "..\Pool\EventPool.h"

#ifdef _DEBUG
#include <atltrace.h>
#define TRACEDBG ATLTRACE
#else
#define TRACEDBG(...)
#endif

#pragma warning(disable:4996)

DECLEARNS(NSIPC)

#ifndef WM_CONNECT_SERVER
#define WM_CONNECT_SERVER WM_USER + 1
#define WM_CLIENT_ALIVE   WM_USER + 2
#endif // !WM_CONNECT_SERVER

static unsigned char CONST_MAX_SEND_THREADS = 2L;
static unsigned short TIMEOUT_CLIENT_CONNECT_SERVER = (ENABLE_IPC_ALIVE_MONITOR == true ? 5000 : 0xFFFF);
static unsigned short TIMEOUT_CLIENT_WAIT_SERVER = 2000;
static unsigned short TIMEOUT_SERVER_CHECK_CLIENT = 5000;
static unsigned short MONITOR_INTERVAL = 2000;

void _WaitAndPeekMsg()
{
    ::WaitMessage();
    MSG msg;
    ::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
}

// ---------------------- CIPCConnectionManager ----------------------
class CIPCConnectionManager
{
public:
    CIPCConnectionManager(void)
        : m_bExit(false)
        , m_hWnd(NULL)
        , m_hThreadMonitor(NULL)
        , m_ulThreadID(0L)
        , m_slcsMapClientInServer(CSpinLock::SPIN_LOCK_INIT)
        , m_slcsMapServerInClient(CSpinLock::SPIN_LOCK_INIT)
        , m_pMapIPCName(NULL)
        , m_pMapClientInServer(NULL)
        , m_pMapServerInClient(NULL) {}

    ~CIPCConnectionManager(void) { _Destroy(); }

private:
    // --------- connection manager ------------
    friend LRESULT __stdcall _IPCWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    friend inline unsigned int __stdcall _ThreadProc_IPCMonitor(void * p);
    typedef union _un_ipc_monitor_info
    {
        __int64 i64Value;
        union
        {
            struct { unsigned long ulServerPID; unsigned long ulIPCNameHash; } KeyInClient;
            struct { unsigned long ulClientPID; unsigned long ulIPCNameHash; } KeyInServer;
            struct { long lNodeID;              unsigned long ulTick;        } ClientInfoInServer;
            struct { long lNodeID;              HWND hWnd;                   } ServerInfoInClient;
            struct { unsigned long ulPID;       unsigned long ulIPCNameHash; } KeyCommon;
            struct { long lNodeID;              long lValue;                 } InfoCommon;
        };
    } UNIPCMONITORINFO, *LPUNIPCMONITORINFO;

    typedef std::map<__int64, __int64> MapIPCMonitorInfo; // key: KeyInfo; value: ClientInfo/ServerInfo
    typedef std::map<unsigned long, CIPC *> MapIPCName;

    bool            m_bExit;
    HWND            m_hWnd;
    HANDLE          m_hThreadMonitor;
    unsigned long   m_ulThreadID;
    CSpinLock::SLCriticalSection m_slcsMapClientInServer;
    CSpinLock::SLCriticalSection m_slcsMapServerInClient;
    MapIPCName         *m_pMapIPCName;
    MapIPCMonitorInfo  *m_pMapClientInServer;   // server monitor client
    MapIPCMonitorInfo  *m_pMapServerInClient;   // client monitor server

private:

    void _Destroy()
    {
        m_bExit = true;
        if (m_ulThreadID > 0L) { ::PostThreadMessage(m_ulThreadID, WM_USER, NULL, NULL); ::Sleep(1); }
        if (m_hThreadMonitor != NULL) { ::WaitForSingleObject(m_hThreadMonitor, INFINITE); ::CloseHandle(m_hThreadMonitor); }
        if (m_pMapIPCName != NULL) { delete m_pMapIPCName; }
        if (m_pMapClientInServer != NULL) { delete m_pMapClientInServer; m_pMapClientInServer = NULL; }
        if (m_pMapServerInClient != NULL) { delete m_pMapServerInClient; m_pMapServerInClient = NULL; }
        if (m_hWnd != NULL) { ::DestroyWindow(m_hWnd); }
    }

    const bool _CreateWnd()
    {
        if (m_hWnd != NULL) return true;
        wchar_t wszClassName[MAX_PATH];
        _snwprintf(wszClassName, MAX_PATH, L"%s%d", STR_CONNECT_WNDNAME, ::GetCurrentProcessId());
        m_hWnd = NSWND::CreateWnd(_IPCWndProc, this, wszClassName, wszClassName);
        if (m_hWnd != NULL)
        {
            NSWND::ChangeMessageFilterEx(m_hWnd, WM_CONNECT_SERVER);
            NSWND::ChangeMessageFilterEx(m_hWnd, WM_CLIENT_ALIVE);
        }
        return (m_hWnd != NULL);
    }

    const void _MonitorStart()
    {
        if (!ENABLE_IPC_ALIVE_MONITOR) return;
        if (m_hThreadMonitor == NULL)
        {
            m_hThreadMonitor = (HANDLE)::_beginthreadex(NULL, 0, _ThreadProc_IPCMonitor, this, 0, (unsigned int*)&m_ulThreadID);
        }
        ::PostThreadMessage(m_ulThreadID, WM_USER, NULL, NULL);
        ::Sleep(1);
    }

    CIPC * const _GetIPCByNameHash(const unsigned long ulIPCNameHash)
    {
        if (m_pMapIPCName == NULL) return NULL;
        MapIPCName::const_iterator cit = m_pMapIPCName->find(ulIPCNameHash);
        if (cit != m_pMapIPCName->end()) return cit->second;
        return NULL;
    }

    const bool _IsProcessAlive(const unsigned long ulPID)
    {
        HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS | PROCESS_QUERY_INFORMATION, FALSE, ulPID);
        if (hProcess == NULL) return false;
        unsigned long ulExitCode = 0L;
        ::GetExitCodeProcess(hProcess, &ulExitCode);
        ::CloseHandle(hProcess);
        return (STILL_ACTIVE == ulExitCode);
    }

    const long _OnClientConnect(WPARAM wParam, LPARAM lParam)
    {
        // server accept client connect
        UNIPCMONITORINFO mapKey;
        mapKey.KeyInServer.ulClientPID = (unsigned long)lParam;
        mapKey.KeyInServer.ulIPCNameHash = (unsigned long)wParam;

        // check ipc server exist
        if (m_pMapIPCName == NULL || m_pMapServerInClient == NULL) return EN_IPC_CONN_SERVER_NOT_EXIST;
        MapIPCName::const_iterator cit = m_pMapIPCName->find(mapKey.KeyInServer.ulIPCNameHash);
        if (cit == m_pMapIPCName->end() || cit->second == NULL) return EN_IPC_CONN_SERVER_NOT_EXIST;

        // check client process alive
        if (!_IsProcessAlive(mapKey.KeyInServer.ulClientPID)) return EN_IPC_CONN_CLIENT_NOT_ALIVE;

        // server add client node
        long lNodeID = cit->second->AddClientNode(mapKey.KeyInServer.ulClientPID);
        if (lNodeID < 0L) return EN_IPC_CONN_NODE_NOT_EXIST;

        UNIPCMONITORINFO monitorInfo;
        monitorInfo.ClientInfoInServer.lNodeID = lNodeID;
        monitorInfo.ClientInfoInServer.ulTick = ::GetTickCount();

        CSpinLock::Lock(&m_slcsMapClientInServer);
        (*m_pMapClientInServer)[mapKey.i64Value] = monitorInfo.i64Value;
        CSpinLock::UnLock(&m_slcsMapClientInServer);

        _MonitorStart();
        return lNodeID;
    }

    const long _OnClientAliveMsg(WPARAM wParam, LPARAM lParam)
    {
        if (m_pMapClientInServer == NULL) return EN_IPC_CONN_NODE_NOT_EXIST;
        UNIPCMONITORINFO mapKey;
        mapKey.KeyInServer.ulIPCNameHash = (unsigned long)wParam;     // wParam(ulIPCNameHash)
        mapKey.KeyInServer.ulClientPID = (unsigned long)lParam;       // lParam(client PID)
        LOCKBLOCK(&m_slcsMapClientInServer);
        MapIPCMonitorInfo::iterator it = m_pMapClientInServer->find(mapKey.i64Value);
        if (it == m_pMapClientInServer->end()) return EN_IPC_CONN_NODE_NOT_EXIST;  // server node not exist
        ((UNIPCMONITORINFO*)&it->second)->ClientInfoInServer.ulTick = ::GetTickCount();     // update client tick count
        UNLOCKBLOCK();
        return 1L;
    }

    const bool _ClientMonitorServer(MapIPCMonitorInfo::iterator & it)
    {
        UNIPCMONITORINFO mapKey = { it->first };
        UNIPCMONITORINFO monitorInfo = { it->second };

        // server window not exist, disconnect
        if (!::IsWindow(monitorInfo.ServerInfoInClient.hWnd)) return true;

        // client send alive msg to server
        unsigned long ulResult = 0L;
        LRESULT lRet = ::SendMessageTimeoutW(monitorInfo.ServerInfoInClient.hWnd, WM_CLIENT_ALIVE, mapKey.KeyInClient.ulIPCNameHash, ::GetCurrentProcessId(), SMTO_NORMAL, TIMEOUT_CLIENT_WAIT_SERVER, &ulResult);

        // server node not exist, disconnect
        if ((signed)ulResult <= EN_IPC_CONN_NODE_NOT_EXIST) return true;

        // send to server timeout, check server process alive, process alive, do not disconnect
        if (::GetLastError() == ERROR_TIMEOUT) return !_IsProcessAlive(mapKey.KeyInClient.ulServerPID);   //

        TRACEDBG("client --> server> server alive PID: %d\n", mapKey.KeyInClient.ulServerPID);
        return false;
    }

    const bool _ServerMonitorClient(MapIPCMonitorInfo::iterator & it)
    {
        UNIPCMONITORINFO mapKey = { it->first };
        UNIPCMONITORINFO monitorInfo = { it->second };

        unsigned long ulTickCount = ::GetTickCount();
        TRACEDBG("server --> client> client PID: %d  tick %d\n", mapKey.KeyInServer.ulClientPID, ulTickCount - monitorInfo.ClientInfoInServer.ulTick);

        // client send alive msg tick < TIMEOUT_SERVER_CHECK_CLIENT, client is alive, do not disconnect
        if (ulTickCount - monitorInfo.ClientInfoInServer.ulTick < TIMEOUT_SERVER_CHECK_CLIENT) return false;

        // check client process alive, process alive, do not disconnect
        if (!_IsProcessAlive(mapKey.KeyInServer.ulClientPID)) return true;

        // update client tick count
        ((UNIPCMONITORINFO*)&it->second)->ClientInfoInServer.ulTick = ::GetTickCount();
        return false;
    }

    const void _IterateMap(MapIPCMonitorInfo * const pMap)
    {
        typedef const bool (CIPCConnectionManager::*PFN_Monitor)(MapIPCMonitorInfo::iterator & it);
        PFN_Monitor pfnMonitor = (pMap == m_pMapServerInClient ? &CIPCConnectionManager::_ClientMonitorServer : &CIPCConnectionManager::_ServerMonitorClient);
        CSpinLock::SLCriticalSection * pslcs = (pMap == m_pMapServerInClient ? &m_slcsMapServerInClient : &m_slcsMapClientInServer);

        bool bErase = false;
        MapIPCMonitorInfo mapDisconnect;
        CSpinLock::Lock(pslcs);
        MapIPCMonitorInfo::iterator it = pMap->begin();
        for (; it != pMap->end(); )
        {
            bErase = (this->*pfnMonitor)(it);
            if (bErase)
            {
                mapDisconnect[it->first] = it->second;
                it = pMap->erase(it);
            }
            else
                ++it;
        }
        CSpinLock::UnLock(pslcs);

        for (it = mapDisconnect.begin(); it != mapDisconnect.end(); ++it)
        {
            UNIPCMONITORINFO key = { it->first };
            UNIPCMONITORINFO info = { it->second };
            TRACEDBG(L"node disconnect> nodeid %d, pid %d\n", info.InfoCommon.lNodeID, key.KeyCommon.ulPID);
            // notify disconnect, both server or client
            CIPC * pIPC = _GetIPCByNameHash(key.KeyCommon.ulIPCNameHash);
            if (pIPC != NULL) pIPC->_OnNodeDisconnect(info.InfoCommon.lNodeID, key.KeyCommon.ulPID);
        }
    }

    const long _OnIPCMonitor_RunOnThread()
    {
        while (!m_bExit)
        {
            if (m_pMapServerInClient->size() <= 0 && m_pMapClientInServer->size() <= 0)
            {
                _WaitAndPeekMsg();
                if (m_bExit) break;
            }

            _IterateMap(m_pMapServerInClient);
            if (m_bExit) break;

            _IterateMap(m_pMapClientInServer);
            if (m_bExit) break;

            ::Sleep(MONITOR_INTERVAL);
        }
        return 0L;
    }

public:
    const bool Init()
    {
        if (m_pMapIPCName == NULL) m_pMapIPCName = new MapIPCName;
        if (m_pMapServerInClient == NULL) m_pMapServerInClient = new MapIPCMonitorInfo;
        if (m_pMapClientInServer == NULL) m_pMapClientInServer = new MapIPCMonitorInfo;
        return true;
    }

    const bool AddIPCServer(const wchar_t * const pwszIPCName, CIPC * const pIPC)
    {
        if (m_pMapIPCName == NULL || pIPC == NULL || pwszIPCName == NULL || wcslen(pwszIPCName) <= 0L) return false;
        if (!_CreateWnd()) return false;
        (*m_pMapIPCName)[NSENCODE::GetStringHash(pwszIPCName)] = pIPC;
        return true;
    }

    const bool RemoveIPCServer(const wchar_t * const pwszIPCName)
    {
        if (m_pMapIPCName == NULL || pwszIPCName == NULL || wcslen(pwszIPCName) <= 0L) return false;
        const unsigned long ulIPCNameHash = NSENCODE::GetStringHash(pwszIPCName);
        MapIPCName::iterator it_ipcName = m_pMapIPCName->find(ulIPCNameHash);
        if (it_ipcName == m_pMapIPCName->end()) return false;
        m_pMapIPCName->erase(it_ipcName);

        // remove server clients
        CSpinLock::Lock(&m_slcsMapClientInServer);
        MapIPCMonitorInfo::iterator it = m_pMapClientInServer->begin();
        for (; it != m_pMapClientInServer->end(); )
        {
            UNIPCMONITORINFO * pKey = (UNIPCMONITORINFO*)&it->first;
            if (pKey->KeyInServer.ulIPCNameHash == ulIPCNameHash) it = m_pMapClientInServer->erase(it); else ++it;
        }
        CSpinLock::UnLock(&m_slcsMapClientInServer);
        return true;
    }

    const long ConnectToServer(const wchar_t * const pwszIPCName, const unsigned long ulServerPID, CIPC * const pIPC)
    {
        if (m_pMapIPCName == NULL || m_pMapServerInClient == NULL) return EN_IPC_CONN_SERVER_NOT_EXIST;

        // client connect to server
        wchar_t wszClassName[MAX_PATH];
        _snwprintf(wszClassName, MAX_PATH, L"%s%d", STR_CONNECT_WNDNAME, ulServerPID);
        HWND hWnd = ::FindWindow(wszClassName, wszClassName);
        if (hWnd == NULL) return -1L;
        const unsigned long ulIPCNameHash = NSENCODE::GetStringHash(pwszIPCName);

        unsigned long ulResult = 0L;
        long lRet = (long)::SendMessageTimeoutW(hWnd, WM_CONNECT_SERVER, ulIPCNameHash, ::GetCurrentProcessId(), SMTO_NORMAL, TIMEOUT_CLIENT_CONNECT_SERVER, &ulResult);
        if (::GetLastError() == ERROR_TIMEOUT) return EN_IPC_CONN_NODE_NOT_EXIST;
        long lNodeID = (signed)ulResult;
        if (lNodeID <= 0L) return EN_IPC_CONN_NODE_NOT_EXIST;

        // map ipc
        (*m_pMapIPCName)[ulIPCNameHash] = pIPC;

        UNIPCMONITORINFO monitorInfo;
        monitorInfo.ServerInfoInClient.lNodeID = lNodeID;
        monitorInfo.ServerInfoInClient.hWnd = hWnd;

        UNIPCMONITORINFO mapKey;
        mapKey.KeyInClient.ulServerPID = ulServerPID;
        mapKey.KeyInClient.ulIPCNameHash = ulIPCNameHash;

        CSpinLock::Lock(&m_slcsMapServerInClient);
        (*m_pMapServerInClient)[mapKey.i64Value] = monitorInfo.i64Value;
        CSpinLock::UnLock(&m_slcsMapServerInClient);

        _MonitorStart();
        return lNodeID;
    }
};

inline unsigned int __stdcall _ThreadProc_IPCMonitor(void * pData)
{
    CIPCConnectionManager * const pConnectionManager = (CIPCConnectionManager * const)pData;
    return pConnectionManager->_OnIPCMonitor_RunOnThread();
}

LRESULT __stdcall _IPCWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    case WM_NCCREATE:
        ::SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG)(LONG_PTR)(((LPCREATESTRUCT)lParam)->lpCreateParams));
        break;
    case WM_CONNECT_SERVER:
    {
        CIPCConnectionManager * pCIPCConnectionManager = (CIPCConnectionManager*)(LONG_PTR)::GetWindowLongPtr(hWnd, GWL_USERDATA);
        return pCIPCConnectionManager->_OnClientConnect(wParam, lParam);
    }
    break;
    case WM_CLIENT_ALIVE:
    {
        CIPCConnectionManager * pCIPCConnectionManager = (CIPCConnectionManager*)(LONG_PTR)::GetWindowLongPtr(hWnd, GWL_USERDATA);
        return pCIPCConnectionManager->_OnClientAliveMsg(wParam, lParam);
    }
    break;
    default:
        break;
    }
    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

// ------------------------ CIPCManager ------------------------
class CIPCManager
{
private:
    CIPCManager()
        : m_bExit(false)
        , m_lSendThread(CONST_MAX_SEND_THREADS)
        , m_plAryThreadID(NULL)
        , m_phAryThreadSend(NULL)
        , m_pIPCConnectionManager(NULL)
        , m_pVecSendTask(NULL)
        , m_pArySpinCS(NULL) {}

    ~CIPCManager(void) { _FreeMem(); }

public:
    static CIPCManager * const GetInstance() { return m_pInst; }
    static CIPCManager * const CreateInstance()
    {
        CSpinLock::Lock(&m_sLock);
        if (!m_bDestroy && m_pInst == NULL)
        {
            m_pInst = new CIPCManager();
            m_pInst->_Init();
        }
        CSpinLock::UnLock(&m_sLock);
        return m_pInst;
    }
    static void Destroy()
    {
        m_bDestroy = true;
        CSpinLock::Lock(&m_sLock);
        if (m_pInst != NULL) { delete m_pInst; m_pInst = NULL; }
        CSpinLock::UnLock(&m_sLock);
    }
private:
    static volatile bool m_bDestroy;
    static CIPCManager * m_pInst;
    static CSpinLock::SLCriticalSection m_sLock;

private:
    friend inline unsigned int __stdcall _ThreadProc_SendTask(void * p);
    typedef struct _st_ipc_send_task
    {
        HANDLE      hEventSync;
        long        lNodeID;
        CIPC       *pIPC;
        const char *pBuf;
        long        lSize;
        char      **pRet;
        long       *plSizeRet;
        long        lSendSize;
    } STIPCSENDTASK, *LPSTIPCSENDTASK;

    bool                m_bExit;
    long                m_lSendThread;
    long               *m_plAryThreadID;
    HANDLE             *m_phAryThreadSend;
    CIPCConnectionManager          *m_pIPCConnectionManager;
    std::vector<STIPCSENDTASK *>   *m_pVecSendTask;
    CSpinLock::SLCriticalSection   *m_pArySpinCS;

private:
    const bool _Init()
    {
        NSEVENTPOOL::CEventPool::CreateInstance();
        m_pIPCConnectionManager = new CIPCConnectionManager;
        m_pIPCConnectionManager->Init();
        m_pVecSendTask = new std::vector<STIPCSENDTASK *>[m_lSendThread];
        m_pArySpinCS = (CSpinLock::SLCriticalSection*)malloc(sizeof(CSpinLock::SLCriticalSection) * m_lSendThread);
        memset((void*)m_pArySpinCS, 0x00, sizeof(CSpinLock::SLCriticalSection) * m_lSendThread);
        m_phAryThreadSend = (HANDLE*)malloc(sizeof(HANDLE) * m_lSendThread);
        memset(m_phAryThreadSend, 0x00, sizeof(HANDLE) * m_lSendThread);
        m_plAryThreadID = (long*)malloc(sizeof(long) * m_lSendThread);
        memset(m_plAryThreadID, 0x00, (sizeof(long) * m_lSendThread));
        return true;
    }

    const void _FreeMem()
    {
        m_bExit = true;

        if (m_pIPCConnectionManager != NULL) { delete m_pIPCConnectionManager; m_pIPCConnectionManager = NULL; }

        // set thread event
        if (m_plAryThreadID != NULL) { for (long l = 0; l < m_lSendThread; ++l) ::PostThreadMessage(m_plAryThreadID[l], WM_USER, NULL, NULL); free(m_plAryThreadID); }
        ::Sleep(1);

        // close thread handle
        if (m_phAryThreadSend != NULL)
        {
            ::WaitForMultipleObjects(m_lSendThread, m_phAryThreadSend, TRUE, INFINITE);
            for (long l = 0; l < m_lSendThread; ++l) ::CloseHandle(m_phAryThreadSend[l]);
            free(m_phAryThreadSend);
            m_phAryThreadSend = NULL;
        }

        // sync task continue
        if (m_pVecSendTask != NULL)
        {
            std::vector<STIPCSENDTASK *> & vec = *m_pVecSendTask;
            NSEVENTPOOL::CEventPool * pEventPool = NSEVENTPOOL::CEventPool::GetInstance();
            HANDLE hEvent = NULL;
            for (long l = 0; l < (long)m_pVecSendTask->size(); ++l) { ::SetEvent(vec[l]->hEventSync); pEventPool->ReleaseEvent(vec[l]->hEventSync); }
            ::Sleep(1);
        }

        if (m_pArySpinCS != NULL) { free((void*)m_pArySpinCS); m_pArySpinCS = NULL; }
        if (m_pVecSendTask != NULL) { delete[] m_pVecSendTask; m_pVecSendTask = NULL; }
    }

    // ------------- ipc send thread pool -------------
    const bool _InitThread(const long lIndex)
    {
        if (m_bExit || lIndex < 0L || lIndex > m_lSendThread) return false;
        if (m_phAryThreadSend[lIndex] != NULL) return true;
        long * plThreadIndex = new long;
        *plThreadIndex = lIndex;
        m_phAryThreadSend[lIndex] = (HANDLE)::_beginthreadex(NULL, 0, _ThreadProc_SendTask, plThreadIndex, 0, (unsigned int*)&m_plAryThreadID[lIndex]);
        return true;
    }

    const long _OnSendTask_RunOnThread(const long lThreadIndex)
    {
        STIPCSENDTASK * pTask = NULL;
        long lTaskIndex = 0L;
        long lNodeState = EN_IPC_BUSY;
        std::vector<STIPCSENDTASK *> & vec = m_pVecSendTask[lThreadIndex];

        while (!m_bExit)
        {
            if (vec.size() <= 0L)
            {
                _WaitAndPeekMsg();
                if (m_bExit) break;
            }

            CSpinLock::Lock(&m_pArySpinCS[lThreadIndex]);
            for (lTaskIndex = 0L; lTaskIndex < (long)vec.size(); ++lTaskIndex)
            {
                STIPCSENDTASK * p = vec[lTaskIndex];
                lNodeState = p->pIPC->GetNodeState(p->lNodeID);
                if (lNodeState <= EN_IPC_IDLE)
                {
                    pTask = p;
                    break;
                }
            }
            if (pTask != NULL)
            {
                vec.erase(vec.begin() + lTaskIndex);
            }
            CSpinLock::UnLock(&m_pArySpinCS[lThreadIndex]);

            if (pTask != NULL)
            {
                if (m_bExit)
                {
                    if (pTask->hEventSync != NULL) ::SetEvent(pTask->hEventSync);
                    break;
                }

                if (pTask->hEventSync != NULL)
                {
                    // sync send
                    if (lNodeState == EN_IPC_IDLE)
                    {
                        pTask->lSendSize = pTask->pIPC->_SendData(pTask->lNodeID, pTask->pBuf, pTask->lSize, pTask->pRet, pTask->plSizeRet);
                    }
                    ::SetEvent(pTask->hEventSync);
                }
                else
                {
                    // async send
                    IIPCReciver * pIPCRevicer = ((IIPCReciver*)pTask->pRet);
                    if (lNodeState == EN_IPC_IDLE)
                    {
                        char * pRet = NULL;
                        long lSizeRet = 0L;
                        pTask->lSendSize = pTask->pIPC->_SendData(pTask->lNodeID, pTask->pBuf, pTask->lSize, &pRet, &lSizeRet);
                        if (pIPCRevicer != NULL) pIPCRevicer->OnIPCRecvData(pTask->lNodeID, pRet, lSizeRet);
                    }
                    if (pTask->pBuf != NULL) free((char*)pTask->pBuf);
                    delete pTask;
                }
            }
            pTask = NULL;
            lNodeState = EN_IPC_BUSY;
        }
        return 0L;
    }

public:

    // ----------- send thread (sync) -----------
    const long AddSendTask(CIPC * const pIPC, const long lNodeID, const char * pBuf, const long lDataSize, OUT char ** const pRet = NULL, OUT long * const plSizeRet = NULL)
    {
        if (m_bExit || pBuf == NULL || lDataSize <= 0L || lDataSize > MAX_IPC_DATA) return -1L;

        STIPCSENDTASK task = { 0 };
        STIPCSENDTASK * pTask = &task;
        pTask->hEventSync = NULL;
        pTask->pIPC = pIPC;
        pTask->lNodeID = lNodeID;
        pTask->pBuf = pBuf;
        pTask->lSize = lDataSize;
        pTask->pRet = pRet;
        pTask->plSizeRet = plSizeRet;
        pTask->lSendSize = 0L;

        long lIndex = lNodeID % m_lSendThread;
        LOCKBLOCK(&m_pArySpinCS[lIndex]);
        if (!_InitThread(lIndex)) return -1L;
        pTask->hEventSync = NSEVENTPOOL::CEventPool::GetInstance()->GetEvent();
        m_pVecSendTask[lIndex].push_back(pTask);
        UNLOCKBLOCK();

        ::PostThreadMessage(m_plAryThreadID[lIndex], WM_USER, NULL, NULL);
        ::WaitForSingleObject(pTask->hEventSync, INFINITE);
        NSEVENTPOOL::CEventPool::GetInstance()->ReleaseEvent(pTask->hEventSync);
        return pTask->lSendSize;
    }

    const long AddPostTask(CIPC * const pIPC, const long lNodeID, const char * pBuf, const long lDataSize, IIPCReciver * const pRevicer)
    {
        if (m_bExit || pBuf == NULL || lDataSize <= 0L || lDataSize > MAX_IPC_DATA) return -1L;

        char * pBufCopy = (char*)malloc(lDataSize);
        memcpy(pBufCopy, pBuf, lDataSize);

        STIPCSENDTASK * pTask = new STIPCSENDTASK;
        pTask->hEventSync = NULL;
        pTask->pIPC = pIPC;
        pTask->lNodeID = lNodeID;
        pTask->pBuf = pBufCopy;
        pTask->lSize = lDataSize;
        pTask->pRet = (char**)pRevicer;
        pTask->plSizeRet = NULL;
        pTask->lSendSize = 0L;

        long lIndex = lNodeID % m_lSendThread;
        LOCKBLOCK(&m_pArySpinCS[lIndex]);
        if (!_InitThread(lIndex)) return -1L;
        m_pVecSendTask[lIndex].push_back(pTask);
        UNLOCKBLOCK();

        ::PostThreadMessage(m_plAryThreadID[lIndex], WM_USER, NULL, NULL);
        return 0L;
    }

    // ----------- revice thread -----------
    const HANDLE AddRecvMonitor(const HANDLE hObject, void * const pData)
    {
        if (hObject == NULL || pData == NULL) return NULL;
        HANDLE hWait = NULL;
        if (!::RegisterWaitForSingleObject(&hWait, hObject, OnIPCRecvDataWaitCallBack, pData, INFINITE, WT_EXECUTEINWAITTHREAD))
        {
            ::CloseHandle(hWait);
            hWait = NULL;
        }
        return hWait;
    }

    const void RemoveRecvMonitor(const HANDLE hNewWaitObject)
    {
        if (hNewWaitObject != NULL) { ::UnregisterWait(hNewWaitObject); }
    }

    // ----------- CIPCConnectionManager -----------
    CIPCConnectionManager * const GetIPCConnectionManager() { return m_pIPCConnectionManager; }
};

volatile bool CIPCManager::m_bDestroy = false;
CIPCManager * CIPCManager::m_pInst = NULL;
CSpinLock::SLCriticalSection CIPCManager::m_sLock = 0L;

inline unsigned int __stdcall _ThreadProc_SendTask(void * p)
{
    unsigned int uiRet = CIPCManager::GetInstance()->_OnSendTask_RunOnThread(*(long*)p);
    delete (long*)p;
    return uiRet;
}

ENDDECLEARNS();