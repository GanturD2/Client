#pragma once

#include "GrpTexture.h"
#include "GrpImageTexture.h"
#include "GrpDIB.h"

#include <vector>
#include <map>

class CGraphicFontTexture : public CGraphicTexture
{
public:
	typedef std::pair<uint16_t, wchar_t> TCharacterKey;

	typedef struct SCharacterInfomation
	{
		int16_t index;
		int16_t width;
		int16_t height;
		float left;
		float top;
		float right;
		float bottom;
		float advance;
	} TCharacterInfomation;

	typedef std::vector<TCharacterInfomation *> TPCharacterInfomationVector;

public:
	CGraphicFontTexture();
	virtual ~CGraphicFontTexture();

	void Destroy();
#ifdef ENABLE_MINI_GAME_YUTNORI
	bool Create(const char * c_szFontName, int fontSize, bool bItalic, bool bBold);
#else
	bool Create(const char * c_szFontName, int fontSize, bool bItalic);
#endif

	bool CreateDeviceObjects();
	void DestroyDeviceObjects();

	bool CheckTextureIndex(uint32_t dwTexture);
	void SelectTexture(uint32_t dwTexture);

	bool UpdateTexture();

	TCharacterInfomation * GetCharacterInfomation(uint16_t codePage, wchar_t keyValue);
	TCharacterInfomation * UpdateCharacterInfomation(TCharacterKey code);

	bool IsEmpty() const;

protected:
	void Initialize();

	bool AppendTexture();

	HFONT GetFont(uint16_t codePage);

protected:
	typedef std::vector<CGraphicImageTexture *> TGraphicImageTexturePointerVector;
	typedef std::map<TCharacterKey, TCharacterInfomation> TCharacterInfomationMap;
	typedef std::map<uint16_t, HFONT> TFontMap;

protected:
	CGraphicDib m_dib;

	HFONT m_hFontOld;
	HFONT m_hFont;

	TGraphicImageTexturePointerVector m_pFontTextureVector;

	TCharacterInfomationMap m_charInfoMap;

	TFontMap m_fontMap;

	int m_x;
	int m_y;
	int m_step;
	bool m_isDirty;

	TCHAR m_fontName[LF_FACESIZE];
	LONG m_fontSize;
	bool m_bItalic;
#ifdef ENABLE_MINI_GAME_YUTNORI
	bool	m_bBold;
#endif
};
