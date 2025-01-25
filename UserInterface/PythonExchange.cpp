#include "StdAfx.h"
#include "PythonExchange.h"

void CPythonExchange::SetSelfName(const char * name)
{
	strncpy(m_self.name, name, CHARACTER_NAME_MAX_LEN);
}

void CPythonExchange::SetTargetName(const char * name)
{
	strncpy(m_victim.name, name, CHARACTER_NAME_MAX_LEN);
}

char * CPythonExchange::GetNameFromSelf()
{
	return m_self.name;
}

char * CPythonExchange::GetNameFromTarget()
{
	return m_victim.name;
}

#ifdef ENABLE_LEVEL_IN_TRADE
void CPythonExchange::SetSelfLevel(uint32_t level)
{
	m_self.level = level;
}

void CPythonExchange::SetTargetLevel(uint32_t level)
{
	m_victim.level = level;
}

uint32_t CPythonExchange::GetLevelFromSelf() const
{
	return m_self.level;
}

uint32_t CPythonExchange::GetLevelFromTarget() const
{
	return m_victim.level;
}
#endif

void CPythonExchange::SetElkToTarget(uint32_t elk)
{
	m_victim.elk = elk;
}

void CPythonExchange::SetElkToSelf(uint32_t elk)
{
	m_self.elk = elk;
}

uint32_t CPythonExchange::GetElkFromTarget() const
{
	return m_victim.elk;
}

uint32_t CPythonExchange::GetElkFromSelf() const
{
	return m_self.elk;
}

#ifdef ENABLE_CHEQUE_SYSTEM
void CPythonExchange::SetChequeToTarget(uint32_t cheque)
{
	m_victim.cheque = cheque;
}

void CPythonExchange::SetChequeToSelf(uint32_t cheque)
{
	m_self.cheque = cheque;
}

uint32_t CPythonExchange::GetChequeFromTarget()
{
	return m_victim.cheque;
}

uint32_t CPythonExchange::GetChequeFromSelf()
{
	return m_self.cheque;
}
#endif

#ifdef ENABLE_REFINE_ELEMENT
//ELEMENT VICTIM
void CPythonExchange::SetItemElementToTarget(int pos, uint8_t value_grade_element, uint8_t type_element)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_grade_element[pos] = value_grade_element;
	m_victim.item_element_type_bonus[pos] = type_element;
}

void CPythonExchange::SetItemElementAttackToTarget(int pos, uint32_t attack_element_index, uint32_t attack_element)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_attack_element[pos][attack_element_index] = attack_element;
}

void CPythonExchange::SetItemElementValueToTarget(int pos, uint32_t elements_value_bonus_index, int16_t elements_value_bonus)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_elements_value_bonus[pos][elements_value_bonus_index] = elements_value_bonus;
}

uint8_t CPythonExchange::GetItemElementGradeFromTarget(uint8_t pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.item_grade_element[pos];
}

uint32_t CPythonExchange::GetItemElementAttackFromTarget(uint8_t pos, uint32_t attack_element_index)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.item_attack_element[pos][attack_element_index];
}

uint8_t CPythonExchange::GetItemElementTypeFromTarget(uint8_t pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.item_element_type_bonus[pos];
}

int16_t CPythonExchange::GetItemElementValueFromTarget(uint8_t pos, uint32_t elements_value_bonus_index)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.item_elements_value_bonus[pos][elements_value_bonus_index];
}

//ELEMENT SELF
void CPythonExchange::SetItemElementToSelf(int pos, uint8_t value_grade_element, uint8_t type_element)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_grade_element[pos] = value_grade_element;
	m_self.item_element_type_bonus[pos] = type_element;
}

void CPythonExchange::SetItemElementAttackToSelf(int pos, uint32_t attack_element_index, uint32_t attack_element)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_attack_element[pos][attack_element_index] = attack_element;
}

void CPythonExchange::SetItemElementValueToSelf(int pos, uint32_t elements_value_bonus_index, int16_t elements_value_bonus)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_elements_value_bonus[pos][elements_value_bonus_index] = elements_value_bonus;
}

uint8_t CPythonExchange::GetItemElementGradeFromSelf(uint8_t pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.item_grade_element[pos];
}

uint32_t CPythonExchange::GetItemElementAttackFromSelf(uint8_t pos, uint32_t attack_element_index)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.item_attack_element[pos][attack_element_index];
}

uint8_t CPythonExchange::GetItemElementTypeFromSelf(uint8_t pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.item_element_type_bonus[pos];
}

int16_t CPythonExchange::GetItemElementValueFromSelf(uint8_t pos, uint32_t elements_value_bonus_index)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.item_elements_value_bonus[pos][elements_value_bonus_index];
}
#endif

#ifdef ENABLE_YOHARA_SYSTEM
void CPythonExchange::SetItemApplyRandomToTarget(int pos, int iattrpos, uint16_t wType, int16_t sValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_sungma_attr[pos][iattrpos].wType = wType;
	m_victim.item_sungma_attr[pos][iattrpos].sValue = sValue;
}

void CPythonExchange::SetItemApplyRandomToSelf(int pos, int iattrpos, uint16_t wType, int16_t sValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_sungma_attr[pos][iattrpos].wType = wType;
	m_self.item_sungma_attr[pos][iattrpos].sValue = sValue;
}

void CPythonExchange::GetItemApplyRandomFromTarget(uint8_t pos, int iAttrPos, uint16_t* pwType, int16_t* psValue) const
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	*pwType = m_victim.item_sungma_attr[pos][iAttrPos].wType;
	*psValue = m_victim.item_sungma_attr[pos][iAttrPos].sValue;
}

void CPythonExchange::GetItemApplyRandomFromSelf(uint8_t pos, int iAttrPos, uint16_t* pwType, int16_t* psValue) const
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	*pwType = m_self.item_sungma_attr[pos][iAttrPos].wType;
	*psValue = m_self.item_sungma_attr[pos][iAttrPos].sValue;
}

void CPythonExchange::SetRandomDefaultAttrs(int pos, int iRandomValuePos, uint16_t iRandomValue, bool bSelf)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	if (bSelf)
		m_victim.item_random_values[pos][iRandomValuePos] = iRandomValue;
	else
		m_self.item_random_values[pos][iRandomValuePos] = iRandomValue;
}

uint16_t CPythonExchange::GetRandomDefaultAttrs(uint8_t iPos, int iRandomValuePos, bool bSelf) const
{
	if (iPos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	uint16_t wRandomValue = bSelf ? m_self.item_random_values[iPos][iRandomValuePos] : m_victim.item_random_values[iPos][iRandomValuePos];
	return wRandomValue;
}
#endif

#ifdef ENABLE_SET_ITEM
void CPythonExchange::SetItemSetValue(int iPos, uint8_t bSetItem, bool bSelf)
{
	if (iPos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	if (bSelf)
		m_self.set_value[iPos] = bSetItem;
	else
		m_victim.set_value[iPos] = bSetItem;
}

uint32_t CPythonExchange::GetItemSetValueFromTarget(uint8_t pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.set_value[pos];
}

uint32_t CPythonExchange::GetItemSetValueFromSelf(uint8_t pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.set_value[pos];
}
#endif

void CPythonExchange::SetItemToTarget(uint32_t pos, uint32_t vnum, uint8_t count)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_vnum[pos] = vnum;
	m_victim.item_count[pos] = count;
}

void CPythonExchange::SetItemToSelf(uint32_t pos, uint32_t vnum, uint8_t count)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_vnum[pos] = vnum;
	m_self.item_count[pos] = count;
}

void CPythonExchange::SetItemMetinSocketToTarget(int pos, int imetinpos, uint32_t vnum)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_metin[pos][imetinpos] = vnum;
}

void CPythonExchange::SetItemMetinSocketToSelf(int pos, int imetinpos, uint32_t vnum)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_metin[pos][imetinpos] = vnum;
}

void CPythonExchange::SetItemAttributeToTarget(int pos, int iattrpos, uint16_t wType, int16_t sValue)	//@fixme436
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_attr[pos][iattrpos].wType = wType;
	m_victim.item_attr[pos][iattrpos].sValue = sValue;
}

void CPythonExchange::SetItemAttributeToSelf(int pos, int iattrpos, uint16_t wType, int16_t sValue)	//@fixme436
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_attr[pos][iattrpos].wType = wType;
	m_self.item_attr[pos][iattrpos].sValue = sValue;
}

void CPythonExchange::DelItemOfTarget(uint8_t pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_vnum[pos] = 0;
	m_victim.item_count[pos] = 0;
}

void CPythonExchange::DelItemOfSelf(uint8_t pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_vnum[pos] = 0;
	m_self.item_count[pos] = 0;
}

uint32_t CPythonExchange::GetItemVnumFromTarget(uint8_t pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.item_vnum[pos];
}

uint32_t CPythonExchange::GetItemVnumFromSelf(uint8_t pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.item_vnum[pos];
}

uint8_t CPythonExchange::GetItemCountFromTarget(uint8_t pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.item_count[pos];
}

uint8_t CPythonExchange::GetItemCountFromSelf(uint8_t pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.item_count[pos];
}

uint32_t CPythonExchange::GetItemMetinSocketFromTarget(uint8_t pos, int iMetinSocketPos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.item_metin[pos][iMetinSocketPos];
}

uint32_t CPythonExchange::GetItemMetinSocketFromSelf(uint8_t pos, int iMetinSocketPos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.item_metin[pos][iMetinSocketPos];
}

void CPythonExchange::GetItemAttributeFromTarget(uint8_t pos, int iAttrPos, uint16_t* pwType, int16_t * psValue) const	//@fixme436
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	*pwType = m_victim.item_attr[pos][iAttrPos].wType;
	*psValue = m_victim.item_attr[pos][iAttrPos].sValue;
}

void CPythonExchange::GetItemAttributeFromSelf(uint8_t pos, int iAttrPos, uint16_t * pwType, int16_t * psValue) const	//@fixme436
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	*pwType = m_self.item_attr[pos][iAttrPos].wType;
	*psValue = m_self.item_attr[pos][iAttrPos].sValue;
}

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
void CPythonExchange::SetItemChangeLookVnum(int iPos, uint32_t dwTransmutation, bool bSelf)
{
	if (iPos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	if (bSelf)
		m_self.dwTransmutationVnum[iPos] = dwTransmutation;
	else
		m_victim.dwTransmutationVnum[iPos] = dwTransmutation;
}

uint32_t CPythonExchange::GetItemChangeLookVnum(int iPos, bool bSelf)
{
	if (iPos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	uint32_t dwTransmutation = bSelf == true ? m_self.dwTransmutationVnum[iPos] : m_victim.dwTransmutationVnum[iPos];
	return dwTransmutation;
}

uint32_t CPythonExchange::GetChangeLookVnumFromSelf(const uint8_t pos) const
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.dwTransmutationVnum[pos];
}

uint32_t CPythonExchange::GetChangeLookVnumFromTarget(const uint8_t pos) const
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.dwTransmutationVnum[pos];
}
#endif

void CPythonExchange::SetAcceptToTarget(uint8_t Accept)
{
	m_victim.accept = Accept ? true : false;
}

void CPythonExchange::SetAcceptToSelf(uint8_t Accept)
{
	m_self.accept = Accept ? true : false;
}

bool CPythonExchange::GetAcceptFromTarget() const
{
	return m_victim.accept ? true : false;
}

bool CPythonExchange::GetAcceptFromSelf() const
{
	return m_self.accept ? true : false;
}

bool CPythonExchange::GetElkMode() const
{
	return m_elk_mode;
}

void CPythonExchange::SetElkMode(bool value)
{
	m_elk_mode = value;
}

void CPythonExchange::Start()
{
	m_isTrading = true;
}

void CPythonExchange::End()
{
	m_isTrading = false;
}

bool CPythonExchange::isTrading() const
{
	return m_isTrading;
}

void CPythonExchange::Clear()
{
	m_self = {};
	m_victim = {};
}

CPythonExchange::CPythonExchange()
{
	Clear();
	m_isTrading = false;
	m_elk_mode = false;
	// Clear로 옴겨놓으면 안됨.
	// trade_start 페킷이 오면 Clear를 실행하는데
	// m_elk_mode는 클리어 되선 안됨.;
}
CPythonExchange::~CPythonExchange() = default;
