#pragma once

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
class CPythonPrivateShopSearch : public CSingleton<CPythonPrivateShopSearch>
{
public:
	struct TSearchItemData : TItemData
	{
		uint32_t vid;
		uint32_t price;
#ifdef ENABLE_CHEQUE_SYSTEM
		uint32_t cheque;
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM //SEARCHSHOP_CHANGELOOK
		uint32_t dwTransmutationVnum;
#endif
#ifdef ENABLE_REFINE_ELEMENT //SEARCHSHOP_ELEMENT
		uint8_t grade_element;
		uint32_t attack_element[MAX_ELEMENTS_SPELL];
		uint8_t element_type_bonus;
		int16_t elements_value_bonus[MAX_ELEMENTS_SPELL];
#endif
		TItemPos Cell;
	};

	using TItemInstanceVector = std::vector<TSearchItemData>;

public:
	CPythonPrivateShopSearch();
	virtual ~CPythonPrivateShopSearch();

#ifdef ENABLE_CHEQUE_SYSTEM
	void AddItemData(uint32_t vid, uint32_t price, uint32_t cheque, const TSearchItemData& rItemData);
#else
	void AddItemData(uint32_t vid, uint32_t price, const TSearchItemData& rItemData);
#endif
	void ClearItemData();

	uint32_t GetItemDataCount() { return m_ItemInstanceVector.size(); }
	uint32_t GetItemDataPtr(uint32_t index, TSearchItemData** ppInstance);

protected:
	TItemInstanceVector m_ItemInstanceVector;
};
#endif
