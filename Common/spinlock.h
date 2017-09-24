#pragma once
#include <windows.h>

#ifndef LOCKBLOCK   // in this LOCKBLOCK, can use return
#define LOCKBLOCK(pCS) { CSpinLock locker((pCS)); 
#define UNLOCKBLOCK() }
#endif // !LOCKBLOCK

class CSpinLock
{
public:
    typedef volatile long SLCriticalSection;

    CSpinLock(SLCriticalSection * const pSpinCS) : m_pcs(pSpinCS) { Lock(m_pcs); }
    ~CSpinLock() { UnLock(m_pcs); }

    static const long SPIN_LOCK_INIT;

    static inline void Lock(SLCriticalSection * const pSpinCS)
    {
        while (::InterlockedExchange(pSpinCS, 1) == 1)
        {
            ::Sleep(1);
        }
        //::Sleep(1);
    }

    static inline void UnLock(SLCriticalSection * const pSpinCS)
    {
        ::InterlockedExchange(pSpinCS, 0);
    }

private:
    SLCriticalSection * m_pcs;
};

__declspec(selectany) const long CSpinLock::SPIN_LOCK_INIT = 0L;