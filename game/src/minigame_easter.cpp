#include "stdafx.h"

#ifdef ENABLE_EASTER_EVENT
#include "config.h"
#include "minigame_manager.h"

#include "../../common/length.h"
#include "../../common/tables.h"

#include "locale_service.h"
#include "char_manager.h"
#include "regen.h"

void CMiniGameManager::InitializeEasterEvent(int iEnable)
{
	const uint32_t stoneMaps[19] =
	{
		1, 3, 21, 23, 41, 43, 61, 62, 63, 64, 65, 67, 68, 69, 70,
		301, 302, 303, 304 // cape of dragon head
	};

	char szFileName10[256 + 1] = { '\0' };
	snprintf(&szFileName10[0], sizeof(szFileName10), "data/event/easter/stone_level_10.txt");

	char szFileName20[256 + 1] = { '\0' };
	snprintf(&szFileName20[0], sizeof(szFileName20), "data/event/easter/stone_level_20.txt");

	char szFileName30[256 + 1] = { '\0' };
	snprintf(&szFileName30[0], sizeof(szFileName30), "data/event/easter/stone_level_30.txt");

	char szFileName40[256 + 1] = { '\0' };
	snprintf(&szFileName40[0], sizeof(szFileName40), "data/event/easter/stone_level_40.txt");

	char szFileName50[256 + 1] = { '\0' };
	snprintf(&szFileName50[0], sizeof(szFileName50), "data/event/easter/stone_level_50.txt");

	char szFileName60[256 + 1] = { '\0' };
	snprintf(&szFileName60[0], sizeof(szFileName60), "data/event/easter/stone_level_60.txt");

	char szFileName70[256 + 1] = { '\0' };
	snprintf(&szFileName70[0], sizeof(szFileName70), "data/event/easter/stone_level_70.txt");

	char szFileName80[256 + 1] = { '\0' };
	snprintf(&szFileName80[0], sizeof(szFileName80), "data/event/easter/stone_level_80.txt");

	char szFileName90[256 + 1] = { '\0' };
	snprintf(&szFileName90[0], sizeof(szFileName90), "data/event/easter/stone_level_90.txt");

	char szFileName95[256 + 1] = { '\0' };
	snprintf(&szFileName95[0], sizeof(szFileName95), "data/event/easter/stone_level_95.txt");

	if (iEnable)
	{
		CMiniGameManager::Instance().SpawnEventNPC(EASTER_RABBIT_NPC);

		for (uint8_t i = 0; i < 19; i++)
		{
			const LPSECTREE_MAP& pkMap = SECTREE_MANAGER::Instance().GetMap(stoneMaps[i]);
			if (pkMap && map_allow_find(stoneMaps[i]))
			{
				if (stoneMaps[i] == 1 || stoneMaps[i] == 21 || stoneMaps[i] == 41)
				{
					regen_load(&szFileName10[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					regen_load(&szFileName20[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
				}
				else if (stoneMaps[i] == 3 || stoneMaps[i] == 23 || stoneMaps[i] == 43)
				{
					regen_load(&szFileName20[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					regen_load(&szFileName30[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
				}
				else if (stoneMaps[i] == 64)
				{
					regen_load(&szFileName30[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					regen_load(&szFileName40[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					regen_load(&szFileName50[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
				}
				else if (stoneMaps[i] == 63 || stoneMaps[i] == 65)
				{
					regen_load(&szFileName40[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					regen_load(&szFileName50[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
				}
				else if (stoneMaps[i] == 61)
				{
					regen_load(&szFileName50[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					regen_load(&szFileName60[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
				}
				else if (stoneMaps[i] == 67 || stoneMaps[i] == 62 || stoneMaps[i] == 69)
				{
					regen_load(&szFileName60[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					regen_load(&szFileName70[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
				}
				else if (stoneMaps[i] == 68 || stoneMaps[i] == 70)
				{
					regen_load(&szFileName70[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					regen_load(&szFileName80[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
				}
				else if (stoneMaps[i] == 301 || stoneMaps[i] == 302 || stoneMaps[i] == 303 || stoneMaps[i] == 304)
				{
					regen_load(&szFileName90[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
					regen_load(&szFileName95[0], stoneMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
				}
			}
		}
	}
	else
	{
		const auto chars = CHARACTER_MANAGER::Instance().GetCharactersByRaceNum(EASTER_RABBIT_NPC);
		for (auto ch : chars)
		{
			M2_DESTROY_CHARACTER(ch);
		}

		clear_regen(&szFileName10[0]);
		clear_regen(&szFileName20[0]);
		clear_regen(&szFileName30[0]);
		clear_regen(&szFileName40[0]);
		clear_regen(&szFileName50[0]);
		clear_regen(&szFileName60[0]);
		clear_regen(&szFileName70[0]);
		clear_regen(&szFileName80[0]);
		clear_regen(&szFileName90[0]);
		clear_regen(&szFileName95[0]);
	}
}
#endif	// ENABLE_EASTER_EVENT