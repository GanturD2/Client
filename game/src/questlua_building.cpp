#include "stdafx.h"
#include "config.h"
#include "questmanager.h"
#include "sectree_manager.h"
#include "char.h"
#include "guild.h"
#include "db.h"
#include "building.h"
#ifdef ENABLE_GUILDBANK_EXTENDED_LOGS
#include "log.h"
#endif

namespace quest
{
	//
	// "building" Lua functions
	//
	ALUA(building_get_land_id)
	{
		using namespace building;

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("invalid argument");
			lua_pushnumber(L, 0);
			return 1;
		}

		CLand * pkLand = CManager::Instance().FindLand((int) lua_tonumber(L, 1), (int) lua_tonumber(L, 2), (int) lua_tonumber(L, 3));
		lua_pushnumber(L, pkLand ? pkLand->GetID() : 0);
		return 1;
	}

	ALUA(building_get_land_info)
	{
		int price = 1000000000;
		int owner = 1000000000;
		int level_limit = 1000000000;

		if (lua_isnumber(L, 1))
		{
			using namespace building;

			CLand * pkLand = CManager::Instance().FindLand((uint32_t) lua_tonumber(L, 1));

			if (pkLand)
			{
				const TLand & t = pkLand->GetData();

				price = t.dwPrice;
				owner = t.dwGuildID;
				level_limit = t.bGuildLevelLimit;
			}
		}
		else
			sys_err("invalid argument");

		lua_pushnumber(L, price);
		lua_pushnumber(L, owner);
		lua_pushnumber(L, level_limit);
		return 3;
	}

	ALUA(building_set_land_owner)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("invalid argument");
			return 0;
		}

		using namespace building;

		CLand * pkLand = CManager::Instance().FindLand((uint32_t) lua_tonumber(L, 1));

		if (pkLand)
		{
			if (pkLand->GetData().dwGuildID == 0)
				pkLand->SetOwner((uint32_t)lua_tonumber(L, 2));
		}

#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDBANK_EXTENDED_LOGS)
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		auto pmsg(DBManager::Instance().DirectQuery("INSERT INTO log.guildstorage (guild_id, time, playerid, chrname, itemvnum, itemname, iteminout, itemcount, datatype) VALUES(%u, NOW(), %u, '%s', %d, '%s', %d, %u, %d)",
			pkLand->GetData().dwGuildID, ch->GetPlayerID(), ch->GetName(), 0, "Object", GUILD_GOLD_TYPE_BUY_GUILD_LAND, pkLand->GetData().dwPrice, 1));
#endif

		return 0;
	}

	ALUA(building_has_land)
	{
		using namespace building;

		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			lua_pushboolean(L, true);
			return 1;
		}

		/*
		if (CManager::Instance().FindLandByGuild((uint32_t) lua_tonumber(L, 1)))
			lua_pushboolean(L, true);
		else
			lua_pushboolean(L, false);
		*/

		auto pmsg(DBManager::Instance().DirectQuery("SELECT COUNT(*) FROM land%s WHERE guild_id = %d", get_table_postfix(), (uint32_t)lua_tonumber(L,1)));

		if ( pmsg->Get()->uiNumRows > 0 )
		{
			MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);

			int	count = 0;
			str_to_number(count, row[0]);

			if (count == 0)
			{
				lua_pushboolean(L, false);
			}
			else
			{
				lua_pushboolean(L, true);
			}
		}
		else
		{
			lua_pushboolean(L, true);
		}

		return 1;
	}

	ALUA(building_reconstruct)
	{
		using namespace building;

		uint32_t dwNewBuilding = (uint32_t)lua_tonumber(L, 1);

		CQuestManager& q = CQuestManager::Instance();

		LPCHARACTER npc = q.GetCurrentNPCCharacterPtr();
		if (!npc)
			return 0;

		CGuild* pGuild = npc->GetGuild();
		if (!pGuild)
			return 0;

		CLand* pLand = CManager::Instance().FindLandByGuild(pGuild->GetID());
		if (!pLand)
			return 0;

		LPOBJECT pObject = pLand->FindObjectByNPC(npc);
		if (!pObject)
			return 0;

		pObject->Reconstruct(dwNewBuilding);

#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDBANK_EXTENDED_LOGS)
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		int price = pObject->GetPrice();

		const TObjectProto* t = CManager::Instance().GetObjectProto(dwNewBuilding);

		//Guild-Bank
		if (dwNewBuilding == 14072)
			LogManager::Instance().GuildLog(ch, pGuild->GetID(), dwNewBuilding, "Object", GUILD_GOLD_TYPE_UPGRADE_GUILDBANK, price, 1);	//Bank-Ausbau [Stage 2]
		else if (dwNewBuilding == 14073)	
			LogManager::Instance().GuildLog(ch, pGuild->GetID(), dwNewBuilding, "Object", GUILD_GOLD_TYPE_UPGRADE_GUILDBANK, price, 1);	//Bank-Ausbau [Stage 3]

		//Altar
		if (dwNewBuilding == 14062)
			LogManager::Instance().GuildLog(ch, pGuild->GetID(), dwNewBuilding, "Object", GUILD_GOLD_TYPE_UPGRADE_ALTEROFPOWER, price, 1);	//Altar-Ausbau [Stage 2]
		else if (dwNewBuilding == 14063)
			LogManager::Instance().GuildLog(ch, pGuild->GetID(), dwNewBuilding, "Object", GUILD_GOLD_TYPE_UPGRADE_ALTEROFPOWER, price, 1);	//Altar-Ausbau [Stage 3]

		//Ofen
		if (t->dwGroupVnum == 3)
			LogManager::Instance().GuildLog(ch, pGuild->GetID(), dwNewBuilding, "Object", GUILD_GOLD_TYPE_CHANGE_SMELTER, price, 1);	//Ofen-Umbau

		sys_log(0, "building_reconstruct");
#endif

		return 0;
	}

	void RegisterBuildingFunctionTable()
	{
		luaL_reg functions[] =
		{
			{ "get_land_id",	building_get_land_id	},
			{ "get_land_info",	building_get_land_info	},
			{ "set_land_owner",	building_set_land_owner	},
			{ "has_land",	building_has_land	},
			{ "reconstruct",	building_reconstruct	},
			{ nullptr,		nullptr			}
		};

		CQuestManager::Instance().AddLuaFunctionTable("building", functions);
	}
};
