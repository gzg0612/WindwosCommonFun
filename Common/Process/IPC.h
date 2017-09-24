/*----------------------------------------------------------------
// IPC Qiuye
// namespace : NSIPC
// class     : CIPC
// useage    :
// server    : CIPC::SetIPCRevicer CIPC::InitIPCServer    IIPCNotify::OnNodeConnect  CIPC::SendData  IIPCReciver::OnIPCRecvData  
// client    : CIPC::SetIPCRevicer CIPC::ConnectToServer  IIPCNotify::OnNodeConnect  CIPC::SendData  IIPCReciver::OnIPCRecvData
//----------------------------------------------------------------*/

#pragma once
#if _WIN32_WINNT < 0x0500
#define _WIN32_WINNT 0x0500
#endif

#include <windows.h>
#include <string>
#include <map>

#ifndef DECLEARNS
#define DECLEARNS(ns) namespace ns {
#define ENDDECLEARNS() };
#endif // !DECLEARNS

DECLEARNS(NSIPC)

static const unsigned long MAX_IPC_DATA = 50 * 1024 * 1024;
static const wchar_t * const STR_CONNECT_WNDNAME = L"IPCCMWND";
const enum { EN_IPC_CONN_CLIENT_NOT_ALIVE = -4, EN_IPC_CONN_SERVER_NOT_EXIST = -2, EN_IPC_CONN_NODE_NOT_EXIST = -1, EN_IPC_ERROR = -1, EN_IPC_IDLE, EN_IPC_BUSY };
static const bool ENABLE_IPC_ALIVE_MONITOR = true;

class CIPC;

class IIPCReciver
{
public:
    virtual const long OnIPCRecvData(const long lNodeID, const char * const pBuf, const long lSize, char ** const ppBufReply = NULL, long * const plSizeReply = NULL) = NULL;
    virtual void OnFreeReplyData(void * const pData) = NULL;
};

class IIPCNotify
{
public:
    // OnNodeConnect, here send data will failed
    virtual const long OnNodeConnect(CIPC * const pIPC, const long lNodeID, const unsigned long ulPID, const bool bConnected) = NULL;
};

class CIPC
{
public:
    CIPC(IIPCNotify * const pIPCNotify);
    ~CIPC(void);
      
private:

    typedef std::map<long, void *> MapIPCNode;

    long            m_lIPCType;
    long            m_lNodeID;
    std::wstring    m_wstrIPCName;
    IIPCNotify     *m_pIPCNotify;
    MapIPCNode     *m_pMapNode;
    CRITICAL_SECTION m_cs;

private:

    const void   _FreeMem();
    const bool   _Init(const wchar_t * const pwszIPCName, const unsigned char nIPCType);
    const HANDLE _InitEvent(const long lNodeID, const wchar_t * const pwszEventName);
    const void   _FreeNode(const void * const pNode, const bool bErase = true);
    const bool   _CreateNode(const long lNodeID, const unsigned long ulPID);
    void * const _FindNode(const long lNodeID);

    // ---------- friend ----------
    friend class CIPCManager;
    const long _SendData(const long lNodeID, const char * pBuf, const long lDataSize, OUT char ** const pRet = NULL, OUT long * const plSizeRet = NULL);

    friend void CALLBACK OnIPCRecvDataWaitCallBack(PVOID lpParam, BOOLEAN bTimerOrWaitFired);
    const long _RecvData(void * const pNode);

    friend class CIPCConnectionManager;
    const long _OnNodeDisconnect(const long lNodeID, const unsigned long ulPID);

public:

    const wchar_t * const GetIPCName() { return m_wstrIPCName.c_str(); }
    const long GetNodeCount();
    const long ConnectToServer(const wchar_t * const pwszIPCName, const unsigned long ulServerPID);
    const bool InitIPCServer(const wchar_t * const pwszIPCName, const bool bMultiClient);
    const long AddClientNode(const unsigned long ulClientPID);
    const void SetIPCRevicer(const long lNodeID, IIPCReciver * const pIPCRevicer);

    const void FreeRecvBuf(char ** const pReadBuf) const;
    const long SendData(const char * pBuf, const long lDataSize, OUT char ** const pRet = NULL, OUT long * const plSizeRet = NULL);
    const long SendData(const long lNodeID, const char * pBuf, const long lDataSize, OUT char ** const pRet = NULL, OUT long * const plSizeRet = NULL);
    const long PostData(const char * pBuf, const long lDataSize, IIPCReciver * const pRevicer = NULL);
    const long PostData(const long lNodeID, const char * pBuf, const long lDataSize, IIPCReciver * const pRevicer = NULL);

    const long GetNodeState(const long lNodeID);
};

ENDDECLEARNS();