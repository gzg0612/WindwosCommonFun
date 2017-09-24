#pragma once
#ifndef CRC32_7957B20D_4D3E_42a6_82F9_50121924FFDE
#define CRC32_7957B20D_4D3E_42a6_82F9_50121924FFDE
class CRC32
{
private:
	long ltblCRC32[257];
	void InitCRC32Table(void);
public:
	CRC32(void);
	~CRC32(void);
	unsigned long GetCRC32Value(const unsigned char * const pData, const unsigned long ulLength, const unsigned long ulOffset) const;
};
#endif