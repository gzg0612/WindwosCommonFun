#pragma once
#pragma comment( lib, "Version.lib" )

namespace NSIO
{
    static bool GetExeFileVersion(const wchar_t * const pwszExePath, OUT DWORD * pdwVersionMS, OUT DWORD * pdwVersionLS)
    {
        if ( pwszExePath == NULL || wcslen(pwszExePath) <= 0 ) return false;

        DWORD dwLen = ::GetFileVersionInfoSizeW(pwszExePath, 0);
        if ( dwLen == 0 ) return false;

        DWORD dwVerHnd = 0;
        wchar_t * pBuf = (wchar_t *)malloc((dwLen + 1)*sizeof(wchar_t));
        memset(pBuf, 0x00, (dwLen + 1)*sizeof(wchar_t));

        unsigned int uiInfoSize = 0;
        VS_FIXEDFILEINFO * pFileInfo = NULL;
        ::GetFileVersionInfoW(pwszExePath, 0, dwLen, pBuf);
        ::VerQueryValueW(pBuf, L"\\", (void**)&pFileInfo, &uiInfoSize);

        *pdwVersionMS = pFileInfo->dwProductVersionMS;
        *pdwVersionLS = pFileInfo->dwProductVersionLS;

        free(pBuf);
        return true;
    }

    static long CompareVersion(const wchar_t * const pwszExePath, const wchar_t * const pwszNewVersion, OUT long * const plExeVer = NULL, OUT long * const plNewVer = NULL)
    {
        // long ver: FF.FF.FF.FF  --> 255.255.255.255

        long lenNewVer = 0L;
        if ( pwszNewVersion == NULL || ((lenNewVer = wcslen(pwszNewVersion)) <= 0) ) return 0L;

        // ¼ì²â¸ñÊ½
        long lPointCount = 0L;
        const wchar_t * p = pwszNewVersion;
        for ( long l = 0L; l < lenNewVer; l++ )
        {
            if ( *p == L'.' )
            {
                lPointCount++;
            }
            else if ( *p < 0x30 || *p > 0x39 )
            {
                lPointCount = 0L;
                break;
            }
            p++;
        }
        if ( lPointCount != 3 ) return 0L;

        DWORD dwVersionMS = 0;
        DWORD dwVersionLS = 0;
        if ( !GetExeFileVersion(pwszExePath, &dwVersionMS, &dwVersionLS) ) return 0L;

        unsigned long exeVer[4] = { 0 };
        exeVer[0] = HIWORD(dwVersionMS);
        exeVer[1] = LOWORD(dwVersionMS);
        exeVer[2] = HIWORD(dwVersionLS);
        exeVer[3] = LOWORD(dwVersionLS);

        unsigned long newVer[4] = { 0 };
        swscanf(pwszNewVersion, L"%d.%d.%d.%d", &newVer[0], &newVer[1], &newVer[2], &newVer[3]);

        *plExeVer = ((exeVer[0] << 24) | (((unsigned char)exeVer[1]) << 16) | (((unsigned char)exeVer[2]) << 8) | (((char)exeVer[3])));
        *plNewVer = ((newVer[0] << 24) | (((unsigned char)newVer[1]) << 16) | (((unsigned char)newVer[2]) << 8) | (((char)newVer[3])));

        for ( long l = 0; l < 4; l++ )
        {
            if ( exeVer[l] > newVer[l] )
            {
                return -1L;
            }
            else if ( exeVer[l] < newVer[l] )
            {
                return 1;
            }
        }
        return 0L;
    }

    static void FormatVer(const unsigned long lVer, wchar_t * pwszBuf, const long lBufSize)
    {
        if ( pwszBuf == NULL ) return;
        char * p = (char*)&lVer;
        _snwprintf(pwszBuf, lBufSize, L"%d.%d.%d.%d", *p, *(p + 1), *(p + 2), *(p + 3));
    }

    static long GetIntVer(const wchar_t * pwszVer)
    {
        unsigned long newVer[4] = { 0 };
        swscanf(pwszVer, L"%d.%d.%d.%d", &newVer[0], &newVer[1], &newVer[2], &newVer[3]);
        return ((newVer[0] << 24) | ((newVer[1] & 0x000000FF) << 16) | ((newVer[2] & 0x000000FF) << 8) | (newVer[3] & 0x000000FF));
    }
};
