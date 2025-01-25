#ifndef __INC_METIN_II_GAME_CMD_H__
#define __INC_METIN_II_GAME_CMD_H__

#define ACMD(name)  void (name)(LPCHARACTER ch, const char *argument, int cmd, int subcmd)
#define CMD_NAME(name) cmd_info[cmd].command

struct command_info
{
	const char * command;
	void (*command_pointer) (LPCHARACTER ch, const char *argument, int cmd, int subcmd);
	int subcmd;
	int minimum_position;
	int gm_level;
};

extern struct command_info cmd_info[];

extern void interpret_command(LPCHARACTER ch, const char * argument, size_t len);
extern void interpreter_set_privilege(const char * cmd, int lvl);

enum SCMD_ACTION
{
	SCMD_SLAP,
	SCMD_KISS,
	SCMD_FRENCH_KISS,
	SCMD_HUG,
	SCMD_LONG_HUG,
	SCMD_SHOLDER,
	SCMD_FOLD_ARM
};

enum SCMD_CMD
{
	SCMD_LOGOUT,
	SCMD_QUIT,
	SCMD_PHASE_SELECT,
	SCMD_SHUTDOWN,
};

enum SCMD_RESTART
{
	SCMD_RESTART_TOWN,
	SCMD_RESTART_HERE
#ifdef ENABLE_BATTLE_FIELD
	, SCMD_RESTART_BATTLE
#endif
#ifdef ENABLE_AUTO_RESTART_EVENT
	, SCMD_RESTART_AUTOHUNT
#endif
};

enum SCMD_XMAS
{
	SCMD_XMAS_BOOM,
	SCMD_XMAS_SNOW,
	SCMD_XMAS_SANTA,
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	SCMD_XMAS_SOUL,
#endif
};

extern void Shutdown(int iSec);
extern void SendLog(const char * c_pszBuf);

// Notice
extern void SendNotice(const char* c_pszBuf, bool bBigFont = false);

// Notice Map
extern void SendNoticeMap(const char* c_pszBuf, int nMapIndex, bool bBigFont);

// Notice OX
#ifdef ENABLE_OX_RENEWAL
extern void SendNoticeOxMap(const char* c_pszBuf, int nMapIndex);
#endif

// Broadcast
extern void BroadcastNotice(const char* c_pszBuf, bool bBigFont = false);

// Notice Monarch
extern void SendMonarchNotice(uint8_t bEmpire, const char * c_pszBuf);

// BigNotice
extern void SendBigNotice(const char * c_pszBuf, int mapIndex);


// System Command + Notifications
#ifdef ENABLE_BATTLE_FIELD
extern void SendCommand(const char* c_pszBuf);
extern void BroadcastCommand(const char* c_pszBuf, ...);
#endif
#ifdef ENABLE_RANKING_SYSTEM
extern void LoadRanking(uint8_t bCategory);
#endif
#ifdef ENABLE_EVENT_MANAGER
extern void SendEventBeginNotification();
extern void SendEventEndNotification();

extern void BroadcastEventReload();
extern void ReloadEvent();
#endif

// LUA_ADD_BGM_INFO
void CHARACTER_SetBGMVolumeEnable() noexcept;
void CHARACTER_AddBGMInfo(unsigned mapIndex, const char* name, float vol);
// END_OF_LUA_ADD_BGM_INFO

// LUA_ADD_GOTO_INFO
extern void CHARACTER_AddGotoInfo(const std::string& c_st_name, uint8_t empire, int mapIndex, uint32_t x, uint32_t y);
// END_OF_LUA_ADD_GOTO_INFO

#endif
