#ifndef _CRASH_DUMPER_H_
#define _CRASH_DUMPER_H_

#include <windows.h>
#include <tchar.h>
#include <shlobj.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

#if _MSC_VER > 1800 && _WIN32_WINNT == 0x0501
#include "DbgHelp2015.h"
#else
#include <dbghelp.h>
#endif
#pragma comment(lib, "dbghelp.lib")
#pragma warning(push)

class CMiniDumper
{

public:

    CMiniDumper(const wchar_t* pDumpPath = NULL)
    {
        if (NULL != pDumpPath)
        {
            wcscpy_s(m_strDumpPath, pDumpPath);
        }
		else
		{
			::GetModuleFileName(NULL, m_strDumpPath, MAX_PATH);
            ::PathRemoveFileSpecW(m_strDumpPath);
            ::PathAppendW(m_strDumpPath, L"\\Dump");
            ::SHCreateDirectoryEx(NULL, m_strDumpPath, NULL);
		}
        m_OriginalFilter = SetUnhandledExceptionFilter( ExceptionFilter );
    }

    virtual ~CMiniDumper()
    {
        SetUnhandledExceptionFilter( m_OriginalFilter );
    }

private:

    LPTOP_LEVEL_EXCEPTION_FILTER m_OriginalFilter;
    static LONG WINAPI ExceptionFilter( struct _EXCEPTION_POINTERS* ExceptionInfo );
    static wchar_t m_strDumpPath[MAX_PATH];
};

wchar_t CMiniDumper::m_strDumpPath[MAX_PATH] = {0};

BOOL CALLBACK MyMiniDumpCallback(
								 PVOID                            pParam, 
								 const PMINIDUMP_CALLBACK_INPUT   pInput, 
								 PMINIDUMP_CALLBACK_OUTPUT        pOutput 
								 ); 

LONG WINAPI CMiniDumper::ExceptionFilter(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
    DWORD dwProcess = GetCurrentProcessId();
    HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, dwProcess );
    if (hProcess != INVALID_HANDLE_VALUE)
    {
        if (wcslen(m_strDumpPath) == 0 )
        {
            return EXCEPTION_EXECUTE_HANDLER;
        }

        if (!::PathFileExists(m_strDumpPath))
        {
            ::SHCreateDirectoryEx( NULL, m_strDumpPath, NULL);
        }

        SYSTEMTIME stLocal;
        ::GetLocalTime( &stLocal );
        wchar_t szPath[MAX_PATH * 2 ] = { 0 };
        wsprintfW( szPath, L"%s\\%d-%d-%d_%d-%d-%d_%d.dmp", m_strDumpPath, stLocal.wYear, stLocal.wMonth, stLocal.wDay, stLocal.wHour, stLocal.wMinute, stLocal.wSecond, GetCurrentProcessId() );

        HANDLE hFile = CreateFile( szPath, FILE_ALL_ACCESS, 0, NULL, CREATE_ALWAYS, NULL, NULL );
        if ( hFile != INVALID_HANDLE_VALUE )
        {
            /*MINIDUMP_EXCEPTION_INFORMATION exception_information;
            exception_information.ThreadId = GetCurrentThreadId();
            exception_information.ExceptionPointers = ExceptionInfo;
            exception_information.ClientPointers = TRUE;
            MiniDumpWriteDump( hProcess, dwProcess, hFile, MiniDumpNormal, &exception_information, NULL, NULL );*/

			MINIDUMP_EXCEPTION_INFORMATION mdei; 

			mdei.ThreadId           = GetCurrentThreadId(); 
			mdei.ExceptionPointers  = ExceptionInfo; 
			mdei.ClientPointers     = TRUE; 

			/*MINIDUMP_CALLBACK_INFORMATION mci; 

			mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)MyMiniDumpCallback; 
			mci.CallbackParam       = 0;

			MINIDUMP_TYPE mdt       = (MINIDUMP_TYPE)(MiniDumpScanMemory|MiniDumpWithIndirectlyReferencedMemory); 

			MiniDumpWriteDump( hProcess, dwProcess, hFile, mdt, &mdei, NULL, &mci ); */

			MINIDUMP_TYPE mdt       = (MINIDUMP_TYPE)(MiniDumpWithDataSegs|MiniDumpWithProcessThreadData|MiniDumpWithPrivateReadWriteMemory); 

			MiniDumpWriteDump( hProcess, dwProcess, hFile, mdt, &mdei, NULL, NULL ); 

            CloseHandle( hFile );
        }
        CloseHandle( hProcess );
    }
    return EXCEPTION_EXECUTE_HANDLER;
}

BOOL CALLBACK MyMiniDumpCallback(
								 PVOID                            pParam, 
								 const PMINIDUMP_CALLBACK_INPUT   pInput, 
								 PMINIDUMP_CALLBACK_OUTPUT        pOutput 
								 ) 
{
	BOOL bRet = FALSE; 


	// Check parameters 

	if( pInput == 0 ) 
		return FALSE; 

	if( pOutput == 0 ) 
		return FALSE; 


	// Process the callbacks 

	switch( pInput->CallbackType ) 
	{
	case IncludeModuleCallback: 
		{
			// Include the module into the dump 
			bRet = TRUE; 
		}
		break; 

	case IncludeThreadCallback: 
		{
			// Include the thread into the dump 
			bRet = TRUE; 
		}
		break; 

	case ModuleCallback: 
		{
			// Does the module have ModuleReferencedByMemory flag set ? 

			if( !(pOutput->ModuleWriteFlags & ModuleReferencedByMemory) ) 
			{
				// No, it does not - exclude it 

				wprintf( L"Excluding module: %s \n", pInput->Module.FullPath ); 

				pOutput->ModuleWriteFlags &= (~ModuleWriteModule); 
			}

			bRet = TRUE; 
		}
		break; 

	case ThreadCallback: 
		{
			// Include all thread information into the minidump 
			bRet = TRUE;  
		}
		break; 

	case ThreadExCallback: 
		{
			// Include this information 
			bRet = TRUE;  
		}
		break; 

	case MemoryCallback: 
		{
			// We do not include any information here -> return FALSE 
			bRet = TRUE; 
		}
		break; 
	}

	return bRet; 

}
#pragma warning(pop)
#endif