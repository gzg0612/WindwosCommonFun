#pragma once

#ifndef WM_TRAYICON_NOTIFY
#define WM_TRAYICON_NOTIFY WM_USER + 200
#endif

#include <ShellAPI.h>

class CTrayIcon
{
public:
    CTrayIcon(void) 
    { 
        memset(&m_NID, 0x00, sizeof(NOTIFYICONDATAW));
        m_NID.cbSize = sizeof(NOTIFYICONDATAW);
    }

    ~CTrayIcon(void) 
    { 
        ::Shell_NotifyIcon( NIM_DELETE, &m_NID );
        if (m_NID.hIcon != NULL) { ::DeleteObject(m_NID.hIcon); } 
    }

private:

    NOTIFYICONDATAW m_NID;

public:

    BOOL LoadTrayIcon( HWND hWnd, HICON hTrayIcon, const wchar_t * const pwszText = NULL )
    {
        m_NID.hWnd = hWnd;
        m_NID.hIcon = hTrayIcon;
        m_NID.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;  
        m_NID.uCallbackMessage = WM_TRAYICON_NOTIFY;

        if (pwszText != NULL && wcslen(pwszText) > 0)
        {
            wcsncpy( m_NID.szTip, pwszText, min(wcslen(pwszText), sizeof(m_NID.szTip)/sizeof(wchar_t)-1) );
        }

        return ::Shell_NotifyIcon( NIM_ADD, &m_NID );
    }
};
