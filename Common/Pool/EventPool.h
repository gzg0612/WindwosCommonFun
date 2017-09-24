#pragma once

#include <vector>
#include "..\spinlock.h"

#ifndef DECLEARNS
#define DECLEARNS(ns) namespace ns {
#define ENDDECLEARNS() };
#endif // !DECLEARNS

DECLEARNS(NSEVENTPOOL)

class CEventPool
{
public:
    static CEventPool * const CreateInstance()
    {
        if (m_pInst == NULL && !m_bDestroy) { m_pInst = new CEventPool; m_pInst->m_pVecEvent = new std::vector<__int64>; }
        return m_pInst;
    }

    static CEventPool * const GetInstance() { return m_pInst; }
    static void Destory() { if (m_pInst != NULL) { delete m_pInst; m_pInst = NULL; } }

private:
    static bool m_bDestroy;
    static CEventPool * m_pInst;
    static CSpinLock::SLCriticalSection m_slcs;

private:
    typedef union _un_event_info
    {
        __int64 i64Value;
        struct { long lCount; HANDLE hEvent; } EventInfo;
    } UNEVENTINFO, *LPUNEVENTINFO;
    std::vector<__int64> * m_pVecEvent;

public:
    CEventPool(void) {}
    ~CEventPool() { m_bDestroy = true; _ReleaseAll(); }

private:
    void _ReleaseAll()
    {
        LOCKBLOCK(&m_slcs);
        std::vector<__int64> & vec = *m_pVecEvent;
        for (long l = 0; l < (long)vec.size(); ++l)
        {
            UNEVENTINFO * pEventInfo = (UNEVENTINFO *)&vec[l];
            if (pEventInfo->EventInfo.lCount > 0L) ::SetEvent(pEventInfo->EventInfo.hEvent);
        }
        ::Sleep(1);
        for (long l = 0; l < (long)vec.size(); ++l) ::CloseHandle(((UNEVENTINFO*)&vec[l])->EventInfo.hEvent);
        delete m_pVecEvent;
        m_pVecEvent = NULL;
        UNLOCKBLOCK();
    }

public:

    const HANDLE GetEvent(const bool bManualReset = false, const bool bInitialState = false)
    {
        HANDLE hEvent = NULL;
        LOCKBLOCK(&m_slcs);
        std::vector<__int64> & vec = *m_pVecEvent;
        for (long l = 0; l < (long)vec.size(); ++l)
        {
            UNEVENTINFO * pEventInfo = (UNEVENTINFO *)&vec[l];
            if (pEventInfo->EventInfo.lCount == 0L)
            {
                pEventInfo->EventInfo.lCount = 1L;
                hEvent = pEventInfo->EventInfo.hEvent;
                break;
            }
        }
        if (hEvent == NULL)
        {
            hEvent = ::CreateEvent(NULL, bManualReset, bInitialState, NULL);
            UNEVENTINFO eventInfo = {};
            eventInfo.EventInfo.lCount = 1L;
            eventInfo.EventInfo.hEvent = hEvent;
            vec.push_back(eventInfo.i64Value);
        }
        UNLOCKBLOCK();
        return hEvent;
    }
    
    void ReleaseEvent(const HANDLE hEvent) 
    {
        LOCKBLOCK(&m_slcs);
        std::vector<__int64> & vec = *m_pVecEvent;
        for (long l = 0; l < (long)vec.size(); ++l)
        {
            UNEVENTINFO * pEventInfo = (UNEVENTINFO *)&vec[l];
            if (pEventInfo->EventInfo.hEvent == hEvent)
            {
                pEventInfo->EventInfo.lCount = 0;
                break;
            }
        }
        UNLOCKBLOCK();
    }
};

__declspec(selectany) bool CEventPool::m_bDestroy = false;
__declspec(selectany) CEventPool * CEventPool::m_pInst = NULL;
__declspec(selectany) CSpinLock::SLCriticalSection CEventPool::m_slcs = CSpinLock::SPIN_LOCK_INIT;

ENDDECLEARNS();