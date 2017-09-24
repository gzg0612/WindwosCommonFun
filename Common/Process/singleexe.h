#pragma once
#include "..\wnd.h"

class CSingleExe
{
public:
    CSingleExe(void) : m_hMutex(NULL) {}
    ~CSingleExe(void)
    {
        if (m_hMutex != NULL)
        {
            ::CloseHandle(m_hMutex);
            m_hMutex = NULL;
        }
    }

private:
    HANDLE m_hMutex;

public:

    template<typename T>
    bool CreateSingleExe(const T * const pTMutexName, const T * const pTWndName, const T * const pTWndClass = NULL, bool bShowToTop = true)
    {
        if (pTMutexName == NULL) return true;
        NSWND::ChangeMessageFilter(WM_SHOWWINDOW);
        bool bWCHAR = sizeof(T) == sizeof(wchar_t);
        m_hMutex = ( bWCHAR == true ? ::CreateMutexW(NULL, FALSE, (const wchar_t *)pTMutexName) : ::CreateMutexA(NULL, FALSE, (const char *)pTMutexName) );
        if (m_hMutex == NULL) return false;

        if ( ERROR_ALREADY_EXISTS == ::GetLastError() )
        {
            if (pTWndName != NULL || pTWndClass != NULL)
            {
                HWND hMsgHandlerWnd = (bWCHAR == true ? ::FindWindowW((wchar_t *)pTWndClass, (const wchar_t *)pTWndName) : ::FindWindowA((char *)pTWndClass, (const char *)pTWndName));
                if (bShowToTop && ::IsWindow(hMsgHandlerWnd))
                {
                    ::SendMessage( hMsgHandlerWnd, WM_SHOWWINDOW, NULL, NULL );
                }
            }            

            ::CloseHandle(m_hMutex);
            m_hMutex = NULL;
            return false;
        }

        return true;
    }
};
