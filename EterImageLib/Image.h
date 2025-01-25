#ifndef __INC_ETERIMAGELIB_IMAGE_H__
#define __INC_ETERIMAGELIB_IMAGE_H__

#include <Windows.h>
#include <string>

#pragma pack(push)
#pragma pack(1)
struct TGA_HEADER
{
	char idLen; // 0
	char palType; // �ķ�Ʈ������ 1, ���� 0
	char imgType; // �ķ�Ʈ������ 1, ���� 2
	uint16_t colorBegin; // 0
	uint16_t colorCount; // �ķ�Ʈ ������ 256, ���� 0
	char palEntrySize; // �ķ�Ʈ ������ 24, ���� 0
	uint16_t left;
	uint16_t top;
	uint16_t width;
	uint16_t height;
	char colorBits;
	char desc;
};
#define IMAGEDESC_ORIGIN_MASK 0x30
#define IMAGEDESC_TOPLEFT 0x20
#define IMAGEDESC_BOTLEFT 0x00
#define IMAGEDESC_BOTRIGHT 0x10
#define IMAGEDESC_TOPRIGHT 0x30
#pragma pack(pop)

class CImage
{
public:
	CImage();
	CImage(CImage & image);

	virtual ~CImage();

	void Destroy();

	virtual void Create(int width, int height);

	void Clear(uint32_t color = 0) const;

	int GetWidth() const;
	int GetHeight() const;

	uint32_t * GetBasePointer() const;
	uint32_t * GetLinePointer(int line) const;

	void PutImage(int x, int y, CImage * pImage) const;
	void FlipTopToBottom() const;

	void SetFileName(const char * c_szFileName);

	const std::string & GetFileNameString() const;

	bool IsEmpty() const;

protected:
	void Initialize();

protected:
	uint32_t * m_pdwColors;
	int m_width;
	int m_height;

	std::string m_stFileName;
};

#endif
