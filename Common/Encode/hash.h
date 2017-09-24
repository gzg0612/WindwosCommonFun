#pragma once

namespace NSENCODE
{
    template<typename T>
    inline unsigned long GetStringHash(const T * pname)
    {
        const T *eos = pname;
        while( *eos++ ) ;
        unsigned long h = ( (unsigned long)(eos - pname - 1) ); // length
        unsigned long step = (h >> 5) + 1;
        for (unsigned long i = h; i >= step; i -= step)
            h = h ^ ((h << 5) + (h >> 2) + (unsigned long)tolower(pname[i - 1]));
        return h;
    }
};