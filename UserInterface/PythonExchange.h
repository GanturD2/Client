#pragma once

#include "Packet.h"

/*
 *	교환 창 관련
 */

class CPythonExchange : public CSingleton<CPythonExchange>
{
public:
	enum
	{
		EXCHANGE_ITEM_MAX_NUM = 12
	};

	typedef struct trade
	{
		char name[CHARACTER_NAME_MAX_LEN + 1];
#ifdef ENABLE_LEVEL_IN_TRADE
		uint32_t level;
#endif

		uint32_t item_vnum[EXCHANGE_ITEM_MAX_NUM];
		uint8_t item_count[EXCHANGE_ITEM_MAX_NUM];
		uint32_t item_metin[EXCHANGE_ITEM_MAX_NUM][ITEM_SOCKET_SLOT_MAX_NUM];
		TPlayerItemAttribute item_attr[EXCHANGE_ITEM_MAX_NUM][ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#ifdef ENABLE_YOHARA_SYSTEM
		TPlayerItemApplyRandom	item_sungma_attr[EXCHANGE_ITEM_MAX_NUM][APPLY_RANDOM_SLOT_MAX_NUM];
		uint16_t item_random_values[EXCHANGE_ITEM_MAX_NUM][ITEM_RANDOM_VALUES_MAX_NUM];
#endif

		uint8_t accept;
		uint32_t elk;
#ifdef ENABLE_CHEQUE_SYSTEM
		uint32_t					cheque;
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		uint32_t					dwTransmutationVnum[EXCHANGE_ITEM_MAX_NUM];
#endif
#ifdef ENABLE_REFINE_ELEMENT
		uint8_t 					item_grade_element[EXCHANGE_ITEM_MAX_NUM];
		uint32_t 	 				item_attack_element[EXCHANGE_ITEM_MAX_NUM][MAX_ELEMENTS_SPELL];
		uint8_t 					item_element_type_bonus[EXCHANGE_ITEM_MAX_NUM];
		int16_t 					item_elements_value_bonus[EXCHANGE_ITEM_MAX_NUM][MAX_ELEMENTS_SPELL];
#endif
#ifdef ENABLE_SET_ITEM
		uint8_t					set_value[EXCHANGE_ITEM_MAX_NUM];
#endif
	} TExchangeData;

public:
	CPythonExchange();
	~CPythonExchange();
	CLASS_DELETE_COPYMOVE(CPythonExchange);

	void Clear();

	void Start();
	void End();
	bool isTrading() const;

	// Interface

	void SetSelfName(const char * name);
	void SetTargetName(const char * name);

	char * GetNameFromSelf();
	char * GetNameFromTarget();

#ifdef ENABLE_LEVEL_IN_TRADE
	void SetSelfLevel(uint32_t level);
	void SetTargetLevel(uint32_t level);

	uint32_t GetLevelFromSelf() const;
	uint32_t GetLevelFromTarget() const;
#endif

	void SetElkToTarget(uint32_t elk);
	void SetElkToSelf(uint32_t elk);

	uint32_t GetElkFromTarget() const;
	uint32_t GetElkFromSelf() const;

#ifdef ENABLE_CHEQUE_SYSTEM
	void			SetChequeToTarget(uint32_t cheque);
	void			SetChequeToSelf(uint32_t cheque);

	uint32_t			GetChequeFromTarget();
	uint32_t			GetChequeFromSelf();
#endif

#ifdef ENABLE_REFINE_ELEMENT
	void 			SetItemElementToTarget(int pos, uint8_t value_grade_element, uint8_t type_element);
	void 			SetItemElementAttackToTarget(int pos, uint32_t attack_element_index, uint32_t attack_element);
	void 			SetItemElementValueToTarget(int pos, uint32_t elements_value_bonus_index, int16_t elements_value_bonus);
	uint8_t 			GetItemElementGradeFromTarget(uint8_t pos);
	uint32_t 			GetItemElementAttackFromTarget(uint8_t pos, uint32_t attack_element_index);
	uint8_t 			GetItemElementTypeFromTarget(uint8_t pos);
	int16_t 			GetItemElementValueFromTarget(uint8_t pos, uint32_t elements_value_bonus_index);
	void 			SetItemElementToSelf(int pos, uint8_t value_grade_element, uint8_t type_element);
	void 			SetItemElementAttackToSelf(int pos, uint32_t attack_element_index, uint32_t attack_element);
	void 			SetItemElementValueToSelf(int pos, uint32_t elements_value_bonus_index, int16_t elements_value_bonus);
	uint8_t 			GetItemElementGradeFromSelf(uint8_t pos);
	uint32_t 			GetItemElementAttackFromSelf(uint8_t pos, uint32_t attack_element_index);
	uint8_t 			GetItemElementTypeFromSelf(uint8_t pos);
	int16_t 			GetItemElementValueFromSelf(uint8_t pos, uint32_t elements_value_bonus_index);
#endif

#ifdef ENABLE_YOHARA_SYSTEM
	void SetItemApplyRandomToTarget(int pos, int iattrpos, uint16_t wType, int16_t sValue);
	void SetItemApplyRandomToSelf(int pos, int iattrpos, uint16_t wType, int16_t sValue);

	void GetItemApplyRandomFromTarget(uint8_t pos, int iAttrPos, uint16_t* pwType, int16_t* psValue) const;
	void GetItemApplyRandomFromSelf(uint8_t pos, int iAttrPos, uint16_t* pwType, int16_t* psValue) const;

	void SetRandomDefaultAttrs(int pos, int iRandomValuePos, uint16_t iRandomValue, bool bSelf);
	uint16_t GetRandomDefaultAttrs(uint8_t iPos, int iRandomValuePos, bool bSelf) const;
#endif

#ifdef ENABLE_SET_ITEM
	void SetItemSetValue(int iPos, uint8_t bSetItem, bool bSelf);
	uint32_t GetItemSetValueFromTarget(uint8_t pos);
	uint32_t GetItemSetValueFromSelf(uint8_t pos);
#endif

	void SetItemToTarget(uint32_t pos, uint32_t vnum, uint8_t count);
	void SetItemToSelf(uint32_t pos, uint32_t vnum, uint8_t count);

	void SetItemMetinSocketToTarget(int pos, int imetinpos, uint32_t vnum);
	void SetItemMetinSocketToSelf(int pos, int imetinpos, uint32_t vnum);

	void SetItemAttributeToTarget(int pos, int iattrpos, uint16_t wType, int16_t sValue);	//@fixme436
	void SetItemAttributeToSelf(int pos, int iattrpos, uint16_t wType, int16_t sValue);	//@fixme436

	void DelItemOfTarget(uint8_t pos);
	void DelItemOfSelf(uint8_t pos);

	uint32_t GetItemVnumFromTarget(uint8_t pos);
	uint32_t GetItemVnumFromSelf(uint8_t pos);

	uint8_t GetItemCountFromTarget(uint8_t pos);
	uint8_t GetItemCountFromSelf(uint8_t pos);

	uint32_t GetItemMetinSocketFromTarget(uint8_t pos, int iMetinSocketPos);
	uint32_t GetItemMetinSocketFromSelf(uint8_t pos, int iMetinSocketPos);

	void GetItemAttributeFromTarget(uint8_t pos, int iAttrPos, uint16_t * pwType, int16_t * psValue) const;	//@fixme436
	void GetItemAttributeFromSelf(uint8_t pos, int iAttrPos, uint16_t * pwType, int16_t * psValue) const;	//@fixme436

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	void SetItemChangeLookVnum(const int pos, const uint32_t blVnum, bool bSelf);
	uint32_t GetItemChangeLookVnum(int iPos, bool bSelf);

	uint32_t GetChangeLookVnumFromSelf(const uint8_t pos) const;
	uint32_t GetChangeLookVnumFromTarget(const uint8_t pos) const;
#endif

	void SetAcceptToTarget(uint8_t Accept);
	void SetAcceptToSelf(uint8_t Accept);

	bool GetAcceptFromTarget() const;
	bool GetAcceptFromSelf() const;

	bool GetElkMode() const;
	void SetElkMode(bool value);

protected:
	bool m_isTrading;

	bool m_elk_mode; // 엘크를 클릭해서 교환했을때를 위한 변종임.
	TExchangeData m_self;
	TExchangeData m_victim;
};
