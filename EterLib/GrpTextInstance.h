#ifndef __INC_ETERLIB_GRPTEXTINSTANCE_H__
#define __INC_ETERLIB_GRPTEXTINSTANCE_H__

#include "Pool.h"
#include "GrpText.h"
#ifdef ENABLE_EMOJI_TEXTLINE
#	include "GrpImageInstance.h"
#endif

class CGraphicTextInstance
{
public:
	typedef CDynamicPool<CGraphicTextInstance> TPool;

public:
	enum EHorizontalAlign
	{
		HORIZONTAL_ALIGN_LEFT = 0x01,
		HORIZONTAL_ALIGN_CENTER = 0x02,
		HORIZONTAL_ALIGN_RIGHT = 0x03
	};
	enum EVerticalAlign
	{
		VERTICAL_ALIGN_TOP = 0x10,
		VERTICAL_ALIGN_CENTER = 0x20,
		VERTICAL_ALIGN_BOTTOM = 0x30
	};

public:
	static void Hyperlink_UpdateMousePos(int x, int y);
	static int Hyperlink_GetText(char * buf, int len);

public:
	CGraphicTextInstance();
	virtual ~CGraphicTextInstance();

	void Destroy();

	void Update();
	void Render(RECT * pClipRect = nullptr);

	void ShowCursor();
	void HideCursor();
	int IsShowCursor();

	void ShowOutLine();
	void HideOutLine();

	void SetColor(uint32_t color);
	void SetColor(float r, float g, float b, float a = 1.0f);
#ifdef ENABLE_INGAME_WIKI
	uint32_t GetColor() const;
#endif

	void SetOutLineColor(uint32_t color);
	void SetOutLineColor(float r, float g, float b, float a = 1.0f);

	void SetHorizonalAlign(int hAlign);
	void SetVerticalAlign(int vAlign);
	void SetMax(int iMax);
	void SetTextPointer(CGraphicText * pText);
	void SetValueString(const std::string & c_stValue);
	void SetValue(const char * c_szText, size_t len = -1);
	void SetPosition(float fx, float fy, float fz = 0.0f);
#ifdef ENABLE_INGAME_WIKI
	float GetPositionX() const { return m_v3Position.x; }
	float GetPositionY() const { return m_v3Position.y; }
#endif
	void SetSecret(bool Value);
	void SetOutline(bool Value);
	void SetFeather(bool Value);
	void SetMultiLine(bool Value);
	void SetLimitWidth(float fWidth);
#ifdef ENABLE_OFFICAL_FEATURES
	uint16_t GetLineHeight();
#endif
#ifdef WJ_MULTI_TEXTLINE
	bool HasEnterToken();
	void ReAdjustedStanXY(int startPosition, float& fStanX, float& fStanY);
	void DisableEnterToken();
	void SetLineHeight(int iLineHeight);
#endif

	void GetTextSize(int * pRetWidth, int * pRetHeight);
#ifdef ENABLE_SUNG_MAHI_TOWER
	void GetCharacterWidth(short* sWidth);
#endif
	const std::string & GetValueStringReference();
	uint16_t GetTextLineCount();

	int PixelPositionToCharacterPosition(int iPixelPosition);
	int GetHorizontalAlign();
#ifdef ENABLE_INGAME_WIKI
	void SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom);
	void iSetRenderingRect(int iLeft, int iTop, int iRight, int iBottom);
	void SetRenderBox(RECT& renderBox);
#endif

protected:
	void __Initialize();
	int __DrawCharacter(CGraphicFontTexture * pFontTexture, uint16_t codePage, wchar_t text, uint32_t dwColor);
	void __GetTextPos(uint32_t index, float * x, float * y);
	int __GetTextTag(const wchar_t * src, int maxLen, int & tagLen, std::wstring & extraInfo);

protected:
	struct SHyperlink
	{
		int16_t sx;
		int16_t ex;
		std::wstring text;

		SHyperlink() : sx(0), ex(0) {}
	};

#ifdef ENABLE_EMOJI_TEXTLINE
	struct SEmoji
	{
		int16_t x;
		CGraphicImageInstance * pInstance;

		SEmoji() : x(0)
		{
			pInstance = nullptr;
		}
	};
#endif

protected:
	uint32_t m_dwTextColor;
	uint32_t m_dwOutLineColor;

	uint16_t m_textWidth;
	uint16_t m_textHeight;

	uint8_t m_hAlign;
	uint8_t m_vAlign;
#ifdef ENABLE_INGAME_WIKI
	RECT m_renderBox;
#endif

	uint16_t m_iMax;
	float m_fLimitWidth;

	bool m_isCursor;
	bool m_isSecret;
	bool m_isMultiLine;

	bool m_isOutline;
	float m_fFontFeather;
#ifdef ENABLE_OFFICAL_FEATURES
	int m_iLineHeight;
#endif
#ifdef WJ_MULTI_TEXTLINE
	wchar_t m_wcEndLine;
	bool m_bDisableEnterToken;
#endif

	/////

	std::string m_stText;
	D3DXVECTOR3 m_v3Position;

private:
	bool m_isUpdate;
	bool m_isUpdateFontTexture;

	CGraphicText::TRef m_roText;
	CGraphicFontTexture::TPCharacterInfomationVector m_pCharInfoVector;
	std::vector<uint32_t> m_dwColorInfoVector;
	std::vector<SHyperlink> m_hyperlinkVector;
#ifdef WJ_MULTI_TEXTLINE
	std::vector<bool> m_bEnterTokenVector;
#endif
#ifdef ENABLE_EMOJI_TEXTLINE
	std::vector<SEmoji> m_emojiVector;
#endif
#ifdef ENABLE_INGAME_WIKI
	bool m_bUseRenderingRect;
	RECT m_RenderingRect;
#endif

public:
	static void CreateSystem(uint32_t uCapacity);
	static void DestroySystem();

	static CGraphicTextInstance * New();
	static void Delete(CGraphicTextInstance * pkInst);

	static CDynamicPool<CGraphicTextInstance> ms_kPool;
};

extern const char * FindToken(const char * begin, const char * end);
extern int ReadToken(const char * token);

#endif