#pragma once

#include "PythonSlotWindow.h"

namespace UI
{
class CGridSlotWindow : public CSlotWindow
{
public:
	static uint32_t Type();

public:
	explicit CGridSlotWindow(PyObject * ppyObject);
	~CGridSlotWindow() override;

	void Destroy() override;

	void ArrangeGridSlot(uint32_t dwStartIndex, uint32_t dwxCount, uint32_t dwyCount, int ixSlotSize, int iySlotSize, int ixTemporarySize,
						 int iyTemporarySize);

#ifdef ENABLE_FISH_EVENT
	void SetPickedAreaRender(bool bFlag) { bPickedAreaRender = bFlag; }
	bool GetPickedAreaRender() { return bPickedAreaRender; }
#endif

protected:
	void __Initialize() override;

	BOOL GetPickedSlotPointer(TSlot ** ppSlot) override;
	BOOL GetPickedSlotList(int iWidth, int iHeight, std::list<TSlot *> * pSlotPointerList);
	BOOL GetGridSlotPointer(int ix, int iy, TSlot ** ppSlot);
#ifdef ENABLE_SWAP_SYSTEM
	BOOL GetSlotPointerByNumber(uint32_t dwSlotNumber, TSlot ** ppSlot);
#endif
	BOOL GetPickedGridSlotPosition(int ixLocal, int iyLocal, int * pix, int * piy);
	BOOL CheckMoving(uint32_t dwSlotNumber, uint32_t dwItemIndex, const std::list<TSlot *> & c_rSlotList) const;
#ifdef ENABLE_SWAP_SYSTEM
	BOOL CheckSwapping(uint32_t dwSlotNumber, uint32_t dwItemIndex, const std::list<TSlot*> & c_rSlotList);
#endif

	BOOL OnIsType(uint32_t dwType) override;

	void OnRefreshSlot() override;
	void OnRenderPickingSlot() override;

protected:
	uint32_t m_dwxCount;
	uint32_t m_dwyCount;
#ifdef ENABLE_FISH_EVENT
	bool bPickedAreaRender;
#endif

	std::vector<TSlot *> m_SlotVector;
};
} // namespace UI
