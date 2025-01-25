#include "stdafx.h"

#ifdef ENABLE_MINI_GAME
#include "config.h"
#include "minigame_manager.h"

#include "../../common/length.h"
#include "../../common/tables.h"
#include "p2p.h"
#include "locale_service.h"
#include "char.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "buffer_manager.h"
#include "packet.h"
#include "questmanager.h"
#include "questlua.h"
#include "start_position.h"
#include "char_manager.h"
#include "item_manager.h"
#include "sectree_manager.h"
#include "regen.h"
#include "log.h"
#include "db.h"
#include "target.h"
#include "party.h"

CMiniGameManager::CMiniGameManager()
{
	//Initialize();
}

CMiniGameManager::~CMiniGameManager()
{
	//Destroy();
}

bool CMiniGameManager::Initialize()
{
#ifdef ENABLE_EVENT_BANNER_FLAG
	m_bIsLoadedBanners = false;
#endif
	return true;
}

void CMiniGameManager::Destroy()
{
	return;
}

#ifdef ENABLE_EVENT_BANNER_FLAG
#include "CsvReader.h"
bool CMiniGameManager::InitializeBanners()
{
	if (m_bIsLoadedBanners)
		return false;

	const char* c_szFileName = "data/banner/list.txt";

	cCsvTable nameData;
	if (!nameData.Load(c_szFileName, '\t'))
	{
		sys_log(0, "%s couldn't be loaded or its format is incorrect.", c_szFileName);
		return false;
	}
	else
	{
		nameData.Next();
		while (nameData.Next())
		{
			if (nameData.ColCount() < 2)
				continue;

			BannerMap.insert(std::make_pair(atoi(nameData.AsStringByIndex(0)), nameData.AsStringByIndex(1)));
		}
	}
	nameData.Destroy();

	m_bIsLoadedBanners = true;

	uint32_t dwFlagVnum = quest::CQuestManager::Instance().GetEventFlag("banner");
	if (dwFlagVnum > 0)
		InitializeEvent(0, dwFlagVnum);

	return true;
}

bool CMiniGameManager::InitializeEvent(uint8_t type, uint16_t value)
{
	if (!m_bIsLoadedBanners)
		InitializeBanners();

	const char* c_szBannerName = "";
	if (type == NONE_EVENT)
		c_szBannerName = nullptr;
	else if (type == FIND_M_EVENT)
		return false;
		//c_szBannerName = "";
	else if (type == YUT_NORI_EVENT)
		return false;
		//c_szBannerName = "";

	else if (type == EASTER_EVENT)
		c_szBannerName = "easter";
	else if (type == HALLOWEEN_EVENT)
		c_szBannerName = "halloween";
	else if (type == RAMADAN_EVENT)
		c_szBannerName = "ramadan";
	else if (type == XMAS_EVENT)
		c_szBannerName = "xmas";

	bool bDestroy = true;
	bool bSpawn = false;

	uint32_t dwBannerVnum = 0;
	std::string strBannerName;

	if (!c_szBannerName)
	{
		BannerMapType::const_iterator it = BannerMap.find(value);
		if (it == BannerMap.end())
			return false;

		dwBannerVnum = it->first;
		strBannerName = it->second;
	}
	else
	{
		for (BannerMapType::const_iterator it = BannerMap.begin(); BannerMap.end() != it; ++it)
		{
			if (!strcmp(it->second.c_str(), c_szBannerName))
			{
				dwBannerVnum = it->first;
				strBannerName = it->second;
				break;
			}
		}
	}

	if (dwBannerVnum == 0 || strBannerName.empty())
		return false;

	if (value > 0)
		bSpawn = true;

	if (bDestroy)
	{
		quest::CQuestManager::Instance().RequestSetEventFlag("banner", 0);

		CharacterVectorInteractor i;
		CHARACTER_MANAGER::Instance().GetCharactersByRaceNum(dwBannerVnum);

		for (CharacterVectorInteractor::iterator it = i.begin(); it != i.end(); it++)
		{
			M2_DESTROY_CHARACTER(*it);
		}
	}

	if (bSpawn)
	{
		quest::CQuestManager::Instance().RequestSetEventFlag("banner", dwBannerVnum);

		if (map_allow_find(MapTypes::MAP_A1))
		{
			std::string strBannerFile = "data/banner/a/" + strBannerName + ".txt";

			if (LPSECTREE_MAP pkMap = SECTREE_MANAGER::Instance().GetMap(MapTypes::MAP_A1))
			{
				regen_do(strBannerFile.c_str(), MapTypes::MAP_A1, pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY, nullptr, false);
			}
		}

		if (map_allow_find(MapTypes::MAP_B1))
		{
			std::string strBannerFile = "data/banner/b/" + strBannerName + ".txt";

			if (LPSECTREE_MAP pkMap = SECTREE_MANAGER::Instance().GetMap(MapTypes::MAP_B1))
			{
				regen_do(strBannerFile.c_str(), MapTypes::MAP_B1, pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY, nullptr, false);
			}
		}

		if (map_allow_find(MapTypes::MAP_C1))
		{
			std::string strBannerFile = "data/banner/c/" + strBannerName + ".txt";

			if (LPSECTREE_MAP pkMap = SECTREE_MANAGER::Instance().GetMap(MapTypes::MAP_C1))
			{
				regen_do(strBannerFile.c_str(), MapTypes::MAP_C1, pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY, nullptr, false);
			}
		}
	}

	return true;
}
#endif

void CMiniGameManager::SpawnEventNPC(uint32_t dwVnum)
{
	CharacterVectorInteractor i;
	bool isNpcA1 = false;
	bool isNpcB1 = false;
	bool isNpcC1 = false;
	uint8_t bVnumC = 0;

	// { red_x, red_y, yellow_x, yellow_y, blue_x, blue_y }
	const uint32_t spawnPos[7][6] =
	{
#ifdef ENABLE_MONSTER_BACK
		{ 60800, 61700, 59600, 61000, 35700, 74300 }, // ATTENDANCE_NPC 30131
#endif
#ifdef ENABLE_EASTER_EVENT
		{ 59000, 47100, 51700, 74200, 30600, 82800 }, // EASTER_RABBIT_NPC 30129
#endif
#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
		{ 60800, 62000, 59600, 61200, 35400, 74000 }, // 20417
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
		{ 60800, 62300, 59600, 61400, 35000, 73800 }, // CATCH_KING_NPC 20506
#endif
#ifdef ENABLE_MINI_GAME_FINDM
		{ 61800, 62000, 60400, 61300, 36000, 74200 }, // 20507
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
		{ 61800, 62000, 60400, 61300, 36000, 74200 }, // 20502
#endif
#ifdef ENABLE_2016_VALENTINE
		{ 68300, 57700, 58600, 68500, 37200, 69500 }, // 20502
#endif
	};

	switch (dwVnum)
	{
#ifdef ENABLE_MONSTER_BACK
		case ATTENDANCE_NPC:
			bVnumC = 0;
			break;
#endif

#ifdef ENABLE_EASTER_EVENT
		case EASTER_RABBIT_NPC:
			bVnumC = 1;
			break;
#endif

#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
		case OKEY_CARD_NPC:
			bVnumC = 2;
			break;
#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
		case CATCH_KING_NPC:
			bVnumC = 3;
			break;
#endif

#ifdef ENABLE_MINI_GAME_FINDM
		case FIND_M_NPC:
			bVnumC = 4;
			break;
#endif

#ifdef ENABLE_MINI_GAME_YUTNORI
		case YUT_NORI_NPC:
			bVnumC = 5;
			break;
#endif

#ifdef ENABLE_2016_VALENTINE
		case 60005:
			bVnumC = 6;
			break;
#endif

		default:
			break;
	}

	const auto chars = CHARACTER_MANAGER::Instance().GetCharactersByRaceNum(dwVnum);
	for (auto tch : chars)
	{
		if (tch && tch->GetMapIndex() == 1)
			isNpcA1 = true;
		else if (tch && tch->GetMapIndex() == 21)
			isNpcB1 = true;
		else if (tch && tch->GetMapIndex() == 41)
			isNpcC1 = true;
	}

	if (map_allow_find(MAP_A1) && !isNpcA1)
	{
		if (const LPSECTREE_MAP& pkMap = SECTREE_MANAGER::Instance().GetMap(MAP_A1))
			CHARACTER_MANAGER::Instance().SpawnMob(dwVnum, MAP_A1, pkMap->m_setting.iBaseX + spawnPos[bVnumC][0], pkMap->m_setting.iBaseY + spawnPos[bVnumC][1], 0, false, 90, true);
	}
	if (map_allow_find(MAP_B1) && !isNpcB1)
	{
		if (const LPSECTREE_MAP& pkMap = SECTREE_MANAGER::Instance().GetMap(MAP_B1))
			CHARACTER_MANAGER::Instance().SpawnMob(dwVnum, MAP_B1, pkMap->m_setting.iBaseX + spawnPos[bVnumC][2], pkMap->m_setting.iBaseY + spawnPos[bVnumC][3], 0, false, 110, true);
	}
	if (map_allow_find(MAP_C1) && !isNpcC1)
	{
		if (const LPSECTREE_MAP& pkMap = SECTREE_MANAGER::Instance().GetMap(MAP_C1))
			CHARACTER_MANAGER::Instance().SpawnMob(dwVnum, MAP_C1, pkMap->m_setting.iBaseX + spawnPos[bVnumC][4], pkMap->m_setting.iBaseY + spawnPos[bVnumC][5], 0, false, 140, true);
	}
}

#endif
