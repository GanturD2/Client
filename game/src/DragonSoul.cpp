#include "stdafx.h"
#include "constants.h"
#include "item.h"
#include "item_manager.h"
#include "unique_item.h"
#include "packet.h"
#include "desc.h"
#include "char.h"
#include "dragon_soul_table.h"
#include "log.h"
#include "DragonSoul.h"
#include <boost/lexical_cast.hpp>

typedef std::vector <std::string> TTokenVector;

int Gamble(std::vector<float>& vec_probs) noexcept
{
	float range = 0.f;
	for (size_t i = 0; i < vec_probs.size(); i++)
	{
		range += vec_probs[i];
	}
	const float fProb = fnumber(0.f, range);
	float sum = 0.f;
	for (size_t idx = 0; idx < vec_probs.size(); idx++)
	{
		sum += vec_probs[idx];
		if (sum >= fProb)
			return idx;
	}
	return -1;
}

// Receive the weight table (prob_lst), select random_set.size() indexes and return random_set
bool MakeDistinctRandomNumberSet(std::list <float> prob_lst, OUT std::vector<int>& random_set)
{
	const int size = prob_lst.size();
	const int n = random_set.size();
	if (size < n)
		return false;

	std::vector <int> select_bit(size, 0);
	for (int i = 0; i < n; i++)
	{
		float range = 0.f;
		for (std::list <float>::iterator it = prob_lst.begin(); it != prob_lst.end(); ++it)	//@fixme541
		{
			range += *it;
		}
		const float r = fnumber(0.f, range);
		float sum = 0.f;
		int idx = 0;
		for (std::list <float>::iterator it = prob_lst.begin(); it != prob_lst.end(); ++it)	//@fixme541
		{
			while (select_bit[idx++]);

			sum += *it;
			if (sum >= r)
			{
				select_bit[idx - 1] = 1;
				random_set[i] = idx - 1;
				prob_lst.erase(it);
				break;
			}
		}
	}
	return true;
}

/* Comment on Dragon Spirit Stone Vnum
* If ITEM VNUM is from 100,000 positions, FEDCBA
* FE: Dragon Soul Stone type. D: grade
* C: Step B: Enhance
* A: Extra numbers...
*/

uint8_t GetType(uint32_t dwVnum) noexcept
{
	return (dwVnum / 10000);
}

uint8_t GetGradeIdx(uint32_t dwVnum) noexcept
{
	return (dwVnum / 1000) % 10;
}

uint8_t GetStepIdx(uint32_t dwVnum) noexcept
{
	return (dwVnum / 100) % 10;
}

uint8_t GetStrengthIdx(uint32_t dwVnum) noexcept
{
	return (dwVnum / 10) % 10;
}

bool DSManager::ReadDragonSoulTableFile(const char* c_pszFileName)
{
	m_pTable = new DragonSoulTable();
	return m_pTable->ReadDragonSoulTableFile(c_pszFileName);
}

void DSManager::GetDragonSoulInfo(uint32_t dwVnum, uint8_t& bType, uint8_t& bGrade, uint8_t& bStep, uint8_t& bStrength) const
{
	bType = GetType(dwVnum);
	bGrade = GetGradeIdx(dwVnum);
	bStep = GetStepIdx(dwVnum);
	bStrength = GetStrengthIdx(dwVnum);
}

bool DSManager::IsValidCellForThisItem(const LPITEM pItem, const TItemPos& Cell) const
{
	if (nullptr == pItem)
		return false;

	const uint16_t wBaseCell = GetBasePosition(pItem);
	if (WORD_MAX == wBaseCell)
		return false;

	if (Cell.window_type != DRAGON_SOUL_INVENTORY
		|| (Cell.cell < wBaseCell || Cell.cell >= wBaseCell + DRAGON_SOUL_BOX_SIZE))
	{
		return false;
	}
	else
		return true;
}

uint16_t DSManager::GetBasePosition(const LPITEM pItem) const
{
	if (nullptr == pItem)
		return WORD_MAX;

	uint8_t type, grade_idx, step_idx, strength_idx;
	GetDragonSoulInfo(pItem->GetVnum(), type, grade_idx, step_idx, strength_idx);

	const uint8_t col_type = pItem->GetSubType();
	const uint8_t row_type = grade_idx;
	if (row_type > DRAGON_SOUL_GRADE_MAX)
		return WORD_MAX;

#ifdef ENABLE_DS_GRADE_MYTH
	return col_type * DRAGON_SOUL_GRADE_MAX * DRAGON_SOUL_BOX_SIZE + row_type * DRAGON_SOUL_BOX_SIZE; //@fixme484
#else
	return col_type * DRAGON_SOUL_STEP_MAX * DRAGON_SOUL_BOX_SIZE + row_type * DRAGON_SOUL_BOX_SIZE;
#endif
}

bool DSManager::RefreshItemAttributes(LPITEM pDS)
{
	if (!pDS)
		return false;

	if (!pDS->IsDragonSoul())
	{
		sys_err("This item(ID : %d) is not DragonSoul.", pDS->GetID());
		return false;
	}

	uint8_t ds_type, grade_idx, step_idx, strength_idx;
	GetDragonSoulInfo(pDS->GetVnum(), ds_type, grade_idx, step_idx, strength_idx);

	DragonSoulTable::TVecApplys vec_basic_applys;
	DragonSoulTable::TVecApplys vec_addtional_applys;

	if (!m_pTable->GetBasicApplys(ds_type, vec_basic_applys))
	{
		sys_err("There is no BasicApply about %d type dragon soul.", ds_type);
		return false;
	}

	if (!m_pTable->GetAdditionalApplys(ds_type, vec_addtional_applys))
	{
		sys_err("There is no AdditionalApply about %d type dragon soul.", ds_type);
		return false;
	}

	// add_min and add_max are read as dummy.
	int basic_apply_num, add_min, add_max;
	if (!m_pTable->GetApplyNumSettings(ds_type, grade_idx, basic_apply_num, add_min, add_max))
	{
		sys_err("In ApplyNumSettings, INVALID VALUES Group type(%d), GRADE idx(%d)", ds_type, grade_idx);
		return false;
	}

	float fWeight = 0.f;
	if (!m_pTable->GetWeight(ds_type, grade_idx, step_idx, strength_idx, fWeight))
	{
		return false;
	}
	fWeight /= 100.f;

	const int n = MIN(basic_apply_num, vec_basic_applys.size());
	for (int i = 0; i < n; i++)
	{
		const SApply& basic_apply = vec_basic_applys[i];
		const uint16_t wType = basic_apply.apply_type; // @fixme-WL001
		const int16_t sValue = (int16_t)(ceil((float)basic_apply.apply_value * fWeight - 0.01f));

		pDS->SetForceAttribute(i, wType, sValue);
	}

	for (int i = DRAGON_SOUL_ADDITIONAL_ATTR_START_IDX; i < ITEM_ATTRIBUTE_MAX_NUM; i++)
	{
		const uint16_t wType = pDS->GetAttributeType(i); // @fixme-WL001
		int16_t sValue = 0;
		if (APPLY_NONE == wType)
			continue;
		for (size_t j = 0; j < vec_addtional_applys.size(); j++)
		{
			if (vec_addtional_applys[j].apply_type == wType)
			{
				sValue = vec_addtional_applys[j].apply_value;
				break;
			}
		}
		pDS->SetForceAttribute(i, wType, (int16_t)(ceil((float)sValue * fWeight - 0.01f)));
	}
	return true;
}

bool DSManager::PutAttributes(LPITEM pDS)
{
	if (!pDS)
		return false;

	if (!pDS->IsDragonSoul())
	{
		sys_err("This item(ID : %d) is not DragonSoul.", pDS->GetID());
		return false;
	}

	uint8_t ds_type, grade_idx, step_idx, strength_idx;
	GetDragonSoulInfo(pDS->GetVnum(), ds_type, grade_idx, step_idx, strength_idx);

	DragonSoulTable::TVecApplys vec_basic_applys;
	DragonSoulTable::TVecApplys vec_addtional_applys;

	if (!m_pTable->GetBasicApplys(ds_type, vec_basic_applys))
	{
		sys_err("There is no BasicApply about %d type dragon soul.", ds_type);
		return false;
	}
	if (!m_pTable->GetAdditionalApplys(ds_type, vec_addtional_applys))
	{
		sys_err("There is no AdditionalApply about %d type dragon soul.", ds_type);
		return false;
	}

	int basic_apply_num, add_min, add_max;
	if (!m_pTable->GetApplyNumSettings(ds_type, grade_idx, basic_apply_num, add_min, add_max))
	{
		sys_err("In ApplyNumSettings, INVALID VALUES Group type(%d), GRADE idx(%d)", ds_type, grade_idx);
		return false;
	}

	float fWeight = 0.f;
	if (!m_pTable->GetWeight(ds_type, grade_idx, step_idx, strength_idx, fWeight))
	{
		return false;
	}
	fWeight /= 100.f;

	const int n = MIN(basic_apply_num, vec_basic_applys.size());
	for (int i = 0; i < n; i++)
	{
		const SApply& basic_apply = vec_basic_applys[i];
		const uint16_t wType = basic_apply.apply_type; // @fixme-WL001
		const int16_t sValue = (int16_t)(ceil((float)basic_apply.apply_value * fWeight - 0.01f));

		pDS->SetForceAttribute(i, wType, sValue);
	}

	const uint8_t additional_attr_num = MIN(number(add_min, add_max), 3);

	std::vector <int> random_set;
	if (additional_attr_num > 0)
	{
		random_set.resize(additional_attr_num);
		std::list <float> list_probs;
		for (size_t i = 0; i < vec_addtional_applys.size(); i++)
		{
			list_probs.emplace_back(vec_addtional_applys[i].prob);
		}
		if (!MakeDistinctRandomNumberSet(list_probs, random_set))
		{
			sys_err("MakeDistinctRandomNumberSet error.");
			return false;
		}

		for (int i = 0; i < additional_attr_num; i++)
		{
			const int r = random_set[i];
			const SApply& additional_attr = vec_addtional_applys[r];
			const uint16_t wType = additional_attr.apply_type; // @fixme-WL001
			const int16_t sValue = (int16_t)(ceil((float)additional_attr.apply_value * fWeight - 0.01f));

			pDS->SetForceAttribute(DRAGON_SOUL_ADDITIONAL_ATTR_START_IDX + i, wType, sValue);
		}
	}

	return true;
}

bool DSManager::DragonSoulItemInitialize(LPITEM pItem)
{
	if (nullptr == pItem || !pItem->IsDragonSoul())
		return false;

	PutAttributes(pItem);

	const int time = DSManager::Instance().GetDuration(pItem);
	if (time > 0)
		pItem->SetSocket(ITEM_SOCKET_REMAIN_SEC, time);
	return true;
}

uint32_t DSManager::MakeDragonSoulVnum(uint8_t bType, uint8_t grade, uint8_t step, uint8_t refine) noexcept
{
	return bType * 10000 + grade * 1000 + step * 100 + refine * 10;
}

int DSManager::GetDuration(const LPITEM pItem) const
{
	if (pItem)
		return pItem->GetDuration();

	return 0;
}

// Function to extract dragon heart by receiving dragon spirit
bool DSManager::ExtractDragonHeart(LPCHARACTER ch, LPITEM pItem, LPITEM pExtractor)
{
	if (nullptr == ch || nullptr == pItem)
		return false;
	if (pItem->IsEquipped())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1091]");
		return false;
	}

	const uint32_t dwVnum = pItem->GetVnum();
	uint8_t ds_type, grade_idx, step_idx, strength_idx;
	GetDragonSoulInfo(dwVnum, ds_type, grade_idx, step_idx, strength_idx);

	int iBonus = 0;

	if (nullptr != pExtractor)
	{
		iBonus = pExtractor->GetValue(0);
	}

	std::vector <float> vec_chargings;
	std::vector <float> vec_probs;

	if (!m_pTable->GetDragonHeartExtValues(ds_type, grade_idx, vec_chargings, vec_probs))
	{
		return false;
	}

	const int idx = Gamble(vec_probs);

	//float sum = 0.f;
	if (-1 == idx)
	{
		sys_err("Gamble is failed. ds_type(%d), grade_idx(%d)", ds_type, grade_idx);
		return false;
	}

	float fCharge = vec_chargings[idx] * (100 + iBonus) / 100.f;
	fCharge = std::clamp(0.f, fCharge, 100.f);

	if (fCharge < FLT_EPSILON)
	{
		pItem->SetCount(pItem->GetCount() - 1);
		if (nullptr != pExtractor)
		{
			pExtractor->SetCount(pExtractor->GetCount() - 1);
		}
		LogManager::Instance().ItemLog(ch, pItem, "DS_HEART_EXTRACT_FAIL", "");

		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1073]");
		return false;
	}
	else
	{
		LPITEM pDH = ITEM_MANAGER::Instance().CreateItem(DRAGON_HEART_VNUM);

		if (nullptr == pDH)
		{
			sys_err("Cannot create DRAGON_HEART(%d).", DRAGON_HEART_VNUM);
			return false;
		}

		pItem->SetCount(pItem->GetCount() - 1);
		if (nullptr != pExtractor)
		{
			pExtractor->SetCount(pExtractor->GetCount() - 1);
		}

		const int iCharge = (int)(fCharge + 0.5f);
		pDH->SetSocket(ITEM_SOCKET_CHARGING_AMOUNT_IDX, iCharge);
		ch->AutoGiveItem(pDH, true);

		std::string s = boost::lexical_cast <std::string> (iCharge);
		s += "%s";
		LogManager::Instance().ItemLog(ch, pItem, "DS_HEART_EXTRACT_SUCCESS", s.c_str());
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1072]");
		return true;
	}
}

// A function that determines success or failure when removing a specific dragon spirit stone from the equipment window and gives a by-product in case of failure.
bool DSManager::PullOut(LPCHARACTER ch, TItemPos DestCell, LPITEM& pItem, LPITEM pExtractor)
{
	if (nullptr == ch || nullptr == pItem)
	{
		sys_err("nullptr POINTER. ch(%p) or pItem(%p)", ch, pItem);
		return false;
	}

	// After checking whether the target position is valid, if not, it searches for a random empty space.
	if (!IsValidCellForThisItem(pItem, DestCell) || ch->GetItem(DestCell)) // @fixme160 (added GetItem check)
	{
		const int iEmptyCell = ch->GetEmptyDragonSoulInventory(pItem);
		if (iEmptyCell < 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1130]");
			return false;
		}
		else
		{
			DestCell.window_type = DRAGON_SOUL_INVENTORY;
			DestCell.cell = iEmptyCell;
		}
	}

	if (!pItem->IsEquipped() || !pItem->RemoveFromCharacter())
		return false;

#ifdef ENABLE_DS_SET
	if (ch->DragonSoul_IsDeckActivated())
	{
		ch->DragonSoul_HandleSetBonus();
		ch->RemoveAffect(NEW_AFFECT_DS_SET);
	}
#endif

	bool bSuccess = false;
	uint32_t dwByProduct = 0;
	int iBonus = 0;
	float fProb = 0.0f;
	float fDice = 0.0f;
	// Determination of success in extracting dragon spirit stone.
	{
		//uint32_t dwVnum = pItem->GetVnum();

		uint8_t ds_type, grade_idx, step_idx, strength_idx;
		GetDragonSoulInfo(pItem->GetVnum(), ds_type, grade_idx, step_idx, strength_idx);

		// If there is no extraction information, let's think that it succeeds unconditionally.
		if (!m_pTable->GetDragonSoulExtValues(ds_type, grade_idx, fProb, dwByProduct))
		{
			pItem->AddToCharacter(ch, DestCell);
			return true;
		}


		if (nullptr != pExtractor)
		{
			iBonus = pExtractor->GetValue(ITEM_VALUE_DRAGON_SOUL_POLL_OUT_BONUS_IDX);
			pExtractor->SetCount(pExtractor->GetCount() - 1);
		}
		fDice = fnumber(0.f, 100.f);
		bSuccess = fDice <= (fProb * (100 + iBonus) / 100.f);

#ifdef ENABLE_REVERSED_FUNCTIONS
		if (iBonus >= 100)
		{
			bSuccess = true;
			fDice = 100.f;
			fProb = 100.f;
		}
#endif
	}

	// Extracting and adding or removing a character's dragon spirit stone. By-products provided.
	{
		char buf[128] = { 0 };

		if (bSuccess)
		{
			if (pExtractor)
			{
				sprintf(buf, "dice(%d) prob(%d + %d) EXTR(VN:%d)", (int)fDice, (int)fProb, iBonus, pExtractor->GetVnum());
			}
			else
			{
				sprintf(buf, "dice(%d) prob(%d)", (int)fDice, (int)fProb);
			}
			LogManager::Instance().ItemLog(ch, pItem, "DS_PULL_OUT_SUCCESS", buf);
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1068]");
			pItem->AddToCharacter(ch, DestCell);
			return true;
		}
		else
		{
			if (pExtractor)
			{
				sprintf(buf, "dice(%d) prob(%d + %d) EXTR(VN:%d) ByProd(VN:%d)", (int)fDice, (int)fProb, iBonus, pExtractor->GetVnum(), dwByProduct);
			}
			else
			{
				sprintf(buf, "dice(%d) prob(%d) ByProd(VNUM:%d)", (int)fDice, (int)fProb, dwByProduct);
			}
			LogManager::Instance().ItemLog(ch, pItem, "DS_PULL_OUT_FAILED", buf);
			M2_DESTROY_ITEM(pItem);
			pItem = nullptr;
			if (dwByProduct)
			{
				LPITEM pByProduct = ch->AutoGiveItem(dwByProduct, true);
				if (pByProduct)
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1074;%s]", pByProduct->GetName());
				else
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1075]");
			}
			else
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1075]");
		}
	}

	return bSuccess;
}

bool DSManager::DoRefineGrade(LPCHARACTER ch, TItemPos(&aItemPoses)[DRAGON_SOUL_REFINE_GRID_SIZE])
{
	if (nullptr == ch)
		return false;

	if (nullptr == aItemPoses)
	{
		return false;
	}

	if (!ch->DragonSoul_RefineWindow_CanRefine())
	{
		sys_err("%s do not activate DragonSoulRefineWindow. But how can he come here?", ch->GetName());
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[SYSTEM ERROR]You cannot upgrade dragon soul without refine window."));
		return false;
	}

	// Use set to remove any duplicate item pointers
	// When sending a strange packet, there may be duplicate TItemPos or wrong TItemPos.
	std::set <LPITEM> set_items;
	for (int i = 0; i < DRAGON_SOUL_REFINE_GRID_SIZE; i++)
	{
		if (aItemPoses[i].IsEquipPosition())
			return false;
		LPITEM pItem = ch->GetItem(aItemPoses[i]);
		if (nullptr != pItem)
		{
			// Items other than Dragon Spirit Stone cannot be in the improvement window.
			if (!pItem->IsDragonSoul())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1076]");
				SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL, TItemPos(pItem->GetWindow(), pItem->GetCell()));

				return false;
			}

			set_items.insert(pItem);
		}
	}

	if (set_items.size() == 0)
	{
		SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL, NPOS);
		return false;
	}

	const int count = set_items.size();
	int need_count = 0;
	int fee = 0;
	std::vector <float> vec_probs;
	//float prob_sum;

	uint8_t ds_type = 0, grade_idx = 0, step_idx = 0, strength_idx = 0;
	int result_grade = 0;

	// Use the first one as the standard for reinforcement.
	std::set <LPITEM>::iterator it = set_items.begin();
	{
		LPITEM pItem = *it;

		GetDragonSoulInfo(pItem->GetVnum(), ds_type, grade_idx, step_idx, strength_idx);

		if (!m_pTable->GetRefineGradeValues(ds_type, grade_idx, need_count, fee, vec_probs))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1077]");
			SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL, TItemPos(pItem->GetWindow(), pItem->GetCell()));

			return false;
		}
	}
	while (++it != set_items.end())
	{
		LPITEM pItem = *it;

		// Because items equipped in the cla ui cannot be uploaded to the upgrade window,
		// No separate notification processing.
		if (pItem->IsEquipped())
		{
			return false;
		}

		if (ds_type != GetType(pItem->GetVnum()) || grade_idx != GetGradeIdx(pItem->GetVnum()))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1076]");
			SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL, TItemPos(pItem->GetWindow(), pItem->GetCell()));

			return false;
		}
	}

	// Because the client checks the number once, if count != need_count, it is likely an invalid client.
	if (count != need_count)
	{
		sys_err("Possiblity of invalid client. Name %s", ch->GetName());
		uint8_t bSubHeader = count < need_count ? DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL : DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL;
		SendRefineResultPacket(ch, bSubHeader, NPOS);
		return false;
	}

	if (ch->GetGold() < fee)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;67]");
		SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY, NPOS);
		return false;
	}

	if (-1 == (result_grade = Gamble(vec_probs)))
	{
		sys_err("Gamble failed. See RefineGardeTables' probabilities");
		return false;
	}

	LPITEM pResultItem = ITEM_MANAGER::Instance().CreateItem(MakeDragonSoulVnum(ds_type, (uint8_t)result_grade, 0, 0));

	if (nullptr == pResultItem)
	{
		sys_err("INVALID DRAGON SOUL(%d)", MakeDragonSoulVnum(ds_type, (uint8_t)result_grade, 0, 0));
		return false;
	}

	ch->PointChange(POINT_GOLD, -fee);
	int left_count = need_count;

	for (std::set <LPITEM>::iterator it = set_items.begin(); it != set_items.end(); it++)
	{
		LPITEM pItem = *it;
		const int n = pItem->GetCount();
		if (left_count > n)
		{
			pItem->RemoveFromCharacter();
			M2_DESTROY_ITEM(pItem);
			left_count -= n;
		}
		else
		{
			pItem->SetCount(n - left_count);
		}
	}

	ch->AutoGiveItem(pResultItem, true);

	if (result_grade > grade_idx)
	{
		char buf[128];
		sprintf(buf, "GRADE : %d -> %d", grade_idx, result_grade);
		LogManager::Instance().ItemLog(ch, pResultItem, "DS_GRADE_REFINE_SUCCESS", buf);
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1082]");
		SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_SUCCEED, TItemPos(pResultItem->GetWindow(), pResultItem->GetCell()));
		return true;
	}
	else
	{
		char buf[128];
		sprintf(buf, "GRADE : %d -> %d", grade_idx, result_grade);
		LogManager::Instance().ItemLog(ch, pResultItem, "DS_GRADE_REFINE_FAIL", buf);
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1079]");
		SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL, TItemPos(pResultItem->GetWindow(), pResultItem->GetCell()));
		return false;
	}
}

bool DSManager::DoRefineStep(LPCHARACTER ch, TItemPos(&aItemPoses)[DRAGON_SOUL_REFINE_GRID_SIZE])
{
	if (nullptr == ch)
		return false;
	if (nullptr == aItemPoses)
	{
		return false;
	}

	if (!ch->DragonSoul_RefineWindow_CanRefine())
	{
		sys_err("%s do not activate DragonSoulRefineWindow. But how can he come here?", ch->GetName());
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[SYSTEM ERROR]You cannot use dragon soul refine window."));
		return false;
	}

	// Use set to remove any duplicate item pointers
	// When sending a strange packet, there may be duplicate TItemPos or wrong TItemPos.
	std::set <LPITEM> set_items;
	for (int i = 0; i < DRAGON_SOUL_REFINE_GRID_SIZE; i++)
	{
		LPITEM pItem = ch->GetItem(aItemPoses[i]);
		if (nullptr != pItem)
		{
			// Items other than Dragon Spirit Stone cannot be in the improvement window.
			if (!pItem->IsDragonSoul())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1084]");
				SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL, TItemPos(pItem->GetWindow(), pItem->GetCell()));
				return false;
			}
			set_items.insert(pItem);
		}
	}

	if (set_items.size() == 0)
	{
		SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL, NPOS);
		return false;
	}

	std::string stGroupName;
	const int count = set_items.size();
	int need_count = 0;
	int fee = 0;
	std::vector <float> vec_probs;

	uint8_t ds_type = 0, grade_idx = 0, step_idx = 0, strength_idx = 0;
	int result_step = 0;

	// Use the first one as the standard for reinforcement.
	std::set <LPITEM>::iterator it = set_items.begin();
	{
		LPITEM pItem = *it;
		GetDragonSoulInfo(pItem->GetVnum(), ds_type, grade_idx, step_idx, strength_idx);

		if (!m_pTable->GetRefineStepValues(ds_type, step_idx, need_count, fee, vec_probs))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1080]");
			SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL, TItemPos(pItem->GetWindow(), pItem->GetCell()));
			return false;
		}
	}

	while (++it != set_items.end())
	{
		LPITEM pItem = *it;
		// Because items equipped in the cla ui cannot be uploaded to the upgrade window,
		// No separate notification processing.
		if (pItem->IsEquipped())
		{
			return false;
		}
		if (ds_type != GetType(pItem->GetVnum()) || grade_idx != GetGradeIdx(pItem->GetVnum()) || step_idx != GetStepIdx(pItem->GetVnum()))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1084]");
			SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL, TItemPos(pItem->GetWindow(), pItem->GetCell()));
			return false;
		}
	}

	// Because the client checks the number once, if count != need_count, it is likely an invalid client.
	if (count != need_count)
	{
		sys_err("Possiblity of invalid client. Name %s", ch->GetName());
		const uint8_t bSubHeader = count < need_count ? DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL : DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL;
		SendRefineResultPacket(ch, bSubHeader, NPOS);
		return false;
	}

	if (ch->GetGold() < fee)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;67]");
		SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY, NPOS);
		return false;
	}

	//float sum = 0.f;

	if (-1 == (result_step = Gamble(vec_probs)))
	{
		sys_err("Gamble failed. See RefineStepTables' probabilities");
		return false;
	}

	LPITEM pResultItem = ITEM_MANAGER::Instance().CreateItem(MakeDragonSoulVnum(ds_type, grade_idx, (uint8_t)result_step, 0));

	if (nullptr == pResultItem)
	{
		sys_err("INVALID DRAGON SOUL(%d)", MakeDragonSoulVnum(ds_type, grade_idx, (uint8_t)result_step, 0));
		return false;
	}

	ch->PointChange(POINT_GOLD, -fee);
	int left_count = need_count;
	for (std::set <LPITEM>::iterator it = set_items.begin(); it != set_items.end(); it++)
	{
		LPITEM pItem = *it;
		const int n = pItem->GetCount();
		if (left_count > n)
		{
			pItem->RemoveFromCharacter();
			M2_DESTROY_ITEM(pItem);
			left_count -= n;
		}
		else
		{
			pItem->SetCount(n - left_count);
		}
	}

	ch->AutoGiveItem(pResultItem, true);
	if (result_step > step_idx)
	{
		char buf[128];
		sprintf(buf, "STEP : %d -> %d", step_idx, result_step);
		LogManager::Instance().ItemLog(ch, pResultItem, "DS_STEP_REFINE_SUCCESS", buf);
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1078]");
		SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_SUCCEED, TItemPos(pResultItem->GetWindow(), pResultItem->GetCell()));
		return true;
	}
	else
	{
		char buf[128];
		sprintf(buf, "STEP : %d -> %d", step_idx, result_step);
		LogManager::Instance().ItemLog(ch, pResultItem, "DS_STEP_REFINE_FAIL", buf);
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1083]");
		SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL, TItemPos(pResultItem->GetWindow(), pResultItem->GetCell()));
		return false;
	}
}

bool IsDragonSoulRefineMaterial(LPITEM pItem)
{
	if (!pItem)
		return false;

	if (pItem->GetType() != ITEM_MATERIAL)
		return false;

	return (pItem->GetSubType() == MATERIAL_DS_REFINE_NORMAL ||
		pItem->GetSubType() == MATERIAL_DS_REFINE_BLESSED ||
		pItem->GetSubType() == MATERIAL_DS_REFINE_HOLLY
#ifdef ENABLE_DS_CHANGE_ATTR
		|| pItem->GetSubType() == MATERIAL_DS_CHANGE_ATTR
#endif
		);
}

bool DSManager::DoRefineStrength(LPCHARACTER ch, TItemPos(&aItemPoses)[DRAGON_SOUL_REFINE_GRID_SIZE])
{
	if (nullptr == ch)
		return false;
	if (nullptr == aItemPoses)
	{
		return false;
	}

	if (!ch->DragonSoul_RefineWindow_CanRefine())
	{
		sys_err("%s do not activate DragonSoulRefineWindow. But how can he come here?", ch->GetName());
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[SYSTEM ERROR]You cannot use dragon soul refine window."));
		return false;
	}

	// Use set to remove any duplicate item pointers
	// When sending a strange packet, there may be duplicate TItemPos or wrong TItemPos.
	std::set <LPITEM> set_items;
	for (int i = 0; i < DRAGON_SOUL_REFINE_GRID_SIZE; i++)
	{
		LPITEM pItem = ch->GetItem(aItemPoses[i]);
		if (pItem)
		{
			set_items.insert(pItem);
		}
	}
	if (set_items.size() == 0)
	{
		return false;
	}

	int fee = 0;

	LPITEM pRefineStone = nullptr;
	LPITEM pDragonSoul = nullptr;
	for (std::set <LPITEM>::iterator it = set_items.begin(); it != set_items.end(); it++)
	{
		LPITEM pItem = *it;
		// Because items equipped in the cla ui cannot be uploaded to the upgrade window,
		// No separate notification processing.
		if (pItem->IsEquipped())
		{
			return false;
		}

		// Only Dragon Spirit Stone and Enhancement Stone can be in the upgrade window.
		// And there should be only one.
		if (pItem->IsDragonSoul())
		{
			if (pDragonSoul != nullptr)
			{
				SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL, TItemPos(pItem->GetWindow(), pItem->GetCell()));
				return false;
			}
			pDragonSoul = pItem;
		}
		else if (IsDragonSoulRefineMaterial(pItem))
		{
			if (pRefineStone != nullptr)
			{
				SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL, TItemPos(pItem->GetWindow(), pItem->GetCell()));
				return false;
			}
			pRefineStone = pItem;
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1076]");
			SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL, TItemPos(pItem->GetWindow(), pItem->GetCell()));
			return false;
		}
	}

	uint8_t bType = 0, bGrade = 0, bStep = 0, bStrength = 0;

	if (!pDragonSoul || !pRefineStone)
	{
		SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL, NPOS);

		return false;
	}

	if (nullptr != pDragonSoul)
	{
		GetDragonSoulInfo(pDragonSoul->GetVnum(), bType, bGrade, bStep, bStrength);

		float fWeight = 0.f;
		// Dragon soul stone that cannot be strengthened without weight value
		if (!m_pTable->GetWeight(bType, bGrade, bStep, bStrength + 1, fWeight))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1085]");
			SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_MAX_REFINE, TItemPos(pDragonSoul->GetWindow(), pDragonSoul->GetCell()));
			return false;
		}
		// If the weight is 0 when reinforced, it should not be reinforced any more.
		if (fWeight < FLT_EPSILON)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1085]");
			SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_MAX_REFINE, TItemPos(pDragonSoul->GetWindow(), pDragonSoul->GetCell()));
			return false;
		}
	}

	float fProb;
	if (!m_pTable->GetRefineStrengthValues(bType, pRefineStone->GetSubType(), bStrength, fee, fProb))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1085]");
		SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL, TItemPos(pDragonSoul->GetWindow(), pDragonSoul->GetCell()));

		return false;
	}

	if (ch->GetGold() < fee)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;67]");
		SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY, NPOS);
		return false;
	}

	ch->PointChange(POINT_GOLD, -fee);
	LPITEM pResult = nullptr;
	uint8_t bSubHeader;

	if (fnumber(0.f, 100.f) <= fProb)
	{
		pResult = ITEM_MANAGER::Instance().CreateItem(MakeDragonSoulVnum(bType, bGrade, bStep, bStrength + 1));
		if (nullptr == pResult)
		{
			sys_err("INVALID DRAGON SOUL(%d)", MakeDragonSoulVnum(bType, bGrade, bStep, bStrength + 1));
			return false;
		}
		pDragonSoul->RemoveFromCharacter();

		pDragonSoul->CopyAttributeTo(pResult);
		RefreshItemAttributes(pResult);

		pDragonSoul->SetCount(pDragonSoul->GetCount() - 1);
		pRefineStone->SetCount(pRefineStone->GetCount() - 1);

		char buf[128];
		sprintf(buf, "STRENGTH : %d -> %d", bStrength, bStrength + 1);
		LogManager::Instance().ItemLog(ch, pDragonSoul, "DS_STRENGTH_REFINE_SUCCESS", buf);
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1087]");
		ch->AutoGiveItem(pResult, true);
		bSubHeader = DS_SUB_HEADER_REFINE_SUCCEED;
	}
	else
	{
		if (bStrength != 0)
		{
			pResult = ITEM_MANAGER::Instance().CreateItem(MakeDragonSoulVnum(bType, bGrade, bStep, bStrength - 1));
			if (nullptr == pResult)
			{
				sys_err("INVALID DRAGON SOUL(%d)", MakeDragonSoulVnum(bType, bGrade, bStep, bStrength - 1));
				return false;
			}
			pDragonSoul->CopyAttributeTo(pResult);
			RefreshItemAttributes(pResult);
		}
		bSubHeader = DS_SUB_HEADER_REFINE_FAIL;

		char buf[128];
		sprintf(buf, "STRENGTH : %d -> %d", bStrength, bStrength - 1);
		// Strengthening may be broken upon failure, leaving a log based on the original item.
		LogManager::Instance().ItemLog(ch, pDragonSoul, "DS_STRENGTH_REFINE_FAIL", buf);

		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1088]");
		pDragonSoul->SetCount(pDragonSoul->GetCount() - 1);
		pRefineStone->SetCount(pRefineStone->GetCount() - 1);
		if (nullptr != pResult)
			ch->AutoGiveItem(pResult, true);

	}

	SendRefineResultPacket(ch, bSubHeader, nullptr == pResult ? NPOS : TItemPos(pResult->GetWindow(), pResult->GetCell()));

	return true;
}

#ifdef ENABLE_DS_CHANGE_ATTR
bool DSManager::DoChangeAttr(LPCHARACTER ch, TItemPos(&aItemPoses)[DRAGON_SOUL_REFINE_GRID_SIZE])
{
	if (nullptr == ch)
		return false;

	if (nullptr == aItemPoses)
		return false;

	if (!ch->DragonSoul_RefineWindow_CanRefine())
	{
		sys_err("%s do not activate DragonSoulRefineWindow. But how can he come here?", ch->GetName());
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[SYSTEM ERROR]You cannot use dragon soul refine window."));
		return false;
	}

	std::set <LPITEM> set_items;
	for (int i = 0; i < DRAGON_SOUL_REFINE_GRID_SIZE; i++)
	{
		LPITEM pItem = ch->GetItem(aItemPoses[i]);
		if (pItem)
			set_items.insert(pItem);
	}

	if (set_items.size() == 0)
	{
		SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL, NPOS);
		return false;
	}

	LPITEM pDragonSoul = nullptr;
	LPITEM pMaterial = nullptr;
	for (std::set <LPITEM>::iterator it = set_items.begin(); it != set_items.end(); it++)
	{
		LPITEM pItem = *it;
		if (pItem->IsEquipped())
			return false;

		if (pItem->IsDragonSoul())
		{
			if (pDragonSoul != nullptr)
			{
				SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL, TItemPos(pItem->GetWindow(), pItem->GetCell()));
				return false;
			}
			pDragonSoul = pItem;
		}
		else if (IsDragonSoulRefineMaterial(pItem))
		{
			if (pMaterial != nullptr)
			{
				SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL, TItemPos(pItem->GetWindow(), pItem->GetCell()));
				return false;
			}
			pMaterial = pItem;
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1076]");
			SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL, TItemPos(pItem->GetWindow(), pItem->GetCell()));
			return false;
		}
	}

	if (!pDragonSoul || !pMaterial)
	{
		SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL, NPOS);
		return false;
	}

	if (pDragonSoul->IsDragonSoul())
	{
		const uint8_t needFireCountList[] = { 1, 3, 9, 27, 81 };
		uint8_t bNeedFire = -1;

		const uint32_t dwDSStep = pDragonSoul->GetVnum() % 1000 / 100;
		const uint32_t dwDSGrade = pDragonSoul->GetVnum() % 10000 / 1000;

		if (dwDSGrade != 5)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You only can change the bonus of the mythical stones."));
			SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL, NPOS);
			return false;
		}

		bNeedFire = needFireCountList[dwDSStep];
		if (bNeedFire != -1)
		{
			if (DSManager::Instance().IsActiveDragonSoul(pDragonSoul) == true)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't change the bonus of an alchemy in use."));
				SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL, NPOS);
				return false;
			}

			const int fee = pMaterial->GetValue(0);
			if (ch->GetGold() < fee)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;67]");
				SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY, NPOS);
				return false;
			}

			// check material count
			if (pMaterial->GetCount() < bNeedFire)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You don't have enough %s. You need at least %d to change the bonus."), pMaterial->GetName(), bNeedFire);
				SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL, NPOS);
				return false;
			}

			// clear attributes
			for (uint8_t i = 0; i < 6; i++)
				pDragonSoul->SetForceAttribute(i, 0, 0);

			// put attributes
			const bool ret = DSManager::Instance().PutAttributes(pDragonSoul);
			if (!ret)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("The Dragon Soul Stone bonus hasn't been changed."));
				SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL, NPOS);
				return false;
			}

			pMaterial->SetCount(pMaterial->GetCount() - bNeedFire);
			ch->PointChange(POINT_GOLD, -fee);

			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1420]");
			SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_SUCCEED, TItemPos(pDragonSoul->GetWindow(), pDragonSoul->GetCell()));
		}
	}

	return true;
}
#endif

void DSManager::SendRefineResultPacket(LPCHARACTER ch, uint8_t bSubHeader, const TItemPos& pos)
{
	if (!ch)
		return;

	TPacketGCDragonSoulRefine pack;
	pack.bSubType = bSubHeader;

	if (pos.IsValidItemPosition())
		pack.Pos = pos;

	LPDESC d = ch->GetDesc();
	if (d)
		d->Packet(&pack, sizeof(pack));
}

int DSManager::LeftTime(LPITEM pItem) const
{
	if (pItem == nullptr)
		return false;

	// For now, only the timer based on wear dragon soul stone will not disappear even if time runs out.
	if (pItem->GetProto()->cLimitTimerBasedOnWearIndex >= 0)
	{
		return pItem->GetSocket(ITEM_SOCKET_REMAIN_SEC);
	}
	// Other limit types of dragon spirits disappear when time goes by, so the item that came here is judged to have time left.
	else
	{
		return INT_MAX;
	}
}

bool DSManager::IsTimeLeftDragonSoul(LPITEM pItem) const
{
	if (pItem == nullptr)
		return false;

	// For now, only the timer based on wear dragon soul stone will not disappear even if time runs out.
	if (pItem->GetProto()->cLimitTimerBasedOnWearIndex >= 0)
	{
		return pItem->GetSocket(ITEM_SOCKET_REMAIN_SEC) > 0;
	}
	// Other limit types of dragon spirits disappear when time goes by, so the item that came here is judged to have time left.
	else
	{
		return true;
	}
}

bool DSManager::IsActiveDragonSoul(LPITEM pItem) const
{
	if (pItem)
		return pItem->GetSocket(ITEM_SOCKET_DRAGON_SOUL_ACTIVE_IDX);

	return false;
}

bool DSManager::ActivateDragonSoul(LPITEM pItem)
{
	if (nullptr == pItem)
		return false;
	LPCHARACTER pOwner = pItem->GetOwner();
	if (nullptr == pOwner)
		return false;

	const int deck_idx = pOwner->DragonSoul_GetActiveDeck();

	if (deck_idx < 0)
		return false;

	if (WEAR_MAX_NUM + DS_SLOT_MAX * deck_idx <= pItem->GetCell() &&
		pItem->GetCell() < WEAR_MAX_NUM + DS_SLOT_MAX * (deck_idx + 1))
	{
		if (IsTimeLeftDragonSoul(pItem) && !IsActiveDragonSoul(pItem))
		{
			char buf[128];
			sprintf(buf, "LEFT TIME(%d)", LeftTime(pItem));
			LogManager::Instance().ItemLog(pOwner, pItem, "DS_ACTIVATE", buf);
			pItem->ModifyPoints(true);
			pItem->SetSocket(ITEM_SOCKET_DRAGON_SOUL_ACTIVE_IDX, 1);

			pItem->StartTimerBasedOnWearExpireEvent();
		}
		return true;
	}
	else
		return false;
}

bool DSManager::DeactivateDragonSoul(LPITEM pItem, bool bSkipRefreshOwnerActiveState)
{
	if (nullptr == pItem)
		return false;

	LPCHARACTER pOwner = pItem->GetOwner();
	if (nullptr == pOwner)
		return false;

	if (!IsActiveDragonSoul(pItem))
		return false;

	char buf[128];
	pItem->StopTimerBasedOnWearExpireEvent();
	pItem->SetSocket(ITEM_SOCKET_DRAGON_SOUL_ACTIVE_IDX, 0);
	pItem->ModifyPoints(false);

	sprintf(buf, "LEFT TIME(%d)", LeftTime(pItem));
	LogManager::Instance().ItemLog(pOwner, pItem, "DS_DEACTIVATE", buf);

	if (false == bSkipRefreshOwnerActiveState)
		RefreshDragonSoulState(pOwner);

	return true;
}

#ifdef ENABLE_DS_SET
bool DSManager::GetDSSetGrade(LPCHARACTER ch, uint8_t& iSetGrade)
{
	if (!ch)
		return false;

	const uint8_t iDeckIdx = ch->DragonSoul_GetActiveDeck();
	const uint16_t wDragonSoulDeckAffectType = AFFECT_DRAGON_SOUL_DECK_0 + iDeckIdx;
	if (!ch->FindAffect(wDragonSoulDeckAffectType))
		return false;

	const uint8_t iStartSlotIndex = WEAR_MAX_NUM + (iDeckIdx * DS_SLOT_MAX);
	const uint8_t iEndSlotIndex = iStartSlotIndex + DS_SLOT_MAX;

	iSetGrade = 0;

	for (uint8_t iSlotIndex = iStartSlotIndex; iSlotIndex < iEndSlotIndex; ++iSlotIndex)
	{
		const LPITEM pkItem = ch->GetWear(iSlotIndex);
		if (!pkItem)
			return false;

		if (!pkItem->IsDragonSoul())
			return false;

		if (!IsTimeLeftDragonSoul(pkItem))
			return false;

		if (!IsActiveDragonSoul(pkItem))
			return false;

		const uint8_t iGrade = GetGradeIdx(pkItem->GetVnum());
		if (iGrade < DRAGON_SOUL_GRADE_ANCIENT)
			return false;

		if (iSetGrade == 0)
			iSetGrade = iGrade;

		if (iSetGrade != iGrade)
			return false;
	}

	return true;
}

int DSManager::GetDSSetValue(uint8_t iAttributeIndex, uint16_t iApplyType, uint32_t iVnum, uint8_t iSetGrade) // @fixme-WL001
{
	const uint8_t iType = GetType(iVnum);
	float fWeight;
	if (!m_pTable->GetWeight(iType, iSetGrade - 1, 0, 0, fWeight))
		return 0;

	int iSetValue = 0;
	if (iAttributeIndex < m_pTable->GetBasicApplyCount(iType))
		m_pTable->GetBasicApplyValue(iType, iApplyType, iSetValue);
	else
		m_pTable->GetAdditionalApplyValue(iType, iApplyType, iSetValue);

	if (iSetValue == 0)
		return 0;

	return (iSetValue * fWeight - 1) / 100 + 1;
}
#endif

void DSManager::RefreshDragonSoulState(LPCHARACTER ch)
{
	if (nullptr == ch)
		return;
	for (int i = WEAR_MAX_NUM; i < WEAR_MAX_NUM + DS_SLOT_MAX * DRAGON_SOUL_DECK_MAX_NUM; i++)
	{
		LPITEM pItem = ch->GetWear(i);
		if (pItem != nullptr)
		{
			if (IsActiveDragonSoul(pItem))
			{
				return;
			}
		}
	}
	ch->DragonSoul_DeactivateAll();
}

DSManager::DSManager()
{
	m_pTable = nullptr;
}

DSManager::~DSManager()
{
	if (m_pTable)
		delete m_pTable;
}
