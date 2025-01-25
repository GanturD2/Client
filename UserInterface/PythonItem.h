#pragma once

// Note : 화면상에 떨어진 Item의 Update와 Rendering을 책임지는 한편
//        각 아이템의 데이타와 Icon Image Instance의 매니져 역할까지 주관
//        조금 난잡해진거 같기도 하다 - 2003. 01. 13. [levites]

#include "../EterGrnLib/ThingInstance.h"

class CItemData;

class CPythonItem : public CSingleton<CPythonItem>
{
public:
	enum
	{
		INVALID_ID = 0xffffffff
	};

	enum
	{
		VNUM_MONEY = 1
	};

	enum
	{
		USESOUND_NONE,
		USESOUND_DEFAULT,
		USESOUND_ARMOR,
		USESOUND_WEAPON,
		USESOUND_BOW,
		USESOUND_ACCESSORY,
		USESOUND_POTION,
		USESOUND_PORTAL,
		USESOUND_NUM
	};

	enum
	{
		DROPSOUND_DEFAULT,
		DROPSOUND_ARMOR,
		DROPSOUND_WEAPON,
		DROPSOUND_BOW,
		DROPSOUND_ACCESSORY,
		DROPSOUND_NUM
	};

	typedef struct SGroundItemInstance
	{
		uint32_t dwVirtualNumber;
		D3DXVECTOR3 v3EndPosition;

		D3DXVECTOR3 v3RotationAxis;
		D3DXQUATERNION qEnd;
		D3DXVECTOR3 v3Center;
		CGraphicThingInstance ThingInstance;
		uint32_t dwStartTime;
		uint32_t dwEndTime;

		uint32_t eDropSoundType;

		bool bAnimEnded;
		bool Update();
		void Clear();

		uint32_t dwEffectInstanceIndex;
		std::string stOwnership;

		static void __PlayDropSound(uint32_t eItemType, const D3DXVECTOR3 & c_rv3Pos);
		static std::string ms_astDropSoundFileName[DROPSOUND_NUM];

		SGroundItemInstance() = default;
		virtual ~SGroundItemInstance() = default;
		CLASS_DELETE_COPYMOVE(SGroundItemInstance);
	} TGroundItemInstance;

	typedef std::map<uint32_t, TGroundItemInstance *> TGroundItemInstanceMap;

public:
	CPythonItem();
	~CPythonItem();
	CLASS_DELETE_COPYMOVE(CPythonItem);

	// Initialize
	void Destroy();
	void Create();

	void PlayUseSound(uint32_t dwItemID);
	void PlayDropSound(uint32_t dwItemID) const;
	void PlayUsePotionSound() const;

	void SetUseSoundFileName(uint32_t eItemType, const std::string & c_rstFileName);
	void SetDropSoundFileName(uint32_t eItemType, const std::string & c_rstFileName) const;

	void GetInfo(std::string * pstInfo);

	void DeleteAllItems();

	void Render();
	void Update(const POINT & c_rkPtMouse);

#ifdef ENABLE_EXTENDED_ITEMNAME_ON_GROUND
	void CreateItem(uint32_t dwVirtualID, uint32_t dwVirtualNumber, float x, float y, float z, bool bDrop = true, long alSockets[ITEM_SOCKET_SLOT_MAX_NUM] = {}, TPlayerItemAttribute aAttrs[ITEM_ATTRIBUTE_SLOT_MAX_NUM] = {});
#else
	void CreateItem(uint32_t dwVirtualID, uint32_t dwVirtualNumber, float x, float y, float z, bool bDrop = true);
#endif
	void DeleteItem(uint32_t dwVirtualID);
	void SetOwnership(uint32_t dwVID, const char * c_pszName);
	bool GetOwnership(uint32_t dwVID, const char ** c_pszName);

	BOOL GetGroundItemPosition(uint32_t dwVirtualID, TPixelPosition * pPosition);

	bool GetPickedItemID(uint32_t * pdwPickedItemID) const;

	bool GetCloseItem(const TPixelPosition & c_rPixelPosition, uint32_t * pdwItemID, uint32_t dwDistance = 300);
	bool GetCloseMoney(const TPixelPosition & c_rPixelPosition, uint32_t * pdwItemID, uint32_t dwDistance = 300);

	uint32_t GetVirtualNumberOfGroundItem(uint32_t dwVID);

	void BuildNoGradeNameData(int iType) const;
	uint32_t GetNoGradeNameDataCount() const;
	CItemData * GetNoGradeNameDataPtr(uint32_t dwIndex);

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	bool	CanAddChangeLookItem(const CItemData* item, const CItemData* other_item) const;
	bool	CanAddChangeLookFreeItem(const uint32_t dwVnum) const;
	bool	IsChangeLookClearScrollItem(const uint32_t dwVnum) const;
#endif

protected:
	uint32_t __Pick(const POINT & c_rkPtMouse);

	uint32_t __GetUseSoundType(const CItemData & c_rkItemData) const;
	uint32_t __GetDropSoundType(const CItemData & c_rkItemData) const;

protected:
	TGroundItemInstanceMap m_GroundItemInstanceMap;
	CDynamicPool<TGroundItemInstance> m_GroundItemInstancePool;

	uint32_t m_dwDropItemEffectID;
	uint32_t m_dwPickedItemID;

	int m_nMouseX;
	int m_nMouseY;

	std::string m_astUseSoundFileName[USESOUND_NUM];

	std::vector<CItemData *> m_NoGradeNameItemData;
};