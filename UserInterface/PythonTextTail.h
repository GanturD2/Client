#pragma once

#include "../EterBase/Singleton.h"

class CPythonTextTail : public CSingleton<CPythonTextTail>
{
public:
	typedef struct STextTail
	{
		CGraphicTextInstance * pTextInstance;
		CGraphicTextInstance * pOwnerTextInstance;

		CGraphicMarkInstance * pMarkInstance;
		CGraphicTextInstance * pGuildNameTextInstance;

		CGraphicTextInstance * pTitleTextInstance;
		CGraphicTextInstance * pLevelTextInstance;
#ifdef WJ_SHOW_MOB_INFO
		CGraphicTextInstance * pAIFlagTextInstance;
#endif
#ifdef ENABLE_BATTLE_FIELD
		CGraphicImageInstance * pRankingInstance;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		CGraphicImageInstance * pLanguageImageInstance;
#endif

		CGraphicObjectInstance * pOwner;

		uint32_t dwVirtualID;

		float x, y, z;
		float fDistanceFromPlayer;
		D3DXCOLOR Color;
		BOOL bNameFlag;

		float xStart, yStart;
		float xEnd, yEnd;

		uint32_t LivingTime;

		float fHeight;
#ifdef WJ_SHOW_MOB_INFO
		BOOL bIsPC;
#endif

		STextTail() = default;
		virtual ~STextTail() = default;
		CLASS_DELETE_COPYMOVE(STextTail);
	} TTextTail;

	typedef std::map<uint32_t, TTextTail *> TTextTailMap;
	using TTextTailList = std::list<TTextTail *>;
	using TChatTailMap = TTextTailMap;

public:
	CPythonTextTail();
	~CPythonTextTail();
	CLASS_DELETE_COPYMOVE(CPythonTextTail);

	void GetInfo(std::string * pstInfo);

	void Initialize() const;
	void Destroy();
	void Clear();

	void UpdateAllTextTail();
	void UpdateShowingTextTail();
	void Render();

	void ArrangeTextTail();
	void HideAllTextTail();
	void ShowAllTextTail();
	void ShowCharacterTextTail(uint32_t VirtualID);
	void ShowItemTextTail(uint32_t VirtualID);

#ifdef ENABLE_SHOW_GUILD_LEADER
	void RegisterCharacterTextTail(uint32_t dwGuildID, uint8_t m_dwNewIsGuildName, uint32_t dwVirtualID, const D3DXCOLOR & c_rColor, float fAddHeight = 10.0f);
#else
	void RegisterCharacterTextTail(uint32_t dwGuildID, uint32_t dwVirtualID, const D3DXCOLOR & c_rColor, float fAddHeight = 10.0f);
#endif
#ifdef ENABLE_EXTENDED_ITEMNAME_ON_GROUND
	void RegisterItemTextTail(uint32_t VirtualID, const char * c_szText, CGraphicObjectInstance * pOwner, bool bHasAttr = false);
#else
	void RegisterItemTextTail(uint32_t VirtualID, const char * c_szText, CGraphicObjectInstance * pOwner);
#endif
	void RegisterChatTail(uint32_t VirtualID, const char * c_szChat);
	void RegisterInfoTail(uint32_t VirtualID, const char * c_szChat);
	void SetCharacterTextTailColor(uint32_t VirtualID, const D3DXCOLOR & c_rColor);
	void SetItemTextTailOwner(uint32_t dwVID, const char * c_szName);
	void DeleteCharacterTextTail(uint32_t VirtualID);
	void DeleteItemTextTail(uint32_t VirtualID);

	int Pick(int ixMouse, int iyMouse);
	void SelectItemName(uint32_t dwVirtualID);

	bool GetTextTailPosition(uint32_t dwVID, float * px, float * py, float * pz);
	bool IsChatTextTail(uint32_t dwVID);

	void EnablePKTitle(BOOL bFlag) const;
	void AttachTitle(uint32_t dwVID, const char * c_szName, const D3DXCOLOR & c_rColor);
	void DetachTitle(uint32_t dwVID);

	void AttachLevel(uint32_t dwVID, const char * c_szText, const D3DXCOLOR & c_rColor, const D3DXCOLOR& Outline);	//@custom014
	void DetachLevel(uint32_t dwVID);
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	void AttachLanguageImage(uint32_t dwVID, uint8_t bLanguage);
#endif

protected:
	TTextTail * RegisterTextTail(uint32_t dwVirtualID, const char * c_szText, CGraphicObjectInstance * pOwner, float fHeight,
								 const D3DXCOLOR & c_rColor);
	void DeleteTextTail(TTextTail * pTextTail);

	void UpdateTextTail(TTextTail * pTextTail) const;
	void RenderTextTailBox(const TTextTail * pTextTail) const;
	void RenderTextTailName(TTextTail * pTextTail) const;
	void UpdateDistance(const TPixelPosition & c_rCenterPosition, TTextTail * pTextTail) const;

	bool isIn(const TTextTail * pSource, TTextTail * pTarget) const;
#ifdef ENABLE_BATTLE_FIELD
	bool 					IsBattleFieldMap();
#endif

protected:
	TTextTailMap m_CharacterTextTailMap;
	TTextTailMap m_ItemTextTailMap;
	TChatTailMap m_ChatTailMap;

	TTextTailList m_CharacterTextTailList;
	TTextTailList m_ItemTextTailList;

private:
	CDynamicPool<STextTail> m_TextTailPool;
};
