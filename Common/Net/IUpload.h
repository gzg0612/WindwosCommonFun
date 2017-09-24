#pragma once

#include "UpDownloadHeader.h"

namespace NSNET
{
    class IUploadNotify
    {
    public:
        virtual long OnUploadNotify(long lState, const char * pszURL, const char * pszAction, char * pBuf, const __int64 i64Read, long lProgress, WPARAM wParam, LPARAM lParam) = NULL;
        virtual long OnDelULTask(const char * pszURL, const char * pszAction, WPARAM wParam, LPARAM lParam) = NULL;
    };

    class IUploader
    {
    public:
        virtual long AddUploadTask(const char * pszURL, IUploadNotify * pIUploadNotify, const unsigned short nPort = 80, const char * pszAction = NULL, const char * pszHeader=NULL, const char * pszData = NULL, long lDataLen = 0,
                                    WPARAM wParam = NULL, LPARAM lParam = NULL, const short iPriority = EN_PRIORITY_NORMAL, const char iRetryCount = 0) = NULL;
        virtual long ClearAllTask() = NULL;
    };
};
