#include "stdafx.h"
#include <sstream>

#include "utils.h"
#include "config.h"
#include "vector.h"
#include "char.h"
#include "char_manager.h"
#include "battle.h"
#include "desc.h"
#include "desc_manager.h"
#include "packet.h"
#include "affect.h"
#include "item.h"
#include "sectree_manager.h"
#include "mob_manager.h"
#include "start_position.h"
#include "party.h"
#include "buffer_manager.h"
#include "guild.h"
#include "log.h"
#include "unique_item.h"
#include "questmanager.h"

#ifdef ENABLE_DAWNMIST_DUNGEON
#	include "DawnMistDungeon.h"
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
#	include "desc_client.h"
#endif
#ifdef ENABLE_BATTLE_FIELD
#	include "battle_field.h"
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
#	include "AchievementSystem.h"
#endif
#ifdef ENABLE_SUNG_MAHI_TOWER
# include "dungeon.h"
#endif

static const uint32_t s_adwSubSkillVnums[] =
{
	SKILL_LEADERSHIP,
	SKILL_COMBO,
	SKILL_MINING,
#ifdef ENABLE_REFINE_ABILITY_SKILL
	SKILL_REFINE,
#endif
	SKILL_LANGUAGE1,
	SKILL_LANGUAGE2,
	SKILL_LANGUAGE3,
	SKILL_POLYMORPH,
	SKILL_HORSE,
	SKILL_HORSE_SUMMON,
	SKILL_HORSE_WILDATTACK,
	SKILL_HORSE_CHARGE,
	SKILL_HORSE_ESCAPE,
	SKILL_HORSE_WILDATTACK_RANGE,
	SKILL_ADD_HP,
	SKILL_RESIST_PENETRATE,
#ifdef ENABLE_PASSIVE_ATTR
#	ifdef ENABLE_CHARISMA
	SKILL_CHARISMA,
#	endif
#	ifdef ENABLE_INSPIRATION
	SKILL_INSPIRATION,
#	endif
#	ifdef ENABLE_PRECISION
	SKILL_PRECISION
#	endif
#endif
};

time_t CHARACTER::GetSkillNextReadTime(uint32_t dwVnum) const
{
	if (dwVnum >= SKILL_MAX_NUM)
	{
		sys_err("vnum overflow (vnum: %u)", dwVnum);
		return 0;
	}

	return m_pSkillLevels ? m_pSkillLevels[dwVnum].tNextRead : 0;
}

void CHARACTER::SetSkillNextReadTime(uint32_t dwVnum, time_t time)
{
	if (m_pSkillLevels && dwVnum < SKILL_MAX_NUM)
		m_pSkillLevels[dwVnum].tNextRead = time;
}

bool TSkillUseInfo::HitOnce(uint32_t dwVnum)
{
	// If you don't use it, you can't hit it.
	if (!bUsed)
		return false;

	sys_log(1, "__HitOnce NextUse %u current %u count %d scount %d", dwNextSkillUsableTime, get_dword_time(), iHitCount, iSplashCount);

#ifdef ENABLE_PVP_BALANCE
	if (dwNextSkillUsableTime && dwNextSkillUsableTime < get_dword_time() && dwVnum != SKILL_MUYEONG && dwVnum != SKILL_HORSE_WILDATTACK && dwVnum != SKILL_GYEONGGONG)
#else
	if (dwNextSkillUsableTime && dwNextSkillUsableTime < get_dword_time() && dwVnum != SKILL_MUYEONG && dwVnum != SKILL_HORSE_WILDATTACK)
#endif
	{
		sys_log(1, "__HitOnce can't hit");

		return false;
	}

	if (iHitCount == -1)
	{
		sys_log(1, "__HitOnce OK %d %d %d", dwNextSkillUsableTime, get_dword_time(), iHitCount);
		return true;
	}

	if (iHitCount)
	{
		sys_log(1, "__HitOnce OK %d %d %d", dwNextSkillUsableTime, get_dword_time(), iHitCount);
		iHitCount--;
		return true;
	}
	return false;
}

#ifdef ENABLE_CHECK_SKILL_HACK
bool TSkillUseInfo::IsSkillCooldown(uint32_t dwVnum, float fSkillPower)
{
	uint32_t maxHitCount = 0;
	if (dwVnum == SKILL_KWANKYEOK || dwVnum == SKILL_HORSE_WILDATTACK_RANGE)
	{
		if (dwVnum == SKILL_KWANKYEOK)
			maxHitCount = static_cast<uint32_t>(2 + floor(6 * fSkillPower));
		else if (dwVnum == SKILL_HORSE_WILDATTACK_RANGE)
			maxHitCount = 5;

		dwHitCount++;

		if (dwHitCount > maxHitCount)
			return true;
		return false;
	}

	if (!bSkillCD)
		return true;

	bSkillCD = false;
	return bSkillCD;
}
#endif

bool TSkillUseInfo::UseSkill(bool isGrandMaster, uint32_t vid, uint32_t dwCooltime, int splashcount, int hitcount, int range)
{
	this->isGrandMaster = isGrandMaster;
	const uint32_t dwCur = get_dword_time();

	// The cooldown is not over yet.
	if (bUsed && dwNextSkillUsableTime > dwCur)
	{
		sys_log(0, "cooltime is not over delta %u", dwNextSkillUsableTime - dwCur);
		iHitCount = 0;
		return false;
	}

	bUsed = true;
#ifdef ENABLE_CHECK_SKILL_HACK
	bSkillCD = true;
	dwHitCount = 0;
#endif

	if (dwCooltime)
		dwNextSkillUsableTime = dwCur + dwCooltime;
	else
		dwNextSkillUsableTime = 0;

	iRange = range;
	iMaxHitCount = iHitCount = hitcount;

	if (test_server)
		sys_log(0, "UseSkill NextUse %u current %u cooltime %d hitcount %d/%d", dwNextSkillUsableTime, dwCur, dwCooltime, iHitCount, iMaxHitCount);

	dwVID = vid;
	iSplashCount = splashcount;
	return true;
}

int CHARACTER::GetChainLightningMaxCount() const
{
	return aiChainLightningCountBySkillLevel[MIN(SKILL_MAX_LEVEL, GetSkillLevel(SKILL_CHAIN))];
}

void CHARACTER::SetAffectedEunhyung()
{
	m_dwAffectedEunhyungLevel = GetSkillPower(SKILL_EUNHYUNG);
}

void CHARACTER::SetSkillGroup(uint8_t bSkillGroup)
{
	if (bSkillGroup > 2)
		return;

	if (GetLevel() < 5)
		return;

	m_points.skill_group = bSkillGroup;

	TPacketGCChangeSkillGroup p{};
	p.header = HEADER_GC_SKILL_GROUP;
	p.skill_group = m_points.skill_group;

	GetDesc()->Packet(&p, sizeof(TPacketGCChangeSkillGroup));
}

int CHARACTER::ComputeCooltime(int time)
{
	return CalculateDuration(GetPoint(POINT_CASTING_SPEED), time);
}

void CHARACTER::SkillLevelPacket()
{
	if (!GetDesc())
		return;

	TPacketGCSkillLevel pack{};

	pack.bHeader = HEADER_GC_SKILL_LEVEL;
	thecore_memcpy(&pack.skills, m_pSkillLevels, sizeof(TPlayerSkill) * SKILL_MAX_NUM);
#ifdef ENABLE_BATTLE_FIELD
	if (CBattleField::Instance().IsBattleZoneMapIndex(GetMapIndex()))
	{
		for (uint8_t i = 0; i < SKILL_MAX_NUM; ++i)
		{
			if (pack.skills[i].bLevel >= 1)
			{
				pack.skills[i].bLevel = 40;
				pack.skills[i].bMasterType = SKILL_PERFECT_MASTER;
			}
		}
	}
#endif
	GetDesc()->Packet(&pack, sizeof(TPacketGCSkillLevel));
}

void CHARACTER::SetSkillLevel(uint32_t dwVnum, uint8_t bLev)
{
	if (nullptr == m_pSkillLevels)
		return;

	if (dwVnum >= SKILL_MAX_NUM)
	{
		sys_err("vnum overflow (vnum %u)", dwVnum);
		return;
	}

#ifdef ENABLE_678TH_SKILL
	if (Is678Skill(dwVnum))
	{
		if (!SkillCanUp(dwVnum) && bLev != 0)
			return;
	}
#endif

#ifdef ENABLE_NINETH_SKILL
	if (IsNineSkill(dwVnum))
	{
		if (!NineSkillCanUp(dwVnum))
			return;
	}
#endif

	m_pSkillLevels[dwVnum].bLevel = static_cast<uint8_t>(MIN(40, bLev));

#ifdef ENABLE_REVERSED_FUNCTIONS
	if (dwVnum == SKILL_HORSE)
	{
		m_pSkillLevels[dwVnum].bMasterType = SKILL_NORMAL;
		return;
	}
#endif

	if (bLev >= 40)
		m_pSkillLevels[dwVnum].bMasterType = SKILL_PERFECT_MASTER;
	else if (bLev >= 30)
		m_pSkillLevels[dwVnum].bMasterType = SKILL_GRAND_MASTER;
	else if (bLev >= 20)
		m_pSkillLevels[dwVnum].bMasterType = SKILL_MASTER;
	else
		m_pSkillLevels[dwVnum].bMasterType = SKILL_NORMAL;
}

bool CHARACTER::IsLearnableSkill(uint32_t dwSkillVnum) const
{
	const CSkillProto* pkSkill = CSkillManager::Instance().Get(dwSkillVnum);

	if (!pkSkill)
		return false;

	if (GetSkillLevel(dwSkillVnum) >= SKILL_MAX_LEVEL)
		return false;

	if (pkSkill->dwType == 0)
	{
		if (GetSkillLevel(dwSkillVnum) >= pkSkill->bMaxLevel)
			return false;

		return true;
	}

	if (pkSkill->dwType == 5)
	{
		if (dwSkillVnum == SKILL_HORSE_WILDATTACK_RANGE && GetJob() != JOB_ASSASSIN)
			return false;

		return true;
	}

	if (GetSkillGroup() == 0)
		return false;

	if (pkSkill->dwType - JOB_ASSASSIN == GetJob())
		return true;

#ifdef ENABLE_WOLFMAN_CHARACTER
	// Handicraft skill
	if (7 == pkSkill->dwType && JOB_WOLFMAN == GetJob())
		return true;
#endif

	if (6 == pkSkill->dwType)
	{
		if (SKILL_7_A_ANTI_TANHWAN <= dwSkillVnum && dwSkillVnum <= SKILL_7_D_ANTI_YONGBI)
		{
			for (int i = 0; i < 4; i++)
			{
				if (unsigned(SKILL_7_A_ANTI_TANHWAN + i) != dwSkillVnum)
				{
					if (0 != GetSkillLevel(SKILL_7_A_ANTI_TANHWAN + i))
					{
						return false;
					}
				}
			}

			return true;
		}

		if (SKILL_8_A_ANTI_GIGONGCHAM <= dwSkillVnum && dwSkillVnum <= SKILL_8_D_ANTI_BYEURAK)
		{
			for (int i = 0; i < 4; i++)
			{
				if (unsigned(SKILL_8_A_ANTI_GIGONGCHAM + i) != dwSkillVnum)
				{
					if (0 != GetSkillLevel(SKILL_8_A_ANTI_GIGONGCHAM + i))
						return false;
				}
			}

			return true;
		}
	}

#ifdef ENABLE_78TH_SKILL
	if (pkSkill->dwType == 6)
	{
		if (dwSkillVnum >= SKILL_ANTI_PALBANG
#	ifdef ENABLE_WOLFMAN_CHARACTER
			&& dwSkillVnum <= SKILL_HELP_SALPOONG
#	else
			&& dwSkillVnum <= SKILL_HELP_BYEURAK
#	endif
			)
		{
			if (GetSkillLevel(dwSkillVnum) != 0)
				return true;
		}
	}
#endif

	return false;
}

// ADD_GRANDMASTER_SKILL
bool CHARACTER::LearnGrandMasterSkill(uint32_t dwSkillVnum)
{
	const CSkillProto* pkSk = CSkillManager::Instance().Get(dwSkillVnum);

	if (!pkSk)
		return false;

	if (!IsLearnableSkill(dwSkillVnum))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;948]");
		return false;
	}

	sys_log(0, "learn grand master skill[%d] cur %d, next %d", dwSkillVnum, get_global_time(), GetSkillNextReadTime(dwSkillVnum));

	/*
	if (get_global_time() < GetSkillNextReadTime(dwSkillVnum))
	{
		if (!(test_server && quest::CQuestManager::Instance().GetEventFlag("no_read_delay")))
		{
			if (FindAffect(AFFECT_SKILL_NO_BOOK_DELAY))
			{
				// Ignore the time limit while using the magic spell
				RemoveAffect(AFFECT_SKILL_NO_BOOK_DELAY);
				ChatPacket(CHAT_TYPE_INFO, "[LS;377]");
			}
			else
			{
				SkillLearnWaitMoreTimeMessage(GetSkillNextReadTime(dwSkillVnum) - get_global_time());
				return false;
			}
		}
	}
	*/

	// If bType is 0, you can study as a book from the beginning.
	if (pkSk->dwType == 0)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;949]");
		return false;
	}

	if (GetSkillMasterType(dwSkillVnum) != SKILL_GRAND_MASTER)
	{
		if (GetSkillMasterType(dwSkillVnum) > SKILL_GRAND_MASTER)
			ChatPacket(CHAT_TYPE_INFO, "[LS;950]");
		else
			ChatPacket(CHAT_TYPE_INFO, "[LS;951]");
		return false;
	}

	std::string strTrainSkill;
	{
		std::ostringstream os;
		os << "training_grandmaster_skill.skill" << dwSkillVnum;
		strTrainSkill = os.str();
	}

	// Here we calculate the probability.
	const auto bLastLevel = static_cast<uint8_t>(GetSkillLevel(dwSkillVnum));

	const int idx = MIN(9, GetSkillLevel(dwSkillVnum) - 30);

	sys_log(0, "LearnGrandMasterSkill %s table idx %d value %d", GetName(), idx, aiGrandMasterSkillBookCountForLevelUp[idx]);

	const int iTotalReadCount = GetQuestFlag(strTrainSkill) + 1;
	SetQuestFlag(strTrainSkill, iTotalReadCount);

	const int iMinReadCount = aiGrandMasterSkillBookMinCount[idx];
	const int iMaxReadCount = aiGrandMasterSkillBookMaxCount[idx];

	int iBookCount = aiGrandMasterSkillBookCountForLevelUp[idx];

	if (FindAffect(AFFECT_SKILL_BOOK_BONUS))
	{
		if (iBookCount & 1)
			iBookCount = iBookCount / 2 + 1;
		else
			iBookCount = iBookCount / 2;

		RemoveAffect(AFFECT_SKILL_BOOK_BONUS);
	}

	const int n = number(1, iBookCount);
	sys_log(0, "Number(%d)", n);

	const uint32_t nextTime = get_global_time() + number(g_dwSkillBookNextReadMin, g_dwSkillBookNextReadMax);

	sys_log(0, "GrandMaster SkillBookCount min %d cur %d max %d (next_time=%d)", iMinReadCount, iTotalReadCount, iMaxReadCount, nextTime);

	bool bSuccess = n == 2;

	if (iTotalReadCount < iMinReadCount)
		bSuccess = false;
	if (iTotalReadCount > iMaxReadCount)
		bSuccess = true;

	if (bSuccess)
	{
		SkillLevelUp(dwSkillVnum, SKILL_UP_BY_QUEST);
	}

	SetSkillNextReadTime(dwSkillVnum, nextTime);

	if (bLastLevel == GetSkillLevel(dwSkillVnum))
	{
		ChatPacket(CHAT_TYPE_TALKING, "[LS;952]");
		ChatPacket(CHAT_TYPE_INFO, "[LS;953]");
		LogManager::Instance().CharLog(this, dwSkillVnum, "GM_READ_FAIL", "");
		return false;
	}

	ChatPacket(CHAT_TYPE_TALKING, "[LS;954]");
	ChatPacket(CHAT_TYPE_TALKING, "[LS;955]");
	ChatPacket(CHAT_TYPE_INFO, "[LS;956]");
	LogManager::Instance().CharLog(this, dwSkillVnum, "GM_READ_SUCCESS", "");
	return true;
}
// END_OF_ADD_GRANDMASTER_SKILL

static bool FN_should_check_exp(LPCHARACTER ch)
{
	// @warme005
	if (!ch)
		return false;

	return ch->GetLevel() < gPlayerMaxLevel;
}

#ifdef ENABLE_YOHARA_SYSTEM
static bool FN_should_check_cexp(LPCHARACTER ch)
{
	// @warme005
	if (!ch)
		return false;

	return ch->GetConquerorLevel() < gConquerorMaxLevel;
}
#endif

bool CHARACTER::LearnSkillByBook(uint32_t dwSkillVnum, uint8_t bProb)
{
	const CSkillProto* pkSk = CSkillManager::Instance().Get(dwSkillVnum);

	if (!pkSk)
		return false;

	if (!IsLearnableSkill(dwSkillVnum))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;948]");
		return false;
	}

	uint32_t need_exp = 0;

	if (FN_should_check_exp(this))
	{
		need_exp = SB_NEED_EXP;

		if (GetExp() < need_exp)
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;957]");
			return false;
		}
	}

	// If bType is 0, you can study as a book from the beginning.
	if (pkSk->dwType != 0)
	{
		if (GetSkillMasterType(dwSkillVnum) != SKILL_MASTER)
		{
			if (GetSkillMasterType(dwSkillVnum) > SKILL_MASTER)
				ChatPacket(CHAT_TYPE_INFO, "[LS;959]");
			else
				ChatPacket(CHAT_TYPE_INFO, "[LS;960]");
			return false;
		}
	}

	if (get_global_time() < GetSkillNextReadTime(dwSkillVnum))
	{
		if (!(test_server && quest::CQuestManager::Instance().GetEventFlag("no_read_delay")))
		{
			if (FindAffect(AFFECT_SKILL_NO_BOOK_DELAY))
			{
				// Ignore the time limit while using an eye spell
				RemoveAffect(AFFECT_SKILL_NO_BOOK_DELAY);
				ChatPacket(CHAT_TYPE_INFO, "[LS;377]");
			}
			else
			{
				SkillLearnWaitMoreTimeMessage(GetSkillNextReadTime(dwSkillVnum) - get_global_time());
				return false;
			}
		}
	}

	// Here we calculate the probability.
	const auto bLastLevel = static_cast<uint8_t>(GetSkillLevel(dwSkillVnum));

	if (bProb != 0)
	{
		// SKILL_BOOK_BONUS
		if (FindAffect(AFFECT_SKILL_BOOK_BONUS))
		{
			bProb += bProb / 2;
			RemoveAffect(AFFECT_SKILL_BOOK_BONUS);
		}
		// END_OF_SKILL_BOOK_BONUS

		sys_log(0, "LearnSkillByBook Pct %u prob %d", dwSkillVnum, bProb);

		if (number(1, 100) <= bProb)
		{
			if (test_server)
				sys_log(0, "LearnSkillByBook %u SUCC", dwSkillVnum);

			SkillLevelUp(dwSkillVnum, SKILL_UP_BY_BOOK);
		}
		else
		{
			if (test_server)
				sys_log(0, "LearnSkillByBook %u FAIL", dwSkillVnum);
		}
	}
	else
	{
		const int idx = MIN(9, GetSkillLevel(dwSkillVnum) - 20);

		sys_log(0, "LearnSkillByBook %s table idx %d value %d", GetName(), idx, aiSkillBookCountForLevelUp[idx]);

		{
			const int need_bookcount = GetSkillLevel(dwSkillVnum) - 20;

			PointChange(POINT_EXP, -need_exp);

			quest::CQuestManager& q = quest::CQuestManager::Instance();
			quest::PC* pPC = q.GetPC(GetPlayerID());

			if (pPC)
			{
				char flag[128 + 1];
				memset(flag, 0, sizeof(flag));
				snprintf(flag, sizeof(flag), "traning_master_skill.%u.read_count", dwSkillVnum);

				const int read_count = pPC->GetFlag(flag);
				int percent = 65;

				if (FindAffect(AFFECT_SKILL_BOOK_BONUS))
				{
					percent = 0;
					RemoveAffect(AFFECT_SKILL_BOOK_BONUS);
				}

				if (number(1, 100) > percent)
				{
					// Success in reading
#ifdef ENABLE_MASTER_SKILLBOOK_NO_STEPS
					if (true)
#else
					if (read_count >= need_bookcount)
#endif
					{
						SkillLevelUp(dwSkillVnum, SKILL_UP_BY_BOOK);
						pPC->SetFlag(flag, 0);

						ChatPacket(CHAT_TYPE_INFO, "[LS;964]");
						LogManager::Instance().CharLog(this, dwSkillVnum, "READ_SUCCESS", "");
						return true;
					}
					else
					{
						pPC->SetFlag(flag, read_count + 1);

						switch (number(1, 3))
						{
							case 1:
								ChatPacket(CHAT_TYPE_TALKING, "[LS;961]");
								break;

							case 2:
								ChatPacket(CHAT_TYPE_TALKING, "[LS;962]");
								break;

							case 3:
							default:
								ChatPacket(CHAT_TYPE_TALKING, "[LS;963]");
								break;
						}

						ChatPacket(CHAT_TYPE_INFO, "[LS;995;%d]", need_bookcount - read_count);
						return true;
					}
				}
			}
			else
			{
				// Failed to load user's quest information
			}
		}
	}

	if (bLastLevel != GetSkillLevel(dwSkillVnum))
	{
		ChatPacket(CHAT_TYPE_TALKING, "[LS;954]");
		ChatPacket(CHAT_TYPE_TALKING, "[LS;955]");
		ChatPacket(CHAT_TYPE_INFO, "[LS;964]");
		LogManager::Instance().CharLog(this, dwSkillVnum, "READ_SUCCESS", "");
	}
	else
	{
		ChatPacket(CHAT_TYPE_TALKING, "[LS;952]");
		ChatPacket(CHAT_TYPE_INFO, "[LS;953]");
		LogManager::Instance().CharLog(this, dwSkillVnum, "READ_FAIL", "");
	}

	return true;
}

bool CHARACTER::SkillLevelDown(uint32_t dwVnum)
{
	if (nullptr == m_pSkillLevels)
		return false;

	if (g_bSkillDisable)
		return false;

	if (IsPolymorphed())
		return false;

	const CSkillProto* pkSk = CSkillManager::Instance().Get(dwVnum);

	if (!pkSk)
	{
		sys_err("There is no such skill by number %u", dwVnum);
		return false;
	}

	if (!IsLearnableSkill(dwVnum))
		return false;

	if (GetSkillMasterType(pkSk->dwVnum) != SKILL_NORMAL)
		return false;

	if (!GetSkillGroup())
		return false;

	if (pkSk->dwVnum >= SKILL_MAX_NUM)
		return false;

	if (m_pSkillLevels[pkSk->dwVnum].bLevel == 0)
		return false;

	uint8_t val = 1;

#ifdef ENABLE_678TH_SKILL
	int dwPoints = GetQuestFlag("678th_skill.points");
	if (Is678Skill(dwVnum))
	{
		if (m_pSkillLevels[pkSk->dwVnum].bLevel <= 1)
			return false;

		if (dwPoints <= 0)
			val = 0;
	}
#endif

#ifdef ENABLE_NINETH_SKILL
	if (IsNineSkill(dwVnum))
	{
		if (m_pSkillLevels[pkSk->dwVnum].bLevel <= 0)
			return false;

		val = 0;
	}
#endif

	int idx = POINT_SKILL;
	switch (pkSk->dwType)
	{
		case 0:
			idx = POINT_SUB_SKILL;
			break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 6:
#ifdef ENABLE_WOLFMAN_CHARACTER
		case 7:
#endif
			idx = POINT_SKILL;
			break;
		case 5:
			idx = POINT_HORSE_SKILL;
			break;
		default:
			sys_err("Wrong skill type %d skill vnum %d", pkSk->dwType, pkSk->dwVnum);
			return false;
	}

	PointChange(idx, val);
	SetSkillLevel(pkSk->dwVnum, m_pSkillLevels[pkSk->dwVnum].bLevel - 1);

#ifdef ENABLE_678TH_SKILL
	if (Is678Skill(dwVnum) && val != 0)
		SetQuestFlag("678th_skill.points", dwPoints - 1);
#endif

	sys_log(0, "SkillDown: %s %u %u %u type %u", GetName(), pkSk->dwVnum, m_pSkillLevels[pkSk->dwVnum].bMasterType, m_pSkillLevels[pkSk->dwVnum].bLevel, pkSk->dwType);
	Save();

	ComputePoints();
	SkillLevelPacket();
	return true;
}

#ifdef ENABLE_678TH_SKILL
bool CHARACTER::SkillCanUp(uint32_t dwVnum)
{
	bool canLevelUP = false;
	switch (dwVnum)
	{
		case SKILL_ANTI_PALBANG:
		{
			if (GetSkillLevel(SKILL_ANTI_AMSEOP) == 0 &&
				GetSkillLevel(SKILL_ANTI_SWAERYUNG) == 0 &&
				GetSkillLevel(SKILL_ANTI_YONGBI) == 0 &&
				GetSkillLevel(SKILL_ANTI_GIGONGCHAM) == 0 &&
				GetSkillLevel(SKILL_ANTI_HWAJO) == 0 &&
				GetSkillLevel(SKILL_ANTI_MARYUNG) == 0 &&
				GetSkillLevel(SKILL_ANTI_BYEURAK) == 0
#	ifdef ENABLE_WOLFMAN_CHARACTER
				&& GetSkillLevel(SKILL_ANTI_SALPOONG) == 0
#	endif
				)
				canLevelUP = true;
			break;
		}

		case SKILL_ANTI_AMSEOP:
		{
			if (GetSkillLevel(SKILL_ANTI_PALBANG) == 0 &&
				GetSkillLevel(SKILL_ANTI_SWAERYUNG) == 0 &&
				GetSkillLevel(SKILL_ANTI_YONGBI) == 0 &&
				GetSkillLevel(SKILL_ANTI_GIGONGCHAM) == 0 &&
				GetSkillLevel(SKILL_ANTI_HWAJO) == 0 &&
				GetSkillLevel(SKILL_ANTI_MARYUNG) == 0 &&
				GetSkillLevel(SKILL_ANTI_BYEURAK) == 0
#	ifdef ENABLE_WOLFMAN_CHARACTER
				&& GetSkillLevel(SKILL_ANTI_SALPOONG) == 0
#	endif
				)
				canLevelUP = true;
			break;
		}

		case SKILL_ANTI_SWAERYUNG:
		{
			if (GetSkillLevel(SKILL_ANTI_PALBANG) == 0 &&
				GetSkillLevel(SKILL_ANTI_AMSEOP) == 0 &&
				GetSkillLevel(SKILL_ANTI_YONGBI) == 0 &&
				GetSkillLevel(SKILL_ANTI_GIGONGCHAM) == 0 &&
				GetSkillLevel(SKILL_ANTI_HWAJO) == 0 &&
				GetSkillLevel(SKILL_ANTI_MARYUNG) == 0 &&
				GetSkillLevel(SKILL_ANTI_BYEURAK) == 0
#	ifdef ENABLE_WOLFMAN_CHARACTER
				&& GetSkillLevel(SKILL_ANTI_SALPOONG) == 0
#	endif
				)
				canLevelUP = true;
			break;
		}

		case SKILL_ANTI_YONGBI:
		{
			if (GetSkillLevel(SKILL_ANTI_PALBANG) == 0 &&
				GetSkillLevel(SKILL_ANTI_AMSEOP) == 0 &&
				GetSkillLevel(SKILL_ANTI_SWAERYUNG) == 0 &&
				GetSkillLevel(SKILL_ANTI_GIGONGCHAM) == 0 &&
				GetSkillLevel(SKILL_ANTI_HWAJO) == 0 &&
				GetSkillLevel(SKILL_ANTI_MARYUNG) == 0 &&
				GetSkillLevel(SKILL_ANTI_BYEURAK) == 0
#	ifdef ENABLE_WOLFMAN_CHARACTER
				&& GetSkillLevel(SKILL_ANTI_SALPOONG) == 0
#	endif
				)
				canLevelUP = true;
			break;
		}

		case SKILL_ANTI_GIGONGCHAM:
		{
			if (GetSkillLevel(SKILL_ANTI_PALBANG) == 0 &&
				GetSkillLevel(SKILL_ANTI_AMSEOP) == 0 &&
				GetSkillLevel(SKILL_ANTI_SWAERYUNG) == 0 &&
				GetSkillLevel(SKILL_ANTI_YONGBI) == 0 &&
				GetSkillLevel(SKILL_ANTI_HWAJO) == 0 &&
				GetSkillLevel(SKILL_ANTI_MARYUNG) == 0 &&
				GetSkillLevel(SKILL_ANTI_BYEURAK) == 0
#	ifdef ENABLE_WOLFMAN_CHARACTER
				&& GetSkillLevel(SKILL_ANTI_SALPOONG) == 0
#	endif
				)
				canLevelUP = true;
			break;
		}

		case SKILL_ANTI_HWAJO:
		{
			if (GetSkillLevel(SKILL_ANTI_PALBANG) == 0 &&
				GetSkillLevel(SKILL_ANTI_AMSEOP) == 0 &&
				GetSkillLevel(SKILL_ANTI_SWAERYUNG) == 0 &&
				GetSkillLevel(SKILL_ANTI_YONGBI) == 0 &&
				GetSkillLevel(SKILL_ANTI_GIGONGCHAM) == 0 &&
				GetSkillLevel(SKILL_ANTI_MARYUNG) == 0 &&
				GetSkillLevel(SKILL_ANTI_BYEURAK) == 0
#	ifdef ENABLE_WOLFMAN_CHARACTER
				&& GetSkillLevel(SKILL_ANTI_SALPOONG) == 0
#	endif
				)
				canLevelUP = true;
			break;
		}

		case SKILL_ANTI_MARYUNG:
		{
			if (GetSkillLevel(SKILL_ANTI_PALBANG) == 0 &&
				GetSkillLevel(SKILL_ANTI_AMSEOP) == 0 &&
				GetSkillLevel(SKILL_ANTI_SWAERYUNG) == 0 &&
				GetSkillLevel(SKILL_ANTI_YONGBI) == 0 &&
				GetSkillLevel(SKILL_ANTI_GIGONGCHAM) == 0 &&
				GetSkillLevel(SKILL_ANTI_HWAJO) == 0 &&
				GetSkillLevel(SKILL_ANTI_BYEURAK) == 0
#	ifdef ENABLE_WOLFMAN_CHARACTER
				&& GetSkillLevel(SKILL_ANTI_SALPOONG) == 0
#	endif
				)
				canLevelUP = true;
			break;
		}

		case SKILL_ANTI_BYEURAK:
		{
			if (GetSkillLevel(SKILL_ANTI_PALBANG) == 0 &&
				GetSkillLevel(SKILL_ANTI_AMSEOP) == 0 &&
				GetSkillLevel(SKILL_ANTI_SWAERYUNG) == 0 &&
				GetSkillLevel(SKILL_ANTI_YONGBI) == 0 &&
				GetSkillLevel(SKILL_ANTI_GIGONGCHAM) == 0 &&
				GetSkillLevel(SKILL_ANTI_HWAJO) == 0 &&
				GetSkillLevel(SKILL_ANTI_MARYUNG) == 0
#	ifdef ENABLE_WOLFMAN_CHARACTER
				&& GetSkillLevel(SKILL_ANTI_SALPOONG) == 0
#	endif
				)
				canLevelUP = true;
			break;
		}

#	ifdef ENABLE_WOLFMAN_CHARACTER
		case SKILL_ANTI_SALPOONG:
		{
			if (GetSkillLevel(SKILL_ANTI_PALBANG) == 0 &&
				GetSkillLevel(SKILL_ANTI_AMSEOP) == 0 &&
				GetSkillLevel(SKILL_ANTI_SWAERYUNG) == 0 &&
				GetSkillLevel(SKILL_ANTI_YONGBI) == 0 &&
				GetSkillLevel(SKILL_ANTI_GIGONGCHAM) == 0 &&
				GetSkillLevel(SKILL_ANTI_HWAJO) == 0 &&
				GetSkillLevel(SKILL_ANTI_MARYUNG) == 0 &&
				GetSkillLevel(SKILL_ANTI_BYEURAK) == 0)
				canLevelUP = true;
			break;
		}
#	endif

		case SKILL_HELP_PALBANG:
		{
			if (GetSkillLevel(SKILL_HELP_AMSEOP) == 0 &&
				GetSkillLevel(SKILL_HELP_SWAERYUNG) == 0 &&
				GetSkillLevel(SKILL_HELP_YONGBI) == 0 &&
				GetSkillLevel(SKILL_HELP_GIGONGCHAM) == 0 &&
				GetSkillLevel(SKILL_HELP_HWAJO) == 0 &&
				GetSkillLevel(SKILL_HELP_MARYUNG) == 0 &&
				GetSkillLevel(SKILL_HELP_BYEURAK) == 0
#	ifdef ENABLE_WOLFMAN_CHARACTER
				&& GetSkillLevel(SKILL_HELP_SALPOONG) == 0
#	endif
				)
				canLevelUP = true;
			break;
		}

		case SKILL_HELP_AMSEOP:
		{
			if (GetSkillLevel(SKILL_HELP_PALBANG) == 0 &&
				GetSkillLevel(SKILL_HELP_SWAERYUNG) == 0 &&
				GetSkillLevel(SKILL_HELP_YONGBI) == 0 &&
				GetSkillLevel(SKILL_HELP_GIGONGCHAM) == 0 &&
				GetSkillLevel(SKILL_HELP_HWAJO) == 0 &&
				GetSkillLevel(SKILL_HELP_MARYUNG) == 0 &&
				GetSkillLevel(SKILL_HELP_BYEURAK) == 0
#	ifdef ENABLE_WOLFMAN_CHARACTER
				&& GetSkillLevel(SKILL_HELP_SALPOONG) == 0
#	endif
				)
				canLevelUP = true;
			break;
		}

		case SKILL_HELP_SWAERYUNG:
		{
			if (GetSkillLevel(SKILL_HELP_PALBANG) == 0 &&
				GetSkillLevel(SKILL_HELP_AMSEOP) == 0 &&
				GetSkillLevel(SKILL_HELP_YONGBI) == 0 &&
				GetSkillLevel(SKILL_HELP_GIGONGCHAM) == 0 &&
				GetSkillLevel(SKILL_HELP_HWAJO) == 0 &&
				GetSkillLevel(SKILL_HELP_MARYUNG) == 0 &&
				GetSkillLevel(SKILL_HELP_BYEURAK) == 0
#	ifdef ENABLE_WOLFMAN_CHARACTER
				&& GetSkillLevel(SKILL_HELP_SALPOONG) == 0
#	endif
				)
				canLevelUP = true;
			break;
		}

		case SKILL_HELP_YONGBI:
		{
			if (GetSkillLevel(SKILL_HELP_PALBANG) == 0 &&
				GetSkillLevel(SKILL_HELP_AMSEOP) == 0 &&
				GetSkillLevel(SKILL_HELP_SWAERYUNG) == 0 &&
				GetSkillLevel(SKILL_HELP_GIGONGCHAM) == 0 &&
				GetSkillLevel(SKILL_HELP_HWAJO) == 0 &&
				GetSkillLevel(SKILL_HELP_MARYUNG) == 0 &&
				GetSkillLevel(SKILL_HELP_BYEURAK) == 0
#	ifdef ENABLE_WOLFMAN_CHARACTER
				&& GetSkillLevel(SKILL_HELP_SALPOONG) == 0
#	endif
				)
				canLevelUP = true;
			break;
		}

		case SKILL_HELP_GIGONGCHAM:
		{
			if (GetSkillLevel(SKILL_HELP_PALBANG) == 0 &&
				GetSkillLevel(SKILL_HELP_AMSEOP) == 0 &&
				GetSkillLevel(SKILL_HELP_SWAERYUNG) == 0 &&
				GetSkillLevel(SKILL_HELP_YONGBI) == 0 &&
				GetSkillLevel(SKILL_HELP_HWAJO) == 0 &&
				GetSkillLevel(SKILL_HELP_MARYUNG) == 0 &&
				GetSkillLevel(SKILL_HELP_BYEURAK) == 0
#	ifdef ENABLE_WOLFMAN_CHARACTER
				&& GetSkillLevel(SKILL_HELP_SALPOONG) == 0
#	endif
				)
				canLevelUP = true;
			break;
		}

		case SKILL_HELP_HWAJO:
		{
			if (GetSkillLevel(SKILL_HELP_PALBANG) == 0 &&
				GetSkillLevel(SKILL_HELP_AMSEOP) == 0 &&
				GetSkillLevel(SKILL_HELP_SWAERYUNG) == 0 &&
				GetSkillLevel(SKILL_HELP_YONGBI) == 0 &&
				GetSkillLevel(SKILL_HELP_GIGONGCHAM) == 0 &&
				GetSkillLevel(SKILL_HELP_MARYUNG) == 0 &&
				GetSkillLevel(SKILL_HELP_BYEURAK) == 0
#	ifdef ENABLE_WOLFMAN_CHARACTER
				&& GetSkillLevel(SKILL_HELP_SALPOONG) == 0
#	endif
				)
				canLevelUP = true;
			break;
		}

		case SKILL_HELP_MARYUNG:
		{
			if (GetSkillLevel(SKILL_HELP_PALBANG) == 0 &&
				GetSkillLevel(SKILL_HELP_AMSEOP) == 0 &&
				GetSkillLevel(SKILL_HELP_SWAERYUNG) == 0 &&
				GetSkillLevel(SKILL_HELP_YONGBI) == 0 &&
				GetSkillLevel(SKILL_HELP_GIGONGCHAM) == 0 &&
				GetSkillLevel(SKILL_HELP_HWAJO) == 0 &&
				GetSkillLevel(SKILL_HELP_BYEURAK) == 0
#	ifdef ENABLE_WOLFMAN_CHARACTER
				&& GetSkillLevel(SKILL_HELP_SALPOONG) == 0
#	endif
				)
				canLevelUP = true;
			break;
		}

		case SKILL_HELP_BYEURAK:
		{
			if (GetSkillLevel(SKILL_HELP_PALBANG) == 0 &&
				GetSkillLevel(SKILL_HELP_AMSEOP) == 0 &&
				GetSkillLevel(SKILL_HELP_SWAERYUNG) == 0 &&
				GetSkillLevel(SKILL_HELP_YONGBI) == 0 &&
				GetSkillLevel(SKILL_HELP_GIGONGCHAM) == 0 &&
				GetSkillLevel(SKILL_HELP_HWAJO) == 0 &&
				GetSkillLevel(SKILL_HELP_MARYUNG) == 0
#	ifdef ENABLE_WOLFMAN_CHARACTER
				&& GetSkillLevel(SKILL_HELP_SALPOONG) == 0
#	endif
				)
				canLevelUP = true;
			break;
		}

#	ifdef ENABLE_WOLFMAN_CHARACTER
		case SKILL_HELP_SALPOONG:
		{
			if (GetSkillLevel(SKILL_HELP_PALBANG) == 0 &&
				GetSkillLevel(SKILL_HELP_AMSEOP) == 0 &&
				GetSkillLevel(SKILL_HELP_SWAERYUNG) == 0 &&
				GetSkillLevel(SKILL_HELP_YONGBI) == 0 &&
				GetSkillLevel(SKILL_HELP_GIGONGCHAM) == 0 &&
				GetSkillLevel(SKILL_HELP_HWAJO) == 0 &&
				GetSkillLevel(SKILL_HELP_MARYUNG) == 0 &&
				GetSkillLevel(SKILL_HELP_BYEURAK) == 0)
				canLevelUP = true;
			break;
		}
#	endif

		default:
			break;
	}

	return canLevelUP;
}
#endif

#ifdef ENABLE_NINETH_SKILL
bool CHARACTER::NineSkillCanUp(uint32_t dwVnum)
{
	bool canLevelUP = false;
	switch (dwVnum)
	{
		case SKILL_FINISH:
		case SKILL_ILGWANGPYO:
		case SKILL_PUNGLOEPO:
		case SKILL_GEOMAGGWI:
		case SKILL_MABEOBAGGWI:
		case SKILL_METEO:
		case SKILL_CHEONUN:
#	ifdef ENABLE_WOLFMAN_CHARACTER
		case SKILL_ILIPUNGU:
#	endif
		{
			if (GetConquerorLevel() > 0 || IsGM())
				canLevelUP = true;
			break;
		}
		default:
			break;
	}

	return canLevelUP;
}
#endif

void CHARACTER::SkillLevelUp(uint32_t dwVnum, uint8_t bMethod)
{
	if (nullptr == m_pSkillLevels)
		return;

	if (g_bSkillDisable)
		return;

	if (IsPolymorphed())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;521]");
		return;
	}

#ifdef ENABLE_678TH_SKILL
	if (Is678Skill(dwVnum))
	{
		if (!SkillCanUp(dwVnum))
			return;
	}
#endif

#ifdef ENABLE_NINETH_SKILL
	if (IsNineSkill(dwVnum))
	{
		if (!NineSkillCanUp(dwVnum))
			return;
	}
#endif

	if (SKILL_7_A_ANTI_TANHWAN <= dwVnum && dwVnum <= SKILL_8_D_ANTI_BYEURAK)
	{
		if (0 == GetSkillLevel(dwVnum))
			return;
	}

	const CSkillProto* pkSk = CSkillManager::Instance().Get(dwVnum);

	if (!pkSk)
	{
		sys_err("There is no such skill by number (vnum %u)", dwVnum);
		return;
	}

	if (pkSk->dwVnum >= SKILL_MAX_NUM)
	{
		sys_err("Skill Vnum overflow (vnum %u)", dwVnum);
		return;
	}

	if (!IsLearnableSkill(dwVnum))
		return;

	// Grand Master can only be performed as a quest
	if (pkSk->dwType != 0)
	{
		switch (GetSkillMasterType(pkSk->dwVnum))
		{
			case SKILL_GRAND_MASTER:
				if (bMethod != SKILL_UP_BY_QUEST)
					return;
				break;

			case SKILL_PERFECT_MASTER:
				return;

			default:
				break;
		}
	}

	if (bMethod == SKILL_UP_BY_POINT)
	{
		// Can only be trained if you are not a master
		if (GetSkillMasterType(pkSk->dwVnum) != SKILL_NORMAL)
			return;

		if (IS_SET(pkSk->dwFlag, SKILL_FLAG_DISABLE_BY_POINT_UP))
			return;
	}
	else if (bMethod == SKILL_UP_BY_BOOK)
	{
		if (pkSk->dwType != 0) // Skills that do not belong to a job or that cannot be raised with points can be learned from the beginning with books.
		{
			if (GetSkillMasterType(pkSk->dwVnum) != SKILL_MASTER)
				return;
		}
	}

	if (GetLevel() < pkSk->bLevelLimit)
		return;

	if (pkSk->preSkillVnum)
	{
		if (GetSkillMasterType(pkSk->preSkillVnum) == SKILL_NORMAL &&
			GetSkillLevel(pkSk->preSkillVnum) < pkSk->preSkillLevel)
			return;
	}

	if (!GetSkillGroup())
		return;

	if (bMethod == SKILL_UP_BY_POINT)
	{
		int idx = 0;

		switch (pkSk->dwType)
		{
			case 0:
				idx = POINT_SUB_SKILL;
				break;

			case 1:
			case 2:
			case 3:
			case 4:
			case 6:
#ifdef ENABLE_WOLFMAN_CHARACTER
			case 7:
#endif
				idx = POINT_SKILL;
				break;

			case 5:
				idx = POINT_HORSE_SKILL;
				break;

			default:
				sys_err("Wrong skill type %d skill vnum %d", pkSk->dwType, pkSk->dwVnum);
				return;
		}

		if (GetPoint(idx) < 1)
			return;

		PointChange(idx, -1);

#ifdef ENABLE_678TH_SKILL
		if (Is678Skill(dwVnum) && idx == POINT_SKILL)
		{
			int Get678SkillPoints = GetQuestFlag("678th_skill.points");
			SetQuestFlag("678th_skill.points", (Get678SkillPoints + 1));
		}
#endif
	}

	const int SkillPointBefore = GetSkillLevel(pkSk->dwVnum);
	SetSkillLevel(pkSk->dwVnum, m_pSkillLevels[pkSk->dwVnum].bLevel + 1);

	if (pkSk->dwType != 0)
	{
		// Coding that suddenly upgrades
		switch (GetSkillMasterType(pkSk->dwVnum))
		{
			case SKILL_NORMAL:
				// Skill Upgrade 17~20 Random Master Training
				if (GetSkillLevel(pkSk->dwVnum) >= 17)
				{
#ifdef ENABLE_FORCE2MASTERSKILL
					SetSkillLevel(pkSk->dwVnum, 20);
#else
					if (GetQuestFlag("reset_scroll.force_to_master_skill") > 0)
					{
						SetSkillLevel(pkSk->dwVnum, 20);
						SetQuestFlag("reset_scroll.force_to_master_skill", 0);
					}
					else
					{
						if (number(1, 21 - MIN(20, GetSkillLevel(pkSk->dwVnum))) == 1)
							SetSkillLevel(pkSk->dwVnum, 20);
					}
#endif
				}
				break;

			case SKILL_MASTER:
				if (GetSkillLevel(pkSk->dwVnum) >= 30)
				{
					if (number(1, 31 - MIN(30, GetSkillLevel(pkSk->dwVnum))) == 1)
						SetSkillLevel(pkSk->dwVnum, 30);
				}
				break;

			case SKILL_GRAND_MASTER:
				if (GetSkillLevel(pkSk->dwVnum) >= 40)
				{
					SetSkillLevel(pkSk->dwVnum, 40);
				}
				break;

			default:
				break;
		}
	}

	char szSkillUp[1024];

	snprintf(szSkillUp, sizeof(szSkillUp), "SkillUp: %s %u %d %d[Before:%d] type %u",
		GetName(), pkSk->dwVnum, m_pSkillLevels[pkSk->dwVnum].bMasterType, m_pSkillLevels[pkSk->dwVnum].bLevel, SkillPointBefore, pkSk->dwType);

	sys_log(0, "%s", szSkillUp);

	LogManager::Instance().CharLog(this, pkSk->dwVnum, "SKILLUP", szSkillUp);
	Save();

	ComputePoints();
	SkillLevelPacket();

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	CAchievementSystem::Instance().OnMasterSkill(this, pkSk->dwVnum, m_pSkillLevels[pkSk->dwVnum].bLevel);
#endif
}

void CHARACTER::ComputeSkillPoints()
{
	if (g_bSkillDisable)
		return;
}

void CHARACTER::ResetSkill()
{
	if (nullptr == m_pSkillLevels)
		return;

	// Does not reset secondary skills
	std::vector<std::pair<uint32_t, TPlayerSkill> > vec;
	const size_t count = sizeof(s_adwSubSkillVnums) / sizeof(s_adwSubSkillVnums[0]);

	for (size_t i = 0; i < count; ++i)
	{
		if (s_adwSubSkillVnums[i] >= SKILL_MAX_NUM)
			continue;

		vec.emplace_back(std::make_pair(s_adwSubSkillVnums[i], m_pSkillLevels[s_adwSubSkillVnums[i]]));
	}

	memset(m_pSkillLevels, 0, sizeof(TPlayerSkill) * SKILL_MAX_NUM);

	std::vector<std::pair<uint32_t, TPlayerSkill> >::const_iterator iter = vec.begin();

	while (iter != vec.end())
	{
		const std::pair<uint32_t, TPlayerSkill>& pair = *(iter++);
		m_pSkillLevels[pair.first] = pair.second;
	}

	ComputePoints();
	SkillLevelPacket();
}

void CHARACTER::ComputePassiveSkill(uint32_t dwVnum)
{
	if (g_bSkillDisable)
		return;

	if (GetSkillLevel(dwVnum) == 0)
		return;

	CSkillProto* pkSk = CSkillManager::Instance().Get(dwVnum);
	if (!pkSk)
		return;

	pkSk->SetPointVar("k", GetSkillLevel(dwVnum));
	const int iAmount = static_cast<int>(pkSk->kPointPoly.Eval());

	sys_log(2, "%s passive #%d on %d amount %d", GetName(), dwVnum, pkSk->wPointOn, iAmount);
	PointChange(pkSk->wPointOn, iAmount);
}

struct FFindNearVictim
{
	FFindNearVictim(LPCHARACTER center, LPCHARACTER attacker, const CHARACTER_SET& excepts_set = empty_set_)
		: m_pkChrCenter(center),
		m_pkChrNextTarget(nullptr),
		m_pkChrAttacker(attacker),
		m_count(0),
		m_excepts_set(excepts_set)
	{
	}

	void operator ()(LPENTITY ent)
	{
		if (!ent || !ent->IsType(ENTITY_CHARACTER))
			return;

		LPCHARACTER pkChr = dynamic_cast<LPCHARACTER>(ent);
		if (!pkChr)
			return;

		if (!m_excepts_set.empty())
		{
			if (m_excepts_set.find(pkChr) != m_excepts_set.end())
				return;
		}

		if (m_pkChrCenter == pkChr)
			return;

		if (!battle_is_attackable(m_pkChrAttacker, pkChr))
			return;

		if (abs(m_pkChrCenter->GetX() - pkChr->GetX()) > 1000 || abs(m_pkChrCenter->GetY() - pkChr->GetY()) > 1000)
			return;

		const float fDist = static_cast<float>(DISTANCE_APPROX(m_pkChrCenter->GetX() - pkChr->GetX(), m_pkChrCenter->GetY() - pkChr->GetY()));

		if (fDist < 1000)
		{
			++m_count;

			if ((m_count == 1) || number(1, m_count) == 1)
				m_pkChrNextTarget = pkChr;
		}
	}

	LPCHARACTER GetVictim()
	{
		return m_pkChrNextTarget;
	}

	LPCHARACTER m_pkChrCenter;
	LPCHARACTER m_pkChrNextTarget;
	LPCHARACTER m_pkChrAttacker;
	int m_count;
	const CHARACTER_SET& m_excepts_set;
private:
	static CHARACTER_SET empty_set_;
};

CHARACTER_SET FFindNearVictim::empty_set_;

EVENTINFO(chain_lightning_event_info)
{
	uint32_t dwVictim;
	uint32_t dwChr;

	chain_lightning_event_info()
		: dwVictim(0)
		, dwChr(0)
	{
	}
};

EVENTFUNC(ChainLightningEvent)
{
	const chain_lightning_event_info* info = dynamic_cast<chain_lightning_event_info*>(event->info);

	LPCHARACTER pkChrVictim = CHARACTER_MANAGER::Instance().Find(info->dwVictim);
	LPCHARACTER pkChr = CHARACTER_MANAGER::Instance().Find(info->dwChr);
	LPCHARACTER pkTarget = nullptr;

	if (!pkChr || !pkChrVictim)
	{
		sys_log(1, "use chainlighting, but no character");
		return 0;
	}

	sys_log(1, "chainlighting event %s", pkChr->GetName());

	if (pkChrVictim->GetParty()) // Party first
	{
		pkTarget = pkChrVictim->GetParty()->GetNextOwnership(nullptr, pkChrVictim->GetX(), pkChrVictim->GetY());
		if (pkTarget == pkChrVictim || !number(0, 2) || pkChr->GetChainLightingExcept().find(pkTarget) != pkChr->GetChainLightingExcept().end())
			pkTarget = nullptr;
	}

	if (!pkTarget)
	{
		// 1. Find Next victim
		FFindNearVictim f(pkChrVictim, pkChr, pkChr->GetChainLightingExcept());

		if (pkChrVictim->GetSectree())
		{
			pkChrVictim->GetSectree()->ForEachAround(f);
			// 2. If exist, compute it again
			pkTarget = f.GetVictim();
		}
	}

	if (pkTarget)
	{
		pkChrVictim->CreateFly(FLY_CHAIN_LIGHTNING, pkTarget);
		pkChr->ComputeSkill(SKILL_CHAIN, pkTarget);
		pkChr->AddChainLightningExcept(pkTarget);
	}
	else
	{
		sys_log(1, "%s use chainlighting, but find victim failed near %s", pkChr->GetName(), pkChrVictim->GetName());
	}

	return 0;
}

void SetPolyVarForAttack(LPCHARACTER ch, CSkillProto* pkSk, LPITEM pkWeapon)
{
	if (!ch)
		return;

	if (ch->IsPC())
	{
		if (pkWeapon && pkWeapon->GetType() == ITEM_WEAPON)
		{
			int iWep = number(pkWeapon->GetValue(3), pkWeapon->GetValue(4));
			iWep += pkWeapon->GetValue(5);

			int iMtk = number(pkWeapon->GetValue(1), pkWeapon->GetValue(2));
			iMtk += pkWeapon->GetValue(5);

			pkSk->SetPointVar("wep", iWep);
			pkSk->SetPointVar("mtk", iMtk);
			pkSk->SetPointVar("mwep", iMtk);
		}
		else
		{
			pkSk->SetPointVar("wep", 0);
			pkSk->SetPointVar("mtk", 0);
			pkSk->SetPointVar("mwep", 0);
		}
	}
	else
	{
		const int iWep = number(ch->GetMobDamageMin(), ch->GetMobDamageMax());
		pkSk->SetPointVar("wep", iWep);
		pkSk->SetPointVar("mwep", iWep);
		pkSk->SetPointVar("mtk", iWep);
	}
}

struct FuncSplashDamage
{
	FuncSplashDamage(int x, int y, CSkillProto* pkSk, LPCHARACTER pkChr, int iAmount, int iAG, int iMaxHit, LPITEM pkWeapon, bool bDisableCooltime, TSkillUseInfo* pInfo, uint8_t bUseSkillPower) noexcept
		:
		m_x(x), m_y(y), m_pkSk(pkSk), m_pkChr(pkChr), m_iAmount(iAmount), m_iAG(iAG), m_iCount(0), m_iMaxHit(iMaxHit), m_pkWeapon(pkWeapon), m_bDisableCooltime(bDisableCooltime), m_pInfo(pInfo), m_bUseSkillPower(bUseSkillPower)
	{
	}

	void operator () (LPENTITY ent)
	{
		if (!ent->IsType(ENTITY_CHARACTER))
		{
			//if (m_pkSk->dwVnum == SKILL_CHAIN) sys_log(0, "CHAIN target not character %s", m_pkChr->GetName());
			return;
		}

		LPCHARACTER pkChrVictim = dynamic_cast<LPCHARACTER>(ent);
		if (!pkChrVictim)
			return;

		if (DISTANCE_APPROX(m_x - pkChrVictim->GetX(), m_y - pkChrVictim->GetY()) > m_pkSk->iSplashRange)
		{
			if (test_server)
				sys_log(0, "XXX target too far %s", m_pkChr->GetName());
			return;
		}

		if (!battle_is_attackable(m_pkChr, pkChrVictim))
		{
			if (test_server)
				sys_log(0, "XXX target not attackable %s", m_pkChr->GetName());
			return;
		}

#ifdef ENABLE_AFTERDEATH_SHIELD
		if (m_pkChr->IsAffectFlag(AFF_AFTERDEATH_SHIELD))
			m_pkChr->RemoveShieldAffect();
#endif

		if (m_pkChr->IsPC())
		{
			// Guild skills do not handle cooldown.
			if (!(m_pkSk->dwVnum >= GUILD_SKILL_START && m_pkSk->dwVnum <= GUILD_SKILL_END))
			{
				if (!m_bDisableCooltime && m_pInfo && !m_pInfo->HitOnce(m_pkSk->dwVnum) && m_pkSk->dwVnum != SKILL_MUYEONG)
				{
					if (test_server)
						sys_log(0, "check guild skill %s", m_pkChr->GetName());
					return;
				}
			}
		}

		++m_iCount;

		int iDam;

		////////////////////////////////////////////////////////////////////////////////
		//float k = 1.0f * m_pkChr->GetSkillPower(m_pkSk->dwVnum) * m_pkSk->bMaxLevel / 100;
		//m_pkSk->kPointPoly2.SetVar("k", 1.0 * m_bUseSkillPower * m_pkSk->bMaxLevel / 100);
		m_pkSk->SetPointVar("k", 1.0 * m_bUseSkillPower * m_pkSk->bMaxLevel / 100);
		m_pkSk->SetPointVar("lv", m_pkChr->GetLevel());
		m_pkSk->SetPointVar("iq", m_pkChr->GetPoint(POINT_IQ));
		m_pkSk->SetPointVar("str", m_pkChr->GetPoint(POINT_ST));
		m_pkSk->SetPointVar("dex", m_pkChr->GetPoint(POINT_DX));
		m_pkSk->SetPointVar("con", m_pkChr->GetPoint(POINT_HT));
		m_pkSk->SetPointVar("def", m_pkChr->GetPoint(POINT_DEF_GRADE));
		m_pkSk->SetPointVar("odef", m_pkChr->GetPoint(POINT_DEF_GRADE) - m_pkChr->GetPoint(POINT_DEF_GRADE_BONUS));
		m_pkSk->SetPointVar("horse_level", m_pkChr->GetHorseLevel());

		//int iPenetratePct = (int)(1 + k*4);
		bool bIgnoreDefense = false;

		if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_PENETRATE))
		{
			const int iPenetratePct = static_cast<int>(m_pkSk->kPointPoly2.Eval());

			if (number(1, 100) <= iPenetratePct)
				bIgnoreDefense = true;
		}

		bool bIgnoreTargetRating = false;

		if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_IGNORE_TARGET_RATING))
		{
			const int iPct = static_cast<int>(m_pkSk->kPointPoly2.Eval());

			if (number(1, 100) <= iPct)
				bIgnoreTargetRating = true;
		}

		m_pkSk->SetPointVar("ar", CalcAttackRating(m_pkChr, pkChrVictim, bIgnoreTargetRating));

		if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_USE_MELEE_DAMAGE))
			m_pkSk->SetPointVar("atk", CalcMeleeDamage(m_pkChr, pkChrVictim, true, bIgnoreTargetRating));
		else if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_USE_ARROW_DAMAGE))
		{
			LPITEM pkBow, pkArrow;

			if (1 == m_pkChr->GetArrowAndBow(&pkBow, &pkArrow, 1))
				m_pkSk->SetPointVar("atk", CalcArrowDamage(m_pkChr, pkChrVictim, pkBow, pkArrow, true));
			else
				m_pkSk->SetPointVar("atk", 0);
		}

		if (m_pkSk->wPointOn == POINT_MOV_SPEED)
			m_pkSk->kPointPoly.SetVar("maxv", pkChrVictim->GetLimitPoint(POINT_MOV_SPEED));

		m_pkSk->SetPointVar("maxhp", pkChrVictim->GetMaxHP());
		m_pkSk->SetPointVar("maxsp", pkChrVictim->GetMaxSP());

		m_pkSk->SetPointVar("chain", m_pkChr->GetChainLightningIndex());
		m_pkChr->IncChainLightningIndex();

		const bool bUnderEunhyung = m_pkChr->GetAffectedEunhyung(); // Why do you do this here?? --> (m_pkChr->GetAffectedEunhyung() > 0)

		m_pkSk->SetPointVar("ek", m_pkChr->GetAffectedEunhyung() * 1. / 100);
		//m_pkChr->ClearAffectedEunhyung();
		SetPolyVarForAttack(m_pkChr, m_pkSk, m_pkWeapon);

		int iAmount = 0;

		if (m_pkChr->GetUsedSkillMasterType(m_pkSk->dwVnum) >= SKILL_GRAND_MASTER)
			iAmount = static_cast<int>(m_pkSk->kMasterBonusPoly.Eval());
		else
			iAmount = static_cast<int>(m_pkSk->kPointPoly.Eval());

		if (test_server && iAmount == 0 && m_pkSk->wPointOn != POINT_NONE)
			m_pkChr->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]There is no effect. Check your skill formula"));
		////////////////////////////////////////////////////////////////////////////////
		iAmount = -iAmount;

		if (m_pkSk->dwVnum == SKILL_AMSEOP)	// Hinterhalt
		{
			const float fDelta = GetDegreeDelta(m_pkChr->GetRotation(), pkChrVictim->GetRotation());
			float adjust = 0.0f;

			if (fDelta < 35.0f)
			{
				adjust = 1.5f;

				if (bUnderEunhyung)
					adjust += 0.5f;

				if (m_pkChr->GetWear(WEAR_WEAPON) && m_pkChr->GetWear(WEAR_WEAPON)->GetSubType() == WEAPON_DAGGER)
				{
					adjust += 0.5f;
				}
			}
			else
			{
				adjust = 1.0f;

				if (bUnderEunhyung)
					adjust += 0.5f;

				if (m_pkChr->GetWear(WEAR_WEAPON) && m_pkChr->GetWear(WEAR_WEAPON)->GetSubType() == WEAPON_DAGGER)
					adjust += 0.5f;
			}

			iAmount = static_cast<int>(iAmount * adjust);
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
			if (m_pkChr->GetPoint(POINT_SKILL_DAMAGE_AMSEOP))	// Schaden von Hinterhalt
				iAmount = iAmount + (iAmount * m_pkChr->GetPoint(POINT_SKILL_DAMAGE_AMSEOP)) / 100;
#endif
		}
		else if (m_pkSk->dwVnum == SKILL_GUNGSIN)	// Blitzangriff
		{
			float adjust = 1.0;

			if (m_pkChr->GetWear(WEAR_WEAPON) && m_pkChr->GetWear(WEAR_WEAPON)->GetSubType() == WEAPON_DAGGER)
			{
				adjust = 1.35f;
			}

			iAmount = static_cast<int>(iAmount * adjust);
		}
#ifdef ENABLE_WOLFMAN_CHARACTER
		else if (m_pkSk->dwVnum == SKILL_GONGDAB)	// Wolfssprung
		{
			float adjust = 1.0;

			if (m_pkChr->GetWear(WEAR_WEAPON) && m_pkChr->GetWear(WEAR_WEAPON)->GetSubType() == WEAPON_CLAW)
			{
				adjust = 1.35f;
			}

			iAmount = static_cast<int>(iAmount * adjust);
		}
#endif

#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
		if (m_pkSk->dwVnum == SKILL_SAMYEON)	// Dreiwege-Schnitt
		{
			float adjust = 1.0;

			if (m_pkChr->GetPoint(POINT_SKILL_DAMAGE_SAMYEON))
			{
				float adjust = 2.0;
			}

			iAmount = static_cast<int>(iAmount * adjust);
		}
#endif

		////////////////////////////////////////////////////////////////////////////////
		//sys_log(0, "name: %s skill: %s amount %d to %s", m_pkChr->GetName(), m_pkSk->szName, iAmount, pkChrVictim->GetName());

		iDam = CalcBattleDamage(iAmount, m_pkChr->GetLevel(), pkChrVictim->GetLevel());

		if (m_pkChr->IsPC() && m_pkChr->m_SkillUseInfo[m_pkSk->dwVnum].GetMainTargetVID() != (uint32_t)pkChrVictim->GetVID())
		{
			// Damage reduction
			iDam = static_cast<int>(iDam * m_pkSk->kSplashAroundDamageAdjustPoly.Eval());
		}

		// Damage type according to TODO skill must be recorded.
		EDamageType dt = EDamageType::DAMAGE_TYPE_NONE;

		switch (m_pkSk->bSkillAttrType)
		{
			case SKILL_ATTR_TYPE_NORMAL:
				break;

			case SKILL_ATTR_TYPE_MELEE:
			{
				dt = EDamageType::DAMAGE_TYPE_MELEE;

				const LPITEM& pkWeapon = m_pkChr->GetWear(WEAR_WEAPON);

				if (pkWeapon)
				{
					switch (pkWeapon->GetSubType())
					{
						case WEAPON_SWORD:
							iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_SWORD)) / 100;
							break;

						case WEAPON_TWO_HANDED:
							iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_TWOHAND)) / 100;
							// Two-handed sword penalty 10%
							//iDam = iDam * 95 / 100;
							break;

						case WEAPON_DAGGER:
							iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_DAGGER)) / 100;
							break;

						case WEAPON_BELL:
							iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_BELL)) / 100;
							break;

						case WEAPON_FAN:
							iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_FAN)) / 100;
							break;
#ifdef ENABLE_WOLFMAN_CHARACTER
						case WEAPON_CLAW:
							iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_CLAW)) / 100;
							break;
#endif
						default:
							break;
					}
				}

				if (!bIgnoreDefense)
					iDam -= pkChrVictim->GetPoint(POINT_DEF_GRADE);
			}
			break;

			case SKILL_ATTR_TYPE_RANGE:
				dt = EDamageType::DAMAGE_TYPE_RANGE;
				// Ahhhhhhhhh
				// There is a bug that has not been applied before, so if you recalculate your defense, the user will be upset
				//iDam -= pkChrVictim->GetPoint(POINT_DEF_GRADE);
				iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_BOW)) / 100;
				break;

			case SKILL_ATTR_TYPE_MAGIC:
				dt = EDamageType::DAMAGE_TYPE_MAGIC;
				iDam = CalcAttBonus(m_pkChr, pkChrVictim, iDam);
				// Ahhhhhhhhh
				// There is a bug that has not been applied before, so if you recalculate your defense, the user will be upset
				//iDam -= pkChrVictim->GetPoint(POINT_MAGIC_DEF_GRADE);
#ifdef ENABLE_MAGIC_REDUCTION_SYSTEM
				{
					const int resist_magic = MINMAX(0, pkChrVictim->GetPoint(POINT_RESIST_MAGIC), 100);
					const int resist_magic_reduction = MINMAX(0, (m_pkChr->GetJob() == JOB_SURA) ? m_pkChr->GetPoint(POINT_RESIST_MAGIC_REDUCTION) / 2 : m_pkChr->GetPoint(POINT_RESIST_MAGIC_REDUCTION), 50);
					const int total_res_magic = MINMAX(0, resist_magic - resist_magic_reduction, 100);
					iDam = iDam * (100 - total_res_magic) / 100;
				}
#else
				iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_MAGIC)) / 100;
#endif
				break;

			default:
				sys_err("Unknown skill attr type %u vnum %u", m_pkSk->bSkillAttrType, m_pkSk->dwVnum);
				break;
		}

		//
		// 20091109 German skill attribute request operation
		// Skills with SKILL_FLAG_WIND, SKILL_FLAG_ELEC, and SKILL_FLAG_FIRE in the existing skill table
		// There was no such thing at all, so the RESIST_WIND, RESIST_ELEC, and RESIST_FIRE of the monster were not used.
		//
		// In order to separate the PvP and PvE balance, only NPCs were intentionally applied, and the difference from the existing balance
		// To not feel, mob_proto's RESIST_MAGIC to RESIST_WIND, RESIST_ELEC, RESIST_FIRE
		// copied.
		//
		if (pkChrVictim->IsNPC())
		{
			if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_WIND))
			{
				iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_WIND)) / 100;
			}

			if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_ELEC))
			{
				iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_ELEC)) / 100;
			}

			if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_FIRE))
			{
				iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_FIRE)) / 100;
			}
		}

#ifdef ENABLE_DAWNMIST_DUNGEON
		if (CDawnMistDungeon::Instance().IsForestDungeonBoss(m_pkChr) && m_pkSk->dwVnum == CDawnMistDungeon::SPECIAL_SKILL_VNUM)
			iDam += CDawnMistDungeon::Instance().GetDamageIncreaserValue(m_pkChr, iDam, pkChrVictim->GetJob());
#endif

		if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_COMPUTE_MAGIC_DAMAGE))
			dt = EDamageType::DAMAGE_TYPE_MAGIC;

		if (pkChrVictim->CanBeginFight())
			pkChrVictim->BeginFight(m_pkChr);

		if (m_pkSk->dwVnum == SKILL_CHAIN)
			sys_log(0, "%s CHAIN INDEX %d DAM %d DT %d", m_pkChr->GetName(), m_pkChr->GetChainLightningIndex() - 1, iDam, dt);

#ifdef ENABLE_78TH_SKILL
		{	// Boost-Fertigkeiten
			uint8_t HELP_SKILL_ID = 0;
			switch (m_pkSk->dwVnum)
			{
				case SKILL_PALBANG:
					HELP_SKILL_ID = SKILL_HELP_PALBANG;
					break;

				case SKILL_AMSEOP:
					HELP_SKILL_ID = SKILL_HELP_AMSEOP;
					break;

				case SKILL_SWAERYUNG:
					HELP_SKILL_ID = SKILL_HELP_SWAERYUNG;
					break;

				case SKILL_YONGBI:
					HELP_SKILL_ID = SKILL_HELP_YONGBI;
					break;

				case SKILL_GIGONGCHAM:
					HELP_SKILL_ID = SKILL_HELP_GIGONGCHAM;
					break;

				case SKILL_HWAJO:
					HELP_SKILL_ID = SKILL_HELP_HWAJO;
					break;

				case SKILL_MARYUNG:
					HELP_SKILL_ID = SKILL_HELP_MARYUNG;
					break;

				case SKILL_BYEURAK:
					HELP_SKILL_ID = SKILL_HELP_BYEURAK;
					break;

#	ifdef ENABLE_WOLFMAN_CHARACTER
				case SKILL_SALPOONG:
					HELP_SKILL_ID = SKILL_HELP_SALPOONG;
					break;
#	endif

				default:
					break;
			}

			if (HELP_SKILL_ID != 0)
			{
				const uint8_t HELP_SKILL_LV = m_pkChr->GetSkillLevel(HELP_SKILL_ID);
				if (HELP_SKILL_LV != 0)
				{
					CSkillProto* pkSk = CSkillManager::Instance().Get(HELP_SKILL_ID);
					if (!pkSk)
						sys_err("Can't find %d skill in skill_proto.", HELP_SKILL_ID);
					else
					{
						pkSk->SetPointVar("k", 1.0f * m_pkChr->GetSkillPower(HELP_SKILL_ID) * pkSk->bMaxLevel / 100);

						const double IncreaseAmount = pkSk->kPointPoly.Eval();
						sys_log(0, "HELP_SKILL: increase amount: %lf, normal damage: %d, increased damage: %d.", IncreaseAmount, iDam, int(iDam * (IncreaseAmount / 100.0)));
						iDam += iDam * static_cast<int>(IncreaseAmount / 100.0);
					}
				}
			}
		}	// END_Boost-Fertigkeiten

		{	// Konter-Fertigkeiten
			uint8_t ANTI_SKILL_ID = 0;
			switch (m_pkSk->dwVnum)
			{
				case SKILL_PALBANG:
					ANTI_SKILL_ID = SKILL_ANTI_PALBANG;
					break;

				case SKILL_AMSEOP:
					ANTI_SKILL_ID = SKILL_ANTI_AMSEOP;
					break;

				case SKILL_SWAERYUNG:
					ANTI_SKILL_ID = SKILL_ANTI_SWAERYUNG;
					break;

				case SKILL_YONGBI:
					ANTI_SKILL_ID = SKILL_ANTI_YONGBI;
					break;

				case SKILL_GIGONGCHAM:
					ANTI_SKILL_ID = SKILL_ANTI_GIGONGCHAM;
					break;

				case SKILL_HWAJO:
					ANTI_SKILL_ID = SKILL_ANTI_HWAJO;
					break;

				case SKILL_MARYUNG:
					ANTI_SKILL_ID = SKILL_ANTI_MARYUNG;
					break;

				case SKILL_BYEURAK:
					ANTI_SKILL_ID = SKILL_ANTI_BYEURAK;
					break;

#	ifdef ENABLE_WOLFMAN_CHARACTER
				case SKILL_SALPOONG:
					ANTI_SKILL_ID = SKILL_ANTI_SALPOONG;
					break;
#	endif
				default:
					break;
			}

			if (ANTI_SKILL_ID != 0)
			{
				const uint8_t ANTI_SKILL_LV = pkChrVictim->GetSkillLevel(ANTI_SKILL_ID);
				if (ANTI_SKILL_LV != 0)
				{
					CSkillProto* pkSk = CSkillManager::Instance().Get(ANTI_SKILL_ID);
					if (!pkSk)
						sys_err("Can't find %d skill in skill_proto.", ANTI_SKILL_ID);
					else
					{
						pkSk->SetPointVar("k", 1.0f * pkChrVictim->GetSkillPower(ANTI_SKILL_ID) * pkSk->bMaxLevel / 100);

						const double ResistAmount = pkSk->kPointPoly.Eval();
						sys_log(0, "ANTI_SKILL: resist amount: %lf, normal damage: %d, reduced damage: %d.", ResistAmount, iDam, int(iDam * (ResistAmount / 100.0)));
						iDam -= iDam * static_cast<int>(ResistAmount / 100.0);
					}
				}
			}
		}	// END_Konter-Fertigkeiten
#endif

		{
			uint8_t AntiSkillID = 0;

			switch (m_pkSk->dwVnum)
			{
				case SKILL_TANHWAN:
					AntiSkillID = SKILL_7_A_ANTI_TANHWAN;
					break;

				case SKILL_AMSEOP:
					AntiSkillID = SKILL_7_B_ANTI_AMSEOP;
					break;

				case SKILL_SWAERYUNG:
					AntiSkillID = SKILL_7_C_ANTI_SWAERYUNG;
					break;

				case SKILL_YONGBI:
					AntiSkillID = SKILL_7_D_ANTI_YONGBI;
					break;

				case SKILL_GIGONGCHAM:
					AntiSkillID = SKILL_8_A_ANTI_GIGONGCHAM;
					break;

				case SKILL_YEONSA:
					AntiSkillID = SKILL_8_B_ANTI_YEONSA;
					break;

				case SKILL_MAHWAN:
					AntiSkillID = SKILL_8_C_ANTI_MAHWAN;
					break;

				case SKILL_BYEURAK:
					AntiSkillID = SKILL_8_D_ANTI_BYEURAK;
					break;

				default:
					break;
			}

			if (0 != AntiSkillID)
			{
				const uint8_t AntiSkillLevel = pkChrVictim->GetSkillLevel(AntiSkillID);

				if (0 != AntiSkillLevel)
				{
					CSkillProto* pkSk = CSkillManager::Instance().Get(AntiSkillID);
					if (!pkSk)
					{
						sys_err("There is no anti skill(%d) in skill proto", AntiSkillID);
					}
					else
					{
						pkSk->SetPointVar("k", 1.0f * pkChrVictim->GetSkillPower(AntiSkillID) * pkSk->bMaxLevel / 100);

						const double ResistAmount = pkSk->kPointPoly.Eval();

						sys_log(0, "ANTI_SKILL: Resist(%lf) Orig(%d) Reduce(%d)", ResistAmount, iDam, int(iDam * (ResistAmount / 100.0)));

						iDam -= iDam * static_cast<int>(ResistAmount / 100.0);
					}
				}
			}
		}

		if (!pkChrVictim->Damage(m_pkChr, iDam, dt) && !pkChrVictim->IsStun())
		{
#ifdef ENABLE_PVP_BALANCE
			if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_KNOCKBACK) && !pkChrVictim->IsStone()
#ifdef ENABLE_DEFENSE_WAVE
				&& !pkChrVictim->IsHydraBoss()
#endif
				)
			{
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
				CAchievementSystem::Instance().DamageDealt(m_pkChr, pkChrVictim, m_pkSk->dwVnum, iDam);
#endif

				constexpr float fknockbackLength = 300.0f; // Distance for KnockBack, defult 300

				float fx, fy;
				float degree = GetDegreeFromPositionXY(m_pkChr->GetX(), m_pkChr->GetY(), pkChrVictim->GetX(), pkChrVictim->GetY());

				if (m_pkSk->dwVnum == SKILL_HORSE_WILDATTACK)
				{
					degree -= m_pkChr->GetRotation();
					degree = fmod(degree, 360.0f) - 180.0f;

					if (degree > 0)
						degree = m_pkChr->GetRotation() + 90.0f;
					else
						degree = m_pkChr->GetRotation() - 90.0f;
				}

				GetDeltaByDegree(degree, fknockbackLength, &fx, &fy);
				sys_log(0, "KNOCKBACK! %s -> %s (%d %d) -> (%d %d)", m_pkChr->GetName(), pkChrVictim->GetName(), pkChrVictim->GetX(), pkChrVictim->GetY(), (long)(pkChrVictim->GetX() + fx), (long)(pkChrVictim->GetY() + fy));
				const long tx = static_cast<long>(pkChrVictim->GetX() + fx);
				const long ty = static_cast<long>(pkChrVictim->GetY() + fy);

				pkChrVictim->Sync(tx, ty);
				pkChrVictim->Goto(tx, ty);
				pkChrVictim->CalculateMoveDuration();

				pkChrVictim->SyncPacket();
			}
#endif

			if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_REMOVE_GOOD_AFFECT))
			{
#ifdef ENABLE_NULLIFYAFFECT_LIMIT
				const int iLevel = m_pkChr->GetLevel();
				const int yLevel = pkChrVictim->GetLevel();
				// const float k = 1.0 * m_pkChr->GetSkillPower(m_pkSk->dwVnum, bSkillLevel) * m_pkSk->bMaxLevel / 100;
				constexpr int iDifLev = 9;
				if ((iLevel - iDifLev <= yLevel) && (iLevel + iDifLev >= yLevel))
#endif
				{
					const int iAmount2 = static_cast<int>(m_pkSk->kPointPoly2.Eval());
					int iDur2 = static_cast<int>(m_pkSk->kDurationPoly2.Eval());
					iDur2 += m_pkChr->GetPoint(POINT_PARTY_BUFFER_BONUS);

					if (number(1, 100) <= iAmount2)
					{
						pkChrVictim->RemoveGoodAffect();
						pkChrVictim->AddAffect(m_pkSk->dwVnum, POINT_NONE, 0, AFF_PABEOP, iDur2, 0, true);
					}
				}
			}
#ifdef ENABLE_WOLFMAN_CHARACTER
			if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_SLOW | SKILL_FLAG_STUN | SKILL_FLAG_FIRE_CONT | SKILL_FLAG_POISON | SKILL_FLAG_BLEEDING))
#else
			if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_SLOW | SKILL_FLAG_STUN | SKILL_FLAG_FIRE_CONT | SKILL_FLAG_POISON))
#endif
			{
				int iPct = static_cast<int>(m_pkSk->kPointPoly2.Eval());
				int iDur = static_cast<int>(m_pkSk->kDurationPoly2.Eval());

#ifdef ENABLE_PVP_BALANCE
				// Set a min-max value for party role bonus just when flag is stun.
				constexpr uint16_t iMaxStunDuration = 20;
				if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_STUN))
				{
					iDur += MINMAX(static_cast<int>(m_pkSk->kDurationPoly2.Eval()), m_pkChr->GetPoint(POINT_PARTY_BUFFER_BONUS), iMaxStunDuration);
				}
				else
				{
					// SLOW, FIRE_CONT, POISON, BLEEDING will still be working with the default calculation.
					iDur += m_pkChr->GetPoint(POINT_PARTY_BUFFER_BONUS);
				}
#else
				iDur += m_pkChr->GetPoint(POINT_PARTY_BUFFER_BONUS);
#endif

				if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_STUN))
				{
					SkillAttackAffect(pkChrVictim, iPct, IMMUNE_STUN, AFFECT_STUN, POINT_NONE, 0, AFF_STUN, iDur, m_pkSk->szName);
				}
				else if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_SLOW))
				{
					SkillAttackAffect(pkChrVictim, iPct, IMMUNE_SLOW, AFFECT_SLOW, POINT_MOV_SPEED, -30, AFF_SLOW, iDur, m_pkSk->szName);
				}
				else if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_FIRE_CONT))
				{
					m_pkSk->SetDurationVar("k", 1.0 * m_bUseSkillPower * m_pkSk->bMaxLevel / 100);
					m_pkSk->SetDurationVar("iq", m_pkChr->GetPoint(POINT_IQ));

					iDur = static_cast<int>(m_pkSk->kDurationPoly2.Eval());
					const int bonus = m_pkChr->GetPoint(POINT_PARTY_BUFFER_BONUS);

					if (bonus != 0)
					{
						iDur += bonus / 2;
					}

					if (number(1, 100) <= iDur)
					{
#ifdef ENABLE_YOHARA_SYSTEM
						if (pkChrVictim->IsSungmaMap() && (pkChrVictim->GetPoint(POINT_SUNGMA_IMMUNE) < pkChrVictim->GetSungmaMapAttribute(POINT_SUNGMA_IMMUNE)) && iPct >= 2)
							iPct /= 2;
#endif

						pkChrVictim->AttackedByFire(m_pkChr, iPct, 5);
					}
				}
				else if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_POISON))
				{
#ifdef ENABLE_YOHARA_SYSTEM
					if (pkChrVictim->IsSungmaMap() && (pkChrVictim->GetPoint(POINT_SUNGMA_IMMUNE) < pkChrVictim->GetSungmaMapAttribute(POINT_SUNGMA_IMMUNE)) && iPct >= 2)
						iPct /= 2;
#endif

					if (number(1, 100) <= iPct)
						pkChrVictim->AttackedByPoison(m_pkChr);
				}
#ifdef ENABLE_WOLFMAN_CHARACTER
				else if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_BLEEDING))
				{
#ifdef ENABLE_YOHARA_SYSTEM
					if (pkChrVictim->IsSungmaMap() && (pkChrVictim->GetPoint(POINT_SUNGMA_IMMUNE) < pkChrVictim->GetSungmaMapAttribute(POINT_SUNGMA_IMMUNE)) && iPct >= 2)
						iPct /= 2;
#endif

					if (number(1, 100) <= iPct)
						pkChrVictim->AttackedByBleeding(m_pkChr);
				}
#endif
			}

			if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_CRUSH | SKILL_FLAG_CRUSH_LONG) &&
				!IS_SET(pkChrVictim->GetAIFlag(), AIFLAG_NOMOVE))
			{
				float fCrushSlidingLength = 200;

				if (m_pkChr->IsNPC())
					fCrushSlidingLength = 400;

				if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_CRUSH_LONG))
					fCrushSlidingLength *= 2;

				float fx, fy;
				float degree = GetDegreeFromPositionXY(m_pkChr->GetX(), m_pkChr->GetY(), pkChrVictim->GetX(), pkChrVictim->GetY());

				if (m_pkSk->dwVnum == SKILL_HORSE_WILDATTACK)
				{
					degree -= m_pkChr->GetRotation();
					degree = fmod(degree, 360.0f) - 180.0f;

					if (degree > 0)
						degree = m_pkChr->GetRotation() + 90.0f;
					else
						degree = m_pkChr->GetRotation() - 90.0f;
				}

				GetDeltaByDegree(degree, fCrushSlidingLength, &fx, &fy);
				sys_log(0, "CRUSH! %s -> %s (%d %d) -> (%d %d)", m_pkChr->GetName(), pkChrVictim->GetName(), pkChrVictim->GetX(), pkChrVictim->GetY(), (long)(pkChrVictim->GetX() + fx), (long)(pkChrVictim->GetY() + fy));
				long tx = static_cast<long>(pkChrVictim->GetX() + fx);
				long ty = static_cast<long>(pkChrVictim->GetY() + fy);

				//@fixme458
				//While toX and toY are not allowed attr, reduce the slidingLength and recalculate the endpoint
				while (pkChrVictim->GetSectree()->GetAttribute(tx, ty) & (ATTR_BLOCK | ATTR_OBJECT) && fCrushSlidingLength > 0) {
					if (fCrushSlidingLength >= 10) {
						fCrushSlidingLength -= 10;
					}
					else {
						fCrushSlidingLength = 0;
					}
					GetDeltaByDegree(degree, fCrushSlidingLength, &fx, &fy);
					tx = (long)(pkChrVictim->GetX() + fx);
					ty = (long)(pkChrVictim->GetY() + fy);
				}
				//@end_fixme458

				pkChrVictim->Sync(tx, ty);
				pkChrVictim->Goto(tx, ty);
				pkChrVictim->CalculateMoveDuration();

				if (m_pkChr->IsPC() && m_pkChr->m_SkillUseInfo[m_pkSk->dwVnum].GetMainTargetVID() == (uint32_t)pkChrVictim->GetVID())
				{
					SkillAttackAffect(pkChrVictim, 1000, IMMUNE_STUN, m_pkSk->dwVnum, POINT_NONE, 0, AFF_STUN, 4, m_pkSk->szName);
				}
				else
				{
					pkChrVictim->SyncPacket();
				}
			}
		}

		if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_HP_ABSORB))
		{
			const int iPct = static_cast<int>(m_pkSk->kPointPoly2.Eval());
			m_pkChr->PointChange(POINT_HP, iDam * iPct / 100);
		}

		if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_SP_ABSORB))
		{
			const int iPct = static_cast<int>(m_pkSk->kPointPoly2.Eval());
			m_pkChr->PointChange(POINT_SP, iDam * iPct / 100);
		}

		if (m_pkSk->dwVnum == SKILL_CHAIN && m_pkChr->GetChainLightningIndex() < m_pkChr->GetChainLightningMaxCount())
		{
			chain_lightning_event_info* info = AllocEventInfo<chain_lightning_event_info>();

			info->dwVictim = pkChrVictim->GetVID();
			info->dwChr = m_pkChr->GetVID();

			event_create(ChainLightningEvent, info, passes_per_sec / 5);
		}
		if (test_server)
			sys_log(0, "FuncSplashDamage End :%s ", m_pkChr->GetName());
	}

	int m_x;
	int m_y;
	CSkillProto* m_pkSk;
	LPCHARACTER m_pkChr;
	int m_iAmount;
	int m_iAG;
	int m_iCount;
	int m_iMaxHit;
	LPITEM m_pkWeapon;
	bool m_bDisableCooltime;
	TSkillUseInfo* m_pInfo;
	uint8_t m_bUseSkillPower;
};

struct FuncSplashAffect
{
	FuncSplashAffect(LPCHARACTER ch, int x, int y, int iDist, uint32_t dwVnum, uint16_t wPointOn, int iAmount, uint32_t dwAffectFlag, int iDuration, int iSPCost, bool bOverride, int iMaxHit)	//@fixme532
	{
		m_x = x;
		m_y = y;
		m_iDist = iDist;
		m_dwVnum = dwVnum;
		m_wPointOn = wPointOn;	//@fixme532
		m_iAmount = iAmount;
		m_dwAffectFlag = dwAffectFlag;
		m_iDuration = iDuration;
		m_iSPCost = iSPCost;
		m_bOverride = bOverride;
		m_pkChrAttacker = ch;
		m_iMaxHit = iMaxHit;
		m_iCount = 0;
	}

	void operator () (LPENTITY ent)
	{
		if (m_iMaxHit && m_iMaxHit <= m_iCount)
			return;

		if (ent && ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER pkChr = dynamic_cast<LPCHARACTER>(ent);
			if (!pkChr)
				return;

			if (test_server)
				sys_log(0, "FuncSplashAffect step 1 : name:%s vnum:%d iDur:%d", pkChr->GetName(), m_dwVnum, m_iDuration);
			if (DISTANCE_APPROX(m_x - pkChr->GetX(), m_y - pkChr->GetY()) < m_iDist)
			{
				if (test_server)
					sys_log(0, "FuncSplashAffect step 2 : name:%s vnum:%d iDur:%d", pkChr->GetName(), m_dwVnum, m_iDuration);
				if (m_dwVnum == SKILL_TUSOK)
					if (pkChr->CanBeginFight())
						pkChr->BeginFight(m_pkChrAttacker);

				if (pkChr->IsPC() && m_dwVnum == SKILL_TUSOK)
					pkChr->AddAffect(m_dwVnum, m_wPointOn, m_iAmount, m_dwAffectFlag, m_iDuration / 3, m_iSPCost, m_bOverride);
				else
					pkChr->AddAffect(m_dwVnum, m_wPointOn, m_iAmount, m_dwAffectFlag, m_iDuration, m_iSPCost, m_bOverride);

				m_iCount++;
			}
		}
	}

	LPCHARACTER m_pkChrAttacker;
	int m_x;
	int m_y;
	int m_iDist;
	uint32_t m_dwVnum;
	uint16_t m_wPointOn;	//@fixme532
	int m_iAmount;
	uint32_t m_dwAffectFlag;
	int m_iDuration;
	int m_iSPCost;
	bool m_bOverride;
	int m_iMaxHit;
	int m_iCount;
};

struct RemoveInvisibleVictim	//@fixme512
{
	RemoveInvisibleVictim(LPCHARACTER pkOldVictim) {
		m_pkOldVictim = pkOldVictim;
	}

	void operator () (LPENTITY ent) {
		if (ent->IsType(ENTITY_CHARACTER)) {
			LPCHARACTER pkChr = (LPCHARACTER)ent;
			if (pkChr && pkChr->IsMonster()) {
				LPCHARACTER pkVictim = pkChr->GetVictim();
				if (pkVictim && pkVictim == m_pkOldVictim) {
					LPCHARACTER new_victim = pkChr->GetNearestVictim(pkChr); // or optional GetHighestDpsVictim(pkChr)
					if (new_victim != m_pkOldVictim)	//NEED TO ADD CHECK HERE AFF_EUNHYUNG WILL BE SET LATER
						pkChr->SetVictim(new_victim);
					else
						pkChr->SetVictim(nullptr);
				}
			}
		}
	}
	LPCHARACTER m_pkOldVictim;
};

EVENTINFO(skill_gwihwan_info)
{
	uint32_t pid;
	uint8_t bsklv;

	skill_gwihwan_info()
		: pid(0)
		, bsklv(0)
	{
	}
};

EVENTFUNC(skill_gwihwan_event)
{
	const skill_gwihwan_info* info = dynamic_cast<skill_gwihwan_info*>(event->info);

	if (info == nullptr)
	{
		sys_err("skill_gwihwan_event> <Factor> Null pointer");
		return 0;
	}

	const uint32_t pid = info->pid;
	const uint8_t sklv = info->bsklv;
	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(pid);

	if (!ch)
		return 0;

	const int percent = 20 * sklv - 1;

	if (number(1, 100) <= percent)
	{
		PIXEL_POSITION pos;

		// success
		if (SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(ch->GetMapIndex(), ch->GetEmpire(), pos))
		{
			sys_log(1, "Recall: %s %d %d -> %d %d", ch->GetName(), ch->GetX(), ch->GetY(), pos.x, pos.y);
			ch->WarpSet(pos.x, pos.y);
		}
		else
		{
			sys_err("CHARACTER::UseItem : cannot find spawn position (name %s, %d x %d)", ch->GetName(), ch->GetX(), ch->GetY());
			ch->GoHome();
		}
	}
	else
	{
		//failure
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;965]");
	}
	return 0;
}

int CHARACTER::ComputeSkillAtPosition(uint32_t dwVnum, const PIXEL_POSITION& posTarget, uint8_t bSkillLevel)
{
	if (GetMountVnum())
		return BATTLE_NONE;

	if (IsPolymorphed())
		return BATTLE_NONE;

	if (g_bSkillDisable)
		return BATTLE_NONE;

	CSkillProto* pkSk = CSkillManager::Instance().Get(dwVnum);

	if (!pkSk)
		return BATTLE_NONE;

	if (test_server)
	{
		sys_log(0, "ComputeSkillAtPosition %s vnum %d x %d y %d level %d", GetName(), dwVnum, posTarget.x, posTarget.y, bSkillLevel);
	}

	// The skill used to me uses my position.
	//if (IS_SET(pkSk->dwFlag, SKILL_FLAG_SELFONLY))
	// posTarget = GetXYZ();

	// Skills other than splash are weird around them
	if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
		return BATTLE_NONE;

	if (0 == bSkillLevel)
	{
		if ((bSkillLevel = GetSkillLevel(pkSk->dwVnum)) == 0)
		{
			return BATTLE_NONE;
		}
	}

	const float k = 1.0f * GetSkillPower(pkSk->dwVnum, bSkillLevel) * pkSk->bMaxLevel / 100;

	pkSk->SetPointVar("k", k);
	pkSk->kSplashAroundDamageAdjustPoly.SetVar("k", k);

	if (IS_SET(pkSk->dwFlag, SKILL_FLAG_USE_MELEE_DAMAGE))
	{
		pkSk->SetPointVar("atk", CalcMeleeDamage(this, this, true, false));
	}
	else if (IS_SET(pkSk->dwFlag, SKILL_FLAG_USE_MAGIC_DAMAGE))
	{
		pkSk->SetPointVar("atk", CalcMagicDamage(this, this));
	}
	else if (IS_SET(pkSk->dwFlag, SKILL_FLAG_USE_ARROW_DAMAGE))
	{
		LPITEM pkBow, pkArrow;
		if (1 == GetArrowAndBow(&pkBow, &pkArrow, 1))
		{
			pkSk->SetPointVar("atk", CalcArrowDamage(this, this, pkBow, pkArrow, true));
		}
		else
		{
			pkSk->SetPointVar("atk", 0);
		}
	}

	if (pkSk->wPointOn == POINT_MOV_SPEED)
	{
		pkSk->SetPointVar("maxv", this->GetLimitPoint(POINT_MOV_SPEED));
	}

	pkSk->SetPointVar("lv", GetLevel());
	pkSk->SetPointVar("iq", GetPoint(POINT_IQ));
	pkSk->SetPointVar("str", GetPoint(POINT_ST));
	pkSk->SetPointVar("dex", GetPoint(POINT_DX));
	pkSk->SetPointVar("con", GetPoint(POINT_HT));
	pkSk->SetPointVar("maxhp", this->GetMaxHP());
	pkSk->SetPointVar("maxsp", this->GetMaxSP());
	pkSk->SetPointVar("chain", 0);
	pkSk->SetPointVar("ar", CalcAttackRating(this, this));
	pkSk->SetPointVar("def", GetPoint(POINT_DEF_GRADE));
	pkSk->SetPointVar("odef", GetPoint(POINT_DEF_GRADE) - GetPoint(POINT_DEF_GRADE_BONUS));
	pkSk->SetPointVar("horse_level", GetHorseLevel());

	if (pkSk->bSkillAttrType != SKILL_ATTR_TYPE_NORMAL)
		OnMove(true);

	LPITEM pkWeapon = GetWear(WEAR_WEAPON);

	SetPolyVarForAttack(this, pkSk, pkWeapon);

	pkSk->SetDurationVar("k", k/*bSkillLevel*/);

	int iAmount = static_cast<int>(pkSk->kPointPoly.Eval());
	const int iAmount2 = static_cast<int>(pkSk->kPointPoly2.Eval());

	// ADD_GRANDMASTER_SKILL
	const int iAmount3 = static_cast<int>(pkSk->kPointPoly3.Eval());
#ifdef ENABLE_SKILL_TABLE_RENEWAL
	const int iAmount4 = static_cast<int>(pkSk->kPointPoly4.Eval());
#endif

	if (GetUsedSkillMasterType(pkSk->dwVnum) >= SKILL_GRAND_MASTER)
	{
		/*
		if (iAmount >= 0)
			iAmount += (int) m_pkSk->kMasterBonusPoly.Eval();
		else
			iAmount -= (int) m_pkSk->kMasterBonusPoly.Eval();
		*/
		iAmount = static_cast<int>(pkSk->kMasterBonusPoly.Eval());
	}

	if (test_server && iAmount == 0 && pkSk->wPointOn != POINT_NONE)
	{
		ChatPacket(CHAT_TYPE_INFO, "[#Unk]There is no effect. Check your skill formula");
	}

	if (IS_SET(pkSk->dwFlag, SKILL_FLAG_REMOVE_BAD_AFFECT))
	{
		if (number(1, 100) <= iAmount2)
		{
			RemoveBadAffect();
		}
	}
	// END_OF_ADD_GRANDMASTER_SKILL

	if (IS_SET(pkSk->dwFlag, SKILL_FLAG_ATTACK | SKILL_FLAG_USE_MELEE_DAMAGE | SKILL_FLAG_USE_MAGIC_DAMAGE))
	{
		//
		// In case of attack skill
		//
		bool bAdded = false;

		if (pkSk->wPointOn == POINT_HP && iAmount < 0)
		{
			constexpr int iAG = 0;

			FuncSplashDamage f(posTarget.x, posTarget.y, pkSk, this, iAmount, iAG, pkSk->lMaxHit, pkWeapon, m_bDisableCooltime, IsPC() ? &m_SkillUseInfo[dwVnum] : nullptr, GetSkillPower(dwVnum, bSkillLevel));

			if (IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
			{
				if (GetSectree())
					GetSectree()->ForEachAround(f);
			}
			else
			{
				//if (dwVnum == SKILL_CHAIN) sys_log(0, "CHAIN skill call FuncSplashDamage %s", GetName());
				f(this);
			}
		}
		else
		{
			//if (dwVnum == SKILL_CHAIN) sys_log(0, "CHAIN skill no damage %d %s", iAmount, GetName());
			int iDur = static_cast<int>(pkSk->kDurationPoly.Eval());

			if (IsPC())
			{
				if (!(dwVnum >= GUILD_SKILL_START && dwVnum <= GUILD_SKILL_END)) // Guild skills do not deal with cooldowns.
				{
					if (!m_bDisableCooltime && !m_SkillUseInfo[dwVnum].HitOnce(dwVnum) && dwVnum != SKILL_MUYEONG)
					{
						//if (dwVnum == SKILL_CHAIN) sys_log(0, "CHAIN skill cannot hit %s", GetName());
						return BATTLE_NONE;
					}
				}
			}

			if (iDur > 0)
			{
				iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);

				if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
					AddAffect(pkSk->dwVnum, pkSk->wPointOn, iAmount, pkSk->dwAffectFlag, iDur, 0, true);
				else
				{
					if (GetSectree())
					{
						FuncSplashAffect f(this, posTarget.x, posTarget.y, pkSk->iSplashRange, pkSk->dwVnum, pkSk->wPointOn, iAmount, pkSk->dwAffectFlag, iDur, 0, true, pkSk->lMaxHit);
						GetSectree()->ForEachAround(f);
					}
				}
				bAdded = true;
			}
		}

		if (pkSk->wPointOn2 != POINT_NONE)
		{
			int iDur = static_cast<int>(pkSk->kDurationPoly2.Eval());

			sys_log(1, "try second %u %d %d", pkSk->dwVnum, pkSk->wPointOn2, iDur);

			if (iDur > 0)
			{
				iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);

				if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
					AddAffect(pkSk->dwVnum, pkSk->wPointOn2, iAmount2, pkSk->dwAffectFlag2, iDur, 0, !bAdded);
				else
				{
					if (GetSectree())
					{
						FuncSplashAffect f(this, posTarget.x, posTarget.y, pkSk->iSplashRange, pkSk->dwVnum, pkSk->wPointOn2, iAmount2, pkSk->dwAffectFlag2, iDur, 0, !bAdded, pkSk->lMaxHit);
						GetSectree()->ForEachAround(f);
					}
				}
				bAdded = true;
			}
			else
			{
				PointChange(pkSk->wPointOn2, iAmount2);
			}
		}

		// ADD_GRANDMASTER_SKILL
		if (GetUsedSkillMasterType(pkSk->dwVnum) >= SKILL_GRAND_MASTER && pkSk->wPointOn3 != POINT_NONE)
		{
			int iDur = static_cast<int>(pkSk->kDurationPoly3.Eval());

			if (iDur > 0)
			{
				iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);

				if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
					AddAffect(pkSk->dwVnum, pkSk->wPointOn3, iAmount3, 0 /*pkSk->dwAffectFlag3*/, iDur, 0, !bAdded);
				else
				{
					if (GetSectree())
					{
						FuncSplashAffect f(this, posTarget.x, posTarget.y, pkSk->iSplashRange, pkSk->dwVnum, pkSk->wPointOn3, iAmount3, 0 /*pkSk->dwAffectFlag3*/, iDur, 0, !bAdded, pkSk->lMaxHit);
						GetSectree()->ForEachAround(f);
					}
				}
			}
			else
			{
				PointChange(pkSk->wPointOn3, iAmount3);
			}
		}
		// END_OF_ADD_GRANDMASTER_SKILL

#ifdef ENABLE_SKILL_TABLE_RENEWAL
		if (pkSk->wPointOn4 != POINT_NONE)
		{
			int iDur = static_cast<int>(pkSk->kDurationPoly4.Eval());
			if (iDur > 0)
			{
				iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);

				if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
					AddAffect(pkSk->dwVnum, pkSk->wPointOn4, iAmount4, 0, iDur, 0, !bAdded);
				else
				{
					if (GetSectree())
					{
						FuncSplashAffect f(this, posTarget.x, posTarget.y, pkSk->iSplashRange, pkSk->dwVnum, pkSk->wPointOn4, iAmount4, 0, iDur, 0, !bAdded, pkSk->lMaxHit);
						GetSectree()->ForEachAround(f);
					}
				}
			}
			else
			{
				PointChange(pkSk->wPointOn4, iAmount4);
			}
		}
#endif

		return BATTLE_DAMAGE;
	}
	else
	{
		bool bAdded = false;
		int iDur = static_cast<int>(pkSk->kDurationPoly.Eval());

		if (iDur > 0)
		{
			iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);
			// If there is no AffectFlag or it is not toggled...
			pkSk->kDurationSPCostPoly.SetVar("k", k/*bSkillLevel*/);

			AddAffect(pkSk->dwVnum,
				pkSk->wPointOn,
				iAmount,
				pkSk->dwAffectFlag,
				iDur,
				(long)pkSk->kDurationSPCostPoly.Eval(),
				!bAdded);

			bAdded = true;
		}
		else
		{
			PointChange(pkSk->wPointOn, iAmount);
		}

		if (pkSk->wPointOn2 != POINT_NONE)
		{
			int iDur2 = static_cast<int>(pkSk->kDurationPoly2.Eval());

			if (iDur2 > 0)
			{
				iDur2 += GetPoint(POINT_PARTY_BUFFER_BONUS);
				AddAffect(pkSk->dwVnum, pkSk->wPointOn2, iAmount2, pkSk->dwAffectFlag2, iDur2, 0, !bAdded);
				bAdded = true;
			}
			else
			{
				PointChange(pkSk->wPointOn2, iAmount2);
			}
		}

		// ADD_GRANDMASTER_SKILL
		if (GetUsedSkillMasterType(pkSk->dwVnum) >= SKILL_GRAND_MASTER && pkSk->wPointOn3 != POINT_NONE)
		{
			int iDur3 = static_cast<int>(pkSk->kDurationPoly3.Eval());

			if (iDur3 > 0)
			{
				iDur3 += GetPoint(POINT_PARTY_BUFFER_BONUS);
				AddAffect(pkSk->dwVnum, pkSk->wPointOn3, iAmount3, 0 /*pkSk->dwAffectFlag3*/, iDur3, 0, !bAdded);
			}
			else
			{
				PointChange(pkSk->wPointOn3, iAmount3);
			}
		}
		// END_OF_ADD_GRANDMASTER_SKILL

#ifdef ENABLE_SKILL_TABLE_RENEWAL
		if (pkSk->wPointOn4 != POINT_NONE)
		{
			int iDur4 = static_cast<int>(pkSk->kDurationPoly4.Eval());
			if (iDur4 > 0)
			{
				iDur4 += GetPoint(POINT_PARTY_BUFFER_BONUS);
				AddAffect(pkSk->dwVnum, pkSk->wPointOn4, iAmount4, 0, iDur4, 0, !bAdded);
			}
			else
			{
				PointChange(pkSk->wPointOn4, iAmount4);
			}
		}
#endif

		return BATTLE_NONE;
	}
}

#ifdef ENABLE_WOLFMAN_CHARACTER
struct FComputeSkillParty
{
	FComputeSkillParty(uint32_t dwVnum, LPCHARACTER pkAttacker, uint8_t bSkillLevel = 0)
		: m_dwVnum(dwVnum), m_pkAttacker(pkAttacker), m_bSkillLevel(bSkillLevel)
	{
	}

	void operator() (LPCHARACTER ch)
	{
#ifdef ENABLE_SKILL_COLOR_SYSTEM
		if (m_dwVnum == SKILL_CHEONGRANG || m_dwVnum == SKILL_HOSIN || m_dwVnum == SKILL_REFLECT || m_dwVnum == SKILL_GICHEON || m_dwVnum == SKILL_KWAESOK || m_dwVnum == SKILL_JEUNGRYEOK
#	ifdef ENABLE_NINETH_SKILL
			|| m_dwVnum == SKILL_CHEONUN
#	endif
			)
		{
			uint8_t skill = 0;
			uint8_t id = 0;
			switch (m_dwVnum)
			{
				case SKILL_HOSIN:
					skill = ESkillColorLength::BUFF_BEGIN + 0;
					id = 3;
					break;
				case SKILL_REFLECT:
					skill = ESkillColorLength::BUFF_BEGIN + 1;
					id = 4;
					break;
				case SKILL_GICHEON:
					skill = ESkillColorLength::BUFF_BEGIN + 2;
					id = 5;
					break;
				case SKILL_KWAESOK:
					skill = ESkillColorLength::BUFF_BEGIN + 3;
					id = 4;
					break;
				case SKILL_JEUNGRYEOK:
					skill = ESkillColorLength::BUFF_BEGIN + 4;
					id = 5;
					break;
#	ifdef ENABLE_NINETH_SKILL
				case SKILL_CHEONUN:
					skill = ESkillColorLength::BUFF_BEGIN + 5;
					id = 8;
					break;
#	endif
#	ifdef ENABLE_WOLFMAN_CHARACTER
				case SKILL_CHEONGRANG:
					skill = ESkillColorLength::BUFF_BEGIN + 6;
					id = 5;
#	endif
				default:
					break;
			}

			uint32_t data[ESkillColorLength::MAX_SKILL_COUNT + ESkillColorLength::MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
			memcpy(data, ch->GetSkillColor(), sizeof(data));

			uint32_t dataAttacker[ESkillColorLength::MAX_SKILL_COUNT + ESkillColorLength::MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
			memcpy(dataAttacker, m_pkAttacker->GetSkillColor(), sizeof(dataAttacker));

			data[skill][0] = dataAttacker[id][0];
			data[skill][1] = dataAttacker[id][1];
			data[skill][2] = dataAttacker[id][2];
			data[skill][3] = dataAttacker[id][3];
			data[skill][4] = dataAttacker[id][4];

			ch->SetSkillColor(data[0]);

			TSkillColor db_pack;
			memcpy(db_pack.dwSkillColor, data, sizeof(data));
			db_pack.player_id = ch->GetPlayerID();
			db_clientdesc->DBPacketHeader(HEADER_GD_SKILL_COLOR_SAVE, 0, sizeof(TSkillColor));
			db_clientdesc->Packet(&db_pack, sizeof(TSkillColor));
		}
#endif

		if (ch)
		{
			m_pkAttacker->ComputeSkill(m_dwVnum, ch, m_bSkillLevel);
#ifdef ENABLE_SHAMAN_PARTY_BUFF_SYSTEM
			if ((ch != m_pkAttacker) && (m_dwVnum == SKILL_JEONGEOP)) // Heal skill
				ch->EffectPacket(SE_HEAL);
#endif
		}
	}

	uint32_t m_dwVnum;
	LPCHARACTER m_pkAttacker;
	uint8_t m_bSkillLevel;
};

int CHARACTER::ComputeSkillParty(uint32_t dwVnum, LPCHARACTER pkVictim, uint8_t bSkillLevel)
{
	FComputeSkillParty f(dwVnum, pkVictim, bSkillLevel);
	if (GetParty() && GetParty()->GetNearMemberCount())
		GetParty()->ForEachNearMember(f);
	else
		f(this);

	return BATTLE_NONE;
}
#endif

#ifdef ENABLE_PVP_BALANCE
int CHARACTER::ComputeGyeongGongSkill(uint32_t dwVnum, LPCHARACTER pkVictim, uint8_t bSkillLevel)
{
	if (IsPolymorphed())
		return BATTLE_NONE;

	if (g_bSkillDisable)
		return BATTLE_NONE;

	CSkillProto* pkSk = CSkillManager::Instance().Get(dwVnum);

	if (!pkSk)
		return BATTLE_NONE;

	if (IS_SET(pkSk->dwFlag, SKILL_FLAG_SELFONLY))
		pkVictim = this;

	if (!pkVictim)
	{
		if (test_server)
			sys_log(0, "ComputeGyeongGongSkill: %s Victim == null, skill %d", GetName(), dwVnum);

		return BATTLE_NONE;
	}

	if (0 == bSkillLevel)
	{
		if ((bSkillLevel = GetSkillLevel(pkSk->dwVnum)) == 0)
		{
			if (test_server)
				sys_log(0, "ComputeGyeongGongSkill: name:%s vnum:%d skillLevelBySkill : %d ", GetName(), pkSk->dwVnum, bSkillLevel);
			return BATTLE_NONE;
		}
	}

	//const float k = 1.0 * GetSkillPower(pkSk->dwVnum, bSkillLevel) * pkSk->bMaxLevel / 100;

	if (pkSk->bSkillAttrType != SKILL_ATTR_TYPE_NORMAL)
		OnMove(true);

	LPITEM pkWeapon = GetWear(WEAR_WEAPON);

	SetPolyVarForAttack(this, pkSk, pkWeapon);
	const int iAmount = static_cast<int>(pkSk->kPointPoly2.Eval());

	// END_OF_ADD_GRANDMASTER_SKILL
	if (iAmount > 0 && dwVnum == SKILL_GYEONGGONG)
	{
		FuncSplashDamage f(pkVictim->GetX(), pkVictim->GetY(), pkSk, this, -iAmount, 0, pkSk->lMaxHit, pkWeapon, m_bDisableCooltime, IsPC() ? &m_SkillUseInfo[dwVnum] : nullptr, GetSkillPower(dwVnum, bSkillLevel));

		if (pkVictim->GetSectree())
			pkVictim->GetSectree()->ForEachAround(f);
		else
		{
			f(pkVictim);
		}
		return BATTLE_DAMAGE;
	}
	return BATTLE_NONE;
}
#endif

//#ifdef ENABLE_SHAMAN_PARTY_BUFF_SYSTEM
bool CHARACTER::IsBuffSkill(uint32_t dwSkillVnum) const noexcept
{
	switch (dwSkillVnum) // Shaman buff skills
	{
		case SKILL_HOSIN: // Blessing
		case SKILL_REFLECT: // Reflect
		case SKILL_GICHEON: // Dragon's Aid
		case SKILL_JEONGEOP: // Cure
		case SKILL_KWAESOK: // Swiftness
		case SKILL_JEUNGRYEOK: // Attack Up
#ifdef ENABLE_NINETH_SKILL
		case SKILL_CHEONUN: // Aether Ward
#endif
			return true;

		default:
			break;
	}

	return false;
}
//#endif

// If bSkillLevel argument is not 0, do not use m_abSkillLevels and force
// Calculate with bSkillLevel.
int CHARACTER::ComputeSkill(uint32_t dwVnum, LPCHARACTER pkVictim, uint8_t bSkillLevel)
{
	const bool bCanUseHorseSkill = CanUseHorseSkill();

	// If you are on horseback but the skill is not available, return
	if (!bCanUseHorseSkill && IsRiding())
		return BATTLE_NONE;

	if (IsPolymorphed())
		return BATTLE_NONE;

	if (g_bSkillDisable)
		return BATTLE_NONE;

	CSkillProto* pkSk = CSkillManager::Instance().Get(dwVnum);

	if (!pkSk)
		return BATTLE_NONE;

	if (bCanUseHorseSkill && pkSk->dwType != SKILL_TYPE_HORSE
		&& pkSk->dwVnum != SKILL_MUYEONG) //@fixme511 - Spirit flame working riding too (GF 19.6.7)
	{
		return BATTLE_NONE;
	}

	if (!bCanUseHorseSkill && pkSk->dwType == SKILL_TYPE_HORSE)
		return BATTLE_NONE;

	if (dwVnum == SKILL_EUNHYUNG && GetSectree()) {	//@fixme512
		RemoveInvisibleVictim f(this);
		GetSectree()->ForEachAround(f);
	}

	// If it's not written to the other person, it should be written to me.
	if (IS_SET(pkSk->dwFlag, SKILL_FLAG_SELFONLY))
		pkVictim = this;
//#ifdef ENABLE_WOLFMAN_CHARACTER
//	else if (IS_SET(pkSk->dwFlag, SKILL_FLAG_PARTY))
//		pkVictim = this;
//#endif

	if (!pkVictim)
	{
		if (test_server)
			sys_log(0, "ComputeSkill: %s Victim == null, skill %d", GetName(), dwVnum);

		return BATTLE_NONE;
	}

#ifdef ENABLE_BLOCK_BUFF_IN_MAP
	if ((pkVictim->GetMapIndex() == MAP_PVP_ARENA) && (IsBuffSkill(pkSk->dwVnum)))
		pkVictim = this;
#endif

	const auto dwDist = static_cast<uint32_t>(DISTANCE_SQRT(GetX() - pkVictim->GetX(), GetY() - pkVictim->GetY()));
#ifdef ENABLE_NEW_DISTANCE_CALC
	if ((pkSk->dwTargetRange) && (dwDist >= ((pkSk->dwTargetRange + 50 / 100) * pkVictim->GetMonsterHitRange()))
#else
	if ((pkSk->dwTargetRange) && (dwDist >= pkSk->dwTargetRange + 50)
#endif
		&& (pkSk->dwType != SKILL_TYPE_HORSE) // Ignoring limit range for horse skills
		)
	{
		if (test_server)
		{
			sys_log(0, "ComputeSkill: Victim too far, skill %d : %s to %s (distance %u limit %u)",
				dwVnum,
				GetName(),
				pkVictim->GetName(),
				dwDist,
				pkSk->dwTargetRange + 50);
		}

		return BATTLE_NONE;
	}

	if (0 == bSkillLevel)
	{
		if ((bSkillLevel = GetSkillLevel(pkSk->dwVnum)) == 0)
		{
			if (test_server)
				sys_log(0, "ComputeSkill : name:%s vnum:%d skillLevelBySkill : %d ", GetName(), pkSk->dwVnum, bSkillLevel);
			return BATTLE_NONE;
		}
	}

	if (pkVictim->IsAffectFlag(AFF_PABEOP) && pkVictim->IsGoodAffect(dwVnum))
	{
		return BATTLE_NONE;
	}

	const float k = 1.0f * GetSkillPower(pkSk->dwVnum, bSkillLevel) * pkSk->bMaxLevel / 100;

	pkSk->SetPointVar("k", k);
	pkSk->kSplashAroundDamageAdjustPoly.SetVar("k", k);

	if (pkSk->dwType == SKILL_TYPE_HORSE)
	{
		LPITEM pkBow, pkArrow;
		if (1 == GetArrowAndBow(&pkBow, &pkArrow, 1))
		{
			pkSk->SetPointVar("atk", CalcArrowDamage(this, pkVictim, pkBow, pkArrow, true));
		}
		else
		{
			pkSk->SetPointVar("atk", CalcMeleeDamage(this, pkVictim, true, false));
		}
	}
	else if (IS_SET(pkSk->dwFlag, SKILL_FLAG_USE_MELEE_DAMAGE))
	{
		pkSk->SetPointVar("atk", CalcMeleeDamage(this, pkVictim, true, false));
	}
	else if (IS_SET(pkSk->dwFlag, SKILL_FLAG_USE_MAGIC_DAMAGE))
	{
		pkSk->SetPointVar("atk", CalcMagicDamage(this, pkVictim));
	}
	else if (IS_SET(pkSk->dwFlag, SKILL_FLAG_USE_ARROW_DAMAGE))
	{
		LPITEM pkBow, pkArrow;
		if (1 == GetArrowAndBow(&pkBow, &pkArrow, 1))
		{
			pkSk->SetPointVar("atk", CalcArrowDamage(this, pkVictim, pkBow, pkArrow, true));
		}
		else
		{
			pkSk->SetPointVar("atk", 0);
		}
	}

	if (pkSk->wPointOn == POINT_MOV_SPEED)
	{
		pkSk->SetPointVar("maxv", pkVictim->GetLimitPoint(POINT_MOV_SPEED));
	}

	pkSk->SetPointVar("lv", GetLevel());
	pkSk->SetPointVar("iq", GetPoint(POINT_IQ));
	pkSk->SetPointVar("str", GetPoint(POINT_ST));
	pkSk->SetPointVar("dex", GetPoint(POINT_DX));
	pkSk->SetPointVar("con", GetPoint(POINT_HT));
	pkSk->SetPointVar("maxhp", pkVictim->GetMaxHP());
	pkSk->SetPointVar("maxsp", pkVictim->GetMaxSP());
	pkSk->SetPointVar("chain", 0);
	pkSk->SetPointVar("ar", CalcAttackRating(this, pkVictim));
	pkSk->SetPointVar("def", GetPoint(POINT_DEF_GRADE));
	pkSk->SetPointVar("odef", GetPoint(POINT_DEF_GRADE) - GetPoint(POINT_DEF_GRADE_BONUS));
	pkSk->SetPointVar("horse_level", GetHorseLevel());

	if (pkSk->bSkillAttrType != SKILL_ATTR_TYPE_NORMAL)
		OnMove(true);

	LPITEM pkWeapon = GetWear(WEAR_WEAPON);

	SetPolyVarForAttack(this, pkSk, pkWeapon);

	pkSk->kDurationPoly.SetVar("k", k/*bSkillLevel*/);
	pkSk->kDurationPoly2.SetVar("k", k/*bSkillLevel*/);

	int iAmount = static_cast<int>(pkSk->kPointPoly.Eval());
	const int iAmount2 = static_cast<int>(pkSk->kPointPoly2.Eval());
	const int iAmount3 = static_cast<int>(pkSk->kPointPoly3.Eval());
#ifdef ENABLE_SKILL_TABLE_RENEWAL
	const int iAmount4 = static_cast<int>(pkSk->kPointPoly4.Eval());
#endif

	if (test_server && IsPC())
		sys_log(0, "iAmount: %d %d %d , atk:%f skLevel:%f k:%f GetSkillPower(%d) MaxLevel:%d Per:%f",
			iAmount, iAmount2, iAmount3,
			pkSk->kPointPoly.GetVar("atk"),
			pkSk->kPointPoly.GetVar("k"),
			k,
			GetSkillPower(pkSk->dwVnum, bSkillLevel),
			pkSk->bMaxLevel,
			pkSk->bMaxLevel / 100
		);

	// ADD_GRANDMASTER_SKILL
	if (GetUsedSkillMasterType(pkSk->dwVnum) >= SKILL_GRAND_MASTER)
	{
		iAmount = static_cast<int>(pkSk->kMasterBonusPoly.Eval());
	}

	if (test_server && iAmount == 0 && pkSk->wPointOn != POINT_NONE)
	{
		ChatPacket(CHAT_TYPE_INFO, "[#Unk]There is no effect. Check your skill formula");
	}
	// END_OF_ADD_GRANDMASTER_SKILL

	//sys_log(0, "XXX SKILL Calc %d Amount %d", dwVnum, iAmount);

	// REMOVE_BAD_AFFECT_BUG_FIX
	if (IS_SET(pkSk->dwFlag, SKILL_FLAG_REMOVE_BAD_AFFECT))
	{
		if (number(1, 100) <= iAmount2)
		{
			pkVictim->RemoveBadAffect();
		}
	}
	// END_OF_REMOVE_BAD_AFFECT_BUG_FIX

	if (IS_SET(pkSk->dwFlag, SKILL_FLAG_ATTACK | SKILL_FLAG_USE_MELEE_DAMAGE | SKILL_FLAG_USE_MAGIC_DAMAGE) &&
		!(pkSk->dwVnum == SKILL_MUYEONG && pkVictim == this) && !(pkSk->IsChargeSkill() && pkVictim == this))
	{
		bool bAdded = false;

		if (pkSk->wPointOn == POINT_HP && iAmount < 0)
		{
			const int iAG = 0;

			FuncSplashDamage f(pkVictim->GetX(), pkVictim->GetY(), pkSk, this, iAmount, iAG, pkSk->lMaxHit, pkWeapon, m_bDisableCooltime, IsPC() ? &m_SkillUseInfo[dwVnum] : nullptr, GetSkillPower(dwVnum, bSkillLevel));
			if (IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
			{
				if (pkVictim->GetSectree())
					pkVictim->GetSectree()->ForEachAround(f);
			}
			else
			{
				f(pkVictim);
			}
		}
		else
		{
			pkSk->kDurationPoly.SetVar("k", k/*bSkillLevel*/);
			int iDur = static_cast<int>(pkSk->kDurationPoly.Eval());

			if (IsPC())
			{
				if (!(dwVnum >= GUILD_SKILL_START && dwVnum <= GUILD_SKILL_END)) // Guild skills do not deal with cooldowns.
				{
					if (!m_bDisableCooltime && !m_SkillUseInfo[dwVnum].HitOnce(dwVnum) && dwVnum != SKILL_MUYEONG)
					{
						return BATTLE_NONE;
					}
				}
			}

			if (iDur > 0)
			{
				iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);

				if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
					pkVictim->AddAffect(pkSk->dwVnum, pkSk->wPointOn, iAmount, pkSk->dwAffectFlag, iDur, 0, true);
				else
				{
					if (pkVictim->GetSectree())
					{
						FuncSplashAffect f(this, pkVictim->GetX(), pkVictim->GetY(), pkSk->iSplashRange, pkSk->dwVnum, pkSk->wPointOn, iAmount, pkSk->dwAffectFlag, iDur, 0, true, pkSk->lMaxHit);
						pkVictim->GetSectree()->ForEachAround(f);
					}
				}
				bAdded = true;
			}
		}

		if (pkSk->wPointOn2 != POINT_NONE && !pkSk->IsChargeSkill())
		{
			pkSk->kDurationPoly2.SetVar("k", k/*bSkillLevel*/);
			int iDur = static_cast<int>(pkSk->kDurationPoly2.Eval());

			if (iDur > 0)
			{
				iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);

#ifdef ENABLE_PRECISION
				if (IsNineSkill(pkSk->dwVnum) && pkSk->wPointOn2 == POINT_HIT_PCT)
					AddAffect(pkSk->dwVnum, pkSk->wPointOn2, iAmount2, AFF_NONE, iDur, 0, !bAdded);
#endif

				if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
					pkVictim->AddAffect(pkSk->dwVnum, pkSk->wPointOn2, iAmount2, pkSk->dwAffectFlag2, iDur, 0, !bAdded);
				else
				{
					if (pkVictim->GetSectree())
					{
						FuncSplashAffect f(this, pkVictim->GetX(), pkVictim->GetY(), pkSk->iSplashRange, pkSk->dwVnum, pkSk->wPointOn2, iAmount2, pkSk->dwAffectFlag2, iDur, 0, !bAdded, pkSk->lMaxHit);
						pkVictim->GetSectree()->ForEachAround(f);
					}
				}

				bAdded = true;
			}
			else
			{
				pkVictim->PointChange(pkSk->wPointOn2, iAmount2);
			}
		}

		// ADD_GRANDMASTER_SKILL
		if (pkSk->wPointOn3 != POINT_NONE && !pkSk->IsChargeSkill() && GetUsedSkillMasterType(pkSk->dwVnum) >= SKILL_GRAND_MASTER)
		{
			pkSk->kDurationPoly3.SetVar("k", k/*bSkillLevel*/);
			int iDur = static_cast<int>(pkSk->kDurationPoly3.Eval());


			if (iDur > 0)
			{
				iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);

				if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
					pkVictim->AddAffect(pkSk->dwVnum, pkSk->wPointOn3, iAmount3, /*pkSk->dwAffectFlag3*/ 0, iDur, 0, !bAdded);
				else
				{
					if (pkVictim->GetSectree())
					{
						FuncSplashAffect f(this, pkVictim->GetX(), pkVictim->GetY(), pkSk->iSplashRange, pkSk->dwVnum, pkSk->wPointOn3, iAmount3, /*pkSk->dwAffectFlag3*/ 0, iDur, 0, !bAdded, pkSk->lMaxHit);
						pkVictim->GetSectree()->ForEachAround(f);
					}
				}

				bAdded = true;
			}
			else
			{
				pkVictim->PointChange(pkSk->wPointOn3, iAmount3);
			}
		}
		// END_OF_ADD_GRANDMASTER_SKILL

#ifdef ENABLE_SKILL_TABLE_RENEWAL
		if (pkSk->wPointOn4 != POINT_NONE && !pkSk->IsChargeSkill())
		{
			pkSk->kDurationPoly4.SetVar("k", k/*bSkillLevel*/);
			int iDur = static_cast<int>(pkSk->kDurationPoly4.Eval());


			if (iDur > 0)
			{
				iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);

				if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
					pkVictim->AddAffect(pkSk->dwVnum, pkSk->wPointOn4, iAmount4, 0, iDur, 0, !bAdded);
				else
				{
					if (pkVictim->GetSectree())
					{
						FuncSplashAffect f(this, pkVictim->GetX(), pkVictim->GetY(), pkSk->iSplashRange, pkSk->dwVnum, pkSk->wPointOn4, iAmount4, 0, iDur, 0, !bAdded, pkSk->lMaxHit);
						pkVictim->GetSectree()->ForEachAround(f);
					}
				}

				bAdded = true;
			}
			else
			{
				pkVictim->PointChange(pkSk->wPointOn4, iAmount4);
			}
		}
#endif

		return BATTLE_DAMAGE;
	}
	else
	{
		if (dwVnum == SKILL_MUYEONG)
		{
			pkSk->kDurationPoly.SetVar("k", k/*bSkillLevel*/);
			pkSk->kDurationSPCostPoly.SetVar("k", k/*bSkillLevel*/);

			auto iDur = static_cast<long>(pkSk->kDurationPoly.Eval());
			iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);

			if (pkVictim == this)
			{
				AddAffect(dwVnum,
					POINT_NONE, 0,
					AFF_MUYEONG,
					iDur,
					(long)pkSk->kDurationSPCostPoly.Eval(),
					true);
			}

			return BATTLE_NONE;
		}

		bool bAdded = false;
		pkSk->kDurationPoly.SetVar("k", k/*bSkillLevel*/);
		int iDur = static_cast<long>(pkSk->kDurationPoly.Eval());

		if (iDur > 0)
		{
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
			if (pkSk->dwVnum == SKILL_EUNHYUNG)
			{
				sys_err("Eunhyung dur before: %d", iDur);
				if (GetPoint(POINT_SKILL_DURATION_INCREASE_EUNHYUNG))
					iDur = iDur + (iDur * GetPoint(POINT_SKILL_DURATION_INCREASE_EUNHYUNG)) / 100;

				sys_err("Eunhyung dur after: %d", iDur);
			}
#endif

			iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);
			// If there is no AffectFlag or it is not toggled...
			pkSk->kDurationSPCostPoly.SetVar("k", k/*bSkillLevel*/);

			if (pkSk->wPointOn2 != POINT_NONE)
			{
				pkVictim->RemoveAffect(pkSk->dwVnum);

				int iDur2 = static_cast<int>(pkSk->kDurationPoly2.Eval());

				if (iDur2 > 0)
				{
					if (test_server)
					{
						sys_log(0, "SKILL_AFFECT: %s %s Dur:%d To:%d Amount:%d",
							GetName(),
							pkSk->szName,
							iDur2,
							pkSk->wPointOn2,
							iAmount2);
					}

					iDur2 += GetPoint(POINT_PARTY_BUFFER_BONUS);
					pkVictim->AddAffect(pkSk->dwVnum, pkSk->wPointOn2, iAmount2, pkSk->dwAffectFlag2, iDur2, 0, false);
				}
				else
				{
					pkVictim->PointChange(pkSk->wPointOn2, iAmount2);
				}

				uint32_t affact_flag = pkSk->dwAffectFlag;

				// ADD_GRANDMASTER_SKILL
				if ((pkSk->dwVnum == SKILL_CHUNKEON && GetUsedSkillMasterType(pkSk->dwVnum) < SKILL_GRAND_MASTER))
					affact_flag = AFF_CHEONGEUN;	//@fixme461 - AFF_CHEONGEUN_WITH_FALL
				// END_OF_ADD_GRANDMASTER_SKILL

				pkVictim->AddAffect(pkSk->dwVnum,
					pkSk->wPointOn,
					iAmount,
					affact_flag,
					iDur,
					(long)pkSk->kDurationSPCostPoly.Eval(),
					false);
			}
			else
			{
				if (test_server)
				{
					sys_log(0, "SKILL_AFFECT: %s %s Dur:%d To:%d Amount:%d",
						GetName(),
						pkSk->szName,
						iDur,
						pkSk->wPointOn,
						iAmount);
				}

				pkVictim->AddAffect(pkSk->dwVnum,
					pkSk->wPointOn,
					iAmount,
					pkSk->dwAffectFlag,
					iDur,
					(long)pkSk->kDurationSPCostPoly.Eval(),
					// ADD_GRANDMASTER_SKILL
					!bAdded
#ifdef ENABLE_NINETH_SKILL
					, false, iAmount2
#endif
				);
				// END_OF_ADD_GRANDMASTER_SKILL
			}

			bAdded = true;
		}
		else
		{
			if (!pkSk->IsChargeSkill())
				pkVictim->PointChange(pkSk->wPointOn, iAmount);

			if (pkSk->wPointOn2 != POINT_NONE)
			{
				pkVictim->RemoveAffect(pkSk->dwVnum);

				int iDur2 = static_cast<int>(pkSk->kDurationPoly2.Eval());

				if (iDur2 > 0)
				{
					iDur2 += GetPoint(POINT_PARTY_BUFFER_BONUS);

					if (pkSk->IsChargeSkill())
						pkVictim->AddAffect(pkSk->dwVnum, pkSk->wPointOn2, iAmount2, AFF_TANHWAN_DASH, iDur2, 0, false);
					else
						pkVictim->AddAffect(pkSk->dwVnum, pkSk->wPointOn2, iAmount2, pkSk->dwAffectFlag2, iDur2, 0, false);
				}
				else
				{
					pkVictim->PointChange(pkSk->wPointOn2, iAmount2);
				}

			}
		}

		// ADD_GRANDMASTER_SKILL
		if (pkSk->wPointOn3 != POINT_NONE && !pkSk->IsChargeSkill() && GetUsedSkillMasterType(pkSk->dwVnum) >= SKILL_GRAND_MASTER)
		{
			pkSk->kDurationPoly3.SetVar("k", k/*bSkillLevel*/);
			int iDur3 = static_cast<int>(pkSk->kDurationPoly3.Eval());

			sys_log(0, "try third %u %d %d %d 1894", pkSk->dwVnum, pkSk->wPointOn3, iDur3, iAmount3);

			if (iDur3 > 0)
			{
				iDur3 += GetPoint(POINT_PARTY_BUFFER_BONUS);

				if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
					pkVictim->AddAffect(pkSk->dwVnum, pkSk->wPointOn3, iAmount3, /*pkSk->dwAffectFlag3*/ 0, iDur3, 0, !bAdded);
				else
				{
					if (pkVictim->GetSectree())
					{
						FuncSplashAffect f(this, pkVictim->GetX(), pkVictim->GetY(), pkSk->iSplashRange, pkSk->dwVnum, pkSk->wPointOn3, iAmount3, /*pkSk->dwAffectFlag3*/ 0, iDur3, 0, !bAdded, pkSk->lMaxHit);
						pkVictim->GetSectree()->ForEachAround(f);
					}
				}

				bAdded = true;
			}
			else
			{
				pkVictim->PointChange(pkSk->wPointOn3, iAmount3);
			}
		}
		// END_OF_ADD_GRANDMASTER_SKILL

#ifdef ENABLE_SKILL_TABLE_RENEWAL
		if (pkSk->wPointOn4 != POINT_NONE && !pkSk->IsChargeSkill())
		{
			pkSk->kDurationPoly4.SetVar("k", k/*bSkillLevel*/);
			int iDur4 = static_cast<int>(pkSk->kDurationPoly4.Eval());

			sys_log(0, "try fourth %u %d %d %d 1894", pkSk->dwVnum, pkSk->wPointOn4, iDur4, iAmount4);

			if (iDur4 > 0)
			{
				iDur4 += GetPoint(POINT_PARTY_BUFFER_BONUS);

				if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
					pkVictim->AddAffect(pkSk->dwVnum, pkSk->wPointOn4, iAmount4, 0, iDur4, 0, !bAdded);
				else
				{
					if (pkVictim->GetSectree())
					{
						FuncSplashAffect f(this, pkVictim->GetX(), pkVictim->GetY(), pkSk->iSplashRange, pkSk->dwVnum, pkSk->wPointOn4, iAmount4, 0, iDur4, 0, !bAdded, pkSk->lMaxHit);
						pkVictim->GetSectree()->ForEachAround(f);
					}
				}

				bAdded = true;
			}
			else
			{
				pkVictim->PointChange(pkSk->wPointOn4, iAmount4);
			}
		}
#endif

#ifdef ENABLE_PVP_BALANCE
		if (pkSk->wPointOn2 == POINT_NONE && iAmount2 > 0 && dwVnum == SKILL_GYEONGGONG)
		{
			FuncSplashDamage f(pkVictim->GetX(), pkVictim->GetY(), pkSk, this, -iAmount2, 0, pkSk->lMaxHit, pkWeapon, m_bDisableCooltime, IsPC() ? &m_SkillUseInfo[dwVnum] : nullptr, GetSkillPower(dwVnum, bSkillLevel));
			if (pkVictim->GetSectree())
				pkVictim->GetSectree()->ForEachAround(f);

			else
			{
				f(pkVictim);
			}
		}
#endif

		return BATTLE_NONE;
	}
}

#ifdef ENABLE_QUEEN_NETHIS
struct FuncSnakeSplashDamage
{
	FuncSnakeSplashDamage(int x, int y, LPCHARACTER pkChr, uint8_t bUseSkillPower)
		:
		m_x(x), m_y(y), m_pkChr(pkChr), m_iCount(0), m_bUseSkillPower(bUseSkillPower)
	{
	}

	void operator () (LPENTITY ent)
	{
		if (!ent->IsType(ENTITY_CHARACTER))
		{
			return;
		}

		LPCHARACTER pkChrVictim = (LPCHARACTER)ent;
		if (!pkChrVictim)
			return;

		if (!m_pkChr)
			return;

		if (DISTANCE_APPROX(m_x - pkChrVictim->GetX(), m_y - pkChrVictim->GetY()) > 500)
		{
			if (test_server)
				sys_log(0, "XXX target too far %s", m_pkChr->GetName());

			return;
		}

		if (!pkChrVictim->IsPC())
			return;

		++m_iCount;

		int iDam;

		iDam = pkChrVictim->GetMaxHP() * 0.2;

		EDamageType dt = EDamageType::DAMAGE_TYPE_MELEE;
		if (!pkChrVictim->Damage(m_pkChr, iDam, dt) && !pkChrVictim->IsStun())
		{
			float fCrushSlidingLength = 200;

			if (m_pkChr->IsNPC())
				fCrushSlidingLength = 400;

			float fx, fy;
			float degree = GetDegreeFromPositionXY(m_pkChr->GetX(), m_pkChr->GetY(), pkChrVictim->GetX(), pkChrVictim->GetY());

			GetDeltaByDegree(degree, fCrushSlidingLength, &fx, &fy);
			sys_log(0, "CRUSH! %s -> %s (%d %d) -> (%d %d)", m_pkChr->GetName(), pkChrVictim->GetName(), pkChrVictim->GetX(), pkChrVictim->GetY(), (long)(pkChrVictim->GetX() + fx), (long)(pkChrVictim->GetY() + fy));
			long tx = (long)(pkChrVictim->GetX() + fx);
			long ty = (long)(pkChrVictim->GetY() + fy);

			pkChrVictim->Sync(tx, ty);
			pkChrVictim->Goto(tx, ty);
			pkChrVictim->CalculateMoveDuration();

			pkChrVictim->SyncPacket();
		}
	}

	int		m_x;
	int		m_y;
	LPCHARACTER	m_pkChr;
	int		m_iCount;
	uint8_t m_bUseSkillPower;
};

EVENTINFO(r_snakeskill_info)
{
	int	x;
	int	y;
	LPCHARACTER pkVictim;
	uint8_t bUseSkillPower;
};

EVENTFUNC(r_snakeskill_event)
{
	r_snakeskill_info* pEventInfo = dynamic_cast<r_snakeskill_info*>(event->info);

	if (!pEventInfo)
		return 0;

	if (!pEventInfo->pkVictim)
		return 0;

	FuncSnakeSplashDamage f(pEventInfo->x, pEventInfo->y, pEventInfo->pkVictim, 1);
	if (pEventInfo->pkVictim->GetSectree())
		pEventInfo->pkVictim->GetSectree()->ForEachAround(f);

	return 0;
}

int CHARACTER::ComputeSnakeSkill(uint32_t dwVnum, LPCHARACTER pkVictim, uint8_t bSkillLevel)
{
	if (!pkVictim)
		return BATTLE_NONE;

	if (m_pkSnakeSkillEvent != nullptr)
		event_cancel(&m_pkSnakeSkillEvent);
	m_pkSnakeSkillEvent = nullptr;

	r_snakeskill_info* pEventInfo = AllocEventInfo<r_snakeskill_info>();

	pEventInfo->x = pkVictim->GetX();
	pEventInfo->y = pkVictim->GetY();
	pEventInfo->pkVictim = this;
	pEventInfo->bUseSkillPower = 1;
	m_pkSnakeSkillEvent = event_create(r_snakeskill_event, pEventInfo, PASSES_PER_SEC(2));

	return BATTLE_NONE;
}
#endif

bool CHARACTER::UseSkill(uint32_t dwVnum, LPCHARACTER pkVictim, bool bUseGrandMaster)
{
	if (false == CanUseSkill(dwVnum))
		return false;

	//@fixme459 begin
	if ((dwVnum == SKILL_GEOMKYUNG || dwVnum == SKILL_GWIGEOM) && !GetWear(WEAR_WEAPON)) // Cannot use Sword Aura / Enchant Blade without weapon
		return false;
	//@fixme459 end

#ifdef ENABLE_BLOCK_SKILL_OXEVENT
	if (GetMapIndex() == MAP_OXEVENT)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1941]");
		return false;
	}
#endif

	// NO_GRANDMASTER
	if (test_server)
	{
		if (quest::CQuestManager::Instance().GetEventFlag("no_grand_master"))
		{
			bUseGrandMaster = false;
		}
	}
	// END_OF_NO_GRANDMASTER

	if (g_bSkillDisable)
		return false;

	if (IsObserverMode())
		return false;

	if (!CanMove())
		return false;

	if (IsPolymorphed())
		return false;

	const bool bCanUseHorseSkill = CanUseHorseSkill();

	if (dwVnum == SKILL_HORSE_SUMMON)
	{
		if (GetSkillLevel(dwVnum) == 0)
			return false;

		if (GetHorseLevel() <= 0)
			ChatPacket(CHAT_TYPE_INFO, "[LS;966]");
		else
			ChatPacket(CHAT_TYPE_INFO, "[LS;967]");

		return true;
	}

	// If you are on horseback but the skill cannot be used, return false
	if (false == bCanUseHorseSkill && true == IsRiding())
		return false;

	CSkillProto* pkSk = CSkillManager::Instance().Get(dwVnum);
	sys_log(0, "%s: USE_SKILL: %d pkVictim %p", GetName(), dwVnum, get_pointer(pkVictim));

	if (!pkSk)
		return false;

	if (bCanUseHorseSkill && pkSk->dwType != SKILL_TYPE_HORSE)
		return BATTLE_NONE;

	if (!bCanUseHorseSkill && pkSk->dwType == SKILL_TYPE_HORSE)
		return BATTLE_NONE;

	if (GetSkillLevel(dwVnum) == 0)
		return false;


	// NO_GRANDMASTER
	if (GetSkillMasterType(dwVnum) < SKILL_GRAND_MASTER)
		bUseGrandMaster = false;
	// END_OF_NO_GRANDMASTER

	// MINING
	if (GetWear(WEAR_WEAPON) && (GetWear(WEAR_WEAPON)->GetType() == ITEM_ROD || GetWear(WEAR_WEAPON)->GetType() == ITEM_PICK))
		return false;
	// END_OF_MINING

	m_SkillUseInfo[dwVnum].TargetVIDMap.clear();

	if (pkSk->IsChargeSkill())
	{
		if ((IsAffectFlag(AFF_TANHWAN_DASH)) || (pkVictim && (pkVictim != this)))
		{
			if (!pkVictim)
				return false;

			if (!IsAffectFlag(AFF_TANHWAN_DASH))
			{
				if (!UseSkill(dwVnum, this))
					return false;
			}

			m_SkillUseInfo[dwVnum].SetMainTargetVID(pkVictim->GetVID());
			// Bullets in DASH state are offensive skills
			ComputeSkill(dwVnum, pkVictim);
			RemoveAffect(dwVnum);
			return true;
		}
	}

	if (dwVnum == SKILL_COMBO)
	{
		if (m_bComboIndex)
			m_bComboIndex = 0;
		else
			m_bComboIndex = GetSkillLevel(SKILL_COMBO);

		ChatPacket(CHAT_TYPE_COMMAND, "combo %d", m_bComboIndex);
		return true;
	}

	// Do not use SP when Toggle (Separated by SelfOnly)
	if ((0 != pkSk->dwAffectFlag || pkSk->dwVnum == SKILL_MUYEONG) && (pkSk->dwFlag & SKILL_FLAG_TOGGLE) && RemoveAffect(pkSk->dwVnum))
	{
		return true;
	}

	if (IsAffectFlag(AFF_REVIVE_INVISIBLE))
		RemoveAffect(AFFECT_REVIVE_INVISIBLE);

	const float k = 1.0f * GetSkillPower(pkSk->dwVnum) * pkSk->bMaxLevel / 100;

	pkSk->SetPointVar("k", k);
	pkSk->kSplashAroundDamageAdjustPoly.SetVar("k", k);

	// Cool time check
	pkSk->kCooldownPoly.SetVar("k", k);
	const int iCooltime = static_cast<int>(pkSk->kCooldownPoly.Eval());
	const int lMaxHit = pkSk->lMaxHit ? pkSk->lMaxHit : -1;

	pkSk->SetSPCostVar("k", k);

	const uint32_t dwCur = get_dword_time();

	if (dwVnum == SKILL_TERROR && m_SkillUseInfo[dwVnum].bUsed && m_SkillUseInfo[dwVnum].dwNextSkillUsableTime > dwCur)
	{
		sys_log(0, " SKILL_TERROR's Cooltime is not delta over %u", m_SkillUseInfo[dwVnum].dwNextSkillUsableTime - dwCur);
		return false;
	}

	int iNeededSP = 0;

	if (IS_SET(pkSk->dwFlag, SKILL_FLAG_USE_HP_AS_COST))
	{
		pkSk->SetSPCostVar("maxhp", GetMaxHP());
		pkSk->SetSPCostVar("v", GetHP());
		iNeededSP = static_cast<int>(pkSk->kSPCostPoly.Eval());

		// ADD_GRANDMASTER_SKILL
		if (GetSkillMasterType(dwVnum) >= SKILL_GRAND_MASTER && bUseGrandMaster)
		{
			iNeededSP = static_cast<int>(pkSk->kGrandMasterAddSPCostPoly.Eval());
		}
		// END_OF_ADD_GRANDMASTER_SKILL

		if (GetHP() < iNeededSP)
			return false;

		PointChange(POINT_HP, -iNeededSP);
	}
	else
	{
		// SKILL_FOMULA_REFACTORING
		pkSk->SetSPCostVar("maxhp", GetMaxHP());
		pkSk->SetSPCostVar("maxv", GetMaxSP());
		pkSk->SetSPCostVar("v", GetSP());

		iNeededSP = static_cast<int>(pkSk->kSPCostPoly.Eval());

		if (GetSkillMasterType(dwVnum) >= SKILL_GRAND_MASTER && bUseGrandMaster)
		{
			iNeededSP = static_cast<int>(pkSk->kGrandMasterAddSPCostPoly.Eval());
		}
		// END_OF_SKILL_FOMULA_REFACTORING

		if (GetSP() < iNeededSP)
			return false;

		if (test_server)
			ChatPacket(CHAT_TYPE_INFO, "%s FP-Verbrauch: %d", pkSk->szName, iNeededSP);

		PointChange(POINT_SP, -iNeededSP);
	}

	if (IS_SET(pkSk->dwFlag, SKILL_FLAG_SELFONLY))
		pkVictim = this;
#ifdef ENABLE_WOLFMAN_CHARACTER
	else if (IS_SET(pkSk->dwFlag, SKILL_FLAG_PARTY)
#ifdef ENABLE_SHAMAN_PARTY_BUFF_SYSTEM
		&& !IsBuffSkill(pkSk->dwVnum)
#endif
		)
		pkVictim = this;
#endif

	if ((pkSk->dwVnum == SKILL_MUYEONG) || (pkSk->IsChargeSkill() && !IsAffectFlag(AFF_TANHWAN_DASH) && !pkVictim))
	{
		// Moo Young-jin, who is using it for the first time, attaches an Affect to herself.
		pkVictim = this;
	}

	constexpr int iSplashCount = 1;

	if (false == m_bDisableCooltime)
	{
#ifdef ENABLE_PVP_BALANCE
		// Ninja skill
		if (dwVnum == SKILL_GYEONGGONG)
		{
			if (false ==
				m_SkillUseInfo[dwVnum].UseSkill(
					bUseGrandMaster,
					(nullptr != pkVictim && SKILL_HORSE_WILDATTACK != dwVnum) ? pkVictim->GetVID() : 0,
					ComputeCooltime(iCooltime * 1000),
					iSplashCount,
					25000))
			{
				if (test_server)
					ChatPacket(CHAT_TYPE_NOTICE, "cooltime not finished %s %d", pkSk->szName, iCooltime);

				return false;
			}
		}
		else
		{
			if (false ==
				m_SkillUseInfo[dwVnum].UseSkill(
					bUseGrandMaster,
					(nullptr != pkVictim && SKILL_HORSE_WILDATTACK != dwVnum) ? pkVictim->GetVID() : 0,
					ComputeCooltime(iCooltime * 1000),
					iSplashCount,
					lMaxHit))
			{
				if (test_server)
					ChatPacket(CHAT_TYPE_NOTICE, "cooltime not finished %s %d", pkSk->szName, iCooltime);

				return false;
			}

		}
#else
		if (false ==
			m_SkillUseInfo[dwVnum].UseSkill(
				bUseGrandMaster,
				(nullptr != pkVictim && SKILL_HORSE_WILDATTACK != dwVnum) ? pkVictim->GetVID() : 0,
				ComputeCooltime(iCooltime * 1000),
				iSplashCount,
				lMaxHit))
		{
			if (test_server)
				ChatPacket(CHAT_TYPE_NOTICE, "cooltime not finished %s %d", pkSk->szName, iCooltime);

			return false;
		}
#endif
	}

	if (dwVnum == SKILL_CHAIN)
	{
		ResetChainLightningIndex();
		AddChainLightningExcept(pkVictim);
	}

#ifdef ENABLE_PVP_BALANCE
	// Shaman skill
	if (dwVnum == SKILL_PAERYONG)
		ComputeSkill(dwVnum, pkVictim);
#endif

#ifdef ENABLE_NINETH_SKILL
	if (dwVnum == SKILL_METEO)
		ComputeSkill(dwVnum, pkVictim);
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
	if (pkVictim != nullptr && (dwVnum == SKILL_HOSIN || dwVnum == SKILL_REFLECT || dwVnum == SKILL_GICHEON || dwVnum == SKILL_KWAESOK || dwVnum == SKILL_JEUNGRYEOK
#	ifdef ENABLE_NINETH_SKILL
		|| dwVnum == SKILL_CHEONUN
#	endif
		))
	{
		uint8_t skill = 0;
		uint8_t id = 0;
		switch (dwVnum)
		{
			case SKILL_HOSIN:
				skill = ESkillColorLength::BUFF_BEGIN + 0;
				id = 3;
				break;
			case SKILL_REFLECT:
				skill = ESkillColorLength::BUFF_BEGIN + 1;
				id = 4;
				break;
			case SKILL_GICHEON:
				skill = ESkillColorLength::BUFF_BEGIN + 2;
				id = 5;
				break;
			case SKILL_KWAESOK:
				skill = ESkillColorLength::BUFF_BEGIN + 3;
				id = 4;
				break;
			case SKILL_JEUNGRYEOK:
				skill = ESkillColorLength::BUFF_BEGIN + 4;
				id = 5;
				break;
#	ifdef ENABLE_NINETH_SKILL
			case SKILL_CHEONUN:
				skill = ESkillColorLength::BUFF_BEGIN + 5;
				id = 9;
				break;
#	endif
			default:
				break;
		}

		uint32_t data[ESkillColorLength::MAX_SKILL_COUNT + ESkillColorLength::MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
		memcpy(data, pkVictim->GetSkillColor(), sizeof(data));

		uint32_t dataAttacker[ESkillColorLength::MAX_SKILL_COUNT + ESkillColorLength::MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
		memcpy(dataAttacker, this->GetSkillColor(), sizeof(dataAttacker));

		data[skill][0] = dataAttacker[id][0];
		data[skill][1] = dataAttacker[id][1];
		data[skill][2] = dataAttacker[id][2];
		data[skill][3] = dataAttacker[id][3];
		data[skill][4] = dataAttacker[id][4];

		pkVictim->SetSkillColor(data[0]);

		TSkillColor db_pack;
		memcpy(db_pack.dwSkillColor, data, sizeof(data));
		db_pack.player_id = pkVictim->GetPlayerID();
		db_clientdesc->DBPacketHeader(HEADER_GD_SKILL_COLOR_SAVE, 0, sizeof(TSkillColor));
		db_clientdesc->Packet(&db_pack, sizeof(TSkillColor));
	}
#endif

	if (IS_SET(pkSk->dwFlag, SKILL_FLAG_SELFONLY))
		ComputeSkill(dwVnum, this);

#ifdef ENABLE_WOLFMAN_CHARACTER
	else if (IS_SET(pkSk->dwFlag, SKILL_FLAG_PARTY))
	{
#	ifdef ENABLE_SHAMAN_PARTY_BUFF_SYSTEM
		if (IsBuffSkill(dwVnum) && (pkVictim && ((!pkVictim->GetParty()) || (pkVictim->GetParty() != GetParty()))))
		{
			ComputeSkill(dwVnum, pkVictim);
		}
		else
#	endif
		{
			ComputeSkillParty(dwVnum, this);
		}
	}
#	endif
	else if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_ATTACK))
		ComputeSkill(dwVnum, pkVictim);
	else if (dwVnum == SKILL_BYEURAK)
		ComputeSkill(dwVnum, pkVictim);
	else if (dwVnum == SKILL_MUYEONG || pkSk->IsChargeSkill())
		ComputeSkill(dwVnum, pkVictim);

	m_dwLastSkillTime = get_dword_time();

	return true;
}

int CHARACTER::GetUsedSkillMasterType(uint32_t dwVnum)
{
	const TSkillUseInfo& rInfo = m_SkillUseInfo[dwVnum];

	if (GetSkillMasterType(dwVnum) < SKILL_GRAND_MASTER)
		return GetSkillMasterType(dwVnum);

	if (rInfo.isGrandMaster)
		return GetSkillMasterType(dwVnum);

	return MIN(GetSkillMasterType(dwVnum), SKILL_MASTER);
}

int CHARACTER::GetSkillMasterType(uint32_t dwVnum) const
{
	if (!IsPC())
		return 0;

	if (dwVnum >= SKILL_MAX_NUM)
	{
		sys_err("%s skill vnum overflow %u", GetName(), dwVnum);
		return 0;
	}

	return m_pSkillLevels ? m_pSkillLevels[dwVnum].bMasterType : SKILL_NORMAL;
}

int CHARACTER::GetSkillPower(uint32_t dwVnum, uint8_t bLevel) const
{
	// Mermaid ring item
	if (dwVnum >= SKILL_LANGUAGE1 && dwVnum <= SKILL_LANGUAGE3 && IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_LANGUAGE))
	{
		return 100;
	}

	if (dwVnum >= GUILD_SKILL_START && dwVnum <= GUILD_SKILL_END)
	{
		if (GetGuild())
			return 100 * GetGuild()->GetSkillLevel(dwVnum) / 7 / 7;
		else
			return 0;
	}

	if (bLevel)
	{
		//SKILL_POWER_BY_LEVEL
		return GetSkillPowerByLevel(bLevel, true);
		//END_SKILL_POWER_BY_LEVEL;
	}

	if (dwVnum >= SKILL_MAX_NUM)
	{
		sys_err("%s skill vnum overflow %u", GetName(), dwVnum);
		return 0;
	}

	//SKILL_POWER_BY_LEVEL
	return GetSkillPowerByLevel(GetSkillLevel(dwVnum));
	//SKILL_POWER_BY_LEVEL
}

int CHARACTER::GetSkillLevel(uint32_t dwVnum) const
{
	if (dwVnum >= SKILL_MAX_NUM)
	{
		sys_err("%s skill vnum overflow %u", GetName(), dwVnum);
		sys_log(0, "%s skill vnum overflow %u", GetName(), dwVnum);
		return 0;
	}

#ifdef ENABLE_BATTLE_FIELD
	bool returnNormal = true;

	if (CBattleField::Instance().IsBattleZoneMapIndex(GetMapIndex()))
	{
		const CSkillProto* pkSk = CSkillManager::Instance().Get(dwVnum);
		if (pkSk)
		{
			if (pkSk->dwType > 0 && pkSk->dwType <= 7)
			{
				returnNormal = GetSkillMasterType(dwVnum) >= SKILL_MASTER ? false : true;
			}
		}
	}

	return (returnNormal == false ? SKILL_MAX_LEVEL : MIN(SKILL_MAX_LEVEL, m_pSkillLevels ? m_pSkillLevels[dwVnum].bLevel : 0));
#else
	return MIN(SKILL_MAX_LEVEL, m_pSkillLevels ? m_pSkillLevels[dwVnum].bLevel : 0);
#endif
}

EVENTFUNC(skill_muyoung_event)
{
	const char_event_info* info = dynamic_cast<char_event_info*>(event->info);

	if (info == nullptr)
	{
		sys_err("skill_muyoung_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == nullptr)// <Factor>
		return 0;

	if (!ch->IsAffectFlag(AFF_MUYEONG))
	{
		ch->StopMuyeongEvent();
		return 0;
	}

	// 1. Find Victim
	FFindNearVictim f(ch, ch);
	if (ch->GetSectree())
	{
		ch->GetSectree()->ForEachAround(f);
		// 2. Shoot!
		if (f.GetVictim())
		{
			ch->CreateFly(FLY_SKILL_MUYEONG, f.GetVictim());
			ch->ComputeSkill(SKILL_MUYEONG, f.GetVictim());
		}
	}

	return PASSES_PER_SEC(3);
}

void CHARACTER::StartMuyeongEvent()
{
	if (m_pkMuyeongEvent)
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();

	info->ch = this;
	m_pkMuyeongEvent = event_create(skill_muyoung_event, info, PASSES_PER_SEC(1));
}

void CHARACTER::StopMuyeongEvent()
{
	event_cancel(&m_pkMuyeongEvent);
}

#ifdef ENABLE_PVP_BALANCE
EVENTFUNC(skill_gyeongGong_event)
{
	const char_event_info* info = dynamic_cast<char_event_info*>(event->info);

	if (info == nullptr)
	{
		sys_err("skill_gyeongGong_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == nullptr) // <Factor>
		return 0;

	if (!ch->IsAffectFlag(AFF_GYEONGGONG))
	{
		ch->StopGyeongGongEvent();
		return 0;
	}

	ch->ComputeGyeongGongSkill(SKILL_GYEONGGONG, ch);

	return PASSES_PER_SEC(2);
}

void CHARACTER::StartGyeongGongEvent()
{
	if (m_pkGyeongGongEvent)
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();

	info->ch = this;
	m_pkGyeongGongEvent = event_create(skill_gyeongGong_event, info, PASSES_PER_SEC(1));
}

void CHARACTER::StopGyeongGongEvent()
{
	event_cancel(&m_pkGyeongGongEvent);
}
#endif

#ifdef ENABLE_NINETH_SKILL
EVENTFUNC(skill_cheonun_event)
{
	const cheonun_event_info* info = dynamic_cast<cheonun_event_info*>(event->info);

	if (info == nullptr)
	{
		sys_err("skill_cheonun_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;
	const auto bShieldChance = info->bShieldChance;
	const auto bShieldDuration = info->bShieldDur;

	if (ch == nullptr || bShieldChance < 0)
		return 0;

	if (!ch->IsAffectFlag(AFF_CHEONUN))
	{
		ch->StopCheonunEvent();
		return 0;
	}

	const auto bPercent = static_cast<uint8_t>(number(1, 100));
	if (bPercent <= bShieldChance)
		ch->AddAffect(2000, POINT_NONE, 0, AFF_CHUNWOON_MOOJUK, bShieldDuration, 0, true);

	if (test_server)
	{
		ch->ChatPacket(CHAT_TYPE_PARTY, "Random Percent : %d", bPercent);
		ch->ChatPacket(CHAT_TYPE_PARTY, "Shield chance : %d", bShieldChance);
		ch->ChatPacket(CHAT_TYPE_PARTY, "Shield Duration : %d", bShieldDuration);
	}

	return PASSES_PER_SEC(12);
}

void CHARACTER::StartCheonunEvent(uint8_t bShieldChance, uint8_t bShieldDuration)
{
	if (m_pkCheonunEvent)
		return;

	cheonun_event_info* info = AllocEventInfo<cheonun_event_info>();

	info->ch = this;
	info->bShieldChance = bShieldChance;
	info->bShieldDur = bShieldDuration;

	m_pkCheonunEvent = event_create(skill_cheonun_event, info, PASSES_PER_SEC(1));
}

void CHARACTER::StopCheonunEvent()
{
	event_cancel(&m_pkCheonunEvent);
}
#endif

void CHARACTER::SkillLearnWaitMoreTimeMessage(uint32_t ms)
{
	//const char* str = "";
	//
	if (ms < 3 * 60)
		ChatPacket(CHAT_TYPE_TALKING, "[LS;970]");
	else if (ms < 5 * 60)
		ChatPacket(CHAT_TYPE_TALKING, "[LS;971]");
	else if (ms < 10 * 60) // 10 minutes
		ChatPacket(CHAT_TYPE_TALKING, "[LS;972]");
	else if (ms < 30 * 60) // 30 minutes
	{
		ChatPacket(CHAT_TYPE_TALKING, "[LS;973]");
		ChatPacket(CHAT_TYPE_TALKING, "[LS;974]");
	}
	else if (ms < 1 * 3600) // 1 hour
		ChatPacket(CHAT_TYPE_TALKING, "[LS;975]");
	else if (ms < 2 * 3600) // 2 hours
		ChatPacket(CHAT_TYPE_TALKING, "[LS;976]");
	else if (ms < 3 * 3600)
		ChatPacket(CHAT_TYPE_TALKING, "[LS;977]");
	else if (ms < 6 * 3600)
	{
		ChatPacket(CHAT_TYPE_TALKING, "[LS;978]");
		ChatPacket(CHAT_TYPE_TALKING, "[LS;979]");
	}
	else if (ms < 12 * 3600)
	{
		ChatPacket(CHAT_TYPE_TALKING, "[LS;981]");
		ChatPacket(CHAT_TYPE_TALKING, "[LS;982]");
	}
	else if (ms < 18 * 3600)
	{
		ChatPacket(CHAT_TYPE_TALKING, "[LS;983]");
		ChatPacket(CHAT_TYPE_TALKING, "[LS;984]");
	}
	else //if (ms < 2 * 86400)
	{
		ChatPacket(CHAT_TYPE_TALKING, "[LS;985]");
		ChatPacket(CHAT_TYPE_TALKING, "[LS;986]");
	}
	/*
	str = "30%";
	else if (ms < 3 * 86400)
		str = "10%";
	else if (ms < 4 * 86400)
		str = "5%";
	else
		str = "0%";*/

	//ChatPacket(CHAT_TYPE_TALKING, "%s", str);
}

void CHARACTER::DisableCooltime()
{
	m_bDisableCooltime = true;
}

bool CHARACTER::HasMobSkill() const
{
	return CountMobSkill() > 0;
}

size_t CHARACTER::CountMobSkill() const
{
	if (!m_pkMobData)
		return 0;

	size_t c = 0;

	for (size_t i = 0; i < MOB_SKILL_MAX_NUM; ++i)
	{
		if (m_pkMobData->m_table.Skills[i].dwVnum)
			++c;
	}

	return c;
}

const TMobSkillInfo* CHARACTER::GetMobSkill(uint32_t idx) const
{
	if (idx >= MOB_SKILL_MAX_NUM)
		return nullptr;

	if (!m_pkMobData)
		return nullptr;

	if (0 == m_pkMobData->m_table.Skills[idx].dwVnum)
		return nullptr;

	return &m_pkMobData->m_mobSkillInfo[idx];
}

bool CHARACTER::CanUseMobSkill(uint32_t idx) const
{
	const TMobSkillInfo* pInfo = GetMobSkill(idx);

	if (!pInfo)
		return false;

	if (m_adwMobSkillCooltime[idx] > get_dword_time())
		return false;

	if (number(0, 1))
		return false;

	return true;
}

EVENTINFO(mob_skill_event_info)
{
	DynamicCharacterPtr ch;
	PIXEL_POSITION pos;
	uint32_t vnum;
	int index;
	uint8_t level;

	mob_skill_event_info()
		: ch()
		, pos()
		, vnum(0)
		, index(0)
		, level(0)
	{
	}
};

EVENTFUNC(mob_skill_hit_event)
{
	const mob_skill_event_info* info = dynamic_cast<mob_skill_event_info*>(event->info);

	if (info == nullptr)
	{
		sys_err("mob_skill_event_info> <Factor> Null pointer");
		return 0;
	}

	// <Factor>
	LPCHARACTER ch = info->ch;
	if (ch == nullptr)
		return 0;

	ch->ComputeSkillAtPosition(info->vnum, info->pos, info->level);
	ch->m_mapMobSkillEvent.erase(info->index);

	return 0;
}

bool CHARACTER::UseMobSkill(uint32_t idx)
{
	if (IsPC())
		return false;

#ifdef ENABLE_DAWNMIST_DUNGEON
	if (CDawnMistDungeon::Instance().IsForestDungeonBoss(this) && idx > CDawnMistDungeon::SPECIAL_SKILL_INDEX)
		idx = CDawnMistDungeon::SPECIAL_SKILL_INDEX;
#endif

	const TMobSkillInfo* pInfo = GetMobSkill(idx);
	if (!pInfo)
		return false;

	const auto dwVnum = pInfo->dwSkillVnum;
	CSkillProto* pkSk = CSkillManager::Instance().Get(dwVnum);

	if (!pkSk)
		return false;

	const float k = 1.0f * GetSkillPower(pkSk->dwVnum, pInfo->bSkillLevel) * pkSk->bMaxLevel / 100;

	pkSk->kCooldownPoly.SetVar("k", k);
	const int iCooltime = static_cast<int>(pkSk->kCooldownPoly.Eval() * 1000);

	m_adwMobSkillCooltime[idx] = get_dword_time() + iCooltime;

	sys_log(0, "USE_MOB_SKILL: %s idx %d vnum %u cooltime %d", GetName(), idx, dwVnum, iCooltime);

#ifdef ENABLE_DAWNMIST_DUNGEON
	if ((IsMonster()) && (pkSk->dwVnum == 265))
	{
		LPPARTY pkParty = GetParty();
		if (pkParty && IS_SET(pkSk->dwFlag, SKILL_FLAG_PARTY))
		{
			CDawnMistDungeon::FHeal f(0);
			pkParty->ForEachMemberPtr(f);
		}

		return true;
	}
#endif

	if (m_pkMobData->m_mobSkillInfo[idx].vecSplashAttack.empty())
	{
		sys_err("No skill hit data for mob %s index %d", GetName(), idx);
		return false;
	}

	for (size_t i = 0; i < m_pkMobData->m_mobSkillInfo[idx].vecSplashAttack.size(); i++)
	{
		PIXEL_POSITION pos = GetXYZ();
		const TMobSplashAttackInfo& rInfo = m_pkMobData->m_mobSkillInfo[idx].vecSplashAttack[i];

		if (rInfo.dwHitDistance)
		{
			float fx, fy;
			GetDeltaByDegree(GetRotation(), static_cast<float>(rInfo.dwHitDistance), &fx, &fy);
			pos.x += static_cast<long>(fx);
			pos.y += static_cast<long>(fy);
		}

		if (rInfo.dwTiming)
		{
			if (test_server)
				sys_log(0, " timing %ums", rInfo.dwTiming);

			mob_skill_event_info* info = AllocEventInfo<mob_skill_event_info>();

			info->ch = this;
			info->pos = pos;
			info->level = pInfo->bSkillLevel;
			info->vnum = dwVnum;
			info->index = i;

			// <Factor> Cancel existing event first
			const auto it = m_mapMobSkillEvent.find(i);
			if (it != m_mapMobSkillEvent.end())
			{
				LPEVENT existing = it->second;
				event_cancel(&existing);
				m_mapMobSkillEvent.erase(it);
			}

			m_mapMobSkillEvent.insert(std::make_pair(i, event_create(mob_skill_hit_event, info, PASSES_PER_SEC(rInfo.dwTiming) / 1000)));
		}
		else
		{
			ComputeSkillAtPosition(dwVnum, pos, pInfo->bSkillLevel);
		}
	}

	return true;
}

void CHARACTER::ResetMobSkillCooltime()
{
	memset(m_adwMobSkillCooltime, 0, sizeof(m_adwMobSkillCooltime));
}

bool CHARACTER::IsUsableSkillMotion(uint32_t dwMotionIndex) const
{
	const uint32_t selfJobGroup = (GetJob() + 1) * 10 + GetSkillGroup();

	constexpr uint32_t SKILL_NUM = 158
#ifdef ENABLE_WOLFMAN_CHARACTER
		+ 18
#endif
#ifdef ENABLE_NINETH_SKILL
		+ 8
#endif
		;

	static uint32_t s_anSkill2JobGroup[SKILL_NUM] =
	{
		0, // common_skill 0

		//WARRIOR_1
		11, // job_skill 1
		11, // job_skill 2
		11, // job_skill 3
		11, // job_skill 4
		11, // job_skill 5
		11, // job_skill 6
		0, // common_skill 7
		0, // common_skill 8
		0, // common_skill 9
		0, // common_skill 10
		0, // common_skill 11
		0, // common_skill 12
		0, // common_skill 13
		0, // common_skill 14
		0, // common_skill 15

		//WARRIOR_2
		12, // job_skill 16
		12, // job_skill 17
		12, // job_skill 18
		12, // job_skill 19
		12, // job_skill 20
		12, // job_skill 21
		0, // common_skill 22
		0, // common_skill 23
		0, // common_skill 24
		0, // common_skill 25
		0, // common_skill 26
		0, // common_skill 27
		0, // common_skill 28
		0, // common_skill 29
		0, // common_skill 30

		//ASSASSIN_1
		21, // job_skill 31
		21, // job_skill 32
		21, // job_skill 33
		21, // job_skill 34
		21, // job_skill 35
		21, // job_skill 36
		0, // common_skill 37
		0, // common_skill 38
		0, // common_skill 39
		0, // common_skill 40
		0, // common_skill 41
		0, // common_skill 42
		0, // common_skill 43
		0, // common_skill 44
		0, // common_skill 45

		//ASSASSIN_2
		22, // job_skill 46
		22, // job_skill 47
		22, // job_skill 48
		22, // job_skill 49
		22, // job_skill 50
		22, // job_skill 51
		0, // common_skill 52
		0, // common_skill 53
		0, // common_skill 54
		0, // common_skill 55
		0, // common_skill 56
		0, // common_skill 57
		0, // common_skill 58
		0, // common_skill 59
		0, // common_skill 60

		//SURA_1
		31, // job_skill 61
		31, // job_skill 62
		31, // job_skill 63
		31, // job_skill 64
		31, // job_skill 65
		31, // job_skill 66
		0, // common_skill 67
		0, // common_skill 68
		0, // common_skill 69
		0, // common_skill 70
		0, // common_skill 71
		0, // common_skill 72
		0, // common_skill 73
		0, // common_skill 74
		0, // common_skill 75

		//SURA_2
		32, // job_skill 76
		32, // job_skill 77
		32, // job_skill 78
		32, // job_skill 79
		32, // job_skill 80
		32, // job_skill 81
		0, // common_skill 82
		0, // common_skill 83
		0, // common_skill 84
		0, // common_skill 85
		0, // common_skill 86
		0, // common_skill 87
		0, // common_skill 88
		0, // common_skill 89
		0, // common_skill 90

		//SHAMAN_1
		41, // job_skill 91
		41, // job_skill 92
		41, // job_skill 93
		41, // job_skill 94
		41, // job_skill 95
		41, // job_skill 96
		0, // common_skill 97
		0, // common_skill 98
		0, // common_skill 99
		0, // common_skill 100
		0, // common_skill 101
		0, // common_skill 102
		0, // common_skill 103
		0, // common_skill 104
		0, // common_skill 105

		//SHAMAN_2
		42, // job_skill 106
		42, // job_skill 107
		42, // job_skill 108
		42, // job_skill 109
		42, // job_skill 110
		42, // job_skill 111
		0, // common_skill 112
		0, // common_skill 113
		0, // common_skill 114
		0, // common_skill 115
		0, // common_skill 116
		0, // common_skill 117
		0, // common_skill 118
		0, // common_skill 119
		0, // common_skill 120

		0, // common_skill 121
		0, // common_skill 122
		0, // common_skill 123
		0, // common_skill 124
		0, // common_skill 125
		0, // common_skill 126
		0, // common_skill 127
		0, // common_skill 128
		0, // common_skill 129
		0, // common_skill 130
		0, // common_skill 131
		0, // common_skill 132
		0, // common_skill 133
		0, // common_skill 134
		0, // common_skill 135
		0, // common_skill 136
		0, // job_skill 137
		0, // job_skill 138
		0, // job_skill 139
		0, // job_skill 140
		0, // common_skill 141
		0, // common_skill 142
		0, // common_skill 143
		0, // common_skill 144
		0, // common_skill 145
		0, // common_skill 146
		0, // common_skill 147
		0, // common_skill 148
		0, // common_skill 149
		0, // common_skill 150
		0, // common_skill 151
		0, // job_skill 152
		0, // job_skill 153
		0, // job_skill 154
		0, // job_skill 155
		0, // job_skill 156
		0, // job_skill 157
#ifdef ENABLE_WOLFMAN_CHARACTER
		0, // empty(reserved) 158
		0, // empty(reserved) 159
		0, // empty(reserved) 160
		0, // empty(reserved) 161
		0, // empty(reserved) 162
		0, // empty(reserved) 163
		0, // empty(reserved) 164
		0, // empty(reserved) 165
		0, // empty(reserved) 166
		0, // empty(reserved) 167
		0, // empty(reserved) 168
		0, // empty(reserved) 169

		//WOLFMAN
		51, // job_skill(WOLFMAN SKILL) 170
		51, // job_skill(WOLFMAN SKILL) 171
		51, // job_skill(WOLFMAN SKILL) 172
		51, // job_skill(WOLFMAN SKILL) 173
		51, // job_skill(WOLFMAN SKILL) 174
		51, // job_skill(WOLFMAN SKILL) 175
#endif

#ifdef ENABLE_NINETH_SKILL
		0, // job_skill(CONQUEROR SKILL) 176
		0, // job_skill(CONQUEROR SKILL) 177
		0, // job_skill(CONQUEROR SKILL) 178
		0, // job_skill(CONQUEROR SKILL) 179
		0, // job_skill(CONQUEROR SKILL) 180
		0, // job_skill(CONQUEROR SKILL) 181
		0, // job_skill(CONQUEROR SKILL) 182
#	ifdef ENABLE_WOLFMAN_CHARACTER
		0, // job_skill(CONQUEROR SKILL) 183
#	endif
#endif
	}; // s_anSkill2JobGroup

	constexpr uint32_t MOTION_MAX_NUM = 124;
#ifdef ENABLE_WOLFMAN_CHARACTER
	constexpr uint32_t SKILL_LIST_MAX_COUNT = 6;
#else
	constexpr uint32_t SKILL_LIST_MAX_COUNT = 5;
#endif

	static uint32_t s_anMotion2SkillVnumList[MOTION_MAX_NUM][SKILL_LIST_MAX_COUNT] =
	{
		// Skill amount - ID Warrior - ID Assassin - ID Sura - ID Shaman - ID Wolfman
		{   0,		0,			0,			0,			0		}, //  0

		// Basic skill for job 1
#ifdef ENABLE_WOLFMAN_CHARACTER
		{   5,		1,			31,			61,			91,	170		}, //  1
		{   5,		2,			32,			62,			92,	171		}, //  2
		{   5,		3,			33,			63,			93,	172		}, //  3
		{   5,		4,			34,			64,			94,	173		}, //  4
		{   5,		5,			35,			65,			95,	174		}, //  5
		{   5,		6,			36,			66,			96,	175		}, //  6
#else
		{   4,		1,			31,			61,			91		}, //  1
		{   4,		2,			32,			62,			92		}, //  2
		{   4,		3,			33,			63,			93		}, //  3
		{   4,		4,			34,			64,			94		}, //  4
		{   4,		5,			35,			65,			95		}, //  5
		{   4,		6,			36,			66,			96		}, //  6
#endif
		{   0,		0,			0,			0,			0		}, //  7
		{   0,		0,			0,			0,			0		}, //  8
#ifdef ENABLE_NINETH_SKILL
#	ifdef ENABLE_WOLFMAN_CHARACTER
		{	5,		176,		177,		179,		181,	183 }, // 9
#	else
		{	4,		176,		177,		179,		181		}, // 9
#	endif
#endif
		// End of basic skill for job 1

		// Extra
#ifndef ENABLE_NINETH_SKILL
		{   0,		0,			0,			0,			0		}, //  9
#endif
		{   0,		0,			0,			0,			0		}, //  10
		{   0,		0,			0,			0,			0		}, //  11
		{   0,		0,			0,			0,			0		}, //  12
		{   0,		0,			0,			0,			0		}, //  13
		{   0,		0,			0,			0,			0		}, //  14
		{   0,		0,			0,			0,			0		}, //  15
		// End of extra

		// Basic skill for job 2
		{   4,		16,			46,			76,			106		}, //  16
		{   4,		17,			47,			77,			107		}, //  17
		{   4,		18,			48,			78,			108		}, //  18
		{   4,		19,			49,			79,			109		}, //  19
		{   4,		20,			50,			80,			110		}, //  20
		{   4,		21,			51,			81,			111		}, //  21
		{   0,		0,			0,			0,			0		}, //  22
		{   0,		0,			0,			0,			0		}, //  23
#ifdef ENABLE_NINETH_SKILL
		{	4,		176,		178,		180,		182		}, //  24
#endif
		// End of basic skill for job 2

		// extra
#ifndef ENABLE_NINETH_SKILL
		{   0,		0,			0,			0,			0		}, //  24
#endif
		{   0,		0,			0,			0,			0		}, //  25
		// End of extra

		// Job 1 Master Skill
#ifdef ENABLE_WOLFMAN_CHARACTER
		{   5,		1,			31,			61,			91,	170		}, //  26
		{   5,		2,			32,			62,			92,	171		}, //  27
		{   5,		3,			33,			63,			93,	172		}, //  28
		{   5,		4,			34,			64,			94,	173		}, //  29
		{   5,		5,			35,			65,			95,	174		}, //  30
		{   5,		6,			36,			66,			96,	175		}, //  31
#else
		{   4,		1,			31,			61,			91		}, //  26
		{   4,		2,			32,			62,			92		}, //  27
		{   4,		3,			33,			63,			93		}, //  28
		{   4,		4,			34,			64,			94		}, //  29
		{   4,		5,			35,			65,			95		}, //  30
		{   4,		6,			36,			66,			96		}, //  31
#endif
		{   0,		0,			0,			0,			0		}, //  32
		{   0,		0,			0,			0,			0		}, //  33
#ifdef ENABLE_NINETH_SKILL
#	ifdef ENABLE_WOLFMAN_CHARACTER
		{	5,		176,		177,		179,		181, 183	}, //  34
#	else
		{	4,		176,		177,		179,		181		}, //  34
#	endif
#endif
		// End of job 1 master skill

		// extra
#ifndef ENABLE_NINETH_SKILL
		{   0,		0,			0,			0,			0		}, //  34
#endif
		{   0,		0,			0,			0,			0		}, //  35
		{   0,		0,			0,			0,			0		}, //  36
		{   0,		0,			0,			0,			0		}, //  37
		{   0,		0,			0,			0,			0		}, //  38
		{   0,		0,			0,			0,			0		}, //  39
		{   0,		0,			0,			0,			0		}, //  40
		// end of extra

		// 2nd job master skill
		{   4,		16,			46,			76,			106		}, //  41
		{   4,		17,			47,			77,			107		}, //  42
		{   4,		18,			48,			78,			108		}, //  43
		{   4,		19,			49,			79,			109		}, //  44
		{   4,		20,			50,			80,			110		}, //  45
		{   4,		21,			51,			81,			111		}, //  46
		{   0,		0,			0,			0,			0		}, //  47
		{   0,		0,			0,			0,			0		}, //  48
#ifdef ENABLE_NINETH_SKILL
		{	4,		176,		178,		180,		182		}, //  49
#endif
		// End of job 2 master skill

		// extra
#ifndef ENABLE_NINETH_SKILL
		{   0,		0,			0,			0,			0		}, //  49
#endif
		{   0,		0,			0,			0,			0		}, //  50
		// end of extra

		// Grand Master Skills for Job 1
#ifdef ENABLE_WOLFMAN_CHARACTER
		{   5,		1,			31,			61,			91,	170		}, //  51
		{   5,		2,			32,			62,			92,	171		}, //  52
		{   5,		3,			33,			63,			93,	172		}, //  53
		{   5,		4,			34,			64,			94,	173		}, //  54
		{   5,		5,			35,			65,			95,	174		}, //  55
		{   5,		6,			36,			66,			96,	175		}, //  56
#else
		{   4,		1,			31,			61,			91		}, //  51
		{   4,		2,			32,			62,			92		}, //  52
		{   4,		3,			33,			63,			93		}, //  53
		{   4,		4,			34,			64,			94		}, //  54
		{   4,		5,			35,			65,			95		}, //  55
		{   4,		6,			36,			66,			96		}, //  56
#endif
		{   0,		0,			0,			0,			0		}, //  57
		{   0,		0,			0,			0,			0		}, //  58
#ifdef ENABLE_NINETH_SKILL
#	ifdef ENABLE_WOLFMAN_CHARACTER
		{	5,		176,		177,		179,		181, 183	}, //  59
#	else
		{	4,		176,		177,		179,		181		}, //  59
#	endif
#endif
		// End of the 1st job Grand Master skill

		// extra
#ifndef ENABLE_NINETH_SKILL
		{   0,		0,			0,			0,			0		}, //  59
#endif
		{   0,		0,			0,			0,			0		}, //  60
		{   0,		0,			0,			0,			0		}, //  61
		{   0,		0,			0,			0,			0		}, //  62
		{   0,		0,			0,			0,			0		}, //  63
		{   0,		0,			0,			0,			0		}, //  64
		{   0,		0,			0,			0,			0		}, //  65
		// end of extra

		// Grand Master Skills for Job 2
		{   4,		16,			46,			76,			106		}, //  66
		{   4,		17,			47,			77,			107		}, //  67
		{   4,		18,			48,			78,			108		}, //  68
		{   4,		19,			49,			79,			109		}, //  69
		{   4,		20,			50,			80,			110		}, //  70
		{   4,		21,			51,			81,			111		}, //  71
		{   0,		0,			0,			0,			0		}, //  72
		{   0,		0,			0,			0,			0		}, //  73
#ifdef ENABLE_NINETH_SKILL
		{	4,		176,		178,		180,		182		}, //  74
#endif
		// End of the 2nd job Grand Master skill

		// extra
#ifndef ENABLE_NINETH_SKILL
		{   0,		0,			0,			0,			0		}, //  74
#endif
		{   0,		0,			0,			0,			0		}, //  75
		// end of extra

		// Perfect Master Skill for Job 1
#ifdef ENABLE_WOLFMAN_CHARACTER
		{   5,		1,			31,			61,			91,	170		}, //  76
		{   5,		2,			32,			62,			92,	171		}, //  77
		{   5,		3,			33,			63,			93,	172		}, //  78
		{   5,		4,			34,			64,			94,	173		}, //  79
		{   5,		5,			35,			65,			95,	174		}, //  80
		{   5,		6,			36,			66,			96,	175		}, //  81
#else
		{   4,		1,			31,			61,			91		}, //  76
		{   4,		2,			32,			62,			92		}, //  77
		{   4,		3,			33,			63,			93		}, //  78
		{   4,		4,			34,			64,			94		}, //  79
		{   4,		5,			35,			65,			95		}, //  80
		{   4,		6,			36,			66,			96		}, //  81
#endif
		{   0,		0,			0,			0,			0		}, //  82
		{   0,		0,			0,			0,			0		}, //  83
#ifdef ENABLE_NINETH_SKILL
#	ifdef ENABLE_WOLFMAN_CHARACTER
		{	5,		176,		177,		179,		181, 183	}, //  84
#	else
		{	4,		176,		177,		179,		181		}, //  84
#	endif
#endif
		// End of job 1 Perfect Master skill

		// extra
#ifndef ENABLE_NINETH_SKILL
		{   0,		0,			0,			0,			0		}, //  84
#endif
		{   0,		0,			0,			0,			0		}, //  85
		{   0,		0,			0,			0,			0		}, //  86
		{   0,		0,			0,			0,			0		}, //  87
		{   0,		0,			0,			0,			0		}, //  88
		{   0,		0,			0,			0,			0		}, //  89
		{   0,		0,			0,			0,			0		}, //  90
		// end of extra

		// Perfect master skill for job 2
		{   4,		16,			46,			76,			106		}, //  91
		{   4,		17,			47,			77,			107		}, //  92
		{   4,		18,			48,			78,			108		}, //  93
		{   4,		19,			49,			79,			109		}, //  94
		{   4,		20,			50,			80,			110		}, //  95
		{   4,		21,			51,			81,			111		}, //  96
		{   0,		0,			0,			0,			0		}, //  97
		{   0,		0,			0,			0,			0		}, //  98
#ifdef ENABLE_NINETH_SKILL
		{	4,		176,		178,		180,		182		}, //  99
#endif
		// End of job 2 Perfect Master skill

		// extra
#ifndef ENABLE_NINETH_SKILL
		{   0,		0,			0,			0,			0		}, //  99
#endif
		{   0,		0,			0,			0,			0		}, //  100
		// end of extra

		// Guild Skill
		{   1,  152,    0,    0,    0}, //  101
		{   1,  153,    0,    0,    0}, //  102
		{   1,  154,    0,    0,    0}, //  103
		{   1,  155,    0,    0,    0}, //  104
		{   1,  156,    0,    0,    0}, //  105
		{   1,  157,    0,    0,    0}, //  106
		// Guild skill end

		// extra
		{   0,    0,    0,    0,    0}, //  107
		{   0,    0,    0,    0,    0}, //  108
		{   0,    0,    0,    0,    0}, //  109
		{   0,    0,    0,    0,    0}, //  110
		{   0,    0,    0,    0,    0}, //  111
		{   0,    0,    0,    0,    0}, //  112
		{   0,    0,    0,    0,    0}, //  113
		{   0,    0,    0,    0,    0}, //  114
		{   0,    0,    0,    0,    0}, //  115
		{   0,    0,    0,    0,    0}, //  116
		{   0,    0,    0,    0,    0}, //  117
		{   0,    0,    0,    0,    0}, //  118
		{   0,    0,    0,    0,    0}, //  119
		{   0,    0,    0,    0,    0}, //  120
		// end of extra

		// Horse riding skill
		{   2,  137,  140,    0,    0}, //  121
		{   1,  138,    0,    0,    0}, //  122
		{   1,  139,    0,    0,    0}, //  123
		// End of riding skill
	};

	if (dwMotionIndex >= MOTION_MAX_NUM)
	{
		sys_err("OUT_OF_MOTION_VNUM: name=%s, motion=%d/%d", GetName(), dwMotionIndex, MOTION_MAX_NUM);
		return false;
	}

	uint32_t* skillVNums = s_anMotion2SkillVnumList[dwMotionIndex];

	const uint32_t skillCount = *skillVNums++;
	if (skillCount >= SKILL_LIST_MAX_COUNT)
	{
		sys_err("OUT_OF_SKILL_LIST: name=%s, count=%d/%d", GetName(), skillCount, SKILL_LIST_MAX_COUNT);
		return false;
	}

	for (uint32_t skillIndex = 0; skillIndex != skillCount; ++skillIndex)
	{
		if (skillIndex >= SKILL_MAX_NUM)
		{
			sys_err("OUT_OF_SKILL_VNUM: name=%s, skill=%d/%d", GetName(), skillIndex, SKILL_MAX_NUM);
			return false;
		}

		const uint32_t eachSkillVNum = skillVNums[skillIndex];
		if (eachSkillVNum != 0)
		{
			const uint32_t eachJobGroup = s_anSkill2JobGroup[eachSkillVNum];

			if (0 == eachJobGroup || eachJobGroup == selfJobGroup)
			{
				// GUILDSKILL_BUG_FIX
				uint32_t eachSkillLevel = 0;

				if (eachSkillVNum >= GUILD_SKILL_START && eachSkillVNum <= GUILD_SKILL_END)
				{
					if (GetGuild())
						eachSkillLevel = GetGuild()->GetSkillLevel(eachSkillVNum);
					else
						eachSkillLevel = 0;
				}
				else
				{
					eachSkillLevel = GetSkillLevel(eachSkillVNum);
				}

				if (eachSkillLevel > 0)
				{
					return true;
				}
				// END_OF_GUILDSKILL_BUG_FIX
			}
		}
	}

	return false;
}

void CHARACTER::ClearSkill()
{
	PointChange(POINT_SKILL, 4 + (GetLevel() - 5) - GetPoint(POINT_SKILL));

	ResetSkill();
}

void CHARACTER::ClearSubSkill()
{
	PointChange(POINT_SUB_SKILL, GetLevel() < 10 ? 0 : (GetLevel() - 9) - GetPoint(POINT_SUB_SKILL));

	if (m_pSkillLevels == nullptr)
	{
		sys_err("m_pSkillLevels nil (name: %s)", GetName());
		return;
	}

	TPlayerSkill CleanSkill;
	memset(&CleanSkill, 0, sizeof(TPlayerSkill));

	const size_t count = sizeof(s_adwSubSkillVnums) / sizeof(s_adwSubSkillVnums[0]);

	for (size_t i = 0; i < count; ++i)
	{
		if (s_adwSubSkillVnums[i] >= SKILL_MAX_NUM)
			continue;

		m_pSkillLevels[s_adwSubSkillVnums[i]] = CleanSkill;
	}

	ComputePoints();
	SkillLevelPacket();
}

bool CHARACTER::ResetOneSkill(uint32_t dwVnum)
{
	if (nullptr == m_pSkillLevels)
	{
		sys_err("m_pSkillLevels nil (name %s, vnum %u)", GetName(), dwVnum);
		return false;
	}

	if (dwVnum >= SKILL_MAX_NUM)
	{
		sys_err("vnum overflow (name %s, vnum %u)", GetName(), dwVnum);
		return false;
	}

	uint8_t level = m_pSkillLevels[dwVnum].bLevel;

	m_pSkillLevels[dwVnum].bLevel = 0;
	m_pSkillLevels[dwVnum].bMasterType = 0;
	m_pSkillLevels[dwVnum].tNextRead = 0;

	if (level > 17)
		level = 17;

	PointChange(POINT_SKILL, level);

	LogManager::Instance().CharLog(this, dwVnum, "ONE_SKILL_RESET_BY_SCROLL", "");

	ComputePoints();
	SkillLevelPacket();

	return true;
}

eMountType GetMountLevelByVnum(uint32_t dwMountVnum, bool IsNew) // updated to 2014/12/10
{
	if (!dwMountVnum)
		return MOUNT_TYPE_NONE;

	switch (dwMountVnum)
	{
		// ### YES SKILL
		// @fixme116 begin
		case 20107: // normal military horse (no guild)
		case 20108: // normal military horse (guild member)
		case 20109: // normal military horse (guild master)
			if (IsNew)
				return MOUNT_TYPE_NONE;
		// @fixme116 end
		// Classic
		case 20110: // Classic Boar
		case 20111: // Classic Wolf
		case 20112: // Classic Tiger
		case 20113: // Classic Lion
		case 20114: // White Lion
		// Special Lv2
		case 20115: // Wild Battle Boar
		case 20116: // Fight Wolf
		case 20117: // Storm Tiger
		case 20118: // Battle Lion (bugged)
		case 20205: // Wild Battle Boar (alternative)
		case 20206: // Fight Wolf (alternative)
		case 20207: // Storm Tiger (alternative)
		case 20208: // Battle Lion (bugged) (alternative)
		// Royal Tigers
		case 20120: // blue
		case 20121: // dark red
		case 20122: // gold
		case 20123: // green
		case 20124: // pied
		case 20125: // white
		// Royal mounts (Special Lv3)
		case 20209: // Royal Boar
		case 20210: // Royal Wolf
		case 20211: // Royal Tiger
		case 20212: // Royal Lion
		//
		case 20215: // Rudolph m Lv3 (yes skill, yes atk)
		case 20218: // Rudolph f Lv3 (yes skill, yes atk)
		case 20225: // Dyno Lv3 (yes skill, yes atk)
		case 20230: // Turkey Lv3 (yes skill, yes atk)
			return MOUNT_TYPE_MILITARY;
			break;
		// ### NO SKILL YES ATK
		// @fixme116 begin
		case 20104: // normal combat horse (no guild)
		case 20105: // normal combat horse (guild member)
		case 20106: // normal combat horse (guild master)
			if (IsNew)
				return MOUNT_TYPE_NONE;
		// @fixme116 end
		case 20119: // Black Horse (no skill, yes atk)
		case 20214: // Rudolph m Lv2 (no skill, yes atk)
		case 20217: // Rudolph f Lv2 (no skill, yes atk)
		case 20219: // Equus Porphyreus (no skill, yes atk)
		case 20220: // Comet (no skill, yes atk)
		case 20221: // Polar Predator (no skill, yes atk)
		case 20222: // Armoured Panda (no skill, yes atk)
		case 20224: // Dyno Lv2 (no skill, yes atk)
		case 20226: // Nightmare (no skill, yes atk)
		case 20227: // Unicorn (no skill, yes atk)
		case 20229: // Turkey Lv2 (no skill, yes atk)
		case 20231: // Leopard (no skill, yes atk)
		case 20232: // Black Panther (no skill, yes atk)
			return MOUNT_TYPE_COMBAT;
			break;
		// ### NO SKILL NO ATK
		// @fixme116 begin
		case 20101: // normal beginner horse (no guild)
		case 20102: // normal beginner horse (guild member)
		case 20103: // normal beginner horse (guild master)
			if (IsNew)
				return MOUNT_TYPE_NONE;
		// @fixme116 end
		case 20213: // Rudolph m Lv1 (no skill, no atk)
		case 20216: // Rudolph f Lv1 (no skill, no atk)
		// Special Lv1
		case 20201: // Boar Lv1 (no skill, no atk)
		case 20202: // Wolf Lv1 (no skill, no atk)
		case 20203: // Tiger Lv1 (no skill, no atk)
		case 20204: // Lion Lv1 (no skill, no atk)
		//
		case 20223: // Dyno Lv1 (no skill, no atk)
		case 20228: // Turkey Lv1 (no skill, no atk)
			return MOUNT_TYPE_NORMAL;
			break;
		default:
			return MOUNT_TYPE_NONE;
			break;
	}
}

#ifdef ENABLE_NINETH_SKILL
constexpr int SKILL_COUNT = 7;
static const uint32_t SkillList[JOB_MAX_NUM][SKILL_GROUP_MAX_NUM][SKILL_COUNT] =
{
	{ {	1,	2,	3,	4,	5,	6,	176	},	{	16,	17,	18,	19,	20,	21,	176	} },	//Warrior
	{ {	31,	32,	33,	34,	35,	36,	177	},	{	46,	47,	48,	49,	50,	51,	178	} },	//Assassin
	{ {	61,	62,	63,	64,	65,	66,	179	},	{	76,	77,	78,	79,	80,	81,	180	} },	//Sura
	{ {	91,	92,	93,	94,	95,	96,	181	},	{	106,107,108,109,110,111,182	} },	//Shaman
#	ifdef ENABLE_WOLFMAN_CHARACTER
	{ {	170,171,172,173,174,175,183	},	{	0,	0,	0,	0,	0,	0	} },		//Wolfman
#	endif
};
#else
const int SKILL_COUNT = 6;
static const uint32_t SkillList[JOB_MAX_NUM][SKILL_GROUP_MAX_NUM][SKILL_COUNT] =
{
	{ {	1,	2,	3,	4,	5,	6	}, {	16,	17,	18,	19,	20,	21	} },
	{ {	31,	32,	33,	34,	35,	36	}, {	46,	47,	48,	49,	50,	51	} },
	{ {	61,	62,	63,	64,	65,	66	}, {	76,	77,	78,	79,	80,	81	} },
	{ {	91,	92,	93,	94,	95,	96	}, {	106,107,108,109,110,111	} },
#	ifdef ENABLE_WOLFMAN_CHARACTER
	{ {	170,171,172,173,174,175	}, {	0,	0,	0,	0,	0,	0	} },
#	endif
};
#endif

const uint32_t GetRandomSkillVnum(uint8_t bJob)
{
	// the chosen skill
	uint32_t dwSkillVnum = 0;
	do
	{
		// tmp stuff
		const uint32_t tmpJob = (bJob != JOB_MAX_NUM) ? MINMAX(0, bJob, JOB_MAX_NUM - 1) : number(0, JOB_MAX_NUM - 1);
		const uint32_t tmpSkillGroup = number(0, SKILL_GROUP_MAX_NUM - 1);
		const uint32_t tmpSkillCount = number(0, SKILL_COUNT - 1);
		// set skill
		dwSkillVnum = SkillList[tmpJob][tmpSkillGroup][tmpSkillCount];

#if defined(ENABLE_WOLFMAN_CHARACTER) && !defined(USE_WOLFMAN_BOOKS)
		if (tmpJob == JOB_WOLFMAN)
			continue;
#endif

#ifdef ENABLE_NINETH_SKILL
#	ifdef ENABLE_WOLFMAN_CHARACTER
		if (dwSkillVnum >= SKILL_FINISH && dwSkillVnum <= SKILL_ILIPUNGU)
#	else
		if (dwSkillVnum >= SKILL_FINISH && dwSkillVnum <= SKILL_CHEONUN)
#	endif
			continue;
#endif

		if (dwSkillVnum != 0 && nullptr != CSkillManager::Instance().Get(dwSkillVnum))
			break;
	} while (true);
	return dwSkillVnum;
}

#ifdef ENABLE_NINETH_SKILL
const uint32_t GetRandomForgetSkillVnum(uint8_t bJob)
{
	// the chosen skill
	uint32_t dwSkillVnum = 0;
	do
	{
		// tmp stuff
		const uint32_t tmpJob = (bJob != JOB_MAX_NUM) ? MINMAX(0, bJob, JOB_MAX_NUM - 1) : number(0, JOB_MAX_NUM - 1);
		const uint32_t tmpSkillGroup = number(0, SKILL_GROUP_MAX_NUM - 1);
		const uint32_t tmpSkillCount = number(0, SKILL_COUNT - 1);
		// set skill
		dwSkillVnum = SkillList[tmpJob][tmpSkillGroup][tmpSkillCount];

		if (dwSkillVnum != 0 && nullptr != CSkillManager::Instance().Get(dwSkillVnum))
			break;
	} while (true);
	return dwSkillVnum;
}
#endif

bool CHARACTER::CanUseSkill(uint32_t dwSkillVnum) const
{
	if (0 == dwSkillVnum)
		return false;

#ifdef ENABLE_SUNG_MAHI_TOWER
	LPDUNGEON dungeonInstance = GetDungeon();
	if (dungeonInstance)
		if (dungeonInstance->GetFlag("sungMahiCurseType") == 3)
			return false;
#endif

	if (0 < GetSkillGroup())
	{
		const uint32_t* pSkill = SkillList[GetJob()][GetSkillGroup() - 1];

		for (int i = 0; i < SKILL_COUNT; ++i)
		{
			if (pSkill[i] == dwSkillVnum) return true;
		}
	}

	//if (true == IsHorseRiding())

	if (IsRiding())
	{
		// Only advanced horses between mounts can use abilities
#ifdef ENABLE_MOUNTSKILL_CHECK
		eMountType eIsMount = GetMountLevelByVnum(GetMountVnum(), false);
		if (eIsMount != MOUNT_TYPE_MILITARY)
		{
			if (test_server)
				sys_log(0, "CanUseSkill: Mount can't skill. vnum(%u) type(%d)", GetMountVnum(), static_cast<int>(eIsMount));
			return false;
		}
#endif
		switch (dwSkillVnum)
		{
			case SKILL_HORSE_WILDATTACK:
			case SKILL_HORSE_CHARGE:
			case SKILL_HORSE_ESCAPE:
			case SKILL_HORSE_WILDATTACK_RANGE:
				return true;

			default:
				break;
		}
	}

	switch (dwSkillVnum)
	{
		case SKILL_LEADERSHIP:
		case SKILL_COMBO:
		case SKILL_CREATE:	//@fixme506
		case SKILL_MINING:
#ifdef ENABLE_REFINE_ABILITY_SKILL
		case SKILL_REFINE:	//@fixme506
#endif
		case SKILL_LANGUAGE1:
		case SKILL_LANGUAGE2:
		case SKILL_LANGUAGE3:
		case SKILL_POLYMORPH:
		case SKILL_HORSE:
		case SKILL_HORSE_SUMMON:
#ifdef ENABLE_PASSIVE_ATTR
#	ifdef ENABLE_CHARISMA
		case SKILL_CHARISMA:
#	endif
#	ifdef ENABLE_INSPIRATION
		case SKILL_INSPIRATION:
#	endif
#endif
		//@fixme506
		case SKILL_HORSE_WILDATTACK:
		case SKILL_HORSE_CHARGE:
		case SKILL_HORSE_ESCAPE:
		case SKILL_HORSE_WILDATTACK_RANGE:
		case SKILL_ADD_HP:
		case SKILL_RESIST_PENETRATE:
		//@end_fixme506
		case GUILD_SKILL_EYE:
		case GUILD_SKILL_BLOOD:
		case GUILD_SKILL_BLESS:
		case GUILD_SKILL_SEONGHWI:
		case GUILD_SKILL_ACCEL:
		case GUILD_SKILL_BUNNO:
		case GUILD_SKILL_JUMUN:
		case GUILD_SKILL_TELEPORT:
		case GUILD_SKILL_DOOR:
			return true;

		default:
			break;
	}

	return false;
}

bool CHARACTER::CheckSkillHitCount(const uint8_t SkillID, const VID TargetVID)
{
	const std::map<int, TSkillUseInfo>::iterator iter = m_SkillUseInfo.find(SkillID);

	if (iter == m_SkillUseInfo.end())
	{
		sys_log(0, "SkillHack: Skill(%u) is not in container", SkillID);
		return false;
	}

	TSkillUseInfo& rSkillUseInfo = iter->second;

	if (false == rSkillUseInfo.bUsed)
	{
		sys_log(0, "SkillHack: not used skill(%u)", SkillID);
		return false;
	}

	switch (SkillID)
	{
		case SKILL_YONGKWON:
		case SKILL_HWAYEOMPOK:
		case SKILL_DAEJINGAK:
		case SKILL_PAERYONG:
			sys_log(0, "SkillHack: cannot use attack packet for skill(%u)", SkillID);
			return false;

		default:
			break;
	}

	const target_map::iterator iterTargetMap = rSkillUseInfo.TargetVIDMap.find(TargetVID);

	if (rSkillUseInfo.TargetVIDMap.end() != iterTargetMap)
	{
		size_t MaxAttackCountPerTarget = 1;

		switch (SkillID)
		{

			case SKILL_SAMYEON:
			case SKILL_CHARYUN:
#ifdef ENABLE_WOLFMAN_CHARACTER
			case SKILL_CHAYEOL:
#endif
				MaxAttackCountPerTarget = 3;
				break;

			case SKILL_HORSE_WILDATTACK_RANGE:
				MaxAttackCountPerTarget = 5;
				break;

			case SKILL_YEONSA:
				MaxAttackCountPerTarget = 7;
				break;

			case SKILL_HORSE_ESCAPE:
				MaxAttackCountPerTarget = 10;
				break;

			default:
				break;
		}

		if (iterTargetMap->second >= MaxAttackCountPerTarget)
		{
			sys_log(0, "SkillHack: Too Many Hit count from SkillID(%u) count(%u)", SkillID, iterTargetMap->second);
			return false;
		}

		iterTargetMap->second++;
	}
	else
	{
		rSkillUseInfo.TargetVIDMap.insert(std::make_pair(TargetVID, 1));
	}

	return true;
}

#ifdef ENABLE_678TH_SKILL
bool CHARACTER::Is678Skill(uint32_t dwSkillVnum)
{
	if (dwSkillVnum >= SKILL_ANTI_PALBANG &&
#	ifdef ENABLE_WOLFMAN_CHARACTER
		dwSkillVnum <= SKILL_HELP_SALPOONG
#	else
		dwSkillVnum <= SKILL_HELP_BYEURAK
#	endif
		)
		return true;
	else
		return false;
}

bool CHARACTER::Learn678SkillByBook(uint32_t dwSkillVnum)
{
	if (!SkillCanUp(dwSkillVnum))
		return false;

	const CSkillProto* pkSk = CSkillManager::Instance().Get(dwSkillVnum);

	if (!pkSk)
		return false;

	if (!IsLearnableSkill(dwSkillVnum) || GetSkillLevel(dwSkillVnum) >= 30)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;948]");
		return false;
	}

	if (FindAffect(AFFECT_SKILL_NO_BOOK_DELAY))
	{
		// Ignore the time limit while using an eye spell
		RemoveAffect(AFFECT_SKILL_NO_BOOK_DELAY);
		ChatPacket(CHAT_TYPE_INFO, "[LS;377]");
	}
	else
	{
		if (GetQuestFlag("traning_678master_skill_time.read_time") > get_global_time())
		{
			SkillLearnWaitMoreTimeMessage(GetQuestFlag("traning_678master_skill_time.read_time"));
			return false;
		}
	}

	uint32_t need_exp = 0;

	if (FN_should_check_exp(this))
	{
		need_exp = 20000;

		if (GetExp() < need_exp)
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;957]");
			return false;
		}
	}

	const int bLastLevel = GetSkillLevel(dwSkillVnum);
	const int idx = MIN(9, GetSkillLevel(dwSkillVnum) - 20);
	sys_log(0, "Learn678SkillByBook %s table idx %d value %d", GetName(), idx, aiSkillBookCountForLevelUp[idx]);

	{
		const int need_bookcount = GetSkillLevel(dwSkillVnum) - 20;

		PointChange(POINT_EXP, -need_exp);

		quest::CQuestManager& q = quest::CQuestManager::Instance();
		quest::PC* pPC = q.GetPC(GetPlayerID());

		if (pPC)
		{
			char flag[128 + 1];
			memset(flag, 0, sizeof(flag));
			snprintf(flag, sizeof(flag), "traning_678master_skill.%u.read_count", dwSkillVnum);

			const int read_count = pPC->GetFlag(flag);
			uint8_t percent = 65;

			if (FindAffect(AFFECT_SKILL_BOOK_BONUS))
			{
				percent = 0;
				RemoveAffect(AFFECT_SKILL_BOOK_BONUS);
			}

			if (number(1, 100) > percent)
			{
				// Success in reading
#	ifdef ENABLE_MASTER_SKILLBOOK_NO_STEPS
				if (true)
#	else
				if (read_count >= need_bookcount)
#	endif
				{
					const int dwSkillLevel = GetSkillLevel(dwSkillVnum);
					// NOTE: 16 points because skill 7/8 start with 1 point.
#	ifdef ENABLE_FORCE2MASTERSKILL
					if (dwSkillLevel == 16)
						SetSkillLevel(dwSkillVnum, 20);
					else
#	endif
					{
						SetSkillLevel(dwSkillVnum, dwSkillLevel + 1);
					}
					pPC->SetFlag(flag, 0);

					SetQuestFlag("traning_678master_skill_time.read_time", get_global_time() + 21600);
					ChatPacket(CHAT_TYPE_INFO, "[LS;964]");
					LogManager::Instance().CharLog(this, dwSkillVnum, "678THSKILL_SKILLUP", "");
					Save();
					ComputePoints();
					SkillLevelPacket();
					return true;
				}
				else
				{
					pPC->SetFlag(flag, read_count + 1);

					switch (number(1, 3))
					{
					case 1:
						ChatPacket(CHAT_TYPE_TALKING, "[LS;961]");
						break;

					case 2:
						ChatPacket(CHAT_TYPE_TALKING, "[LS;962]");
						break;

					case 3:
					default:
						ChatPacket(CHAT_TYPE_TALKING, "[LS;963]");
						break;
					}

					ChatPacket(CHAT_TYPE_INFO, "[LS;995;%d]", (need_bookcount - read_count));
					return true;
				}
			}
		}
	}

	if (bLastLevel != GetSkillLevel(dwSkillVnum))
	{
		ChatPacket(CHAT_TYPE_TALKING, "[LS;954]");
		ChatPacket(CHAT_TYPE_TALKING, "[LS;955]");
		ChatPacket(CHAT_TYPE_INFO, "[LS;964]");
		LogManager::Instance().CharLog(this, dwSkillVnum, "678THSKILL_READ_SUCCESS", "");
	}
	else
	{
		ChatPacket(CHAT_TYPE_TALKING, "[LS;952]");
		ChatPacket(CHAT_TYPE_INFO, "[LS;953]");
		LogManager::Instance().CharLog(this, dwSkillVnum, "678THSKILL_READ_FAIL", "");
	}

	SetQuestFlag("traning_678master_skill_time.read_time", get_global_time() + 21600);

	return true;
}
#endif

#ifdef ENABLE_NINETH_SKILL
bool CHARACTER::IsNineSkill(uint32_t dwSkillVnum)
{
	if (dwSkillVnum >= SKILL_FINISH &&
#	ifdef ENABLE_WOLFMAN_CHARACTER
		dwSkillVnum <= SKILL_ILIPUNGU
#	else
		dwSkillVnum <= SKILL_CHEONUN
#	endif
		)
		return true;
	else
		return false;
}

bool CHARACTER::LearnNineSkillByBook(uint32_t dwSkillVnum, uint8_t bSkillGroup)
{
	if (!NineSkillCanUp(dwSkillVnum))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1710]");
		return false;
	}

	const CSkillProto* pkSk = CSkillManager::Instance().Get(dwSkillVnum);

	if (!pkSk)
		return false;

	if (!IsLearnableSkill(dwSkillVnum) || GetSkillLevel(dwSkillVnum) >= 30)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;948]");
		return false;
	}

	uint32_t need_cexp = 0;

	if (FN_should_check_cexp(this))
	{
		need_cexp = SB_C_NEED_EXP;

		if (GetConquerorExp() < need_cexp)
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;957]");
			return false;
		}
	}

	if ((bSkillGroup != 0) && (GetSkillGroup() != bSkillGroup))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;960]");
		return false;
	}

	if (FindAffect(AFFECT_SKILL_NO_BOOK_DELAY))
	{
		// Ignore the time limit while using an eye spell
		RemoveAffect(AFFECT_SKILL_NO_BOOK_DELAY);
		ChatPacket(CHAT_TYPE_INFO, "[LS;377]");
	}
	else
	{
		if (GetQuestFlag("traning_nine_master_skill_time.read_time") > get_global_time())
		{
			SkillLearnWaitMoreTimeMessage(GetQuestFlag("traning_nine_master_skill_time.read_time"));
			return false;
		}
	}

	// Here we calculate the probability.
	const int bLastLevel = GetSkillLevel(dwSkillVnum);
	const int idx = MIN(9, GetSkillLevel(dwSkillVnum) - 20);
	sys_log(0, "LearnNineSkillByBook %s table idx %d value %d", GetName(), idx, aiSkillBookCountForLevelUp[idx]);

	const int need_bookcount = GetSkillLevel(dwSkillVnum) - 20;

	PointChange(POINT_CONQUEROR_EXP, -need_cexp);

	quest::CQuestManager& q = quest::CQuestManager::Instance();
	quest::PC* pPC = q.GetPC(GetPlayerID());

	if (pPC)
	{
		char flag[128 + 1];
		memset(flag, 0, sizeof(flag));
		snprintf(flag, sizeof(flag), "traning_nine_master_skill.%u.read_count", dwSkillVnum);

		const int read_count = pPC->GetFlag(flag);
		uint8_t percent = 65;

		if (FindAffect(AFFECT_SKILL_BOOK_BONUS))
		{
			percent = 0;
			RemoveAffect(AFFECT_SKILL_BOOK_BONUS);
		}

		if (number(1, 100) > percent)
		{
			// Success in reading
#	ifdef ENABLE_MASTER_SKILLBOOK_NO_STEPS
			if (true)
#	else
			if (read_count >= need_bookcount)
#	endif
			{
				const int dwSkillLevel = GetSkillLevel(dwSkillVnum);
#	ifdef ENABLE_FORCE2MASTERSKILL
				if (dwSkillLevel == 17)
					SetSkillLevel(dwSkillVnum, 20);
				else
#	endif
				{
					SetSkillLevel(dwSkillVnum, dwSkillLevel + 1);
				}
				pPC->SetFlag(flag, 0);

				SetQuestFlag("traning_nine_master_skill_time.read_time", get_global_time() + 21600);
				ChatPacket(CHAT_TYPE_INFO, "[LS;964]");
				LogManager::Instance().CharLog(this, dwSkillVnum, "NINETH_SKILL_SKILLUP", "");
				Save();
				ComputePoints();
				SkillLevelPacket();
				return true;
			}
			else
			{
				pPC->SetFlag(flag, read_count + 1);

				switch (number(1, 3))
				{
				case 1:
					ChatPacket(CHAT_TYPE_TALKING, "[LS;961]");
					break;

				case 2:
					ChatPacket(CHAT_TYPE_TALKING, "[LS;962]");
					break;

				case 3:
				default:
					ChatPacket(CHAT_TYPE_TALKING, "[LS;963]");
					break;
				}

				ChatPacket(CHAT_TYPE_INFO, "[LS;995;%d]", (need_bookcount - read_count));
				return true;
			}
		}
	}

	if (bLastLevel != GetSkillLevel(dwSkillVnum))
	{
		ChatPacket(CHAT_TYPE_TALKING, "[LS;954]");
		ChatPacket(CHAT_TYPE_TALKING, "[LS;955]");
		ChatPacket(CHAT_TYPE_INFO, "[LS;964]");
		LogManager::Instance().CharLog(this, dwSkillVnum, "NINETH_SKILL_READ_SUCCESS", "");
	}
	else
	{
		ChatPacket(CHAT_TYPE_TALKING, "[LS;952]");
		ChatPacket(CHAT_TYPE_INFO, "[LS;953]");
		LogManager::Instance().CharLog(this, dwSkillVnum, "NINETH_SKILL_READ_FAIL", "");
	}

	SetQuestFlag("traning_nine_master_skill_time.read_time", get_global_time() + 21600);

	return true;
}
#endif

#ifdef ENABLE_SKILL_COOLTIME_UPDATE
void CHARACTER::ResetSkillCoolTimes()
{
	if (!GetSkillGroup() || m_SkillUseInfo.empty())
		return;

	for (std::map<int, TSkillUseInfo>::iterator it = m_SkillUseInfo.begin(); it != m_SkillUseInfo.end(); ++it)
		it->second.dwNextSkillUsableTime = 0;
}
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
bool CHARACTER::IsLearnableSkillAchievement(uint32_t dwSkillVnum) const
{
	const CSkillProto* pkSkill = CSkillManager::instance().Get(dwSkillVnum);

	if (!pkSkill)
		return false;

	if (pkSkill->dwType == 0)
		return true;

	if (!CanUseSkill(dwSkillVnum))
		return false;

	if (pkSkill->dwType - 1 == GetJob())
		return true;

	return false;
}
#endif
