#include "StdAfx.h"

#ifdef ENABLE_GRAPHIC_ON_OFF
#include "PythonGraphicOnOff.h"

bool NOT_SAME_VALUE(uint8_t curValue, uint8_t bMin, uint8_t bMax)
{
	return (curValue < bMin || curValue > bMax);
}

bool CPythonGraphicOnOff::SetEffectOnOffLevel(uint8_t bLevel)
{
	if (NOT_SAME_VALUE(bLevel, m_bEffectLevelMin, m_bEffectLevelMax))
		return false;

	m_bEffectLevel = bLevel;
	return true;
}

uint8_t CPythonGraphicOnOff::GetEffectOnOffLevel() const
{
	return m_bEffectLevel;
}

bool CPythonGraphicOnOff::IsEffectOnOffLevel(uint8_t bActorType, bool isMain, uint8_t bRank) const
{
	if (bRank >= MOB_RANK_KING
		&& (bActorType == TYPE_WARP || bActorType == TYPE_GOTO || bActorType == TYPE_NPC)
		|| bRank >= MOB_RANK_BOSS && bActorType == TYPE_ENEMY)
		return true;

	switch (m_bEffectLevel)
	{
		case EFFECT_ONOFF_LEVEL_ALL:
		{
			return true;
		}
		break;

		case EFFECT_ONOFF_LEVEL_SELF_MONSTER:
		{
			if (bActorType == TYPE_ENEMY)
				return true;
			if (bActorType != TYPE_PC)
				return false;

			return isMain;
		}
		break;

		case EFFECT_ONOFF_LEVEL_SELF_PLAYER:
		{
			if (bActorType == TYPE_ENEMY)
				return false;

			return bActorType == TYPE_PC;
		}
		break;

		case EFFECT_ONOFF_LEVEL_SELF_ONLY:
		{
			if (bActorType == TYPE_ENEMY || bActorType != TYPE_PC)
				return false;

			return isMain;
		}
		break;

		case EFFECT_ONOFF_LEVEL_NONE:
		default:
		{
			return false;
		}
		break;
	}
}

bool CPythonGraphicOnOff::SetPrivateShopOnOffLevel(uint8_t bLevel)
{
	if (NOT_SAME_VALUE(bLevel, m_bPrivateShopLevelMin, m_bPrivateShopLevelMax))
		return false;

	m_bPrivateShopLevel = bLevel;
	return true;
}

uint8_t CPythonGraphicOnOff::GetPrivateShopOnOffLevel() const
{
	return m_bPrivateShopLevel;
}

bool CPythonGraphicOnOff::IsPrivateShopOnOffLevel(uint32_t dwRange) const
{
	bool bRet = false;

	switch (m_bPrivateShopLevel)
	{
	case PRIVATE_SHOP_ONOFF_LEVEL_FAR:
		bRet = dwRange <= 100000000;
		break;
	case PRIVATE_SHOP_ONOFF_LEVEL_BIG_RANGE:
		bRet = dwRange <= 25000000;
		break;
	case PRIVATE_SHOP_ONOFF_LEVEL_MIDLE_RANGE:
		bRet = dwRange <= 6250000;
		break;
	case PRIVATE_SHOP_ONOFF_LEVEL_SMALL_RANGE:
		bRet = dwRange <= 1000000;
		break;
	case PRIVATE_SHOP_ONOFF_LEVEL_CLOSE:
		bRet = dwRange <= 90000;
		break;
	default:
		break;
	}
	return bRet;
}
bool CPythonGraphicOnOff::SetItemDropOnOffLevel(uint8_t bLevel)
{
	if (NOT_SAME_VALUE(bLevel, m_bItemDropLevelMin, m_bItemDropLevelMax))
		return false;

	m_bItemDropLevel = bLevel;
	return true;
}

uint8_t CPythonGraphicOnOff::GetItemDropOnOffLevel()
{
	return m_bItemDropLevel;
}

bool CPythonGraphicOnOff::IsItemDropOnOffLevel(uint8_t bLevel) const
{
	if (m_bItemDropLevel == ITEM_DROP_ONOFF_LEVEL_NAME_N_EFFECT)
		return true;

	if (m_bItemDropLevel == ITEM_DROP_ONOFF_LEVEL_NONE)
		return false;

	return bLevel > m_bItemDropLevel;
}

bool CPythonGraphicOnOff::SetPetOnOffStatus(uint8_t bStatus)
{
	if (NOT_SAME_VALUE(bStatus, m_bPetStatusMin, m_bPetStatusMax))
		return false;

	m_bPetStatus = bStatus;
	return true;
}

uint8_t CPythonGraphicOnOff::GetPetOnOffStatus()
{
	return m_bPetStatus;
}

bool CPythonGraphicOnOff::IsPetOnOffStatus() const
{
	return (PET_STATUS_SHOW == m_bPetStatus || m_bPetStatus != PET_STATUS_HIDE);
}

bool CPythonGraphicOnOff::SetNPCNameOnOffStatus(uint8_t bStatus)
{
	if (NOT_SAME_VALUE(bStatus, m_bNPCNameStatusMin, m_bNPCNameStatusMax))
		return false;

	m_bNPCNameStatus = bStatus;
	return true;
}

uint8_t CPythonGraphicOnOff::GetNPCNameOnOffStatus()
{
	return m_bNPCNameStatus;
}

bool CPythonGraphicOnOff::IsNPCNameOnOffStatus() const
{
	return (NPC_NAME_STATUS_SHOW == m_bNPCNameStatus || m_bNPCNameStatus != NPC_NAME_STATUS_HIDE);
}

void CPythonGraphicOnOff::Destroy()
{
	Tracef("Python GraphicOnOff Destroy\n");
}

CPythonGraphicOnOff::CPythonGraphicOnOff()
{
	m_bEffectLevelMin = EFFECT_ONOFF_LEVEL_ALL;
	m_bEffectLevelMax = EFFECT_ONOFF_LEVEL_NONE;
	m_bPrivateShopLevelMin = PRIVATE_SHOP_ONOFF_LEVEL_FAR;
	m_bPrivateShopLevelMax = PRIVATE_SHOP_ONOFF_LEVEL_CLOSE;
	m_bItemDropLevelMin = ITEM_DROP_ONOFF_LEVEL_NAME_N_EFFECT;
	m_bItemDropLevelMax = ITEM_DROP_ONOFF_LEVEL_NONE;
	m_bPetStatusMin = PET_STATUS_SHOW;
	m_bPetStatusMax = PET_STATUS_HIDE;
	m_bNPCNameStatusMin = NPC_NAME_STATUS_SHOW;
	m_bNPCNameStatusMax = NPC_NAME_STATUS_HIDE;
	m_bEffectLevel = EFFECT_ONOFF_LEVEL_ALL;
	m_bPrivateShopLevel = PRIVATE_SHOP_ONOFF_LEVEL_FAR;
	m_bItemDropLevel = ITEM_DROP_ONOFF_LEVEL_NAME_N_EFFECT;
	m_bPetStatus = PET_STATUS_SHOW;
	m_bNPCNameStatus = NPC_NAME_STATUS_SHOW;
}

CPythonGraphicOnOff::~CPythonGraphicOnOff()
{
	Tracef("Python GraphicOnOff Clear\n");
}
#endif
