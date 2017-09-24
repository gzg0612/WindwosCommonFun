#pragma once

#include <vector>
#include <string>
#include <ShlObj.h>
#include <Windows.h>
#include <wininet.h>  
#pragma comment(lib,"wininet.lib")
#pragma warning(push)
#pragma warning(disable:4996)

#include "IDownload.h"
#include "IUpload.h"

namespace NSNET
{

#define GETDOWNLOADER() NSNET::CDownLoader::GetInstance()

    static const unsigned long CONST_INTERNET_BUF_LENGTH = 256*1024;
    static const unsigned short HTTP_STATE_CODE_200 = 200;
    static const unsigned long HTTP_TIMEOUT_TIME = 10000;
    static const unsigned long THREAD_HIGH_PRIORITY_BEGIN = 0;
    static const unsigned long THREAD_HIGH_PRIORITY_END = 1;
    static bool g_bRun = true;

    inline void * CheckHttpOnCallback(void * p);
    inline unsigned int __stdcall ThreadProcDownload( void * p );

    inline DWORD _HttpQueryInfo(HINTERNET hRequest, const long lType, std::string & strInfo, DWORD dwIndex = 0)
    {
        if (hRequest == NULL) return ERROR_HTTP_HEADER_NOT_FOUND;
        LPVOID lpOutBuffer = NULL;
        DWORD dwSize = 0;
        DWORD dwErr = 0;
        if(!::HttpQueryInfoA(hRequest,lType,(LPVOID)lpOutBuffer,&dwSize,&dwIndex))
        {
            if ( ((dwErr = ::GetLastError()) == ERROR_INSUFFICIENT_BUFFER) && dwSize > 0)
            {
                // Allocate the necessary buffer.
                lpOutBuffer = (char*)malloc(dwSize);
                if(::HttpQueryInfoA(hRequest,lType,(LPVOID)lpOutBuffer,&dwSize,&dwIndex))
                {
                    strInfo.append((char*)lpOutBuffer);
                    strInfo.append("\r\n");
                    free((char*)lpOutBuffer);
                }
            }
        }
        return dwErr;
    }

    class CHttp
    {
    public:
        CHttp(void) : m_bStop(false), m_i64ContentLength(0x7FFFFFFFFFFFFFFF), m_i64ReadCount(0), m_hInternetOpen(NULL), m_hInternetOpenUrl(NULL), m_hEventStop(NULL), m_hEventOpenUrl(NULL), m_pbuffer(NULL), m_wstrUrl(L"") {}
        ~CHttp(void) 
        {
            Stop();
            if (m_hEventStop != NULL) ::CloseHandle(m_hEventStop);
            if (m_hEventOpenUrl != NULL) ::CloseHandle(m_hEventOpenUrl);
            if (NULL != m_pbuffer) free(m_pbuffer);
        }

    private:
        bool		m_bStop;
        __int64		m_i64ContentLength;
        __int64		m_i64ReadCount;
        HINTERNET	m_hInternetOpen;
        HINTERNET	m_hInternetOpenUrl;
        HANDLE      m_hEventStop;
        HANDLE      m_hEventOpenUrl;
        char    *   m_pbuffer;
        std::wstring    m_wstrUrl;
    private:

        void _ResetHandle()
        {
            m_i64ContentLength = 0x7FFFFFFFFFFFFFFF;
            m_i64ReadCount = 0;
            if (m_hInternetOpenUrl != NULL) { ::InternetCloseHandle(m_hInternetOpenUrl); m_hInternetOpenUrl = NULL; }
            if (m_hInternetOpen != NULL) { ::InternetSetStatusCallback(m_hInternetOpen, NULL); ::InternetCloseHandle(m_hInternetOpen); m_hInternetOpen = NULL; }
        }

        FILE * _GetTempFileHandle(wchar_t * pwszTempFilePath, unsigned long ulSize = MAX_PATH)
        {
            wchar_t wszModulePath[MAX_PATH*2];
            ::GetModuleFileNameW(NULL, wszModulePath, MAX_PATH*2);
            ::PathRemoveExtensionW(wszModulePath);
            wchar_t * pwszFileName = ::PathFindFileNameW(wszModulePath);

            SYSTEMTIME st;
            ::GetLocalTime(&st);

            // try save file to %temp%
            wchar_t wszTempPath[MAX_PATH*2];
            ::GetTempPathW(MAX_PATH, wszTempPath);
            _snwprintf(pwszTempFilePath, MAX_PATH*2, L"%s\\%s\\%d%d%d\\", wszTempPath, pwszFileName, st.wYear, st.wMonth, st.wDay);
            ::SHCreateDirectoryExW(NULL, pwszTempFilePath, NULL);
            ::GetTempFileNameW(pwszTempFilePath, L"D", 0, pwszTempFilePath);

            FILE * f = _wfopen(pwszTempFilePath, L"w+b");
            if (f != NULL) return f;

            // can not read temp file, save file to module path
            ::PathRemoveFileSpecW(wszModulePath);
            _snwprintf(wszTempPath, MAX_PATH*2, L"%s\\DownTemp\\%d%d%d\\", wszModulePath, st.wYear, st.wMonth, st.wDay);
            ::SHCreateDirectoryExW(NULL, wszTempPath, NULL);
            ::GetTempFileNameW(wszTempPath, L"D", 0, pwszTempFilePath);

            f = _wfopen(pwszTempFilePath, L"w+b");
            return f;
        }

    public:
        static void CALLBACK InternetStatusCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus,
            LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
        {
            if (!g_bRun) return;

            switch (dwInternetStatus)
            {
            case INTERNET_STATUS_HANDLE_CREATED:
                {
                    CHttp * pHttp = (CHttp*)CheckHttpOnCallback((void*)dwContext);
                    if (pHttp != NULL)
                    {
                        pHttp->m_hInternetOpenUrl = (HINTERNET)(((LPINTERNET_ASYNC_RESULT)(lpvStatusInformation))->dwResult); 
                    }
                    break;
                }
            case INTERNET_STATUS_REQUEST_COMPLETE:
                {
                    //CHttp* pObj= (CHttp*)dwContext;
                    CHttp * pHttp = (CHttp*)CheckHttpOnCallback((void*)dwContext);
                    if (pHttp != NULL && pHttp->m_hEventOpenUrl)
                    {
                        ::SetEvent(pHttp->m_hEventOpenUrl);
                    }                    
                    //if   (ERROR_SUCCESS   ==   ((LPINTERNET_ASYNC_RESULT)(lpvStatusInformation))->dwError)
                    //{   //设置句柄被创建事件或者读数据成功完成事件
                    //    ::SetEvent(pObj->m_hEventOpenUrl);
                    //}   
                    //else   
                    //{   //如果发生错误，则设置子线程退出事件
                    //    //这里也是一个陷阱，经常会忽视处理这个错误，
                    //    DWORD dw = ((LPINTERNET_ASYNC_RESULT)(lpvStatusInformation))->dwError;
                    //    ATLTRACE(L"INTERNET_STATUS_REQUEST_COMPLETE error: %d\r\n", dw);
                    //}   
                    break; 
                }               
            default:
                break;
            }
        }
        void Stop() 
        {
            m_bStop = true; 
            if (m_hInternetOpen != NULL) { _ResetHandle(); ::WaitForSingleObject(m_hEventStop, INFINITE); } 
        }
        __int64 GetContentLength() { return m_i64ContentLength; }
        __int64 GetReadCount() { return m_i64ReadCount; }
        long GetProgress() { return min(100, (long)(100*m_i64ReadCount/m_i64ContentLength)); }
        wstring GetUrl(){return m_wstrUrl; }

        bool Download(const wchar_t * pwszURL, const wchar_t * pwszSavePath, const long lState, IDownloadNotify * pIDownloadNotify, WPARAM wParam, LPARAM lParam)
        {
            m_bStop = false;
            if (pwszURL == NULL || wcslen(pwszURL) <= 0L) return false;
            if (m_pbuffer == NULL) m_pbuffer = (char *)malloc(CONST_INTERNET_BUF_LENGTH+1);
            if (m_hEventStop == NULL) m_hEventStop = ::CreateEvent(NULL, FALSE, FALSE, NULL); 
            if (m_hEventOpenUrl == NULL) m_hEventOpenUrl = ::CreateEvent(NULL, FALSE, FALSE, NULL);
            //memset(m_pbuffer, 0x00, CONST_INTERNET_BUF_LENGTH);
            _ResetHandle();
            m_wstrUrl = pwszURL;
            m_hInternetOpen = ::InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_ASYNC);  

            if (m_bStop || m_hInternetOpen == NULL)  
            {
                ATLTRACE("InternetOpen failed. lasterror = %d\n", GetLastError());
                _ResetHandle();
                ::SetEvent(m_hEventStop);
                return false;
            }
            INTERNET_STATUS_CALLBACK res = ::InternetSetStatusCallback(m_hInternetOpen, InternetStatusCallback);

            if (res == INTERNET_INVALID_STATUS_CALLBACK)
            {
                _ResetHandle();
                ::SetEvent(m_hEventStop);
                return false;
            }

            ::InternetOpenUrlW(m_hInternetOpen, pwszURL, NULL, 0, INTERNET_FLAG_RELOAD, (DWORD_PTR)this);
            ::WaitForSingleObject(m_hEventOpenUrl, HTTP_TIMEOUT_TIME);//INFINITE);

            if (m_bStop || m_hInternetOpenUrl == NULL)
            {
                ATLTRACE("InternetOpenUrl failed. lasterror = %d URL[%S]\n", GetLastError(), pwszURL);
                _ResetHandle();
                ::SetEvent(m_hEventStop);
                return false;
            }

            // query http state
            std::string strStateCode;
            _HttpQueryInfo(m_hInternetOpenUrl, HTTP_QUERY_STATUS_CODE, strStateCode);
            long lHttpStateCode = atol(strStateCode.c_str());

            // create temp file
            FILE * pfSaveTemp = NULL;
            wchar_t wszTempFilePath[MAX_PATH];
            if (lHttpStateCode == HTTP_STATE_CODE_200 && pwszSavePath != NULL && wcslen(pwszSavePath) > 0L) pfSaveTemp = _GetTempFileHandle(wszTempFilePath); 

            BOOL bRet = TRUE;
            DWORD dwHttpRead = CONST_INTERNET_BUF_LENGTH;

            // query download length
            bRet = ::HttpQueryInfoA(m_hInternetOpenUrl, HTTP_QUERY_CONTENT_LENGTH, m_pbuffer, &dwHttpRead, NULL);
            if (bRet && dwHttpRead > 0L) { m_pbuffer[dwHttpRead] = 0x00; m_i64ContentLength = _atoi64(m_pbuffer); }

            // verify http code state
            bRet = lHttpStateCode == HTTP_STATE_CODE_200;

            // start downloading
            for ( ; !m_bStop && bRet; )
            {
                memset(m_pbuffer, 0x00, dwHttpRead);
                dwHttpRead = 0;
                bRet = ::InternetReadFile(m_hInternetOpenUrl, m_pbuffer, CONST_INTERNET_BUF_LENGTH, &dwHttpRead);
                if (!bRet && ::GetLastError() == ERROR_IO_PENDING)
                {
                    // async, when ERROR_IO_PENDING, wait INTERNET_STATUS_REQUEST_COMPLETE
                    // on INTERNET_STATUS_REQUEST_COMPLETE, write data to buf and set read number
                    DWORD dwRet = ::WaitForSingleObject(m_hEventOpenUrl, HTTP_TIMEOUT_TIME);
                    if (dwRet == WAIT_TIMEOUT) 
                    {
                        ATLTRACE("Wait REQUEST_COMPLETE timeout, dwHttpRead=%d [URL:%S]\n", dwHttpRead, pwszURL);
                        break;
                    }
                    bRet = TRUE;    // set bRet = TRUE, for continue
                }
               
                if(m_bStop || !bRet || dwHttpRead == 0) break;
                m_i64ReadCount = min(m_i64ReadCount + dwHttpRead, m_i64ContentLength);

                if (pfSaveTemp != NULL)
                {
                    bRet = (dwHttpRead == fwrite(m_pbuffer, 1, dwHttpRead, pfSaveTemp));
                    if (!bRet) break;
                }

                if (pIDownloadNotify != NULL && !m_bStop) { pIDownloadNotify->OnDownloadNotify(EN_DOWNLING, pwszURL, pwszSavePath, m_pbuffer, dwHttpRead, GetProgress(), wParam, lParam); }
            }

            if (m_bStop) bRet = FALSE;

            // copy temp file to dest
            if (pfSaveTemp != NULL) 
            { 
                fclose(pfSaveTemp); 
                if (bRet) 
                {
                    wchar_t wszPath[MAX_PATH];
                    ::PathCombineW(wszPath, pwszSavePath, NULL);
                    ::PathRemoveFileSpecW(wszPath);
                    ::SHCreateDirectoryEx(NULL, wszPath, NULL);
                    ::CopyFileW(wszTempFilePath, pwszSavePath, FALSE);   // only copy on Success
                }
                _wremove(wszTempFilePath);  // remove temp file
            }

            m_wstrUrl = L"";
            _ResetHandle();
            ::SetEvent(m_hEventStop);
            return (bRet != FALSE);
        }
    };

    // ------------------------- CDownLoader ------------------------
    class CDownLoader : public IDownloader
    {
    private:
        CDownLoader(void) : m_bExit(false), m_lThreadCount(6L), m_pAryThread(NULL), m_pAryEvent(NULL), m_pAryHttp(NULL), m_pCS(NULL), m_pVecTask(NULL) {}
        ~CDownLoader(void) { _FreeMem(); }

        // ------------- static -------------
    private:
        static CDownLoader * m_pInstance;

    public:
        static CDownLoader * const GetInstance()
        {
            if (m_pInstance == NULL)
            {
                m_pInstance = new CDownLoader;
                m_pInstance->Init();
            }
            return m_pInstance;
        }

        static void Destroy()
        {
            if (m_pInstance != NULL) { delete m_pInstance; m_pInstance = NULL; }
        }

    private:

        friend class CHttp;
        typedef struct _stru_download_task_info
        {
            char            iTryCount;
            char            iCurTryCount;
            short           iPriority;
            volatile long	lState;
            WPARAM			wParam;
            LPARAM			lParam;
            IDownloadNotify *pIDownloadNotify;
            std::wstring	wstrURL;
            std::wstring 	wstrSavePath;
        } STDLTASKINFO, *LPSTDLTASKINFO;

        bool			m_bExit;
        long			m_lThreadCount;
        long		*	m_pAryIndex;
        HANDLE		*	m_pAryThread;
        HANDLE		*	m_pAryEvent;
        CHttp		*	m_pAryHttp;
        CRITICAL_SECTION			* m_pCS;
        std::vector<STDLTASKINFO*>	* m_pVecTask;

    private:

        void _FreeMem()
        {
            g_bRun = false;
            m_bExit = true;
            if (m_pAryHttp != NULL) { for (long l = 0L; l < m_lThreadCount; l++) m_pAryHttp[l].Stop(); }
            if (m_pAryEvent != NULL) { for (long l = 0; l < m_lThreadCount; l++) ::SetEvent(m_pAryEvent[l]); }
            Sleep(1);
            if (m_pAryThread != NULL) { ::WaitForMultipleObjects(m_lThreadCount, m_pAryThread, TRUE, INFINITE); }
            for (long l = 0; l < m_lThreadCount; l++)
            {
                if (m_pAryEvent != NULL) ::CloseHandle(m_pAryEvent[l]);
                if (m_pAryThread != NULL) ::CloseHandle(m_pAryThread[l]);
            }
            if (m_pAryEvent != NULL) { delete[] m_pAryEvent; m_pAryEvent = NULL; }
            if (m_pAryThread != NULL) { delete[] m_pAryThread; m_pAryThread = NULL; }
            if (m_pAryIndex != NULL) { delete[] m_pAryIndex; m_pAryIndex = NULL; }
            if (m_pAryHttp != NULL) { delete[] m_pAryHttp; m_pAryHttp = NULL; }
            std::vector<STDLTASKINFO*> & vec = *m_pVecTask;
            if (m_pVecTask != NULL)
            {
                for (long l = 0L; l < (long)m_pVecTask->size(); l++ )
                {
                    STDLTASKINFO * pTask = vec[l];
                    if (pTask->pIDownloadNotify != NULL && pTask->lState != NSNET::EN_FINISH && pTask->lState != NSNET::EN_FAILED && pTask->lState != NSNET::EN_DEL_TASK)
                    {
                        pTask->pIDownloadNotify->OnDelTask(pTask->wstrURL.c_str(), pTask->wstrSavePath.c_str(), pTask->wParam, pTask->lParam);
                    }
                    delete pTask;
                }
                m_pVecTask->clear();
                std::vector<STDLTASKINFO*>().swap(*m_pVecTask);
                delete m_pVecTask;
                m_pVecTask = NULL;
            }
            if (m_pCS != NULL) { ::DeleteCriticalSection(m_pCS); delete m_pCS; m_pCS = NULL; }
        }

    public:

        bool Init()
        {
            if (m_pCS == NULL)
            {
                m_pCS = new CRITICAL_SECTION;
                if (!InitializeCriticalSectionAndSpinCount(m_pCS, 0x80000400))
                {
                    delete m_pCS; 
                    m_pCS = NULL;
                    return false;
                }
            }

            m_pVecTask = new std::vector<STDLTASKINFO*>;
            m_pAryHttp = new CHttp[m_lThreadCount];
            m_pAryThread = new HANDLE[m_lThreadCount];
            m_pAryEvent = new HANDLE[m_lThreadCount];
            m_pAryIndex = new long[m_lThreadCount];

            DWORD dwThreadID = 0;
            for (long l = 0; l < m_lThreadCount; l++)
            {
                m_pAryIndex[l] = l;
                m_pAryEvent[l] = ::CreateEvent(NULL, FALSE, FALSE, NULL);
                m_pAryThread[l] = (HANDLE)_beginthreadex(NULL, 0, ThreadProcDownload, &m_pAryIndex[l], 0, (unsigned int *)&dwThreadID);
            }
            return true;
        }

        STDLTASKINFO * const GetWaitTask(const long lThreadIndex)
        {
            STDLTASKINFO * pTaskRet = NULL;
            STDLTASKINFO * p = NULL;
            ::EnterCriticalSection(m_pCS);
            std::vector<STDLTASKINFO*> & vec = *m_pVecTask;
            for (long l = 0L; l < (long)vec.size(); l++)
            {
                p = vec[l];
                if (p->lState == EN_WAIT)
                {
                    // find waiting task
                    if ( (p->iPriority == EN_PRIORITY_HIGH && (lThreadIndex<THREAD_HIGH_PRIORITY_BEGIN || lThreadIndex>THREAD_HIGH_PRIORITY_END)) ||
                         (p->iPriority != EN_PRIORITY_HIGH && (lThreadIndex>=THREAD_HIGH_PRIORITY_BEGIN && lThreadIndex<=THREAD_HIGH_PRIORITY_END)) )
                    {
                        // thread 0 only use for high priority tasks
                        continue;
                    }
                    p->lState = EN_BEGIN;  // change state
                    pTaskRet = p;
                    break;
                }
                else if (p->lState == EN_DEL_TASK)
                {
                    // delete task
                    vec.erase(vec.begin() + l);
                    delete p;
                }
            }
            ::LeaveCriticalSection(m_pCS);
            return pTaskRet;
        }

        virtual long AddDownloadTask(const wchar_t * pwszURL, const wchar_t * pwszSavePath, IDownloadNotify * pIDownloadNotify, WPARAM wParam = NULL, LPARAM lParam = NULL, const short iPriority = EN_PRIORITY_NORMAL, const char iRetryCount = CONST_DEFAULT_TRY_COUNT)
        {
            if (pwszURL == NULL || wcslen(pwszURL) <= 0L) return NULL;

            STDLTASKINFO * p = NULL;
            bool bFound = false;
            ::EnterCriticalSection(m_pCS);
            std::vector<STDLTASKINFO*> & vec = *m_pVecTask;
            for (long l = 0L; l < (long)m_pVecTask->size(); l++)
            {
                p = vec[l];
                if (wcsicmp(pwszURL, p->wstrURL.c_str()) == 0)
                {
                    bFound = true;
                    break;
                }
            }

            if (!bFound)
            {
                p = new STDLTASKINFO;
                p->lState = EN_WAIT;
                p->iPriority = iPriority;
                p->pIDownloadNotify = pIDownloadNotify;
                p->wParam = wParam;
                p->lParam = lParam;
                p->iTryCount = iRetryCount;
                p->iCurTryCount = 0L;
                p->wstrURL = pwszURL;
				p->wstrSavePath = (pwszSavePath == NULL ? L"" : pwszSavePath);
                m_pVecTask->push_back(p);
            }

            ::LeaveCriticalSection(m_pCS);
            for (long l = 0L; l < m_lThreadCount; l++) { ::SetEvent(m_pAryEvent[l]); }
            Sleep(1);
            return 0L;
        }

        virtual long ClearAllTask()
        {
            ::EnterCriticalSection(m_pCS);
            std::vector<STDLTASKINFO*> & vec = *m_pVecTask;
            for (long l = 0L; l < (long)m_pVecTask->size(); l++)
            {
                if (vec[l]->lState == EN_WAIT)
                {
                    vec[l]->lState = EN_DEL_TASK;
                }
            }
            ::LeaveCriticalSection(m_pCS);
            Sleep(1);
            return 0L;
        }

        virtual long CancelRunningTask(const wchar_t * pwszURL)
        {
            for (long l = 0; l < m_lThreadCount; ++l)
            {
                if (wcsicmp(pwszURL, m_pAryHttp[l].GetUrl().c_str()) == 0)
                {
                    m_pAryHttp[l].Stop();
                    break;
                }
            }

            return 0L;
        }

        STDLTASKINFO * const AddTask(STDLTASKINFO * pDLTaskInfo)
        {
            if ( pDLTaskInfo == NULL ) return NULL;
            ::EnterCriticalSection(m_pCS);
            m_pVecTask->push_back(pDLTaskInfo);
            ::LeaveCriticalSection(m_pCS);
            return pDLTaskInfo;
        }

        HANDLE GetEvent(const long lIndex) { return m_pAryEvent[lIndex]; }
        CHttp * GetHttp(const long lIndex) { return &m_pAryHttp[lIndex]; }

        unsigned long DownLoadOnThread(const long lThreadIndex)
        {
            HANDLE hWaitEvent = m_pAryEvent[lThreadIndex];
            CHttp * pHttp = &m_pAryHttp[lThreadIndex];
            STDLTASKINFO * pTaskInfo = NULL;
            long lRet = 0L;
            for (; !m_bExit; )
            {
                pTaskInfo = GetWaitTask(lThreadIndex);
                if (pTaskInfo == NULL) 
                {
                    // no waiting tasks, wait 2s
                    ::WaitForSingleObject(hWaitEvent, 2000);
                    continue;
                }
                if (m_bExit) break;

                if (pTaskInfo->pIDownloadNotify != NULL && !m_bExit) { lRet = pTaskInfo->pIDownloadNotify->OnDownloadNotify(EN_BEGIN, pTaskInfo->wstrURL.c_str(), pTaskInfo->wstrSavePath.c_str(), NULL, 0, pHttp->GetProgress(), pTaskInfo->wParam, pTaskInfo->lParam); }
                pTaskInfo->lState = EN_DOWNLING;
                if (pHttp->Download(pTaskInfo->wstrURL.c_str(), pTaskInfo->wstrSavePath.c_str(), pTaskInfo->lState, pTaskInfo->pIDownloadNotify, pTaskInfo->wParam, pTaskInfo->lParam)) 
                {
                    // success
                    //ATLTRACE("thread:%d [Success:%S]\n", lThreadIndex, pTaskInfo->wstrURL.c_str());
                    pTaskInfo->lState = EN_FINISH;
                    if (pTaskInfo->pIDownloadNotify != NULL && !m_bExit) { lRet = pTaskInfo->pIDownloadNotify->OnDownloadNotify(pTaskInfo->lState, pTaskInfo->wstrURL.c_str(), pTaskInfo->wstrSavePath.c_str(), NULL, 0, pHttp->GetProgress(), pTaskInfo->wParam, pTaskInfo->lParam); }
                    pTaskInfo->lState = EN_DEL_TASK;
                }
                else
                {
                    // failed
                    //ATLTRACE("thread:%d [Failed:%S]\n", lThreadIndex, pTaskInfo->wstrURL.c_str());
                    if (!m_bExit)
                    {
                        if (pTaskInfo->iTryCount < 0 || pTaskInfo->iCurTryCount < pTaskInfo->iTryCount) 
                        {
                            // iTryCount < 0 do not inc Current try count, infinite retry      
                            if (pTaskInfo->iTryCount > 0) ++pTaskInfo->iCurTryCount;
                            pTaskInfo->lState = EN_WAIT;
                        }
                        else
                        {
                            pTaskInfo->lState = EN_FAILED;
                            if (pTaskInfo->pIDownloadNotify != NULL && !m_bExit) { lRet = pTaskInfo->pIDownloadNotify->OnDownloadNotify(pTaskInfo->lState, pTaskInfo->wstrURL.c_str(), pTaskInfo->wstrSavePath.c_str(), NULL, 0, pHttp->GetProgress(), pTaskInfo->wParam, pTaskInfo->lParam); }
                            pTaskInfo->lState = EN_DEL_TASK;
                        }
                    }
                }                
            }
            return 0L;
        }

        void * CheckHttpInst(void * p)
        {
            for (long l = 0; l < m_lThreadCount; ++l)
            {
                if (&m_pAryHttp[l] == p) return p;
            }
            return NULL;
        }

    };

    __declspec(selectany) CDownLoader * CDownLoader::m_pInstance = NULL;

    inline void * CheckHttpOnCallback(void * p)
    {
        return CDownLoader::GetInstance()->CheckHttpInst(p);
    }

    inline unsigned int __stdcall ThreadProcDownload( void * p )
    {
        CDownLoader::GetInstance()->DownLoadOnThread(*(long*)p);
        //_endthreadex(0);
        return 0L;
    }

    inline unsigned int __stdcall ThreadProcUpload( void * p );
    class CHttpRQ
    {
    public:
        CHttpRQ(void) : m_bStop(false), m_hInternetOpen(NULL), m_hConnect(NULL), m_hRequest(NULL) {}
        ~CHttpRQ(void) { DisConnect(); }
    private:

        bool		m_bStop;
        HINTERNET	m_hInternetOpen;
        HINTERNET	m_hConnect;
        HINTERNET   m_hRequest;

    private:

        DWORD _OnError(HINTERNET hRequest = NULL)
        {
            DWORD dwErr = ::GetLastError();
            DisConnect();
            return dwErr;
        }

    private:
        DWORD _SendRequest(const char * pszVerbGetPost, const char * pszAction, const char * pszHeader = NULL, const char * pszReferer = NULL, const char * pszPostData = NULL, DWORD dwDataLen = 0)
        {
            if (m_hRequest != NULL) ::InternetCloseHandle(m_hRequest);
            const char * szAcceptedType[] = { "*/*" , NULL } ;
            m_hRequest = ::HttpOpenRequestA(m_hConnect,pszVerbGetPost,pszAction,"HTTP 1.1",pszReferer,szAcceptedType,INTERNET_FLAG_RELOAD,1);
            if (m_hRequest == NULL) return ::GetLastError();

            if (pszHeader != NULL)
            {
                ::HttpAddRequestHeadersA(m_hRequest, pszHeader, -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
            }

            DWORD dwLenPostData = 0L;
            if (pszPostData != NULL)
            {
                dwLenPostData = dwDataLen;//(DWORD)strlen(pszPostData);
                if (dwLenPostData > 0L)
                {
                    char szContentLength[64];
                    _snprintf(szContentLength, 64, "Content-Length: %d", dwLenPostData);
                    ::HttpAddRequestHeadersA(m_hRequest, szContentLength, -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
                }
            }

            //::HttpAddRequestHeadersA(m_hRequest, "Content-Type: application/x-www-form-urlencoded\r\n\r\n", -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);            
            if (!HttpSendRequestA(m_hRequest, pszHeader, -1, (LPVOID)pszPostData, (pszPostData == NULL ? 0 : dwLenPostData)))
            {
                DWORD dwErr = ::GetLastError();
                ::InternetCloseHandle(m_hRequest);
                m_hRequest = NULL;
                return dwErr;
            }
            return GetState();
        }

    public:

        const bool IsConnect() { return (m_hConnect != NULL); }

        void DisConnect()
        {
            if (m_hRequest != NULL) { ::InternetCloseHandle(m_hRequest); m_hRequest = NULL; }
            if (m_hConnect != NULL) { ::InternetCloseHandle(m_hConnect); m_hConnect = NULL; }
            if (m_hInternetOpen != NULL) { ::InternetCloseHandle(m_hInternetOpen); m_hInternetOpen = NULL; }
        }

        const DWORD Connect(const char * pszHost, const unsigned short nPort)
        {
            if (m_hInternetOpen == NULL)
            {
                if ((m_hInternetOpen = ::InternetOpen(NULL,INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,0)) == NULL)
                    return _OnError();
            }

            if (m_hConnect == NULL)
            {
                if ((m_hConnect = ::InternetConnectA(m_hInternetOpen,pszHost,nPort,NULL,NULL,INTERNET_SERVICE_HTTP,0,1)) == NULL)
                    return _OnError();
            }

            return 0L;
        }

        const DWORD Get( const char * pszAction, const char * pszHeader = NULL, const char * pszReferer = NULL )
        {
            return _SendRequest("GET", pszAction, pszHeader, pszReferer);
        }

        const DWORD Post( const char * pszAction, const char * pszHeader = NULL, const char * pszReferer = NULL, const char * pszPostData = NULL, DWORD dwDataLen = 0 )
        {
            return _SendRequest("POST", pszAction, pszHeader, pszReferer, pszPostData, dwDataLen);
        }

        const long GetState()
        {
            std::string strStateCode;
            _HttpQueryInfo(m_hRequest, HTTP_QUERY_STATUS_CODE, strStateCode);
            return atol(strStateCode.c_str());
        }

        void GetHeader(OUT std::string & strHeader)
        {
            _HttpQueryInfo(m_hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, strHeader);
        }

        void GetCookie(OUT std::string & strCookie)
        {
            DWORD dwIndex = 0;
            while(_HttpQueryInfo(m_hRequest, HTTP_QUERY_SET_COOKIE, strCookie, dwIndex++) != ERROR_HTTP_HEADER_NOT_FOUND);
        }

        void GetHtmlContent(OUT std::string & strHtml)
        {
            strHtml.clear();
            char szBuf[CONST_INTERNET_BUF_LENGTH+1];
            DWORD dwRead = 0;
            BOOL bRet = FALSE;
            for (; !m_bStop; )
            {
                bRet = ::InternetReadFile(m_hRequest, szBuf, CONST_INTERNET_BUF_LENGTH, &dwRead);
                if (!bRet || dwRead == 0 || m_bStop) break;
                szBuf[dwRead] = '\0';
                strHtml.append(szBuf);
            }
        }
        
        bool Upload(const char * pszHost, const unsigned short nPort, const char * pszAction, const char * pszHeader, const char * pszReferer, const char * pszPostData, DWORD dwDataLen,
                    const long lState, IUploadNotify * pIUploadNotify, WPARAM wParam, LPARAM lParam)
        {
            if ( Connect(pszHost, nPort) != 0 )
                return false;

            DWORD dwState = 0;

            if ( pszPostData != NULL && dwDataLen>0 )
            {
                dwState = Post(pszAction, pszHeader, NULL, pszPostData, dwDataLen);
            }
            else
            {
                dwState= Get(pszAction, pszHeader, NULL);
            }

            if (dwState != 200) return false;

            // success
            std::string strRet = "";
            char * pszBuf = NULL;
            GetHtmlContent(strRet);
            if (strRet.length()>0)
            {
                pszBuf = new char[strRet.length()+1]();
                memcpy(pszBuf, strRet.c_str(), strRet.length());
            }
            
            if ( pIUploadNotify != NULL ) 
            {
                pIUploadNotify->OnUploadNotify(EN_DOWNLING, pszHost, pszAction, pszBuf, strRet.length(), GetState(), wParam, lParam); 
            }

            delete pszBuf;

            return true;
        }
    };

    // ------------------------- CUpLoader ------------------------
    class CUpLoader : public IUploader
    {
    private:
        CUpLoader(void) : m_bExit(false), m_lThreadCount(6L), m_pAryThread(NULL), m_pAryEvent(NULL), m_pAryHttp(NULL), m_pCS(NULL), m_pVecTask(NULL) {}
        ~CUpLoader(void) { _FreeMem(); }

        // ------------- static -------------
    private:
        static CUpLoader * m_pInstance;

    public:
        static CUpLoader * const GetInstance()
        {
            if (m_pInstance == NULL)
            {
                m_pInstance = new CUpLoader;
                m_pInstance->Init();
            }
            return m_pInstance;
        }

        static void Destroy()
        {
            if (m_pInstance != NULL) { delete m_pInstance; m_pInstance = NULL; }
        }

    private:

        friend class CHttpRQ;

        typedef struct _stru_upload_task_info
        {
            char            iTryCount;
            char            iCurTryCount;
            short           iPriority;
            volatile long	lState;
            WPARAM			wParam;
            LPARAM			lParam;
            IUploadNotify * pIUploadNotify;
            std::string	    strURL;
            unsigned short  nPort;
            std::string     strAction;
            std::string     strHeader;
            long            lDataLen;
            char *          pszPostData;
        } STULTASKINFO, *LPSTULTASKINFO;

        bool			m_bExit;
        long			m_lThreadCount;
        long		*	m_pAryIndex;
        HANDLE		*	m_pAryThread;
        HANDLE		*	m_pAryEvent;
        CHttpRQ		*	m_pAryHttp;
        CRITICAL_SECTION			* m_pCS;
        std::vector<STULTASKINFO*>	* m_pVecTask;

    private:

        void _FreeMem()
        {
            g_bRun = false;
            m_bExit = true;
            if (m_pAryHttp != NULL) { for (long l = 0L; l < m_lThreadCount; l++) m_pAryHttp[l].DisConnect(); }
            if (m_pAryEvent != NULL) { for (long l = 0; l < m_lThreadCount; l++) ::SetEvent(m_pAryEvent[l]); }
            Sleep(1);
            if (m_pAryThread != NULL) { ::WaitForMultipleObjects(m_lThreadCount, m_pAryThread, TRUE, INFINITE); }
            for (long l = 0; l < m_lThreadCount; l++)
            {
                if (m_pAryEvent != NULL) ::CloseHandle(m_pAryEvent[l]);
                if (m_pAryThread != NULL) ::CloseHandle(m_pAryThread[l]);
            }
            if (m_pAryEvent != NULL) { delete[] m_pAryEvent; m_pAryEvent = NULL; }
            if (m_pAryThread != NULL) { delete[] m_pAryThread; m_pAryThread = NULL; }
            if (m_pAryIndex != NULL) { delete[] m_pAryIndex; m_pAryIndex = NULL; }
            if (m_pAryHttp != NULL) { delete[] m_pAryHttp; m_pAryHttp = NULL; }
            std::vector<STULTASKINFO*> & vec = *m_pVecTask;
            if (m_pVecTask != NULL)
            {
                for (long l = 0L; l < (long)m_pVecTask->size(); l++ )
                {
                    STULTASKINFO * pTask = vec[l];
                    if (pTask->pIUploadNotify != NULL) pTask->pIUploadNotify->OnDelULTask(pTask->strURL.c_str(), pTask->strAction.c_str(), pTask->wParam, pTask->lParam);
                    delete pTask;
                }
                m_pVecTask->clear();
                std::vector<STULTASKINFO*>().swap(*m_pVecTask);
                delete m_pVecTask;
                m_pVecTask = NULL;
            }
            if (m_pCS != NULL) { ::DeleteCriticalSection(m_pCS); delete m_pCS; m_pCS = NULL; }
        }

    public:

        bool Init()
        {
            if (m_pCS == NULL)
            {
                m_pCS = new CRITICAL_SECTION;
                if (!InitializeCriticalSectionAndSpinCount(m_pCS, 0x80000400))
                {
                    delete m_pCS; 
                    m_pCS = NULL;
                    return false;
                }
            }

            m_pVecTask = new std::vector<STULTASKINFO*>;
            m_pAryHttp = new CHttpRQ[m_lThreadCount];
            m_pAryThread = new HANDLE[m_lThreadCount];
            m_pAryEvent = new HANDLE[m_lThreadCount];
            m_pAryIndex = new long[m_lThreadCount];

            DWORD dwThreadID = 0;
            for (long l = 0; l < m_lThreadCount; l++)
            {
                m_pAryIndex[l] = l;
                m_pAryEvent[l] = ::CreateEvent(NULL, FALSE, FALSE, NULL);
                m_pAryThread[l] = (HANDLE)_beginthreadex(NULL, 0, ThreadProcUpload, &m_pAryIndex[l], 0, (unsigned int *)&dwThreadID);
            }
            return true;
        }

        STULTASKINFO * const GetWaitTask(const long lThreadIndex)
        {
            STULTASKINFO * pTaskRet = NULL;
            STULTASKINFO * p = NULL;
            ::EnterCriticalSection(m_pCS);
            std::vector<STULTASKINFO*> & vec = *m_pVecTask;
            for (long l = 0L; l < (long)vec.size(); l++)
            {
                p = vec[l];
                if (p->lState == EN_WAIT)
                {
                    // find waiting task
                    if ( (p->iPriority == EN_PRIORITY_HIGH && lThreadIndex != 0) ||
                        (p->iPriority != EN_PRIORITY_HIGH && lThreadIndex == 0) )
                    {
                        // thread 0 only use for high priority tasks
                        continue;
                    }
                    p->lState = EN_BEGIN;  // change state
                    pTaskRet = p;
                    break;
                }
                else if (p->lState == EN_DEL_TASK)
                {
                    // delete task
                    vec.erase(vec.begin() + l);
                    delete p;
                }
            }
            ::LeaveCriticalSection(m_pCS);
            return pTaskRet;
        }

        virtual long AddUploadTask(const char * pszURL, IUploadNotify * pIUploadNotify, const unsigned short nPort = INTERNET_DEFAULT_HTTP_PORT, const char * pszAction = NULL, const char * pszHeader=NULL, const char * pszData = NULL, long lDataLen = 0,
                                    WPARAM wParam = NULL, LPARAM lParam = NULL, const short iPriority = EN_PRIORITY_NORMAL, const char iRetryCount = CONST_DEFAULT_TRY_COUNT)
        {
            if (pszURL == NULL || strlen(pszURL) <= 0L) return NULL;

            STULTASKINFO * p = NULL;
            bool bFound = false;
            ::EnterCriticalSection(m_pCS);
            std::vector<STULTASKINFO*> & vec = *m_pVecTask;
            for (long l = 0L; l < (long)m_pVecTask->size(); l++)
            {
                p = vec[l];
                if (strcmpi(pszURL, p->strURL.c_str()) == 0)
                {
                    bFound = true;
                    break;
                }
            }

            if (!bFound)
            {
                p = new STULTASKINFO;
                p->lState = EN_WAIT;
                p->iPriority = iPriority;
                p->pIUploadNotify = pIUploadNotify;
                p->wParam = wParam;
                p->lParam = lParam;
                p->iTryCount = iRetryCount;
                p->iCurTryCount = 0L;
                p->strURL = pszURL;
                p->nPort = nPort;
                p->strAction = (pszAction == NULL ? "" : pszAction);
                p->strHeader = (pszHeader == NULL ? "" : pszHeader);

                p->lDataLen = lDataLen;
                if (lDataLen > 0)
                {
                    p->pszPostData = new char[lDataLen+1]();
                    memcpy(p->pszPostData, pszData, lDataLen);
                }
                else
                {
                    p->pszPostData = NULL;
                }

                m_pVecTask->push_back(p);
            }

            ::LeaveCriticalSection(m_pCS);
            for (long l = 0L; l < m_lThreadCount; l++) { ::SetEvent(m_pAryEvent[l]); }
            Sleep(1);
            return 0L;
        }

        virtual long ClearAllTask()
        {
            ::EnterCriticalSection(m_pCS);
            std::vector<STULTASKINFO*> & vec = *m_pVecTask;
            for (long l = 0L; l < (long)m_pVecTask->size(); l++)
            {
                if (vec[l]->lState == EN_WAIT)
                {
                    vec[l]->lState = EN_DEL_TASK;
                    delete vec[l]->pszPostData;
                    vec[l]->pszPostData = NULL;
                }
            }
            ::LeaveCriticalSection(m_pCS);
            Sleep(1);
            return 0L;
        }


        STULTASKINFO * const AddTask(STULTASKINFO * pDLTaskInfo)
        {
            if ( pDLTaskInfo == NULL ) return NULL;
            ::EnterCriticalSection(m_pCS);
            m_pVecTask->push_back(pDLTaskInfo);
            ::LeaveCriticalSection(m_pCS);
            return pDLTaskInfo;
        }

        HANDLE GetEvent(const long lIndex) { return m_pAryEvent[lIndex]; }
        CHttpRQ * GetHttp(const long lIndex) { return &m_pAryHttp[lIndex]; }

        unsigned long UpLoadOnThread(const long lThreadIndex)
        {
            HANDLE hWaitEvent = m_pAryEvent[lThreadIndex];
            CHttpRQ * pHttp = &m_pAryHttp[lThreadIndex];
            STULTASKINFO * pTaskInfo = NULL;
            long lRet = 0L;
            for (; !m_bExit; )
            {
                pTaskInfo = GetWaitTask(lThreadIndex);
                if (pTaskInfo == NULL) 
                {
                    // no waiting tasks, wait 2s
                    ::WaitForSingleObject(hWaitEvent, 2000);
                    continue;
                }
                if (m_bExit) break;

                if (pTaskInfo->pIUploadNotify != NULL && !m_bExit) 
                {
                    lRet = pTaskInfo->pIUploadNotify->OnUploadNotify(EN_BEGIN, pTaskInfo->strURL.c_str(), pTaskInfo->strAction.c_str(), NULL, 0, pHttp->GetState(), pTaskInfo->wParam, pTaskInfo->lParam); 
                }

                pTaskInfo->lState = EN_DOWNLING;
                if ( pHttp->Upload( pTaskInfo->strURL.c_str(), pTaskInfo->nPort, pTaskInfo->strAction.c_str(), pTaskInfo->strHeader.c_str(), NULL, pTaskInfo->pszPostData, pTaskInfo->lDataLen,
                        pTaskInfo->lState, pTaskInfo->pIUploadNotify, pTaskInfo->wParam, pTaskInfo->lParam) )
                {
                    // success
                    //ATLTRACE("thread:%d [Success:%S]\n", lThreadIndex, pTaskInfo->wstrURL.c_str());
                    pTaskInfo->lState = EN_FINISH;
                    if (pTaskInfo->pIUploadNotify != NULL && !m_bExit) 
                    {
                        lRet = pTaskInfo->pIUploadNotify->OnUploadNotify(pTaskInfo->lState, pTaskInfo->strURL.c_str(), pTaskInfo->strAction.c_str(), NULL, 0, pHttp->GetState(), pTaskInfo->wParam, pTaskInfo->lParam); 
                    }
                    pTaskInfo->lState = EN_DEL_TASK;
                    pTaskInfo->pszPostData = NULL;
                }
                else
                {
                    // failed
                    //ATLTRACE("thread:%d [Failed:%S]\n", lThreadIndex, pTaskInfo->wstrURL.c_str());
                    if (!m_bExit)
                    {
                        if (pTaskInfo->iTryCount < 0 || pTaskInfo->iCurTryCount < pTaskInfo->iTryCount) 
                        {
                            // iTryCount < 0 do not inc Current try count, infinite retry      
                            if (pTaskInfo->iTryCount > 0) ++pTaskInfo->iCurTryCount;
                            pTaskInfo->lState = EN_WAIT;
                        }
                        else
                        {
                            pTaskInfo->lState = EN_FAILED;
                            if (pTaskInfo->pIUploadNotify != NULL && !m_bExit) { lRet = pTaskInfo->pIUploadNotify->OnUploadNotify(pTaskInfo->lState, pTaskInfo->strURL.c_str(), pTaskInfo->strAction.c_str(), NULL, 0, pHttp->GetState(), pTaskInfo->wParam, pTaskInfo->lParam); }
                            pTaskInfo->lState = EN_DEL_TASK;
                            delete pTaskInfo->pszPostData;
                            pTaskInfo->pszPostData = NULL;
                        }
                    }
                }                
            }
            return 0L;
        }

        void * CheckHttpInst(void * p)
        {
            for (long l = 0; l < m_lThreadCount; ++l)
            {
                if (&m_pAryHttp[l] == p) return p;
            }
            return NULL;
        }

    };

    __declspec(selectany) CUpLoader * CUpLoader::m_pInstance = NULL;

    inline void * CheckUpHttpOnCallback(void * p)
    {
        return CDownLoader::GetInstance()->CheckHttpInst(p);
    }

    inline unsigned int __stdcall ThreadProcUpload( void * p )
    {
        CUpLoader::GetInstance()->UpLoadOnThread(*(long*)p);
        //_endthreadex(0);
        return 0L;
    }
}
#pragma warning(pop)