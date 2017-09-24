#pragma once

#include <windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

class CFileMapping
{
public:
    CFileMapping(void) : m_lBufSize(0L), m_pBuf(NULL), m_hMapFile(NULL) {}
    ~CFileMapping(void) { FreeMem(); }

private:

    long    m_lBufSize;
    char *  m_pBuf;
    HANDLE  m_hMapFile;

public:

    const long GetBufSize() { return m_lBufSize; }
    const void SetBufSize(const long lBufSize) { m_lBufSize = lBufSize; }

    char * GetBuf() { return m_pBuf; }

    const void FreeMem()
    {
        m_lBufSize = 0L;
        if (m_pBuf != NULL) { ::UnmapViewOfFile(m_pBuf); m_pBuf = NULL; }
        if (m_hMapFile != NULL) { ::CloseHandle(m_hMapFile); m_hMapFile = NULL; }
    }

    const bool InitFileMapping(const wchar_t * pwszName, const long lBufSize = 0)
    {
        if (m_pBuf != NULL) return true;
        if (pwszName == NULL || wcslen(pwszName) < 0L) return false;

        m_lBufSize = (lBufSize > 0 ? lBufSize : 8 * 1024);

        m_hMapFile = ::OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, pwszName);
        if (m_hMapFile == NULL)
        {
            m_hMapFile = ::CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, m_lBufSize, pwszName);
            if (m_hMapFile == NULL) return false;
        }

        m_pBuf = (char*)::MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, m_lBufSize);
        if (m_pBuf == NULL) 
        {
            FreeMem(); 
            return false;
        }
        return true;
    }

    const bool FindFileMapping(const wchar_t * pwszName, const long lBufSize)
    {
        if (m_pBuf != NULL) return true;
        if (pwszName == NULL || wcslen(pwszName) < 0L) return false;

        m_lBufSize = lBufSize;
        m_hMapFile = ::OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, pwszName);
        if (m_hMapFile == NULL) return false;

        m_pBuf = (char*)::MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (m_pBuf == NULL) 
        {
            FreeMem();
            return false;
        }
        return true;
    }

    const long Write(const char * pszBuf, const long lWriteLength, const long lOffset = 0L)
    {
        //if (m_pBuf == NULL || lWriteLength <= 0L || lOffset < 0) return 0L;
        long lenWrite = min(m_lBufSize, lOffset + lWriteLength) - lOffset;
        //if (lenWrite <= 0L) return 0L;
        if (pszBuf == NULL)
        {
            // clear buf
            memset(m_pBuf + lOffset, 0x00, lenWrite);
        }
        else
        {
            // copy data
            memcpy(m_pBuf + lOffset, pszBuf, lenWrite);
        }       
        return lenWrite;
    }

    const long Read(OUT char * pszBuf, const long lReadLength, const long lOffset = 0L)
    {
        //if (m_pBuf == NULL || pszBuf == NULL || lReadLength < 0 || lOffset < 0L) return false;
        long lenRead = min(m_lBufSize, lOffset + lReadLength) - lOffset;
        memcpy(pszBuf, m_pBuf + lOffset, lenRead);
        return lenRead;
    }
};