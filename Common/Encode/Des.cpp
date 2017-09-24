/*----------------------------------------------------------------
// Copyright (C) 2010.09 Qiuye 
//
// 文件名：DES.cpp
// 功  能：DES加密类 cpp文件
//----------------------------------------------------------------*/

#include "StdAfx.h"
#include ".\des.h"

#define HLINE1(a)		(((a & 0x80) >> 6) | ((a & 0x04) >> 2))
#define HCOL1(a)		(((a & 0x78) >> 3))
#define LLINE1(a, b)	((a & 0x02) | ((b & 0x10) >> 4))
#define LCOL1(a, b)		(((a & 0x01) << 3) | ((b & 0xE0) >> 5))

#define HLINE2(c, d)	(((c & 0x08) >> 2) | ((d & 0x40) >> 6))
#define HCOL2(c, d)		(((c & 0x07) << 1) | ((d & 0x80) >> 7))
#define LLINE2(d)		(((d & 0x21) >> 4) | (d & 0x01))
#define LCOL2(d)		(((d & 0x1E) >> 1))

#define SBOXTABLEINDEX(line, col)	(line << 4) + col

#define HSBOX1(i, a)		(m_byTblSubstitution[i][SBOXTABLEINDEX(HLINE1(a), HCOL1(a))] << 4)
#define LSBOX1(i, a, b)		(m_byTblSubstitution[i][SBOXTABLEINDEX(LLINE1(a, b), LCOL1(a, b))])
#define HSBOX2(i, c, d)		(m_byTblSubstitution[i][SBOXTABLEINDEX(HLINE2(c, d), HCOL2(c, d))] << 4)
#define LSBOX2(i, d)		(m_byTblSubstitution[i][SBOXTABLEINDEX(LLINE2(d), LCOL2(d))])

CDes::CDes(void)
{
}

CDes::~CDes(void)
{
}

const BYTE CDes::m_byTblPermutedChoice1[] = 
{
	57, 49, 41, 33, 25, 17,  9,  
	 1, 58, 50, 42, 34, 26, 18,
	10,  2, 59, 51, 43, 35, 27, 
	19, 11,  3, 60, 52, 44, 36,
	63, 55, 47, 39, 31, 23, 15,  
	 7, 62, 54, 46, 38, 30, 22,
	14,  6, 61, 53, 45, 37, 29, 
	21, 13,  5, 28, 20, 12,  4
};

const BYTE CDes::m_byTblSubKeyShift[16] = 
{
	1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1
};

const BYTE CDes::m_byTblPermutedChoice2[] = 
{
	14, 17, 11, 24,  1,  5,  
	 3, 28, 15,  6, 21, 10,
	23, 19, 12,  4, 26,  8, 
	16,  7, 27, 20, 13,  2,
	41, 52, 31, 37, 47, 55, 
	30, 40, 51, 45, 33, 48,
	44, 49, 39, 56, 34, 53, 
	46, 42, 50, 36, 29, 32
};

const BYTE CDes::m_byTblInitialPermutation[64] = 
{
	58, 50, 42, 34, 26, 18, 10, 2, 
	60, 52, 44, 36, 28, 20, 12, 4,
	62, 54, 46, 38, 30, 22, 14, 6, 
	64, 56, 48, 40, 32, 24, 16, 8,
	57, 49, 41, 33, 25, 17,  9, 1, 
	59, 51, 43, 35, 27, 19, 11, 3,
	61, 53, 45, 37, 29, 21, 13, 5, 
	63, 55, 47, 39, 31, 23, 15, 7
};

const BYTE CDes::m_byTblExpansion[48] =
{
	32,  1,  2,  3,  4,  5,  
	 4,  5,  6,  7,  8,  9,
	 8,  9, 10, 11, 12, 13, 
	12, 13, 14, 15, 16, 17,
	16, 17, 18, 19, 20, 21, 
	20, 21, 22, 23, 24, 25,
	24, 25, 26, 27, 28, 29, 
	28, 29, 30, 31, 32,  1
};

const BYTE CDes::m_byTblSubstitution[8][64] = 
{
	// S1
	14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
	0,  15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
	4,   1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
	15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13,
	// S2 
	15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
	 3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
	 0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
	13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9,
	// S3 
	10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
	13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
	13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
	 1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12,
	// S4 
	 7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
	13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
	10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
	 3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14,
	// S5 
	 2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
	14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
	 4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
	11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3,
	// S6 
	12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
	10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
	 9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
	 4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13,
	// S7 
	 4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
	13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
	 1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
	 6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12,
	// S8 
	13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
	 1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
	 7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
	 2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
};

const BYTE CDes::m_byTblPermutation[32] = 
{
	16,  7, 20, 21, 
	29, 12, 28, 17, 
	 1, 15, 23, 26, 
	 5, 18, 31, 10,
	 2,  8, 24, 14, 
	32, 27,  3,  9,  
	19, 13, 30,  6,  
	22, 11,  4, 25
};

const BYTE CDes::m_byTblFinalPermutation[64] = 
{
	40, 8, 48, 16, 56, 24, 64, 32, 
	39, 7, 47, 15, 55, 23, 63, 31,
	38, 6, 46, 14, 54, 22, 62, 30, 
	37, 5, 45, 13, 53, 21, 61, 29,
	36, 4, 44, 12, 52, 20, 60, 28, 
	35, 3, 43, 11, 51, 19, 59, 27,
	34, 2, 42, 10, 50, 18, 58, 26, 
	33, 1, 41,  9, 49, 17, 57, 25
};

BOOLEAN CDes::_ExChangeByTable( IN const BYTE byTblSize, IN const BYTE* const lpbyTbl, IN const BYTE* const lpbySrcBuf, OUT BYTE* const lpbyDestBuf )
{
	BYTE byPos = 0x00;
	BYTE byPosMod = 0x00;
	BYTE byMod = 0x00;
	for (BYTE i = 0; i < byTblSize; i++)
	{
		byPos = lpbyTbl[i] - 1;
		byPosMod = byPos % 8;
		lpbyDestBuf[i / 8] |= ((lpbySrcBuf[byPos / 8] & (0x80 >> byPosMod))) << byPosMod >> (i % 8);
		// ex. PermutedChoice1[0] = 57,  
		// the BYTE of 57th bit: X = byKey[57 / 8] & (0x80 >> (57 % 8 - 1))
		// the new pos of 57th bit : X << (byPosMod) >> (i % 8)
	}
	return TRUE;
}

BOOLEAN CDes::_MakeSubKeyK48( OUT BYTE* const lpbyBuf_C28_D28, OUT BYTE * const lpbyDestAryK48 )
{
	BYTE byLeftShiftCount = 0x00;
	BYTE byRightShiftForFillCount = 0x00;
	BYTE byCriticalFillCount = 0x00;
	BYTE byLoopLeftShiftHead = 0x00;
	BYTE byOldCritical = 0x00;
	BYTE byOldCriticalLow4bit = 0x00;
	BYTE byNewCriticalHigh4bit = 0x00;
	BYTE byNewCriticalLow4bit = 0x00;

	// make all sub key K16, the left shift is loop left shift, header bit move to the end
	for (BYTE i = 0; i < 16; i++)
	{
		// left shift count
		byLeftShiftCount = m_byTblSubKeyShift[i];
		byRightShiftForFillCount = 8 - byLeftShiftCount;

		// header & critical : C[28], D[28]. the 4th(28/8+4) BYTE is Critial BYTE, high 4 bit for C[28], low 4 bit for D[28]
		byLoopLeftShiftHead = lpbyBuf_C28_D28[0];
		byCriticalFillCount = 4 - byLeftShiftCount;
		
		lpbyBuf_C28_D28[0] = ((lpbyBuf_C28_D28[0] << byLeftShiftCount) | (lpbyBuf_C28_D28[1] >> byRightShiftForFillCount));
		lpbyBuf_C28_D28[1] = ((lpbyBuf_C28_D28[1] << byLeftShiftCount) | (lpbyBuf_C28_D28[2] >> byRightShiftForFillCount));
		lpbyBuf_C28_D28[2] = ((lpbyBuf_C28_D28[2] << byLeftShiftCount) | (lpbyBuf_C28_D28[3] >> byRightShiftForFillCount));

		byOldCritical = lpbyBuf_C28_D28[3];
		byOldCriticalLow4bit = (byOldCritical & 0x0F);
		byNewCriticalHigh4bit = ((((byOldCritical & 0xF0) << byLeftShiftCount) | (byLoopLeftShiftHead >> byCriticalFillCount)) & 0xF0);
		byNewCriticalLow4bit = ((byOldCriticalLow4bit << byLeftShiftCount) & 0x0F) | (lpbyBuf_C28_D28[4] >> byRightShiftForFillCount);

		lpbyBuf_C28_D28[3] = (byNewCriticalHigh4bit | byNewCriticalLow4bit);

		lpbyBuf_C28_D28[4] = ((lpbyBuf_C28_D28[4] << byLeftShiftCount) | (lpbyBuf_C28_D28[5] >> byRightShiftForFillCount));
		lpbyBuf_C28_D28[5] = ((lpbyBuf_C28_D28[5] << byLeftShiftCount) | (lpbyBuf_C28_D28[6] >> byRightShiftForFillCount));
		lpbyBuf_C28_D28[6] = ((lpbyBuf_C28_D28[6] << byLeftShiftCount) | (byOldCriticalLow4bit >> byCriticalFillCount));

		// Permuted Choice 2 (PC-2)
		_ExChangeByTable(48, (LPBYTE)m_byTblPermutedChoice2, lpbyBuf_C28_D28, lpbyDestAryK48 + (i * 6));
	}

	return TRUE;
}

BOOLEAN CDes::_InitialPermutation( IN const BYTE* const lpbySrc, OUT BYTE* const lpbyDestLeft, OUT BYTE* const lpbyDestRight )
{
	// 64 bit  = 8 Byte
	BYTE byInitialPermutation[8] = {0};
	_ExChangeByTable(64, (LPBYTE)m_byTblInitialPermutation, lpbySrc, byInitialPermutation);
	for (BYTE i = 0; i < 4; i++)
	{
		lpbyDestLeft[i] = byInitialPermutation[i];
		lpbyDestRight[i] = byInitialPermutation[i + 4];
	}
	return TRUE;
}

BOOLEAN CDes::_FounctionF( IN const BYTE* const lpbySubKeyK48Round, OUT BYTE * const lpbySecretKey, OUT BYTE* const lpbyLeft, OUT BYTE* const lpbyRight )
{
	// 将32位的R[I-1]按下表（E）扩展为48位的E[I-1]
	BYTE byAryRightEx[6] = {0};
	_ExChangeByTable(48, (LPBYTE)m_byTblExpansion, lpbyRight, byAryRightEx);

	// 异或E[I-1]和K[I]，即E[I-1] XOR K[I]
	BYTE byAryXOR[6] = {0};
	for (BYTE byJ = 0; byJ < 6/* 6=48/8 */; byJ	++)
		byAryXOR[byJ] = (byAryRightEx[byJ] ^ lpbySubKeyK48Round[byJ]);

	BYTE bySBox[4] = {0};
	BYTE bySBoxPermutation[4] = {0};

	// 按S表变换所有的 异或后的结果
	_GetSBox(byAryXOR, bySBox);

	// 将B[1]到B[8]组合，按下表[Permutation] 变换，得到P。
	_ExChangeByTable(32, (LPBYTE)m_byTblPermutation, bySBox, bySBoxPermutation);

	// L[I] = R[I] ,之后异或P和L[I-1]结果放在R[I]，即R[I] = P XOR L[I - 1]。 
	BYTE bySwap = 0x00;
	for (BYTE byJ = 0; byJ < 4; byJ++)
	{
		bySwap = lpbyRight[byJ];
		lpbyRight[byJ] = lpbyLeft[byJ] ^ bySBoxPermutation[byJ];
		lpbyLeft[byJ] = bySwap;
	}
	return TRUE;
}

BOOLEAN CDes::_GetSBox( IN const BYTE* const lpbyXOR, OUT BYTE* const lpbyDestSBox )
{
	// lpbyXOR : 6BYTE = 48bit, split lpbyXOR into 6bit * 8(lpbyDestSBox : 8BYTE)
	// lpbyXOR : bit (1, 6) == line Index of Substitution Box [I]
	// lpbyXOR : bit (2, 3, 4, 5) == col Index of Substitution Box [I]
	// get the number from Substitution Box as 4bit Binary. the number in SBox all < 1111(2) = 15(10)
	// 4 * 6bit = 3Byte (3 * 8bit)

	//                High 4 bit                        | Low 4 bit
	lpbyDestSBox[0] = HSBOX1(0, lpbyXOR[0])             | LSBOX1(1, lpbyXOR[0], lpbyXOR[1]);
	lpbyDestSBox[1] = HSBOX2(2, lpbyXOR[1], lpbyXOR[2]) | LSBOX2(3, lpbyXOR[2]);
	lpbyDestSBox[2] = HSBOX1(4, lpbyXOR[3])             | LSBOX1(5, lpbyXOR[3], lpbyXOR[4]);
	lpbyDestSBox[3] = HSBOX2(6, lpbyXOR[4], lpbyXOR[5]) | LSBOX2(7, lpbyXOR[5]);

	return TRUE;
}

BOOLEAN CDes::_EncryptAndDecrypt( IN const BYTE* const lpbySubKeyK48, IN const BYTE* const lpbySrcBuf_8Byte, OUT BYTE* const lpbySecretKey_C28D28, OUT BYTE * const lpbyDestBuf_8Byte, BYTE byProcType )
{
	if (lpbySecretKey_C28D28 == NULL || lpbySubKeyK48 == NULL || lpbySrcBuf_8Byte == NULL || lpbyDestBuf_8Byte == NULL || byProcType < ENCRYPT || byProcType > DECRYPT)
		return FALSE;

	BYTE byAryLeft[4] = {0};
	BYTE byAryRight[4] = {0};
    
	//// 变换密钥
	//_InitialSecretKey(lpbyKey_8Byte, bySecretKey);

	//// 生成16个子密钥K[I], 全部一次生成, 否则解密操作不方便
	//BYTE byAryK48[16][6] = {0};
	//_MakeSubKeyK48( bySecretKey, byAryK48[0] );	// bySecretKey = C0D0, byAryK48 = K[1..16]

	// 将64位数据按表Initial Permutation 变换（IP）, 将变换后的数据分为两部分，开始的32位称为L[0]，最后的32位称为R[0]。
	_InitialPermutation(lpbySrcBuf_8Byte, byAryLeft, byAryRight);

	// 进行16次数据处理
	if (byProcType == ENCRYPT)
	{
		for (BYTE i = 0; i < 16; i++)
			_FounctionF(lpbySubKeyK48 + i * 6, lpbySecretKey_C28D28, byAryLeft, byAryRight );
	}
	else
	{
		for (BYTE i = 0; i < 16; i++)
			_FounctionF(lpbySubKeyK48 + (15 - i) * 6, lpbySecretKey_C28D28, byAryLeft, byAryRight );
	}

	BYTE byRL[8] = {0};
	for (BYTE i = 0; i < 4; i++)
	{
		byRL[i] = byAryRight[i];
		byRL[i + 4] = byAryLeft[i];
	}

	// 组合变换后的最后一组R,L(R[16]L[16])（注意：R作为开始的32位），按表FinalPermutation变换得到最后的结果
	_ExChangeByTable(64, (LPBYTE)m_byTblFinalPermutation, byRL, lpbyDestBuf_8Byte);

	return TRUE;
}

BOOLEAN CDes::_DESProc( IN BYTE byProcType, IN const BYTE* const lpbyKey_8Byte, IN const DWORD dwBufLength, IN const BYTE* lpbyInBuf, IN const DWORD dwMaxOutBufSize, OUT BYTE* lpbyOutBuf )
{
	if (lpbyKey_8Byte == NULL || lpbyInBuf == NULL || dwBufLength == 0L || dwMaxOutBufSize == 0L || lpbyOutBuf == NULL || byProcType < ENCRYPT || byProcType > DECRYPT)
		return FALSE;

	DWORD dwNewDataLen = 0L;
	dwNewDataLen = GetRequiredBufSize(dwBufLength);

	if (dwNewDataLen == 0L || dwMaxOutBufSize < dwNewDataLen)
		return FALSE;

	BYTE bySecretKey[8] = {0};		// 变换后密钥
	BYTE byAryK48[16][6] = {0};		// 生成的子密钥K

	// 变换密钥 Permuted Choice 1 (PC-1)
	_ExChangeByTable(56, (LPBYTE)m_byTblPermutedChoice1, lpbyKey_8Byte, bySecretKey);

	// 生成16个子密钥K[I], 全部一次生成, 否则解密操作不方便
	_MakeSubKeyK48( bySecretKey, byAryK48[0] );	// bySecretKey = C0D0, byAryK48 = K[1..16]

	// clear out buf
	memset(lpbyOutBuf, 0x00, dwNewDataLen);

	// 每64bit(8字节)为一个处理单元进行编码
	const BYTE *pbyEnd = lpbyInBuf + dwNewDataLen;
	for (; lpbyInBuf < pbyEnd; lpbyInBuf +=8, lpbyOutBuf += 8)
		_EncryptAndDecrypt(byAryK48[0], lpbyInBuf, bySecretKey, lpbyOutBuf, byProcType);

	return TRUE;
}

BOOLEAN CDes::_TripleDESProc( BYTE byProcType, IN const BYTE* const lpbyKey_8Byte1, IN const BYTE* const lpbyKey_8Byte2, IN const DWORD dwBufLength, IN const BYTE* lpbyInBuf, IN const DWORD dwMaxOutBufSize, OUT BYTE* lpbyOutBuf )
{
	if (lpbyKey_8Byte1 == NULL || lpbyInBuf == NULL || dwBufLength == 0L || dwMaxOutBufSize == 0L || lpbyOutBuf == NULL || byProcType < ENCRYPT || byProcType > DECRYPT)
		return FALSE;

	BYTE bySecretKey1[8] = {0};		// 变换后密钥1
	BYTE bySecretKey2[8] = {0};		// 变换后密钥2
	BYTE byAryK48_1[16][6] = {0};	// 密钥1生成的子密钥K
	BYTE byAryK48_2[16][6] = {0};	// 密钥2生成的子密钥K

	// 变换密钥1,2	Permuted Choice 1 (PC-1)
	_ExChangeByTable(56, (LPBYTE)m_byTblPermutedChoice1, lpbyKey_8Byte1, bySecretKey1);
	_ExChangeByTable(56, (LPBYTE)m_byTblPermutedChoice1, lpbyKey_8Byte2, bySecretKey2);

	// 生成16个子密钥K[I], 全部一次生成, 否则解密操作不方便
	_MakeSubKeyK48( bySecretKey1, byAryK48_1[0] );	// bySecretKey = C0D0, byAryK48 = K[1..16]
	_MakeSubKeyK48( bySecretKey2, byAryK48_2[0] );	// bySecretKey = C0D0, byAryK48 = K[1..16]

	DWORD dwNewDataLen = 0L;
	dwNewDataLen = GetRequiredBufSize(dwBufLength);

	if (dwNewDataLen == 0L || dwMaxOutBufSize < dwNewDataLen)
		return FALSE;

	// clear out buf
	memset(lpbyOutBuf, 0x00, dwNewDataLen);
	
	// 每64bit(8字节)为一个处理单元进行编码
	const BYTE *pbyEnd = lpbyInBuf + dwNewDataLen;
	DWORD dwEnd = dwNewDataLen - 8;
	BYTE byProcByte[8] = {0};
	for (; lpbyInBuf < pbyEnd; lpbyInBuf +=8, lpbyOutBuf += 8)
	{
		memcpy(byProcByte, lpbyInBuf, 8);
		_EncryptAndDecrypt(byAryK48_1[0], byProcByte, bySecretKey1, lpbyOutBuf, byProcType);	// dwIndex << 3 === dwIndex * 8 编码结果保存地址偏移
		memset(byProcByte, 0x00, 8);	// clear temp buf
		_EncryptAndDecrypt(byAryK48_2[0], lpbyOutBuf, bySecretKey2, byProcByte, !byProcType);
		memset(lpbyOutBuf, 0x00, 8);	// clear temp buf
		_EncryptAndDecrypt(byAryK48_1[0], byProcByte, bySecretKey1, lpbyOutBuf, byProcType);
	}

	return TRUE;
}

// public functions
DWORD CDes::GetRequiredBufSize( DWORD dwDataLen )
{
	return dwDataLen == 0L ? 0L : 8 * (dwDataLen % 8 == 0 ? dwDataLen / 8 : dwDataLen / 8 + 1);
}

BOOLEAN CDes::Encrypt( IN const BYTE* const lpbyKey_8Byte, IN const DWORD dwPlainTextLength, IN const BYTE* lpbyPlainTextBuf, IN const DWORD dwCipherTextLength, OUT BYTE* lpbyCipherTextBuf )
{
	return _DESProc(ENCRYPT, lpbyKey_8Byte, dwPlainTextLength, lpbyPlainTextBuf, dwCipherTextLength, lpbyCipherTextBuf);
}

BOOLEAN CDes::Decrypt( IN const BYTE* const lpbyKey_8Byte, IN const DWORD dwCipherTextLength, IN const BYTE* lpbyCipherTextBuf, IN const DWORD dwPlainTextLength, OUT BYTE* lpbyPlainTextBuf )
{
	
	return _DESProc(DECRYPT, lpbyKey_8Byte, dwCipherTextLength, lpbyCipherTextBuf, dwPlainTextLength, lpbyPlainTextBuf);
}

BOOLEAN CDes::Encrypt3DES( IN const BYTE* const lpbyKey_8Byte1, IN const BYTE* const lpbyKey_8Byte2, IN const DWORD dwPlainTextLength, IN const BYTE* lpbyPlainTextBuf, IN const DWORD dwCipherTextLength, OUT BYTE* lpbyCipherTextBuf )
{
	return _TripleDESProc(ENCRYPT, lpbyKey_8Byte1, lpbyKey_8Byte2, dwPlainTextLength, lpbyPlainTextBuf, dwCipherTextLength, lpbyCipherTextBuf);
}

BOOLEAN CDes::Decrypt3DES( IN const BYTE* const lpbyKey_8Byte1, IN const BYTE* const lpbyKey_8Byte2, IN const DWORD dwCipherTextLength, IN const BYTE* lpbyCipherTextBuf, IN const DWORD dwPlainTextLength, OUT BYTE* lpbyPlainTextBuf )
{
	return _TripleDESProc(DECRYPT, lpbyKey_8Byte1, lpbyKey_8Byte2, dwCipherTextLength, lpbyCipherTextBuf, dwPlainTextLength, lpbyPlainTextBuf);
}

BOOLEAN CDes::EncryptForTest( IN const BYTE* const lpbyKey_8Byte, IN const BYTE* const lpbyBuf_8Byte )
{
	BYTE bySecretKey[8] = {0};		// 变换后密钥
	BYTE byAryK48[16][6] = {0};		// 生成的子密钥K

	// 变换密钥
	_ExChangeByTable(56, (LPBYTE)m_byTblPermutedChoice1, lpbyKey_8Byte, bySecretKey);

	// 生成16个子密钥K[I], 全部一次生成, 否则解密操作不方便
	_MakeSubKeyK48( bySecretKey, byAryK48[0] );	// bySecretKey = C0D0, byAryK48 = K[1..16]

	BYTE byAryLeft[4] = {0};
	BYTE byAryRight[4] = {0};

	// 将64位数据按表Initial Permutation 变换（IP）, 将变换后的数据分为两部分，开始的32位称为L[0]，最后的32位称为R[0]。
	_InitialPermutation(lpbyBuf_8Byte, byAryLeft, byAryRight);

	// 进行16次数据处理
	for (BYTE i = 0; i < 16; i++)
		_FounctionF(byAryK48[i], bySecretKey, byAryLeft, byAryRight );

	BYTE byRL[8] = {0};
	for (BYTE i = 0; i < 4; i++)
	{
		byRL[i] = byAryRight[i];
		byRL[i + 4] = byAryLeft[i];
	}

	// 组合变换后的最后一组R,L(R[16]L[16])（注意：R作为开始的32位），按表FinalPermutation变换得到最后的结果
	BYTE byEncrptyData[8] = {0};
	_ExChangeByTable(64, (LPBYTE)m_byTblFinalPermutation, byRL, byEncrptyData);
	return TRUE;
}