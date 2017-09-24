#include "stdafx.h"
#include "ExtractArchive.h"
#include <stdio.h>
#include <atlstr.h>

DEFINE_GUID(CLSID_CFormat7z,
    0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatXz,
    0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x0C, 0x00, 0x00);

#define CLSID_Format CLSID_CFormat7z

CExtractArchive::CExtractArchive(void)
{
    m_fstrArchiveFilePath = us2fs(L"");
    m_fstrOutPutPath = us2fs(L"");
    pCreateObjectFun = NULL;
}

CExtractArchive::~CExtractArchive(void)
{
}

void CExtractArchive::Init(const wchar_t * pszDLLPath)
{
    FString fstrDllFolder = us2fs(pszDLLPath);
    if (!m_lib.Load(pszDLLPath))
    {
        OutputDebugStringW(L"Load dll failed!!");
    }
    pCreateObjectFun = (Func_CreateObject)m_lib.GetProc("CreateObject");
    //m_fstrArchiveFilePath = us2fs(pszArchiveFilePath);
    //m_fstrOutPutPath = us2fs(pszOutPutPath);
}

void CExtractArchive::BeginExtract(const wchar_t * pszArchiveFilePath, const wchar_t * pszOutPutPath, PFN_COMMON pfnCallback)
{
    CMyComPtr<IInArchive> archive;
    if (pCreateObjectFun(&CLSID_Format, &IID_IInArchive, (void **)&archive) != S_OK)
    {
        return;
    }

    CInFileStream *fileSpec = new CInFileStream;
    CMyComPtr<IInStream> file = fileSpec;
    if (!fileSpec->Open(pszArchiveFilePath))
    {
        OutputDebugStringW(L"Can not open archive file");
        return;
    }


    CArchiveOpenCallback *openCallbackSpec = new CArchiveOpenCallback;
    CMyComPtr<IArchiveOpenCallback> openCallback(openCallbackSpec);
    openCallbackSpec->PasswordIsDefined = false;

    const UInt64 scanSize = 1 << 23;
    HRESULT result1 = archive->Open(file, &scanSize, openCallback);
    if (result1 != S_OK)
    {
        return ;
    }
    UInt32 ui32Totle = 0;
    archive->GetNumberOfItems(&ui32Totle);
    CArchiveExtractCallback *extractCallbackSpec = new CArchiveExtractCallback;
    extractCallbackSpec->SetTotleCount(ui32Totle);
    CMyComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);
    extractCallbackSpec->Init(archive, pszOutPutPath, pfnCallback);
    extractCallbackSpec->PasswordIsDefined = false;
    HRESULT result = archive->Extract(NULL, (UInt32)(Int32)(-1), false, extractCallback);
    if (result != S_OK)
    {
        OutputDebugStringW(L"Extract Error");
        return;
    }
}

void CExtractArchive::Clear()
{
    m_lib.Free();
}

HRESULT CExtractArchive::IsArchiveItemProp(IInArchive *archive, UInt32 index, PROPID propID, bool &result)
{
    NCOM::CPropVariant prop;
    RINOK(archive->GetProperty(index, propID, &prop));
    if (prop.vt == VT_BOOL)
        result = VARIANT_BOOLToBool(prop.boolVal);
    else if (prop.vt == VT_EMPTY)
        result = false;
    else
        return E_FAIL;
    return S_OK;
}

HRESULT CExtractArchive::IsArchiveItemFolder(IInArchive *archive, UInt32 index, bool &result)
{
    return IsArchiveItemProp(archive, index, kpidIsDir, result);
}

STDMETHODIMP CArchiveOpenCallback::SetTotal(const UInt64 *files, const UInt64 *bytes)
{
    UNREFERENCED_PARAMETER(files);
    UNREFERENCED_PARAMETER(bytes);
    return S_OK;
}

STDMETHODIMP CArchiveOpenCallback::SetCompleted(const UInt64 *files, const UInt64 *bytes)
{
    UNREFERENCED_PARAMETER(files);
    UNREFERENCED_PARAMETER(bytes);
    return S_OK;
}

STDMETHODIMP CArchiveOpenCallback::CryptoGetTextPassword(BSTR *password)
{
    if (!PasswordIsDefined)
    {
        OutputDebugStringW(L"Password is not defined");
        return E_ABORT;
    }

    *password = ::SysAllocString(Password);
    return (*password) ? S_OK : E_OUTOFMEMORY;
}

void CArchiveExtractCallback::Init(IInArchive *archiveHandler, const FString &directoryPath, PFN_COMMON pfnCallback)
{
    NumErrors = 0;
    _archiveHandler = archiveHandler;
    _directoryPath = directoryPath;
    NName::NormalizeDirPathPrefix(_directoryPath);
    m_pProccessFn = pfnCallback;
}

void CArchiveExtractCallback::SetTotleCount(UInt32 ui32TotleCount)
{
    m_ui32totalCount = ui32TotleCount;
}

STDMETHODIMP CArchiveExtractCallback::SetTotal(UInt64 size)
{
    return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::SetCompleted(const UInt64 *completeValue)
{
    CString  str = L"";
    OutputDebugString(str);
    return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::GetStream(UInt32 index,
                                                ISequentialOutStream **outStream, Int32 askExtractMode)
{
    *outStream = 0;
    _outFileStream.Release();

    {
        // Get Name
        NCOM::CPropVariant prop;
        RINOK(_archiveHandler->GetProperty(index, kpidPath, &prop));

        UString fullPath;
        if (prop.vt == VT_EMPTY)
            fullPath = kEmptyFileAlias;
        else
        {
            if (prop.vt != VT_BSTR)
                return E_FAIL;
            fullPath = prop.bstrVal;
        }


        _filePath = fullPath;
    }

    if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
        return S_OK;

    {
        // Get Attrib
        NCOM::CPropVariant prop;
        RINOK(_archiveHandler->GetProperty(index, kpidAttrib, &prop));
        if (prop.vt == VT_EMPTY)
        {
            _processedFileInfo.Attrib = 0;
            _processedFileInfo.AttribDefined = false;
        }
        else
        {
            if (prop.vt != VT_UI4)
                return E_FAIL;
            _processedFileInfo.Attrib = prop.ulVal;
            _processedFileInfo.AttribDefined = true;
        }
    }

    RINOK(CExtractArchive::IsArchiveItemFolder(_archiveHandler, index, _processedFileInfo.isDir));

    {
        // Get Modified Time
        NCOM::CPropVariant prop;
        RINOK(_archiveHandler->GetProperty(index, kpidMTime, &prop));
        _processedFileInfo.MTimeDefined = false;
        switch (prop.vt)
        {
        case VT_EMPTY:
            // _processedFileInfo.MTime = _utcMTimeDefault;
            break;
        case VT_FILETIME:
            _processedFileInfo.MTime = prop.filetime;
            _processedFileInfo.MTimeDefined = true;
            break;
        default:
            return E_FAIL;
        }

    }
    {
        // Get Size
        NCOM::CPropVariant prop;
        RINOK(_archiveHandler->GetProperty(index, kpidSize, &prop));
        UInt64 newFileSize;
        /* bool newFileSizeDefined = */ ConvertPropVariantToUInt64(prop, newFileSize);
    }


    {
        // Create folders for file
        int slashPos = _filePath.ReverseFind_PathSepar();
        if (slashPos >= 0)
            CreateComplexDir(_directoryPath + us2fs(_filePath.Left(slashPos)));
    }

    FString fullProcessedPath = _directoryPath + us2fs(_filePath);
    _diskFilePath = fullProcessedPath;

    if (_processedFileInfo.isDir)
    {
        CreateComplexDir(fullProcessedPath);
    }
    else
    {
        NFind::CFileInfo fi;
        if (fi.Find(fullProcessedPath))
        {
            if (!DeleteFileAlways(fullProcessedPath))
            {
                OutputDebugStringW(L"Can not delete output file 209");
                return E_ABORT;
            }
        }

        _outFileStreamSpec = new COutFileStream;
        CMyComPtr<ISequentialOutStream> outStreamLoc(_outFileStreamSpec);
        if (!_outFileStreamSpec->Open(fullProcessedPath, CREATE_ALWAYS))
        {
            OutputDebugStringW(L"Can not open output file 218");
            return E_ABORT;
        }
        _outFileStream = outStreamLoc;
        *outStream = outStreamLoc.Detach();
    }

    if (NULL != m_pProccessFn)
    {

        if (m_pProccessFn(2, (LPARAM)static_cast<int> ((float)((index * 100) / (m_ui32totalCount * 100)) * 100)))
        {
            _archiveHandler->Close();
            m_pProccessFn(3, (LPARAM)0);
            return S_FALSE;
        }
    }
    return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::PrepareOperation(Int32 askExtractMode)
{
    _extractMode = false;
    switch (askExtractMode)
    {
    case NArchive::NExtract::NAskMode::kExtract:  _extractMode = true; break;
    };
    return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::SetOperationResult(Int32 operationResult)
{
    switch (operationResult)
    {
    case NArchive::NExtract::NOperationResult::kOK:
        break;
    default:
        {
            NumErrors++;
            //const char *s = NULL;
            switch (operationResult)
            {
            case NArchive::NExtract::NOperationResult::kUnsupportedMethod:
                break;
            case NArchive::NExtract::NOperationResult::kCRCError:
                break;
            case NArchive::NExtract::NOperationResult::kDataError:
                break;
            case NArchive::NExtract::NOperationResult::kUnavailable:
                break;
            case NArchive::NExtract::NOperationResult::kUnexpectedEnd:
                break;
            case NArchive::NExtract::NOperationResult::kDataAfterEnd:
                break;
            case NArchive::NExtract::NOperationResult::kIsNotArc:
                break;
            case NArchive::NExtract::NOperationResult::kHeadersError:
                break;
            }
        }
    }

    if (_outFileStream)
    {
        if (_processedFileInfo.MTimeDefined)
            _outFileStreamSpec->SetMTime(&_processedFileInfo.MTime);
        RINOK(_outFileStreamSpec->Close());
    }
    _outFileStream.Release();
    if (_extractMode && _processedFileInfo.AttribDefined)
        SetFileAttrib(_diskFilePath, _processedFileInfo.Attrib);
    return S_OK;
}


STDMETHODIMP CArchiveExtractCallback::CryptoGetTextPassword(BSTR *password)
{
    if (!PasswordIsDefined)
    {
        // You can ask real password here from user
        // Password = GetPassword(OutStream);
        // PasswordIsDefined = true;
        OutputDebugStringW(L"Password is not defined");
        return E_ABORT;
    }

    *password = ::SysAllocString(Password);
    return (*password) ? S_OK : E_OUTOFMEMORY;
}