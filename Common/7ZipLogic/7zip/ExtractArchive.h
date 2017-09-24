#pragma once

#include <initguid.h>
#include "../Windows/DLL.h"
#include "../Windows/FileDir.h"
#include "../Windows/FileFind.h"
#include "../Windows/FileName.h"
#include "../Windows/PropVariant.h"
#include "../Windows/PropVariantConv.h"

#include "Common/FileStreams.h"

#include "Archive/IArchive.h"

#include "IPassword.h"

using namespace NWindows;
using namespace NFile;
using namespace NDir;
#define DLL_7Z_DLL_NAME L"7zxr.dll"

static const wchar_t *kEmptyFileAlias = L"[Content]";

typedef const long(__stdcall *PFN_COMMON)(WPARAM wParam, LPARAM lParam);



class CExtractArchive
{
public:
    CExtractArchive(void);
    ~CExtractArchive(void);
    void Init(const wchar_t * pszDLLPath);
    void BeginExtract(const wchar_t * pszArchiveFilePath, const wchar_t * pszOutPutPath, PFN_COMMON pfnCallback);
    void Clear();
public:
    static HRESULT IsArchiveItemProp(IInArchive *archive, UInt32 index, PROPID propID, bool &result);
    static HRESULT IsArchiveItemFolder(IInArchive *archive, UInt32 index, bool &result);
private:
    NDLL::CLibrary m_lib;
    FString m_fstrArchiveFilePath;
    FString m_fstrOutPutPath;
    CMyComPtr<IInArchive> m_pArchiveHandler;
private:
    Func_CreateObject pCreateObjectFun;

};

class CArchiveOpenCallback:
    public IArchiveOpenCallback,
    public ICryptoGetTextPassword,
    public CMyUnknownImp
{
public:
    MY_UNKNOWN_IMP1(ICryptoGetTextPassword)

    STDMETHOD(SetTotal)(const UInt64 *files, const UInt64 *bytes);
    STDMETHOD(SetCompleted)(const UInt64 *files, const UInt64 *bytes);

    STDMETHOD(CryptoGetTextPassword)(BSTR *password);

    bool PasswordIsDefined;
    UString Password;

    CArchiveOpenCallback() : PasswordIsDefined(false) {}
};

class CArchiveExtractCallback:
    public IArchiveExtractCallback,
    public ICryptoGetTextPassword,
    public CMyUnknownImp
{
public:
    MY_UNKNOWN_IMP1(ICryptoGetTextPassword)

    // IProgress
    STDMETHOD(SetTotal)(UInt64 size);
    STDMETHOD(SetCompleted)(const UInt64 *completeValue);

    // IArchiveExtractCallback
    STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode);
    STDMETHOD(PrepareOperation)(Int32 askExtractMode);
    STDMETHOD(SetOperationResult)(Int32 resultEOperationResult);

    // ICryptoGetTextPassword
    STDMETHOD(CryptoGetTextPassword)(BSTR *aPassword);

private:
    CMyComPtr<IInArchive> _archiveHandler;
    FString _directoryPath;  // Output directory
    UString _filePath;       // name inside arcvhive
    FString _diskFilePath;   // full path to file on disk
    bool _extractMode;
    struct CProcessedFileInfo
    {
        FILETIME MTime;
        UInt32 Attrib;
        bool isDir;
        bool AttribDefined;
        bool MTimeDefined;
    } _processedFileInfo;

    COutFileStream *_outFileStreamSpec;
    CMyComPtr<ISequentialOutStream> _outFileStream;
    UInt32 m_ui32totalCount;
    PFN_COMMON m_pProccessFn;

public:
    void Init(IInArchive *archiveHandler, const FString &directoryPath, PFN_COMMON pfnCallback);
    void SetTotleCount(UInt32 ui32TotleCount);
    UInt64 NumErrors;
    bool PasswordIsDefined;
    UString Password;

    CArchiveExtractCallback() : PasswordIsDefined(false) {}
};