/*----------------------------------------------------------------
// Copyright (C) 2010.09 Qiuye 
//
// 文件名：DES.cpp
// 功  能：DES加密类 h文件
//----------------------------------------------------------------*/
#pragma once

class CDes
{
public:
	CDes(void);
	~CDes(void);

private:

	static const BYTE m_byTblPermutedChoice1[56];
	static const BYTE m_byTblPermutedChoice2[48];
	static const BYTE m_byTblSubKeyShift[16];
	static const BYTE m_byTblInitialPermutation[64];
	static const BYTE m_byTblExpansion[48];
	static const BYTE m_byTblSubstitution[8][64];
	static const BYTE m_byTblPermutation[32];
	static const BYTE m_byTblFinalPermutation[64];

private:
	static BOOLEAN _ExChangeByTable( IN const BYTE byTblSize, IN const BYTE* const lpbyTbl, IN const BYTE* const lpbySrcBuf, OUT BYTE* const lpbyDestBuf );

	static BOOLEAN _MakeSubKeyK48( OUT BYTE* const lpbyBuf_C28_D28, OUT BYTE* const lpbyDestAryK48 );
	static BOOLEAN _InitialPermutation( IN const BYTE* const lpbySrc, OUT BYTE* const lpbyDestLeft, OUT BYTE* const lpbyDestRight );
	static BOOLEAN _FounctionF(IN const BYTE* const lpbySubKeyK48Round, OUT BYTE * const lpbySecretKey, OUT BYTE* const lpbyLeft, OUT BYTE* const lpbyRight );
	static BOOLEAN _GetSBox( IN const BYTE* const lpbyXOR, OUT BYTE* const lpbyDestSBox);

	static BOOLEAN _EncryptAndDecrypt( IN const BYTE* const lpbySubKeyK48, IN const BYTE* const lpbySrcBuf_8Byte, OUT BYTE* const lpbySecretKey_C28D28, OUT BYTE * const lpbyDestBuf_8Byte, BYTE byProcType);
	static BOOLEAN _DESProc( IN BYTE byProcType, IN const BYTE* const lpbyKey_8Byte, IN const DWORD dwBufLength, IN const BYTE* lpbyInBuf, IN const DWORD dwMaxOutBufSize, OUT BYTE* lpbyOutBuf );
	static BOOLEAN _TripleDESProc(BYTE byProcType, IN const BYTE* const lpbyKey_8Byte1, IN const BYTE* const lpbyKey_8Byte2, IN const DWORD dwBufLength, IN const BYTE* lpbyInBuf, IN const DWORD dwMaxOutBufSize, OUT BYTE* lpbyOutBuf);

public:

	enum ProcType { ENCRYPT = 0x00, DECRYPT = 0x01 };
	
	static DWORD	GetRequiredBufSize( DWORD dwDataLen );
	static BOOLEAN	Encrypt( IN const BYTE* const lpbyKey_8Byte, IN const DWORD dwPlainTextLength, IN const BYTE* lpbyPlainTextBuf, IN const DWORD dwCipherTextLength, OUT BYTE* lpbyCipherTextBuf );
	static BOOLEAN	Decrypt( IN const BYTE* const lpbyKey_8Byte, IN const DWORD dwCipherTextLength, IN const BYTE* lpbyCipherTextBuf, IN const DWORD dwPlainTextLength, OUT BYTE* lpbyPlainTextBuf );
	static BOOLEAN	Encrypt3DES( IN const BYTE* const lpbyKey_8Byte1, IN const BYTE* const lpbyKey_8Byte2, IN const DWORD dwPlainTextLength, IN const BYTE* lpbyPlainTextBuf, IN const DWORD dwCipherTextLength, OUT BYTE* lpbyCipherTextBuf );
	static BOOLEAN	Decrypt3DES( IN const BYTE* const lpbyKey_8Byte1, IN const BYTE* const lpbyKey_8Byte2, IN const DWORD dwCipherTextLength, IN const BYTE* lpbyCipherTextBuf, IN const DWORD dwPlainTextLength, OUT BYTE* lpbyPlainTextBuf );

	static BOOLEAN	EncryptForTest( IN const BYTE* const lpbyKey_8Byte, IN const BYTE* const lpbyBuf_8Byte );

};
