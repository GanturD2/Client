#include "StdAfx.h"
#include "../EterBase/CRC32.h"
#include "PythonGridSlotWindow.h"

using namespace UI;

void CGridSlotWindow::OnRenderPickingSlot()
{
	if (!CWindowManager::Instance().IsAttaching())
		return;

#ifdef ENABLE_FISH_EVENT
	if (!GetPickedAreaRender())
		return;
#endif

	uint8_t byWidth, byHeight;
	CWindowManager::Instance().GetAttachingIconSize(&byWidth, &byHeight);

	std::list<TSlot *> SlotList;
	if (GetPickedSlotList(byWidth, byHeight, &SlotList))
	{
		uint32_t dwSlotNumber = CWindowManager::Instance().GetAttachingSlotNumber();
		uint32_t dwItemIndex = CWindowManager::Instance().GetAttachingIndex();

		if (m_isUseMode)
		{
			TSlot * pSlot = *SlotList.begin();
			TSlot * pCenterSlot;
			if (GetSlotPointer(pSlot->dwCenterSlotNumber, &pCenterSlot))
			{
				if (pCenterSlot->isItem)
				{
					if (m_isUsableItem)
						CPythonGraphic::Instance().SetDiffuseColor(1.0f, 1.0f, 0.0f, 0.5f);
#ifdef ENABLE_SWAP_SYSTEM
					else if (pCenterSlot->dwItemIndex != dwItemIndex)
						CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.0f, 0.0f, 0.5f);
#endif
					else
						CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.0f, 0.0f, 0.5f);

					CPythonGraphic::Instance().RenderBar2d(
						m_rect.left + pCenterSlot->ixPosition, m_rect.top + pCenterSlot->iyPosition,
						m_rect.left + pCenterSlot->ixPosition + pCenterSlot->byxPlacedItemSize * ITEM_WIDTH,
						m_rect.top + pCenterSlot->iyPosition + pCenterSlot->byyPlacedItemSize * ITEM_HEIGHT);
					return;
				}
			}
		}
#ifdef ENABLE_FISH_EVENT
		uint32_t dwAttachedType = UI::CWindowManager::Instance().GetAttachingType();
		uint32_t dwAttachedIndex = UI::CWindowManager::Instance().GetAttachingIndex();
		std::list<TSlot*> SlotListForCheck = SlotList;

		if (dwAttachedType == 19 && (dwAttachedIndex == 3 || dwAttachedIndex == 4 || dwAttachedIndex == 6))
		{
			int pos = 0;
			for (auto it = SlotListForCheck.begin(); it != SlotListForCheck.end(); ++it, ++pos)
			{
				if ((dwAttachedIndex == 6 && (pos == 1 || pos == 3)) || (dwAttachedIndex == 3 && pos == 2) || (dwAttachedIndex == 4 && pos == 1))
					it = SlotListForCheck.erase(it);
			}
		}

		if (CheckMoving(dwSlotNumber, dwItemIndex, SlotListForCheck))
			CPythonGraphic::Instance().SetDiffuseColor(1.0f, 1.0f, 1.0f, 0.5f);
#else
		if (CheckMoving(dwSlotNumber, dwItemIndex, SlotList))
			CPythonGraphic::Instance().SetDiffuseColor(1.0f, 1.0f, 1.0f, 0.5f);
#endif
#ifdef ENABLE_SWAP_SYSTEM
		else if (CheckSwapping(dwSlotNumber, byHeight, SlotList))
			CPythonGraphic::Instance().SetDiffuseColor(0.22f, 0.67f, 0.65f, 0.5f);
#endif
		else
			CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.0f, 0.0f, 0.5f);

		RECT Rect;
		Rect.left = m_rect.right;
		Rect.top = m_rect.bottom;
		Rect.right = 0;
		Rect.bottom = 0;

#ifdef ENABLE_FISH_EVENT
		TSlot * pSlotFish = *SlotList.begin();

		RECT SecondRect; SecondRect.left = m_rect.right; SecondRect.top = m_rect.bottom; SecondRect.right = 0; SecondRect.bottom = 0;

		if (dwAttachedType == 19 && (dwAttachedIndex == 3 || dwAttachedIndex == 4 || dwAttachedIndex == 6))
		{
			Rect.left = std::min(Rect.left, m_rect.left + pSlotFish->ixPosition);
			Rect.top = std::min(Rect.top, m_rect.top + pSlotFish->iyPosition);
			Rect.right = std::max(Rect.right, m_rect.left + pSlotFish->ixPosition + (dwAttachedIndex == 3 ? 32 : 64));
			Rect.bottom = std::max(Rect.bottom, m_rect.top + pSlotFish->iyPosition + 32);

			SecondRect.left = std::min(SecondRect.left, m_rect.left + pSlotFish->ixPosition + (dwAttachedIndex == 3 ? 0 : 32));
			SecondRect.top = std::min(SecondRect.top, m_rect.top + pSlotFish->iyPosition + 32);
			SecondRect.right = std::max(SecondRect.right, m_rect.left + pSlotFish->ixPosition + (dwAttachedIndex == 6 ? 96 : 64));
			SecondRect.bottom = std::max(SecondRect.bottom, m_rect.top + pSlotFish->iyPosition + 64);

			CPythonGraphic::Instance().RenderBar2d(Rect.left, Rect.top, Rect.right, Rect.bottom);
			CPythonGraphic::Instance().RenderBar2d(SecondRect.left, SecondRect.top, SecondRect.right, SecondRect.bottom);
		}
		else
		{
			for (auto & pSlot : SlotList)
			{
				Rect.left = std::min(Rect.left, m_rect.left + pSlot->ixPosition);
				Rect.top = std::min(Rect.top, m_rect.top + pSlot->iyPosition);
				Rect.right = std::max(Rect.right, m_rect.left + pSlot->ixPosition + pSlot->byxPlacedItemSize * ITEM_WIDTH);
				Rect.bottom = std::max(Rect.bottom, m_rect.top + pSlot->iyPosition + pSlot->byxPlacedItemSize * ITEM_HEIGHT);
			}

			CPythonGraphic::Instance().RenderBar2d(Rect.left, Rect.top, Rect.right, Rect.bottom);
		}
#else
		for (auto & pSlot : SlotList)
		{
			Rect.left = std::min(Rect.left, m_rect.left + pSlot->ixPosition);
			Rect.top = std::min(Rect.top, m_rect.top + pSlot->iyPosition);
			Rect.right = std::max(Rect.right, m_rect.left + pSlot->ixPosition + pSlot->byxPlacedItemSize * ITEM_WIDTH);
			Rect.bottom = std::max(Rect.bottom, m_rect.top + pSlot->iyPosition + pSlot->byxPlacedItemSize * ITEM_HEIGHT);
		}

		CPythonGraphic::Instance().RenderBar2d(Rect.left, Rect.top, Rect.right, Rect.bottom);
#endif
	}
}

BOOL CGridSlotWindow::GetPickedSlotPointer(TSlot ** ppSlot)
{
	if (!CWindowManager::Instance().IsAttaching())
		return CSlotWindow::GetPickedSlotPointer(ppSlot);

	uint8_t byWidth, byHeight;
	CWindowManager::Instance().GetAttachingIconSize(&byWidth, &byHeight);

	std::list<TSlot *> SlotList;
	if (!GetPickedSlotList(byWidth, byHeight, &SlotList))
		return FALSE;

#ifdef ENABLE_FISH_EVENT
	uint32_t dwAttachedType = UI::CWindowManager::Instance().GetAttachingType();
	uint32_t dwAttachedIndex = UI::CWindowManager::Instance().GetAttachingIndex();
	if (dwAttachedType == 19 && (dwAttachedIndex == 3 || dwAttachedIndex == 4 || dwAttachedIndex == 6))
	{
		int pos = 0;
		for (auto it = SlotList.begin(); it != SlotList.end(); ++it, ++pos)
		{
			if ((dwAttachedIndex == 6 && (pos == 1 || pos == 3)) || (dwAttachedIndex == 3 && pos == 2) || (dwAttachedIndex == 4 && pos == 1))
				it = SlotList.erase(it);
		}
	}
#endif

	TSlot * pMinSlot = nullptr;
	//uint32_t dwSlotNumber = UI::CWindowManager::Instance().GetAttachingSlotNumber();
	//uint32_t dwAttachingItemIndex = UI::CWindowManager::Instance().GetAttachingIndex();

	for (auto & pSlot : SlotList)
	{
		// NOTE : 한 슬롯 이상 사이즈의 아이템의 경우 가장 왼쪽 위의 슬롯 포인터를 리턴한다.
		//        명시적이지 못한 코드.. 더 좋은 방법은 없는가? - [levites]
		if (!pMinSlot)
			pMinSlot = pSlot;
		else
		{
			if (pSlot->dwSlotNumber < pMinSlot->dwSlotNumber)
				pMinSlot = pSlot;
			else
			{
				if (!pMinSlot->isItem && pSlot->isItem)
					pMinSlot = pSlot;
			}
		}
	}

	if (!pMinSlot)
		return FALSE;
	TSlot * pCenterSlot;
	if (!GetSlotPointer(pMinSlot->dwCenterSlotNumber, &pCenterSlot))
		return FALSE;

	*ppSlot = pCenterSlot;

	// 현재 아이템을 들고 있는 중이고..
	if (CWindowManager::Instance().IsAttaching())
	{
		uint32_t dwSlotNumber = CWindowManager::Instance().GetAttachingSlotNumber();

		if (dwSlotNumber == pCenterSlot->dwSlotNumber)
			*ppSlot = pMinSlot;
	}

	return TRUE;
}

BOOL CGridSlotWindow::GetPickedSlotList(int iWidth, int iHeight, std::list<TSlot *> * pSlotPointerList)
{
	long lx, ly;
	GetMouseLocalPosition(lx, ly);

	if (lx < 0)
		return FALSE;
	if (ly < 0)
		return FALSE;
	if (lx >= GetWidth())
		return FALSE;
	if (ly >= GetHeight())
		return FALSE;

	pSlotPointerList->clear();

	int ix, iy;
	if (GetPickedGridSlotPosition(lx, ly, &ix, &iy))
	{
		int ixHalfStep = (iWidth / 2);
		//int iyHalfStep = (iHeight / 2);

		int ixStart = int(ix) - int(ixHalfStep - (ixHalfStep % 2));
		int ixEnd = int(ix) + int(ixHalfStep);

		// FIXME : 제대로 된 계산 공식을 찾자 - [levites]
		int iyStart = 0, iyEnd = 0;

		if (1 == iHeight)
		{
			iyStart = iy;
			iyEnd = iy;
		}
		else if (2 == iHeight)
		{
			iyStart = iy;
			iyEnd = iy + 1;
		}
		else if (3 == iHeight)
		{
			iyStart = iy - 1;
			iyEnd = iy + 1;
		}

#ifdef ENABLE_FISH_EVENT
		if (1 == iWidth)
		{
			ixStart = ix;
			ixEnd = ix;
		}
		else if (2 == iWidth)
		{
			ixStart = ix;
			ixEnd = ix + 1;
		}
		else if (3 == iWidth)
		{
			ixStart = ix - 1;
			ixEnd = ix + 1;
		}
#endif

		if (ixStart < 0)
		{
			ixEnd += -ixStart;
			ixStart = 0;
		}

		if (iyStart < 0)
		{
			iyEnd += -iyStart;
			iyStart = 0;
		}

		if (uint32_t(ixEnd) >= m_dwxCount)
		{
			int ixTemporary = uint32_t(ixEnd) - m_dwxCount + 1;
			ixStart -= ixTemporary;
			ixEnd -= ixTemporary;
		}

		if (uint32_t(iyEnd) >= m_dwyCount)
		{
			int iyTemporary = uint32_t(iyEnd) - m_dwyCount + 1;
			iyStart -= iyTemporary;
			iyEnd -= iyTemporary;
		}

		for (int i = ixStart; i <= ixEnd; ++i)
		{
			for (int j = iyStart; j <= iyEnd; ++j)
			{
				TSlot * pSlot;
				if (GetGridSlotPointer(uint32_t(i), uint32_t(j), &pSlot))
					pSlotPointerList->emplace_back(pSlot);
			}
		}

		// Refine Scroll 등을 위한 예외 처리
		if (m_isUseMode && 1 == pSlotPointerList->size())
		{
			TSlot * pMainSlot = *pSlotPointerList->begin();

			auto itor = m_SlotVector.begin();
			for (; itor != m_SlotVector.end(); ++itor)
			{
				TSlot * pSlot = *itor;
				if (pSlot->dwCenterSlotNumber == pMainSlot->dwCenterSlotNumber)
				{
					if (pSlotPointerList->end() == std::find(pSlotPointerList->begin(), pSlotPointerList->end(), pSlot))
						pSlotPointerList->emplace_back(pSlot);
				}
			}
		}

		if (!pSlotPointerList->empty())
			return TRUE;
	}

	return FALSE;
}

BOOL CGridSlotWindow::GetGridSlotPointer(int ix, int iy, TSlot ** ppSlot)
{
	uint32_t dwSlotIndex = ix + iy * m_dwxCount;
	if (dwSlotIndex >= m_SlotVector.size())
		return FALSE;

	*ppSlot = m_SlotVector[dwSlotIndex];

	return TRUE;
}

#ifdef ENABLE_SWAP_SYSTEM
BOOL CGridSlotWindow::GetSlotPointerByNumber(uint32_t dwSlotNumber, TSlot ** ppSlot)
{
	if (dwSlotNumber >= m_SlotVector.size())
		return false;

	*ppSlot = m_SlotVector[dwSlotNumber];

	return true;
}
#endif

BOOL CGridSlotWindow::GetPickedGridSlotPosition(int ixLocal, int iyLocal, int * pix, int * piy)
{
	for (uint32_t x = 0; x < m_dwxCount; ++x)
	{
		for (uint32_t y = 0; y < m_dwyCount; ++y)
		{
			TSlot * pSlot;
			if (!GetGridSlotPointer(x, y, &pSlot))
				continue;

			if (ixLocal >= pSlot->ixPosition)
			{
				if (iyLocal >= pSlot->iyPosition)
				{
					if (ixLocal <= pSlot->ixPosition + pSlot->ixCellSize)
					{
						if (iyLocal <= pSlot->iyPosition + pSlot->iyCellSize)
						{
							*pix = x;
							*piy = y;
							return TRUE;
						}
					}
				}
			}
		}
	}

	return FALSE;
}

void CGridSlotWindow::ArrangeGridSlot(uint32_t dwStartIndex, uint32_t dwxCount, uint32_t dwyCount, int ixSlotSize, int iySlotSize,
									  int ixTemporarySize, int iyTemporarySize)
{
	Destroy();

	m_dwxCount = dwxCount;
	m_dwyCount = dwyCount;

	m_SlotVector.clear();
	m_SlotVector.resize(dwxCount * dwyCount);

	for (uint32_t x = 0; x < dwxCount; ++x)
	{
		for (uint32_t y = 0; y < dwyCount; ++y)
		{
			uint32_t dwIndex = dwStartIndex + x + y * dwxCount;
			int ixPosition = x * (ixSlotSize + ixTemporarySize);
			int iyPosition = y * (iySlotSize + iyTemporarySize);

			AppendSlot(dwIndex, ixPosition, iyPosition, ixSlotSize, iySlotSize);

			m_SlotVector[x + y * dwxCount] = &(*m_SlotList.rbegin());
		}
	}

	int iWidth = dwxCount * (ixSlotSize + ixTemporarySize);
	int iHeight = dwyCount * (iySlotSize + iyTemporarySize);
	SetSize(iWidth, iHeight);
}

void CGridSlotWindow::OnRefreshSlot()
{
	uint32_t x, y;

	for (x = 0; x < m_dwxCount; ++x)
	{
		for (y = 0; y < m_dwyCount; ++y)
		{
			TSlot * pSlot;
			if (!GetGridSlotPointer(x, y, &pSlot))
				continue;

			pSlot->dwCenterSlotNumber = pSlot->dwSlotNumber;
		}
	}

	for (x = 0; x < m_dwxCount; ++x)
	{
		for (y = 0; y < m_dwyCount; ++y)
		{
			TSlot * pSlot;
			if (!GetGridSlotPointer(x, y, &pSlot))
				continue;

			if (pSlot->isItem)
			{
				for (uint32_t xSub = 0; xSub < pSlot->byxPlacedItemSize; ++xSub)
				{
					for (uint32_t ySub = 0; ySub < pSlot->byyPlacedItemSize; ++ySub)
					{
						TSlot * pSubSlot;
						if (!GetGridSlotPointer(x + xSub, y + ySub, &pSubSlot))
							continue;

						pSubSlot->dwCenterSlotNumber = pSlot->dwSlotNumber;
						pSubSlot->dwItemIndex = pSlot->dwItemIndex;
					}
				}
			}
			else
				pSlot->dwItemIndex = 0;
		}
	}
}

#ifdef ENABLE_SWAP_SYSTEM
BOOL CGridSlotWindow::CheckMoving(uint32_t dwSlotNumber, uint32_t dwItemIndex, const std::list<TSlot *> & c_rSlotList) const	//@fixme404
{
	if (m_dwSlotStyle != SLOT_STYLE_PICK_UP)
		return TRUE;

	uint16_t wCellMaxPerPage = m_SlotVector.size();
	while (dwSlotNumber >= wCellMaxPerPage)
		dwSlotNumber -= wCellMaxPerPage;

	for (std::list<TSlot*>::const_iterator itor = c_rSlotList.begin(); itor != c_rSlotList.end(); ++itor)
	{
		TSlot* pSlot = *itor;
		if (dwSlotNumber == pSlot->dwSlotNumber && itor == c_rSlotList.begin())
			return TRUE;

		if (dwSlotNumber != pSlot->dwCenterSlotNumber)
		{
			if (c_rSlotList.size() == 2)
			{
				std::list<TSlot*>::const_iterator it = c_rSlotList.begin();
				std::advance(it, 1);
				if (0 != pSlot->dwItemIndex && 0 != (*it)->dwItemIndex)
					return FALSE;
			}
			if (c_rSlotList.size() == 3)
			{
				std::list<TSlot*>::const_iterator it = c_rSlotList.begin();
				std::advance(it, 1);
				if (0 != pSlot->dwItemIndex && 0 != (*it)->dwItemIndex)
					return FALSE;
				else
				{
					std::advance(it, 1);
					if (0 != pSlot->dwItemIndex && 0 != (*it)->dwItemIndex)
						return FALSE;
				}
			}

			if (0 != pSlot->dwItemIndex || pSlot->dwCenterSlotNumber != pSlot->dwSlotNumber)
				if (dwItemIndex != pSlot->dwItemIndex)
					return FALSE;
		}
	}

	return TRUE;
}

BOOL CGridSlotWindow::CheckSwapping(uint32_t dwSlotNumber, uint32_t dwItemIndex, const std::list<TSlot*> & c_rSlotList)
{
	if (m_dwSlotStyle != SLOT_STYLE_PICK_UP)
		return TRUE;

	uint8_t byWidth, byHeight;
	UI::CWindowManager::Instance().GetAttachingIconSize(&byWidth, &byHeight);

	int iyBound = byHeight;
	int iyBasePosition = 0;

	for (std::list<TSlot*>::const_iterator itor = c_rSlotList.begin(); itor != c_rSlotList.end(); ++itor)
	{
		TSlot * pSlot = *itor;

		if (dwSlotNumber == pSlot->dwCenterSlotNumber) // I can't swap with myself
			return false;

		if (itor == c_rSlotList.begin()) { //First one, mark
			iyBasePosition = pSlot->iyPosition;
		}

		if (pSlot->dwSlotNumber == pSlot->dwCenterSlotNumber)
			iyBound -= pSlot->byyPlacedItemSize;

		if (!pSlot->dwItemIndex) {
			TSlot * centerItem;
			if (!GetSlotPointerByNumber(pSlot->dwCenterSlotNumber, &centerItem)) //Some sort of error
				continue;

			if (!centerItem || !centerItem->dwCenterSlotNumber) {
				continue; // I can always swap with empty slots, but this may not be the only overlayed slot, so lets continue
			}

			if (centerItem->iyPosition < iyBasePosition)
				return false; //Out of bounds, upper side
		}

		if (pSlot->iyPosition < iyBasePosition) //Out of bounds, upper side
			return false;

		if (iyBound < 0) //An item will go out of bounds on the lower side
			return false;
	}

	if (iyBound > 0) //Space was not perfectly filled
		return false;

	return true;
}
#else
BOOL CGridSlotWindow::CheckMoving(uint32_t dwSlotNumber, uint32_t dwItemIndex, const std::list<TSlot *> & c_rSlotList) const
{
	if (m_dwSlotStyle != SLOT_STYLE_PICK_UP)
		return TRUE;

	for (auto & pSlot : c_rSlotList)
	{
		if (dwSlotNumber != pSlot->dwCenterSlotNumber) // 들었던 자리가 아닐 경우에
		{
			if (0 != pSlot->dwItemIndex || pSlot->dwCenterSlotNumber != pSlot->dwSlotNumber) // 아이템이 있고
			{
				if (dwItemIndex != pSlot->dwItemIndex) // 다른 아이템이면 못 옮김
					return false;
			}
		}
	}

	return true;
}
#endif

void CGridSlotWindow::Destroy()
{
	CSlotWindow::Destroy();

	m_SlotVector.clear();

	__Initialize();
}

void CGridSlotWindow::__Initialize()
{
	m_dwxCount = 0;
	m_dwyCount = 0;
#ifdef ENABLE_FISH_EVENT
	bPickedAreaRender = true;
#endif
}

uint32_t CGridSlotWindow::Type()
{
	static int s_Type = GetCRC32("CGridSlotWindow", strlen("CGridSlotWindow"));
	return s_Type;
}

BOOL CGridSlotWindow::OnIsType(uint32_t dwType)
{
	if (Type() == dwType)
		return TRUE;

	return CSlotWindow::OnIsType(dwType);
}

CGridSlotWindow::CGridSlotWindow(PyObject * ppyObject) : CSlotWindow(ppyObject), m_dwxCount(0), m_dwyCount(0) {}

CGridSlotWindow::~CGridSlotWindow() = default;
