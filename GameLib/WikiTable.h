#pragma once
#include "../UserInterface/Locale_inc.h"

#ifdef ENABLE_INGAME_WIKI
#ifndef __INC_IN_GAME_WIKI_H__
#define __INC_IN_GAME_WIKI_H__

#pragma pack(1)

enum eWikiData
{
	WIKI_MAX_REFINE_COUNT = 9,
	WIKI_REFINE_MATERIAL_MAX = 5,
};

namespace NWikiData
{
	typedef struct SRefineMaterialNew
	{
		uint32_t vnum;
		int count;
	} TRefineMaterialNew;

	typedef struct SWikiChestInfo
	{
		uint32_t vnum;
		int count;
	} TWikiChestInfo;

	typedef struct SWikiMobDropInfo
	{
		uint32_t vnum;
		int count;
	} TWikiMobDropInfo;

	typedef struct SWikiRefineInfo
	{
		int index;
		TRefineMaterialNew materials[WIKI_REFINE_MATERIAL_MAX];
		uint32_t mat_count;
		int price;
	} TWikiRefineInfo;

	typedef struct SWikiItemOriginInfo
	{
		uint32_t vnum;
		bool is_mob;
	} TWikiItemOriginInfo;

	typedef struct SWikiInfoTable
	{
		bool is_common;
		int refine_infos_count;
		int chest_info_count;
		uint32_t origin_vnum;
	} TWikiInfoTable;
}

#pragma pack()
#endif // __INC_IN_GAME_WIKI_H__

#endif
