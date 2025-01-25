#ifndef __HEADER_QUEST_LUA__
#define __HEADER_QUEST_LUA__

#include "quest.h"
#include "buffer_manager.h"

extern int test_server;
extern int speed_server;

namespace quest
{
	extern void RegisterPCFunctionTable();
	extern void RegisterNPCFunctionTable();
	extern void RegisterTargetFunctionTable();
	extern void RegisterAffectFunctionTable();
	extern void RegisterBuildingFunctionTable();
	extern void RegisterMarriageFunctionTable();
	extern void RegisterITEMFunctionTable();
	extern void RegisterDungeonFunctionTable();
	extern void RegisterQuestFunctionTable();
	extern void RegisterPartyFunctionTable();
	extern void RegisterHorseFunctionTable();
	extern void RegisterPetFunctionTable();
	extern void RegisterGuildFunctionTable();
	extern void RegisterGameFunctionTable();
	extern void RegisterArenaFunctionTable();
	extern void RegisterGlobalFunctionTable(lua_State* L);
	extern void RegisterForkedFunctionTable();
	extern void RegisterMonarchFunctionTable();
	extern void RegisterOXEventFunctionTable();
	extern void RegisterMgmtFunctionTable();
	extern void RegisterBattleArenaFunctionTable();
	extern void RegisterDanceEventFunctionTable();
	extern void RegisterDragonLairFunctionTable();
	extern void RegisterSpeedServerFunctionTable();
	extern void RegisterDragonSoulFunctionTable();
#ifdef ENABLE_QUEST_DND_EVENT
	extern void RegisterDNDFunctionTable();
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	extern void RegisterMeleyLairFunctionTable();
#endif
#ifdef ENABLE_12ZI
	extern void RegisterZodiacTempleFunctionTable();
#endif
#ifdef ENABLE_ATTR_6TH_7TH
	extern void RegisterAttr67FunctionTable();
#endif
#ifdef ENABLE_QUEEN_NETHIS
	extern void RegisterSnakeLairFunctionTable();
#endif
#ifdef ENABLE_WHITE_DRAGON
	extern void RegisterWhiteLairFunctionTable();
#endif

	extern void combine_lua_string(lua_State* L, std::ostringstream& s);

#ifdef ENABLE_DUNGEON_INFO_SYSTEM
	struct FUpdateDungeonRanking
	{
		std::string m_strQuestName;
		FUpdateDungeonRanking(std::string strQuestName) : m_strQuestName(strQuestName) {}
		void operator () (LPCHARACTER ch);
	};
#endif

	struct FSetWarpLocation
	{
		long map_index;
		long x;
		long y;

		FSetWarpLocation(long _map_index, long _x, long _y) :
			map_index(_map_index), x(_x), y(_y)
		{}
		void operator () (LPCHARACTER ch);
	};

	struct FDungeonGiveItemAll
	{
		uint32_t vnum;
		int count;
		void operator () (LPCHARACTER ch);
	};

	struct FSetQuestFlag
	{
		std::string flagname;
		int value = 0;

		void operator () (LPCHARACTER ch);
	};

	struct FPartyCheckFlagLt
	{
		std::string flagname;
		int value = 0;

		bool operator () (LPCHARACTER ch);
	};

	struct FPartyChat
	{
		int iChatType;
		const char* str;

		FPartyChat(int ChatType, const char* str);
		void operator() (LPCHARACTER ch);
	};

	struct FPartyClearReady
	{
		void operator() (LPCHARACTER ch);
	};

	struct FSendPacket
	{
		TEMP_BUFFER buf;

		void operator() (LPENTITY ent);
	};

#ifdef ENABLE_NEWSTUFF
	struct FSendChatPacket
	{
		uint32_t m_chat_type;
		std::string m_text;

		FSendChatPacket(uint32_t chat_type, std::string text) : m_chat_type(chat_type), m_text(text) {}
		void operator() (LPENTITY ent);
	};
#endif

	struct FSendPacketToEmpire
	{
		TEMP_BUFFER buf;
		uint8_t bEmpire;

		void operator() (LPENTITY ent);
	};

	struct FWarpEmpire
	{
		uint8_t m_bEmpire;
		long m_lMapIndexTo;
		long m_x;
		long m_y;

		void operator() (LPENTITY ent);
	};

	EVENTINFO(warp_all_to_map_my_empire_event_info)
	{
		uint8_t m_bEmpire;
		long m_lMapIndexFrom;
		long m_lMapIndexTo;
		long m_x;
		long m_y;

		warp_all_to_map_my_empire_event_info()
			: m_bEmpire(0)
			, m_lMapIndexFrom(0)
			, m_lMapIndexTo(0)
			, m_x(0)
			, m_y(0)
		{
		}
	};

	EVENTFUNC(warp_all_to_map_my_empire_event);

	struct FBuildLuaGuildWarList
	{
		lua_State* L;
		int m_count;

		FBuildLuaGuildWarList(lua_State* L);
		void operator() (uint32_t g1, uint32_t g2);
	};

#ifdef ENABLE_BATTLE_PASS_SYSTEM
	struct FDungeonUpdateAllBattlepassProcess
	{
		int dungeon_index;

		void operator () (LPCHARACTER ch);
	};
#endif
}
#endif /*__HEADER_QUEST_LUA__*/
