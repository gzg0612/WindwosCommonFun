#pragma once
#include <Windows.h>

namespace NSENCODE
{
    class CCodeTransfer
    {
    public:
        CCodeTransfer(void) : m_psz(NULL) {}
        ~CCodeTransfer()
        {
            if ( m_psz != NULL ) free(m_psz);
        };
    private:
        char * m_psz;

    public:

        const char * UnicodeToAnsi(const wchar_t * pwsz)
        {
            long lenW = ::WideCharToMultiByte(CP_ACP, 0, pwsz, -1, NULL, 0, NULL, NULL);
            if ( lenW <= 0L ) return NULL;
            if ( m_psz != NULL ) free(m_psz);
            m_psz = (char *)malloc(lenW);
            ::WideCharToMultiByte(CP_ACP, 0, pwsz, -1, m_psz, lenW, NULL, NULL);
            return m_psz;
        }

        const wchar_t * AnsiToUnicode(const char * psz)
        {
            long lenA = ::MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0);
            if ( lenA <= 0L ) return NULL;
            if ( m_psz != NULL ) free(m_psz);
            m_psz = (char *)malloc(lenA + lenA); // 2*(len)
            ::MultiByteToWideChar(CP_ACP, 0, psz, -1, (wchar_t*)m_psz, lenA);
            return (wchar_t*)m_psz;
        }

        const char * UnicodeToUTF8(const wchar_t * pwsz)
        {
            long lenA = ::WideCharToMultiByte(CP_UTF8, 0, pwsz, -1, NULL, 0, NULL, NULL);
            if ( lenA <= 0L ) return NULL;
            if ( m_psz != NULL ) free(m_psz);
            m_psz = (char *)malloc(lenA);
            ::WideCharToMultiByte(CP_UTF8, 0, pwsz, -1, m_psz, lenA, NULL, NULL);
            return m_psz;
        }

        const wchar_t * UTF8ToUnicode(const char * psz)
        {
            long lenW = ::MultiByteToWideChar(CP_UTF8, NULL, psz, -1, NULL, 0);
            if ( lenW <= 0L ) return NULL;
            if ( m_psz != NULL ) free(m_psz);
            m_psz = (char *)malloc(lenW + lenW);    // lenW*sizeof(wchar_t)
            ::MultiByteToWideChar(CP_UTF8, 0, psz, -1, (wchar_t *)m_psz, lenW);
            return (const wchar_t *)m_psz;
        }

        const char * UTF8ToAnsi(const char * pszUTF8)
        {
            CCodeTransfer c;
            return UnicodeToAnsi(c.UTF8ToUnicode(pszUTF8));
        }

        const char * AnsiToUTF8(const char * psz)
        {
            CCodeTransfer c;
            return UnicodeToUTF8(c.AnsiToUnicode(psz));
        }
    };
};

using namespace NSENCODE;
