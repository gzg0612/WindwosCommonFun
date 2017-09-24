#pragma once

class AES  
{
public:
    //key:密钥
	AES(unsigned char* key);
	virtual ~AES();
	unsigned char* Cipher(unsigned char* input);
	unsigned char* InvCipher(unsigned char* input);
    //input：需要加密的内容，length：明文长度，可以为0，若为0 许保证原文为16字节对齐
	void* Cipher(void* input, int length=0);
    // input: 密文， length：密文长度
	void* InvCipher(void* input, int length);

private:
	static unsigned char Sbox[256];
	static unsigned char InvSbox[256];
	unsigned char w[11][4][4];

	void KeyExpansion(unsigned char* key, unsigned char w[][4][4]);
	unsigned char FFmul(unsigned char a, unsigned char b);

	void SubBytes(unsigned char state[][4]);
	void ShiftRows(unsigned char state[][4]);
	void MixColumns(unsigned char state[][4]);
	void AddRoundKey(unsigned char state[][4], unsigned char k[][4]);

	void InvSubBytes(unsigned char state[][4]);
	void InvShiftRows(unsigned char state[][4]);
	void InvMixColumns(unsigned char state[][4]);
};
