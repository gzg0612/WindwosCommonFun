#include "IPC.h"
#include "..\io\FileMapping.h"
#include "IPCManager.h"

#pragma warning(disable:4996)

DECLEARNS(NSIPC)

#define IPC_INIT_FAILED(pNode, retValue, FreeNodeFuncName) { FreeNodeFuncName((pNode)); return retValue; }

// event array index = 0 : server write <--> client read
// event array index = 1 : client write <--> server read 
const enum { EN_IPC_CLIENT, EN_IPC_SERVER, EN_IPC_SERVER_MULTI};
static const wchar_t * CONST_STR_ARRAY_EVENT_NAME[] = { L"EventSW", L"EventSR", L"EventCW", L"EventCR" };   // Server Wirte, Server Read, Client Write, Client Read
static const unsigned char CONST_DEF_NODEID = 1;

typedef struct _st_ipc_node
{
    bool            bExit;
    char            nStateSend;     // -1: node can not found; 0: idle; 1: busy
    char            nStateRecv;
    long            lNodeID;    // seq inc id
    unsigned long   ulPID;      // server: client pid;  client : server pid
    CIPC            *pIPC;
    void            *pDataBuf;
    IIPCReciver     *pIPCRevicer;
    HANDLE          hRegistWaitObject;
    HANDLE          hAryEvent[_countof(CONST_STR_ARRAY_EVENT_NAME)];    // send buffer write complete, read complete;  recv buffer write complete, recv buffer read
} STIPCNODE, *LPSTIPCNODE;

typedef struct _st_ipc_data_header
{
    long lDataSize;
    long lOffset;
    long lReserved1;
    long lReserved2;
} STIPCDATAHEADER, *LPIPCDATAHEADER;

typedef struct _st_ipc_buf_pos
{
    long lFMBufPos;
    long lIPCPacketPos;
    long lIPCPacketSize;
} STIPCBUFINFO, *LPSTIPCBUFPOS;
static const unsigned char CONST_IPC_DATA_HEADER_SIZE = sizeof(STIPCDATAHEADER);

void CALLBACK NSIPC::OnIPCRecvDataWaitCallBack(PVOID lpParam, BOOLEAN bTimerOrWaitFired)
{
    STIPCNODE * const pNode = (STIPCNODE * const)lpParam;
    if (pNode == NULL) return;
    pNode->pIPC->_RecvData(pNode);
}

const char _GetRecvEventIndex(const long lIPCType)
{
    // normal : server recv 2, client recv 0
    return (lIPCType == EN_IPC_CLIENT ? 0 : 2);
}

const char _GetSendEventIndex(const long lIPCType)
{
    // normal : server send 0, client send 2
    return (lIPCType == EN_IPC_CLIENT ? 2 : 0);
}

inline void _GetBufPos(OUT STIPCBUFINFO * const pBufPos, const char nEventIndex, const long lFMBufSize)
{
    long lBufSize = (lFMBufSize >> 1);
    pBufPos->lFMBufPos = lBufSize * (nEventIndex >> 1);  // buf: 2 blocks, nEventIndex: 0~3, nEventIndex / 2 --> buf index
    pBufPos->lIPCPacketPos = pBufPos->lFMBufPos + CONST_IPC_DATA_HEADER_SIZE;
    pBufPos->lIPCPacketSize = lBufSize - CONST_IPC_DATA_HEADER_SIZE;
}

const long _WriteData(const char nEventIndex, STIPCNODE * const pNode, const char * pBuf, const long lDataSize)
{
    CFileMapping * pFileMapping = (CFileMapping*)pNode->pDataBuf;
    STIPCBUFINFO stBufInfo;
    _GetBufPos(&stBufInfo, nEventIndex, pFileMapping->GetBufSize());

    STIPCDATAHEADER stHeader = { lDataSize, 0, 0, 0 };
    STIPCDATAHEADER stHeaderRecvRet = { lDataSize, 0, 0, 0 };
    while (!pNode->bExit && stHeader.lOffset + stBufInfo.lIPCPacketSize < lDataSize)    // write count: (lDataSize / stBufInfo.lIPCPacketSize)
    {
        pFileMapping->Write((char*)&stHeader, CONST_IPC_DATA_HEADER_SIZE, stBufInfo.lFMBufPos);
        stHeader.lOffset += pFileMapping->Write(pBuf + stHeader.lOffset, stBufInfo.lIPCPacketSize, stBufInfo.lIPCPacketPos);
        ::SetEvent(pNode->hAryEvent[nEventIndex]);
        ::WaitForSingleObject(pNode->hAryEvent[nEventIndex + 1], INFINITE);
        pFileMapping->Read((char*)&stHeaderRecvRet, CONST_IPC_DATA_HEADER_SIZE, stBufInfo.lFMBufPos);
        if (stHeaderRecvRet.lDataSize != lDataSize) break;
    }
   
    // write last packet
    if (!pNode->bExit && stHeaderRecvRet.lDataSize == lDataSize)
    {
        pFileMapping->Write((char*)&stHeader, CONST_IPC_DATA_HEADER_SIZE, stBufInfo.lFMBufPos);
        stHeader.lOffset += pFileMapping->Write(pBuf + stHeader.lOffset, lDataSize - stHeader.lOffset, stBufInfo.lIPCPacketPos);
        ::SetEvent(pNode->hAryEvent[nEventIndex]);
        ::WaitForSingleObject(pNode->hAryEvent[nEventIndex + 1], INFINITE);
    }
    else
    {
        stHeader.lOffset = stHeaderRecvRet.lOffset;
        ::SetEvent(pNode->hAryEvent[nEventIndex]);
    }

    return stHeader.lOffset;
}

const long _ReadData(const char nEventIndex, STIPCNODE * const pNode, OUT char ** const pRet, OUT long * plSizeRet)
{
    CFileMapping * pFileMapping = (CFileMapping*)pNode->pDataBuf;
    STIPCBUFINFO stBufInfo;
    _GetBufPos(&stBufInfo, nEventIndex, pFileMapping->GetBufSize());

    STIPCDATAHEADER stHeader = { 0 };
    pFileMapping->Read((char*)&stHeader, CONST_IPC_DATA_HEADER_SIZE, stBufInfo.lFMBufPos);
    long lDataSize = stHeader.lDataSize;

    if (pRet == NULL || *pRet != NULL || lDataSize <= 0 || lDataSize > MAX_IPC_DATA)
    {
        ::SetEvent(pNode->hAryEvent[nEventIndex + 1]);
        return -1L;
    }

   char * pBufRead = (char *)malloc(lDataSize);
    long lTotalRead = 0L;
    while (!pNode->bExit && lTotalRead + stBufInfo.lIPCPacketSize < lDataSize)
    {
        pFileMapping->Read((char*)&stHeader, CONST_IPC_DATA_HEADER_SIZE, stBufInfo.lFMBufPos);
        if (stHeader.lDataSize != lDataSize) break;    // check total size || current read size

        lTotalRead += pFileMapping->Read(pBufRead + lTotalRead, stBufInfo.lIPCPacketSize, stBufInfo.lIPCPacketPos);
        ::SetEvent(pNode->hAryEvent[nEventIndex + 1]);
        ::WaitForSingleObject(pNode->hAryEvent[nEventIndex], INFINITE);
    }

    // read last data
    if (!pNode->bExit && stHeader.lDataSize == lDataSize)
    {
        lTotalRead += pFileMapping->Read(pBufRead + lTotalRead, lDataSize - lTotalRead, stBufInfo.lIPCPacketPos);
    }
    ::SetEvent(pNode->hAryEvent[nEventIndex + 1]);

    if (lTotalRead == lDataSize)
    {
        *pRet = pBufRead;
        *plSizeRet = stHeader.lDataSize;
    }
    else
    {
        free(pBufRead);
    }
    return lTotalRead;
}

const void _OnRecvEnd(const char nEventIndex, STIPCNODE * const pNode, const long lReadSize, const bool bReply = false)
{
    CFileMapping * pFileMapping = (CFileMapping*)pNode->pDataBuf;
    STIPCBUFINFO stBufInfo;
    _GetBufPos(&stBufInfo, nEventIndex, pFileMapping->GetBufSize());

    STIPCDATAHEADER stHeader = { 0, lReadSize, 0, 0 };
    pFileMapping->Write((char*)&stHeader, CONST_IPC_DATA_HEADER_SIZE, stBufInfo.lFMBufPos);

    if (bReply)
    {
        // event for _ReadData
        ::SetEvent(pNode->hAryEvent[nEventIndex]);
        ::WaitForSingleObject(pNode->hAryEvent[nEventIndex + 1], INFINITE);
    }
    else
    {
        // event for _WriteData
        ::SetEvent(pNode->hAryEvent[nEventIndex + 1]);
        ::WaitForSingleObject(pNode->hAryEvent[nEventIndex], INFINITE);
    }
}

// ------------------------------- CIPC -------------------------------
CIPC::CIPC(IIPCNotify * const pIPCNotify /*= NULL*/)
: m_lIPCType(-1)
, m_lNodeID(0L)
, m_wstrIPCName(L"")
, m_pIPCNotify(pIPCNotify)
, m_pMapNode(NULL)
{
}

CIPC::~CIPC(void)
{
    _FreeMem();
}

const void CIPC::_FreeMem()
{
    //CIPCManager::Destroy();   for test release
    if (m_pMapNode != NULL)
    {
        MapIPCNode::const_iterator it = m_pMapNode->begin();
        for (; it != m_pMapNode->end(); ++it) _FreeNode(it->second, false);
        delete m_pMapNode;
    }
    ::DeleteCriticalSection(&m_cs);
    //NSEVENTPOOL::CEventPool::Destory();   for test release
}

const bool CIPC::_Init(const wchar_t * const pwszIPCName, const unsigned char nIPCType)
{
    if (pwszIPCName == NULL || wcslen(pwszIPCName) < 0L) return false;
    if (!InitializeCriticalSectionAndSpinCount(&m_cs, 0x80000400)) return false;
    CIPCManager::CreateInstance();
    m_wstrIPCName = pwszIPCName;
    if (m_pMapNode == NULL) m_pMapNode = new MapIPCNode;
    m_lIPCType = nIPCType;
    return true;
}

const HANDLE CIPC::_InitEvent(const long lNodeID, const wchar_t * const pwszEventName)
{
    wchar_t wszName[MAX_PATH];
    _snwprintf(wszName, MAX_PATH, L"%s/%s%d", m_wstrIPCName.c_str(), pwszEventName, lNodeID);
    HANDLE hEvent = ::OpenEventW(EVENT_ALL_ACCESS | EVENT_MODIFY_STATE, FALSE, wszName);
    if (hEvent == NULL) hEvent = ::CreateEvent(NULL, FALSE, FALSE, wszName);
    return hEvent;
}

const void CIPC::_FreeNode(const void * const pNode, const bool bErase /*= true*/)
{
    if (pNode == NULL) return;
    STIPCNODE * p = (STIPCNODE *)pNode;
    p->bExit = true;
    CIPCManager::GetInstance()->RemoveRecvMonitor(p->hRegistWaitObject);
    while (p->nStateSend != EN_IPC_IDLE || p->nStateRecv != EN_IPC_IDLE)
    {
        for (long l = 0; l < _countof(CONST_STR_ARRAY_EVENT_NAME); ++l) ::SetEvent(p->hAryEvent[l]);
        ::Sleep(1);
    }
    for (long l = 0; l < _countof(CONST_STR_ARRAY_EVENT_NAME); ++l) ::CloseHandle(p->hAryEvent[l]);
    if (p->pDataBuf != NULL) { delete ((CFileMapping*)p->pDataBuf); }
    if (m_pMapNode != NULL && bErase)
    {
        ::EnterCriticalSection(&m_cs);
        MapIPCNode::iterator it = m_pMapNode->find(p->lNodeID);
        if (it != m_pMapNode->end()) m_pMapNode->erase(it);
        ::LeaveCriticalSection(&m_cs);
    }
    delete pNode;
}

const bool CIPC::_CreateNode(const long lNodeID, const unsigned long ulPID)
{
    STIPCNODE * pNode = (STIPCNODE *)_FindNode(lNodeID);
    if (pNode != NULL) return true;

    pNode = new STIPCNODE;
    memset(pNode, 0x00, sizeof(STIPCNODE));

    ::EnterCriticalSection(&m_cs);
    (*m_pMapNode)[lNodeID] = pNode; // add node to map
    ::LeaveCriticalSection(&m_cs);

    pNode->nStateSend = EN_IPC_IDLE;
    pNode->nStateRecv = EN_IPC_IDLE;
    pNode->lNodeID = lNodeID;
    pNode->ulPID = ulPID;
    pNode->pIPC = this;

    // init event 
    for (long l = 0; l < _countof(CONST_STR_ARRAY_EVENT_NAME); ++l)
    {
        if (NULL == (pNode->hAryEvent[l] = _InitEvent(lNodeID, CONST_STR_ARRAY_EVENT_NAME[l])))
            IPC_INIT_FAILED(pNode, false, _FreeNode);
    }

    // init File Mapping
    CFileMapping * pFM = new CFileMapping;
    pNode->pDataBuf = pFM;
    wchar_t wszBuf[MAX_PATH];
    _snwprintf(wszBuf, MAX_PATH, L"%s/fm%d", m_wstrIPCName.c_str(), lNodeID);
    if (!pFM->InitFileMapping(wszBuf)) IPC_INIT_FAILED(pNode, false, _FreeNode);

    // init recv thread
    pNode->hRegistWaitObject = CIPCManager::GetInstance()->AddRecvMonitor(pNode->hAryEvent[_GetRecvEventIndex(m_lIPCType)], pNode);
    if (NULL == pNode->hRegistWaitObject) IPC_INIT_FAILED(pNode, false, _FreeNode);

    return true;
}

void * const CIPC::_FindNode(const long lNodeID)
{
    if (m_pMapNode == NULL) return NULL;
    void * pNode = NULL;
    ::EnterCriticalSection(&m_cs);
    MapIPCNode::iterator it = m_pMapNode->find(lNodeID);
    if (it != m_pMapNode->end()) pNode = it->second;
    ::LeaveCriticalSection(&m_cs);
    return pNode;
}

const long CIPC::_SendData(const long lNodeID, const char * pBuf, const long lDataSize, OUT char ** const pRet /*= NULL*/, OUT long * const plSizeRet /*= NULL*/)
{
    if (pBuf == NULL || lDataSize <= 0L) return 0L;

    // server send: 0, client send 1 --> event index = 0*2 or 1*2
    STIPCNODE * const pNode = (STIPCNODE * const)_FindNode(lNodeID);
    if (pNode == NULL || pNode->bExit) return -1L;

    const char nEventIndex = _GetSendEventIndex(m_lIPCType);
    pNode->nStateSend = EN_IPC_BUSY;
    long lWriteLen = _WriteData(nEventIndex, pNode, pBuf, lDataSize);

    if (lWriteLen == lDataSize)
    {
        // send success : wait sync return data, use send buffer
        ::WaitForSingleObject(pNode->hAryEvent[nEventIndex], INFINITE);
        _ReadData(nEventIndex, pNode, pRet, plSizeRet);
    }
    pNode->nStateSend = EN_IPC_IDLE;
    return lWriteLen;
}

const long CIPC::_RecvData(void * const pNodeInfo)
{
    // server send to client --> client sync reply to server : use client recv buf, index 0 * 2
    // client send to server --> server sync reply to client : use server recv buf, index 1 * 2
    
    STIPCNODE * const pNode = (STIPCNODE * const)pNodeInfo;
    if (pNode == NULL || pNode->bExit) return -1L;

    const char nEventIndex = _GetRecvEventIndex(m_lIPCType);
    pNode->nStateRecv = EN_IPC_BUSY;
    if (pNode->pIPCRevicer == NULL)
    {
        _OnRecvEnd(nEventIndex, pNode, 0L);
        pNode->nStateRecv = EN_IPC_IDLE;
        return -1L;
    }

    // server recv 1, client recv 0 --> event index = 0*2 or 1*2
    char * pBuf = NULL;
    long lSizeRet = 0L;
    long lRead = _ReadData(nEventIndex, pNode, &pBuf, &lSizeRet);
    if (lRead != lSizeRet)
    {
        _OnRecvEnd(nEventIndex, pNode, lRead);
        pNode->nStateRecv = EN_IPC_IDLE;
        return -1L;
    }

    char * pBufReply = NULL;
    long lSizeReply = 0L;
    long lRet = pNode->pIPCRevicer->OnIPCRecvData(pNode->lNodeID, pBuf, lSizeRet, &pBufReply, &lSizeReply);
    if (pBuf != NULL) free(pBuf);

    if (pBufReply != NULL && lSizeReply > 0L)
    {
        // reply : only write data
        long lWriteLen = _WriteData(nEventIndex, pNode, pBufReply, lSizeReply);
        pNode->pIPCRevicer->OnFreeReplyData(pBufReply);
    }
    else
    {
        _OnRecvEnd(nEventIndex, pNode, 0, true);
    }
    pNode->nStateRecv = EN_IPC_IDLE;
    return lRet;
}

const long CIPC::_OnNodeDisconnect(const long lNodeID, const unsigned long ulPID)
{
    if (m_pIPCNotify == NULL) return -1L;
    _FreeNode(_FindNode(lNodeID), true);
    m_pIPCNotify->OnNodeConnect(this, lNodeID, ulPID, false);
    return 0L;
}

// ------------------ public ------------------
const long NSIPC::CIPC::GetNodeCount()
{
    if (m_pMapNode != NULL) return (long)m_pMapNode->size();
    return 0L;
}

const long CIPC::ConnectToServer(const wchar_t * const pwszIPCName, const unsigned long ulServerPID)
{
    if (!_Init(pwszIPCName, EN_IPC_CLIENT)) return false;
    const long lNodeID_FromServer = CIPCManager::GetInstance()->GetIPCConnectionManager()->ConnectToServer(pwszIPCName, ulServerPID, this);
    if (lNodeID_FromServer <= 0L) return false;
    m_lNodeID = lNodeID_FromServer;
    _CreateNode(m_lNodeID, ulServerPID);
    return lNodeID_FromServer;
}

const bool CIPC::InitIPCServer(const wchar_t * const pwszIPCName, const bool bMultiClient)
{
    if (!_Init(pwszIPCName, (bMultiClient == true ? EN_IPC_SERVER_MULTI : EN_IPC_SERVER))) return false;
    if (bMultiClient) CIPCManager::GetInstance()->GetIPCConnectionManager()->AddIPCServer(pwszIPCName, this);
    return true;
}

const long CIPC::AddClientNode(const unsigned long ulClientPID)
{
    if (!_CreateNode(m_lNodeID + 1, ulClientPID)) return -1L;
    if (m_pIPCNotify != NULL) m_pIPCNotify->OnNodeConnect(this, m_lNodeID + 1, ulClientPID, true);
    m_lNodeID++;
    return m_lNodeID;
}

const void CIPC::SetIPCRevicer(const long lNodeID, IIPCReciver * const pIPCRevicer)
{
    if (pIPCRevicer == NULL) return;
    STIPCNODE * const pNode = (STIPCNODE * const)_FindNode(lNodeID);
    if (pNode != NULL) pNode->pIPCRevicer = pIPCRevicer;
}

const void CIPC::FreeRecvBuf(char ** const pReadBuf) const
{
    if (pReadBuf == NULL) return;
    if (*pReadBuf != NULL) { free((*pReadBuf)); *pReadBuf = NULL; }
}

const long CIPC::SendData(const char * pBuf, const long lDataSize, OUT char ** const pRet /*= NULL*/, OUT long * const plSizeRet /*= NULL*/)
{
    return SendData(m_lNodeID, pBuf, lDataSize, pRet, plSizeRet);
}

const long CIPC::SendData(const long lNodeID, const char * pBuf, const long lDataSize, OUT char ** const pRet /*= NULL*/, OUT long * const plSizeRet /*= NULL*/)
{
    if (lNodeID > m_lNodeID || _FindNode(lNodeID) == NULL) return -1L;
    return CIPCManager::GetInstance()->AddSendTask(this, lNodeID, pBuf, lDataSize, pRet, plSizeRet);
}

const long CIPC::PostData(const char * pBuf, const long lDataSize, IIPCReciver * const pRevicer /*= NULL*/)
{
    return PostData(m_lNodeID, pBuf, lDataSize);
}

const long CIPC::PostData(const long lNodeID, const char * pBuf, const long lDataSize, IIPCReciver * const pRevicer /*= NULL*/)
{
    if (lNodeID > m_lNodeID) return -1L;    // block post data on AddClientNode --> m_pIPCNotify->OnNodeConnect
    STIPCNODE * const pNode = (STIPCNODE * const)_FindNode(lNodeID);
    if (pNode == NULL) return -1L;
    IIPCReciver * const pIPCRevicer = (pRevicer != NULL ? pRevicer : pNode->pIPCRevicer);
    return CIPCManager::GetInstance()->AddPostTask(this, lNodeID, pBuf, lDataSize, pIPCRevicer);
}

const long CIPC::GetNodeState(const long lNodeID)
{
    STIPCNODE * const pNode = (STIPCNODE * const)_FindNode(lNodeID);
    if (pNode == NULL) return EN_IPC_ERROR;
    return (pNode->nStateSend);
}

ENDDECLEARNS();