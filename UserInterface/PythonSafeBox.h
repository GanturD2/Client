#pragma once

class CPythonSafeBox : public CSingleton<CPythonSafeBox>
{
public:
	enum
	{
		SAFEBOX_SLOT_X_COUNT = 5,
		SAFEBOX_SLOT_Y_COUNT = 9,
		SAFEBOX_PAGE_SIZE = SAFEBOX_SLOT_X_COUNT * SAFEBOX_SLOT_Y_COUNT
	};

	using TItemInstanceVector = std::vector<TItemData>;

public:
	CPythonSafeBox();
	~CPythonSafeBox();
	CLASS_DELETE_COPYMOVE(CPythonSafeBox);

	void OpenSafeBox(int iSize);
	void SetItemData(uint32_t dwSlotIndex, const TItemData & rItemInstance);
	void DelItemData(uint32_t dwSlotIndex);

#ifdef ENABLE_SAFEBOX_MONEY
	void SetMoney(uint32_t dwMoney);
	uint32_t GetMoney() const;
#endif

	BOOL GetSlotItemID(uint32_t dwSlotIndex, uint32_t * pdwItemID);

	int GetCurrentSafeBoxSize() const;
	BOOL GetItemDataPtr(uint32_t dwSlotIndex, TItemData ** ppInstance);

	// MALL
	void OpenMall(int iSize);
	void SetMallItemData(uint32_t dwSlotIndex, const TItemData & rItemData);
	void DelMallItemData(uint32_t dwSlotIndex);
	BOOL GetMallItemDataPtr(uint32_t dwSlotIndex, TItemData ** ppInstance);
	BOOL GetSlotMallItemID(uint32_t dwSlotIndex, uint32_t * pdwItemID);
	uint32_t GetMallSize() const;

	bool IsOpen() const { return isOpen; }
	void IsOpen(bool val) { isOpen = val; }

protected:
	TItemInstanceVector m_ItemInstanceVector;
	TItemInstanceVector m_MallItemInstanceVector;

#ifdef ENABLE_SAFEBOX_MONEY
	uint32_t m_dwMoney;
#endif
	bool isOpen;
};