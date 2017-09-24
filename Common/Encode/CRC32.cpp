#include "StdAfx.h"
#include "CRC32.h"
#pragma once
CRC32::CRC32(void)
{
	memset(ltblCRC32, 0x00, 257);
	InitCRC32Table();
}

CRC32::~CRC32(void)
{
}

unsigned long CRC32::GetCRC32Value( const unsigned char * const pData, const unsigned long ulLength, const unsigned long ulOffset ) const
{
	long lCRC32Value = -1;
	const unsigned char *p = pData + ulOffset;
	unsigned long ulIndex = ulOffset; 
	while(ulIndex++ < ulLength)
		lCRC32Value = ((lCRC32Value >> 8) & 0xFFFFFF ) ^ ltblCRC32[( lCRC32Value & 0xFF ) ^ *p++];
	return ~lCRC32Value;
}

void CRC32::InitCRC32Table(void)
{
	long lPoly = -306674912;
	unsigned long lAccNum = 0L;
    for (unsigned short i = 0; i < 256; i++ )
	{
		lAccNum = i;
        for (unsigned short j = 0; j < 8; j++ )
			lAccNum = (lAccNum & 1) > 0 ? (( lAccNum >> 1 ) ^ lPoly) : (lAccNum >> 1);

		*(ltblCRC32 + i) = lAccNum;
	}
}
