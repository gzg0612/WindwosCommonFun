#pragma once
#include <string>
namespace NSENCODE
{
    static const char Base64EncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    static const long Base64Encode(const unsigned char* Data, long lDataByte, char * const pBufOut, const long lBufSize)
    {
        long lDstSize = ((lDataByte / 3 + 1) * 4);
        if (Data == NULL || pBufOut == NULL || lBufSize < lDstSize)
        {
            return lDstSize;
        }

        char * pOut = pBufOut;
        unsigned char Tmp[4] = { 0 };
        int LineLength = 0;
        for ( int i = 0; i < (int)(lDataByte / 3); i++ )
        {
            Tmp[1] = *Data++;
            Tmp[2] = *Data++;
            Tmp[3] = *Data++;
            
            *pOut++ = Base64EncodeTable[Tmp[1] >> 2];
            *pOut++ = Base64EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
            *pOut++ = Base64EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
            *pOut++ = Base64EncodeTable[Tmp[3] & 0x3F];
            //if(LineLength+=4,LineLength==76) {strEncode+="\r\n";LineLength=0;}	URL中不能存在回车换行
        }

        int Mod = lDataByte % 3;
        if ( Mod == 1 )
        {
            Tmp[1] = *Data++;
            *pOut++ = Base64EncodeTable[(Tmp[1] & 0xFC) >> 2];
            *pOut++ = Base64EncodeTable[((Tmp[1] & 0x03) << 4)];
            *pOut++ = '=';
            *pOut++ = '=';
        }
        else if ( Mod == 2 )
        {
            Tmp[1] = *Data++;
            Tmp[2] = *Data++;
            *pOut++ = Base64EncodeTable[(Tmp[1] & 0xFC) >> 2];
            *pOut++ = Base64EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
            *pOut++ = Base64EncodeTable[((Tmp[2] & 0x0F) << 2)];
            *pOut++ = '=';
        }

        return lDstSize;
    };

    static std::string Base64Decode(const char* Data,int DataByte,int& OutByte)
    {
        //解码表
        static const char DecodeTable[] =
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            62, // '+'
            0, 0, 0,
            63, // '/'
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // '0'-'9'
            0, 0, 0, 0, 0, 0, 0,
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
            13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // 'A'-'Z'
            0, 0, 0, 0, 0, 0,
            26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
            39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
        };
        //返回值
        string strDecode;
        int nValue;
        int i= 0;
        while (i < DataByte)
        {
            if (*Data != '\r' && *Data!='\n')
            {
                nValue = DecodeTable[*Data++] << 18;
                nValue += DecodeTable[*Data++] << 12;
                strDecode+=(nValue & 0x00FF0000) >> 16;
                OutByte++;
                if (*Data != '=')
                {
                    nValue += DecodeTable[*Data++] << 6;
                    strDecode+=(nValue & 0x0000FF00) >> 8;
                    OutByte++;
                    if (*Data != '=')
                    {
                        nValue += DecodeTable[*Data++];
                        strDecode+=nValue & 0x000000FF;
                        OutByte++;
                    }
                }
                i += 4;
            }
            else// 回车换行,跳过
            {
                Data++;
                i++;
            }
        }
        return strDecode;
    }
};
