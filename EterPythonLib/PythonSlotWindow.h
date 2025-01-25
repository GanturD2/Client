#pragma once

#include "PythonWindow.h"

namespace UI
{
	enum
	{
		ITEM_WIDTH = 32,
		ITEM_HEIGHT = 32,

		SLOT_NUMBER_NONE = 0xffffffff
	};

	enum ESlotStyle
	{
		SLOT_STYLE_NONE,
		SLOT_STYLE_PICK_UP,
		SLOT_STYLE_SELECT
	};

	enum ESlotState
	{
		SLOT_STATE_LOCK = (1 << 0),
		SLOT_STATE_CANT_USE = (1 << 1),
		SLOT_STATE_DISABLE = (1 << 2),
		SLOT_STATE_ALWAYS_RENDER_COVER = (1 << 3),
#ifdef WJ_ENABLE_TRADABLE_ICON
		SLOT_STATE_CANT_MOUSE_EVENT = (1 << 4),
		SLOT_STATE_UNUSABLE = (1 << 5),
#endif
	};

#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
	enum ESlotDiffuseColorType
	{
		COLOR_TYPE_ORANGE,
		COLOR_TYPE_WHITE,
		COLOR_TYPE_RED,
		COLOR_TYPE_GREEN,
		COLOR_TYPE_YELLOW,
		COLOR_TYPE_SKY,
		COLOR_TYPE_PINK,
		COLOR_TYPE_BLUE,
		COLOR_TYPE_PURPLE,
		COLOR_TYPE_MAX,
	};

	enum ESlotHighLight
	{
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		HILIGHTSLOT_ACCE = 0,
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		HILIGHTSLOT_CHANGE_LOOK = 1,
#endif
#ifdef ENABLE_AURA_SYSTEM
		HILIGHTSLOT_AURA = 2,
#endif
		HILIGHTSLOT_CUBE = 3,

		HILIGHT_MAX,
	};
#endif

	class CSlotWindow : public CWindow
	{
	public:
		static uint32_t Type();

	public:
		class CSlotButton;
		class CCoverButton;
		class CCoolTimeFinishEffect;

		friend class CSlotButton;
		friend class CCoverButton;

		typedef struct SSlot
		{
			uint32_t dwState;
			uint32_t dwSlotNumber;
			uint32_t dwCenterSlotNumber; // NOTE : 사이즈가 큰 아이템의 경우 아이템의 실제 위치 번호
			uint32_t dwItemIndex; // NOTE : 여기서 사용되는 Item이라는 단어는 좁은 개념의 것이 아닌,
			BOOL isItem; //        "슬롯의 내용물"이라는 포괄적인 개념어. 더 좋은 것이 있을까? - [levites]

			// CoolTime
			float fCoolTime;
			float fStartCoolTime;
#ifdef ENABLE_GROWTH_PET_SYSTEM
			bool bIsInverseCoolTime;
#endif

			// Toggle
			BOOL bActive;
#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
			uint8_t byToggleColorType;
#endif

			int ixPosition;
			int iyPosition;

			int ixCellSize;
			int iyCellSize;

			uint8_t byxPlacedItemSize;
			uint8_t byyPlacedItemSize;

			CGraphicImageInstance* pInstance;
			CNumberLine* pNumberLine;

#ifdef SIGN_FOR_PLUS_ITEMS
			CNumberLine* pPlusLine;
			int		ixPlusPosition;
			int		iyPlusPosition;
#endif

			bool bRenderBaseSlotImage;
#ifdef ENABLE_SLOT_COVER_IMAGE_SYSTEM
			BOOL	bSlotCoverImage;
#endif

			CCoverButton* pCoverButton;
			CSlotButton* pSlotButton;
			CImageBox* pSignImage;
			CAniImageBox* pFinishCoolTimeEffect;
		} TSlot;

		using TSlotList = std::list<TSlot>;
		using TSlotListIterator = TSlotList::iterator;
#ifdef ENABLE_OFFICAL_FEATURES
		typedef struct SStoreCoolDown
		{
			float fCoolTime;
			float fElapsedTime;
			bool bActive;
		} TStoreCoolDown;
#endif

	public:
		explicit CSlotWindow(PyObject* ppyObject);
		~CSlotWindow() override;

		virtual void Destroy();

		// Manage Slot
		void SetSlotType(uint32_t dwType);
		void SetSlotStyle(uint32_t dwStyle);

#ifdef SIGN_FOR_PLUS_ITEMS
		void AppendPlusOnSlot(uint32_t dwIndex, int ixPlusPosition, int iyPlusPosition);
#endif

		void AppendSlot(uint32_t dwIndex, int ixPosition, int iyPosition, int ixCellSize, int iyCellSize);
#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_GROWTH_PET_SYSTEM)
		void SetCoverButton(uint32_t dwIndex, const char* c_szUpImageName, const char* c_szOverImageName, const char* c_szDownImageName,
			const char* c_szDisableImageName, BOOL bLeftButtonEnable, BOOL bRightButtonEnable, float fx, float fy);
#else
		void SetCoverButton(uint32_t dwIndex, const char* c_szUpImageName, const char* c_szOverImageName, const char* c_szDownImageName,
			const char* c_szDisableImageName, BOOL bLeftButtonEnable, BOOL bRightButtonEnable);
#endif

		void SetSlotBaseImage(const char* c_szFileName, float fr, float fg, float fb, float fa);
		void AppendSlotButton(const char* c_szUpImageName, const char* c_szOverImageName, const char* c_szDownImageName);
		void AppendRequirementSignImage(const char* c_szImageName);

		void EnableCoverButton(uint32_t dwIndex);
		void DisableCoverButton(uint32_t dwIndex);
		void SetAlwaysRenderCoverButton(uint32_t dwIndex, bool bAlwaysRender = false);

		void ShowSlotBaseImage(uint32_t dwIndex);
		void HideSlotBaseImage(uint32_t dwIndex);
		BOOL IsDisableCoverButton(uint32_t dwIndex);
#ifdef ENABLE_FISH_EVENT
		void DeleteCoverButton(uint32_t dwIndex);
#endif
		BOOL HasSlot(uint32_t dwIndex);

		void ClearAllSlot();
		void ClearSlot(uint32_t dwIndex);

		void SetSlot(uint32_t dwIndex, uint32_t dwVirtualNumber, uint8_t byWidth, uint8_t byHeight, CGraphicImage* pImage, D3DXCOLOR& diffuseColor
#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_GROWTH_PET_SYSTEM)
			, float fx, float fy
#endif
		);

#ifdef ENABLE_FISH_EVENT
		void SetFishSlot(uint32_t dwIndex, uint32_t dwVirtualNumber, uint8_t byWidth, uint8_t byHeight, CGraphicImage* pImage);
#endif

#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
		void SetCardSlot(uint32_t dwIndex, uint32_t dwVirtualNumber, uint8_t byWidth, uint8_t byHeight, const char* c_szFileName, D3DXCOLOR& diffuseColor);
#endif
#ifdef ENABLE_PLUS_ICON_ITEM
		void GetValue(uint8_t dwIndex, char* number);
#endif
		void SetSlotCount(uint32_t dwIndex, uint32_t dwCount);
		void SetSlotCountNew(uint32_t dwIndex, uint32_t dwGrade, uint32_t dwCount);
		void SetSlotCoolTime(uint32_t dwIndex, float fCoolTime, float fElapsedTime = 0.0f);
#ifdef ENABLE_GROWTH_PET_SYSTEM
		void SetSlotCoolTimeColor(uint32_t dwIndex, float fr = 0.0f, float fg = 0.0f, float fb = 0.0f, float fa = 0.5f);
#endif
#ifdef ENABLE_OFFICAL_FEATURES
		void StoreSlotCoolTime(uint32_t dwKey, uint32_t dwSlotIndex, float fCoolTime, float fElapsedTime = .0f);
		void RestoreSlotCoolTime(uint32_t dwKey);
#endif
		void ActivateSlot(uint32_t dwIndex
#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
			, uint8_t byColorType
#endif
		);
		void DeactivateSlot(uint32_t dwIndex);
		void RefreshSlot();

#ifdef ENABLE_SLOT_WINDOW_EX
		float GetSlotCoolTime(uint32_t dwIndex, float* fElapsedTime);
		bool IsActivatedSlot(uint32_t dwIndex);
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
		void SetSlotCoolTimeInverse(uint32_t dwIndex, float fCoolTime, float fElapsedTime);
#endif

		uint32_t GetSlotCount() const;

		void LockSlot(uint32_t dwIndex);
		void UnlockSlot(uint32_t dwIndex);
		BOOL IsLockSlot(uint32_t dwIndex);
		void SetCantUseSlot(uint32_t dwIndex);
		void SetUseSlot(uint32_t dwIndex);
		BOOL IsCantUseSlot(uint32_t dwIndex);
		void EnableSlot(uint32_t dwIndex);
		void DisableSlot(uint32_t dwIndex);
		BOOL IsEnableSlot(uint32_t dwIndex);

#ifdef WJ_ENABLE_TRADABLE_ICON
		void SetCanMouseEventSlot(uint32_t dwIndex);
		void SetCantMouseEventSlot(uint32_t dwIndex);
		void SetUsableSlotOnTopWnd(uint32_t dwIndex);
		void SetUnusableSlotOnTopWnd(uint32_t dwIndex);
#endif

		// Select
		void ClearSelected();
		void SelectSlot(uint32_t dwSelectingIndex);
		BOOL isSelectedSlot(uint32_t dwIndex);
		uint32_t GetSelectedSlotCount() const;
		uint32_t GetSelectedSlotNumber(uint32_t dwIndex);

		// Slot Button
		void ShowSlotButton(uint32_t dwSlotNumber);
		void HideAllSlotButton();
		void OnPressedSlotButton(uint32_t dwType, uint32_t dwSlotNumber, BOOL isLeft = TRUE);

		// Requirement Sign
		void ShowRequirementSign(uint32_t dwSlotNumber);
		void HideRequirementSign(uint32_t dwSlotNumber);

		// ToolTip
		BOOL OnOverInItem(uint32_t dwSlotNumber);
		void OnOverOutItem();

		BOOL OnOverIn(uint32_t dwSlotNumber);
		void OnOverOut();

		// For Usable Item
		void SetUseMode(BOOL bFlag);
		void SetUsableItem(BOOL bFlag);

		// CallBack
		void ReserveDestroyCoolTimeFinishEffect(uint32_t dwSlotIndex);

#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_GROWTH_PET_SYSTEM)
		void SetScale(float fy, float fx);
		float GetScaleX() { return m_pScaleX; };
		float GetScaleY() { return m_pScaleY; };
#endif

#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
		void SetSlotDiffuseColor(uint32_t dwIndex, uint8_t iColorType);
#endif

#ifdef ENABLE_SLOT_COVER_IMAGE_SYSTEM
		void SetSlotCoverImage(uint32_t dwIndex, const char* c_filename);
		void EnableSlotCoverImage(uint32_t dwIndex, BOOL bEnable);
#endif

	protected:
		virtual void __Initialize();
		void __CreateToggleSlotImage();
		void __CreateSlotEnableEffect();
		void __CreateFinishCoolTimeEffect(TSlot* pSlot);
		void __CreateBaseImage(const char* c_szFileName, float fr, float fg, float fb, float fa);

		void __DestroyToggleSlotImage();
		void __DestroySlotEnableEffect();
		static void __DestroyFinishCoolTimeEffect(TSlot* pSlot);
		void __DestroyBaseImage();

		// Event
		void OnUpdate() override;
		void OnRender() override;
		BOOL OnMouseLeftButtonDown() override;
		BOOL OnMouseLeftButtonUp() override;
		BOOL OnMouseRightButtonDown() override;
		BOOL OnMouseLeftButtonDoubleClick() override;
		void OnMouseOverOut() override;
		void OnMouseOver() override;
		void RenderSlotBaseImage();
		void RenderLockedSlot();
		virtual void OnRenderPickingSlot();
		virtual void OnRenderSelectedSlot();

		// Select
		void OnSelectEmptySlot(int iSlotNumber) const;
		void OnSelectItemSlot(int iSlotNumber);
		void OnUnselectEmptySlot(int iSlotNumber) const;
		void OnUnselectItemSlot(int iSlotNumber) const;
		void OnUseSlot();

		// Manage Slot
		BOOL GetSlotPointer(uint32_t dwIndex, TSlot** ppSlot);
		BOOL GetSelectedSlotPointer(TSlot** ppSlot);
		virtual BOOL GetPickedSlotPointer(TSlot** ppSlot);
		static void ClearSlot(TSlot* pSlot);
		virtual void OnRefreshSlot();

		// ETC
		BOOL OnIsType(uint32_t dwType) override;

#ifdef ENABLE_OFFICAL_FEATURES
	public:
		bool GetSlotLocalPosition(uint32_t dwIndex, int* iX, int* iY);
		bool GetSlotGlobalPosition(uint32_t dwIndex, int* iX, int* iY);
#endif

	protected:
		uint32_t m_dwSlotType;
		uint32_t m_dwSlotStyle;
		std::list<uint32_t> m_dwSelectedSlotIndexList;
		TSlotList m_SlotList;
		uint32_t m_dwToolTipSlotNumber;
		uint32_t m_dwOverInSlotNumber;
#ifdef ENABLE_OFFICAL_FEATURES
		std::map<uint32_t, std::map<uint32_t, SStoreCoolDown>> m_CoolDownStore;
#endif

		BOOL m_isUseMode;
		BOOL m_isUsableItem;

#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_GROWTH_PET_SYSTEM)
		BOOL m_isScale;
		float m_pScaleX;
		float m_pScaleY;
#endif

		CGraphicImageInstance* m_pBaseImageInstance;
		CImageBox* m_pToggleSlotImage;
#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
		CAniImageBox* m_apSlotActiveEffect[3];
#else
		CAniImageBox* m_pSlotActiveEffect;
#endif
#ifdef ENABLE_SLOT_COVER_IMAGE_SYSTEM
		CAniImageBox* m_pSlotCoverImage;
#endif
		std::deque<uint32_t> m_ReserveDestroyEffectDeque;
	};
} // namespace UI
