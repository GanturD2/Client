#include "stdafx.h"
#include "constants.h"
#include "utils.h"
#include "desc.h"
#include "char.h"
#include "char_manager.h"
#include "mob_manager.h"
#include "party.h"
#include "regen.h"
#include "p2p.h"
#include "dungeon.h"
#include "db.h"
#include "config.h"
#include "xmas_event.h"
#include "questmanager.h"
#include "questlua.h"
#include "locale_service.h"
#include "shutdown_manager.h"
#include "../../common/CommonDefines.h"

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
#	include "../../libgame/include/grid.h"
#	include "desc_client.h"
#	include "shop_manager.h"
#	include "shop.h"
#	include <ctime> // used by std::time
#endif

#ifdef ENABLE_EASTER_EVENT
#	include "minigame_manager.h"
#endif

#ifdef ENABLE_WORLD_BOSS
# include <chrono>
# include <ctime>
#endif

CHARACTER_MANAGER::CHARACTER_MANAGER() :
	m_iVIDCount(0),
	m_pkChrSelectedStone(nullptr),
	m_bUsePendingDestroy(false)
{
	RegisterRaceNum(xmas::MOB_XMAS_FIRWORK_SELLER_VNUM);
	RegisterRaceNum(xmas::MOB_SANTA_VNUM);
	RegisterRaceNum(xmas::MOB_XMAS_TREE_VNUM);

	m_iMobItemRate = 100;
	m_iMobDamageRate = 100;
	m_iMobGoldAmountRate = 100;
	m_iMobGoldDropRate = 100;
	m_iMobExpRate = 100;

	m_iMobItemRatePremium = 100;
	m_iMobGoldAmountRatePremium = 100;
	m_iMobGoldDropRatePremium = 100;
	m_iMobExpRatePremium = 100;

	m_iUserDamageRate = 100;
	m_iUserDamageRatePremium = 100;

#ifdef ENABLE_WORLD_BOSS
	m_dwWBVID = 0;
	pkWB = nullptr;
#endif
}

CHARACTER_MANAGER::~CHARACTER_MANAGER()
{
	Destroy();
}

void CHARACTER_MANAGER::Destroy()
{
	auto it = m_map_pkChrByVID.begin();
	while (it != m_map_pkChrByVID.end())
	{
		LPCHARACTER ch = it->second;
		M2_DESTROY_CHARACTER(ch); // m_map_pkChrByVID is changed here
		it = m_map_pkChrByVID.begin();
	}
}

void CHARACTER_MANAGER::GracefulShutdown()
{
	NAME_MAP::iterator it = m_map_pkPCChr.begin();

	while (it != m_map_pkPCChr.end())
		(it++)->second->Disconnect("GracefulShutdown");
}

uint32_t CHARACTER_MANAGER::AllocVID()
{
	++m_iVIDCount;
	return m_iVIDCount;
}

LPCHARACTER CHARACTER_MANAGER::CreateCharacter(const char* name, uint32_t dwPID)
{
	uint32_t dwVID = AllocVID();

	LPCHARACTER ch = M2_NEW CHARACTER;
	ch->Create(name, dwVID, dwPID ? true : false);

	m_map_pkChrByVID.insert(std::make_pair(dwVID, ch));

	if (dwPID)
	{
		char szName[CHARACTER_NAME_MAX_LEN + 1];
		str_lower(name, szName, sizeof(szName));

		m_map_pkPCChr.insert(NAME_MAP::value_type(szName, ch));
		m_map_pkChrByPID.insert(std::make_pair(dwPID, ch));
	}

	return (ch);
}

void CHARACTER_MANAGER::DestroyCharacter(LPCHARACTER ch)
{
	if (!ch)
		return;

	// <Factor> Check whether it has been already deleted or not.
	const auto it = m_map_pkChrByVID.find(ch->GetVID());
	if (it == m_map_pkChrByVID.end())
	{
		sys_err("[CHARACTER_MANAGER::DestroyCharacter] <Factor> %d not found", (long)(ch->GetVID()));
		return; // prevent duplicated destrunction
	}

	// Monsters belonging to the dungeon should also be deleted from the dungeon.
	if (ch->IsNPC() && !ch->IsPet() && ch->GetRider() == nullptr
#ifdef ENABLE_PROTO_RENEWAL
		&& !ch->IsHorse()
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
		&& !ch->IsGrowthPet()
#endif
		)
	//if (ch->IsMonster() || ch->IsStone() && ch->GetRider() == nullptr)	//@fixme539
	{
		if (ch->GetDungeon())
		{
			ch->GetDungeon()->DeadCharacter(ch);
		}
	}

	if (m_bUsePendingDestroy)
	{
		m_set_pkChrPendingDestroy.insert(ch);
		return;
	}

	m_map_pkChrByVID.erase(it);

	if (true == ch->IsPC())
	{
		char szName[CHARACTER_NAME_MAX_LEN + 1];

		str_lower(ch->GetName(), szName, sizeof(szName));

		const NAME_MAP::iterator it2 = m_map_pkPCChr.find(szName);

		if (m_map_pkPCChr.end() != it2)
			m_map_pkPCChr.erase(it2);
	}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	//Shops and possible fake character copies (non-PC) hold the player ID as GetPlayerID(), but we should't delete the actual player from here.
	if (ch->GetPlayerID() && ch->IsPC())
#else
	if (0 != ch->GetPlayerID())
#endif
	{
		const auto it3 = m_map_pkChrByPID.find(ch->GetPlayerID());

		if (m_map_pkChrByPID.end() != it3)
		{
			m_map_pkChrByPID.erase(it3);
		}
	}

	UnregisterRaceNumMap(ch);
	RemoveFromStateList(ch);
	M2_DELETE(ch);
}

LPCHARACTER CHARACTER_MANAGER::Find(uint32_t dwVID)
{
	const auto it = m_map_pkChrByVID.find(dwVID);

	if (m_map_pkChrByVID.end() == it)
		return nullptr;

	// <Factor> Added sanity check
	LPCHARACTER found = it->second;
	if (found != nullptr && dwVID != (uint32_t)found->GetVID())
	{
		sys_err("[CHARACTER_MANAGER::Find] <Factor> %u != %u", dwVID, (uint32_t)found->GetVID());
		return nullptr;
	}
	return found;
}

LPCHARACTER CHARACTER_MANAGER::Find(const VID& vid)
{
	LPCHARACTER tch = Find((uint32_t)vid);

	if (!tch || tch->GetVID() != vid)
		return nullptr;

	return tch;
}

LPCHARACTER CHARACTER_MANAGER::FindByPID(uint32_t dwPID)
{
	const auto it = m_map_pkChrByPID.find(dwPID);

	if (m_map_pkChrByPID.end() == it)
		return nullptr;

	// <Factor> Added sanity check
	LPCHARACTER found = it->second;
	if (found != nullptr && dwPID != found->GetPlayerID())
	{
		sys_err("[CHARACTER_MANAGER::FindByPID] <Factor> %u != %u", dwPID, found->GetPlayerID());
		return nullptr;
	}
	return found;
}

LPCHARACTER CHARACTER_MANAGER::FindPC(const char* name)
{
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	str_lower(name, szName, sizeof(szName));
	const NAME_MAP::iterator it = m_map_pkPCChr.find(szName);

	if (it == m_map_pkPCChr.end())
		return nullptr;

	// <Factor> Added sanity check
	LPCHARACTER found = it->second;
	if (found != nullptr && strncasecmp(szName, found->GetName(), CHARACTER_NAME_MAX_LEN) != 0)
	{
		sys_err("[CHARACTER_MANAGER::FindPC] <Factor> %s != %s", name, found->GetName());
		return nullptr;
	}
	return found;
}

LPCHARACTER CHARACTER_MANAGER::SpawnMobRandomPosition(uint32_t dwVnum, long lMapIndex)
{
	// Allows you to decide why or not to spawn
	{
		if (dwVnum == 5001 && !quest::CQuestManager::Instance().GetEventFlag("japan_regen"))
		{
			sys_log(1, "WAEGU[5001] regen disabled.");
			return nullptr;
		}
	}

	// Allows you to decide whether or not to spawn a hatchet
	{
		if (dwVnum == 5002 && !quest::CQuestManager::Instance().GetEventFlag("newyear_mob"))
		{
			sys_log(1, "HAETAE (new-year-mob) [5002] regen disabled.");
			return nullptr;
		}
	}

	// Liberation Day event
	{
		if (dwVnum == 5004 && !quest::CQuestManager::Instance().GetEventFlag("independence_day"))
		{
			sys_log(1, "INDEPENDECE DAY [5004] regen disabled.");
			return nullptr;
		}
	}

#ifdef ENABLE_MONSTER_BACK
	if (dwVnum >= EMonsterBackEvent::ATTENDANCE_VNUM_BERAN && dwVnum <= EMonsterBackEvent::ATTENDANCE_VNUM_RED_DRAGON && !quest::CQuestManager::Instance().GetEventFlag("enable_attendance_event"))
	{
		sys_log(1, "ATTENDANCE [6415 - 6419] regen disabled.");
		return nullptr;
	}

	if (dwVnum >= 8041 && dwVnum <= 8050 && !quest::CQuestManager::Instance().GetEventFlag("enable_easter_event"))
	{
		sys_log(1, "Easter Event [8041 - 8050] regen disabled.");
		return nullptr;
	}
#endif

	const CMob* pkMob = CMobManager::Instance().Get(dwVnum);

	if (!pkMob)
	{
		sys_err("no mob data for vnum %u", dwVnum);
		return nullptr;
	}

	if (!map_allow_find(lMapIndex))
	{
		sys_err("not allowed map %u", lMapIndex);
		return nullptr;
	}

	LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::Instance().GetMap(lMapIndex);
	if (pkSectreeMap == nullptr)
		return nullptr;

	int i;
	long x, y;
	for (i = 0; i < 2000; i++)
	{
		x = number(1, (pkSectreeMap->m_setting.iWidth / 100) - 1) * 100 + pkSectreeMap->m_setting.iBaseX;
		y = number(1, (pkSectreeMap->m_setting.iHeight / 100) - 1) * 100 + pkSectreeMap->m_setting.iBaseY;
		//LPSECTREE tree = SECTREE_MANAGER::Instance().Get(lMapIndex, x, y);
		LPSECTREE tree = pkSectreeMap->Find(x, y);

		if (!tree)
			continue;

		const uint32_t dwAttr = tree->GetAttribute(x, y);

		if (IS_SET(dwAttr, ATTR_BLOCK | ATTR_OBJECT))
			continue;

		if (IS_SET(dwAttr, ATTR_BANPK))
			continue;

		break;
	}

	if (i == 2000)
	{
		sys_err("cannot find valid location");
		return nullptr;
	}

	const LPSECTREE& sectree = SECTREE_MANAGER::Instance().Get(lMapIndex, x, y);

	if (!sectree)
	{
		sys_log(0, "SpawnMobRandomPosition: cannot create monster at non-exist sectree %d x %d (map %d)", x, y, lMapIndex);
		return nullptr;
	}

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().CreateCharacter(pkMob->m_table.szLocaleName);

	if (!ch)
	{
		sys_log(0, "SpawnMobRandomPosition: cannot create new character");
		return nullptr;
	}

	ch->SetProto(pkMob);

	// if mob is npc with no empire assigned, assign to empire of map
	if (pkMob->m_table.bType == CHAR_TYPE_NPC)
	{
		if (ch->GetEmpire() == 0)
			ch->SetEmpire(SECTREE_MANAGER::Instance().GetEmpireFromMapIndex(lMapIndex));
	}

	ch->SetRotation(static_cast<float>(number(0, 360)));

	if (!ch->Show(lMapIndex, x, y, 0, false))
	{
		M2_DESTROY_CHARACTER(ch);
		sys_err(0, "SpawnMobRandomPosition: cannot show monster");
		return nullptr;
	}

	char buf[512 + 1];
	const long local_x = x - pkSectreeMap->m_setting.iBaseX;
	const long local_y = y - pkSectreeMap->m_setting.iBaseY;
	snprintf(buf, sizeof(buf), "spawn %s[%d] random position at %ld %ld %ld %ld (time: %d)", ch->GetName(), dwVnum, x, y, local_x, local_y, get_global_time());

	sys_log(0, buf);
	return (ch);
}

LPCHARACTER CHARACTER_MANAGER::SpawnMob(uint32_t dwVnum, long lMapIndex, long x, long y, long z, bool bSpawnMotion, int iRot, bool bShow)
{
	const CMob* pkMob = CMobManager::Instance().Get(dwVnum);
	if (!pkMob)
	{
		sys_err("SpawnMob: no mob data for vnum %u", dwVnum);
		return nullptr;
	}

	if (!(pkMob->m_table.bType == CHAR_TYPE_NPC || pkMob->m_table.bType == CHAR_TYPE_WARP || pkMob->m_table.bType == CHAR_TYPE_GOTO
#ifdef ENABLE_PROTO_RENEWAL
		|| pkMob->m_table.bType == CHAR_TYPE_HORSE
		|| pkMob->m_table.bType == CHAR_TYPE_PET
		|| pkMob->m_table.bType == CHAR_TYPE_PET_PAY
#endif
		) || mining::IsVeinOfOre(dwVnum))
	{
		LPSECTREE tree = SECTREE_MANAGER::Instance().Get(lMapIndex, x, y);
		if (!tree)
		{
			sys_log(0, "no sectree for spawn at %d %d mobvnum %d mapindex %d", x, y, dwVnum, lMapIndex);
			return nullptr;
		}

		const uint32_t dwAttr = tree->GetAttribute(x, y);

		bool is_set = false;

		if (mining::IsVeinOfOre(dwVnum))
			is_set = IS_SET(dwAttr, ATTR_BLOCK);
		else
			is_set = IS_SET(dwAttr, ATTR_BLOCK | ATTR_OBJECT);

		if (is_set)
		{
			// SPAWN_BLOCK_LOG
			static bool s_isLog = quest::CQuestManager::Instance().GetEventFlag("spawn_block_log");
			static uint32_t s_nextTime = get_global_time() + 10000;

			const uint32_t curTime = get_global_time();

			if (curTime > s_nextTime)
			{
				s_nextTime = curTime;
				s_isLog = quest::CQuestManager::Instance().GetEventFlag("spawn_block_log");
			}

			if (s_isLog)
				sys_log(0, "SpawnMob: BLOCKED position for spawn %s %u at %d %d (attr %u)", pkMob->m_table.szName, dwVnum, x, y, dwAttr);
			// END_OF_SPAWN_BLOCK_LOG
			return nullptr;
		}

		if (IS_SET(dwAttr, ATTR_BANPK))
		{
			sys_log(0, "SpawnMob: BAN_PK position for mob spawn %s %u at %d %d", pkMob->m_table.szName, dwVnum, x, y);
			return nullptr;
		}
	}

	const LPSECTREE& sectree = SECTREE_MANAGER::Instance().Get(lMapIndex, x, y);
	if (!sectree)
	{
		sys_log(0, "SpawnMob: cannot create monster at non-exist sectree %d x %d (map %d)", x, y, lMapIndex);
		return nullptr;
	}

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().CreateCharacter(pkMob->m_table.szLocaleName);
	if (!ch)
	{
		sys_log(0, "SpawnMob: cannot create new character");
		return nullptr;
	}

	if (iRot == -1)
		iRot = number(0, 360);

	ch->SetProto(pkMob);

	// if mob is npc with no empire assigned, assign to empire of map
	if (pkMob->m_table.bType == CHAR_TYPE_NPC)
	{
		if (ch->GetEmpire() == 0)
			ch->SetEmpire(SECTREE_MANAGER::Instance().GetEmpireFromMapIndex(lMapIndex));
	}

	ch->SetRotation(static_cast<float>(iRot));

	if (bShow && !ch->Show(lMapIndex, x, y, z, bSpawnMotion))
	{
		M2_DESTROY_CHARACTER(ch);
		sys_log(0, "SpawnMob: cannot show monster");
		return nullptr;
	}

	return (ch);
}

LPCHARACTER CHARACTER_MANAGER::SpawnMobRange(uint32_t dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, bool bIsException, bool bSpawnMotion, bool bAggressive
#ifdef ENABLE_12ZI
	, uint8_t bLevel
#endif
)
{
	const auto* pkMob = CMobManager::Instance().Get(dwVnum);
	if (!pkMob)
		return nullptr;

	if (pkMob->m_table.bType == CHAR_TYPE_STONE) // Stones always have SPAWN motion.
		bSpawnMotion = true;

	int i = 16;

	while (i--)
	{
		const int x = number(sx, ex);
		const int y = number(sy, ey);

		const auto ch = SpawnMob(dwVnum, lMapIndex, x, y, 0, bSpawnMotion);
		if (ch)
		{
			sys_log(1, "MOB_SPAWN: %s(%d) %dx%d", ch->GetName(), (uint32_t)ch->GetVID(), ch->GetX(), ch->GetY());
			if (bAggressive)
				ch->SetAggressive();

#ifdef ENABLE_12ZI
			if (bLevel)
			{
				if (ch->IsStone())
				{
					const int calcHP = ch->GetMaxHP() + (bLevel * 6500);
					ch->SetMaxHP(calcHP);
					ch->SetHP(calcHP);
				}
				else if (ch->IsMonster() && !ch->IsZodiacBoss())
				{
					const int calcHP = ch->GetMaxHP() + (bLevel * 1250);
					ch->SetMaxHP(calcHP);
					ch->SetHP(calcHP);
				}
				else if (ch->IsMonster() && ch->IsZodiacBoss())
				{
					const int calcHP = ch->GetMaxHP() + (bLevel * 9000);
					ch->SetMaxHP(calcHP);
					ch->SetHP(calcHP);
				}

				ch->SetLevel(bLevel);
				ch->UpdatePacket();
			}
#endif

			return (ch);
		}
	}

	return nullptr;
}

void CHARACTER_MANAGER::SelectStone(LPCHARACTER pkChr)
{
	m_pkChrSelectedStone = pkChr;
}

bool CHARACTER_MANAGER::SpawnMoveGroup(uint32_t dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, int tx, int ty, LPREGEN pkRegen, bool bAggressive_)
{
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	if (!dwVnum)
		return false;
#endif

	CMobGroup* pkGroup = CMobManager::Instance().GetGroup(dwVnum);

	if (!pkGroup)
	{
		sys_err("NOT_EXIST_GROUP_VNUM(%u) Map(%u) ", dwVnum, lMapIndex);
		return false;
	}

	LPCHARACTER pkChrMaster = nullptr;
	LPPARTY pkParty = nullptr;

	const std::vector<uint32_t>& c_rdwMembers = pkGroup->GetMemberVector();

	bool bSpawnedByStone = false;
	bool bAggressive = bAggressive_;

	if (m_pkChrSelectedStone)
	{
		bSpawnedByStone = true;
		if (m_pkChrSelectedStone->GetDungeon())
			bAggressive = true;
	}

	for (uint32_t i = 0; i < c_rdwMembers.size(); ++i)
	{
		LPCHARACTER tch = SpawnMobRange(c_rdwMembers[i], lMapIndex, sx, sy, ex, ey, true, bSpawnedByStone);

		if (!tch)
		{
			if (i == 0) // If the monster you couldn't make is the captain, it just fails.
				return false;

			continue;
		}

		sx = tch->GetX() - number(300, 500);
		sy = tch->GetY() - number(300, 500);
		ex = tch->GetX() + number(300, 500);
		ey = tch->GetY() + number(300, 500);

		if (m_pkChrSelectedStone)
			tch->SetStone(m_pkChrSelectedStone);
		else if (pkParty)
		{
			pkParty->Join(tch->GetVID());
			pkParty->Link(tch);
		}
		else if (!pkChrMaster)
		{
			pkChrMaster = tch;
			pkChrMaster->SetRegen(pkRegen);

			pkParty = CPartyManager::Instance().CreateParty(pkChrMaster);
		}
		if (bAggressive)
			tch->SetAggressive();

		if (tch->Goto(tx, ty))
			tch->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);
	}

	return true;
}

bool CHARACTER_MANAGER::SpawnGroupGroup(uint32_t dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, LPREGEN pkRegen, bool bAggressive_, LPDUNGEON pDungeon)
{
	const uint32_t dwGroupID = CMobManager::Instance().GetGroupFromGroupGroup(dwVnum);

	if (dwGroupID != 0)
	{
		return SpawnGroup(dwGroupID, lMapIndex, sx, sy, ex, ey, pkRegen, bAggressive_, pDungeon);
	}
	else
	{
		sys_err("NOT_EXIST_GROUP_GROUP_VNUM(%u) MAP(%ld)", dwVnum, lMapIndex);
		return false;
	}
}

LPCHARACTER CHARACTER_MANAGER::SpawnGroup(uint32_t dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, LPREGEN pkRegen, bool bAggressive_, LPDUNGEON pDungeon)
{
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	if (!dwVnum)
		return nullptr;
#endif

	CMobGroup* pkGroup = CMobManager::Instance().GetGroup(dwVnum);

	if (!pkGroup)
	{
		sys_err("NOT_EXIST_GROUP_VNUM(%u) Map(%u) ", dwVnum, lMapIndex);
		return nullptr;
	}

	LPCHARACTER pkChrMaster = nullptr;
	LPPARTY pkParty = nullptr;

	const std::vector<uint32_t>& c_rdwMembers = pkGroup->GetMemberVector();

	bool bSpawnedByStone = false;
	bool bAggressive = bAggressive_;

	if (m_pkChrSelectedStone)
	{
		bSpawnedByStone = true;

		if (m_pkChrSelectedStone->GetDungeon())
			bAggressive = true;
	}

	LPCHARACTER chLeader = nullptr;

	for (uint32_t i = 0; i < c_rdwMembers.size(); ++i)
	{
		LPCHARACTER tch = SpawnMobRange(c_rdwMembers[i], lMapIndex, sx, sy, ex, ey, true, bSpawnedByStone);

		if (!tch)
		{
			if (i == 0) // If the monster you couldn't make is the captain, it just fails.
				return nullptr;

			continue;
		}

		if (i == 0)
			chLeader = tch;

		if (pDungeon)
			tch->SetDungeon(pDungeon);

		sx = tch->GetX() - number(300, 500);
		sy = tch->GetY() - number(300, 500);
		ex = tch->GetX() + number(300, 500);
		ey = tch->GetY() + number(300, 500);

		if (m_pkChrSelectedStone)
			tch->SetStone(m_pkChrSelectedStone);
		else if (pkParty)
		{
			pkParty->Join(tch->GetVID());
			pkParty->Link(tch);
		}
		else if (!pkChrMaster)
		{
			pkChrMaster = tch;
			pkChrMaster->SetRegen(pkRegen);

			pkParty = CPartyManager::Instance().CreateParty(pkChrMaster);
		}

		if (bAggressive)
			tch->SetAggressive();

#ifdef ENABLE_SUNG_MAHI_TOWER
		if (tch && pDungeon)
		{
			uint8_t bDungeonLevel = pDungeon->GetDungeonDifficulty();
			tch->SetDungeonMultipliers(bDungeonLevel);
		}
#endif
	}

	return chLeader;
}

#ifdef ENABLE_12ZI
bool CHARACTER_MANAGER::SpawnGroupGroupZodiac(uint32_t dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, LPREGEN pkRegen, bool bAggressive_, LPZODIAC pZodiac, uint8_t bLevel)
{
	const uint32_t dwGroupID = CMobManager::Instance().GetGroupFromGroupGroup(dwVnum);

	if (dwGroupID != 0)
	{
		return SpawnGroupZodiac(dwGroupID, lMapIndex, sx, sy, ex, ey, pkRegen, bAggressive_, pZodiac, bLevel);
	}
	else
	{
		sys_err("NOT_EXIST_GROUP_GROUP_VNUM(%u) MAP(%ld)", dwVnum, lMapIndex);
		return false;
	}
}

LPCHARACTER CHARACTER_MANAGER::SpawnGroupZodiac(uint32_t dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, LPREGEN pkRegen, bool bAggressive_, LPZODIAC pZodiac, uint8_t bLevel)
{
	CMobGroup* pkGroup = CMobManager::Instance().GetGroup(dwVnum);

	if (!pkGroup)
	{
		sys_err("NOT_EXIST_GROUP_VNUM(%u) Map(%u) ", dwVnum, lMapIndex);
		return nullptr;
	}

	LPCHARACTER pkChrMaster = nullptr;
	LPPARTY pkParty = nullptr;

	const std::vector<uint32_t>& c_rdwMembers = pkGroup->GetMemberVector();

	const bool bSpawnedByStone = m_pkChrSelectedStone ? true : false;
	const bool bAggressive = pZodiac ? true : bAggressive_;

	LPCHARACTER chLeader = nullptr;
	for (size_t i = 0; i < c_rdwMembers.size(); ++i)
	{
		LPCHARACTER tch = SpawnMobRange(c_rdwMembers[i], lMapIndex, sx, sy, ex, ey, true, bSpawnedByStone, bAggressive, bLevel);

		if (!tch)
		{
			if (i == 0) // If the monster you couldn't make is the captain, it just fails.
				return nullptr;

			continue;
		}

		if (i == 0)
			chLeader = tch;

		tch->SetZodiac(pZodiac);
		tch->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);

		sx = tch->GetX() - number(300, 500);
		sy = tch->GetY() - number(300, 500);
		ex = tch->GetX() + number(300, 500);
		ey = tch->GetY() + number(300, 500);

		if (m_pkChrSelectedStone)
		{
			tch->SetStone(m_pkChrSelectedStone);
		}
		else if (pkParty)
		{
			pkParty->Join(tch->GetVID());
			pkParty->Link(tch);
		}
		else if (!pkChrMaster)
		{
			pkChrMaster = tch;
			pkChrMaster->SetRegen(pkRegen);

			pkParty = CPartyManager::Instance().CreateParty(pkChrMaster);
		}
	}

	return chLeader;
}
#endif

#ifdef ENABLE_NEWSTUFF //LEVEL_RANGE_EXTEND
LPCHARACTER CHARACTER_MANAGER::SpawnGroupLevelRange(uint32_t dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, uint8_t min_level, uint8_t max_level,
	bool bIncreaseHP, bool bIncreaseDamage, LPREGEN pkRegen, bool bAggressive_, LPDUNGEON pDungeon)
{
	if (!dwVnum)
		return nullptr;

	CMobGroup* pkGroup = CMobManager::Instance().GetGroup(dwVnum);

	if (!pkGroup)
	{
		sys_err("NOT_EXIST_GROUP_VNUM(%u) Map(%u) ", dwVnum, lMapIndex);
		return nullptr;
	}

	LPCHARACTER pkChrMaster = nullptr;
	LPPARTY pkParty = nullptr;

	const std::vector<uint32_t>& c_rdwMembers = pkGroup->GetMemberVector();

	bool bSpawnedByStone = false;
	bool bAggressive = bAggressive_;

	if (m_pkChrSelectedStone)
	{
		bSpawnedByStone = true;

		if (m_pkChrSelectedStone->GetDungeon())
			bAggressive = true;
	}

	LPCHARACTER chLeader = nullptr;

	for (uint32_t i = 0; i < c_rdwMembers.size(); ++i)
	{
		LPCHARACTER tch = SpawnMobRange(c_rdwMembers[i], lMapIndex, sx, sy, ex, ey, true, bSpawnedByStone);

		if (!tch)
		{
			if (i == 0) // If the monster you couldn't make is the captain, it just fails.
				return nullptr;

			continue;
		}

		if (i == 0)
			chLeader = tch;

		if (pDungeon)
			tch->SetDungeon(pDungeon);

		sx = tch->GetX() - number(300, 500);
		sy = tch->GetY() - number(300, 500);
		ex = tch->GetX() + number(300, 500);
		ey = tch->GetY() + number(300, 500);

		if (m_pkChrSelectedStone)
			tch->SetStone(m_pkChrSelectedStone);
		else if (pkParty)
		{
			pkParty->Join(tch->GetVID());
			pkParty->Link(tch);
		}
		else if (!pkChrMaster)
		{
			pkChrMaster = tch;
			pkChrMaster->SetRegen(pkRegen);

			pkParty = CPartyManager::Instance().CreateParty(pkChrMaster);
		}

		if (bAggressive)
			tch->SetAggressive();

		const uint8_t bLevel = number(min_level, max_level);
		tch->SetLevel(bLevel);

		if (bIncreaseHP)
		{
			tch->SetMaxHP(tch->GetMaxHP() * bLevel);
			tch->SetHP(tch->GetMaxHP());
		}

		if (bIncreaseDamage)
		{
			const float fAttMul = static_cast<float>(bLevel / 10);
			const float fDamMul = 1.0f + ((gPlayerMaxLevel - bLevel) / fAttMul / 2);
			tch->SetAttMul(fAttMul);
			tch->SetDamMul(fDamMul);
		}

		tch->UpdatePacket();
	}

	return chLeader;
}
#endif

struct FuncUpdateAndResetChatCounter
{
	void operator () (LPCHARACTER ch)
	{
		if (ch)
		{
			ch->ResetChatCounter();
			ch->ResetWhisperCounter();	//@custom013
			ch->CFSM::Update();
		}
	}
};

#ifdef ENABLE_WORLD_BOSS
void CHARACTER_MANAGER::OnKill(uint32_t vid)
{
	if (m_dwWBVID == vid)
	{
		m_dwWBVID = 0;
		pkWB = nullptr;

		m_lWBPhase = 0;
		m_bWBState = WORLD_BOSS_STATE_BOSS_KILLED;
		m_lWBCooldown = get_global_time() + (COOLDOWN_PHASE * 60 * 60);

		TPacketGGSendWorldBossStates pack;
		pack.bHeader = HEADER_GG_WORLD_BOSS;
		pack.WBState = m_bWBState;
		pack.WBTimer = m_lWBPhase;
		pack.WBCooldown = m_lWBCooldown;
		P2P_MANAGER::Instance().Send(&pack, sizeof(pack));

		return;
	}
}

bool wb_Spawned = false;
uint8_t wblast_SpawnTime = 1;
#endif

#include "xmas_event.h"
void CHARACTER_MANAGER::Update(int iPulse)
{
	BeginPendingDestroy();

	auto resetChatCounter = !(iPulse % PASSES_PER_SEC(5));

	// Update PC character
	std::for_each(m_map_pkPCChr.begin(), m_map_pkPCChr.end(),
		[&resetChatCounter, &iPulse](const NAME_MAP::value_type& v)
		{
			auto ch = v.second;
			if (ch)
			{
				if (resetChatCounter)
				{
					ch->ResetChatCounter();
					ch->CFSM::Update();
				}

				ch->UpdateCharacter(iPulse);
			}
		}
	);

	// Update Monster
	std::for_each(m_set_pkChrState.begin(), m_set_pkChrState.end(),
		[iPulse](LPCHARACTER ch)
		{
			ch->UpdateStateMachine(iPulse);
		}
	);

	// Update to Santa
	{
		auto snapshot = CHARACTER_MANAGER::Instance().GetCharactersByRaceNum(xmas::MOB_SANTA_VNUM);
		if (!snapshot.empty())
		{
			std::for_each(snapshot.begin(), snapshot.end(), [iPulse](LPCHARACTER ch)
				{
					ch->UpdateStateMachine(iPulse);
				}
			);
		}
	}

	// Record mob hunting counts once every hour
	if (0 == (iPulse % PASSES_PER_SEC(3600)))
	{
		for (const auto& it : m_map_dwMobKillCount)
			DBManager::Instance().SendMoneyLog(MONEY_LOG_MONSTER_KILL, it.first, it.second);

		m_map_dwMobKillCount.clear();
	}

#ifdef ENABLE_WORLD_BOSS
	if (quest::CQuestManager::instance().GetEventFlag("world_boss_event") == 1)
	{
		auto row = rand() % 4;
		auto WB_MAP_INDEX = WBMapIndexes[row];
		// auto WB_MAP_INDEX = 63;
		if (map_allow_find(WB_MAP_INDEX) && !(iPulse % passes_per_sec))
		{
			LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::instance().GetMap(WB_MAP_INDEX);
			if (!pkSectreeMap)
				return;

			auto curTime = std::chrono::system_clock::now();
			time_t tT = std::chrono::system_clock::to_time_t(curTime);
			auto utc_tm = *gmtime(&tT);
			auto cur_hour = utc_tm.tm_hour + 2;
			auto cur_min = utc_tm.tm_min;
			auto cur_sec = utc_tm.tm_sec;

			if (wblast_SpawnTime != 1)
			{
				if (wblast_SpawnTime != cur_sec)
					wb_Spawned = false;
			}

			if ((wb_Spawned) && (
				cur_hour == (WORLD_BOSS_PHASE)-COOLDOWN_PHASE
				|| cur_hour == (WORLD_BOSS_PHASE * 2) - COOLDOWN_PHASE
				|| cur_hour == (WORLD_BOSS_PHASE * 3) - COOLDOWN_PHASE
				|| cur_hour == (WORLD_BOSS_PHASE * 4) - COOLDOWN_PHASE
				))
			{
				if (cur_min == 0)
				{
					if (pkWB != nullptr)
					{
						M2_DESTROY_CHARACTER(pkWB);
					}
					wb_Spawned = false;
				}
			}

			if ((!wb_Spawned) && (
				cur_hour == WORLD_BOSS_PHASE
				|| cur_hour == WORLD_BOSS_PHASE * 2
				|| cur_hour == WORLD_BOSS_PHASE * 3
				|| cur_hour == WORLD_BOSS_PHASE * 4 // 24
				|| cur_hour == 0 // 24
				))
			{
				if (cur_min == 0)
				{
					if (m_dwWBVID == 0)
					{
						auto wbrow = rand() % 5;

						auto wbVnum = WBVnums[wbrow];

						pkWB = SpawnMob(wbVnum, WB_MAP_INDEX, pkSectreeMap->m_setting.iBaseX + 920 * 100, pkSectreeMap->m_setting.iBaseY + 595 * 100, 0);

						sys_err("vnum %d index %d", wbVnum, WB_MAP_INDEX);

						if (pkWB)
						{
							m_dwWBVID = pkWB->GetVID();

							sys_err("vnum %d index %d SPAWNED", wbVnum, WB_MAP_INDEX);

							char buf[512 + 1];
							auto pkMob = CMobManager::instance().Get(wbVnum);
							snprintf(buf, sizeof(buf), "The world boss %s spawns", pkMob->m_table.szLocaleName);
							SendNotice(buf, true);

							m_lWBPhase = get_global_time() + (BATTLE_PHASE * 60 * 60);
							m_bWBState = WORLD_BOSS_STATE_BOSS_SPAWNED;
							m_lWBCooldown = 0;

							TPacketGGSendWorldBossStates pack;
							pack.bHeader = HEADER_GG_WORLD_BOSS;
							pack.WBState = m_bWBState;
							pack.WBTimer = m_lWBPhase;
							pack.WBCooldown = m_lWBCooldown;
							P2P_MANAGER::Instance().Send(&pack, sizeof(pack));
						}
					}

					wb_Spawned = true;
					wblast_SpawnTime = cur_hour;
				}
			}
		}
	}
#endif

	// The test server counts the number of characters every 60 seconds
	if (test_server && 0 == (iPulse % PASSES_PER_SEC(60)))
		sys_log(0, "CHARACTER COUNT vid %zu pid %zu", m_map_pkChrByVID.size(), m_map_pkChrByPID.size());

	// Delayed DestroyCharacter
	FlushPendingDestroy();

	// ShutdownManager Update
	CShutdownManager::Instance().Update();
}

void CHARACTER_MANAGER::ProcessDelayedSave()
{
	CHARACTER_SET::iterator it = m_set_pkChrForDelayedSave.begin();

	while (it != m_set_pkChrForDelayedSave.end())
	{
		LPCHARACTER pkChr = *it++;
		if (pkChr)
			pkChr->SaveReal();
	}

	m_set_pkChrForDelayedSave.clear();
}

bool CHARACTER_MANAGER::AddToStateList(LPCHARACTER ch)
{
	assert(ch != nullptr);

	const CHARACTER_SET::iterator it = m_set_pkChrState.find(ch);

	if (it == m_set_pkChrState.end())
	{
		m_set_pkChrState.insert(ch);
		return true;
	}

	return false;
}

void CHARACTER_MANAGER::RemoveFromStateList(LPCHARACTER ch)
{
	const CHARACTER_SET::iterator it = m_set_pkChrState.find(ch);

	if (it != m_set_pkChrState.end())
	{
		//sys_log(0, "RemoveFromStateList %p", ch);
		m_set_pkChrState.erase(it);
	}
}

void CHARACTER_MANAGER::DelayedSave(LPCHARACTER ch)
{
	m_set_pkChrForDelayedSave.insert(ch);
}

bool CHARACTER_MANAGER::FlushDelayedSave(LPCHARACTER ch)
{
	if (!ch)
		return false;

	const CHARACTER_SET::iterator it = m_set_pkChrForDelayedSave.find(ch);

	if (it == m_set_pkChrForDelayedSave.end())
		return false;

	m_set_pkChrForDelayedSave.erase(it);
	ch->SaveReal();
	return true;
}

void CHARACTER_MANAGER::RegisterForMonsterLog(LPCHARACTER ch)
{
	m_set_pkChrMonsterLog.insert(ch);
}

void CHARACTER_MANAGER::UnregisterForMonsterLog(LPCHARACTER ch)
{
	m_set_pkChrMonsterLog.erase(ch);
}

void CHARACTER_MANAGER::PacketMonsterLog(LPCHARACTER ch, const void* buf, int size)
{
	for (auto it = m_set_pkChrMonsterLog.begin(); it != m_set_pkChrMonsterLog.end(); ++it)
	{
		const LPCHARACTER& c = *it;
		if (!c)
			continue;

		if (ch && DISTANCE_APPROX(c->GetX() - ch->GetX(), c->GetY() - ch->GetY()) > 6000)
			continue;

		LPDESC d = c->GetDesc();
		if (d)
			d->Packet(buf, size);
	}
}

void CHARACTER_MANAGER::KillLog(uint32_t dwVnum)
{
	constexpr uint32_t SEND_LIMIT = 10000;

	const auto it = m_map_dwMobKillCount.find(dwVnum);

	if (it == m_map_dwMobKillCount.end())
		m_map_dwMobKillCount.insert(std::make_pair(dwVnum, 1));
	else
	{
		++it->second;

		if (it->second > SEND_LIMIT)
		{
			DBManager::Instance().SendMoneyLog(MONEY_LOG_MONSTER_KILL, it->first, it->second);
			m_map_dwMobKillCount.erase(it);
		}
	}
}

void CHARACTER_MANAGER::RegisterRaceNum(uint32_t dwVnum)
{
	m_set_dwRegisteredRaceNum.insert(dwVnum);
}

void CHARACTER_MANAGER::RegisterRaceNumMap(LPCHARACTER ch)
{
	if (!ch)
		return;

	const uint32_t dwVnum = ch->GetRaceNum();

	if (m_set_dwRegisteredRaceNum.find(dwVnum) != m_set_dwRegisteredRaceNum.end()) // If it is a registered number
	{
		sys_log(0, "RegisterRaceNumMap %s %u", ch->GetName(), dwVnum);
		m_map_pkChrByRaceNum[dwVnum].insert(ch);
	}
}

void CHARACTER_MANAGER::UnregisterRaceNumMap(LPCHARACTER ch)
{
	if (!ch)
		return;

	const uint32_t dwVnum = ch->GetRaceNum();

	const auto it = m_map_pkChrByRaceNum.find(dwVnum);

	if (it != m_map_pkChrByRaceNum.end())
		it->second.erase(ch);
}

CharacterVectorInteractor CHARACTER_MANAGER::GetCharactersByRaceNum(uint32_t dwRaceNum)
{
	const auto it = m_map_pkChrByRaceNum.find(dwRaceNum);
	if (it == m_map_pkChrByRaceNum.end())
		return CharacterVectorInteractor();

	return CharacterVectorInteractor(it->second);
}

#if defined(ENABLE_PREMIUM_PRIVATE_SHOP) && defined(ENABLE_MYSHOP_DECO_PREMIUM)
void CHARACTER_MANAGER::GetCharactersShopRaceNum(std::vector<CharacterVectorInteractor>& i)
{
	const std::vector<uint32_t> shopDecoVnums = { 30000 , 30002, 30003, 30004, 30005, 30006, 30007, 30008 };
	for (const auto& dwShop : shopDecoVnums)
	{
		const auto it = m_map_pkChrByRaceNum.find(dwShop);
		if (it != m_map_pkChrByRaceNum.end())
			i.emplace_back(it->second);
	}
}
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
int CHARACTER_MANAGER::CountOfflineShops()
{
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
	int count = 0;

	const std::vector<uint32_t> shopDecoVnums = { 30000, 30002, 30003, 30004, 30005, 30006, 30007, 30008 };
	for (const auto& dwShop : shopDecoVnums)
	{
		const auto it = m_map_pkChrByRaceNum.find(dwShop);
		if (it != m_map_pkChrByRaceNum.end())
		{
			for (auto ch : it->second)
			{
				if (ch && ch->IsShopOfflineEventRunning())
					++count;
			}
		}
	}
#else
	const std::map<uint32_t, CHARACTER_SET>::iterator it = m_map_pkChrByRaceNum.find(30000);

	if (it == m_map_pkChrByRaceNum.end())
		return 0;

	int count = 0;
	for (auto ch : it->second)
	{
		if (ch && ch->IsShopOfflineEventRunning())
			++count;
	}
#endif

	return count;
}
#endif

#define FIND_JOB_WARRIOR_0	(1 << 3)
#define FIND_JOB_WARRIOR_1	(1 << 4)
#define FIND_JOB_WARRIOR_2	(1 << 5)
#define FIND_JOB_WARRIOR	(FIND_JOB_WARRIOR_0 | FIND_JOB_WARRIOR_1 | FIND_JOB_WARRIOR_2)
#define FIND_JOB_ASSASSIN_0	(1 << 6)
#define FIND_JOB_ASSASSIN_1	(1 << 7)
#define FIND_JOB_ASSASSIN_2	(1 << 8)
#define FIND_JOB_ASSASSIN	(FIND_JOB_ASSASSIN_0 | FIND_JOB_ASSASSIN_1 | FIND_JOB_ASSASSIN_2)
#define FIND_JOB_SURA_0		(1 << 9)
#define FIND_JOB_SURA_1		(1 << 10)
#define FIND_JOB_SURA_2		(1 << 11)
#define FIND_JOB_SURA		(FIND_JOB_SURA_0 | FIND_JOB_SURA_1 | FIND_JOB_SURA_2)
#define FIND_JOB_SHAMAN_0	(1 << 12)
#define FIND_JOB_SHAMAN_1	(1 << 13)
#define FIND_JOB_SHAMAN_2	(1 << 14)
#define FIND_JOB_SHAMAN		(FIND_JOB_SHAMAN_0 | FIND_JOB_SHAMAN_1 | FIND_JOB_SHAMAN_2)
#ifdef ENABLE_WOLFMAN_CHARACTER
#define FIND_JOB_WOLFMAN_0	(1 << 15)
#define FIND_JOB_WOLFMAN_1	(1 << 16)
#define FIND_JOB_WOLFMAN_2	(1 << 17)
#define FIND_JOB_WOLFMAN		(FIND_JOB_WOLFMAN_0 | FIND_JOB_WOLFMAN_1 | FIND_JOB_WOLFMAN_2)
#endif

//
// (job+1)*3+(skill_group)
//
LPCHARACTER CHARACTER_MANAGER::FindSpecifyPC(uint32_t uiJobFlag, long lMapIndex, LPCHARACTER except, int iMinLevel, int iMaxLevel)
{
	LPCHARACTER chFind = nullptr;
	int n = 0;

	for (auto& it : m_map_pkChrByPID)
	{
		LPCHARACTER ch = it.second;
		if (!ch)
			continue;

		if (ch == except)
			continue;

		if (ch->GetLevel() < iMinLevel)
			continue;

		if (ch->GetLevel() > iMaxLevel)
			continue;

		if (ch->GetMapIndex() != lMapIndex)
			continue;

		if (uiJobFlag)
		{
			const uint32_t uiChrJob = (1 << ((ch->GetJob() + 1) * 3 + ch->GetSkillGroup()));

			if (!IS_SET(uiJobFlag, uiChrJob))
				continue;
		}

		if (!chFind || number(1, ++n) == 1)
			chFind = ch;
	}

	return chFind;
}

int CHARACTER_MANAGER::GetMobItemRate(LPCHARACTER ch)
{
	if (!ch)
		return m_iMobItemRate;

	//PREVENT_TOXICATION_FOR_CHINA
	if (g_bChinaIntoxicationCheck)
	{
		if (ch->IsOverTime(OT_3HOUR))
		{
			if (ch && ch->GetPremiumRemainSeconds(PREMIUM_ITEM) > 0)
				return m_iMobItemRatePremium / 2;
			return m_iMobItemRate / 2;
		}
		else if (ch->IsOverTime(OT_5HOUR))
		{
			return 0;
		}
	}
	//END_PREVENT_TOXICATION_FOR_CHINA
	if (ch && ch->GetPremiumRemainSeconds(PREMIUM_ITEM) > 0)
		return m_iMobItemRatePremium;
	return m_iMobItemRate;
}

int CHARACTER_MANAGER::GetMobDamageRate(LPCHARACTER ch)
{
	return m_iMobDamageRate;
}

int CHARACTER_MANAGER::GetMobGoldAmountRate(LPCHARACTER ch)
{
	if (!ch)
		return m_iMobGoldAmountRate;

	//PREVENT_TOXICATION_FOR_CHINA
	if (g_bChinaIntoxicationCheck)
	{
		if (ch->IsOverTime(OT_3HOUR))
		{
			if (ch && ch->GetPremiumRemainSeconds(PREMIUM_GOLD) > 0)
				return m_iMobGoldAmountRatePremium / 2;
			return m_iMobGoldAmountRate / 2;
		}
		else if (ch->IsOverTime(OT_5HOUR))
		{
			return 0;
		}
	}
	//END_PREVENT_TOXICATION_FOR_CHINA
	if (ch && ch->GetPremiumRemainSeconds(PREMIUM_GOLD) > 0)
		return m_iMobGoldAmountRatePremium;
	return m_iMobGoldAmountRate;
}

int CHARACTER_MANAGER::GetMobGoldDropRate(LPCHARACTER ch)
{
	if (!ch)
		return m_iMobGoldDropRate;

	//PREVENT_TOXICATION_FOR_CHINA
	if (g_bChinaIntoxicationCheck)
	{
		if (ch->IsOverTime(OT_3HOUR))
		{
			if (ch && ch->GetPremiumRemainSeconds(PREMIUM_GOLD) > 0)
				return m_iMobGoldDropRatePremium / 2;
			return m_iMobGoldDropRate / 2;
		}
		else if (ch->IsOverTime(OT_5HOUR))
		{
			return 0;
		}
	}
	//END_PREVENT_TOXICATION_FOR_CHINA
	if (ch && ch->GetPremiumRemainSeconds(PREMIUM_GOLD) > 0)
		return m_iMobGoldDropRatePremium;
	return m_iMobGoldDropRate;
}

int CHARACTER_MANAGER::GetMobExpRate(LPCHARACTER ch)
{
	if (!ch)
		return m_iMobExpRate;

	//PREVENT_TOXICATION_FOR_CHINA
	if (g_bChinaIntoxicationCheck)
	{
		if (ch->IsOverTime(OT_3HOUR))
		{
			if (ch && ch->GetPremiumRemainSeconds(PREMIUM_EXP) > 0)
				return m_iMobExpRatePremium / 2;
			return m_iMobExpRate / 2;
		}
		else if (ch->IsOverTime(OT_5HOUR))
		{
			return 0;
		}
	}
	//END_PREVENT_TOXICATION_FOR_CHINA
	if (ch && ch->GetPremiumRemainSeconds(PREMIUM_EXP) > 0)
		return m_iMobExpRatePremium;
	return m_iMobExpRate;
}

int CHARACTER_MANAGER::GetUserDamageRate(LPCHARACTER ch)
{
	if (!ch)
		return m_iUserDamageRate;

	if (ch && ch->GetPremiumRemainSeconds(PREMIUM_EXP) > 0)
		return m_iUserDamageRatePremium;

	return m_iUserDamageRate;
}

void CHARACTER_MANAGER::SendScriptToMap(long lMapIndex, const std::string& s)
{
	LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap(lMapIndex);

	if (nullptr == pSecMap)
		return;

	TPacketGCScript p{};

	p.header = HEADER_GC_SCRIPT;
	p.skin = 1;
	p.src_size = static_cast<uint16_t>(s.size());

	quest::FSendPacket f;
	p.size = p.src_size + sizeof(TPacketGCScript);
	f.buf.write(&p, sizeof(TPacketGCScript));
	f.buf.write(&s[0], s.size());

	pSecMap->for_each(f);
}

bool CHARACTER_MANAGER::BeginPendingDestroy()
{
	// To support the function that does not flush when Begin is repeated after Begin
	// If already started, return false
	if (m_bUsePendingDestroy)
		return false;

	m_bUsePendingDestroy = true;
	return true;
}

void CHARACTER_MANAGER::FlushPendingDestroy()
{
	using namespace std;

	m_bUsePendingDestroy = false; // A flag must be set first for actual destruction to occur.

	if (!m_set_pkChrPendingDestroy.empty())
	{
		sys_log(0, "FlushPendingDestroy size %d", m_set_pkChrPendingDestroy.size());

		CHARACTER_SET::iterator it = m_set_pkChrPendingDestroy.begin(),
			end = m_set_pkChrPendingDestroy.end();
		for (; it != end; ++it) {
			M2_DESTROY_CHARACTER(*it);
		}

		m_set_pkChrPendingDestroy.clear();
	}
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
LPCHARACTER CHARACTER_MANAGER::SpawnShop(LPCHARACTER ch, std::string shopSign, TShopItemTable* shopItemTable, uint8_t itemCount, uint32_t startTime /*= 0*/
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
	, uint32_t dwPolyVnum, uint8_t bTitleType, bool isShopDeco
#endif
)
{
	// Create the shop character instance (The name's only used for VID CRC generation)
	LPCHARACTER myDetachedShop = CHARACTER_MANAGER::Instance().CreateCharacter("shop");
	if (!myDetachedShop)
		return nullptr;
	myDetachedShop->SetShopSign(shopSign);

	// Load the shop npc info and store it
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
	const CMob* pkMob = CMobManager::Instance().Get(dwPolyVnum != 0 ? dwPolyVnum : 30000);
#else
	const CMob* pkMob = CMobManager::Instance().Get(30000);
#endif
	if (!pkMob)
	{
		sys_err("Could not load shop npc info when creating shop!");
		return nullptr;
	}
	myDetachedShop->SetProto(pkMob);

	// Register detached Shop as real Shop
	std::string name = ch->GetName();
	myDetachedShop->SetName(name);
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
	myDetachedShop->SetLevel(ch->GetLevel());
#ifdef ENABLE_YOHARA_SYSTEM
	myDetachedShop->SetConquerorLevel(ch->GetConquerorLevel());
#endif
	myDetachedShop->SetAlignment(ch->GetAlignment());
	myDetachedShop->SetEmpire(ch->GetEmpire());
	//
	if (ch->GetEmpire() < 1 || ch->GetEmpire() > 3)
		sys_err("Invalid shop empire for pid: %u", ch->GetPlayerID());
	//

	if (ch->GetGuild())
		myDetachedShop->SetGuild(ch->GetGuild());
#endif
	//myDetachedShop->MarkAsShop();

	// Block it from randomly moving around.
	myDetachedShop->SetNoMoveFlag();

	// Link the ghost shop with the owner. Note that while the shop carries
	// the player ID, it will not be possible to find the shop on
	// CHARACTER_MANAGER's find-by-player-id map.
	myDetachedShop->SetPlayerID(ch->GetPlayerID());

	// Create the shop and bind it
	myDetachedShop->SetMyShop(CShopManager::Instance().CreatePCShop(myDetachedShop, ch, shopItemTable, itemCount, shopSign.c_str()));

	/*
	* This shop should be as separated as possible from other shops.
	* To achieve this we are going to create a 20x20 grid around the shop, each grid being a surface of value 1 in the map.
	*
	* The grid will have some initially unavailable positions for the coordinates that can not be walked on.
	* The grid will then be filled with all the shops within that 400 units area (those positions will be unavailable).
	* Finally, a shop position will be picked
	*/
	const long curX = ch->GetX();
	const long curY = ch->GetY();
	const long mapIndex = ch->GetMapIndex();
	int deltaX = 0;
	int deltaY = 0;

	if (!mapIndex)
	{
		sys_log(0, "OpenMyShop: invalid mapindex for char");
		M2_DESTROY_CHARACTER(myDetachedShop);
		return nullptr;
	}

	{
		// Create the grid
		constexpr int gridSide = 33;
		assert(gridSide % 2 != 0); // Must be odd!
		CGrid grid(gridSide, gridSide);

		constexpr int gridToCoordRatio = 70 * 5 / 2; // One grid cell will equal to a N units area in the world. 70 is the minimum
		constexpr int centerPos = (gridSide + 1) / 2 * (gridSide - 1); // Center position of the grid
		const int centerPosColumn = grid.GetPosColumn(centerPos);
		const int centerPosRow = grid.GetPosRow(centerPos);

		constexpr int coordVar = (gridSide - 1) / 2 + 1; // Coordinate variance (considering we are assuming the centerPos is our current position).

		// Inspect terrain
		LPSECTREE tree;
		for (long col = 0; col < gridSide; ++col)
		{
			for (long row = 0; row < gridSide; ++row)
			{
				const int x = (col - coordVar) * gridToCoordRatio + curX;
				const int y = (row - coordVar) * gridToCoordRatio + curY;
				tree = SECTREE_MANAGER::Instance().Get(mapIndex, x, y);
				if (tree->IsAttr(x, y, ATTR_BLOCK | ATTR_OBJECT | ATTR_WATER))
					grid.Put(col + row * gridSide, 1, 1);
			}
		}

		// Add shops to the grid
#	ifdef ENABLE_MYSHOP_DECO
		const auto cvi = GetCharactersByRaceNum(dwPolyVnum);
#	else
		const auto cvi = GetCharactersByRaceNum(30000);
#	endif

		for (const auto shopChar : cvi)
		{
			if (!shopChar)
				continue;

			if (shopChar->GetPlayerID() == ch->GetPlayerID())
				continue;

			const long shopX = shopChar->GetX();
			const long shopY = shopChar->GetY();

			// Is shop within our grid?
			if (shopX >= curX - coordVar * gridToCoordRatio && shopX <= curX + coordVar * gridToCoordRatio &&
				shopY >= curY - coordVar * gridToCoordRatio && shopY <= curY + coordVar * gridToCoordRatio)
			{
				const int col = (shopX - curX) / gridToCoordRatio + centerPosColumn;
				const int row = (shopY - curY) / gridToCoordRatio + centerPosRow;

				const int pos = col + row * gridSide;
				grid.Put(pos, 1, 1);
			}
		}

		// Grid is created, let's see where we place our new shop.
		bool placed = false;

		// First stick to the position we are at if possible (center position of the grid)

		if (grid.IsEmpty(centerPos, 1, 1))
		{
			placed = true;
			grid.Put(centerPos, 1, 1);
		}
		// Otherwise, find a blank position in the grid, closest to the center
		else
		{
			int bestDist = -1;
			int bestPos = -1;

			for (int col = 0; col < gridSide; ++col)
			{
				for (int row = 0; row < gridSide; ++row)
				{
					const int pos = row * gridSide + col;
					if (grid.IsEmpty(pos, 1, 1))
					{
						const int dist = abs(row - centerPosRow) + abs(col - centerPosColumn);
						if (bestDist < 0 || bestDist > dist)
						{
							bestDist = dist;
							bestPos = pos;
						}
					}
				}
			}

			if (bestPos > -1)
			{
				placed = true;
				deltaX = (grid.GetPosColumn(bestPos) - centerPosColumn) * gridToCoordRatio;
				deltaY = (grid.GetPosRow(bestPos) - centerPosRow) * gridToCoordRatio;

				grid.Put(bestPos, 1, 1);
			}
		}

		if (!placed)
			sys_err("Shop #%u could not placed at coords (%ld, %ld) - Mapindex %ld, channel %d", ch->GetPlayerID(), ch->GetX(), ch->GetY(), ch->GetMapIndex(), g_bChannel);
		else
			sys_log(0, "Shop #%u placed around (%ld, %ld) with an offset of (%d, %d)", ch->GetPlayerID(), ch->GetX(), ch->GetY(), deltaX, deltaY);

		/*if (test_server)
			grid.Print();*/
	}

	// Reposition shop if needed
	if (deltaX != 0 || deltaY != 0)
	{
		const PIXEL_POSITION pos = { curX + deltaX, curY + deltaY, 0 };
		ch->SetXYZ(pos);
	}

	TPacketPlayerShopSign p{};
	p.header = HEADER_GC_MY_SHOP_SIGN;

	if (!myDetachedShop->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY(), ch->GetZ(), true))
	{
		memset(p.sign, 0, sizeof(p.sign)); // Close shop on client
		ch->GetDesc()->Packet(&p, sizeof(TPacketPlayerShopSign));
		sys_log(0, "OpenMyShop: cannot display detached shop");
		M2_DESTROY_CHARACTER(myDetachedShop);
		return nullptr;
	}

	if (ch->IsPC())
	{
		//Return to our old position
		const PIXEL_POSITION ppos = { curX, curY, 0 };
		ch->SetXYZ(ppos);

		// Set the shop sign for the owner
		strlcpy(p.sign, shopSign.c_str(), sizeof(p.sign));
		ch->GetDesc()->Packet(&p, sizeof(TPacketPlayerShopSign));

		// Broadcast if we are spawning dynamically (not on boot), as only then there will be an actual character.
		TPacketGCShopSign p2{};
		p2.bHeader = HEADER_GC_SHOP_SIGN;
		p2.dwVID = myDetachedShop->GetVID();
		strlcpy(p2.szSign, shopSign.c_str(), sizeof(p2.szSign));
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
		p2.bType = bTitleType;
#endif
		ch->PacketAround(&p2, sizeof(TPacketGCShopSign));

		// Sync position on client
		TPacketGCShopSyncPos pos{};
		pos.bHeader = HEADER_GC_SYNC_SHOP_POSITION;
		pos.channel = g_bChannel;
		pos.xGlobal = curX + deltaX;
		pos.yGlobal = curY + deltaY;
		ch->GetDesc()->Packet(&pos, sizeof(TPacketGCShopSyncPos));
	}

	//LPSECTREE_MAP pTargetMap = SECTREE_MANAGER::Instance().GetMap(myDetachedShop->GetMapIndex());

	// Set the start time
	if (!startTime)
		startTime = std::time(nullptr);

	if (!ch->IsPC())
	{
		// Shop was created on boot & no player is online!
		myDetachedShop->StartShopOfflineEvent();
	}
#	ifdef ENABLE_SHOP_PREMIUM_TIME
	else
	{
		// Request syncing of times (offline / premium offline), as we are spawning it *now*
		// and shop may already have some
		uint32_t pid = ch->GetPlayerID();
		db_clientdesc->DBPacketHeader(HEADER_GD_SHOP, 0, sizeof(uint8_t) + sizeof(uint32_t));
		uint8_t subheader = SHOP_SUBHEADER_GD_REQUEST_PREMIUM_TIME_SYNC;
		db_clientdesc->Packet(&subheader, sizeof(uint8_t));
		db_clientdesc->Packet(&pid, sizeof(uint32_t));
	}
#	endif

	myDetachedShop->GetMyShop()->SetOpenTime(startTime);

#ifdef ENABLE_MYSHOP_DECO_PREMIUM
	myDetachedShop->GetMyShop()->SetTitleType(bTitleType);
	myDetachedShop->GetMyShop()->SetShopDeco(isShopDeco);
#endif
	// This is the event that will save the shop in db to recover from crashes.
	myDetachedShop->GetMyShop()->Save();

	return myDetachedShop;
}

LPCHARACTER CHARACTER_MANAGER::FindPCShopCharacterByPID(uint32_t pid)
{
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
	std::vector<CharacterVectorInteractor> cvi;
	GetCharactersShopRaceNum(cvi);

	for (auto& ptr : cvi)
	{
		for (auto character : ptr)
		{
			if (character && character->GetPlayerID() == pid && character->IsShop())
			{
				return character;
			}
		}
	}
#else
	const auto cvi = CHARACTER_MANAGER::Instance().GetCharactersByRaceNum(30000);
	for (auto character : cvi)
	{
		if (character && character->GetPlayerID() == pid && character->IsShop())
		{
			return character;
		}
	}
#endif

	return nullptr;
}
#endif

#ifdef ENABLE_SUPERSTONE_RAIN_EVENT
void CHARACTER_MANAGER::InitializeSuperStoneEvent(int iEnable)
{
	const uint32_t stoneMaps[17] = { 1, 3, 21, 23, 41, 43, 61, 62, 63, 64, 67, 68, 301, 302, 303, 304, 353 };

	// Shinsoo
	char szFileMapA1[256 + 1] = { '\0' };
	snprintf(&szFileMapA1[0], sizeof(szFileMapA1), "data/event/supermetins/map_a1.txt");
	char szFileMapA3[256 + 1] = { '\0' };
	snprintf(&szFileMapA3[0], sizeof(szFileMapA3), "data/event/supermetins/map_a3.txt");

	// Chunjo
	char szFileMapB1[256 + 1] = { '\0' };
	snprintf(&szFileMapB1[0], sizeof(szFileMapB1), "data/event/supermetins/map_b1.txt");
	char szFileMapB3[256 + 1] = { '\0' };
	snprintf(&szFileMapB3[0], sizeof(szFileMapB3), "data/event/supermetins/map_b3.txt");

	// Jinno
	char szFileMapC1[256 + 1] = { '\0' };
	snprintf(&szFileMapC1[0], sizeof(szFileMapC1), "data/event/supermetins/map_c1.txt");
	char szFileMapC3[256 + 1] = { '\0' };
	snprintf(&szFileMapC3[0], sizeof(szFileMapC3), "data/event/supermetins/map_c3.txt");

	// Mount Sohan
	char szFileMapSnowm[256 + 1] = { '\0' };
	snprintf(&szFileMapSnowm[0], sizeof(szFileMapSnowm), "data/event/supermetins/map_snowm.txt");

	// Doyyumhwan
	char szFileMapFlame[256 + 1] = { '\0' };
	snprintf(&szFileMapFlame[0], sizeof(szFileMapFlame), "data/event/supermetins/map_flame.txt");

	// Yongbi Desert
	char szFileMapDesert[256 + 1] = { '\0' };
	snprintf(&szFileMapDesert[0], sizeof(szFileMapDesert), "data/event/supermetins/map_desert.txt");

	// Valley of Seungryong
	char szFileMapOrcs[256 + 1] = { '\0' };
	snprintf(&szFileMapOrcs[0], sizeof(szFileMapOrcs), "data/event/supermetins/map_orcs.txt");

	// Ghost Forest
	char szFileMapTrent1[256 + 1] = { '\0' };
	snprintf(&szFileMapTrent1[0], sizeof(szFileMapTrent1), "data/event/supermetins/map_trent1.txt");

	// Red Forest
	char szFileMapTrent2[256 + 1] = { '\0' };
	snprintf(&szFileMapTrent2[0], sizeof(szFileMapTrent2), "data/event/supermetins/map_trent2.txt");

	// Cape Dragon Fire
	char szFileMapCapeDragonHead[256 + 1] = { '\0' };
	snprintf(&szFileMapCapeDragonHead[0], sizeof(szFileMapCapeDragonHead), "data/event/supermetins/map_capedragonhead.txt");

	// Gautama Cliff
	char szFileMapDawnmistwood[256 + 1] = { '\0' };
	snprintf(&szFileMapDawnmistwood[0], sizeof(szFileMapDawnmistwood), "data/event/supermetins/map_dawnmistwood.txt");

	// Nephrite Bay
	char szFileMapBayBlackSand[256 + 1] = { '\0' };
	snprintf(&szFileMapBayBlackSand[0], sizeof(szFileMapBayBlackSand), "data/event/supermetins/map_bayblacksand.txt");

	// Thunder Mountains
	char szFileMapMtThunder[256 + 1] = { '\0' };
	snprintf(&szFileMapMtThunder[0], sizeof(szFileMapMtThunder), "data/event/supermetins/map_mtthunder.txt");

	// Enchant Forest
	char szFileMapEnchantForest[256 + 1] = { '\0' };
	snprintf(&szFileMapEnchantForest[0], sizeof(szFileMapEnchantForest), "data/event/supermetins/map_forest.txt");

	if (iEnable)
	{
		for (uint8_t i = 0; i < 17; i++)
		{
			const LPSECTREE_MAP& pkMap = SECTREE_MANAGER::Instance().GetMap(stoneMaps[i]);
			if (pkMap && map_allow_find(stoneMaps[i]))
			{
				switch (stoneMaps[i])
				{
				case 1:
					regen_load(&szFileMapA1[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					break;
				case 3:
					regen_load(&szFileMapA3[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					break;
				case 21:
					regen_load(&szFileMapB1[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					break;
				case 23:
					regen_load(&szFileMapB3[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					break;
				case 41:
					regen_load(&szFileMapC1[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					break;
				case 43:
					regen_load(&szFileMapC3[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					break;
				case 61:
					regen_load(&szFileMapSnowm[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					break;
				case 62:
					regen_load(&szFileMapFlame[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					break;
				case 63:
					regen_load(&szFileMapDesert[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					break;
				case 64:
					regen_load(&szFileMapOrcs[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					break;
				case 67:
					regen_load(&szFileMapTrent1[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					break;
				case 68:
					regen_load(&szFileMapTrent2[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					break;
				case 301:
					regen_load(&szFileMapCapeDragonHead[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					break;
				case 302:
					regen_load(&szFileMapDawnmistwood[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					break;
				case 303:
					regen_load(&szFileMapBayBlackSand[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					break;
				case 304:
					regen_load(&szFileMapMtThunder[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					break;
				case 353:
					regen_load(&szFileMapEnchantForest[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					break;
				default:
					break;
				}
			}
		}
	}
	else
	{
		clear_regen(&szFileMapA1[0]);
		clear_regen(&szFileMapA3[0]);
		clear_regen(&szFileMapB1[0]);
		clear_regen(&szFileMapB3[0]);
		clear_regen(&szFileMapC1[0]);
		clear_regen(&szFileMapC3[0]);
		clear_regen(&szFileMapSnowm[0]);
		clear_regen(&szFileMapFlame[0]);
		clear_regen(&szFileMapDesert[0]);
		clear_regen(&szFileMapOrcs[0]);
		clear_regen(&szFileMapTrent1[0]);
		clear_regen(&szFileMapTrent2[0]);
		clear_regen(&szFileMapCapeDragonHead[0]);
		clear_regen(&szFileMapDawnmistwood[0]);
		clear_regen(&szFileMapBayBlackSand[0]);
		clear_regen(&szFileMapMtThunder[0]);
		clear_regen(&szFileMapEnchantForest[0]);
	}
}
#endif

#ifdef ENABLE_MINING_EVENT
void CHARACTER_MANAGER::InitializeMiningEvent(int iEnable)
{
	// Mining Map
	char szFileMiningMap[256 + 1] = { '\0' };
	snprintf(&szFileMiningMap[0], sizeof(szFileMiningMap), "data/event/mining/map_mining.txt");

	constexpr uint32_t alchemist = 20001;
	constexpr uint32_t deokbae = 20015;

	if (iEnable)
	{
		const LPSECTREE_MAP& pkMap = SECTREE_MANAGER::Instance().GetMap(103);
		if (pkMap && map_allow_find(103))
		{
			CHARACTER_MANAGER::Instance().SpawnMob(alchemist, 103, pkMap->m_setting.iBaseX + 38600, pkMap->m_setting.iBaseY + 38300, 0, false, 90, true);
			CHARACTER_MANAGER::Instance().SpawnMob(deokbae, 103, pkMap->m_setting.iBaseX + 38100, pkMap->m_setting.iBaseY + 38300, 0, false, 90, true);
			regen_load(&szFileMiningMap[0], 103, pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
		}
	}
	else
	{
		const auto chars_alch = CHARACTER_MANAGER::Instance().GetCharactersByRaceNum(alchemist);
		const auto chars_deok = CHARACTER_MANAGER::Instance().GetCharactersByRaceNum(deokbae);
		for (auto ch_alch : chars_alch)
			M2_DESTROY_CHARACTER(ch_alch);

		for (auto ch_deok : chars_deok)
			M2_DESTROY_CHARACTER(ch_deok);

		clear_regen(&szFileMiningMap[0]);
	}
}
#endif

CharacterVectorInteractor::CharacterVectorInteractor(const CHARACTER_SET & r)
{
	using namespace std;

	reserve(r.size());
	insert(end(), r.begin(), r.end());

	if (CHARACTER_MANAGER::Instance().BeginPendingDestroy())
		m_bMyBegin = true;
}

CharacterVectorInteractor::~CharacterVectorInteractor()
{
	if (m_bMyBegin)
		CHARACTER_MANAGER::Instance().FlushPendingDestroy();
}

#ifdef ENABLE_PORTAL_LEVEL_LIMIT
#include <sstream>
#include <fstream>

void CHARACTER_MANAGER::InitPortalLevelLimit(const char* FileName)
{
	std::ifstream file(FileName);
	if (file.is_open() == false)
	{
		sys_err("Cannot open %s file.", FileName);
		return;
	}

	std::string   line;

	while (std::getline(file, line))
	{
		std::stringstream linestream(line);
		std::string dwRaceNum;
		int iMinLevel;
		int iMaxLevel;

		std::getline(linestream, dwRaceNum, '\t');

		if (dwRaceNum.empty() || dwRaceNum.at(0) == '#')
			continue;

		linestream >> iMinLevel >> iMaxLevel;
		AddPortalLevelLimit(std::stoul(dwRaceNum), iMinLevel, iMaxLevel);
	}

	file.close();
}

void CHARACTER_MANAGER::AddPortalLevelLimit(uint32_t dwRace, int iMinLevel, int iMaxLevel)
{
	iMinLevel = MINMAX(1, iMinLevel, gPlayerMaxLevel);
	iMaxLevel = MINMAX(1, iMaxLevel, gPlayerMaxLevel);

	m_mapPlayerPortalLevelLimit[dwRace] = std::make_pair(iMinLevel, iMaxLevel);
}

const std::pair<int, int>* CHARACTER_MANAGER::GetPortalLevelLimit(uint32_t dwRace) const
{
	auto it = m_mapPlayerPortalLevelLimit.find(dwRace);
	if (it != m_mapPlayerPortalLevelLimit.end())
		return &it->second;

	return nullptr;
}
#endif

#ifdef ENABLE_MULTI_FARM_BLOCK
void CHARACTER_MANAGER::CheckMultiFarmAccounts(const char* szIP)
{
	auto it = m_mapmultiFarm.find(szIP);
	if (it != m_mapmultiFarm.end())
	{
		auto itVec = it->second.begin();
		while (itVec != it->second.end())
		{
			LPCHARACTER ch = FindByPID(itVec->playerID);
			CCI* chP2P = P2P_MANAGER::Instance().FindByPID(itVec->playerID);
			if (!ch && !chP2P)
				itVec = it->second.erase(itVec);
			else
				++itVec;
		}
		if (!it->second.size())
			m_mapmultiFarm.erase(szIP);
	}
}

void CHARACTER_MANAGER::RemoveMultiFarm(const char* szIP, const uint32_t playerID, const bool isP2P)
{
	if (!isP2P)
	{
		TPacketGGMultiFarm p;
		p.header = HEADER_GG_MULTI_FARM;
		p.subHeader = MULTI_FARM_REMOVE;
		p.playerID = playerID;
		strlcpy(p.playerIP, szIP, sizeof(p.playerIP));
		P2P_MANAGER::Instance().Send(&p, sizeof(TPacketGGMultiFarm));
	}

	auto it = m_mapmultiFarm.find(szIP);
	if (it != m_mapmultiFarm.end())
	{
		for (auto itVec = it->second.begin(); itVec != it->second.end(); ++itVec)
		{
			if (itVec->playerID == playerID)
			{
				it->second.erase(itVec);
				break;
			}
		}
		if (!it->second.size())
			m_mapmultiFarm.erase(szIP);
	}
}

void CHARACTER_MANAGER::SetMultiFarm(const char* szIP, const uint32_t playerID, const char* playerName, const bool bStatus, const uint8_t affectType, const int affectTime)
{
	const auto it = m_mapmultiFarm.find(szIP);
	if (it != m_mapmultiFarm.end())
	{
		for (auto itVec = it->second.begin(); itVec != it->second.end(); ++itVec)
		{
			if (itVec->playerID == playerID)
			{
				itVec->farmStatus = bStatus;
				itVec->affectType = affectType;
				itVec->affectTime = affectTime;
				return;
			}
		}
		it->second.emplace_back(TMultiFarm(playerID, playerName, bStatus, affectType, affectTime));
	}
	else
	{
		std::vector<TMultiFarm> m_vecFarmList;
		m_vecFarmList.emplace_back(TMultiFarm(playerID, playerName, bStatus, affectType, affectTime));
		m_mapmultiFarm.emplace(szIP, m_vecFarmList);
	}
}

int CHARACTER_MANAGER::GetMultiFarmCount(const char* playerIP, std::map<uint32_t, std::pair<std::string, bool>>& m_mapNames)
{
	int accCount = 0;
	bool affectTimeHas = false;
	uint8_t affectType = 0;
	const auto it = m_mapmultiFarm.find(playerIP);
	if (it != m_mapmultiFarm.end())
	{
		for (auto itVec = it->second.begin(); itVec != it->second.end(); ++itVec)
		{
			if (itVec->farmStatus)
				accCount++;
			if (itVec->affectTime > get_global_time())
				affectTimeHas = true;
			if (itVec->affectType > affectType)
				affectType = itVec->affectType;
			m_mapNames.emplace(itVec->playerID, std::make_pair(itVec->playerName, itVec->farmStatus));
		}
	}

	if (affectTimeHas && affectType > 0)
		accCount -= affectType;
	if (accCount < 0)
		accCount = 0;

	return accCount;
}

void CHARACTER_MANAGER::CheckMultiFarmAccount(const char* szIP, const uint32_t playerID, const char* playerName, const bool bStatus, uint8_t affectType, int affectDuration, bool isP2P)
{
	CheckMultiFarmAccounts(szIP);

	LPCHARACTER ch = FindByPID(playerID);
	if (ch && bStatus)
	{
		affectDuration = ch->FindAffect(AFFECT_MULTI_FARM_PREMIUM) ? get_global_time() + ch->FindAffect(AFFECT_MULTI_FARM_PREMIUM)->lDuration : 0;
		affectType = ch->FindAffect(AFFECT_MULTI_FARM_PREMIUM) ? ch->FindAffect(AFFECT_MULTI_FARM_PREMIUM)->lApplyValue : 0;
	}

	std::map<uint32_t, std::pair<std::string, bool>> m_mapNames;
	int farmPlayerCount = GetMultiFarmCount(szIP, m_mapNames);
	if (bStatus)
	{
		if (farmPlayerCount >= 2)
		{
			CheckMultiFarmAccount(szIP, playerID, playerName, false);
			return;
		}
	}

	if (!isP2P)
	{
		TPacketGGMultiFarm p;
		p.header = HEADER_GG_MULTI_FARM;
		p.subHeader = MULTI_FARM_SET;
		p.playerID = playerID;
		strlcpy(p.playerIP, szIP, sizeof(p.playerIP));
		strlcpy(p.playerName, playerName, sizeof(p.playerIP));
		p.farmStatus = bStatus;
		p.affectType = affectType;
		p.affectTime = affectDuration;
		P2P_MANAGER::Instance().Send(&p, sizeof(TPacketGGMultiFarm));
	}

	SetMultiFarm(szIP, playerID, playerName, bStatus, affectType, affectDuration);
	if (ch)
		ch->SetMultiStatus(bStatus);

	m_mapNames.clear();
	farmPlayerCount = GetMultiFarmCount(szIP, m_mapNames);

	for (auto it = m_mapNames.begin(); it != m_mapNames.end(); ++it)
	{
		LPCHARACTER newCh = FindByPID(it->first);
		if (newCh)
		{
			newCh->ChatPacket(CHAT_TYPE_COMMAND, "UpdateMultiFarmAffect %d %d", newCh->GetMultiStatus(), newCh == ch ? true : false);
			for (auto itEx = m_mapNames.begin(); itEx != m_mapNames.end(); ++itEx)
			{
				if (itEx->second.second)
					newCh->ChatPacket(CHAT_TYPE_COMMAND, "UpdateMultiFarmPlayer %s", itEx->second.first.c_str());
			}
		}
	}
}
#endif
