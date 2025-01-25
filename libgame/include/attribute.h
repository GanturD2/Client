#ifndef __INC_METIN_II_ATTRIBUTE_H__
#define __INC_METIN_II_ATTRIBUTE_H__

enum EDataType
{
    D_DWORD,
    D_WORD,
    D_BYTE
};

//
// �� �Ӽ����� ó���� �� ���
//
class CAttribute
{
    public:
	CAttribute(uint32_t width, uint32_t height); // dword Ÿ������ ��� 0�� ä���.
	CAttribute(uint32_t* attr, uint32_t width, uint32_t height); // attr�� �о smart�ϰ� �Ӽ��� �о�´�.
	~CAttribute();
	void Alloc();
	int GetDataType();
	void * GetDataPtr();
	void Set(DWORD x, DWORD y, DWORD attr);
	void Remove(DWORD x, DWORD y, DWORD attr);
	DWORD Get(DWORD x, DWORD y);
	void CopyRow(uint32_t y, uint32_t* row);

    private:
	void Initialize(DWORD width, DWORD height);

    private:
	int dataType;
	DWORD defaultAttr;
	DWORD width, height;

	void * data;

	BYTE **	bytePtr;
	WORD **	wordPtr;
	DWORD ** dwordPtr;
};

#endif
