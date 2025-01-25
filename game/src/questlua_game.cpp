#include "stdafx.h"
#include "questlua.h"
#include "questmanager.h"
#include "desc_client.h"
#include "char.h"
#include "item_manager.h"
#include "item.h"
#include "cmd.h"
#include "packet.h"
#ifdef ENABLE_DICE_SYSTEM
#	include "party.h"
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
#	include "minigame_manager.h"
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
#	include "Transmutation.h"
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	#include "AchievementSystem.h"
#endif

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

extern ACMD(do_in_game_mall);

namespace quest
{
	ALUA(game_set_event_flag)
	{
		CQuestManager & q = CQuestManager::Instance();

		if (lua_isstring(L,1) && lua_isnumber(L, 2))
			q.RequestSetEventFlag(lua_tostring(L,1), (int)lua_tonumber(L,2));

		return 0;
	}

	ALUA(game_get_event_flag)
	{
		CQuestManager& q = CQuestManager::Instance();

		if (lua_isstring(L,1))
			lua_pushnumber(L, q.GetEventFlag(lua_tostring(L,1)));
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(game_request_make_guild)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPDESC d = q.GetCurrentCharacterPtr()->GetDesc();
		if (d)
		{
			uint8_t header = HEADER_GC_REQUEST_MAKE_GUILD;
			d->Packet(&header, 1);
		}
		return 0;
	}

	ALUA(game_get_safebox_level)
	{
		CQuestManager& q = CQuestManager::Instance();
		lua_pushnumber(L, q.GetCurrentCharacterPtr()->GetSafeboxSize()/SAFEBOX_PAGE_SIZE);
		return 1;
	}

	ALUA(game_set_safebox_level)
	{
		CQuestManager& q = CQuestManager::Instance();

		//q.GetCurrentCharacterPtr()->ChangeSafeboxSize(3*(int)lua_tonumber(L,-1));
		TSafeboxChangeSizePacket p;
		p.dwID = q.GetCurrentCharacterPtr()->GetDesc()->GetAccountTable().id;
		p.bSize = (int)lua_tonumber(L,-1);
		db_clientdesc->DBPacket(HEADER_GD_SAFEBOX_CHANGE_SIZE,  q.GetCurrentCharacterPtr()->GetDesc()->GetHandle(), &p, sizeof(p));

		q.GetCurrentCharacterPtr()->SetSafeboxSize(SAFEBOX_PAGE_SIZE * (int)lua_tonumber(L,-1));
		return 0;
	}

	ALUA(game_open_safebox)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		ch->SetSafeboxOpenPosition();
		ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeSafeboxPassword");
		return 0;
	}

	ALUA(game_open_mall)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		ch->SetSafeboxOpenPosition();
		ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeMallPassword");
		return 0;
	}

	ALUA(game_drop_item)
	{
		//
		// Syntax: game.drop_item(50050, 1)
		//
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		uint32_t item_vnum = (uint32_t) lua_tonumber(L, 1);
		int count = (int) lua_tonumber(L, 2);
		long x = ch->GetX();
		long y = ch->GetY();

		LPITEM item = ITEM_MANAGER::Instance().CreateItem(item_vnum, count);

		if (!item)
		{
			sys_err("cannot create item vnum %d count %d", item_vnum, count);
			return 0;
		}

		PIXEL_POSITION pos;
		pos.x = x + number(-200, 200);
		pos.y = y + number(-200, 200);

		item->AddToGround(ch->GetMapIndex(), pos);
		item->StartDestroyEvent();

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		CAchievementSystem::Instance().Collect(ch, achievements::ETaskTypes::TYPE_COLLECT, item->GetVnum(), item->GetCount());
#endif

		return 0;
	}

	ALUA(game_drop_item_with_ownership)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		LPITEM item = nullptr;
		switch (lua_gettop(L))
		{
		case 1:
			item = ITEM_MANAGER::Instance().CreateItem((uint32_t) lua_tonumber(L, 1));
			break;
		case 2:
		case 3:
			item = ITEM_MANAGER::Instance().CreateItem((uint32_t) lua_tonumber(L, 1), (int) lua_tonumber(L, 2));
			break;
		default:
			return 0;
		}

		if ( item == nullptr )
		{
			return 0;
		}

		if (lua_isnumber(L, 3))
		{
			int sec = (int) lua_tonumber(L, 3);
			if (sec <= 0)
			{
				item->SetOwnership( ch );
			}
			else
			{
				item->SetOwnership( ch, sec );
			}
		}
		else
			item->SetOwnership( ch );

		PIXEL_POSITION pos;
		pos.x = ch->GetX() + number(-200, 200);
		pos.y = ch->GetY() + number(-200, 200);

		item->AddToGround(ch->GetMapIndex(), pos);
		item->StartDestroyEvent();

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		CAchievementSystem::Instance().Collect(ch, achievements::ETaskTypes::TYPE_COLLECT, item->GetVnum(), item->GetCount());
#endif

		return 0;
	}

#ifdef ENABLE_DICE_SYSTEM
	ALUA(game_drop_item_with_ownership_and_dice)
	{
		LPITEM item = nullptr;
		switch (lua_gettop(L))
		{
		case 1:
			item = ITEM_MANAGER::Instance().CreateItem((uint32_t) lua_tonumber(L, 1));
			break;
		case 2:
		case 3:
			item = ITEM_MANAGER::Instance().CreateItem((uint32_t) lua_tonumber(L, 1), (int) lua_tonumber(L, 2));
			break;
		default:
			return 0;
		}

		if ( item == nullptr )
		{
			return 0;
		}

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch->GetParty())
		{
			FPartyDropDiceRoll f(item, ch);
			f.Process(nullptr);
		}

		if (lua_isnumber(L, 3))
		{
			int sec = (int) lua_tonumber(L, 3);
			if (sec <= 0)
			{
				item->SetOwnership( ch );
			}
			else
			{
				item->SetOwnership( ch, sec );
			}
		}
		else
			item->SetOwnership( ch );

		PIXEL_POSITION pos;
		pos.x = ch->GetX() + number(-200, 200);
		pos.y = ch->GetY() + number(-200, 200);

		item->AddToGround(ch->GetMapIndex(), pos);
		item->StartDestroyEvent();

		return 0;
	}
#endif

	ALUA(game_web_mall)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if ( ch != nullptr )
		{
			do_in_game_mall(ch, const_cast<char*>(""), 0, 0);
		}
		return 0;
	}

#ifdef ENABLE_12ZI
	ALUA(game_open_zodiac_temple_table)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		ch->ZTT_LOAD_INFO();
		return 0;
	}
#endif

#ifdef ENABLE_ATTR_6TH_7TH
	ALUA(game_open_skillbook_comb)	//Fragments
	{
		CQuestManager& q = CQuestManager::Instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();

		if (!ch)
			return 0;

		if (ch->IsOpenSkillBookComb())
			return 0;

		ch->SetSkillBookCombType(1);
		ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenSkillbookCombinationDialog");
		return 0;
	}

	ALUA(game_open_skillbook_comb_books)	//Skillbooks
	{
		CQuestManager& q = CQuestManager::Instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();

		if (!ch)
			return 0;

		if (ch->IsOpenSkillBookComb())
			return 0;

		ch->SetSkillBookCombType(2);
		ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenSkillbookCombinationDialog");
		return 0;
	}

	ALUA(game_open_attr67_add)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();

		if (!ch)
			return 0;

		if (ch->IsOpenSkillBookComb())
			return 0;

		ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenAttr67AddDlg");
		return 0;
	}
#endif

#ifdef ENABLE_MOVE_COSTUME_ATTR
	ALUA(game_open_move_costume)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		//PREVENT_TRADE_WINDOW
		if (ch->IsOpenSafebox() || ch->GetExchange() || ch->GetMyShop() || ch->IsCubeOpen())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;876]");
			return 0;
		}
		//END_PREVENT_TRADE_WINDOW

		ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowItemCombinationDialog");
		return 0;
	}
#endif

#ifdef ENABLE_AURA_SYSTEM
	ALUA(game_open_aura_absorb_window)
	{
		const LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (nullptr == ch)
		{
			sys_err("nullptr POINT ERROR");
			return 0;
		}

		ch->OpenAuraRefineWindow(CQuestManager::Instance().GetCurrentNPCCharacterPtr(), AURA_WINDOW_TYPE_ABSORB);
		return 0;
	}

	ALUA(game_open_aura_growth_window)
	{
		const LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (nullptr == ch)
		{
			sys_err("nullptr POINT ERROR");
			return 0;
		}

		ch->OpenAuraRefineWindow(CQuestManager::Instance().GetCurrentNPCCharacterPtr(), AURA_WINDOW_TYPE_GROWTH);
		return 0;
	}

	ALUA(game_open_aura_evolve_window)
	{
		const LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (nullptr == ch)
		{
			sys_err("nullptr POINT ERROR");
			return 0;
		}

		ch->OpenAuraRefineWindow(CQuestManager::Instance().GetCurrentNPCCharacterPtr(), AURA_WINDOW_TYPE_EVOLVE);
		return 0;
	}
#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
	ALUA(mini_game_catch_king_get_score)
	{
		uint32_t dwArg = (uint32_t)lua_tonumber(L, 1);
		bool isTotal = dwArg ? true : false;

		CMiniGameManager::Instance().MiniGameCatchKingGetScore(L, isTotal);
		return 1;
	}
#endif

#ifdef ENABLE_MAILBOX
	ALUA(game_open_mailbox)
	{
		CMailBox::Open(CQuestManager::Instance().GetCurrentCharacterPtr());
		return 0;
	}
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	ALUA(game_open_transmutation)
	{
		if (lua_isboolean(L, 1))
		{
			const LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
			const bool bType = lua_toboolean(L, 1);
			CTransmutation::Open(ch, bType);
		}

		return 0;
	}
#endif

#ifdef ENABLE_MINI_GAME_FINDM
	ALUA(mini_game_find_m_get_score)
	{
		uint32_t dwArg = (uint32_t)lua_tonumber(L, 1);
		bool isTry = dwArg ? true : false;

		CMiniGameManager::instance().MiniGameFindMGetScore(L, isTry);
		return 1;
	}
#endif

#ifdef ENABLE_MINI_GAME_YUTNORI
	ALUA(mini_game_yut_nori_get_score)
	{
		CMiniGameManager::instance().MiniGameYutNoriGetScore(L);
		return 1;
	}

	ALUA(mini_game_yut_nori_get_my_score)
	{
		lua_pushnumber(L, CMiniGameManager::instance().MiniGameYutNoriGetMyScore(CQuestManager::instance().GetCurrentCharacterPtr()));
		return 1;
	}
#endif

	void RegisterGameFunctionTable()
	{
		luaL_reg game_functions[] =
		{
			{ "get_safebox_level",			game_get_safebox_level			},
			{ "request_make_guild",			game_request_make_guild			},
			{ "set_safebox_level",			game_set_safebox_level			},
			{ "open_safebox",				game_open_safebox				},
			{ "open_mall",					game_open_mall					},
			{ "get_event_flag",				game_get_event_flag				},
			{ "set_event_flag",				game_set_event_flag				},
			{ "drop_item",					game_drop_item					},
			{ "drop_item_with_ownership",	game_drop_item_with_ownership	},
#ifdef ENABLE_DICE_SYSTEM
			{ "drop_item_with_ownership_and_dice",	game_drop_item_with_ownership_and_dice	},
#endif
			{ "open_web_mall",				game_web_mall					},
#ifdef ENABLE_12ZI
			{ "zodiac_temple_table",		game_open_zodiac_temple_table	},
#endif
#ifdef ENABLE_ATTR_6TH_7TH
			{ "open_skillbook_comb", game_open_skillbook_comb },
			{ "open_skillbook_comb_books", game_open_skillbook_comb_books },
			{ "open_attr67_add", game_open_attr67_add },
#endif
#ifdef ENABLE_MOVE_COSTUME_ATTR
			{ "open_move_costume", game_open_move_costume },
#endif

#ifdef ENABLE_AURA_SYSTEM
			{ "open_aura_absorb_window",	game_open_aura_absorb_window	},
			{ "open_aura_growth_window",	game_open_aura_growth_window	},
			{ "open_aura_evolve_window",	game_open_aura_evolve_window	},
#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
			{ "mini_game_catch_king_get_score",	mini_game_catch_king_get_score		},
#endif

#ifdef ENABLE_MAILBOX
			{ "open_mailbox",				game_open_mailbox				},
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			{ "open_transmutation",			game_open_transmutation			},
#endif

#ifdef ENABLE_MINI_GAME_FINDM
			{ "mini_game_find_m_get_score",	mini_game_find_m_get_score		},
#endif

#ifdef ENABLE_MINI_GAME_YUTNORI
			{ "mini_game_yut_nori_get_score",		mini_game_yut_nori_get_score			},
			{ "mini_game_yut_nori_get_my_score",	mini_game_yut_nori_get_my_score			},
#endif

			{ nullptr,					nullptr				}
		};

		CQuestManager::Instance().AddLuaFunctionTable("game", game_functions);
	}
}

