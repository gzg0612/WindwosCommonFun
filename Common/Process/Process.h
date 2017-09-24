#pragma once

#include <Windows.h>
//#include <TlHelp32.h>
//#include <Psapi.h>
#include <process.h>

#ifndef DECLEARNS
#define DECLEARNS(ns) namespace ns {
#define ENDDECLEARNS() };
#endif // !DECLEARNS

DECLEARNS(NSPROCESS)

static BOOL CreateProcess(const wchar_t * pwszCmd, OUT PROCESS_INFORMATION * pProcessInfo = NULL, const bool bShow = true, const bool bCloseHandle = true, const wchar_t* pwszWorkingPath = NULL )
{
    long len = 0L;
    if (pwszCmd == NULL || ((len = (long)wcslen(pwszCmd)) <= 0L)) return FALSE;
    
    PROCESS_INFORMATION processInfo = { 0 };
    if ( pProcessInfo == NULL ) pProcessInfo = &processInfo;

    STARTUPINFO si = { 0 };
    si.cb = sizeof(STARTUPINFO);

    if ( !bShow )
    {
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
    }

    wchar_t wszPath[MAX_PATH * 2] = { 0 };
    memcpy(wszPath, pwszCmd, sizeof(wchar_t) * len);
    BOOL bRet = ::CreateProcessW(NULL, wszPath, NULL, NULL, FALSE, CREATE_BREAKAWAY_FROM_JOB, NULL, pwszWorkingPath, &si, pProcessInfo);
    if (bCloseHandle)
    {
        ::CloseHandle(pProcessInfo->hProcess);
        ::CloseHandle(pProcessInfo->hThread);
    }

    return bRet;
}

ENDDECLEARNS();