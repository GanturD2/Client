//#define __FISHING_MAIN__
#include "stdafx.h"
#include "constants.h"
#include "fishing.h"
#include "locale_service.h"

#ifndef __FISHING_MAIN__
#include "item_manager.h"

#include "config.h"
#include "packet.h"

#include "sectree_manager.h"
#include "char.h"
#include "char_manager.h"

#include "log.h"

#include "questmanager.h"
#include "buffer_manager.h"
#include "desc_client.h"
#include "locale_service.h"

#include "affect.h"
#include "unique_item.h"
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
#	include "GrowthPetSystem.h"
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	#include "AchievementSystem.h"
#endif

namespace fishing
{
	int aFishingTime[FISHING_TIME_COUNT][MAX_FISHING_TIME_COUNT] =
	{
		{   0,   0,   0,   0,   0,   2,   4,   8,  12,  16,  20,  22,  25,  30,  50,  80,  50,  30,  25,  22,  20,  16,  12,   8,   4,   2,   0,   0,   0,   0,   0 },
		{   0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   4,   8,  12,  16,  20,  22,  25,  30,  50,  80,  50,  30,  25,  22,  20 },
		{  20,  22,  25,  30,  50,  80,  50,  30,  25,  22,  20,  16,  12,   8,   4,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 },
		{ 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
		{  20,  20,  20,  20,  20,  22,  24,  28,  32,  36,  40,  42,  45,  50,  70, 100,  70,  50,  45,  42,  40,  36,  32,  28,  24,  22,  20,  20,  20,  20,  20 },
	};

#ifdef ENABLE_FISHING_RENEWAL
	const int aFishFirstTableNormal[5] = { 27803, 27806, 27816, 27807, 27818 };
	const int aFishFirstTableRare[7] = { 27804, 27811, 27810, 27809, 27814, 27812, 27808 };
	const int aFishSecondTableNormal[5] = { 27805, 27822, 27823, 27824, 27825 };
	const int aFishSecondTableRare[10] = { 27826, 27827, 27813, 27815, 27819, 27820, 27821 };
#endif

	struct SFishInfo
	{
		char name[FISH_NAME_MAX_LEN];

		uint32_t vnum;
		uint32_t dead_vnum;
		uint32_t grill_vnum;

		int prob[MAX_PROB];
		int difficulty;

		int time_type;
		int length_range[3]; // MIN MAX EXTRA_MAX : 99% MIN~MAX, 1% MAX~EXTRA_MAX

		int used_table[NUM_USE_RESULT_COUNT];
		// 6000 2000 1000 500 300 100 50 30 10 5 4 1
	};

	bool operator < ( const SFishInfo& lhs, const SFishInfo& rhs )
	{
		return lhs.vnum < rhs.vnum;
	}

	int g_prob_sum[MAX_PROB];
	int g_prob_accumulate[MAX_PROB][MAX_FISH];

	SFishInfo fish_info[MAX_FISH] = { { "\0", }, };
	/*
	   {
	   { "꽝",		00000, 00000, 00000, {  750, 1750, 2750 },   10, FISHING_LIMIT_NONE,	{  0,   0,   0}, FISHING_TIME_NORMAL, { 0,           },
	   {0, } },
	   { "금반지",	50002, 00000, 00000, {   50,   50,    0 },  200, FISHING_LIMIT_NONE,	{  0,   0,   0}, FISHING_TIME_NORMAL, { 0,           },
	   {0, } },
	   { "피라미",	27802, 00000, 00000, { 2100, 1500,   50 },   10, FISHING_LIMIT_NONE,	{  0,   0,   0}, FISHING_TIME_EASY,   {500, 550, 600},
	   {0, } },
	   { "붕어",	27803, 27833, 27863, { 2100, 1500,  100 },   13, FISHING_LIMIT_NONE,	{  0,   0,   0}, FISHING_TIME_EASY,   {1000,2500,2800},
	   {USED_NONE, USED_SHELLFISH, USED_NONE, USED_NONE, USED_WATER_STONE, USED_TREASURE_MAP, USED_EARTHWARM, USED_NONE,USED_NONE,  USED_NONE } },
	   { "쏘가리",	27804, 27834, 27864, { 1100, 1300,  150 },   16, FISHING_LIMIT_NONE,	{  0,   0,   0}, FISHING_TIME_NORMAL, {2000,3500,3800},
	   {USED_NONE, USED_SHELLFISH, USED_NONE, USED_NONE, USED_WATER_STONE, USED_TREASURE_MAP, USED_EARTHWARM, USED_NONE,USED_NONE,  USED_NONE } },
	   { "월척붕어",	27805, 27835, 27865, { 1100, 1100,  200 },   20, FISHING_LIMIT_NONE,	{  0,   0,   0}, FISHING_TIME_SLOW, {3030,3500,4300},
	   {USED_NONE, USED_SHELLFISH, USED_NONE, USED_NONE, USED_WATER_STONE, USED_TREASURE_MAP, USED_EARTHWARM, USED_NONE,USED_NONE,  USED_NONE } },
	   { "잉어",	27806, 27836, 27866, { 1100,  500,  300 },   26, FISHING_LIMIT_NONE,	{  0,   0,   0}, FISHING_TIME_NORMAL, {4000,6000,10000},
	   {USED_NONE, USED_SHELLFISH, USED_NONE, USED_NONE, USED_WATER_STONE, USED_TREASURE_MAP, USED_EARTHWARM, USED_NONE,USED_NONE,  USED_NONE } },
	   { "연어",	27807, 27837, 27867, { 1100,  450,  400 },   33, FISHING_LIMIT_NONE,	{  0,   0,   0}, FISHING_TIME_NORMAL,{6000,8000,10000},
	   {USED_NONE, USED_SHELLFISH, USED_NONE, USED_NONE, USED_WATER_STONE, USED_TREASURE_MAP, USED_EARTHWARM, USED_NONE,USED_NONE,  USED_NONE } },
	   { "향어",	27808, 27838, 27868, {  200,  400,  500 },   42, FISHING_LIMIT_NONE,	{  0,   0,   0}, FISHING_TIME_NORMAL,{1500,3000,3800},
	   {USED_NONE, USED_SHELLFISH, USED_NONE, USED_NONE, USED_WATER_STONE, USED_TREASURE_MAP, USED_EARTHWARM, USED_NONE,USED_NONE,  USED_NONE } },
	   { "송어",	27809, 27839, 27869, {  200,  300,  700 },   54, FISHING_LIMIT_NONE,	{  0,   0,   0}, FISHING_TIME_NORMAL,{5000,7000,8000},
	   {USED_NONE, USED_SHELLFISH, USED_NONE, USED_NONE, USED_WATER_STONE, USED_TREASURE_MAP, USED_EARTHWARM, USED_NONE,USED_NONE,  USED_NONE } },
	   { "민물장어",	27810, 27840, 27870, {    0,  270, 1000 },   70, FISHING_LIMIT_NONE,	{  0,   0,   0}, FISHING_TIME_NORMAL,{4000,5000,6000},
	   {USED_SHELLFISH, USED_NONE, USED_NONE, USED_WATER_STONE, USED_TREASURE_MAP, USED_NONE, USED_EARTHWARM, USED_NONE,USED_NONE,  USED_NONE } },
	   { "무지개송어",	27811, 27841, 27871, {    0,  200, 1000 },   91, FISHING_LIMIT_APPEAR,	{  0,   0,   0}, FISHING_TIME_NORMAL,{5000,7000,8000},
	   {USED_SHELLFISH, USED_NONE, USED_NONE, USED_WATER_STONE, USED_TREASURE_MAP, USED_NONE, USED_EARTHWARM, USED_NONE,USED_NONE,  USED_NONE } },
	   { "강송어",	27812, 27842, 27872, {    0,  160, 1000 },  118, FISHING_LIMIT_NONE,	{  0,   0,   0}, FISHING_TIME_QUICK,{4000,6000,7000},
	   {USED_SHELLFISH, USED_NONE, USED_NONE, USED_WATER_STONE, USED_TREASURE_MAP, USED_NONE, USED_EARTHWARM, USED_NONE,USED_NONE,  USED_NONE } },
	   { "러드",	27813, 27843, 27873, {    0,  130,  700 },  153, FISHING_LIMIT_NONE,	{  0,   0,   0}, FISHING_TIME_NORMAL,{4000,6000,10000},
	   {USED_SHELLFISH, USED_NONE, USED_NONE, USED_WATER_STONE, USED_TREASURE_MAP, USED_NONE, USED_EARTHWARM, USED_NONE,USED_NONE,  USED_NONE } },
	   { "퍼치",	27814, 27844, 27874, {    0,  100,  400 },  198, FISHING_LIMIT_NONE,	{  0,   0,   0}, FISHING_TIME_NORMAL,{3000,4000,5000},
	   {USED_SHELLFISH, USED_NONE, USED_NONE, USED_WATER_STONE, USED_TREASURE_MAP, USED_NONE, USED_EARTHWARM, USED_NONE,USED_NONE,  USED_NONE } },
	   { "텐치",	27815, 27845, 27875, {    0,   50,  300 },  257, FISHING_LIMIT_NONE,	{  0,   0,   0}, FISHING_TIME_NORMAL,{3500,5500,8000},
	   {USED_SHELLFISH, USED_NONE, USED_NONE, USED_WATER_STONE, USED_TREASURE_MAP, USED_NONE, USED_EARTHWARM, USED_NONE,USED_NONE,  USED_NONE } },
	   { "메기",	27816, 27846, 27876, {    0,   30,  100 },  334, FISHING_LIMIT_NONE,	{  0,   0,   0}, FISHING_TIME_NORMAL,{3000,5000,10000},
	   {USED_SHELLFISH, USED_NONE, USED_NONE, USED_WATER_STONE, USED_TREASURE_MAP, USED_NONE, USED_EARTHWARM, USED_NONE,USED_NONE,  USED_NONE } },
	   { "미꾸라지",	27817, 27847, 27877, {    0,   10,   64 },  434, FISHING_LIMIT_NONE,	{  0,   0,   0}, FISHING_TIME_QUICK,{1800,2200,3000},
	   {USED_SHELLFISH, USED_NONE, USED_NONE, USED_WATER_STONE, USED_TREASURE_MAP, USED_NONE, USED_EARTHWARM, USED_NONE,USED_NONE,  USED_NONE } },
	   { "백련",	27818, 27848, 27878, {    0,    0,   15 },  564, FISHING_LIMIT_NONE,	{  0,   0,   0}, FISHING_TIME_NORMAL,{5000,8000,10000},
	   {USED_SHELLFISH, USED_NONE, USED_WATER_STONE, USED_TREASURE_MAP, USED_NONE, USED_NONE, USED_EARTHWARM, USED_NONE,USED_NONE,  USED_NONE } },
	   { "은어",	27819, 27849, 27879, {    0,    0,    9 },  733, FISHING_LIMIT_NONE,	{  0,   0,   0}, FISHING_TIME_NORMAL,{1500,3000,3800},
	   {USED_SHELLFISH, USED_NONE, USED_WATER_STONE, USED_TREASURE_MAP, USED_NONE, USED_NONE, USED_EARTHWARM, USED_NONE,USED_NONE,  USED_NONE } },
	   { "빙어",	27820, 27850, 27880, {    0,    0,    6 },  952, FISHING_LIMIT_NONE,	{  0,   0,   0}, FISHING_TIME_QUICK,{1500,3000,3800},
	   {USED_SHELLFISH, USED_NONE, USED_WATER_STONE, USED_TREASURE_MAP, USED_NONE, USED_NONE, USED_EARTHWARM, USED_NONE,USED_NONE,  USED_NONE } },
	   { "쉬리",	27821, 27851, 27881, {    0,    0,    3 }, 1237, FISHING_LIMIT_NONE,	{  0,   0,   0}, FISHING_TIME_NORMAL,{1000,1500,2000},
	   {USED_SHELLFISH, USED_NONE, USED_WATER_STONE, USED_TREASURE_MAP, USED_NONE, USED_NONE, USED_EARTHWARM, USED_NONE,USED_NONE,  USED_NONE } },
	   { "비늘잉어",	27822, 27852, 27882, {    0,    0,    2 }, 1608, FISHING_LIMIT_NONE,	{  0,   0,   0}, FISHING_TIME_SLOW,{4000,6000,10000},
	   {USED_SHELLFISH, USED_NONE, USED_WATER_STONE, USED_TREASURE_MAP, USED_NONE, USED_NONE, USED_EARTHWARM, USED_NONE,USED_NONE,  USED_NONE } },
	   { "황금붕어",	27823, 27853, 27883, {    0,    0,    1 }, 2090, FISHING_LIMIT_NONE,	{  0,   0,   0}, FISHING_TIME_SLOW,{1000,3000,3500},
	   {USED_SHELLFISH, USED_NONE, USED_WATER_STONE, USED_TREASURE_MAP, USED_NONE, USED_NONE, USED_EARTHWARM, USED_NONE,USED_NONE,  USED_NONE } },
	   { "탈색약",     70201, 00000, 00000, { 5,    5,  0 },   60, FISHING_LIMIT_NONE,    {  0,   0,   0}, FISHING_TIME_NORMAL, {0,           },
	   {0,	}},
	   { "염색약(흰색)",  70202, 00000, 00000, { 15,  15,  0 },   60, FISHING_LIMIT_NONE,    {  0,   0,   0}, FISHING_TIME_NORMAL, {0,           },
	   {0,	}},
	   { "염색약(금색)",  70203, 00000, 00000, { 15,  15,  0 },   60, FISHING_LIMIT_NONE,    {  0,   0,   0}, FISHING_TIME_NORMAL, {0,           },
	   {0,	}},
	   { "염색약(빨간색)",70204, 00000, 00000, { 15,  15,  0 },   60, FISHING_LIMIT_NONE,    {  0,   0,   0}, FISHING_TIME_NORMAL, {0,           },
	   {0,	}},
	   { "염색약(갈색)",  70205, 00000, 00000, { 15,  15,  0 },   60, FISHING_LIMIT_NONE,    {  0,   0,   0}, FISHING_TIME_NORMAL, {0,           },
	   {0,	}},
	   { "염색약(검은색)",70206, 00000, 00000, { 15,  15,  0 },   60, FISHING_LIMIT_NONE,    {  0,   0,   0}, FISHING_TIME_NORMAL, {0,           },
	   {0,	}},
	   { "은둔자의 망토", 70048, 00000, 00000, {  8,   8,  0 },   60, FISHING_LIMIT_NONE,    {  0,   0,   0}, FISHING_TIME_NORMAL, {0,           },
	   {0,	}},
	   { "행운의 반지",   70049, 00000, 00000, {  8,   8,  0 },   60, FISHING_LIMIT_NONE,    {  0,   0,   0}, FISHING_TIME_NORMAL, {0,           },
	   {0,	}},
	   { "선왕의 증표",   70050, 00000, 00000, {  8,   8,  0 },   60, FISHING_LIMIT_NONE,    {  0,   0,   0}, FISHING_TIME_NORMAL, {0,           },
	   {0,	}},
	   { "선왕의 장갑",   70051, 00000, 00000, {  8,   8,  0 },   60, FISHING_LIMIT_NONE,    {  0,   0,   0}, FISHING_TIME_NORMAL, {0,           },
	   {0,	}},
	   { "금덩어리",	   80008, 00000, 00000, { 20,  20,  0 },  250, FISHING_LIMIT_NONE,    {  0,   0,   0}, FISHING_TIME_SLOW,    {0,           },
	   {0, } },
	{ "은열쇠",	   50009, 00000, 00000, {300, 300, 0, },   70, FISHING_LIMIT_NONE,    { 0, 0, 0}, FISHING_TIME_NORMAL, {0,	}, {0, } },

	{ "금열쇠",	   50008, 00000, 00000, {110, 110, 0, },  100, FISHING_LIMIT_NONE,    { 0, 0, 0}, FISHING_TIME_NORMAL, {0,	}, {0, } },
};
	*/
void Initialize()
{
	SFishInfo fish_info_bak[MAX_FISH];
	thecore_memcpy(fish_info_bak, fish_info, sizeof(fish_info));

	memset(fish_info, 0, sizeof(fish_info));


	// LOCALE_SERVICE
	const int FILE_NAME_LEN = 256;
	char szFishingFileName[FILE_NAME_LEN+1];
	snprintf(szFishingFileName, sizeof(szFishingFileName),
			"%s/fishing.txt", LocaleService_GetBasePath().c_str());
	FILE * fp = fopen(szFishingFileName, "r");
	// END_OF_LOCALE_SERVICE

	if (*fish_info_bak[0].name)
		SendLog("Reloading fish table.");

	if (!fp)
	{
		SendLog("error! cannot open fishing.txt");

		if (*fish_info_bak[0].name)
		{
			thecore_memcpy(fish_info, fish_info_bak, sizeof(fish_info));
			SendLog("  restoring to backup");
		}
		return;
	}

	memset(fish_info, 0, sizeof(fish_info));

	char buf[512];
	int idx = 0;

	while (fgets(buf, 512, fp))
	{
		if (*buf == '#')
			continue;

		char * p = strrchr(buf, '\n');
		*p = '\0';

		const char * start = buf;
		const char * tab = strchr(start, '\t');

		if (!tab)
		{
			printf("Tab error on line: %s\n", buf);
			SendLog("error! parsing fishing.txt");

			if (*fish_info_bak[0].name)
			{
				thecore_memcpy(fish_info, fish_info_bak, sizeof(fish_info));
				SendLog("  restoring to backup");
			}
			break;
		}

		char szCol[256], szCol2[256];
		int iColCount = 0;

		do
		{
			strlcpy(szCol2, start, MIN(sizeof(szCol2), (tab - start) + 1));
			szCol2[tab-start] = '\0';

			trim_and_lower(szCol2, szCol, sizeof(szCol));

			if (!*szCol || *szCol == '\t')
				iColCount++;
			else
			{
				switch (iColCount++)
				{
					case 0: strlcpy(fish_info[idx].name, szCol, sizeof(fish_info[idx].name)); break;
					case 1: str_to_number(fish_info[idx].vnum, szCol); break;
					case 2: str_to_number(fish_info[idx].dead_vnum, szCol); break;
					case 3: str_to_number(fish_info[idx].grill_vnum, szCol); break;
					case 4: str_to_number(fish_info[idx].prob[0], szCol); break;
					case 5: str_to_number(fish_info[idx].prob[1], szCol); break;
					case 6: str_to_number(fish_info[idx].prob[2], szCol); break;
					case 7: str_to_number(fish_info[idx].prob[3], szCol); break;
					case 8: str_to_number(fish_info[idx].difficulty, szCol); break;
					case 9: str_to_number(fish_info[idx].time_type, szCol); break;
					case 10: str_to_number(fish_info[idx].length_range[0], szCol); break;
					case 11: str_to_number(fish_info[idx].length_range[1], szCol); break;
					case 12: str_to_number(fish_info[idx].length_range[2], szCol); break;
					case 13: // 0
					case 14: // 1
					case 15: // 2
					case 16: // 3
					case 17: // 4
					case 18: // 5
					case 19: // 6
					case 20: // 7
					case 21: // 8
					case 22: // 9
							 str_to_number(fish_info[idx].used_table[iColCount-1-12], szCol);
							 break;
				}
			}

			start = tab + 1;
			tab = strchr(start, '\t');
		} while (tab);

		idx++;

		if (idx == MAX_FISH)
			break;
	}

	fclose(fp);

	for (int i = 0; i < MAX_FISH; ++i)
	{
		sys_log(0, "FISH: %-24s vnum %5lu prob %4d %4d %4d %4d len %d %d %d",
				fish_info[i].name,
				fish_info[i].vnum,
				fish_info[i].prob[0],
				fish_info[i].prob[1],
				fish_info[i].prob[2],
				fish_info[i].prob[3],
				fish_info[i].length_range[0],
				fish_info[i].length_range[1],
				fish_info[i].length_range[2]);
	}

	for (int j = 0; j < MAX_PROB; ++j)
	{
		g_prob_accumulate[j][0] = fish_info[0].prob[j];

		for (int i = 1; i < MAX_FISH; ++i)
			g_prob_accumulate[j][i] = fish_info[i].prob[j] + g_prob_accumulate[j][i - 1];

		g_prob_sum[j] = g_prob_accumulate[j][MAX_FISH - 1];
		sys_log(0, "FISH: prob table %d %d", j, g_prob_sum[j]);
	}
}

int DetermineFishByProbIndex(int prob_idx)
{
	int rv = number(1, g_prob_sum[prob_idx]);
	int * p = std::lower_bound(g_prob_accumulate[prob_idx], g_prob_accumulate[prob_idx]+ MAX_FISH, rv);
	int fish_idx = p - g_prob_accumulate[prob_idx];
	return fish_idx;
}

int GetProbIndexByMapIndex(int index)
{
	if (index > 60)
		return -1;

	switch (index)
	{
		case MAP_A1:
		case MAP_B1:
		case MAP_C1:
			return 0;

		case MAP_A3:
		case MAP_B3:
		case MAP_C3:
			return 1;
	}

	return -1;
}

#ifndef __FISHING_MAIN__
int DetermineFish(LPCHARACTER ch)
{
	int map_idx = ch->GetMapIndex();
	int prob_idx = GetProbIndexByMapIndex(map_idx);

	if (prob_idx < 0)
		return 0;

	// ADD_PREMIUM
	if (ch->GetPremiumRemainSeconds(PREMIUM_FISH_MIND) > 0 ||
			ch->IsEquipUniqueGroup(UNIQUE_GROUP_FISH_MIND)
#ifdef ENABLE_FISHING_RENEWAL
			|| ch->GetPoint(POINT_FISHING_RARE) > 0
#endif
		)
	{
		if (quest::CQuestManager::Instance().GetEventFlag("manwoo") != 0)
			prob_idx = 3;
		else
			prob_idx = 2;
	}
	// END_OF_ADD_PREMIUM

	int adjust = 0;
	if (quest::CQuestManager::Instance().GetEventFlag("fish_miss_pct") != 0)
	{
		int fish_pct_value = MINMAX(0, quest::CQuestManager::Instance().GetEventFlag("fish_miss_pct"), 200);
		adjust = (100-fish_pct_value) * fish_info[0].prob[prob_idx] / 100;
	}

	int rv = number(adjust + 1, g_prob_sum[prob_idx]);

	int * p = std::lower_bound(g_prob_accumulate[prob_idx], g_prob_accumulate[prob_idx] + MAX_FISH, rv);
	int fish_idx = p - g_prob_accumulate[prob_idx];

	{
		uint32_t vnum = fish_info[fish_idx].vnum;

		if (vnum == 50008 || vnum == 50009 || vnum == 80008)
			return 0;
	}

	return (fish_idx);
}

void FishingReact(LPCHARACTER ch)
{
	TPacketGCFishing p{};
	p.header = HEADER_GC_FISHING;
	p.subheader = FISHING_SUBHEADER_GC_REACT;
	p.info = ch->GetVID();
	ch->PacketAround(&p, sizeof(p));
}

void FishingSuccess(LPCHARACTER ch)
{
	TPacketGCFishing p{};
	p.header = HEADER_GC_FISHING;
	p.subheader = FISHING_SUBHEADER_GC_SUCCESS;
	p.info = ch->GetVID();
	ch->PacketAround(&p, sizeof(p));
}

void FishingFail(LPCHARACTER ch)
{
	TPacketGCFishing p{};
	p.header = HEADER_GC_FISHING;
	p.subheader = FISHING_SUBHEADER_GC_FAIL;
	p.info = ch->GetVID();
	ch->PacketAround(&p, sizeof(p));
}

void FishingStop(LPCHARACTER ch)
{
	if (!ch)
		return;

	TPacketGCFishing p{};
	p.header = HEADER_GC_FISHING;
	p.subheader = FISHING_SUBHEADER_GC_STOP;
	p.info = ch->GetVID();
	ch->GetDesc()->Packet(&p, sizeof(TPacketGCFishing));
	ch->m_pkFishingEvent = nullptr;
}

void FishingPractice(LPCHARACTER ch)
{
	LPITEM rod = ch->GetWear(WEAR_WEAPON);
	if (rod && rod->GetType() == ITEM_ROD)
	{
		if ( rod->GetRefinedVnum()>0 && rod->GetSocket(0) < rod->GetValue(2) && number(1,rod->GetValue(1))==1 )
		{
			rod->SetSocket(0, rod->GetSocket(0) + 1);
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;796;%d;%d]",rod->GetSocket(0), rod->GetValue(2));
			if (rod->GetSocket(0) == rod->GetValue(2))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("낚시대가 최대 수련도에 도달하였습니다."));
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("어부를 통해 다음 레벨의 낚시대로 업그레이드 할 수 있습니다."));
			}
		}
	}

	rod->SetSocket(2, 0);
}

bool PredictFish(LPCHARACTER ch)
{
	// ADD_PREMIUM
	if (ch->FindAffect(AFFECT_FISH_MIND_PILL) ||
			ch->GetPremiumRemainSeconds(PREMIUM_FISH_MIND) > 0 ||
			ch->IsEquipUniqueGroup(UNIQUE_GROUP_FISH_MIND))
		return true;
	// END_OF_ADD_PREMIUM

	return false;
}

EVENTFUNC(fishing_event)
{
	fishing_event_info * info = dynamic_cast<fishing_event_info *>( event->info );

	if ( info == nullptr )
	{
		sys_err( "fishing_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(info->pid);

	if (!ch)
		return 0;

	LPITEM rod = ch->GetWear(WEAR_WEAPON);

	if (!(rod && rod->GetType() == ITEM_ROD))
	{
		ch->m_pkFishingEvent = nullptr;
		return 0;
	}

	switch (info->step)
	{
		case 0:
		{
			if (ch->GetExchange()
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
				|| ch->GetMyShop()
				|| ch->GetViewingShopOwner()
#else
				|| ch->GetMyShop()
				|| ch->GetShopOwner()
#endif
				|| ch->IsOpenSafebox() || ch->IsCubeOpen()
#ifdef ENABLE_GUILDBANK
				|| ch->IsOpenGuildBank()
#endif
				)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1439]");
				FishingStop(ch);
				return 0;
			}

#ifdef ENABLE_GROWTH_PET_SYSTEM
			if (ch->GetGrowthPetSystem()->IsActivePet())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1434]");
				FishingStop(ch);
				return 0;
			}

#	ifdef ENABLE_PET_ATTR_DETERMINE
			if (ch->IsGrowthPetDetermineWindow())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1435]");
				FishingStop(ch);
				return 0;
			}
#	endif
#endif

			++info->step;

			//info->ch->Motion(MOTION_FISHING_SIGN);
			info->hang_time = get_dword_time();
			info->fish_id = DetermineFish(ch);
			FishingReact(ch);
			ch->ChatPacket(CHAT_TYPE_INFO, "FISH_REACT!");

			if (PredictFish(ch))
			{
				TPacketGCFishing p{};
				p.header = HEADER_GC_FISHING;
				p.subheader = FISHING_SUBHEADER_GC_FISH;
				p.info = fish_info[info->fish_id].vnum;
				ch->GetDesc()->Packet(&p, sizeof(TPacketGCFishing));
			}

			return (PASSES_PER_SEC(6));
		}

		default:
		{
			++info->step;

			if (info->step > 5)
				info->step = 5;

			ch->m_pkFishingEvent = nullptr;
			FishingFail(ch);
			rod->SetSocket(2, 0);
			return 0;
		}
	}
}

LPEVENT CreateFishingEvent(LPCHARACTER ch)
{
	fishing_event_info* info = AllocEventInfo<fishing_event_info>();
	info->pid	= ch->GetPlayerID();
	info->step	= 0;
	info->hang_time	= 0;

#ifdef ENABLE_GROWTH_PET_SYSTEM
	int time = 0;
	if (ch->GetGrowthPetSystem()->IsActivePet()
#	ifdef ENABLE_PET_ATTR_DETERMINE
		|| ch->IsGrowthPetDetermineWindow()
#	endif
		)
	{
		time = number(2, 3);
	}
	else
		time = number(10, 40);
#else
	const int time = number(10, 40);
#endif

	TPacketGCFishing p{};
	p.header	= HEADER_GC_FISHING;
	p.subheader	= FISHING_SUBHEADER_GC_START;
	p.info		= ch->GetVID();
	p.dir		= (uint8_t)(ch->GetRotation()/5);
	ch->PacketAround(&p, sizeof(TPacketGCFishing));

	return event_create(fishing_event, info, PASSES_PER_SEC(time));
}

int GetFishingLevel(LPCHARACTER ch)
{
	LPITEM rod = ch->GetWear(WEAR_WEAPON);

	if (!rod || rod->GetType()!= ITEM_ROD)
		return 0;

	return rod->GetSocket(2) + rod->GetValue(0);
}

int Compute(uint32_t fish_id, uint32_t ms, uint32_t* item, int level)
{
	if (fish_id == 0)
		return -2;

	if (fish_id >= MAX_FISH)
	{
		sys_err("Wrong FISH ID : %d", fish_id);
		return -2;
	}

	if (ms > 6000)
		return -1;

	int time_step = MINMAX(0,((ms + 99) / 200), MAX_FISHING_TIME_COUNT - 1);

	if (number(1, 100) <= aFishingTime[fish_info[fish_id].time_type][time_step])
	{
		if (number(1, fish_info[fish_id].difficulty) <= level)
		{
			*item = fish_info[fish_id].vnum;
			return 0;
		}
		return -3;
	}

	return -1;
}

int GetFishLength(int fish_id)
{
	if (number(0,99))
	{
		// 99% : normal size
		return (int)(fish_info[fish_id].length_range[0] +
				(fish_info[fish_id].length_range[1] - fish_info[fish_id].length_range[0])
				* (number(0,2000)+number(0,2000)+number(0,2000)+number(0,2000)+number(0,2000))/10000);
	}
	else
	{
		// 1% : extra LARGE size
		return (int)(fish_info[fish_id].length_range[1] +
				(fish_info[fish_id].length_range[2] - fish_info[fish_id].length_range[1])
				* 2 * asin(number(0,10000)/10000.) / M_PI);
	}
}

void Take(fishing_event_info* info, LPCHARACTER ch)
{
	if (info->step == 1)
	{
		long ms = (long) ((get_dword_time() - info->hang_time));
		uint32_t item_vnum = 0;
		int ret = Compute(info->fish_id, ms, &item_vnum, GetFishingLevel(ch));

		//if (test_server)
		//ch->ChatPacket(CHAT_TYPE_INFO, "%d ms", ms);

		switch (ret)
		{
			case -2:
			case -3:
			case -1:
				//ch->ChatPacket(CHAT_TYPE_INFO, "[LS;799]");
				{
					int map_idx = ch->GetMapIndex();
					int prob_idx = GetProbIndexByMapIndex(map_idx);

					LogManager::Instance().FishLog(
							ch->GetPlayerID(),
							prob_idx,
							info->fish_id,
							GetFishingLevel(ch),
							ms);
				}
				FishingFail(ch);
				break;

			case 0:
				//ch->ChatPacket(CHAT_TYPE_INFO, "[LS;800]", fish_info[info->fish_id].name);
				if (item_vnum)
				{
					FishingSuccess(ch);

#ifdef ENABLE_FISH_EVENT
					if (quest::CQuestManager::Instance().GetEventFlag("fish_event"))
					{
						if (ch->IsEquipUniqueItem(UNIQUE_ITEM_FISH_MIND))
						{
							int r = number(1, 100);
							if (r <= 10)
								ch->AutoGiveItem(ITEM_FISH_EVENT_BOX_SPECIAL, 1, -1, false);
							else
								ch->AutoGiveItem(ITEM_FISH_EVENT_BOX, 5, -1, false);
						}
						else
						{
							ch->AutoGiveItem(ITEM_FISH_EVENT_BOX, 5, -1, false);
						}
					}
#endif

					TPacketGCFishing p{};
					p.header = HEADER_GC_FISHING;
					p.subheader = FISHING_SUBHEADER_GC_FISH;
					p.info = item_vnum;
					ch->GetDesc()->Packet(&p, sizeof(TPacketGCFishing));

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
					CAchievementSystem::Instance().OnFishItem(ch, achievements::ETaskTypes::TYPE_FISH, item_vnum, 1);
#endif

					LPITEM item = ch->AutoGiveItem(item_vnum, 1, -1, false);
					if (item)
					{
#ifdef ENABLE_BATTLE_PASS_SYSTEM
						ch->UpdateExtBattlePassMissionProgress(FISH_FISHING, 1, item_vnum);
#endif

#ifndef ENABLE_FISHING_RENEWAL
						item->SetSocket(0, GetFishLength(info->fish_id));
#endif
						if (test_server)
						{
							ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("이번에 잡은 물고기의 길이는 %.2fcm"), item->GetSocket(0)/100.f);
						}

						if (quest::CQuestManager::Instance().GetEventFlag("fishevent") > 0 && (info->fish_id == 5 || info->fish_id == 6))
						{
							TPacketGDHighscore p;
							p.dwPID = ch->GetPlayerID();
							p.lValue = item->GetSocket(0);

							if (info->fish_id == 5)
							{
								strlcpy(p.szBoard, LC_TEXT("낚시이벤트월척붕어"), sizeof(p.szBoard));
							}
							else if (info->fish_id == 6)
							{
								strlcpy(p.szBoard, LC_TEXT("낚시이벤트잉어"), sizeof(p.szBoard));
							}

							db_clientdesc->DBPacket(HEADER_GD_HIGHSCORE_REGISTER, 0, &p, sizeof(TPacketGDHighscore));
						}
					}

					int map_idx = ch->GetMapIndex();
					int prob_idx = GetProbIndexByMapIndex(map_idx);

					LogManager::Instance().FishLog(
							ch->GetPlayerID(),
							prob_idx,
							info->fish_id,
							GetFishingLevel(ch),
							ms,
							true,
							item ? item->GetSocket(0) : 0);

				}
				else
				{
					int map_idx = ch->GetMapIndex();
					int prob_idx = GetProbIndexByMapIndex(map_idx);

					LogManager::Instance().FishLog(
							ch->GetPlayerID(),
							prob_idx,
							info->fish_id,
							GetFishingLevel(ch),
							ms);
					FishingFail(ch);
				}
				break;
		}
	}
	else if (info->step > 1)
	{
		int map_idx = ch->GetMapIndex();
		int prob_idx = GetProbIndexByMapIndex(map_idx);

		LogManager::Instance().FishLog(
				ch->GetPlayerID(),
				prob_idx,
				info->fish_id,
				GetFishingLevel(ch),
				7000);
		//ch->ChatPacket(CHAT_TYPE_INFO, "[LS;799]");
		FishingFail(ch);
	}
	else
	{
		TPacketGCFishing p{};
		p.header = HEADER_GC_FISHING;
		p.subheader = FISHING_SUBHEADER_GC_STOP;
		p.info = ch->GetVID();
		ch->PacketAround(&p, sizeof(p));
	}

	if (info->step)
	{
		FishingPractice(ch);
	}
	//Motion(MOTION_FISHING_PULL);
}

void Simulation(int level, int count, int prob_idx, LPCHARACTER ch)
{
	std::map<std::string, int> fished;
	int total_count = 0;

	for (int i = 0; i < count; ++i)
	{
		int fish_id = DetermineFishByProbIndex(prob_idx);
		uint32_t item = 0;
		Compute(fish_id, (number(2000, 4000) + number(2000,4000)) / 2, &item, level);

		if (item)
		{
			fished[fish_info[fish_id].name]++;
			total_count ++;
		}
	}

	for (std::map<std::string,int>::iterator it = fished.begin(); it != fished.end(); ++it)
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;805;%s;%d]", it->first.c_str(), it->second);

	ch->ChatPacket(CHAT_TYPE_INFO, "[LS;806;%d;%d]", fished.size(), total_count);
}

void UseFish(LPCHARACTER ch, LPITEM item)
{
	int idx = item->GetVnum() - fish_info[2].vnum+2;

	if (idx<=1 || idx >= MAX_FISH)
		return;

	int r = number(1, 10000);

	item->SetCount(item->GetCount()-1);
#ifdef ENABLE_BATTLE_PASS_SYSTEM
	ch->UpdateExtBattlePassMissionProgress(FISH_CATCH, 1, item->GetVnum());
#endif

	if (r >= 4001)
	{
		ch->AutoGiveItem(fish_info[idx].dead_vnum);
	}
	else if (r >= 2001)
	{
		ch->AutoGiveItem(FISH_BONE_VNUM);
	}
#ifndef ENABLE_FISHING_RENEWAL
	else
	{
		// 1000 500 300 100 50 30 10 5 4 1
		static int s_acc_prob[NUM_USE_RESULT_COUNT] = { 1000, 1500, 1800, 1900, 1950, 1980, 1990, 1995, 1999, 2000 };
		int u_index = std::lower_bound(s_acc_prob, s_acc_prob + NUM_USE_RESULT_COUNT, r) - s_acc_prob;

		switch (fish_info[idx].used_table[u_index])
		{
			case USED_TREASURE_MAP:	// 3
			case USED_NONE:		// 0
			case USED_WATER_STONE:	// 2
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;807]");
				break;

			case USED_SHELLFISH:	// 1
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;808]");
				ch->AutoGiveItem(SHELLFISH_VNUM);
				break;

			case USED_EARTHWARM:	// 4
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;809]");
				ch->AutoGiveItem(EARTHWORM_VNUM);
				break;

			default:
				ch->AutoGiveItem(fish_info[idx].used_table[u_index]);
				break;
		}
	}
#endif
}

void Grill(LPCHARACTER ch, LPITEM item)
{
	/*if (item->GetVnum() < fish_info[3].vnum)
	  return;
	  int idx = item->GetVnum()-fish_info[3].vnum+3;
	  if (idx>=MAX_FISH)
	  idx = item->GetVnum()-fish_info[3].dead_vnum+3;
	  if (idx>=MAX_FISH)
	  return;*/
	int idx = -1;
	uint32_t vnum = item->GetVnum();
	if (vnum >= 27803 && vnum <= 27830)
		idx = vnum - 27800;
	if (vnum >= 27833 && vnum <= 27860)
		idx = vnum - 27830;
	if (idx == -1)
		return;

	int count = item->GetCount();

	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s를 구웠습니다."), item->GetName());
	item->SetCount(0);
	ch->AutoGiveItem(fish_info[idx].grill_vnum, count);

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	CAchievementSystem::Instance().OnFishItem(ch, achievements::ETaskTypes::TYPE_BURN, fish_info[idx].grill_vnum, count);
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
	ch->UpdateExtBattlePassMissionProgress(FISH_GRILL, count, item->GetVnum());
#endif
}

bool RefinableRod(LPITEM rod)
{
	if (rod->GetType() != ITEM_ROD)
		return false;

	if (rod->IsEquipped())
		return false;

	return (rod->GetSocket(0) == rod->GetValue(2));
}

int RealRefineRod(LPCHARACTER ch, LPITEM item)
{
	if (!ch || !item)
		return 2;

	// REFINE_ROD_HACK_BUG_FIX
	if (!RefinableRod(item))
	{
		sys_err("REFINE_ROD_HACK pid(%u) item(%s:%d)", ch->GetPlayerID(), item->GetName(), item->GetID());

		LogManager::Instance().RefineLog(ch->GetPlayerID(), item->GetName(), item->GetID(), -1, 1, "ROD_HACK");

		return 2;
	}
	// END_OF_REFINE_ROD_HACK_BUG_FIX

	LPITEM rod = item;

	int iAdv = rod->GetValue(0) / 10;

	if (number(1,100) <= rod->GetValue(3))
	{
		LogManager::Instance().RefineLog(ch->GetPlayerID(), rod->GetName(), rod->GetID(), iAdv, 1, "ROD");

		LPITEM pkNewItem = ITEM_MANAGER::Instance().CreateItem(rod->GetRefinedVnum(), 1);

		if (pkNewItem)
		{
			uint8_t bCell = rod->GetCell();
			ITEM_MANAGER::Instance().RemoveItem(rod, "REMOVE (REFINE FISH_ROD)");
			pkNewItem->AddToCharacter(ch, TItemPos (INVENTORY, bCell));
			LogManager::Instance().ItemLog(ch, pkNewItem, "REFINE FISH_ROD SUCCESS", pkNewItem->GetName());
			return 1;
		}

		return 2;
	}
	else
	{
		LogManager::Instance().RefineLog(ch->GetPlayerID(), rod->GetName(), rod->GetID(), iAdv, 0, "ROD");

#ifdef ENABLE_FISHINGROD_RENEWAL
			{
				const int cur = rod->GetSocket(0);
				rod->SetSocket(0, (cur > 0) ? (cur - (cur * 10 / 100)) : 0);
				LogManager::Instance().ItemLog(ch, rod, "REFINE FISH_ROD FAIL", rod->GetName());
				return 0;
			}
#else
			LPITEM pkNewItem = ITEM_MANAGER::Instance().CreateItem(rod->GetValue(4), 1);
			if (pkNewItem)
			{
				uint8_t bCell = rod->GetCell();
				ITEM_MANAGER::Instance().RemoveItem(rod, "REMOVE (REFINE FISH_ROD)");
				pkNewItem->AddToCharacter(ch, TItemPos(INVENTORY, bCell));
				LogManager::Instance().ItemLog(ch, pkNewItem, "REFINE FISH_ROD FAIL", pkNewItem->GetName());
				return 0;
			}
#endif
			return 2;
		}
	}
#endif

#ifdef ENABLE_FISHING_RENEWAL
	uint32_t GetFishCatchedVnum(uint8_t normal_chance, uint8_t rare_chance, bool second)
	{
		if (number(1, 100) >= uint8_t(normal_chance - rare_chance))
			return second == true ? aFishSecondTableRare[number(0, 6)] : aFishFirstTableRare[number(0, 4)];

		return second == true ? aFishSecondTableNormal[number(0, 6)] : aFishFirstTableNormal[number(0, 4)];
	}
#endif
}

#ifdef __FISHING_MAIN__
int main(int argc, char **argv)
{
	//srandom(time(0) + getpid());
	srandomdev();
	/*
	   struct SFishInfo
	   {
	   const char* name;

	   uint32_t vnum;
	   uint32_t dead_vnum;
	   uint32_t grill_vnum;

	   int prob[3];
	   int difficulty;

	   int limit_type;
	   int limits[3];

	   int time_type;
	   int length_range[3]; // MIN MAX EXTRA_MAX : 99% MIN~MAX, 1% MAX~EXTRA_MAX

	   int used_table[NUM_USE_RESULT_COUNT];
	// 6000 2000 1000 500 300 100 50 30 10 5 4 1
	};
	 */
	using namespace fishing;

	Initialize();

	for (int i = 0; i < MAX_FISH; ++i)
	{
		printf("%s\t%u\t%u\t%u\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d",
				fish_info[i].name,
				fish_info[i].vnum,
				fish_info[i].dead_vnum,
				fish_info[i].grill_vnum,
				fish_info[i].prob[0],
				fish_info[i].prob[1],
				fish_info[i].prob[2],
				fish_info[i].difficulty,
				fish_info[i].time_type,
				fish_info[i].length_range[0],
				fish_info[i].length_range[1],
				fish_info[i].length_range[2]);

		for (int j = 0; j < NUM_USE_RESULT_COUNT; ++j)
			printf("\t%d", fish_info[i].used_table[j]);

		puts("");
	}

	return 1;
}
#endif
