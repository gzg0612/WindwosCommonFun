#pragma once
#include <windows.h>

#ifndef DECLEARNS
#define DECLEARNS(ns) namespace ns {
#define ENDDECLEARNS() };
#endif // !DECLEARNS

DECLEARNS(NSWND)

static const HWND CreateWnd(const WNDPROC pfnWndProc, void * const pData, const wchar_t * const pwszClassName, const wchar_t * const pwszTitle = NULL)
{
    if (pfnWndProc == NULL || pwszClassName == NULL || wcslen(pwszClassName) <= 0L) return NULL;

    WNDCLASSEX wndclassex = { 0 };
    wndclassex.cbSize = sizeof(WNDCLASSEX);
    wndclassex.style = CS_HREDRAW | CS_VREDRAW;
    wndclassex.lpfnWndProc = pfnWndProc;
    wndclassex.cbClsExtra = 0;
    wndclassex.cbWndExtra = 0;
    wndclassex.hInstance = GetModuleHandle(NULL);
    wndclassex.hIcon = NULL; //LoadIcon(NULL, IDI_APPLICATION);
    wndclassex.hIconSm = NULL;
    wndclassex.hCursor = NULL;
    wndclassex.hbrBackground = NULL;
    wndclassex.lpszMenuName = NULL;
    wndclassex.lpszClassName = pwszClassName;

    ATOM atRet = RegisterClassEx(&wndclassex);
    if (atRet == 0L && ::GetLastError() != ERROR_CLASS_ALREADY_EXISTS) return NULL;
    return ::CreateWindowEx(WS_EX_TOOLWINDOW, wndclassex.lpszClassName, pwszTitle, WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, wndclassex.hInstance, pData);
}

static void ChangeMessageFilter(unsigned long ulMsgID)
{
    HMODULE hUser32 = ::LoadLibrary(L"User32.dll");
    if (hUser32 == NULL) return;
    typedef BOOL(WINAPI *PFN_ChangeWindowMessageFilter)(UINT, DWORD);
    PFN_ChangeWindowMessageFilter pfn = (PFN_ChangeWindowMessageFilter)::GetProcAddress(hUser32, "ChangeWindowMessageFilter");
    if (pfn != NULL)
    {
        pfn(ulMsgID, 1); // #define MSGFLT_ADD 1
    }
    FreeLibrary(hUser32);
}

static void ChangeMessageFilterEx(const HWND hWnd, unsigned long ulMsgID)
{
    HMODULE hUser32 = ::LoadLibrary(L"User32.dll");
    if (hUser32 == NULL) return;
    
    //typedef BOOL(WINAPI *PFN_ChangeWindowMessageFilter)(_In_  UINT, _In_  DWORD);
    //typedef BOOL (WINAPI *PFN_ChangeWindowMessageFilterEx)(_In_ HWND hWnd, _In_ UINT message, _In_ DWORD action, _Inout_opt_ PCHANGEFILTERSTRUCT pChangeFilterStruct);
    typedef BOOL(WINAPI *PFN_ChangeWindowMessageFilterEx)(HWND hWnd, UINT message, DWORD action, void * pChangeFilterStruct);
        
    PFN_ChangeWindowMessageFilterEx pfn = (PFN_ChangeWindowMessageFilterEx)::GetProcAddress(hUser32, "ChangeWindowMessageFilterEx");
    if (pfn != NULL)
    {
        pfn(hWnd, ulMsgID, 1, NULL); // #define MSGFLT_ADD 1
    }
    else
    {
        // ChangeWindowMessageFilterEx failed, try ChangeWindowMessageFilter
        typedef BOOL(WINAPI *PFN_ChangeWindowMessageFilter)(UINT, DWORD);
        PFN_ChangeWindowMessageFilter pfn = (PFN_ChangeWindowMessageFilter)::GetProcAddress(hUser32, "ChangeWindowMessageFilter");
        if (pfn != NULL)
        {
            pfn(ulMsgID, 1); // #define MSGFLT_ADD 1
        }
    }
    FreeLibrary(hUser32);
}

ENDDECLEARNS();