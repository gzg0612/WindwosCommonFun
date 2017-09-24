#pragma once
#include <string>
#include <map>
#define WM_PROCESS_LIVE WM_USER + 0x01
#define TIMEOUT_SERVER_CHECK_CLIENT 5000
#define MONITOR_INTERVAL 2000
class CProcessMonitor
{
public:
    CProcessMonitor(void)
    {
        ::InitializeCriticalSection(&m_csMonitorListCritical);
        m_bIsRunning = false;
        m_hThread = NULL;
        m_hSendDelyEvent = NULL;
        m_bIsTwoWay = true;
    }
    ~CProcessMonitor(void)
    {
        StopMonitor();
        ::DeleteCriticalSection(&m_csMonitorListCritical);
    }
public:
    bool InitMonitor( unsigned int uiProcessId,  HWND hWnd,  std::wstring& wstrErrorMessage, bool bIsTowWay = true)
    {
        m_bIsTwoWay = bIsTowWay;
        ::EnterCriticalSection(&m_csMonitorListCritical);
        bool bResult = false;
        do 
        {
            if (NULL == hWnd && bIsTowWay)
            {
                wstrErrorMessage = L"ProcessId or hWnd is error.";
                break;
            }

            m_mapMonitorList[uiProcessId] = hWnd;
            bResult = true;
        } while (false);
        ::LeaveCriticalSection(&m_csMonitorListCritical);

        m_hSendDelyEvent = ::CreateEvent(NULL, FALSE, FALSE, L"");
        if (bResult && !m_bIsRunning)
        {
            m_bIsRunning = true;
            m_hThread = (HANDLE)_beginthreadex(NULL, NULL, MonitorThread, (void*)this, NULL, NULL);
        }
        return bResult;
    }
    void StopMonitor()
    {

        m_bIsRunning = false;
        ::SetEvent(m_hSendDelyEvent);
        ::Sleep(10);

        if (NULL != m_hThread)
        {
            WaitForSingleObject(m_hThread, INFINITE);
            CloseHandle(m_hThread);
            m_hThread = NULL;
        }
       
        if (NULL != m_hSendDelyEvent)
        {
            CloseHandle(m_hSendDelyEvent);
            m_hSendDelyEvent = NULL;
        }
        m_mapMonitorList.clear();
    }

    bool RemoveMonitorItem( unsigned int uiProcessId)
    {
        bool bIsResult = false;
        ::EnterCriticalSection(&m_csMonitorListCritical);

        if (m_mapMonitorList.size() > 0)
        {
            m_mapMonitorList.erase(uiProcessId);
            bIsResult = true;
        }

        ::LeaveCriticalSection(&m_csMonitorListCritical);
        return bIsResult;
    }

    bool AddMonitorItem(unsigned int uiProcessId,  HWND hWnd,  std::wstring& wstrErrorMessage)
    {
        bool bIsResult = false;
        ::EnterCriticalSection(&m_csMonitorListCritical);

        if ( NULL == hWnd && m_bIsTwoWay)
        {
            wstrErrorMessage = L"hWnd is error.";
            bIsResult = false;
        }
        else
        {
            m_mapMonitorList[uiProcessId] = hWnd;
            bIsResult = true;

        }
        ::LeaveCriticalSection(&m_csMonitorListCritical);

        return bIsResult;
    }
    unsigned int GetMonitorCount()
    {
        ::EnterCriticalSection(&m_csMonitorListCritical);
        unsigned int iSize = (unsigned int)m_mapMonitorList.size();
        ::LeaveCriticalSection(&m_csMonitorListCritical);

        return iSize;
    }
    bool GetMonitorState(){return m_bIsRunning;}
public:
    static unsigned int __stdcall MonitorThread( void* pParam)
    {
        CProcessMonitor* pObj = (CProcessMonitor*) pParam;
        pObj->Monitor();
        return 0;
    }
public:
    virtual unsigned int ProcessDeadNotofy(unsigned int uiProcess) = 0;
private:
    unsigned int Monitor()
    {
        while (m_bIsRunning)
        {
            WaitForSingleObject(m_hSendDelyEvent, MONITOR_INTERVAL);
            if (!m_bIsRunning)
            {
                break;
            }
            ::EnterCriticalSection(&m_csMonitorListCritical);
            std::map<unsigned int, HWND>::iterator itmap = m_mapMonitorList.begin();
            for (; itmap != m_mapMonitorList.end();)
            {
                if (!m_bIsRunning)
                {
                    break;
                }

                bool bRet = _MessageCheck(itmap->first, itmap->second);

                if (bRet && !_IsProcessAlive(itmap->first))
                {
                    unsigned int uiProccessID = itmap->first;
                    itmap = m_mapMonitorList.erase(itmap);
                    ProcessDeadNotofy(uiProccessID);
                    continue;
                }
                 ++itmap;
            }
            ::LeaveCriticalSection(&m_csMonitorListCritical);
        }
        return 0;
    }

    const bool _MessageCheck(const unsigned long ulPID, HWND hWnd)
    {
        if (!m_bIsTwoWay)
        {
            return true;
        }
        unsigned long ulResult = 0L;
        LRESULT lRet = ::SendMessageTimeoutW(hWnd, WM_PROCESS_LIVE, NULL, ulPID, SMTO_NORMAL, TIMEOUT_SERVER_CHECK_CLIENT, &ulResult);
        DWORD dwLastError = GetLastError();
        return ERROR_INVALID_WINDOW_HANDLE == dwLastError || ERROR_TIMEOUT == dwLastError;
    }

    const bool _IsProcessAlive(const unsigned long ulPID)
    {
        HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS | PROCESS_QUERY_INFORMATION, FALSE, ulPID);
        if (hProcess == NULL) 
            return false;
        unsigned long ulExitCode = 0L;
        ::GetExitCodeProcess(hProcess, &ulExitCode);
        ::CloseHandle(hProcess);
        return (STILL_ACTIVE == ulExitCode);
    }
private:
    bool m_bIsRunning;
    std::map<unsigned int, HWND> m_mapMonitorList;
    CRITICAL_SECTION    m_csMonitorListCritical;
    HANDLE m_hThread;
    HANDLE m_hSendDelyEvent;// = ::CreateEvent(NULL, FALSE, FALSE, L"");
    bool m_bIsTwoWay;
};
