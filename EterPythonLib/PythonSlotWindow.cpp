#include "StdAfx.h"
#include "../EterBase/CRC32.h"
#include "../EterBase/Filename.h"
#include "PythonWindow.h"
#include "PythonSlotWindow.h"

//#define __RENDER_SLOT_AREA__

using namespace UI;

class CSlotWindow::CSlotButton : public CButton
{
public:
	enum ESlotButtonType
	{
		SLOT_BUTTON_TYPE_PLUS,
		SLOT_BUTTON_TYPE_COVER
	};

public:
	CSlotButton(ESlotButtonType dwType, uint32_t dwSlotNumber, CSlotWindow* pParent) : CButton(nullptr)
	{
		m_dwSlotButtonType = dwType;
		m_dwSlotNumber = dwSlotNumber;
		m_pParent = pParent;
	}

	~CSlotButton() override = default;

	BOOL OnMouseLeftButtonDown() override
	{
		if (!IsEnable())
			return TRUE;
		if (CWindowManager::Instance().IsAttaching())
			return TRUE;

		m_isPressed = TRUE;
		Down();

		return TRUE;
	}
	BOOL OnMouseLeftButtonUp() override
	{
		if (!IsEnable())
			return TRUE;
		if (!IsPressed())
			return TRUE;

		if (IsIn())
			SetCurrentVisual(&m_overVisual);
		else
			SetCurrentVisual(&m_upVisual);

		m_pParent->OnPressedSlotButton(m_dwSlotButtonType, m_dwSlotNumber);

		return TRUE;
	}

	void OnMouseOverIn() override
	{
		if (IsEnable())
			SetCurrentVisual(&m_overVisual);
		m_pParent->OnOverInItem(m_dwSlotNumber);
	}
	void OnMouseOverOut() override
	{
		if (IsEnable())
		{
			SetUp();
			SetCurrentVisual(&m_upVisual);
		}
		m_pParent->OnOverOutItem();
	}

protected:
	ESlotButtonType m_dwSlotButtonType;
	uint32_t m_dwSlotNumber;
	CSlotWindow* m_pParent;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

class CSlotWindow::CCoverButton : public CSlotButton
{
public:
	CCoverButton(ESlotButtonType dwType, uint32_t dwSlotNumber, CSlotWindow* pParent) : CSlotButton(dwType, dwSlotNumber, pParent)
	{
		m_bLeftButtonEnable = TRUE;
		m_bRightButtonEnable = TRUE;
	}

	~CCoverButton() override = default;

	void SetLeftButtonEnable(BOOL bEnable) { m_bLeftButtonEnable = bEnable; }
	void SetRightButtonEnable(BOOL bEnable) { m_bRightButtonEnable = bEnable; }

	void OnRender() override {}

	void RenderButton() { CButton::OnRender(); }

	BOOL OnMouseLeftButtonDown() override
	{
		if (!IsEnable())
			return TRUE;
		if (m_bLeftButtonEnable)
			CButton::OnMouseLeftButtonDown();
		else
			m_pParent->OnMouseLeftButtonDown();
		return TRUE;
	}
	BOOL OnMouseLeftButtonUp() override
	{
		if (!IsEnable())
			return TRUE;
		if (m_bLeftButtonEnable)
		{
			CButton::OnMouseLeftButtonUp();
			m_pParent->OnMouseLeftButtonDown();
		}
		else
			m_pParent->OnMouseLeftButtonUp();
		return TRUE;
	}

	BOOL OnMouseRightButtonDown() override
	{
		if (!IsEnable())
			return TRUE;
		if (m_bRightButtonEnable)
			CButton::OnMouseLeftButtonDown();
		return TRUE;
	}
	BOOL OnMouseRightButtonUp() override
	{
		if (!IsEnable())
			return TRUE;
		m_pParent->OnMouseRightButtonDown();
		if (m_bRightButtonEnable)
			CButton::OnMouseLeftButtonUp();
		return TRUE;
	}

protected:
	BOOL m_bLeftButtonEnable;
	BOOL m_bRightButtonEnable;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

class CSlotWindow::CCoolTimeFinishEffect : public CAniImageBox
{
public:
	CCoolTimeFinishEffect(CSlotWindow* pParent, uint32_t dwSlotIndex) : CAniImageBox(nullptr)
	{
		m_pParent = pParent;
		m_dwSlotIndex = dwSlotIndex;
	}

	~CCoolTimeFinishEffect() override = default;

	void OnEndFrame() override { msl::inherit_cast<CSlotWindow*>(m_pParent)->ReserveDestroyCoolTimeFinishEffect(m_dwSlotIndex); }

protected:
	uint32_t m_dwSlotIndex;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// Set & Append

void CSlotWindow::SetSlotType(uint32_t dwType)
{
	m_dwSlotType = dwType;
}

void CSlotWindow::SetSlotStyle(uint32_t dwStyle)
{
	m_dwSlotStyle = dwStyle;
}

void CSlotWindow::AppendSlot(uint32_t dwIndex, int ixPosition, int iyPosition, int ixCellSize, int iyCellSize)
{
	TSlot Slot;
	Slot.pInstance = nullptr;
	Slot.pNumberLine = nullptr;
#ifdef SIGN_FOR_PLUS_ITEMS
	Slot.pPlusLine = nullptr;
#endif
	Slot.pCoverButton = nullptr;
	Slot.pSlotButton = nullptr;
	Slot.pSignImage = nullptr;
	Slot.pFinishCoolTimeEffect = nullptr;

	ClearSlot(&Slot);
	Slot.dwSlotNumber = dwIndex;
	Slot.dwCenterSlotNumber = dwIndex;
	Slot.ixPosition = ixPosition;
	Slot.iyPosition = iyPosition;
	Slot.ixCellSize = ixCellSize;
	Slot.iyCellSize = iyCellSize;
	m_SlotList.emplace_back(Slot);
}

#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_GROWTH_PET_SYSTEM)
void CSlotWindow::SetCoverButton(uint32_t dwIndex, const char* c_szUpImageName, const char* c_szOverImageName,
	const char* c_szDownImageName, const char* c_szDisableImageName, BOOL bLeftButtonEnable,
	BOOL bRightButtonEnable, float fx, float fy)
#else
void CSlotWindow::SetCoverButton(uint32_t dwIndex, const char* c_szUpImageName, const char* c_szOverImageName,
	const char* c_szDownImageName, const char* c_szDisableImageName, BOOL bLeftButtonEnable,
	BOOL bRightButtonEnable)
#endif
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	CCoverButton*& rpCoverButton = pSlot->pCoverButton;

	if (!rpCoverButton)
	{
		rpCoverButton = new CCoverButton(CSlotButton::SLOT_BUTTON_TYPE_COVER, pSlot->dwSlotNumber, this);
		CWindowManager::Instance().SetParent(rpCoverButton, this);
	}

#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_GROWTH_PET_SYSTEM)
	rpCoverButton->SetScale(fx, fy);
#endif
	rpCoverButton->SetLeftButtonEnable(bLeftButtonEnable);
	rpCoverButton->SetRightButtonEnable(bRightButtonEnable);
	rpCoverButton->SetUpVisual(c_szUpImageName);
	rpCoverButton->SetOverVisual(c_szOverImageName);
	rpCoverButton->SetDownVisual(c_szDownImageName);
	rpCoverButton->SetDisableVisual(c_szDisableImageName);
	rpCoverButton->Enable();
	rpCoverButton->Show();

	// NOTE : Cover 버튼이 Plus 버튼을 가려버려서 임시 코드를..
	if (pSlot->pSlotButton)
		SetTop(pSlot->pSlotButton);
}


#ifdef ENABLE_OFFICAL_FEATURES
bool CSlotWindow::GetSlotLocalPosition(uint32_t dwIndex, int* iX, int* iY)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return false;

	*iX = pSlot->ixPosition;
	*iY = pSlot->iyPosition;
	return true;
}

bool CSlotWindow::GetSlotGlobalPosition(uint32_t dwIndex, int* iX, int* iY)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return false;

	*iX = m_rect.left + pSlot->ixPosition;
	*iY = m_rect.top + pSlot->iyPosition;
	return true;
}
#endif

void CSlotWindow::EnableCoverButton(uint32_t dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (!pSlot->pCoverButton)
		return;

	pSlot->pCoverButton->Enable();
}

void CSlotWindow::DisableCoverButton(uint32_t dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (!pSlot->pCoverButton)
		return;

	pSlot->pCoverButton->Disable();
}

#ifdef ENABLE_FISH_EVENT
void CSlotWindow::DeleteCoverButton(uint32_t dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (!pSlot->pCoverButton)
		return;

	CWindowManager::Instance().DestroyWindow(pSlot->pCoverButton);
	pSlot->pCoverButton = NULL;
}
#endif

void CSlotWindow::SetAlwaysRenderCoverButton(uint32_t dwIndex, bool bAlwaysRender)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (bAlwaysRender)
		SET_BIT(pSlot->dwState, SLOT_STATE_ALWAYS_RENDER_COVER);
	else
		REMOVE_BIT(pSlot->dwState, SLOT_STATE_ALWAYS_RENDER_COVER);
}

void CSlotWindow::ShowSlotBaseImage(uint32_t dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->bRenderBaseSlotImage = true;
}

void CSlotWindow::HideSlotBaseImage(uint32_t dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->bRenderBaseSlotImage = false;
}

BOOL CSlotWindow::IsDisableCoverButton(uint32_t dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return FALSE;

	if (!pSlot->pCoverButton)
		return FALSE;

	return pSlot->pCoverButton->IsDisable();
}

void CSlotWindow::SetSlotBaseImage(const char* c_szFileName, float fr, float fg, float fb, float fa)
{
	__CreateBaseImage(c_szFileName, fr, fg, fb, fa);
}

void CSlotWindow::AppendSlotButton(const char* c_szUpImageName, const char* c_szOverImageName, const char* c_szDownImageName)
{
	for (auto& rSlot : m_SlotList)
	{
		CSlotButton*& rpSlotButton = rSlot.pSlotButton;

		if (!rpSlotButton)
		{
			rpSlotButton = new CSlotButton(CSlotButton::SLOT_BUTTON_TYPE_PLUS, rSlot.dwSlotNumber, this);
			rpSlotButton->AddFlag(FLAG_FLOAT);
			CWindowManager::Instance().SetParent(rpSlotButton, this);
		}

		rpSlotButton->SetUpVisual(c_szUpImageName);
		rpSlotButton->SetOverVisual(c_szOverImageName);
		rpSlotButton->SetDownVisual(c_szDownImageName);
		rpSlotButton->SetPosition(rSlot.ixPosition + 1, rSlot.iyPosition + 19);
		rpSlotButton->Hide();
	}
}

void CSlotWindow::AppendRequirementSignImage(const char* c_szImageName)
{
	for (auto& rSlot : m_SlotList)
	{
		CImageBox*& rpSignImage = rSlot.pSignImage;

		if (!rpSignImage)
		{
			rpSignImage = new CImageBox(nullptr);
			CWindowManager::Instance().SetParent(rpSignImage, this);
		}

		rpSignImage->LoadImage(c_szImageName);
		rpSignImage->Hide();
	}
}

BOOL CSlotWindow::HasSlot(uint32_t dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return FALSE;

	return TRUE;
}

void CSlotWindow::SetSlot(uint32_t dwIndex, uint32_t dwVirtualNumber, uint8_t byWidth, uint8_t byHeight, CGraphicImage* pImage, D3DXCOLOR& diffuseColor
#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_GROWTH_PET_SYSTEM)
	, float fx, float fy
#endif
)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (pSlot->isItem)
	{
		if (pSlot->dwItemIndex == dwVirtualNumber)
		{
			pSlot->dwState = 0;
			pSlot->isItem = TRUE;
			if (pImage && pSlot->pInstance)
				pSlot->pInstance->SetImagePointer(pImage);
			return;
		}
	}

	ClearSlot(pSlot);
	pSlot->dwState = 0;
	pSlot->isItem = TRUE;
	pSlot->dwItemIndex = dwVirtualNumber;

	if (pImage)
	{
		assert(nullptr == pSlot->pInstance);
		pSlot->pInstance = CGraphicImageInstance::New();
		pSlot->pInstance->SetDiffuseColor(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a);
		pSlot->pInstance->SetImagePointer(pImage);
	}

#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_GROWTH_PET_SYSTEM)
	pSlot->pInstance->SetScale(fx, fy);
#endif
	pSlot->byxPlacedItemSize = byWidth;
	pSlot->byyPlacedItemSize = byHeight;

	if (pSlot->pCoverButton)
		pSlot->pCoverButton->Show();
}

#ifdef ENABLE_FISH_EVENT
void CSlotWindow::SetFishSlot(uint32_t dwIndex, uint32_t dwVirtualNumber, uint8_t byWidth, uint8_t byHeight, CGraphicImage* pImage)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (pSlot->isItem)
	{
		if (pSlot->dwItemIndex == dwVirtualNumber)
		{
			pSlot->dwState = 0;
			pSlot->isItem = TRUE;
			if (pImage && pSlot->pInstance)
				pSlot->pInstance->SetImagePointer(pImage);
			return;
		}
	}

	ClearSlot(pSlot);
	pSlot->dwState = 0;
	pSlot->isItem = TRUE;
	pSlot->dwItemIndex = dwVirtualNumber;

	if (pImage)
	{
		assert(nullptr == pSlot->pInstance);
		pSlot->pInstance = CGraphicImageInstance::New();
		pSlot->pInstance->SetImagePointer(pImage);
	}

	pSlot->byxPlacedItemSize = byWidth;
	pSlot->byyPlacedItemSize = byHeight;

	if (pSlot->pCoverButton)
		pSlot->pCoverButton->Show();
}
#endif

#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
void CSlotWindow::SetCardSlot(uint32_t dwIndex, uint32_t dwVirtualNumber, uint8_t byWidth, uint8_t byHeight, const char* c_szFileName, D3DXCOLOR& diffuseColor)
{
	CGraphicImage* pImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(c_szFileName);
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (pSlot->isItem)
		if (pSlot->dwItemIndex == dwVirtualNumber)
		{
			pSlot->dwState = 0;
			pSlot->isItem = TRUE;
			if (pImage && pSlot->pInstance)
			{
				pSlot->pInstance->SetImagePointer(pImage);
			}
			return;
		}

	ClearSlot(pSlot);
	pSlot->dwState = 0;
	pSlot->isItem = TRUE;
	pSlot->dwItemIndex = dwVirtualNumber;

	if (pImage)
	{
		assert(NULL == pSlot->pInstance);
		pSlot->pInstance = CGraphicImageInstance::New();
		pSlot->pInstance->SetImagePointer(pImage);
		pSlot->pInstance->SetDiffuseColor(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a);
	}

	pSlot->byxPlacedItemSize = byWidth;
	pSlot->byyPlacedItemSize = byHeight;

	if (pSlot->pCoverButton)
	{
		pSlot->pCoverButton->Show();
	}
}
#endif

#ifdef ENABLE_PLUS_ICON_ITEM
void CSlotWindow::GetValue(uint8_t dwIndex, char* number)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	char szCount[16 + 1];
	_snprintf(szCount, sizeof(szCount), "+ %s", number);

	if (!pSlot->pNumberLine)
	{
		CNumberLine* pNumberLine = new UI::CNumberLine(this);
		pNumberLine->SetHorizontalAlign(CNumberLine::HORIZONTAL_ALIGN_RIGHT);
		pNumberLine->Show();
		pSlot->pNumberLine = pNumberLine;
	}

	pSlot->pNumberLine->SetNumber(szCount);
}
#endif

#ifdef SIGN_FOR_PLUS_ITEMS
void CSlotWindow::AppendPlusOnSlot(uint32_t dwIndex, int ixPlusPosition, int iyPlusPosition)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (ixPlusPosition == 0 && iyPlusPosition == 0)
	{
		if (pSlot->pPlusLine)
		{
			delete pSlot->pPlusLine;
			pSlot->pPlusLine = nullptr;
		}
	}
	else
	{
		if (!pSlot->pPlusLine)
		{
			CNumberLine* pPlusLine = new UI::CNumberLine(this);
			pPlusLine->SetPath("d:/ymir work/ui/public/number2/");
			pPlusLine->SetHorizontalAlign(CNumberLine::HORIZONTAL_ALIGN_RIGHT);
			pPlusLine->Show();
			pSlot->pPlusLine = pPlusLine;
		}

		pSlot->ixPlusPosition = ixPlusPosition;
		pSlot->iyPlusPosition = iyPlusPosition;

		pSlot->pPlusLine->SetNumber("+");
	}
}
#endif

void CSlotWindow::SetSlotCount(uint32_t dwIndex, uint32_t dwCount)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (dwCount <= 0)
	{
		if (pSlot->pNumberLine)
		{
			delete pSlot->pNumberLine;
			pSlot->pNumberLine = nullptr;
		}
	}
	else
	{
		char szCount[16 + 1];
		_snprintf(szCount, sizeof(szCount), "%d", dwCount);

		if (!pSlot->pNumberLine)
		{
			auto* pNumberLine = new CNumberLine(this);
			pNumberLine->SetHorizontalAlign(HORIZONTAL_ALIGN_RIGHT);
			pNumberLine->Show();
			pSlot->pNumberLine = pNumberLine;
		}

		pSlot->pNumberLine->SetNumber(szCount);
	}
}

void CSlotWindow::SetSlotCountNew(uint32_t dwIndex, uint32_t dwGrade, uint32_t dwCount)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (dwCount <= 0)
	{
		if (pSlot->pNumberLine)
		{
			delete pSlot->pNumberLine;
			pSlot->pNumberLine = nullptr;
		}
	}
	else
	{
		char szCount[16 + 1];

		switch (dwGrade)
		{
		case 0:
			_snprintf(szCount, sizeof(szCount), "%d", dwCount);
			break;
		case 1:
			_snprintf(szCount, sizeof(szCount), "m%d", dwCount);
			break;
		case 2:
			_snprintf(szCount, sizeof(szCount), "g%d", dwCount);
			break;
		case 3:
			_snprintf(szCount, sizeof(szCount), "p");
			break;
		}

		if (!pSlot->pNumberLine)
		{
			auto* pNumberLine = new CNumberLine(this);
			pNumberLine->SetHorizontalAlign(HORIZONTAL_ALIGN_RIGHT);
			pNumberLine->Show();
			pSlot->pNumberLine = pNumberLine;
		}

		pSlot->pNumberLine->SetNumber(szCount);
	}
}

void CSlotWindow::SetSlotCoolTime(uint32_t dwIndex, float fCoolTime, float fElapsedTime)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->fCoolTime = fCoolTime;
	pSlot->fStartCoolTime = CTimer::Instance().GetCurrentSecond() - fElapsedTime;
}

#ifdef ENABLE_GROWTH_PET_SYSTEM
void CSlotWindow::SetSlotCoolTimeColor(uint32_t dwIndex, float fr, float fg, float fb, float fa)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->pInstance->SetDiffuseColor(fr, fg, fb, fa);
}
#endif

#ifdef ENABLE_OFFICAL_FEATURES
void CSlotWindow::StoreSlotCoolTime(uint32_t dwKey, uint32_t dwSlotIndex, float fCoolTime, float fElapsedTime)
{
	std::map<uint32_t, SStoreCoolDown>::iterator it = m_CoolDownStore[dwKey].find(dwSlotIndex);
	if (it != m_CoolDownStore[dwKey].end())
	{
		it->second.fCoolTime = fCoolTime;
		it->second.fElapsedTime = CTimer::Instance().GetCurrentSecond() - fElapsedTime;
		it->second.bActive = false;
	}
	else
	{
		SStoreCoolDown m_storeCoolDown;
		m_storeCoolDown.fCoolTime = fCoolTime;
		m_storeCoolDown.fElapsedTime = CTimer::Instance().GetCurrentSecond() - fElapsedTime;
		m_storeCoolDown.bActive = false;
		m_CoolDownStore[dwKey].emplace(std::map<uint32_t, SStoreCoolDown>::value_type(dwSlotIndex, m_storeCoolDown));
	}
}

void CSlotWindow::RestoreSlotCoolTime(uint32_t dwKey)
{
	for (std::map<uint32_t, SStoreCoolDown>::iterator it = m_CoolDownStore[dwKey].begin(); it != m_CoolDownStore[dwKey].end(); it++)
	{
		TSlot* pSlot;
		if (!GetSlotPointer(it->first, &pSlot))
			return;

		pSlot->fCoolTime = it->second.fCoolTime;
		pSlot->fStartCoolTime = it->second.fElapsedTime;
		if (it->second.bActive)
		{
			ActivateSlot(it->first
#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
				, COLOR_TYPE_WHITE
#endif
			);
		}
		else
			DeactivateSlot(it->first);
	}
}
#endif

void CSlotWindow::ActivateSlot(uint32_t dwIndex
#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
	, uint8_t byColorType
#endif
)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
	pSlot->byToggleColorType = byColorType;
#endif
	pSlot->bActive = TRUE;

#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
	if (!m_apSlotActiveEffect[0] || !m_apSlotActiveEffect[1] || !m_apSlotActiveEffect[2])
#else
	if (!m_pSlotActiveEffect)
#endif
		__CreateSlotEnableEffect();

#ifdef ENABLE_OFFICAL_FEATURES
	std::map<uint32_t, SStoreCoolDown>::iterator it = m_CoolDownStore[1].find(dwIndex);
	if (it != m_CoolDownStore[1].end())
		it->second.bActive = true;
#endif
}

void CSlotWindow::DeactivateSlot(uint32_t dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->bActive = FALSE;

#ifdef ENABLE_OFFICAL_FEATURES
	std::map<uint32_t, SStoreCoolDown>::iterator it = m_CoolDownStore[1].find(dwIndex);
	if (it != m_CoolDownStore[1].end())
		it->second.bActive = false;
#endif
}

#ifdef ENABLE_SLOT_WINDOW_EX
float CSlotWindow::GetSlotCoolTime(uint32_t dwIndex, float* fElapsedTime)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return 0.0;

	*fElapsedTime = CTimer::Instance().GetCurrentSecond() - pSlot->fStartCoolTime;
	return pSlot->fCoolTime;
}

bool CSlotWindow::IsActivatedSlot(uint32_t dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return false;
	return pSlot->bActive;
}
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
void CSlotWindow::SetSlotCoolTimeInverse(uint32_t dwIndex, float fCoolTime, float fElapsedTime)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->fCoolTime = fCoolTime;
	pSlot->fStartCoolTime = CTimer::Instance().GetCurrentSecond() - fElapsedTime;
	pSlot->bIsInverseCoolTime = true;
}
#endif

void CSlotWindow::ClearSlot(uint32_t dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	ClearSlot(pSlot);
}

void CSlotWindow::ClearSlot(TSlot* pSlot)
{
	pSlot->bActive = FALSE;
#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
	pSlot->byToggleColorType = COLOR_TYPE_WHITE;
#endif
	pSlot->byxPlacedItemSize = 1;
	pSlot->byyPlacedItemSize = 1;

	pSlot->isItem = FALSE;
	pSlot->dwState = 0;
	pSlot->fCoolTime = 0.0f;
	pSlot->fStartCoolTime = 0.0f;
	pSlot->dwCenterSlotNumber = 0xffffffff;

	pSlot->dwItemIndex = 0;
	pSlot->bRenderBaseSlotImage = true;

#ifdef ENABLE_GROWTH_PET_SYSTEM
	pSlot->bIsInverseCoolTime = false;
#endif

#ifdef ENABLE_SLOT_COVER_IMAGE_SYSTEM
	pSlot->bSlotCoverImage = FALSE;
#endif

	if (pSlot->pInstance)
	{
		CGraphicImageInstance::Delete(pSlot->pInstance);
		pSlot->pInstance = nullptr;
	}
	if (pSlot->pCoverButton)
		pSlot->pCoverButton->Hide();
	if (pSlot->pSlotButton)
		pSlot->pSlotButton->Hide();
	if (pSlot->pSignImage)
		pSlot->pSignImage->Hide();
	if (pSlot->pFinishCoolTimeEffect)
		pSlot->pFinishCoolTimeEffect->Hide();
}

void CSlotWindow::ClearAllSlot()
{
	Destroy();
}

void CSlotWindow::RefreshSlot()
{
	OnRefreshSlot();

	// NOTE : Refresh 될때 ToolTip 도 갱신 합니다 - [levites]
	if (IsRendering())
	{
		TSlot* pSlot;
		if (GetPickedSlotPointer(&pSlot))
		{
			OnOverOutItem();
			OnOverInItem(pSlot->dwSlotNumber);
		}
	}
}

void CSlotWindow::OnRefreshSlot() {}

uint32_t CSlotWindow::GetSlotCount() const
{
	return m_SlotList.size();
}

void CSlotWindow::LockSlot(uint32_t dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	SET_BIT(pSlot->dwState, SLOT_STATE_LOCK);
}
void CSlotWindow::UnlockSlot(uint32_t dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	REMOVE_BIT(pSlot->dwState, SLOT_STATE_DISABLE);
}
void CSlotWindow::SetCantUseSlot(uint32_t dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->dwState |= SLOT_STATE_CANT_USE;
}
void CSlotWindow::SetUseSlot(uint32_t dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->dwState ^= SLOT_STATE_CANT_USE;
}

void CSlotWindow::EnableSlot(uint32_t dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	REMOVE_BIT(pSlot->dwState, SLOT_STATE_DISABLE);
	//pSlot->dwState |= SLOT_STATE_DISABLE;
}
void CSlotWindow::DisableSlot(uint32_t dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;
	SET_BIT(pSlot->dwState, SLOT_STATE_DISABLE);
	//pSlot->dwState ^= SLOT_STATE_DISABLE;
}


#ifdef WJ_ENABLE_TRADABLE_ICON
void CSlotWindow::SetCanMouseEventSlot(uint32_t dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	REMOVE_BIT(pSlot->dwState, SLOT_STATE_CANT_MOUSE_EVENT);
	//pSlot->dwState |= SLOT_STATE_CANT_MOUSE_EVENT;
}

void CSlotWindow::SetCantMouseEventSlot(uint32_t dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	SET_BIT(pSlot->dwState, SLOT_STATE_CANT_MOUSE_EVENT);
	//pSlot->dwState ^= SLOT_STATE_CANT_MOUSE_EVENT;
}

void CSlotWindow::SetUsableSlotOnTopWnd(uint32_t dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	REMOVE_BIT(pSlot->dwState, SLOT_STATE_UNUSABLE);
	//pSlot->dwState |= SLOT_STATE_UNUSABLE;
}

void CSlotWindow::SetUnusableSlotOnTopWnd(uint32_t dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	SET_BIT(pSlot->dwState, SLOT_STATE_UNUSABLE);
	//pSlot->dwState ^= SLOT_STATE_UNUSABLE;
}
#endif

// Select

void CSlotWindow::SelectSlot(uint32_t dwSelectingIndex)
{
	auto itor = m_dwSelectedSlotIndexList.begin();
	for (; itor != m_dwSelectedSlotIndexList.end();)
	{
		if (dwSelectingIndex == *itor)
		{
			m_dwSelectedSlotIndexList.erase(itor);
			return;
		}
		++itor;
	}

	TSlot* pSlot;
	if (GetSlotPointer(dwSelectingIndex, &pSlot))
	{
		if (!pSlot->isItem)
			return;

		m_dwSelectedSlotIndexList.emplace_back(dwSelectingIndex);
	}
}

BOOL CSlotWindow::isSelectedSlot(uint32_t dwIndex)
{
	auto itor = m_dwSelectedSlotIndexList.begin();
	for (; itor != m_dwSelectedSlotIndexList.end(); ++itor)
	{
		if (dwIndex == *itor)
			return TRUE;
	}

	return FALSE;
}

void CSlotWindow::ClearSelected()
{
	m_dwSelectedSlotIndexList.clear();
}

uint32_t CSlotWindow::GetSelectedSlotCount() const
{
	return m_dwSelectedSlotIndexList.size();
}

uint32_t CSlotWindow::GetSelectedSlotNumber(uint32_t dwIndex)
{
	if (dwIndex >= m_dwSelectedSlotIndexList.size())
		return uint32_t(-1);

	uint32_t dwCount = 0;
	auto itor = m_dwSelectedSlotIndexList.begin();
	for (; itor != m_dwSelectedSlotIndexList.end(); ++itor)
	{
		if (dwIndex == dwCount)
			break;

		++dwCount;
	}

	return *itor;
}

void CSlotWindow::ShowSlotButton(uint32_t dwSlotNumber)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwSlotNumber, &pSlot))
		return;

	if (pSlot->pSlotButton)
		pSlot->pSlotButton->Show();
}

void CSlotWindow::HideAllSlotButton()
{
	for (auto& rSlot : m_SlotList)
	{
		if (rSlot.pSlotButton)
			rSlot.pSlotButton->Hide();
	}
}

void CSlotWindow::ShowRequirementSign(uint32_t dwSlotNumber)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwSlotNumber, &pSlot))
		return;

	if (!pSlot->pSignImage)
		return;

	pSlot->pSignImage->Show();
}

void CSlotWindow::HideRequirementSign(uint32_t dwSlotNumber)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwSlotNumber, &pSlot))
		return;

	if (!pSlot->pSignImage)
		return;

	pSlot->pSignImage->Hide();
}

// Event

BOOL CSlotWindow::OnMouseLeftButtonDown()
{
	TSlot* pSlot;
	if (!GetPickedSlotPointer(&pSlot))
	{
		CWindowManager::Instance().DeattachIcon();
		return TRUE;
	}

#ifdef WJ_ENABLE_TRADABLE_ICON
	if (pSlot->dwState & SLOT_STATE_CANT_MOUSE_EVENT)
		return TRUE;
#endif

	if (pSlot->isItem && !(pSlot->dwState & SLOT_STATE_LOCK))
		OnSelectItemSlot(pSlot->dwSlotNumber);
	else
		OnSelectEmptySlot(pSlot->dwSlotNumber);

	return TRUE;
}

BOOL CSlotWindow::OnMouseLeftButtonUp()
{
	if (CWindowManager::Instance().IsAttaching())
	{
		if (CWindowManager::Instance().IsDragging())
		{
			if (IsIn())
			{
				CWindow* pWin = CWindowManager::Instance().GetPointWindow();

				if (pWin)
				{
					if (pWin->IsType(Type()))
					{
						auto* pSlotWin = msl::inherit_cast<CSlotWindow*>(pWin);

						TSlot* pSlot;
						if (!pSlotWin->GetPickedSlotPointer(&pSlot))
						{
							CWindowManager::Instance().DeattachIcon();
							return TRUE;
						}

#ifdef WJ_ENABLE_TRADABLE_ICON
						if (pSlot->dwState & SLOT_STATE_CANT_MOUSE_EVENT)
							return TRUE;
#endif

						if (pSlot->isItem)
							pSlotWin->OnSelectItemSlot(pSlot->dwSlotNumber);
						else
							pSlotWin->OnSelectEmptySlot(pSlot->dwSlotNumber);

						return TRUE;
					}
				}

				return FALSE;
			}
		}
	}

	return FALSE;
}

BOOL CSlotWindow::OnMouseRightButtonDown()
{
	TSlot* pSlot;
	if (!GetPickedSlotPointer(&pSlot))
		return TRUE;

#ifdef WJ_ENABLE_TRADABLE_ICON
	if (pSlot->dwState & SLOT_STATE_CANT_MOUSE_EVENT)
		return TRUE;
#endif

	if (pSlot->isItem)
		OnUnselectItemSlot(pSlot->dwSlotNumber);
	else
		OnUnselectEmptySlot(pSlot->dwSlotNumber);

	return TRUE;
}

BOOL CSlotWindow::OnMouseLeftButtonDoubleClick()
{
#ifdef WJ_ENABLE_TRADABLE_ICON
	TSlot* pSlot;
	if ((GetPickedSlotPointer(&pSlot)) && (pSlot->dwState & SLOT_STATE_CANT_MOUSE_EVENT))
		return TRUE;
#endif
	OnUseSlot();

	return TRUE;
}

void CSlotWindow::OnMouseOverOut()
{
	OnOverOutItem();
}

void CSlotWindow::OnMouseOver()
{
	// FIXME : 윈도우를 드래깅 하는 도중에 SetTop이 되어버리면 Capture가 풀어져 버린다. 그것의 방지 코드.
	//         좀 더 근본적인 해결책을 찾아야 할 듯 - [levites]
	//	if (UI::CWindowManager::Instance().IsCapture())
	//	if (!UI::CWindowManager::Instance().IsAttaching())
	//		return;

	CWindow* pPointWindow = CWindowManager::Instance().GetPointWindow();
	if (this == pPointWindow)
	{
		TSlot* pSlot;
		if (GetPickedSlotPointer(&pSlot))
		{
			if (OnOverInItem(pSlot->dwSlotNumber))
				return;
		}
	}

	OnOverOutItem();
}

void CSlotWindow::OnSelectEmptySlot(int iSlotNumber) const
{
	PyCallClassMemberFunc(m_poHandler, "OnSelectEmptySlot", Py_BuildValue("(i)", iSlotNumber));
}
void CSlotWindow::OnSelectItemSlot(int iSlotNumber)
{
	//	OnOverOutItem();
	PyCallClassMemberFunc(m_poHandler, "OnSelectItemSlot", Py_BuildValue("(i)", iSlotNumber));

	if (CWindowManager::Instance().IsAttaching())
		OnOverOutItem();
}
void CSlotWindow::OnUnselectEmptySlot(int iSlotNumber) const
{
	PyCallClassMemberFunc(m_poHandler, "OnUnselectEmptySlot", Py_BuildValue("(i)", iSlotNumber));
}
void CSlotWindow::OnUnselectItemSlot(int iSlotNumber) const
{
	PyCallClassMemberFunc(m_poHandler, "OnUnselectItemSlot", Py_BuildValue("(i)", iSlotNumber));
}
void CSlotWindow::OnUseSlot()
{
	TSlot* pSlot;
	if (GetPickedSlotPointer(&pSlot))
	{
		if (pSlot->isItem)
			PyCallClassMemberFunc(m_poHandler, "OnUseSlot", Py_BuildValue("(i)", pSlot->dwSlotNumber));
	}
}

BOOL CSlotWindow::OnOverInItem(uint32_t dwSlotNumber)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwSlotNumber, &pSlot))
		return FALSE;

	if (!pSlot->isItem)
		return FALSE;

	if (pSlot->dwSlotNumber == m_dwToolTipSlotNumber)
		return TRUE;

	m_dwToolTipSlotNumber = dwSlotNumber;
	PyCallClassMemberFunc(m_poHandler, "OnOverInItem", Py_BuildValue("(i)", dwSlotNumber));

	return TRUE;
}

void CSlotWindow::OnOverOutItem()
{
	if (SLOT_NUMBER_NONE == m_dwToolTipSlotNumber)
		return;

	m_dwToolTipSlotNumber = SLOT_NUMBER_NONE;
	PyCallClassMemberFunc(m_poHandler, "OnOverOutItem", Py_BuildValue("()"));
}


BOOL CSlotWindow::OnOverIn(uint32_t dwSlotNumber)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwSlotNumber, &pSlot))
		return FALSE;

	if (pSlot->isItem)
		return FALSE;

	if (pSlot->dwSlotNumber == m_dwOverInSlotNumber)
		return TRUE;

	m_dwOverInSlotNumber = dwSlotNumber;
	PyCallClassMemberFunc(m_poHandler, "OnOverIn", Py_BuildValue("(i)", dwSlotNumber));

	return TRUE;
}

void CSlotWindow::OnOverOut()
{
	if (SLOT_NUMBER_NONE == m_dwOverInSlotNumber)
		return;

	m_dwOverInSlotNumber = SLOT_NUMBER_NONE;
	PyCallClassMemberFunc(m_poHandler, "OnOverOut", Py_BuildValue("()"));
}

void CSlotWindow::OnPressedSlotButton(uint32_t dwType, uint32_t dwSlotNumber, BOOL isLeft)
{
	if (CSlotButton::SLOT_BUTTON_TYPE_PLUS == dwType)
		PyCallClassMemberFunc(m_poHandler, "OnPressedSlotButton", Py_BuildValue("(i)", dwSlotNumber));
	else if (CSlotButton::SLOT_BUTTON_TYPE_COVER == dwType)
	{
		if (isLeft)
			OnMouseLeftButtonDown();
	}
}

void CSlotWindow::OnUpdate()
{
	for (auto& dwSlotIndex : m_ReserveDestroyEffectDeque)
	{
		TSlot* pSlot;
		if (!GetSlotPointer(dwSlotIndex, &pSlot))
			continue;

		__DestroyFinishCoolTimeEffect(pSlot);
	}
	m_ReserveDestroyEffectDeque.clear();


#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
	for (int i = 0; i < 3; ++i)
	{
		if (m_apSlotActiveEffect[i])
			m_apSlotActiveEffect[i]->Update();
	}
#else
	if (m_pSlotActiveEffect)
		m_pSlotActiveEffect->Update();
#endif

#ifdef ENABLE_SLOT_COVER_IMAGE_SYSTEM
	if (m_pSlotCoverImage)
		m_pSlotCoverImage->Update();
#endif
}

void CSlotWindow::OnRender()
{
	RenderSlotBaseImage();

	switch (m_dwSlotStyle)
	{
	case SLOT_STYLE_PICK_UP:
		OnRenderPickingSlot();
		break;
	case SLOT_STYLE_SELECT:
		OnRenderSelectedSlot();
		break;
	}

	std::for_each(m_pChildList.begin(), m_pChildList.end(), std::mem_fn(&CWindow::OnRender));

	//
	// 모든 슬롯 상자 그리기
	//////////////////////////////////////////////////////////////////////////
#ifdef __RENDER_SLOT_AREA__
	CPythonGraphic::Instance().SetDiffuseColor(0.5f, 0.5f, 0.5f);
	for (itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
	{
		TSlot& rSlot = *itor;
		CPythonGraphic::Instance().RenderBox2d(m_rect.left + rSlot.ixPosition, m_rect.top + rSlot.iyPosition,
			m_rect.left + rSlot.ixPosition + rSlot.ixCellSize,
			m_rect.top + rSlot.iyPosition + rSlot.iyCellSize);
	}
	CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.0f, 0.0f, 1.0f);
	CPythonGraphic::Instance().RenderBox2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
#endif
	//////////////////////////////////////////////////////////////////////////

	for (auto& rSlot : m_SlotList)
	{
		if (rSlot.pSlotButton)
			rSlot.pSlotButton->SetPosition(rSlot.ixPosition + 1, rSlot.iyPosition + 19);
		if (rSlot.pSignImage)
			rSlot.pSignImage->SetPosition(rSlot.ixPosition - 7, rSlot.iyPosition + 10);

		if (rSlot.pInstance)
		{
			rSlot.pInstance->SetPosition(m_rect.left + rSlot.ixPosition, m_rect.top + rSlot.iyPosition);
			rSlot.pInstance->Render();
		}

#ifdef ENABLE_SLOT_COVER_IMAGE_SYSTEM
		if (rSlot.bSlotCoverImage)
		{
			if (m_pSlotCoverImage)
			{
				int ix = m_rect.left + rSlot.ixPosition;
				int iy = m_rect.top + rSlot.iyPosition;

				m_pSlotCoverImage->SetPosition(ix, iy);
				m_pSlotCoverImage->Render();
			}
		}
#endif

		if (!rSlot.isItem)
		{
			if (IS_SET(rSlot.dwState, SLOT_STATE_ALWAYS_RENDER_COVER))
			{
				rSlot.pCoverButton->Show();
				rSlot.pCoverButton->SetPosition(rSlot.ixPosition, rSlot.iyPosition);
				rSlot.pCoverButton->RenderButton();
			}

			continue;
		}

		if (IS_SET(rSlot.dwState, SLOT_STATE_DISABLE))
		{
			CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.0f, 0.0f, 0.3f);
			CPythonGraphic::Instance().RenderBar2d(m_rect.left + rSlot.ixPosition, m_rect.top + rSlot.iyPosition,
				m_rect.left + rSlot.ixPosition + rSlot.ixCellSize,
				m_rect.top + rSlot.iyPosition + rSlot.iyCellSize);
		}

#ifdef WJ_ENABLE_TRADABLE_ICON
		if (IS_SET(rSlot.dwState, SLOT_STATE_CANT_MOUSE_EVENT))
		{
			CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.0f, 0.0f, 0.3f);
			CPythonGraphic::Instance().RenderBar2d(m_rect.left + rSlot.ixPosition,
				m_rect.top + rSlot.iyPosition,
				m_rect.left + rSlot.ixPosition + rSlot.byxPlacedItemSize * ITEM_WIDTH,
				m_rect.top + rSlot.iyPosition + rSlot.byyPlacedItemSize * ITEM_HEIGHT);
		}

		if (IS_SET(rSlot.dwState, SLOT_STATE_UNUSABLE))
		{
			CPythonGraphic::Instance().SetDiffuseColor(1.0f, 1.0f, 1.0f, 0.3f);
			CPythonGraphic::Instance().RenderBar2d(m_rect.left + rSlot.ixPosition,
				m_rect.top + rSlot.iyPosition,
				m_rect.left + rSlot.ixPosition + rSlot.byxPlacedItemSize * ITEM_WIDTH,
				m_rect.top + rSlot.iyPosition + rSlot.byyPlacedItemSize * ITEM_HEIGHT);
		}
#endif

		if (rSlot.fCoolTime != 0.0f)
		{
			float fcurTime = CTimer::Instance().GetCurrentSecond();
			float fPercentage = (fcurTime - rSlot.fStartCoolTime) / rSlot.fCoolTime;

#ifdef ENABLE_GROWTH_PET_SYSTEM
			int iCellY = rSlot.iyCellSize;
			int iCellX = rSlot.ixCellSize;

			iCellX = std::min(iCellX, iCellY);

			float fRadius = iCellX * 0.5f;
			float fxCenter = m_rect.left + rSlot.ixPosition + iCellX * 0.5f;
			float fyCenter = m_rect.top + rSlot.iyPosition + iCellY * 0.5f;

			if (!rSlot.bIsInverseCoolTime)
				CPythonGraphic::Instance().RenderCoolTimeBox(m_rect.left + rSlot.ixPosition + (rSlot.ixCellSize / 2), m_rect.top + rSlot.iyPosition + (rSlot.iyCellSize / 2),
					(rSlot.ixCellSize / 2), fPercentage);
			else
				CPythonGraphic::Instance().RenderCoolTimeBoxInverse(fxCenter, fyCenter, fRadius, fPercentage);
#else
			CPythonGraphic::Instance().RenderCoolTimeBox(m_rect.left + rSlot.ixPosition + 16.0f, m_rect.top + rSlot.iyPosition + 16.0f, 16.0f, fPercentage);
#endif

			if (fcurTime - rSlot.fStartCoolTime >= rSlot.fCoolTime)
			{
#ifdef ENABLE_GROWTH_PET_SYSTEM
				if (rSlot.bIsInverseCoolTime)
				{
					CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 0.5f);
					CPythonGraphic::Instance().RenderBar2d(m_rect.left + rSlot.ixPosition, m_rect.top + rSlot.iyPosition,
						m_rect.left + rSlot.ixPosition + rSlot.ixCellSize,
						m_rect.top + rSlot.iyPosition + rSlot.iyCellSize);
				}
				else
#endif
				{
					// If the cooldown is less than 1 second...
					if ((fcurTime - rSlot.fStartCoolTime) - rSlot.fCoolTime < 1.0f)
						__CreateFinishCoolTimeEffect(&rSlot);

					rSlot.fCoolTime = 0.0f;
					rSlot.fStartCoolTime = 0.0f;
				}
			}
		}

		if (rSlot.pCoverButton)
		{
			rSlot.pCoverButton->SetPosition(rSlot.ixPosition, rSlot.iyPosition);
			rSlot.pCoverButton->RenderButton();
		}

		if (rSlot.pNumberLine)
		{
			int ix = rSlot.byxPlacedItemSize * ITEM_WIDTH + rSlot.ixPosition - 4;
			int iy = rSlot.iyPosition + rSlot.byyPlacedItemSize * ITEM_HEIGHT - 12 + 2;
			rSlot.pNumberLine->SetPosition(ix, iy);
			rSlot.pNumberLine->Update();
			rSlot.pNumberLine->Render();
		}

#ifdef SIGN_FOR_PLUS_ITEMS
		if (rSlot.pPlusLine)
		{
			int ix = rSlot.ixPosition + rSlot.byxPlacedItemSize + rSlot.ixPlusPosition;
			int iy = rSlot.iyPosition + rSlot.byyPlacedItemSize + rSlot.iyPlusPosition;
			rSlot.pPlusLine->SetPosition(ix, iy);
			rSlot.pPlusLine->Update();
			rSlot.pPlusLine->Render();
		}
#endif

		if (rSlot.pFinishCoolTimeEffect)
		{
			rSlot.pFinishCoolTimeEffect->SetPosition(rSlot.ixPosition, rSlot.iyPosition);
			rSlot.pFinishCoolTimeEffect->Update();
			rSlot.pFinishCoolTimeEffect->Render();
		}

		if (rSlot.bActive)
		{
#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
			if (m_apSlotActiveEffect[0] || m_apSlotActiveEffect[1] || m_apSlotActiveEffect[2])
#else
			if (m_pSlotActiveEffect)
#endif
			{
				int ix = m_rect.left + rSlot.ixPosition;
				int iy = m_rect.top + rSlot.iyPosition;
#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
				int highlight = rSlot.byyPlacedItemSize - 1;
				m_apSlotActiveEffect[highlight]->Show();
				m_apSlotActiveEffect[highlight]->SetPosition(ix, iy);
				m_apSlotActiveEffect[highlight]->SetDiffuseColorByType(rSlot.byToggleColorType);
				m_apSlotActiveEffect[highlight]->Render();
#else
				m_pSlotActiveEffect->SetPosition(ix, iy);
				m_pSlotActiveEffect->Render();
#endif
			}
		}
	}

	RenderLockedSlot();
}

void CSlotWindow::RenderSlotBaseImage()
{
	if (!m_pBaseImageInstance)
		return;

	for (auto& rSlot : m_SlotList)
	{
		if (!rSlot.bRenderBaseSlotImage)
			continue;

		m_pBaseImageInstance->SetPosition(m_rect.left + rSlot.ixPosition, m_rect.top + rSlot.iyPosition);
		m_pBaseImageInstance->Render();
	}
}

void CSlotWindow::OnRenderPickingSlot()
{
	if (!CWindowManager::Instance().IsAttaching())
		return;

	TSlot* pSlot;
	if (!GetSelectedSlotPointer(&pSlot))
		return;

	CPythonGraphic::Instance().SetDiffuseColor(1.0f, 1.0f, 1.0f, 0.5f);
	CPythonGraphic::Instance().RenderBar2d(m_rect.left + pSlot->ixPosition, m_rect.top + pSlot->iyPosition,
		m_rect.left + pSlot->ixPosition + pSlot->ixCellSize,
		m_rect.top + pSlot->iyPosition + pSlot->iyCellSize);
}

void CSlotWindow::OnRenderSelectedSlot()
{
	auto itor = m_dwSelectedSlotIndexList.begin();
	for (; itor != m_dwSelectedSlotIndexList.end(); ++itor)
	{
		TSlot* pSlot;
		if (!GetSlotPointer(*itor, &pSlot))
			continue;

		CPythonGraphic::Instance().SetDiffuseColor(1.0f, 1.0f, 1.0f, 0.5f);
		CPythonGraphic::Instance().RenderBar2d(m_rect.left + pSlot->ixPosition, m_rect.top + pSlot->iyPosition,
			m_rect.left + pSlot->ixPosition + pSlot->ixCellSize,
			m_rect.top + pSlot->iyPosition + pSlot->iyCellSize);
	}
}

void CSlotWindow::RenderLockedSlot()
{
	CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 0.5f);
	for (auto& rSlot : m_SlotList)
	{
		if (!rSlot.isItem)
			continue;

		if (rSlot.dwState & SLOT_STATE_LOCK)
		{
			CPythonGraphic::Instance().RenderBar2d(m_rect.left + rSlot.ixPosition, m_rect.top + rSlot.iyPosition,
				m_rect.left + rSlot.ixPosition + rSlot.ixCellSize,
				m_rect.top + rSlot.iyPosition + rSlot.iyCellSize);
		}
	}
}

// Check Slot

BOOL CSlotWindow::GetSlotPointer(uint32_t dwIndex, TSlot** ppSlot)
{
	for (auto& rSlot : m_SlotList)
	{
		if (dwIndex == rSlot.dwSlotNumber)
		{
			*ppSlot = &rSlot;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CSlotWindow::GetSelectedSlotPointer(TSlot** ppSlot)
{
	long lx, ly;
	GetMouseLocalPosition(lx, ly);

	for (auto& rSlot : m_SlotList)
	{
		if (lx >= rSlot.ixPosition)
			if (ly >= rSlot.iyPosition)
			{
				if (lx <= rSlot.ixPosition + rSlot.ixCellSize)
				{
					if (ly <= rSlot.iyPosition + rSlot.iyCellSize)
					{
						*ppSlot = &rSlot;
						return TRUE;
					}
				}
			}
	}

	return FALSE;
}

BOOL CSlotWindow::GetPickedSlotPointer(TSlot** ppSlot)
{
	long lx, ly;
	CWindowManager::Instance().GetMousePosition(lx, ly);

	int ixLocal = lx - m_rect.left;
	int iyLocal = ly - m_rect.top;

	// NOTE : 왼쪽 맨위 상단 한곳이 기준 이라는 점을 이용해 왼쪽 위에서부터 오른쪽 아래로
	//        차례로 검색해 감으로써 덮혀 있는 Slot은 자동 무시 된다는 특성을 이용한다. - [levites]
	for (auto& rSlot : m_SlotList)
	{
		int ixCellSize = rSlot.ixCellSize;
		int iyCellSize = rSlot.iyCellSize;

		// NOTE : Item이 Hide 되어있을 경우를 위한..
		if (rSlot.isItem)
		{
			ixCellSize = std::max(rSlot.ixCellSize, int(rSlot.byxPlacedItemSize * ITEM_WIDTH));
			iyCellSize = std::max(rSlot.iyCellSize, int(rSlot.byyPlacedItemSize * ITEM_HEIGHT));
		}

		if (ixLocal >= rSlot.ixPosition)
		{
			if (iyLocal >= rSlot.iyPosition)
			{
				if (ixLocal <= rSlot.ixPosition + ixCellSize)
					if (iyLocal <= rSlot.iyPosition + iyCellSize)
					{
						*ppSlot = &rSlot;
						return TRUE;
					}
			}
		}
	}

	return FALSE;
}

void CSlotWindow::SetUseMode(BOOL bFlag)
{
	m_isUseMode = bFlag;
}

void CSlotWindow::SetUsableItem(BOOL bFlag)
{
	m_isUsableItem = bFlag;
}

void CSlotWindow::ReserveDestroyCoolTimeFinishEffect(uint32_t dwSlotIndex)
{
	m_ReserveDestroyEffectDeque.emplace_back(dwSlotIndex);
}

uint32_t CSlotWindow::Type()
{
	static int s_Type = GetCRC32("CSlotWindow", strlen("CSlotWindow"));
	return s_Type;
}

BOOL CSlotWindow::OnIsType(uint32_t dwType)
{
	if (Type() == dwType)
		return TRUE;

	return CWindow::OnIsType(dwType);
}

void CSlotWindow::__CreateToggleSlotImage()
{
	__DestroyToggleSlotImage();

	m_pToggleSlotImage = new CImageBox(nullptr);
	m_pToggleSlotImage->LoadImage("d:/ymir work/ui/public/slot_toggle.sub");
	m_pToggleSlotImage->Show();
}

void CSlotWindow::__CreateSlotEnableEffect()
{
	__DestroySlotEnableEffect();

#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
	for (int i = 0; i < 3; ++i)
	{
		CAniImageBox* pEff = new CAniImageBox(nullptr);
		for (int j = 0; j <= 12; ++j)
		{
			char buf[64 + 1];
			if (i == 0)
				sprintf_s(buf, "d:/ymir work/ui/public/slotactiveeffect/%02d.sub", j);
			else
				sprintf_s(buf, "d:/ymir work/ui/public/slotactiveeffect/slot%d/%02d.sub", (i + 1), j);

			pEff->AppendImage(buf);
		}

		pEff->SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
		m_apSlotActiveEffect[i] = pEff;
	}
#else
	m_pSlotActiveEffect = new CAniImageBox(nullptr);
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/00.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/01.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/02.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/03.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/04.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/05.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/06.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/07.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/08.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/09.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/10.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/11.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/12.sub");
	m_pSlotActiveEffect->SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
	m_pSlotActiveEffect->Show();
#endif
}

void CSlotWindow::__CreateFinishCoolTimeEffect(TSlot* pSlot)
{
	__DestroyFinishCoolTimeEffect(pSlot);

	CAniImageBox* pFinishCoolTimeEffect = new CCoolTimeFinishEffect(this, pSlot->dwSlotNumber);
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/00.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/01.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/02.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/03.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/04.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/05.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/06.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/07.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/08.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/09.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/10.sub");
	pFinishCoolTimeEffect->SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
	pFinishCoolTimeEffect->ResetFrame();
	pFinishCoolTimeEffect->SetDelay(2);
	pFinishCoolTimeEffect->Show();

	pSlot->pFinishCoolTimeEffect = pFinishCoolTimeEffect;
}

void CSlotWindow::__CreateBaseImage(const char* c_szFileName, float fr, float fg, float fb, float fa)
{
	__DestroyBaseImage();

	auto* pImage = msl::inherit_cast<CGraphicImage*>(CResourceManager::Instance().GetResourcePointer(c_szFileName));
	m_pBaseImageInstance = CGraphicImageInstance::New();
	m_pBaseImageInstance->SetImagePointer(pImage);
	m_pBaseImageInstance->SetDiffuseColor(fr, fg, fb, fa);
#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_GROWTH_PET_SYSTEM)
	m_pBaseImageInstance->SetScale(m_pScaleX, m_pScaleY);
#endif
}

void CSlotWindow::__DestroyToggleSlotImage()
{
	if (m_pToggleSlotImage)
	{
		delete m_pToggleSlotImage;
		m_pToggleSlotImage = nullptr;
	}
}

void CSlotWindow::__DestroySlotEnableEffect()
{
#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
	for (int i = 0; i < 3; ++i)
	{
		if (m_apSlotActiveEffect[i])
		{
			delete m_apSlotActiveEffect[i];
			m_apSlotActiveEffect[i] = nullptr;
		}
	}
#else
	if (m_pSlotActiveEffect)
	{
		delete m_pSlotActiveEffect;
		m_pSlotActiveEffect = nullptr;
	}
#endif
}

void CSlotWindow::__DestroyFinishCoolTimeEffect(TSlot* pSlot)
{
	if (pSlot->pFinishCoolTimeEffect)
	{
		delete pSlot->pFinishCoolTimeEffect;
		pSlot->pFinishCoolTimeEffect = nullptr;
	}
}

void CSlotWindow::__DestroyBaseImage()
{
	if (m_pBaseImageInstance)
	{
		CGraphicImageInstance::Delete(m_pBaseImageInstance);
		m_pBaseImageInstance = nullptr;
	}
}

void CSlotWindow::__Initialize()
{
	m_dwSlotType = 0;
	m_dwSlotStyle = SLOT_STYLE_PICK_UP;
	m_dwToolTipSlotNumber = SLOT_NUMBER_NONE;
	m_dwOverInSlotNumber = SLOT_NUMBER_NONE;
#ifdef ENABLE_OFFICAL_FEATURES
	m_CoolDownStore.clear();
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	m_pScaleX = 1.0;
	m_pScaleY = 1.0;
#endif

	m_isUseMode = FALSE;
	m_isUsableItem = FALSE;

	m_pToggleSlotImage = nullptr;
#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
	for (int i = 0; i < 3; ++i)
		m_apSlotActiveEffect[i] = nullptr;
#else
	m_pSlotActiveEffect = nullptr;
#endif
	m_pBaseImageInstance = nullptr;
#ifdef ENABLE_SLOT_COVER_IMAGE_SYSTEM
	m_pSlotCoverImage = nullptr;
#endif
}

#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_GROWTH_PET_SYSTEM)
void CSlotWindow::SetScale(float fy, float fx)
{
	if (fx < 0.0)
		fx = 1.0;
	if (fy < 0.0)
		fy = 1.0;

	m_pScaleX = fx;
	m_pScaleY = fy;
}
#endif

#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
void CSlotWindow::SetSlotDiffuseColor(uint32_t dwIndex, uint8_t iColorType)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->byToggleColorType = iColorType;
}
#endif

#ifdef ENABLE_SLOT_COVER_IMAGE_SYSTEM
void CSlotWindow::SetSlotCoverImage(uint32_t dwIndex, const char* c_filename)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->bSlotCoverImage = TRUE;

	if (!m_pSlotCoverImage)
	{
		m_pSlotCoverImage = new CAniImageBox(nullptr);
		m_pSlotCoverImage->AppendImage(c_filename);
		m_pSlotCoverImage->Show();
	}
}

void CSlotWindow::EnableSlotCoverImage(uint32_t dwIndex, BOOL bEnable)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (bEnable)
		pSlot->bSlotCoverImage = TRUE;
	else
		pSlot->bSlotCoverImage = FALSE;
}
#endif

void CSlotWindow::Destroy()
{
	for (auto& rSlot : m_SlotList)
	{
		ClearSlot(&rSlot);

		if (rSlot.pNumberLine)
		{
			delete rSlot.pNumberLine;
			rSlot.pNumberLine = nullptr;
		}

#ifdef SIGN_FOR_PLUS_ITEMS
		if (rSlot.pPlusLine)
		{
			delete rSlot.pPlusLine;
			rSlot.pPlusLine = nullptr;
		}
#endif

		if (rSlot.pCoverButton)
			CWindowManager::Instance().DestroyWindow(rSlot.pCoverButton);
		if (rSlot.pSlotButton)
			CWindowManager::Instance().DestroyWindow(rSlot.pSlotButton);
		if (rSlot.pSignImage)
			CWindowManager::Instance().DestroyWindow(rSlot.pSignImage);
		if (rSlot.pFinishCoolTimeEffect)
			CWindowManager::Instance().DestroyWindow(rSlot.pFinishCoolTimeEffect);
	}

	m_SlotList.clear();

	__DestroyToggleSlotImage();
	__DestroySlotEnableEffect();
	__DestroyBaseImage();

	__Initialize();
}

CSlotWindow::CSlotWindow(PyObject* ppyObject) : CWindow(ppyObject)
{
	__Initialize();
}

CSlotWindow::~CSlotWindow()
{
	Destroy();
}
