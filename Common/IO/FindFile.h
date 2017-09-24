#pragma once
#include <Windows.h>

#pragma warning(push)
#pragma warning(disable:4996)

class CFindFile
{
public:
    CFindFile(void) : m_bExit(FALSE) { }
    ~CFindFile(void) { m_bExit = true; }

private:
    BOOL m_bExit;

public:
    typedef long (__stdcall *PFN_OnFindFile)(bool bDirectory, const wchar_t * pwszPath, const wchar_t * pwszFile, WPARAM wParam, LPARAM lParam);

    void FindFile(const bool bTraverse, const wchar_t * const pwszPath, PFN_OnFindFile pfnCallBack, WPARAM wParam = NULL, LPARAM lParam = NULL)
    {

        wchar_t wszFind[MAX_PATH] = {0};
        WIN32_FIND_DATAW findFileData;

        wcscpy(wszFind, pwszPath);
        wcscat(wszFind, L"\\*.*" );	

        HANDLE hFind = ::FindFirstFileW( wszFind, &findFileData );
        if ( INVALID_HANDLE_VALUE == hFind || m_bExit )
        {
            return;
        }

        BOOL bRet = TRUE;
        while ( bRet && !m_bExit )
        {
            if ( findFileData.cFileName[0] != L'.' )
            {
                if ( findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
                {				
                    if (bTraverse)
                    {
                        wchar_t wszPath[MAX_PATH];
                        _snwprintf(wszPath, MAX_PATH, L"%s\\%s\0", pwszPath, findFileData.cFileName);
                        FindFile( bTraverse, wszPath, pfnCallBack, wParam, lParam );
                    }
                    m_bExit = pfnCallBack(true, pwszPath, findFileData.cFileName, wParam, lParam);
                }
                else
                {
                    m_bExit = pfnCallBack(false, pwszPath, findFileData.cFileName, wParam, lParam);
                }
            }
            bRet = ::FindNextFile( hFind, &findFileData );
        }

        ::FindClose( hFind );
    }
};

#pragma warning(pop)