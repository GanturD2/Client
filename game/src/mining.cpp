#include "stdafx.h"
#include "mining.h"
#include "char.h"
#include "char_manager.h"
#include "item_manager.h"
#include "item.h"
#include "config.h"
#include "db.h"
#include "log.h"
#include "skill.h"
#ifdef ENABLE_BATTLE_FIELD
#	include "battle_field.h"
#endif

namespace mining
{
	enum
	{
#ifdef ENABLE_YOHARA_SYSTEM
		MAX_ORE = 23,
#else
		MAX_ORE = 19,
#endif
		MAX_FRACTION_COUNT = 9,
		ORE_COUNT_FOR_REFINE = 100,
	};

	struct SInfo
	{
		uint32_t dwLoadVnum;
		uint32_t dwRawOreVnum;
		uint32_t dwRefineVnum;
	};

	SInfo info[MAX_ORE] =
	{
		{ 20047, 50601, 50621 },	//Diamantader
		{ 20048, 50602, 50622 },	//Bernsteinader
		{ 20049, 50603, 50623 },	//Fossilholzader
		{ 20050, 50604, 50624 },	//Kupferader
		{ 20051, 50605, 50625 },	//Silberader
		{ 20052, 50606, 50626 },	//Goldader
		{ 20053, 50607, 50627 },	//Jadeader
		{ 20054, 50608, 50628 },	//Ebenholzader
		{ 20055, 50609, 50629 },	//Muschelstapel
		{ 20056, 50610, 50630 },	//Wei�goldader
		{ 20057, 50611, 50631 },	//Kristallader
		{ 20058, 50612, 50632 },	//Quarzader
		{ 20059, 50613, 50633 },	//Himmelstr�nenader
		{ 30301, 50614, 50634 },	//Seelenkristallader
		{ 30302, 50615, 50635 },	//Rubinader
		{ 30303, 50616, 50636 },	//Granatader
		{ 30304, 50617, 50637 },	//Smaragdader
		{ 30305, 50618, 50638 },	//Saphirader
		{ 30306, 50619, 50639 },	//Turmalinerzader
#ifdef ENABLE_YOHARA_SYSTEM
		{ 30310, 50641, 50661 },	//Sonnenjediterz
		{ 30311, 50642, 50662 },	//Nachtjediterz
		{ 30312, 50643, 50663 },	//Rauchjediterz
		{ 30313, 50644, 50664 },	//Nebeljediterz
#endif
	};

	int fraction_info[MAX_FRACTION_COUNT][3] =
	{
		{ 20,  1, 10 },
		{ 30, 11, 20 },
		{ 20, 21, 30 },
		{ 15, 31, 40 },
		{  5, 41, 50 },
		{  4, 51, 60 },
		{  3, 61, 70 },
		{  2, 71, 80 },
		{  1, 81, 90 },
	};

	int PickGradeAddPct[10] =
	{
		3, 5, 8, 11, 15, 20, 26, 32, 40, 50
	};

	int SkillLevelAddPct[SKILL_MAX_LEVEL + 1] =
	{
		0,
		1, 1, 1, 1,		//  1 - 4
		2, 2, 2, 2,		//  5 - 8
		3, 3, 3, 3,		//  9 - 12
		4, 4, 4, 4,		// 13 - 16
		5, 5, 5, 5,		// 17 - 20
		6, 6, 6, 6,		// 21 - 24
		7, 7, 7, 7,		// 25 - 28
		8, 8, 8, 8,		// 29 - 32
		9, 9, 9, 9,		// 33 - 36
		10, 10, 10, 	// 37 - 39
		11,				// 40
	};

	uint32_t GetRawOreFromLoad(uint32_t dwLoadVnum)
	{
		for (int i = 0; i < MAX_ORE; ++i)
		{
			if (info[i].dwLoadVnum == dwLoadVnum)
				return info[i].dwRawOreVnum;
		}
		return 0;
	}

	uint32_t GetRefineFromRawOre(uint32_t dwRawOreVnum)
	{
		for (int i = 0; i < MAX_ORE; ++i)
		{
			if (info[i].dwRawOreVnum == dwRawOreVnum)
				return info[i].dwRefineVnum;
		}
		return 0;
	}

	int GetFractionCount()
	{
		int r = number(1, 100);

		for (int i = 0; i < MAX_FRACTION_COUNT; ++i)
		{
			if (r <= fraction_info[i][0])
				return number(fraction_info[i][1], fraction_info[i][2]);
			else
				r -= fraction_info[i][0];
		}

		return 0;
	}

	void OreDrop(LPCHARACTER ch, uint32_t dwLoadVnum)
	{
		if (!ch)
			return;

		const uint32_t dwRawOreVnum = GetRawOreFromLoad(dwLoadVnum);

		const int iFractionCount = GetFractionCount();

		if (iFractionCount == 0)
		{
			sys_err("Wrong ore fraction count");
			return;
		}

		LPITEM item = ITEM_MANAGER::Instance().CreateItem(dwRawOreVnum, iFractionCount); // Nyx : Avoid create twice the random fraction count, because is already declared

		if (!item)
		{
			sys_err("cannot create item vnum %d", dwRawOreVnum);
			return;
		}

		PIXEL_POSITION pos;
		pos.x = ch->GetX() + number(-200, 200);
		pos.y = ch->GetY() + number(-200, 200);

		item->AddToGround(ch->GetMapIndex(), pos);
		item->StartDestroyEvent();
#ifdef ENABLE_BATTLE_FIELD
		if (!CBattleField::Instance().IsBattleZoneMapIndex(ch->GetMapIndex()))
		{
			item->SetOwnership(ch, 15);
		}
#else
		item->SetOwnership(ch, 15);
#endif

		DBManager::Instance().SendMoneyLog(MONEY_LOG_DROP, item->GetVnum(), item->GetCount());
	}

	int GetOrePct(LPCHARACTER ch)
	{
		if (!ch)
			return 0;

		const int defaultPct = 20;
		const int iSkillLevel = ch->GetSkillLevel(SKILL_MINING);

		LPITEM pick = ch->GetWear(WEAR_WEAPON);

		if (!pick || pick->GetType() != ITEM_PICK)
			return 0;

		return defaultPct + SkillLevelAddPct[MINMAX(0, iSkillLevel, 40)] + PickGradeAddPct[MINMAX(0, pick->GetRefineLevel(), 9)];
	}

	EVENTINFO(mining_event_info)
	{
		uint32_t pid;
		uint32_t vid_load;

		mining_event_info()
			: pid(0)
			, vid_load(0)
		{
		}
	};

	// REFINE_PICK
	bool Pick_Check(CItem& item)
	{
		if (item.GetType() != ITEM_PICK)
			return false;

		return true;
	}

	int Pick_GetMaxExp(CItem& pick)
	{
		return pick.GetValue(2);
	}

	int Pick_GetCurExp(CItem& pick)
	{
		return pick.GetSocket(0);
	}

	void Pick_IncCurExp(CItem& pick)
	{
		const int cur = Pick_GetCurExp(pick);
		pick.SetSocket(0, cur + 1);
	}

#ifdef ENABLE_PICKAXE_RENEWAL
	void Pick_SetPenaltyExp(CItem& pick)
	{
		const int cur = Pick_GetCurExp(pick);
		pick.SetSocket(0, (cur > 0) ? (cur - (cur * 10 / 100)) : 0);
	}
#endif

	void Pick_MaxCurExp(CItem& pick)
	{
		const int max = Pick_GetMaxExp(pick);
		pick.SetSocket(0, max);
	}

	bool Pick_Refinable(CItem& item)
	{
		if (Pick_GetCurExp(item) <= Pick_GetMaxExp(item))	//@fixme538
			return false;

		return true;
	}

	bool Pick_IsPracticeSuccess(CItem& pick)
	{
		return (number(1, pick.GetValue(1)) == 1);
	}

	bool Pick_IsRefineSuccess(CItem& pick)
	{
		return (number(1, 100) <= pick.GetValue(3));
	}

	int RealRefinePick(LPCHARACTER ch, LPITEM item)
	{
		if (!ch || !item)
			return 2;

		LogManager& rkLogMgr = LogManager::Instance();
		ITEM_MANAGER& rkItemMgr = ITEM_MANAGER::Instance();

		if (!Pick_Check(*item))
		{
			sys_err("REFINE_PICK_HACK pid(%u) item(%s:%d) type(%d)", ch->GetPlayerID(), item->GetName(), item->GetID(), item->GetType());
			rkLogMgr.RefineLog(ch->GetPlayerID(), item->GetName(), item->GetID(), -1, 1, "PICK_HACK");
			return 2;
		}

		CItem& rkOldPick = *item;

		if (!Pick_Refinable(rkOldPick))
			return 2;

		const int iAdv = rkOldPick.GetValue(0) / 10;

		if (rkOldPick.IsEquipped() == true)
			return 2;

		if (Pick_IsRefineSuccess(rkOldPick))
		{
			rkLogMgr.RefineLog(ch->GetPlayerID(), rkOldPick.GetName(), rkOldPick.GetID(), iAdv, 1, "PICK");

			LPITEM pkNewPick = ITEM_MANAGER::Instance().CreateItem(rkOldPick.GetRefinedVnum(), 1);
			if (pkNewPick)
			{
				const uint8_t bCell = rkOldPick.GetCell();
				rkItemMgr.RemoveItem(item, "REMOVE (REFINE PICK)");
				pkNewPick->AddToCharacter(ch, TItemPos(INVENTORY, bCell));
				LogManager::Instance().ItemLog(ch, pkNewPick, "REFINE PICK SUCCESS", pkNewPick->GetName());
				return 1;
			}

			return 2;
		}
		else
		{
			rkLogMgr.RefineLog(ch->GetPlayerID(), rkOldPick.GetName(), rkOldPick.GetID(), iAdv, 0, "PICK");

#ifdef ENABLE_PICKAXE_RENEWAL
			{
				/*if (test_server)
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Pickax> PRE %u"), Pick_GetCurExp(*item));*/
				Pick_SetPenaltyExp(*item);
				/*if (test_server)
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Pickax> POST %u"), Pick_GetCurExp(*item));*/
				// ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Pickax> The upgrade has failed, and the pickax has lost 10%% of its mastery points."));
				rkLogMgr.ItemLog(ch, item, "REFINE PICK FAIL", item->GetName());
				return 0;
			}
#else
			LPITEM pkNewPick = ITEM_MANAGER::Instance().CreateItem(rkOldPick.GetValue(4), 1);

			if (pkNewPick)
			{
				uint8_t bCell = rkOldPick.GetCell();
				rkItemMgr.RemoveItem(item, "REMOVE (REFINE PICK)");
				pkNewPick->AddToCharacter(ch, TItemPos(INVENTORY, bCell));
				rkLogMgr.ItemLog(ch, pkNewPick, "REFINE PICK FAIL", pkNewPick->GetName());
				return 0;
			}
#endif
			return 2;
		}
	}

	void CHEAT_MAX_PICK(LPCHARACTER ch, LPITEM item)
	{
		if (!ch)
			return;

		if (!item)
			return;

		if (!Pick_Check(*item))
			return;

		CItem& pick = *item;
		Pick_MaxCurExp(pick);

		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;727;%d]", Pick_GetCurExp(pick));
	}

	void PracticePick(LPCHARACTER ch, LPITEM item)
	{
		if (!ch)
			return;

		if (!item)
			return;

		if (!Pick_Check(*item))
			return;

		CItem& pick = *item;
		if (pick.GetRefinedVnum() <= 0)
			return;

		if (Pick_IsPracticeSuccess(pick))
		{

			if (Pick_Refinable(pick))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;728]");
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;729]");
			}
			else
			{
				Pick_IncCurExp(pick);

				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;730;%d;%d]", Pick_GetCurExp(pick), Pick_GetMaxExp(pick));

				if (Pick_Refinable(pick))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;728]");
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;729]");
				}
			}
		}
	}
	// END_OF_REFINE_PICK

	EVENTFUNC(mining_event)
	{
		const mining_event_info* info = dynamic_cast<mining_event_info*>(event->info);

		if (info == nullptr)
		{
			sys_err("mining_event_info> <Factor> Null pointer");
			return 0;
		}

		LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(info->pid);
		const LPCHARACTER& load = CHARACTER_MANAGER::Instance().Find(info->vid_load);

		if (!ch)
			return 0;

		ch->mining_take();

		LPITEM pick = ch->GetWear(WEAR_WEAPON);

#ifdef ENABLE_MULTI_FARM_BLOCK
		if (!ch->GetMultiStatus())
			return 0;
#endif

		// REFINE_PICK
		if (!pick || !Pick_Check(*pick))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;731]");
			return 0;
		}
		// END_OF_REFINE_PICK

		if (!load)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;732]");
			return 0;
		}

		const int iPct = GetOrePct(ch);

		if (number(1, 100) <= iPct)
		{
			OreDrop(ch, load->GetRaceNum());
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;733]");
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;734]");
		}

		PracticePick(ch, pick);

		return 0;
	}

	LPEVENT CreateMiningEvent(LPCHARACTER ch, LPCHARACTER load, int count)
	{
		/*if (!ch || !load)
			return; // what i should to return? ...
		*/

		mining_event_info* info = AllocEventInfo<mining_event_info>();
		info->pid = ch->GetPlayerID();
		info->vid_load = load->GetVID();

		return event_create(mining_event, info, PASSES_PER_SEC(2 * count));
	}

	bool OreRefine(LPCHARACTER ch, LPCHARACTER npc, LPITEM item, int cost, int pct, LPITEM metinstone_item)
	{
		if (!ch || !npc || !item)
			return false;

		if (item->GetOwner() != ch)
		{
			sys_err("wrong owner");
			return false;
		}

		if (item->GetCount() < ORE_COUNT_FOR_REFINE)
		{
			sys_err("not enough count");
			return false;
		}

		const uint32_t dwRefinedVnum = GetRefineFromRawOre(item->GetVnum());

		if (dwRefinedVnum == 0)
			return false;

		ch->SetRefineNPC(npc);
		item->SetCount(item->GetCount() - ORE_COUNT_FOR_REFINE);
		const int iCost = ch->ComputeRefineFee(cost, 1);

		if (ch->GetGold() < iCost)
			return false;

		ch->PayRefineFee(iCost);

		if (metinstone_item)
			ITEM_MANAGER::Instance().RemoveItem(metinstone_item, "REMOVE (MELT)");

		if (number(1, 100) <= pct)
		{
			ch->AutoGiveItem(dwRefinedVnum, 1);
			return true;
		}

		return false;
	}

	bool IsVeinOfOre(uint32_t vnum)
	{
		for (int i = 0; i < MAX_ORE; i++)
		{
			if (info[i].dwLoadVnum == vnum)
				return true;
		}
		return false;
	}
}
