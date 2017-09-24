#pragma once

class AES  
{
public:
    //key:��Կ
	AES(unsigned char* key);
	virtual ~AES();
	unsigned char* Cipher(unsigned char* input);
	unsigned char* InvCipher(unsigned char* input);
    //input����Ҫ���ܵ����ݣ�length�����ĳ��ȣ�����Ϊ0����Ϊ0 ��֤ԭ��Ϊ16�ֽڶ���
	void* Cipher(void* input, int length=0);
    // input: ���ģ� length�����ĳ���
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
