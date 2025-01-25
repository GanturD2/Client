#ifndef __METIN2_BUFF_ON_ATTRIBUTES_H
#define __METIN2_BUFF_ON_ATTRIBUTES_H

class CHARACTER;

class CBuffOnAttributes
{
public:
	CBuffOnAttributes(LPCHARACTER pOwner, uint16_t m_point_type, std::vector <uint8_t>* vec_buff_targets);	//@fixme532
	~CBuffOnAttributes();

	void RemoveBuffFromItem(LPITEM pItem);
	void AddBuffFromItem(LPITEM pItem);
	void ChangeBuffValue(uint8_t bNewValue);

	bool On(uint8_t bValue);
	void Off();

	void Initialize();

private:
	LPCHARACTER m_pBuffOwner;
	uint16_t m_wPointType;	//@fixme532
	uint8_t m_bBuffValue;
	std::vector <uint8_t>* m_p_vec_buff_wear_targets;

	// apply_type, apply_value
	typedef std::map <uint8_t, int> TMapAttr;
	TMapAttr m_map_additional_attrs;
};

#endif
