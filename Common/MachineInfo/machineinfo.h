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
    // 64λ�²�֧���������. Ӧʹ��__cpuid��__cpuidex��Intrinsics������  
#else  
//#if _MSC_VER < 1600  // VS2010. ��˵VC2008 SP1֮���֧��__cpuidex  
    static void __cpuidex(long CPUInfo[4], long InfoType, long ECXValue)
    {
        if ( NULL == CPUInfo )  return;
        _asm
        {
            // load. ��ȡ�������Ĵ���  
            mov edi, CPUInfo;   // ׼����ediѰַCPUInfo  
            mov eax, InfoType;
            mov ecx, ECXValue;
            // CPUID  
            cpuid;
            // save. ���Ĵ������浽CPUInfo  
            mov[edi], eax;
            mov[edi + 4], ebx;
            mov[edi + 8], ecx;
            mov[edi + 12], edx;
        }
    }
//#endif  // #if _MSC_VER < 1600   // VS2010. ��˵VC2008 SP1֮���֧��__cpuidex 

#if _MSC_VER < 1400  // VC2005��֧��__cpuid  
    void __cpuid(long CPUInfo[4], long InfoType)
    {
        __cpuidex(CPUInfo, InfoType, 0);
    }
#endif  // #if _MSC_VER < 1400   // VC2005��֧��__cpuid

#endif  // #if defined(_WIN64)  

    // ȡ��CPU���̣�Vendor��  
    //  
    // result: �ɹ�ʱ�����ַ����ĳ��ȣ�һ��Ϊ12����ʧ��ʱ����0��  
    // pvendor: ���ճ�����Ϣ���ַ���������������Ϊ13�ֽڡ�  
    //int cpu_getvendor(char* pvendor)
    //{
    //    long dwBuf[4];
    //    if ( NULL == pvendor )  return 0;
    //    // Function 0: Vendor-ID and Largest Standard Function  
    //    __cpuid(dwBuf, 0);
    //    // save. ���浽pvendor  
    //    *(long*)&pvendor[0] = dwBuf[1];    // ebx: ǰ�ĸ��ַ�  
    //    *(long*)&pvendor[4] = dwBuf[3];    // edx: �м��ĸ��ַ�  
    //    *(long*)&pvendor[8] = dwBuf[2];    // ecx: ����ĸ��ַ�  
    //    pvendor[12] = '\0';
    //    return 12;
    //}

    // ȡ��CPU�̱꣨Brand��  
    //  
    // result: �ɹ�ʱ�����ַ����ĳ��ȣ�һ��Ϊ48����ʧ��ʱ����0��  
    // pbrand: �����̱���Ϣ���ַ���������������Ϊ49�ֽڡ�  
    //int cpu_getbrand(char* pbrand)
    //{
    //    long dwBuf[4];
    //    if ( NULL == pbrand )   return 0;
    //    // Function 0x80000000: Largest Extended Function Number  
    //    __cpuid(dwBuf, 0x80000000);
    //    if ( dwBuf[0] < 0x80000004 )   return 0;
    //    // Function 80000002h,80000003h,80000004h: Processor Brand String  
    //    __cpuid((long*)&pbrand[0], 0x80000002);    // ǰ16���ַ�  
    //    __cpuid((long*)&pbrand[16], 0x80000003);   // �м�16���ַ�  
    //    __cpuid((long*)&pbrand[32], 0x80000004);   // ���16���ַ�  
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