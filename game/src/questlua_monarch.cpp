#include "stdafx.h"

#include "questlua.h"
#include "questmanager.h"
#include "monarch.h"
#include "desc_client.h"
#include "start_position.h"
#include "config.h"
#include "mob_manager.h"
#include "castle.h"
#include "dev_log.h"
#include "char.h"
#include "char_manager.h"
#include "utils.h"
#include "p2p.h"
#include "guild.h"
#include "sectree_manager.h"

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

ACMD(do_monarch_mob);

namespace quest
{
	EVENTINFO(monarch_powerup_event_info)
	{
		int EmpireIndex;

		monarch_powerup_event_info()
		: EmpireIndex( 0 )
		{
		}
	};

	// NOTE: Copied from SPacketGGMonarchTransfer for event data
	EVENTINFO(monarch_transfer2_event_info)
	{
		uint8_t	bHeader;
		uint32_t	dwTargetPID;
		long	x;
		long	y;

		monarch_transfer2_event_info()
		: bHeader( 0 )
		, dwTargetPID( 0 )
		, x( 0 )
		, y( 0 )
		{
		}
	};

	EVENTFUNC(monarch_powerup_event)
	{
		monarch_powerup_event_info * info =  dynamic_cast<monarch_powerup_event_info*>(event->info);

		if ( info == nullptr )
		{
			sys_err( "monarch_powerup_event> <Factor> Null pointer" );
			return 0;
		}

		CMonarch::Instance().PowerUp(info->EmpireIndex, false);
		return 0;
	}

	EVENTINFO(monarch_defenseup_event_info)
	{
		int EmpireIndex;

		monarch_defenseup_event_info()
		: EmpireIndex( 0 )
		{
		}
	};

	EVENTFUNC(monarch_defenseup_event)
	{
		monarch_powerup_event_info * info =  dynamic_cast<monarch_powerup_event_info*>(event->info);

		if ( info == nullptr )
		{
			sys_err( "monarch_defenseup_event> <Factor> Null pointer" );
			return 0;
		}

		CMonarch::Instance().DefenseUp(info->EmpireIndex, false);
		return 0;
	}

	//
	// "monarch_" lua functions
	//
	ALUA(takemonarchmoney)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

#ifdef __UNIMPLEMENTED__
		if (!ch) {
			lua_pushboolean(L, false);
			return 1;
		}

		if (!ch->IsMonarch() && !ch->IsGM())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;547]");
			sys_err("A regular player is trying to use monarch cash - pid %d ", ch->GetPlayerID());

			lua_pushboolean(L, false);
			return 1;
		}
#endif

		int nMoney = (int)lua_tonumber(L,1);
		int nPID = ch->GetPlayerID();
		int nEmpire = ch->GetEmpire();
		nMoney = nMoney * 10000;

#ifdef __UNIMPLEMENTED__
		if (nMoney < 0) 
		{
			sys_err("Invalid money to take from monarch vault: %d", nMoney);
			lua_pushboolean(L, false);
			return 1;
		}

		if (!CMonarch::Instance().IsMoneyOk(nMoney, ch->GetEmpire()))
		{
			int NationMoney = CMonarch::Instance().GetMoney(ch->GetEmpire());
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ���� �����մϴ�. ���� : %u �ʿ�ݾ� : %u"), NationMoney, nMoney);
			lua_pushboolean(L, false);
			return 1;
		}

		CMonarch::Instance().SendtoDBDecMoney(nMoney, ch->GetEmpire(), ch);
#endif

		sys_log(0 ,"[MONARCH] Take Money Empire(%d) pid(%d) Money(%d)", ch->GetEmpire(), ch->GetPlayerID(), nMoney);


		db_clientdesc->DBPacketHeader(HEADER_GD_TAKE_MONARCH_MONEY, ch->GetDesc()->GetHandle(), sizeof(int) * 3);
		db_clientdesc->Packet(&nEmpire, sizeof(int));
		db_clientdesc->Packet(&nPID, sizeof(int));
		db_clientdesc->Packet(&nMoney, sizeof(int));
		return 1;
	}

	ALUA(is_guild_master)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (ch->GetGuild()	)
		{
			TGuildMember * pMember = ch->GetGuild()->GetMember(ch->GetPlayerID());

			if (pMember)
			{
				if (pMember->grade <= 4)
				{
					lua_pushnumber(L ,1);
					return 1;
				}
			}

		}
		lua_pushnumber(L ,0);

		return 1;
	}

	ALUA(monarch_bless)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (false==ch->IsMonarch())
		{
			if (!ch->IsGM())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;547]");
				sys_err("No Monarch pid %d ", ch->GetPlayerID());
				return 0;
			}
		}

		int HealPrice = quest::CQuestManager::Instance().GetEventFlag("MonarchHealGold");
		if (HealPrice == 0)
			HealPrice = 2000000;	// 200��

		if (CMonarch::Instance().HealMyEmpire(ch, HealPrice))
		{
			char szNotice[256];
			snprintf(szNotice, sizeof(szNotice),
					LC_TEXT("������ �ູ���� ������ %s ������ HP,SP�� ��� ä�����ϴ�."), EMPIRE_NAME(ch->GetEmpire()));
			SendNoticeMap(szNotice, ch->GetMapIndex(), false);

			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;570]");
		}

		return 1;
	}

	// ������ ������ ����Ʈ �Լ�
	ALUA(monarch_powerup)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (!ch)
			return 0;

		//���� üũ
		if (false==ch->IsMonarch())
		{
			if (!ch->IsGM())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;547]");
				sys_err("No Monarch pid %d ", ch->GetPlayerID());
				return 0;
			}
		}

		//���� ���� �˻�
		int	money_need = 5000000;	// 500��
		if (!CMonarch::Instance().IsMoneyOk(money_need, ch->GetEmpire()))
		{
			int NationMoney = CMonarch::Instance().GetMoney(ch->GetEmpire());
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ���� �����մϴ�. ���� : %u �ʿ�ݾ� : %u"), NationMoney, money_need);
			return 0;
		}

		if (!CMonarch::Instance().CheckPowerUpCT(ch->GetEmpire()))
		{
			int	next_sec = CMonarch::Instance().GetPowerUpCT(ch->GetEmpire()) / passes_per_sec;
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;581;%d]", next_sec);
			return 0;
		}

		//������ ������ ����
		CMonarch::Instance().PowerUp(ch->GetEmpire(), true);

		//������ ������ ����ð�
		int g_nMonarchPowerUpCT = 60 * 3;

		monarch_powerup_event_info* info = AllocEventInfo<monarch_powerup_event_info>();

		info->EmpireIndex = ch->GetEmpire();

		event_create(quest::monarch_powerup_event, info, PASSES_PER_SEC(g_nMonarchPowerUpCT));

		CMonarch::Instance().SendtoDBDecMoney(5000000, ch->GetEmpire(), ch);

		char szNotice[256];
		snprintf(szNotice, sizeof(szNotice), LC_TEXT("������ ������ �������� ������ %s ������ 3�а� 10 %% �� ���ݷ���  �����˴ϴ�"), EMPIRE_NAME(ch->GetEmpire()));

		SendNoticeMap(szNotice, ch->GetMapIndex(), false);

		return 1;
	}
	// ������ �ݰ��� ����Ʈ �Լ�
	ALUA(monarch_defenseup)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (!ch)
			return 0;


		//���� üũ

		if (false==ch->IsMonarch())
		{
			if (!ch->IsGM())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;547]");
				sys_err("No Monarch pid %d ", ch->GetPlayerID());
				return 0;
			}
		}

		int	money_need = 5000000;	// 500��
		if (!CMonarch::Instance().IsMoneyOk(money_need, ch->GetEmpire()))
		{
			int NationMoney = CMonarch::Instance().GetMoney(ch->GetEmpire());
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ���� �����մϴ�. ���� : %u �ʿ�ݾ� : %u"), NationMoney, money_need);
			return 0;
		}

		if (!CMonarch::Instance().CheckDefenseUpCT(ch->GetEmpire()))
		{
			int	next_sec = CMonarch::Instance().GetDefenseUpCT(ch->GetEmpire()) / passes_per_sec;
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;581;%d]", next_sec);
			return 0;
		}

		//������ �ݰ��� ����
		CMonarch::Instance().DefenseUp(ch->GetEmpire(), true);

		//������ �ݰ��� ���� �ð�
		int g_nMonarchDefenseUpCT = 60 * 3;

		monarch_defenseup_event_info* info = AllocEventInfo<monarch_defenseup_event_info>();

		info->EmpireIndex = ch->GetEmpire();

		event_create(quest::monarch_defenseup_event, info, PASSES_PER_SEC(g_nMonarchDefenseUpCT));

		CMonarch::Instance().SendtoDBDecMoney(5000000, ch->GetEmpire(), ch);

		char szNotice[256];
		snprintf(szNotice, sizeof(szNotice), LC_TEXT("������ �ݰ��� �������� ������ %s ������ 3�а� 10 %% �� ������  �����˴ϴ�"), EMPIRE_NAME(ch->GetEmpire()));

		SendNoticeMap(szNotice, ch->GetMapIndex(), false);

		return 1;
	}

	ALUA(is_monarch)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;
		lua_pushnumber(L, ch->IsMonarch());
		return 1;
	}

	ALUA(spawn_mob)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}

		uint32_t mob_vnum = (uint32_t)lua_tonumber(L, 1);

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (!ch)
			return 0;

		const CMob * pkMob = nullptr;

		if (!(pkMob = CMobManager::Instance().Get(mob_vnum)))
			if (pkMob == nullptr)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "No such mob by that vnum");
				return 0;
			}

		if (false == ch->IsMonarch())
		{
			if (!ch->IsGM())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;547]");
				sys_err("No Monarch pid %d ", ch->GetPlayerID());
				return 0;
			}
		}

		uint32_t dwQuestIdx = CQuestManager::Instance().GetCurrentPC()->GetCurrentQuestIndex();

		bool ret = false;
		LPCHARACTER mob = nullptr;

		{
			long x = ch->GetX();
			long y = ch->GetY();
#if 0
			if (11505 == mob_vnum)	// Ȳ�ݵβ���
			{
				//���� ���� �˻�
				if (!CMonarch::Instance().IsMoneyOk(CASTLE_FROG_PRICE, ch->GetEmpire()))
				{
					int NationMoney = CMonarch::Instance().GetMoney(ch->GetEmpire());
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ���� �����մϴ�. ���� : %u �ʿ�ݾ� : %u"), NationMoney, CASTLE_FROG_PRICE);
					return 0;
				}

				mob = castle_spawn_frog(ch->GetEmpire());

				if (mob)
				{
					// ������
					CMonarch::Instance().SendtoDBDecMoney(CASTLE_FROG_PRICE, ch->GetEmpire(), ch);
					castle_save();
				}
			}
			else
#endif
			{
				mob = CHARACTER_MANAGER::Instance().SpawnMob(mob_vnum, ch->GetMapIndex(), x, y, 0, pkMob->m_table.bType == CHAR_TYPE_STONE, -1);
			}

			if (mob)
			{
				//if (bAggressive)
				//	mob->SetAggressive();

				mob->SetQuestBy(dwQuestIdx);

				if (!ret)
				{
					ret = true;
					lua_pushnumber(L, (uint32_t) mob->GetVID());
				}
			}
		}

		return 1;
	}

	ALUA(spawn_guard)
	{
		// mob_level(0-2), region(0~3)
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("invalid argument");
			return 0;
		}

		uint32_t	group_vnum		= (uint32_t)lua_tonumber(L,1);
		int		region_index	= (int)lua_tonumber(L, 2);
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		/*-----
		const CMob * pkMob = nullptr;
		if (!(pkMob = CMobManager::Instance().Get(mob_vnum)))

		if (pkMob == nullptr)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "No such mob by that vnum");
			return 0;
		}
		-----*/

		if (false==ch->IsMonarch())
		{
			if (!ch->IsGM())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;547]");
				sys_err("No Monarch pid %d ", ch->GetPlayerID());
				return 0;
			}
		}

		if (false==castle_is_my_castle(ch->GetEmpire(), ch->GetMapIndex()))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;997]");
			return 0;
		}

		//uint32_t 			dwQuestIdx 	= CQuestManager::Instance().GetCurrentPC()->GetCurrentQuestIndex();

		LPCHARACTER		guard_leader = nullptr;
		{
			int	money_need = castle_cost_of_hiring_guard(group_vnum);

			//���� ���� �˻�
			if (!CMonarch::Instance().IsMoneyOk(money_need, ch->GetEmpire()))
			{
				int NationMoney = CMonarch::Instance().GetMoney(ch->GetEmpire());
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ���� �����մϴ�. ���� : %u �ʿ�ݾ� : %u"), NationMoney, money_need);
				return 0;
			}
			guard_leader = castle_spawn_guard(ch->GetEmpire(), group_vnum, region_index);

			if (guard_leader)
			{
				// ������
				CMonarch::Instance().SendtoDBDecMoney(money_need, ch->GetEmpire(), ch);
				castle_save();
			}
		}

		return 1;
	}

	ALUA(frog_to_empire_money)
	{
		LPCHARACTER ch	= CQuestManager::Instance().GetCurrentCharacterPtr();

		if (nullptr==ch)
			return false;

		if (!ch->IsMonarch())
		{
			if (!ch->IsGM())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;547]");
				sys_err("No Monarch pid %d ", ch->GetPlayerID());
				return 0;
			}
		}

		if (castle_frog_to_empire_money(ch))
		{
			int empire_money = CMonarch::Instance().GetMoney(ch->GetEmpire());
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;998]");
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;999;%d]", empire_money);
			castle_save();
			return 1;
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1000]");
			return 0;
		}
	}

	ALUA(monarch_warp)
	{
		if (!lua_isstring(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}

		std::string name = lua_tostring(L, 1);

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;



		if (!CMonarch::Instance().IsMonarch(ch->GetPlayerID(), ch->GetEmpire()))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1007]");
			return 0;
		}

		//���� ��Ÿ�� �˻�
		if (!ch->IsMCOK(CHARACTER::MI_WARP))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d �ʰ� ��Ÿ���� �������Դϴ�."), ch->GetMCLTime(CHARACTER::MI_WARP));
			return 0;
		}


		//���� �� ��ȯ ���
		const int WarpPrice = 10000;

		//���� ���� �˻�
		if (!CMonarch::Instance().IsMoneyOk(WarpPrice, ch->GetEmpire()))
		{
			int NationMoney = CMonarch::Instance().GetMoney(ch->GetEmpire());
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ���� �����մϴ�. ���� : %u �ʿ�ݾ� : %u"), NationMoney, WarpPrice);
			return 0;
		}

		int x, y;

		LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(name.c_str());

		if (!tch)
		{
			CCI * pkCCI = P2P_MANAGER::Instance().Find(name.c_str());

			if (pkCCI)
			{
				if (pkCCI->bEmpire != ch->GetEmpire())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;554]");
					return 0;
				}
				if (pkCCI->bChannel != g_bChannel)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;555;%d;%d]", pkCCI->bChannel, g_bChannel);
					return 0;
				}

				if (!IsMonarchWarpZone(pkCCI->lMapIndex))
				{
					ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("�ش� �������� �̵��� �� �����ϴ�."));
					return 0;
				}

				PIXEL_POSITION pos;

				if (!SECTREE_MANAGER::Instance().GetCenterPositionOfMap(pkCCI->lMapIndex, pos))
					ch->ChatPacket(CHAT_TYPE_INFO, "Cannot find map (index %d)", pkCCI->lMapIndex);
				else
				{
					//ch->ChatPacket(CHAT_TYPE_INFO, "You warp to (%d, %d)", pos.x, pos.y);
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;556;%s]", name.c_str());
					ch->WarpSet(pos.x, pos.y);

					//���� �� �谨
					CMonarch::Instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);

					//��Ÿ�� �ʱ�ȭ
					ch->SetMC(CHARACTER::MI_WARP);
				}

			}
			else if (nullptr == CHARACTER_MANAGER::Instance().FindPC(name.c_str()))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "There is no one by that name");
			}

			return 0;
		}
		else
		{
			if (tch->GetEmpire() != ch->GetEmpire())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;554]");
				return 0;
			}

			if (!IsMonarchWarpZone(tch->GetMapIndex()))
			{
				ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("�ش� �������� �̵��� �� �����ϴ�."));
				return 0;
			}

			x = tch->GetX();
			y = tch->GetY();
		}

		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;556;%s]", name.c_str());
		ch->WarpSet(x,y);
		ch->Stop();

		//���� �� �谨
		CMonarch::Instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);

		//��Ÿ�� �ʱ�ȭ
		ch->SetMC(CHARACTER::MI_WARP);

		return 0;
	}

	ALUA(empire_info)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (nullptr == ch)
			return false;

		TMonarchInfo * p = CMonarch::Instance().GetMonarch();

		if (CMonarch::Instance().IsMonarch(ch->GetPlayerID(), ch->GetEmpire()))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;560]");

			for (int n = 1; n < 4; ++n)
			{
				if (n == ch->GetEmpire())
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;561;%s;%s;%d]", EMPIRE_NAME(n), p->name[n], p->money[n]);
				else
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;562;%s;%s]", EMPIRE_NAME(n), p->name[n]);
			}
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;563]");

			for (int n = 1; n < 4; ++n)
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;562;%s;%s]", EMPIRE_NAME(n), p->name[n]);
		}

		return 0;
	}

	ALUA(monarch_transfer)
	{
		if (!lua_isstring(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}

		std::string name = lua_tostring(L, 1);

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (!ch)
			return 0;

		if (!CMonarch::Instance().IsMonarch(ch->GetPlayerID(), ch->GetEmpire()))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���ָ��� ��� ������ ����Դϴ�"));
			return 0;
		}

		// ���� ��Ÿ�� �˻�
		if (!ch->IsMCOK(CHARACTER::MI_TRANSFER))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d �ʰ� ��Ÿ���� �������Դϴ�."), ch->GetMCLTime(CHARACTER::MI_TRANSFER));
			return 0;
		}

		// ���� ���� ���
		const int WarpPrice = 10000;

		// ���� ���� �˻�
		if (!CMonarch::Instance().IsMoneyOk(WarpPrice, ch->GetEmpire()))
		{
			int NationMoney = CMonarch::Instance().GetMoney(ch->GetEmpire());
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ���� �����մϴ�. ���� : %u �ʿ�ݾ� : %u"), NationMoney, WarpPrice);
			return 0;
		}

		LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(name.c_str());

		if (!tch)
		{
			CCI * pkCCI = P2P_MANAGER::Instance().Find(name.c_str());

			if (pkCCI)
			{
				if (pkCCI->bEmpire != ch->GetEmpire())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1008]");
					return 0;
				}

				if (pkCCI->bChannel != g_bChannel)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1009;%s;%d;%d]", name.c_str(), pkCCI->bChannel, g_bChannel);
					return 0;
				}

				if (!IsMonarchWarpZone(pkCCI->lMapIndex))
				{
					ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("�ش� �������� �̵��� �� �����ϴ�."));
					return 0;
				}
				if (!IsMonarchWarpZone(ch->GetMapIndex()))
				{
					ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("�ش� �������� ��ȯ�� �� �����ϴ�."));
					return 0;
				}

				TPacketGGTransfer pgg{};

				pgg.bHeader = HEADER_GG_TRANSFER;
				strlcpy(pgg.szName, name.c_str(), sizeof(pgg.szName));
				pgg.lX = ch->GetX();
				pgg.lY = ch->GetY();

				P2P_MANAGER::Instance().Send(&pgg, sizeof(TPacketGGTransfer));
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1010;%s]", name.c_str());

				// ���� �� �谨
				CMonarch::Instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);

				// ��Ÿ�� �ʱ�ȭ
				ch->SetMC(CHARACTER::MI_TRANSFER);
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1011]");
			}

			return 0;
		}

		if (ch == tch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1012]");
			return 0;
		}

		if (tch->GetEmpire() != ch->GetEmpire())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1008]");
			return 0;
		}

		if (!IsMonarchWarpZone(tch->GetMapIndex()))
		{
			ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("�ش� �������� �̵��� �� �����ϴ�."));
			return 0;
		}
		if (!IsMonarchWarpZone(ch->GetMapIndex()))
		{
			ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("�ش� �������� ��ȯ�� �� �����ϴ�."));
			return 0;
		}
		tch->WarpSet(ch->GetX(), ch->GetY(), ch->GetMapIndex());

		// ���� �� �谨
		CMonarch::Instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);
		// ��Ÿ�� �ʱ�ȭ
		ch->SetMC(CHARACTER::MI_TRANSFER);
		return 0;
	}

	ALUA(monarch_notice)
	{
		if (!lua_isstring(L, 1))
			return 0;

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (ch == nullptr)
			return 0;

		if (ch->IsMonarch() == false)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1007]");
			return 0;
		}

		std::string strNotice = lua_tostring(L, 1);

		if (strNotice.length() > 0)
			SendMonarchNotice(ch->GetEmpire(), strNotice.c_str());

		return 0;
	}

	ALUA(monarch_mob)
	{
		if (!lua_isstring(L, 1))
			return 0;

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (ch == nullptr)
			return 0;

		char vnum[256];
		strlcpy(vnum, lua_tostring(L, 1), sizeof(vnum));
		do_monarch_mob(ch, vnum, 0, 0);
		return 0;
	}

	EVENTFUNC(monarch_transfer2_event)
	{
		monarch_transfer2_event_info* info = dynamic_cast<monarch_transfer2_event_info*>(event->info);

		if ( info == nullptr )
		{
			sys_err( "monarch_transfer2_event> <Factor> Null pointer" );
			return 0;
		}

		LPCHARACTER pTargetChar = CHARACTER_MANAGER::Instance().FindByPID(info->dwTargetPID);

		if (pTargetChar != nullptr)
		{
			unsigned int qIndex = quest::CQuestManager::Instance().GetQuestIndexByName("monarch_transfer");

			if (qIndex != 0)
			{
				pTargetChar->SetQuestFlag("monarch_transfer.x", info->x);
				pTargetChar->SetQuestFlag("monarch_transfer.y", info->y);
				quest::CQuestManager::Instance().Letter(pTargetChar->GetPlayerID(), qIndex, 0);
			}
		}

		return 0;
	}

	ALUA(monarch_transfer2)
	{
		if (lua_isstring(L, 1) == false) return 0;

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch == nullptr) return false;

		if (CMonarch::Instance().IsMonarch(ch->GetPlayerID(), ch->GetEmpire()) == false)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1007]");
			return 0;
		}

		if (ch->IsMCOK(CHARACTER::MI_TRANSFER) == false)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d �ʰ� ��Ÿ���� �������Դϴ�."), ch->GetMCLTime(CHARACTER::MI_TRANSFER));
			return 0;
		}

		const int ciTransferCost = 10000;

		if (CMonarch::Instance().IsMoneyOk(ciTransferCost, ch->GetEmpire()) == false)
		{
			int NationMoney = CMonarch::Instance().GetMoney(ch->GetEmpire());
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("���� ���� �����մϴ�. ���� : %u �ʿ�ݾ� : %u"), NationMoney, ciTransferCost);
			return 0;
		}

		std::string strTargetName = lua_tostring(L, 1);

		LPCHARACTER pTargetChar = CHARACTER_MANAGER::Instance().FindPC(strTargetName.c_str());

		if (pTargetChar == nullptr)
		{
			CCI* pCCI = P2P_MANAGER::Instance().Find(strTargetName.c_str());

			if (pCCI != nullptr)
			{
				if (pCCI->bEmpire != ch->GetEmpire())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1008]");
					return 0;
				}

				if (pCCI->bChannel != g_bChannel)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ���� %d ä�ο� �������Դϴ�. (���� ä��: %d)"),
						   strTargetName.c_str(), pCCI->bChannel, g_bChannel);
					return 0;
				}

				if (!IsMonarchWarpZone(pCCI->lMapIndex))
				{
					ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("�ش� �������� �̵��� �� �����ϴ�."));
					return 0;
				}
				if (!IsMonarchWarpZone(ch->GetMapIndex()))
				{
					ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("�ش� �������� ��ȯ�� �� �����ϴ�."));
					return 0;
				}

				TPacketMonarchGGTransfer packet{};
				packet.bHeader = HEADER_GG_MONARCH_TRANSFER;
				packet.dwTargetPID = pCCI->dwPID;
				packet.x = ch->GetX();
				packet.y = ch->GetY();

				P2P_MANAGER::Instance().Send(&packet, sizeof(TPacketMonarchGGTransfer));
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("��ȯ ��û�� ���½��ϴ�"));

				CMonarch::Instance().SendtoDBDecMoney(ciTransferCost, ch->GetEmpire(), ch);
				ch->SetMC(CHARACTER::MI_TRANSFER);
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1011]");
				return 0;
			}
		}
		else
		{
			if (pTargetChar == ch)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1012]");
				return 0;
			}

			if (pTargetChar->GetEmpire() != ch->GetEmpire())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1008]");
				return 0;
			}

			if (DISTANCE_APPROX(pTargetChar->GetX() - ch->GetX(), pTargetChar->GetY() - ch->GetY()) <= 5000)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ���� ��ó�� �ֽ��ϴ�"), pTargetChar->GetName());
				return 0;
			}

			if (!IsMonarchWarpZone(pTargetChar->GetMapIndex()))
			{
				ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("�ش� �������� �̵��� �� �����ϴ�."));
				return 0;
			}
			if (!IsMonarchWarpZone(ch->GetMapIndex()))
			{
				ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("�ش� �������� ��ȯ�� �� �����ϴ�."));
				return 0;
			}

			monarch_transfer2_event_info* info = AllocEventInfo<monarch_transfer2_event_info>();

			info->bHeader = HEADER_GG_MONARCH_TRANSFER;
			info->dwTargetPID = pTargetChar->GetPlayerID();
			info->x = ch->GetX();
			info->y = ch->GetY();

			event_create(monarch_transfer2_event, info, 1);

			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("��ȯ ��û�� ���½��ϴ�"));

			CMonarch::Instance().SendtoDBDecMoney(ciTransferCost, ch->GetEmpire(), ch);
			ch->SetMC(CHARACTER::MI_TRANSFER);
			return 0;
		}

		return 0;
	}

	void RegisterMonarchFunctionTable()
	{
		luaL_reg Monarch_functions[] =
		{
			{ "takemonarchmoney",		takemonarchmoney	},
			{ "isguildmaster",			is_guild_master		},
			{ "ismonarch",				is_monarch 			},
			{ "monarchbless",			monarch_bless		},
			{ "monarchpowerup",			monarch_powerup		},
			{ "monarchdefenseup",		monarch_defenseup	},
			{ "spawnmob",				spawn_mob			},
			{ "spawnguard",				spawn_guard			},
//			{ "frog_to_empire_money",	frog_to_empire_money		},
			{ "warp",					monarch_warp 		},
			{ "transfer",				monarch_transfer	},
			{ "transfer2",				monarch_transfer2	},
			{ "info",					empire_info 		},
			{ "notice",					monarch_notice		},
			{ "monarch_mob",			monarch_mob			},

			{ nullptr,						nullptr				}
		};

		CQuestManager::Instance().AddLuaFunctionTable("oh", Monarch_functions);
	}

}

