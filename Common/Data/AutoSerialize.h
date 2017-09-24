/*----------------------------------------------------------------
// AutoSerialize
// namespace : 
// class     : CAutoPack  _st_serialize_base
// useage    :
// 1. must use #pragma pack(push)  #pragma pack(1)  ... #pragma pack(pop)
// 2. struct xxx : public _st_serialize_base
// 3. DEFINE_TYPES(xxx, T_I1 T_I2)  // ex. testSerialize()
//----------------------------------------------------------------*/

#pragma once
#include <windows.h>
#include <vector>
#include <string>

#define PACKSTRUCT(stru, pBuf, lSize, bFree) CAutoPack pack_##stru(pBuf); pack_##stru.Pack(&stru); (pBuf) = pack_##stru.GetBuf(bFree); (lSize) = pack_##stru.GetRealSize();
#define UNPACKSTRUCT(stru, pBuf) { CAutoPack pack(pBuf); pack.UnPack(&stru); }

#define DEFINE_TYPES(struct_name, types) \
public: struct_name::struct_name() { CAutoPack pack; pack.InitStruct(this, types, sizeof(struct_name)); } \
public: struct_name::~struct_name() { CAutoPack pack; pack.UnInitStruct(this); }

#define NUMSPLITE 0xFA
#define _ARYLEN(n) #n "\xFA"

#define _VTHEXSTR(s) #s   
#define VTHEXSTR(s)  _VTHEXSTR(\x0##s)
#define VTHEX(val) 0x##val

#define TMARKER     (0x0F)
#define BASETYPE    NULL

#define T_I1               VTHEX(01)     // char
#define T_I2               VTHEX(02)     // wchar_t short
#define T_I4               VTHEX(03)     // long int
#define T_R4               VTHEX(04)     // float
#define T_R8               VTHEX(05)     // double
#define T_BOOL             VTHEX(06)     // BOOL(long)
#define T_bool             VTHEX(07)     // bool
#define T_I1_ARY           VTHEX(11)     // char[]
#define T_I2_ARY           VTHEX(12)     // wchar_t[] short[]
#define T_I4_ARY           VTHEX(13)     // long[] int[]
#define T_R4_ARY           VTHEX(14)     // float[]
#define T_R8_ARY           VTHEX(15)     // double[]
#define T_BOOL_ARY         VTHEX(16)     // BOOL[]
#define T_bool_ARY         VTHEX(17)     // bool[]
#define T_PI1              VTHEX(21)     // char*            --> PARY_T<char>
#define T_PI2              VTHEX(22)     // wchar_t* short*  --> PARY_T<wchar>
#define T_PI4              VTHEX(23)     // long* int*       --> PARY_T<long>
#define T_PR4              VTHEX(24)     // float*           --> PARY_T<float>
#define T_PR8              VTHEX(25)     // double*          --> PARY_T<double>
#define T_PBOOL            VTHEX(26)     // BOOL*            --> PARY_T<BOOL>
#define T_Pbool            VTHEX(27)     // bool*            --> PARY_T<bool>
#define T_LPSTR            VTHEX(31)     // char*     --> char string, use strlen get length
#define T_LPWSTR           VTHEX(32)     // wchar_t*  --> wchar_t string, use wcslen get length
#define T_STDSTR           VTHEX(33)     // std::string
#define T_STDWSTR          VTHEX(34)     // std::wstring
#define T_STRUCT           VTHEX(35)     // struct xxx : public _st_serialize_base
#define T_STRUCT_ARY       VTHEX(36)     // xxx[];   xxx --> struct xxx : public _st_serialize_base
#define T_STRUCT_PARY      VTHEX(37)     // xxx* p = new xxx[n] --> PARY_T<xxx>  xxx --> struct xxx : public _st_serialize_base
#define T_PSTRUCT          VTHEX(38)     // xxx* p

#define TS_I1              VTHEXSTR(01)
#define TS_I2              VTHEXSTR(02)
#define TS_I4              VTHEXSTR(03)
#define TS_R4              VTHEXSTR(04)
#define TS_R8              VTHEXSTR(05)
#define TS_BOOL            VTHEXSTR(06)
#define TS_bool            VTHEXSTR(07)
#define TS_I1_ARY(n)       VTHEXSTR(11)   _ARYLEN(n)
#define TS_I2_ARY(n)       VTHEXSTR(12)   _ARYLEN(n)
#define TS_I4_ARY(n)       VTHEXSTR(13)   _ARYLEN(n)
#define TS_R4_ARY(n)       VTHEXSTR(14)   _ARYLEN(n)
#define TS_R8_ARY(n)       VTHEXSTR(15)   _ARYLEN(n)
#define TS_BOOL_ARY(n)     VTHEXSTR(16)   _ARYLEN(n)
#define TS_bool_ARY(n)     VTHEXSTR(17)   _ARYLEN(n)
#define TS_PI1             VTHEXSTR(21)
#define TS_PI2             VTHEXSTR(22)
#define TS_PI4             VTHEXSTR(23)
#define TS_PR4             VTHEXSTR(24)
#define TS_PR8             VTHEXSTR(25)
#define TS_PBOOL           VTHEXSTR(26)
#define TS_Pbool           VTHEXSTR(27)
#define TS_LPSTR           VTHEXSTR(31)
#define TS_LPWSTR          VTHEXSTR(32)
#define TS_STDSTR          VTHEXSTR(33)
#define TS_STDWSTR         VTHEXSTR(34)
#define TS_STRUCT          VTHEXSTR(35)
#define TS_STRUCT_ARY(n)   VTHEXSTR(36)   _ARYLEN(n)
#define TS_STRUCT_PARY     VTHEXSTR(37)
#define TS_PSTRUCT         VTHEXSTR(38)

#define T_CHAR             T_I1
#define T_WCHAR            T_I2
#define T_PCHAR            T_PI1
#define T_PWCHAR           T_PI2
#define T_CHAR_ARY         T_I1_ARY
#define T_WCHAR_ARY        T_I2_ARY
#define T_PBUF             T_PI1        // char* buf

#define TS_CHAR            TS_I1
#define TS_WCHAR           TS_I2
#define TS_PCHAR           TS_PI1
#define TS_PWCHAR          TS_PI2
#define TS_CHAR_ARY(n)     TS_I1_ARY(n)
#define TS_WCHAR_ARY(n)    TS_I2_ARY(n)
#define TS_PBUF            TS_PCHAR        // char* buf

// size of arguments on stack when pushed by value
static const unsigned long _sizeOfType[] = 
{ 
    0, 
    sizeof(char),       // char
    sizeof(short),      // wchar_t, short
    sizeof(long),       // long, int
    sizeof(float),      // float
    sizeof(double),     // double
    sizeof(int),        // BOOL
    sizeof(bool)        // bool
};                                                                    

#pragma pack(push)
#pragma pack(1)
struct _st_serialize_header
{
    long lSizeOfStru;
    unsigned char * pTypes;
};
struct _st_serialize_base
{
private:
    _st_serialize_header _header;

protected:
    _st_serialize_base & operator = (const _st_serialize_base & stru)
    {
        this->_header.lSizeOfStru = _header.lSizeOfStru;

        this->_header.pTypes = this->_header.pTypes;
//         long lSize = _header.lSizeOfStru;
//         unsigned char * pTypes = _header.pTypes;
//         *this = stru;
//         _header.lSizeOfStru = lSize;
//         _header.pTypes = pTypes;
        return (*this);
    }
};

template<typename T>
struct PARRAYT
{
    long m_lCount;
    T * m_pAryT;
};
#pragma pack(pop)

class CAutoPack
{
public:
    CAutoPack(const char * pData = NULL) : m_bPack(pData == NULL), m_bFree(true), m_lRealSize(0L), m_lBufBlockSize(0L), m_pBuf((char*)pData) {};
    ~CAutoPack(void) { _FreeBuf(); }
private:
    bool m_bPack;
    bool m_bFree;
    long m_lRealSize;
    long m_lBufBlockSize;
    char * m_pBuf;

private:
    void _FreeBuf()
    {
        if (m_bPack && m_bFree) { free(m_pBuf); m_pBuf = NULL; m_lRealSize = 0L; m_lBufBlockSize = 0L; }
    }
    const long _GetSizeOfStruct(void * const pStruct)
    {
        // bit mark: ab111111 --> a: is unpack; b: types from unpack buf, need free
        return ( (0x3FFFFFFF) & (((_st_serialize_header*)pStruct)->lSizeOfStru) );
    }
    const unsigned char * _GetStructTypes(void * const pStruct)
    {
        return (((_st_serialize_header*)pStruct)->pTypes);
    }
    char * _GetStructDataItem(void * const pStruct)
    {
        return ((char*)(pStruct)) + sizeof(_st_serialize_header);
    }
    const bool _IsUnpackStruct(void * const pStruct)
    {
        return ( ( (((_st_serialize_header*)pStruct)->lSizeOfStru) & (0x80000000) ) > 0 );
    }
    const bool _IsFreeHeaderTypes(void * const pStruct)
    {
        return ( ( (((_st_serialize_header*)pStruct)->lSizeOfStru) & (0x40000000) ) > 0 );
    }
    const long _GetUnPackStructSize()
    {
        return (((_st_serialize_header*)(m_pBuf + m_lRealSize))->lSizeOfStru);
    }
    void _PackStructInfo(void * const pStruct)
    {
        _st_serialize_header * pHeader = (_st_serialize_header *)pStruct;
        Serialize(&pHeader->lSizeOfStru, sizeof(long));
        Serialize((char**)&pHeader->pTypes);
    }
    void _UnPackStructInfo(void * const pStruct)
    {
        _st_serialize_header * pHeader = (_st_serialize_header*)pStruct;
        long lSizeOfStru = 0L;
        Serialize(&lSizeOfStru, sizeof(long));

        if (pHeader->pTypes == NULL)
        {
            Serialize((char**)&pHeader->pTypes);
            pHeader->lSizeOfStru = (lSizeOfStru | 0x40000000);  // need free header->pTypes
        }
        else
        {
            Serialize((char**)NULL);
        }
        pHeader->lSizeOfStru |= 0x80000000; // is unpack
    }
    const unsigned char * _AryToNext(const unsigned char * p)
    {
        while (*++p != (unsigned char)NUMSPLITE);
        return p;
    }

    const long _ZeroStructMem(void * const pStruct, CAutoPack * pAutoPack = NULL)
    {
        pAutoPack = (pAutoPack == NULL ? this : pAutoPack);

        long lSizeOfStru = _GetSizeOfStruct(pStruct);
        const unsigned char * pTypes = _GetStructTypes(pStruct);
        char * pItem = _GetStructDataItem(pStruct);

        if (pTypes == NULL || strlen((const char*)pTypes) <= 0L)
        {
            memset(pItem, 0x00, lSizeOfStru - sizeof(_st_serialize_base));
            return lSizeOfStru;
        }

        long lDataSize = 0L;
        while (*pTypes != '\0')
        {
            switch (*pTypes)
            {
            case T_I1: case T_I2: case T_I4: case T_R4: case T_R8: case T_BOOL: case T_bool:
                lDataSize = _sizeOfType[*pTypes];
                memset(pItem, 0x00, lDataSize);
                pItem += lDataSize;
                break;
            case T_I1_ARY: case T_I2_ARY: case T_I4_ARY: case T_R4_ARY: case T_R8_ARY: case T_BOOL_ARY: case T_bool_ARY:
                lDataSize = _sizeOfType[(*pTypes) & (TMARKER)] * atol((char*)(pTypes + 1));
                memset(pItem, 0x00, lDataSize);
                pTypes = _AryToNext(pTypes);
                pItem += lDataSize;
                break;
            case T_PI1: case T_PI2: case T_PI4: case T_PR4: case T_PR8: case T_PBOOL: case T_Pbool:
                memset(pItem, 0x00, sizeof(PARRAYT<void>));
                pItem += sizeof(PARRAYT<void>);
                break;
            case T_LPSTR: case T_LPWSTR: case T_PSTRUCT:
                *(void**)pItem = NULL;
                pItem += sizeof(void*);
                break;
            case T_STDSTR: 
                pItem += sizeof(std::string);
                break;
            case T_STDWSTR:
                pItem += sizeof(std::wstring);
                break;
            case T_STRUCT:      // stxxx;
                pItem += pAutoPack->_ZeroStructMem(pItem, this);
                break;
            case T_STRUCT_ARY:  // stxxx[n];
                {
                    long lCount = atol((char*)(pTypes + 1));
                    pTypes = _AryToNext(pTypes);
                    for (long l = 0L; l < lCount; ++l)
                    {
                        pItem += pAutoPack->_ZeroStructMem(pItem, this);
                    }
                }
                break;
            case T_STRUCT_PARY: // stxxx* pAry;
                {
                    PARRAYT<void> * pStruAry = (PARRAYT<void>*)pItem;
                    //pStruAry->m_bUseAryT = false;
                    pStruAry->m_lCount = 0L;
                    pStruAry->m_pAryT = NULL;
                    pItem += sizeof(PARRAYT<void>);
                }
                break;
            default:
                return lSizeOfStru;
                break;
            }
            pTypes++;
        }
        return lSizeOfStru;
    }

    void _FreeStructTypes(void * const pStruct)
    {
        _st_serialize_header * pHeader = (_st_serialize_header*)pStruct;
        if (_IsFreeHeaderTypes(pStruct) && pHeader->pTypes != NULL) 
        { 
            free(pHeader->pTypes); }
    }

    const long _FreeStructMem(void * const pStruct, CAutoPack * pAutoPack = NULL)
    {
        pAutoPack = (pAutoPack == NULL ? this : pAutoPack);

        long lSizeOfStru = _GetSizeOfStruct(pStruct);
        const unsigned char * pTypes = _GetStructTypes(pStruct);
        char * pItem = _GetStructDataItem(pStruct);

        if (pTypes == BASETYPE) 
        {
            _FreeStructTypes(pStruct);
            return lSizeOfStru;
        }

        long lDataSize = 0L;
        while (*pTypes != '\0')
        {
            switch (*pTypes)
            {
            case T_I1: case T_I2: case T_I4: case T_R4: case T_R8: case T_BOOL: case T_bool:
                lDataSize = _sizeOfType[*pTypes];
                pItem += lDataSize;
                break;
            case T_I1_ARY: case T_I2_ARY: case T_I4_ARY: case T_R4_ARY: case T_R8_ARY: case T_BOOL_ARY: case T_bool_ARY:
                lDataSize = _sizeOfType[(*pTypes) & (TMARKER)] * atol((char*)(pTypes + 1));
                pTypes = _AryToNext(pTypes);
                pItem += lDataSize;
                break;
            case T_PI1: case T_PI2: case T_PI4: case T_PR4: case T_PR8: case T_PBOOL: case T_Pbool:
                {
                    PARRAYT<void> * pAryStru = (PARRAYT<void>*)pItem;
                    /*if (!pAryStru->m_bUseAryT)*/ { free(pAryStru->m_pAryT); pAryStru->m_pAryT = NULL; pAryStru->m_lCount = 0L; }
                    pItem += sizeof(PARRAYT<void>);
                }
                break;
            case T_LPSTR: case T_LPWSTR:
                free(*(void**)pItem);
                *(void**)pItem = NULL;
                pItem += sizeof(void*);
                break;
            case T_STDSTR: 
                pItem += sizeof(std::string);
                break;
            case T_STDWSTR:
                pItem += sizeof(std::wstring);
                break;
            case T_STRUCT:
                pItem += pAutoPack->_FreeStructMem(pItem, this);
                break;
            case T_PSTRUCT:
                _FreeStructTypes(*(void**)pItem);
                free(*(void**)pItem);
                *(void**)pItem = NULL;
                pItem += sizeof(void*);
                break;
            case T_STRUCT_ARY:
                {
                    // stxxx[n];
                    long lCount = atol((char*)(pTypes + 1));
                    pTypes = _AryToNext(pTypes);
                    for (long l = 0L; l < lCount; ++l)
                    {
                        pItem += pAutoPack->_FreeStructMem(pItem, this);
                    }
                }
                break;
            case T_STRUCT_PARY:
                {
                    PARRAYT<void> * pStruArray = (PARRAYT<void>*)pItem;
                    //if (!pStruArray->m_bUseAryT)
                    {
                        char * p = (char*)pStruArray->m_pAryT;
                        for (long l = 0; l < pStruArray->m_lCount; ++l)
                        {
                            p += pAutoPack->_FreeStructMem(p, this);
                        }
                        free(pStruArray->m_pAryT);
                        pStruArray->m_pAryT = NULL;
                        pStruArray->m_lCount = 0L;
                    }
                    pItem += sizeof(PARRAYT<void>);
                }
                break;
            default:
                return lSizeOfStru;
                break;
            }
            pTypes++;
        }

        _FreeStructTypes(pStruct);
        return lSizeOfStru;
    }

public:
    const bool IsPack() { return m_bPack; }
    const long GetRealSize() { return m_lRealSize; }
    char * const GetBuf(const bool bFree = true) { m_bFree = bFree; return m_pBuf; }

    void InitStruct(void * const pStruct, const char * pTypes, const long lSizeOfStru)
    {
        _st_serialize_header * pHeader = (_st_serialize_header*)pStruct;
        pHeader->lSizeOfStru = lSizeOfStru;
        pHeader->pTypes =(unsigned char*)pTypes;
        _ZeroStructMem(pStruct);
    }

    void UnInitStruct(void * const pStruct)
    {
        if (!_IsUnpackStruct(pStruct)) return;
        // unpack
        _FreeStructMem(pStruct);
    }

    const long Pack(void * const pStruct, CAutoPack * pAutoPack = NULL)
    {
        m_bPack = true;
        pAutoPack = (pAutoPack == NULL ? this : pAutoPack);

        // pack struct info
        _PackStructInfo(pStruct);

        long lSizeOfStru = _GetSizeOfStruct(pStruct);
        const unsigned char * pTypes = _GetStructTypes(pStruct);
        char * pItem = _GetStructDataItem(pStruct);

        long lenTypes = (pTypes == NULL ? 0L : (long)strlen((const char*)pTypes));
        if (lenTypes <= 0L)
        {
            Serialize(pItem, lSizeOfStru - sizeof(_st_serialize_base)); // memcpy ...
            return lSizeOfStru;
        }

        long lDataSize = 0L;
        while (*pTypes != '\0')
        {
            switch (*pTypes)
            {
            case T_I1: case T_I2: case T_I4: case T_R4: case T_R8: case T_BOOL: case T_bool:
                lDataSize = _sizeOfType[*pTypes];
                pAutoPack->Serialize(pItem, lDataSize);
                pItem += lDataSize;
                break;
            case T_I1_ARY: case T_I2_ARY: case T_I4_ARY: case T_R4_ARY: case T_R8_ARY: case T_BOOL_ARY: case T_bool_ARY:
                lDataSize = _sizeOfType[(*pTypes) & (TMARKER)] * atol((char*)(pTypes + 1));
                pAutoPack->Serialize(pItem, lDataSize);
                pTypes = _AryToNext(pTypes);
                pItem += lDataSize;
                break;
            case T_PI1: case T_PI2: case T_PI4: case T_PR4: case T_PR8: case T_PBOOL: case T_Pbool:
            {
                PARRAYT<void> * pAry = (PARRAYT<void>*)pItem;
                pAutoPack->Serialize(&pAry->m_lCount, sizeof(long));
                pAutoPack->Serialize(pAry->m_pAryT, pAry->m_lCount * _sizeOfType[(*pTypes) & (TMARKER)]);
                pItem += sizeof(PARRAYT<void>);
            }
            break;
            case T_LPSTR:
                pAutoPack->Serialize((char**)pItem);
                pItem += sizeof(char*);
                break;
            case T_LPWSTR:
                pAutoPack->Serialize((wchar_t**)pItem);
                pItem += sizeof(wchar_t*);
                break;
            case T_STDSTR:
                pAutoPack->Serialize<std::string>((std::string*)pItem);
                pItem += sizeof(std::string);
                break;
            case T_STDWSTR:
                pAutoPack->Serialize<std::wstring>((std::wstring*)pItem);
                pItem += sizeof(std::wstring);
                break;
            case T_STRUCT:
                pItem += pAutoPack->Pack(pItem, this);
                break;
            case T_PSTRUCT:
                {
                    bool bNULL = (*(void**)pItem == NULL ? true : false);
                    pAutoPack->Serialize(&bNULL, sizeof(bool));
                    if (!bNULL) pAutoPack->Pack(*(void**)pItem, this);
                    pItem += sizeof(void*);
                }
                break;
            case T_STRUCT_ARY:
            {
                long lCount = atol((char*)(pTypes + 1));
                pTypes = _AryToNext(pTypes);
                for (long l = 0L; l < lCount; ++l)
                {
                    pItem += pAutoPack->Pack(pItem, this);
                }
                break;
            }
            case T_STRUCT_PARY:
            {
                PARRAYT<void> * pStruArray = (PARRAYT<void>*)pItem;
                pAutoPack->Serialize(&pStruArray->m_lCount, sizeof(long));
                char * p = (char*)pStruArray->m_pAryT;
                for (long l = 0; l < pStruArray->m_lCount; ++l)
                {
                    p += pAutoPack->Pack(p, this);
                }
                pItem += sizeof(PARRAYT<void>);
                break;
            }
            default:
                return lSizeOfStru;
                break;
            }
            pTypes++;
        }
        return lSizeOfStru;
    }

    const long UnPack(void * const pStruct, CAutoPack * pAutoPack = NULL)
    {
        m_bPack = false;
        if (m_pBuf == NULL || pStruct == NULL) return -1L;

        pAutoPack = (pAutoPack == NULL ? this : pAutoPack);

        // unpack struct info
        _UnPackStructInfo(pStruct);

        long lSizeOfStru = _GetSizeOfStruct(pStruct);
        const unsigned char * pTypes = _GetStructTypes(pStruct);
        char * pItem = _GetStructDataItem(pStruct);

        if (pTypes == NULL || strlen((const char*)pTypes) <= 0L)
        {
            Serialize(pItem, lSizeOfStru - sizeof(_st_serialize_base));
            return lSizeOfStru;
        }

        long lDataSize = 0L;
        while (*pTypes != '\0')
        {
            switch (*pTypes)
            {
            case T_I1: case T_I2: case T_I4: case T_R4: case T_R8: case T_BOOL: case T_bool:
                lDataSize = _sizeOfType[*pTypes];
                pAutoPack->Serialize(pItem, lDataSize);
                pItem += lDataSize;
                break;
            case T_I1_ARY: case T_I2_ARY: case T_I4_ARY: case T_R4_ARY: case T_R8_ARY: case T_BOOL_ARY: case T_bool_ARY:
                lDataSize = _sizeOfType[(*pTypes) & (TMARKER)] * atol((char*)(pTypes + 1));
                pAutoPack->Serialize(pItem, lDataSize);
                pItem += lDataSize;
                pTypes = _AryToNext(pTypes);
                break;
            case T_PI1: case T_PI2: case T_PI4: case T_PR4: case T_PR8: case T_PBOOL: case T_Pbool:
            {
                PARRAYT<void> * pAry = (PARRAYT<void>*)pItem;
                pAutoPack->Serialize(&pAry->m_lCount, sizeof(long));
                long lSize = pAry->m_lCount * _sizeOfType[(*pTypes) & (TMARKER)];
                pAry->m_pAryT = (char *)malloc(lSize);
                pAutoPack->Serialize(pAry->m_pAryT, lSize);
                pItem += sizeof(PARRAYT<void>);
                break;
            }
            case T_LPSTR:
                pAutoPack->Serialize((char**)pItem);
                pItem += sizeof(char*);
                break;
            case T_LPWSTR:
                pAutoPack->Serialize((wchar_t**)pItem);
                pItem += sizeof(wchar_t*);
                break;
            case T_STDSTR:
                pAutoPack->Serialize<std::string>((std::string *)pItem);
                pItem += sizeof(std::string);
                break;
            case T_STDWSTR:
                pAutoPack->Serialize<std::wstring>((std::wstring *)pItem);
                pItem += sizeof(std::wstring);
                break;
            case T_STRUCT:
                pItem += pAutoPack->UnPack(pItem, this);
                break;
            case T_PSTRUCT:
                {
                    bool bNULL = true;
                    pAutoPack->Serialize(&bNULL, sizeof(bool));
                    char * p = NULL;
                    if (!bNULL) 
                    {
                        long lSizeOfStruTmp = _GetUnPackStructSize();
                        p = (char*)malloc(lSizeOfStruTmp);
                        memset(p, 0x00, lSizeOfStruTmp);
                        pAutoPack->UnPack(p, this);
                    }
                    *((void**)pItem) = p;
                    pItem += sizeof(void*);
                }
                break;
            case T_STRUCT_ARY:
            {
                long lCount = atol((char*)(pTypes + 1));
                for (long l = 0L; l < lCount; ++l)
                {
                    pItem += pAutoPack->UnPack(pItem, this);
                }
                pTypes = _AryToNext(pTypes);
                break;
            }
            case T_STRUCT_PARY:
            {
                PARRAYT<void> * pStruArray = (PARRAYT<void>*)pItem;
                pAutoPack->Serialize(&pStruArray->m_lCount, sizeof(long));
                if (pStruArray->m_lCount > 0L)
                {
                    long lSize = pStruArray->m_lCount * _GetUnPackStructSize();
                    pStruArray->m_pAryT = (char *)malloc(lSize);
                    memset(pStruArray->m_pAryT, 0x00, lSize);
                    char * pUnpack = (char*)pStruArray->m_pAryT;
                    for (long l = 0; l < pStruArray->m_lCount; ++l)
                    {
                        pUnpack += pAutoPack->UnPack(pUnpack, this);
                    }
                }
                pItem += sizeof(PARRAYT<void>);
            }
            break;
            default:
                return lSizeOfStru;
                break;
            }
            pTypes++;
        }
        return lSizeOfStru;
    }
    void Serialize(void * const pItem, const long lDataSize)
    {
        if (lDataSize <= 0L) return;
        if (pItem == NULL) { m_lRealSize += lDataSize; return; }
        if (m_bPack)
        {
            if (m_lBufBlockSize < m_lRealSize + lDataSize)
            {
                m_lBufBlockSize = ((((m_lRealSize + lDataSize) >> 8) + 1) << 8); //((m_lRealSize + lDataSize) / 256 + 1) * 256;
                char * pNewBuf = (char*)malloc(m_lBufBlockSize);
                if (m_pBuf != NULL) { memcpy(pNewBuf, m_pBuf, m_lRealSize); }
                memset(pNewBuf + m_lRealSize, 0x00, m_lBufBlockSize - m_lRealSize);
                if (m_pBuf != NULL) free(m_pBuf);
                m_pBuf = pNewBuf;
            }
            memcpy(m_pBuf + m_lRealSize, pItem, lDataSize);
            m_lRealSize += lDataSize;
        }
        else
        {
            memcpy(pItem, m_pBuf + m_lRealSize, lDataSize);
            m_lRealSize += lDataSize;
        }
    }
    template<typename T> void Serialize(T ** const pItem, long lenStr)
    {
        // sizeof(long) bytes length, string content, \0
        if (m_bPack)
        {
            Serialize(&lenStr, sizeof(long));    // write string length
            Serialize(*pItem, sizeof(T)*lenStr); // write string content without \0
            T tszEnd = (T)0;
            Serialize(&tszEnd, sizeof(T));   // write end width \0 
        }
        else
        {
            Serialize(&lenStr, sizeof(long));
            if (pItem != NULL)
            {
                T * p = NULL;
                if (lenStr > 0L) p = (T*)malloc(sizeof(T) * (lenStr + 1));
                Serialize(p, sizeof(T) * (lenStr + 1));     // string content width \0
                *pItem = p;
            }
            else
            {
                Serialize(NULL, sizeof(T) * (lenStr + 1));  // end with \0
            }
        }
    }
    template<typename T> void Serialize(T ** const pItem)
    {
        long len = 0L;
        if (m_bPack && *pItem != NULL)
        {
            const T *eos = *pItem;
            while( *eos++ ) ;
            len = ( (long)(eos - *pItem - 1) ); // length
        }
        Serialize(pItem, len);
    }
    template<typename TSTDSTRING> void Serialize(TSTDSTRING * const pItem)
    {
        if (m_bPack)
        {
            typename TSTDSTRING::value_type * pStr = (pItem->size() <= 0L ? NULL : (typename TSTDSTRING::value_type*)pItem->c_str());
            Serialize(&pStr, (long)pItem->size());
        }
        else
        {
            typename TSTDSTRING::value_type * pStr = NULL;
            Serialize(&pStr, 0);
            if (pStr != NULL) { *pItem = pStr; free((void*)pStr); }
        }
    }
}; 

#ifdef _DEBUG
// ------------------ test ------------------
#pragma pack(push)
#pragma pack(1)
struct STAAA : public _st_serialize_base
{
    short i;
    long  l;

    //DEFINE_TYPES(STAAA, TS_I2 TS_I4);
    DEFINE_TYPES(STAAA, BASETYPE);
};

struct MyStructAA : public _st_serialize_base
{
    DEFINE_TYPES(MyStructAA, TS_I2 TS_STRUCT);

    short i2;
    STAAA staaa;
    //long i4;
    //double r8;
    //char c;
    //char sz[250];
    //wchar_t wsz[250];
};

struct STTestAutoSerialize : public _st_serialize_base
{
    DEFINE_TYPES(STTestAutoSerialize,
        TS_PR8
        TS_STRUCT
        TS_PSTRUCT
        TS_STRUCT_ARY(10)
        TS_STRUCT_PARY
        TS_I2 
        TS_I4
        TS_bool
        TS_R4
        TS_R8 
        TS_I1 
        TS_STDSTR 
        TS_STDWSTR
        TS_CHAR_ARY(250) 
        TS_WCHAR_ARY(250) 
        TS_LPSTR 
        TS_LPWSTR
        );

   
    PARRAYT<double> pdouble;
    MyStructAA saa;
    MyStructAA * psaa1;
    MyStructAA saaAry[10];
    PARRAYT<MyStructAA> psaa;
    short i2;
    long i4;
    bool b;
    float f4;
    double r8;
    char c;
    std::string stdstr;
    std::wstring wstr;
    char sz[250];
    wchar_t wsz[250];
    char * p;
    wchar_t * pw;
};
#pragma pack(pop)

static inline void testSerialize()
{
    STTestAutoSerialize s;

    s.stdstr = "aaalaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdflaksdjfladsflasdf";
    s.wstr = L"test wstring";
    s.i2 = 1;
    s.i4 = 1000000;
    s.f4 = 716823.3284f;
    s.r8 = 1209380128321.1293123;
    s.c = 'A';
    s.p = NULL;
    s.pw = L"12345asdfasdf";
    ::GetModuleFileNameA(NULL, s.sz, MAX_PATH);
    ::GetModuleFileNameW(NULL, s.wsz, MAX_PATH);

    s.psaa1 = new MyStructAA;
    s.psaa1->i2 = 998;
    s.saa.i2 = 999;

    s.psaa.m_lCount = 10;
    s.psaa.m_pAryT = new MyStructAA[s.psaa.m_lCount];

    s.saa.staaa.i = 20;
    s.saa.staaa.l = 30;

    s.pdouble.m_lCount = 10;
    s.pdouble.m_pAryT = new double[s.pdouble.m_lCount];
    for (long l = 0; l < 10; l++)
    {
        s.pdouble.m_pAryT[l] = rand();
        s.saaAry[l].i2 = (short)(1000 + l);
        s.saaAry[l].staaa.i = (short)(2000 + l);
        s.saaAry[l].staaa.l = 3000 + l;

        s.psaa.m_pAryT[l].i2 = (short)(4000 + l);
        s.psaa.m_pAryT[l].staaa.i = (short)(5000 + l);
        s.psaa.m_pAryT[l].staaa.l = 6000 + l;
    }

    
    //s.saa.staaa.i = 998;
    //s.saa.staaa.l = 997;
    

    CAutoPack pack;
    //pack.FreeMem(&s);


    pack.Pack(&s);
    long len = pack.GetRealSize();
    char * pBuf = pack.GetBuf();

    STTestAutoSerialize s1;
    //s1.MyStruct();
    CAutoPack unpack(pBuf);
    unpack.UnPack(&s1);

    delete s.pdouble.m_pAryT;
    delete s.psaa1;
    delete[] s.psaa.m_pAryT;
    
    //unpack.UnPack(pBuf, len, &saa1);

    //wchar_t wszBuf[2000];
    //_snwprintf(wszBuf, 2000, L"i2=%d, i4=%d, r8=%lf, c=%c, p=%S, pw=%s, sz=%S, wsz=%s, stdstr=%S",
    //    s1.i2, s1.i4, s1.r8, s1.c, s1.p, s1.pw, s1.sz, s1.wsz, s1.stdstr.c_str());
    //::MessageBox(NULL, wszBuf, NULL, NULL);

    //s.FreeMem(&pBuf);
}
#endif

