#pragma once
#include <windows.h>
#include <winioctl.h>
#include <WinSock2.h> 
#include <Iphlpapi.h>
#include <atlstr.h>
class CComputerInfo
{
public:
    CComputerInfo(void);
    ~CComputerInfo(void);
    bool GetPhyDriveSerial(wchar_t* pModelNo, wchar_t* pSerialNo);
    CString GetMACaddress(void);
private:
    void ToLittleEndian(PUSHORT pWords, int nFirstIndex, int nLastIndex, wchar_t* pBuf);
    void TrimStart(wchar_t* pBuf);
};
