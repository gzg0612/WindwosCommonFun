#pragma once

#pragma warning(disable:4996)

#define OID_802_3_PERMANENT_ADDRESS      0x01010101
#define OID_802_3_CURRENT_ADDRESS        0x01010102
#define IOCTL_NDIS_QUERY_GLOBAL_STATS    0x00170002

namespace NSMACHINE
{
    template<typename T>
    static void GetMac(OUT T * const pMac, const long lBuffSize)
    {
        if ( pMac == NULL || lBuffSize < 12 ) return;

        wchar_t wszKey_NetworkCards[] = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards\\";
        HKEY hKey_NetworkCards = NULL;
        HKEY hKey_Device = NULL;
        HANDLE hFile = NULL;
        if ( ERROR_SUCCESS != ::RegOpenKeyW(HKEY_LOCAL_MACHINE, wszKey_NetworkCards, &hKey_NetworkCards) ) return;

        wchar_t wszSubKey[MAX_PATH];
        wchar_t wszDevice[MAX_PATH];
        unsigned long lenSubKey = MAX_PATH;
        unsigned long lenMaxPathBytes = MAX_PATH * sizeof(wchar_t);
        unsigned long ulRegSZ = REG_SZ;
        long lIndex = 0;
        while (lIndex >= 0 && ERROR_SUCCESS == ::RegEnumKeyExW(hKey_NetworkCards, lIndex++, wszSubKey, &lenSubKey, NULL, NULL, NULL, NULL) )
        {
            lenSubKey = MAX_PATH; lenMaxPathBytes = MAX_PATH * sizeof(wchar_t);
            _snwprintf(wszDevice, MAX_PATH, L"%s%s", wszKey_NetworkCards, wszSubKey);
            if (ERROR_SUCCESS != ::RegOpenKeyW(HKEY_LOCAL_MACHINE, wszDevice, &hKey_Device)) continue;
            if ( ERROR_SUCCESS == ::RegQueryValueExW(hKey_Device, L"ServiceName", NULL, &ulRegSZ, (unsigned char *)wszSubKey, &lenMaxPathBytes) )
            {
                _snwprintf(wszDevice, MAX_PATH, L"\\\\.\\%s", wszSubKey);
                if ( INVALID_HANDLE_VALUE != (hFile = ::CreateFileW(wszDevice, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) )
                {
                    unsigned long ulQueryDevice = OID_802_3_PERMANENT_ADDRESS;
                    unsigned char szOutBuf[32] = { 0 };
                    unsigned long ulBytes = sizeof(szOutBuf);
                    if ( ::DeviceIoControl(hFile, IOCTL_NDIS_QUERY_GLOBAL_STATS, &ulQueryDevice, sizeof(ulQueryDevice), szOutBuf, sizeof(szOutBuf), &ulBytes, NULL) )
                    {
                        if (sizeof(T) == sizeof(wchar_t))
                        {
                            _snwprintf((wchar_t*)pMac, lBuffSize, L"%02X%02X%02X%02X%02X%02X", szOutBuf[0], szOutBuf[1], szOutBuf[2], szOutBuf[3], szOutBuf[4], szOutBuf[5]);
                        }
                        else
                        {
                            _snprintf((char*)pMac, lBuffSize, "%02X%02X%02X%02X%02X%02X", szOutBuf[0], szOutBuf[1], szOutBuf[2], szOutBuf[3], szOutBuf[4], szOutBuf[5]);
                        }
                        
                        lIndex = -1L;
                    }
                    ::CloseHandle(hFile);
                }
            }
            ::RegCloseKey(hKey_Device);
        }

        ::RegCloseKey(hKey_NetworkCards);
    }

    static const long GetDiskSerial(OUT char * pszNumber, IN int nBuffSize)
    {
        if ( pszNumber == NULL || nBuffSize < 10 ) return -1;
        unsigned long ulVolumeSerialNumber;
        ::GetVolumeInformation(L"C:\\ ", NULL, 12, &ulVolumeSerialNumber, NULL, NULL, NULL, 10);
        _ltoa_s(ulVolumeSerialNumber, pszNumber, nBuffSize, 10);
        return ulVolumeSerialNumber;
    }


    // ----------------------- CPUID -----------------------
#if defined(_WIN64)  
    // 64位下不支持内联汇编. 应使用__cpuid、__cpuidex等Intrinsics函数。  
#else  
//#if _MSC_VER < 1600  // VS2010. 据说VC2008 SP1之后才支持__cpuidex  
    static void __cpuidex(long CPUInfo[4], long InfoType, long ECXValue)
    {
        if ( NULL == CPUInfo )  return;
        _asm
        {
            // load. 读取参数到寄存器  
            mov edi, CPUInfo;   // 准备用edi寻址CPUInfo  
            mov eax, InfoType;
            mov ecx, ECXValue;
            // CPUID  
            cpuid;
            // save. 将寄存器保存到CPUInfo  
            mov[edi], eax;
            mov[edi + 4], ebx;
            mov[edi + 8], ecx;
            mov[edi + 12], edx;
        }
    }
//#endif  // #if _MSC_VER < 1600   // VS2010. 据说VC2008 SP1之后才支持__cpuidex 

#if _MSC_VER < 1400  // VC2005才支持__cpuid  
    void __cpuid(long CPUInfo[4], long InfoType)
    {
        __cpuidex(CPUInfo, InfoType, 0);
    }
#endif  // #if _MSC_VER < 1400   // VC2005才支持__cpuid

#endif  // #if defined(_WIN64)  

    // 取得CPU厂商（Vendor）  
    //  
    // result: 成功时返回字符串的长度（一般为12）。失败时返回0。  
    // pvendor: 接收厂商信息的字符串缓冲区。至少为13字节。  
    //int cpu_getvendor(char* pvendor)
    //{
    //    long dwBuf[4];
    //    if ( NULL == pvendor )  return 0;
    //    // Function 0: Vendor-ID and Largest Standard Function  
    //    __cpuid(dwBuf, 0);
    //    // save. 保存到pvendor  
    //    *(long*)&pvendor[0] = dwBuf[1];    // ebx: 前四个字符  
    //    *(long*)&pvendor[4] = dwBuf[3];    // edx: 中间四个字符  
    //    *(long*)&pvendor[8] = dwBuf[2];    // ecx: 最后四个字符  
    //    pvendor[12] = '\0';
    //    return 12;
    //}

    // 取得CPU商标（Brand）  
    //  
    // result: 成功时返回字符串的长度（一般为48）。失败时返回0。  
    // pbrand: 接收商标信息的字符串缓冲区。至少为49字节。  
    //int cpu_getbrand(char* pbrand)
    //{
    //    long dwBuf[4];
    //    if ( NULL == pbrand )   return 0;
    //    // Function 0x80000000: Largest Extended Function Number  
    //    __cpuid(dwBuf, 0x80000000);
    //    if ( dwBuf[0] < 0x80000004 )   return 0;
    //    // Function 80000002h,80000003h,80000004h: Processor Brand String  
    //    __cpuid((long*)&pbrand[0], 0x80000002);    // 前16个字符  
    //    __cpuid((long*)&pbrand[16], 0x80000003);   // 中间16个字符  
    //    __cpuid((long*)&pbrand[32], 0x80000004);   // 最后16个字符  
    //    pbrand[48] = '\0';
    //    return 48;
    //}

    static void GetCPUID(OUT char * pszBuf, const long lBuffSize, const long lInfoType = 0L)
    {
        if ( pszBuf == NULL || lBuffSize < 32 ) return;

        INT32 CPUID[4] = { 0 };
        __cpuid(CPUID, lInfoType);

        _snprintf(pszBuf, lBuffSize, "%08X%08X%08X%08X", CPUID[0], CPUID[1], CPUID[2], CPUID[3]);
    }
};