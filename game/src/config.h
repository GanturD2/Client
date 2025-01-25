
#ifndef __INC_METIN_II_GAME_CONFIG_H__
#define __INC_METIN_II_GAME_CONFIG_H__

enum
{
	ADDRESS_MAX_LEN = 15
};

enum ItemDestroyTime {ITEM_DESTROY_TIME_AUTOGIVE, ITEM_DESTROY_TIME_DROPGOLD, ITEM_DESTROY_TIME_DROPITEM, ITEM_DESTROY_TIME_MAX};

void config_init(const std::string& st_localeServiceName); // default "" is CONFIG

extern char sql_addr[256];

extern uint16_t mother_port;
extern uint16_t p2p_port;

extern char db_addr[ADDRESS_MAX_LEN + 1];
extern uint16_t db_port;

extern int passes_per_sec;
extern int save_event_second_cycle;
extern int ping_event_second_cycle;
extern int test_server;
extern bool	guild_mark_server;
extern uint8_t guild_mark_min_level;
extern bool	distribution_test_server;

extern bool	g_bNoMoreClient;
extern bool	g_bNoRegen;

// #ifdef ENABLE_NEWSTUFF
extern bool	g_bEmpireShopPriceTripleDisable;
extern bool g_bShoutAddonEnable;
extern bool g_bGlobalShoutEnable;
extern bool g_bDisablePrismNeed;
extern bool g_bDisableEmotionMask;
extern uint8_t g_bItemCountLimit;
extern uint32_t g_dwItemBonusChangeTime;
extern bool	g_bAllMountAttack;
extern bool	g_bEnableBootaryCheck;
extern bool	g_bGMHostCheck;
extern bool	g_bGuildInviteLimit;
extern bool	g_bGuildInfiniteMembers;
extern bool	g_bChinaIntoxicationCheck;
extern bool	g_bEnableSpeedHackCrash;
extern int g_iStatusPointGetLevelLimit;
extern int g_iStatusPointSetMaxValue;
#ifdef ENABLE_YOHARA_SYSTEM
extern int g_iConquerorPointGetLevelLimit;
extern int g_iConquerorPointSetMaxValue;
extern int gConquerorMaxLevel;
#endif
extern int g_iShoutLimitLevel;
extern uint32_t g_dwSkillBookNextReadMin;
extern uint32_t g_dwSkillBookNextReadMax;
#ifdef ENABLE_PROXY_IP
extern std::string g_stProxyIP;
#endif
// extern int g_iShoutLimitTime;
extern int g_iDbLogLevel;
extern int g_iSysLogLevel;
extern int g_aiItemDestroyTime[ITEM_DESTROY_TIME_MAX];
extern bool	g_bDisableEmpireLanguageCheck;
// #endif

//Custom
extern int g_npcGroupRespawnRange[];
extern bool emotion_without_mask;
extern bool emotion_same_gender;
extern bool g_bBlockPositionsInDuell;
extern bool	g_bShoutFrozen;

extern bool attr_always_add;	//@custom029
extern bool attr_always_5_add;	//@custom030
extern bool attr_rare_enable;	//@custom026
extern bool belt_allow_all_items;	//@custom009
extern bool ban_force_reason;
extern bool trade_effect;
extern int trade_effect_exchange_threshold;
extern int trade_effect_shop_threshold;
extern bool view_equip_for_all;
extern int view_equip_sp_cost;
extern bool attribute_double_change;
extern int ride_seconds_to_ride_again;
extern bool	g_bDisableMovspeedHacklog;
extern double g_dMovspeedHackThreshold;
extern bool poison_draw_monsters_aggro;

extern std::set<int> s_mtr_map_disable;
extern bool mtr_map_disable_find(int index);
extern std::set<int> s_closed_empire_reg;
// END_Custom

extern bool	g_bTrafficProfileOn;

extern uint8_t	g_bChannel;

extern bool	map_allow_find(int index);
extern void	map_allow_copy(long * pl, int size);
extern bool	no_wander;

extern int		g_iUserLimit;
extern time_t	g_global_time;

const char *	get_table_postfix();

extern std::string	g_stHostname;
extern std::string	g_stLocale;
extern std::string	g_stLocaleFilename;

extern char		g_szPublicIP[16];
extern char		g_szInternalIP[16];

extern int (*is_twobyte) (const char * str);
extern int (*check_name) (const char * str);

extern bool		g_bSkillDisable;

extern int		g_iFullUserCount;
extern int		g_iBusyUserCount;
extern void		LoadStateUserCount();

extern bool	g_bEmpireWhisper;

extern uint8_t	g_bAuthServer;

extern uint8_t	PK_PROTECT_LEVEL;

extern void	LoadValidCRCList();
extern bool	IsValidProcessCRC(uint32_t dwCRC);
extern bool	IsValidFileCRC(uint32_t dwCRC);

extern std::string	g_stAuthMasterIP;
extern uint16_t		g_wAuthMasterPort;

extern std::string	g_stClientVersion;
extern bool		g_bCheckClientVersion;
extern void		CheckClientVersion();

extern std::string	g_stQuestDir;
//extern std::string	g_stQuestObjectDir;
extern std::set<std::string> g_setQuestObjectDir;


extern std::vector<std::string>	g_stAdminPageIP;
extern std::string	g_stAdminPagePassword;

extern int	SPEEDHACK_LIMIT_COUNT;
extern int 	SPEEDHACK_LIMIT_BONUS;

extern int g_iSyncHackLimitCount;

extern int g_server_id;
extern std::string g_strWebMallURL;

extern int VIEW_RANGE;
extern int VIEW_BONUS_RANGE;

extern bool g_bCheckMultiHack;
extern bool g_protectNormalPlayer;
extern bool g_noticeBattleZone;

extern uint32_t g_GoldDropTimeLimitValue;
// #ifdef ENABLE_NEWSTUFF
extern uint32_t g_ItemDropTimeLimitValue;
extern uint32_t g_BoxUseTimeLimitValue;
extern uint32_t g_BuySellTimeLimitValue;
extern bool g_NoDropMetinStone;
extern bool g_NoMountAtGuildWar;
extern bool g_NoPotionsOnPVP;
// #endif

extern int gPlayerMaxLevel;
extern int gShutdownAge;
extern int gShutdownEnable;

extern bool gHackCheckEnable;

extern bool g_BlockCharCreation;

// missing begin
extern int speed_server;
extern std::string g_stBlockDate;

extern int g_iSpamBlockMaxLevel;
extern unsigned int g_uiSpamBlockScore;
extern unsigned int g_uiSpamBlockDuration;
extern unsigned int g_uiSpamReloadCycle;

extern void map_allow_log();
// missing end

#endif /* __INC_METIN_II_GAME_CONFIG_H__ */
