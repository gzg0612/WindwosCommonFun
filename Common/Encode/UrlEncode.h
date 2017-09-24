#pragma once
#include <string>
namespace URLCODE
{
    inline BYTE toHex(const BYTE &x)
    {
        return x > 9 ? x + 55: x + 48;
    }
    inline BYTE fromHex(const BYTE &x)
    { 
        return x > 64 ? x - 55 : x - 48;
    }

    inline void _UrlEncoding(string &sIn, string &sOut)
    {
        for(long ix = 0; ix < (long)sIn.size(); ix++)
        {
            BYTE buf[4];
            memset( buf, 0, 4 );
            if(isalnum((BYTE)sIn[ix])||strchr("-_.~!=()", sIn[ix]) )
            {
                buf[0] = sIn[ix];
            }
            else if (isspace((BYTE)sIn[ix]))
            {
                buf[0] = '+';
            }
            else
            {
                buf[0] = '%';
                buf[1] = toHex((BYTE)sIn[ix] >> 4);
                buf[2] = toHex((BYTE)sIn[ix] % 16);
            }
            sOut += (char *)buf;
        }
    }

    static void UrlEncoding(const char *apIn, string &sOut)
    {
        string lstrIn(apIn);
        _UrlEncoding(lstrIn, sOut);
    }

    inline void _UrlDecoding(string &sIn, string &sOut)
    {
        int ilen = (int)sIn.size();
        for(int ix = 0; ix < ilen; ix++)
        {
            if(sIn.at(ix)=='%' && ix < ilen - 2)
            {
                ix++;
                char c = fromHex(sIn.at(ix++));
                c = c << 4;
                c += fromHex(sIn.at(ix));
                sOut += c;
            }
            else if(sIn.at(ix)=='+')
                sOut += ' ';
            else
                sOut += sIn.at(ix);
        }
    }

    static void UrlDecoding(const char * apIn, string &sOut)
    {
        string lstrIn(apIn);
        _UrlDecoding(lstrIn, sOut);	
    }
};
