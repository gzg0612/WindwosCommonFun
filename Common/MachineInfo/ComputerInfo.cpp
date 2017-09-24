#include "StdAfx.h"
#include "ComputerInfo.h"
#include <Iphlpapi.h>

CComputerInfo::CComputerInfo(void)
{
}

CComputerInfo::~CComputerInfo(void)
{
}

bool CComputerInfo::GetPhyDriveSerial( wchar_t* pModelNo,wchar_t* pSerialNo )
{
    //-1是因为 SENDCMDOUTPARAMS 的结尾是 BYTE bBuffer[1];
    BYTE IdentifyResult[sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1];
    DWORD dwBytesReturned;
    GETVERSIONINPARAMS get_version;
    SENDCMDINPARAMS send_cmd = { 0 };

    HANDLE hFile = CreateFile(_T("\\\\.\\PHYSICALDRIVE0"), GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if(hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    //get version  
    DeviceIoControl(hFile, SMART_GET_VERSION, NULL, 0,
        &get_version, sizeof(get_version), &dwBytesReturned, NULL);

    //identify device
    send_cmd.irDriveRegs.bCommandReg = 
        (get_version.bIDEDeviceMap & 0x10)? ATAPI_ID_CMD : ID_CMD;
    
    DeviceIoControl(hFile, SMART_RCV_DRIVE_DATA, &send_cmd, sizeof(SENDCMDINPARAMS) - 1,
        IdentifyResult, sizeof(IdentifyResult), &dwBytesReturned, NULL);
    CloseHandle(hFile);

    //adjust the byte order
    PUSHORT pWords = (USHORT*)(((SENDCMDOUTPARAMS*)IdentifyResult)->bBuffer);

    ToLittleEndian(pWords, 27, 46, pModelNo);
    ToLittleEndian(pWords, 10, 19, pSerialNo);
    
    TrimStart(pModelNo);
    TrimStart(pSerialNo);

    return TRUE;
}
void CComputerInfo::ToLittleEndian(PUSHORT pWords, int nFirstIndex, int nLastIndex, wchar_t* pBuf)  
{
    int index;
    wchar_t* pDest = pBuf;

    for(index = nFirstIndex; index <= nLastIndex; ++index)
    {  
        pDest[0] = pWords[index] >> 8;
        pDest[1] = pWords[index] & 0xFF;
        pDest += 2;
    }

    *pDest = 0;

    --pDest;
    while(*pDest == 0x20)
    {
        *pDest = 0;
        --pDest;
    }
}
//滤除字符串起始位置的空格  
void CComputerInfo::TrimStart(wchar_t* pBuf)  
{  
    if(*pBuf != 0x20)
    {
        return;
    }

    wchar_t* pDest = pBuf;
    wchar_t* pSrc = pBuf + 1;

    while(*pSrc == 0x20)
    {
        ++pSrc;
    }

    while(*pSrc)
    {  
        *pDest = *pSrc;
        ++pDest;
        ++pSrc;
    }  

    *pDest = 0;
}  

CString CComputerInfo::GetMACaddress( void )
{
    DWORD MACaddress = 0;
    IP_ADAPTER_INFO AdapterInfo[16];       // Allocate information
    // for up to 16 NICs
    DWORD dwBufLen = sizeof(AdapterInfo);  // Save memory size of buffer

    CString strMacAddr = _T("");
    
    DWORD dwStatus = GetAdaptersInfo(      // Call GetAdapterInfo
        AdapterInfo,                 // [out] buffer to receive data
        &dwBufLen);                  // [in] size of receive data buffer
    
    if (ERROR_SUCCESS == dwStatus)
    {
        strMacAddr.Format(_T("%X%X%X%X%X%X"), AdapterInfo[0].Address[0],
            AdapterInfo[0].Address[1],
            AdapterInfo[0].Address[2],
            AdapterInfo[0].Address[3],
            AdapterInfo[0].Address[4],
            AdapterInfo[0].Address[5]);
    }
    return strMacAddr;
}