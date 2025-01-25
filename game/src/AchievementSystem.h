#pragma once
#include "stdafx.h"

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
#include "../../common/tables.h"
#include "item.h"

namespace achievements
{
	const char CONFIG_FILE[] = "achievements.xml";

	enum EAchievementStatus : int8_t
	{
		LOCKED,
		UNLOCKED,
		FINISHED
	};

	enum EAchievementSubHeadersClient : int8_t
	{
		HEADER_GC_INITIAL,
		HEADER_GC_UPDATE,
		HEADER_GC_UPDATE_POINTS,
		HEADER_GC_RANKING,

		HEADER_GC_NOTIFICATION,
		HEADER_GC_UPDATE_TITLES,
		HEADER_GC_RELOAD_TITLES,

	};

	enum EAchievementSubHeadersGame : int8_t
	{
		HEADER_CG_SELECT_TITLE,
		HEADER_CG_OPEN_SHOP,
		HEADER_CG_OPEN_RANKING,

	};

	enum ETaskTypes : int8_t
	{
		TYPE_NONE,

		TYPE_KILL,

		TYPE_REACH_LEVEL,
		TYPE_REACH_PLAYTIME,
		TYPE_SUMMON_PET,
		TYPE_SUMMON_MOUNT,
		TYPE_ACTIVATE_TOGGLE,
		TYPE_REACH_SPEED,

		TYPE_FISH,
		TYPE_BURN,
		TYPE_USE_BURN,

		TYPE_WIN_WARS,
		TYPE_DIE,
		TYPE_DEAL_DAMAGE,
		TYPE_GET_DAMAGAE,

		TYPE_COLLECT,
		TYPE_COLLECT_ALIGNMENT,
		TYPE_COLLECT_GOLD,

		TYPE_EXPLORE,

		TYPE_SPEND_SEARCH_SHOP,
		TYPE_SPEND_SHOP,
		TYPE_SPEND_UPGRADE,

		TYPE_WITHDRAW,
		TYPE_TRADE,
		TYPE_UPGRADE_15,
		TYPE_UPGRADE_9,

		TYPE_ADD_FRIEND,
		TYPE_JOIN_GUILD,
		TYPE_WHISPER,
		TYPE_SHOUTS,
		TYPE_PARTY,

		TYPE_SKILL,

		TYPE_DUNGEON,

		TYPE_MAX_NUM,

	};

	enum ERestrictionTypes : int8_t
	{
		RESTRICTION_NONE,

		RESTRICTION_VNUM,
		RESTRICTION_MAP_INDEX,
		RESTRICTION_MAXED_BONUSES,
		RESTRICTION_BOSS,
		RESTRICTION_STONES,
		RESTRICTION_MONSTERS,

		RESTRICTION_PLAYER,
		RESTRICTION_EMPIRE,
		RESTRICTION_RACE,
		RESTRICTION_DUEL,
		RESTRICTION_DUNGEON,

		RESTRICTION_PK,

		RESTRICTION_TIME,
		RESTRICTION_PARTY,

		RESTRICTION_SKILL,

		RESTRICTION_DUEL_ROW,

		RESTRICTIONS_MAX_NUM
	};

	enum ERewardTypes : int8_t
	{
		REWARD_NONE,

		REWARD_ITEM,
		REWARD_GOLD,
		REWARD_TITLE,
		REWARD_ACHIEVEMENT_POINTS,

		REWARDS_MAX_NUM
	};

	typedef struct SAchievement
	{
		int8_t status;
		uint8_t percentage;
		uint32_t finish_timestamp;
		SAchievement() : status(LOCKED), percentage(0), finish_timestamp(0) {}
	} TAchievement;

	typedef struct STask
	{
		uint8_t type;
		uint64_t max_value;
		uint32_t min_level;
		uint32_t max_level;

		// restrictions
		uint32_t restrictions[RESTRICTIONS_MAX_NUM + 1];
		STask() : type(0), max_value(0), min_level(0), max_level(0) {
			memset(&restrictions, 0, sizeof(restrictions));
		}
	} TTask;
	typedef std::map<uint32_t, TTask> TTaskList;

	typedef struct SReward
	{
		uint8_t type;
		uint32_t value;
		uint32_t count;
		SReward() : type(0), value(0), count(0) {}
	} TReward;
	typedef std::vector<TReward> TRewardList;

	typedef struct SAchievementData
	{
		TTaskList tasks;
		TRewardList rewards;
		uint64_t max_value;
	} TAchievementData;
	typedef std::map<uint32_t, TAchievementData> TAchievementMap;

	typedef std::map<uint32_t, std::pair<uint8_t, uint32_t>> TTitleMap;

#pragma pack(1)
	struct GC_packet
	{
		uint8_t header;
		uint16_t size;
		uint8_t subHeader;
	};

	struct GC_load
	{
		uint32_t points;
		uint8_t achievements;
		uint8_t titles;
	};

	struct GC_Update
	{
		uint32_t id;
		TAchievement achievement;
	};

	struct GC_ranking
	{
		uint8_t level;
		char name[CHARACTER_NAME_MAX_LEN + 1];
		char guild[GUILD_NAME_MAX_LEN + 1];
		uint32_t progress;
	};

	struct CG_packet
	{
		uint8_t header;
		uint8_t subHeader;
		uint32_t dummy;
	};
#pragma pack()
};

class CAchievementSystem : public singleton<CAchievementSystem> {
public:
	CAchievementSystem();
	~CAchievementSystem();

public:
	void Initialize();

	void ProcessDBPackets(LPDESC desc, const char* c_pData);
	void ProcessClientPackets(LPCHARACTER player, const char* c_pData);


	void OnLogin(LPCHARACTER player,
		TAchievementsMap& achievement,
		uint32_t points,
		uint32_t title);

	void OnLogout(LPCHARACTER player);

	void SelectTitle(LPCHARACTER player, uint32_t titleID);

public:
	void OnKill(LPCHARACTER player, LPCHARACTER victim, bool isDuel = false);
	void OnCharacterUpdate(LPCHARACTER player);
	void OnSummon(LPCHARACTER player, uint8_t type, uint32_t value, uint32_t time = 0, bool maxed = false);
	void OnToggle(LPCHARACTER player, uint32_t vnum);
	void OnFishItem(LPCHARACTER player, uint8_t type, uint32_t vnum, uint32_t count = 1);
	void OnWinGuildWar(LPCHARACTER player);
	void DamageDealt(LPCHARACTER player, LPCHARACTER victim, uint32_t skill, uint32_t damage);
	void Collect(LPCHARACTER player,
		uint8_t type,
		uint32_t value,
		uint32_t count);
	void OnGoldChange(LPCHARACTER player, uint8_t type, uint32_t value);
	void OnTrade(LPCHARACTER player);
	void OnUpgrade(LPCHARACTER player, uint32_t vnum, bool _to15);
	void OnSocial(LPCHARACTER player, uint8_t type);
	void OnMasterSkill(LPCHARACTER player, uint32_t skill, uint32_t value);
	void SendTasksInfo(LPCHARACTER player, uint32_t achievement);
	void OnFinishDungeon(LPCHARACTER player,
		uint32_t dungeon_id,
		uint32_t time,
		uint32_t members);
	void OnVisitMap(LPCHARACTER player);
	void FinishAchievement(LPCHARACTER player, uint32_t achievement_id);
	void FinishAchievementTask(LPCHARACTER player,
		uint32_t achievement_id,
		uint32_t task_id);

	void UpdateAchievementPoints(LPCHARACTER player);
	void ChangeAchievementPoints(LPCHARACTER player, int32_t value);
	void ComputePoints(LPCHARACTER player);
	bool IsAchievementFinished(LPCHARACTER player, uint32_t achievement_id);
	uint16_t GetAchievements() const { return _achievements.size(); }
private:
	bool GetAchievementInfo(uint32_t id, achievements::TAchievementData** achievement);

	void RefreshAchievementTitles(LPCHARACTER player);

	bool IsTaskFinished(LPCHARACTER player,
		const uint32_t task_value,
		const achievements::TTask& category_task);

	bool IsTaskFinished(LPCHARACTER player,
		const uint32_t achievement_id,
		const uint8_t task_id);
	uint32_t GetTotalAvailableTasks(LPCHARACTER player, uint32_t achievement_id);
	uint32_t GetAchievementProgress(LPCHARACTER player, uint32_t achievement_id);
	uint32_t GetAchievementProgressFast(LPCHARACTER player, uint32_t achievement_id);
	uint32_t GetTaskProgress(LPCHARACTER player,
		const uint64_t task_value,
		const achievements::TTask category_task);
	void UpdatePacket(LPCHARACTER player, uint32_t achievement_id);
	void UpdateTitleList(LPCHARACTER player);

	void RewardPlayer(LPCHARACTER player, uint32_t achievement_id);

	bool GetTitleInfo(uint32_t title_id, uint8_t& type, uint32_t& value);

private:
	achievements::TAchievementMap _achievements;
	achievements::TTitleMap _titles;
};
#endif
