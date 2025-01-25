#pragma once

#pragma warning(disable : 4702)
#pragma warning(disable : 4100)
#pragma warning(disable : 4201)
#pragma warning(disable : 4511)
#pragma warning(disable : 4663)
#pragma warning(disable : 4018)
#pragma warning(disable : 4245)
// #pragma warning(disable : 4099) // pdb not found

#if _MSC_VER >= 1400
//if don't use below, time_t is 64bit
#	define _USE_32BIT_TIME_T
#endif
#include <iterator>
#include "../EterLib/StdAfx.h"
#include "../EterPythonLib/StdAfx.h"
#include "../GameLib/StdAfx.h"
#include "../ScriptLib/StdAfx.h"
#include "../MilesLib/Stdafx.h"
#include "../EffectLib/StdAfx.h"
#include "../PRTerrainLib/StdAfx.h"
#include "../SpeedTreeLib/StdAfx.h"

#ifndef __D3DRM_H__
#	define __D3DRM_H__
#endif

#include <dshow.h>
//#include <d3d8/qedit.h>
#include <d3d9/qedit.h>

#include "Locale.h"

#include "GameType.h"
extern uint32_t __DEFAULT_CODE_PAGE__;

#define APP_NAME "Metin 2"

enum
{
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
	POINT_MAX_NUM = 500,
#else
	POINT_MAX_NUM = 255,
#endif
	CHARACTER_NAME_MAX_LEN = 48,	//@custom003
	PLAYER_NAME_MAX_LEN = 12,
#ifdef ENABLE_GROWTH_PET_SYSTEM
	GROWTH_EVO_NAME_LENGTH = 20,
#endif
};

void initapp();
void initime();
void initsystemSetting();
void initchr();
void initchrmgr();
void initChat();
void initTextTail();
void initime();
void initItem();
void initNonPlayer();
void initnet();
void initPlayer();
void initServerStateChecker();
#ifdef ENABLE_RENDER_TARGET
void initRenderTarget();
#endif
void initTrade();
void initMiniMap();
void initProfiler();
void initEvent();
void initeffect();
void initsnd();
#ifdef ENABLE_EVENT_MANAGER
void initeventmgr() noexcept;
#endif
void initBackground();
void initwndMgr();
void initshop();
void initpack();
void initskill();
void initfly();
void initquest();
void initsafebox();
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
void initguildbank();
#endif
void initguild();
void initMessenger();
#ifdef ENABLE_SWITCHBOT
void initSwitchbot();
#endif
#ifdef ENABLE_CUBE_RENEWAL
void intcuberenewal();
#endif
#ifdef ENABLE_RANKING_SYSTEM
void initRanking() noexcept;
#endif
#ifdef ENABLE_INGAME_WIKI
void initWiki();
#endif
#ifdef ENABLE_TICKET_SYSTEM
void initTicket();
#endif
#ifdef ENABLE_MAILBOX
void initmailbox();
#endif
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
void initDungeonInfo();
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
void InitAchievementModule();
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
void initBiologManager() noexcept;
#endif

extern const std::string & ApplicationStringTable_GetString(uint32_t dwID);
extern const std::string & ApplicationStringTable_GetString(uint32_t dwID, LPCSTR szKey);

extern const char * ApplicationStringTable_GetStringz(uint32_t dwID);
void PythonRegisterCommandLine(const char * lpCmdLine);
bool PythonRun(const char * lpCmdLine, CPythonLauncher & pyLauncher);
extern const char * ApplicationStringTable_GetStringz(uint32_t dwID, LPCSTR szKey);

extern void ApplicationSetErrorString(const char * szErrorString) noexcept;
