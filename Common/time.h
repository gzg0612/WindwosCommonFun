#pragma once
#include <stdio.h>

namespace NSTIME
{
    static const __int64 GetTimeStamp()
    {
        // ns / 1000 --> ms
        union
        {
            long long ns100;
            FILETIME ft;
        } now;


        struct
        {
            long    tv_sec;         /* seconds */
            long    tv_usec;        /* and microseconds */
        } tv;

        SYSTEMTIME st;
        ::GetLocalTime(&st);
        ::SystemTimeToFileTime(&st, &now.ft);
        //::GetSystemTimeAsFileTime(&now.ft);
        tv.tv_usec = (long)((now.ns100 / 10LL) % 1000000LL);
        tv.tv_sec = (long)((now.ns100 - 116444736000000000LL) / 10000000LL);
        return __int64((__int64)tv.tv_sec * (__int64)1000000 + tv.tv_usec);
    }

    static void GetTimeMS(wchar_t * pBuf, const long lSize)
    {
        if ( pBuf == NULL ) return;
        SYSTEMTIME st;
        ::GetLocalTime(&st);
        _snwprintf(pBuf, lSize, L"%02d%02d%02d%03d\0", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    }

    static void GetDate(wchar_t * pBuf, const long lSize)
    {
        if ( pBuf == NULL ) return;
        SYSTEMTIME st;
        ::GetLocalTime(&st);
        _snwprintf(pBuf, lSize, L"%04d%02d%02d\0", st.wYear, st.wMonth, st.wDay);
    }

    static void GetDateTime(wchar_t * pBuf, const long lSize)
    {
        if ( pBuf == NULL ) return;
        SYSTEMTIME st;
        ::GetLocalTime(&st);
        _snwprintf(pBuf, lSize, L"%04d%02d%02d%02d%02d%02d\0", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    }

    static void GetDateTimeMS(wchar_t * pBuf, const long lSize)
    {
        if ( pBuf == NULL ) return;
        SYSTEMTIME st;
        ::GetLocalTime(&st);
        _snwprintf(pBuf, lSize, L"%04d%02d%02d%02d%02d%02d%03d\0", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    }
}