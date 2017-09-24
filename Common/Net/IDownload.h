#pragma once

#include "UpDownloadHeader.h"

namespace NSNET
{
    class IDownloadNotify
    {
    public:
        virtual long OnDownloadNotify(long lState, const wchar_t * pwszURL, const wchar_t * pwszSavePath, char * pBuf, const __int64 i64Read, long lProgress, WPARAM wParam, LPARAM lParam) = NULL;
        virtual long OnDelTask(const wchar_t * pwszURL, const wchar_t * pwszSavePath, WPARAM wParam, LPARAM lParam) = NULL;
    };

    class IDownloader
    {
    public:
        virtual long AddDownloadTask(const wchar_t * pwszURL, const wchar_t * pwszSavePath, IDownloadNotify * pIDownloadNotify, WPARAM wParam = NULL, LPARAM lParam = NULL, const short iPriority = EN_PRIORITY_NORMAL, const char iRetryCount = CONST_DEFAULT_TRY_COUNT) = NULL;
        virtual long ClearAllTask() = NULL;
        virtual long CancelRunningTask(const wchar_t * pwszURL) = NULL;
    };
};
