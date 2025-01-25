#include "stdafx.h"

#include "questlua.h"
#include "questmanager.h"
#include "char.h"

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

namespace quest
{
	//
	// "quest" Lua functions
	//
	ALUA(quest_start)
	{
		CQuestManager& q = CQuestManager::Instance();

		//q.GetPC(q.GetCurrentCharacterPtr()->GetPlayerID())->SetCurrentQuestStartFlag();
		q.GetCurrentPC()->SetCurrentQuestStartFlag();
		return 0;
	}

	ALUA(quest_done)
	{
		CQuestManager& q = CQuestManager::Instance();

		q.GetCurrentPC()->SetCurrentQuestDoneFlag();
		//q.GetPC(q.GetCurrentCharacterPtr()->GetPlayerID())->SetCurrentQuestDoneFlag();
		return 0;
	}

	ALUA(quest_set_title)
	{
		CQuestManager& q = CQuestManager::Instance();

		//q.GetPC(q.GetCurrentCharacterPtr()->GetPlayerID())->SetCurrentQuestTitle(lua_tostring(L,-1));
		if (lua_isstring(L,-1))
			q.GetCurrentPC()->SetCurrentQuestTitle(lua_tostring(L,-1));

		return 0;
	}

	ALUA(quest_set_another_title)
	{
		CQuestManager& q = CQuestManager::Instance();

		if (lua_isstring(L,1) && lua_isstring(L,2))
			q.GetCurrentPC()->SetQuestTitle(lua_tostring(L,1),lua_tostring(L,2));

		return 0;
	}

	ALUA(quest_set_clock_name)
	{
		CQuestManager& q = CQuestManager::Instance();

		//q.GetPC(q.GetCurrentCharacterPtr()->GetPlayerID())->SetCurrentQuestClockName(lua_tostring(L,-1));
		if (lua_isstring(L,-1))
			q.GetCurrentPC()->SetCurrentQuestClockName(lua_tostring(L,-1));

		return 0;
	}

	ALUA(quest_set_clock_value)
	{
		CQuestManager& q = CQuestManager::Instance();

		//q.GetPC(q.GetCurrentCharacterPtr()->GetPlayerID())->SetCurrentQuestClockValue((int)rint(lua_tonumber(L,-1)));
		if (lua_isnumber(L,-1))
			q.GetCurrentPC()->SetCurrentQuestClockValue((int)rint(lua_tonumber(L,-1)));

		return 0;
	}

	ALUA(quest_set_counter_name)
	{
		CQuestManager& q = CQuestManager::Instance();

		//q.GetPC(q.GetCurrentCharacterPtr()->GetPlayerID())->SetCurrentQuestCounterName(lua_tostring(L,-1));
		if (lua_isstring(L,-1))
			q.GetCurrentPC()->SetCurrentQuestCounterName(lua_tostring(L,-1));

		return 0;
	}

	ALUA(quest_set_counter_value)
	{
		CQuestManager& q = CQuestManager::Instance();

		//q.GetPC(q.GetCurrentCharacterPtr()->GetPlayerID())->SetCurrentQuestCounterValue((int)rint(lua_tonumber(L,-1)));
		if (lua_isnumber(L,-1))
			q.GetCurrentPC()->SetCurrentQuestCounterValue((int)rint(lua_tonumber(L,-1)));

		return 0;
	}

	ALUA(quest_set_icon_file)
	{
		CQuestManager& q = CQuestManager::Instance();

		//q.GetPC(q.GetCurrentCharacterPtr()->GetPlayerID())->SetCurrentQuestCounterValue((int)rint(lua_tonumber(L,-1)));
		if (lua_isstring(L,-1))
			q.GetCurrentPC()->SetCurrentQuestIconFile(lua_tostring(L,-1));

		return 0;
	}

	ALUA(quest_setstate)
	{
		if (lua_tostring(L, -1)==nullptr)
		{
			sys_err("state name is empty");
			return 0;
		}

		CQuestManager& q = CQuestManager::Instance();
		QuestState * pqs = q.GetCurrentState();
		PC* pPC = q.GetCurrentPC();
		//assert(L == pqs->co);

		if (L!=pqs->co)
		{
			luaL_error(L, "running thread != current thread???");
			if (test_server)
				sys_log(0 ,"running thread != current thread???");
			return 0;
		}

		if (pPC)
		{
			//pqs->st = lua_tostring(L, -1);
			//cerr << "QUEST new state" << pPC->GetCurrentQuestName(); << ":"
			//cerr <<  lua_tostring(L,-1);
			//cerr << endl;
			//
			std::string stCurrentState = lua_tostring(L,-1);
			if (test_server)
				sys_log ( 0 ,"questlua->setstate( %s, %s )", pPC->GetCurrentQuestName().c_str(), stCurrentState.c_str() );
			pqs->st = q.GetQuestStateIndex(pPC->GetCurrentQuestName(), stCurrentState);
			pPC->SetCurrentQuestStateName(stCurrentState );
		}
		return 0;
	}

	ALUA(quest_coroutine_yield)
	{
		CQuestManager& q = CQuestManager::Instance();
		// other_pc_block ���ο����� yield�� �Ͼ���� �ȵȴ�. �����.
		if (q.IsInOtherPCBlock())
		{
			sys_err("FATAL ERROR! Yield occur in other_pc_block.");
			PC* pPC = q.GetOtherPCBlockRootPC();
			if (nullptr == pPC)
			{
				sys_err("	... FFFAAATTTAAALLL Error. RootPC is nullptr");
				return 0;
			}
			QuestState* pQS = pPC->GetRunningQuestState();
			if (nullptr == pQS || nullptr == q.GetQuestStateName(pPC->GetCurrentQuestName(), pQS->st))
			{
				sys_err("	... WHO AM I? WHERE AM I? I only know QuestName(%s)...", pPC->GetCurrentQuestName().c_str());
			}
			else
			{
				sys_err("	Current Quest(%s). State(%s)", pPC->GetCurrentQuestName().c_str(), q.GetQuestStateName(pPC->GetCurrentQuestName(), pQS->st));
			}
			return 0;
		}
		return lua_yield(L, lua_gettop(L));
	}

	ALUA(quest_no_send)
	{
		CQuestManager& q = CQuestManager::Instance();
		q.SetNoSend();
		return 0;
	}

	ALUA(quest_get_current_quest_index)
	{
		CQuestManager& q = CQuestManager::Instance();
		PC* pPC = q.GetCurrentPC();

		int idx = q.GetQuestIndexByName(pPC->GetCurrentQuestName());
		lua_pushnumber(L, idx);
		return 1;
	}

#ifdef ENABLE_NEWSTUFF
	ALUA(quest_get_current_quest_name)
	{
		CQuestManager& q = CQuestManager::Instance();
		PC* pPC = q.GetCurrentPC();

		lua_pushstring(L, pPC->GetCurrentQuestName().c_str());
		return 1;
	}
#endif

	ALUA(quest_begin_other_pc_block)
	{
		CQuestManager& q = CQuestManager::Instance();
		uint32_t pid = lua_tonumber(L, -1);
		q.BeginOtherPCBlock(pid);
		return 0;
	}

	ALUA(quest_end_other_pc_block)
	{
		CQuestManager& q = CQuestManager::Instance();
		q.EndOtherPCBlock();
		return 0;
	}

	ALUA(quest_start_other_quest)
	{
		if (!lua_isstring(L, 1))
		{
			lua_pushboolean(L, 0);
			return 0;
		}

		CQuestManager& q = CQuestManager::Instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		
		if (ch == nullptr)
		{
			sys_err("no character instance found");
			lua_pushboolean(L, 0);
			
			return 0;
		}

		unsigned int qidx = q.GetQuestIndexByName(lua_tostring(L, 1));

		if (qidx == 0)
		{
			lua_pushboolean(L, 0);
			return 0;
		}

		//setting qf to verify its triggered by a quest
		ch->SetQuestFlag("quest_call.called_by_quest", 1);
		q.QuestButton(ch->GetPlayerID(), qidx);
		ch->SetQuestFlag("quest_call.called_by_quest", 0);
		
		lua_pushboolean(L, 1);
		
		return 1;
	}

	ALUA(quest_get_state_index)
	{

		if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
		{
			sys_err("wrong get state index flag");
		}

		lua_pushnumber(L, CQuestManager::Instance().GetQuestStateIndex(lua_tostring(L, 1), lua_tostring(L, 2)));
		return 1;
	}

#ifdef ENABLE_NEWSTUFF
	ALUA(quest_set_clock)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("set_clock invalid args");
			return 0;
		}

		sys_log(0, "locale_quest %d, time_remaining %d", lua_isnumber(L, 1), lua_isnumber(L, 2));
		return 1;
	}
#endif

	void RegisterQuestFunctionTable()
	{
		luaL_reg quest_functions[] =
		{
			{ "setstate",				quest_setstate				},
			{ "set_state",				quest_setstate				},
			{ "yield",					quest_coroutine_yield		},
			{ "set_title",				quest_set_title				},
			{ "set_title2",				quest_set_another_title		},
			{ "set_clock_name",			quest_set_clock_name		},
			{ "set_clock_value",		quest_set_clock_value		},
			{ "set_counter_name",		quest_set_counter_name		},
			{ "set_counter_value",		quest_set_counter_value		},
			{ "set_icon",				quest_set_icon_file			},
			{ "start",					quest_start					},
			{ "done",					quest_done					},
			{ "getcurrentquestindex",	quest_get_current_quest_index	},
#ifdef ENABLE_NEWSTUFF
			{ "getcurrentquestname",	quest_get_current_quest_name	},
#endif
			{ "no_send",				quest_no_send				},
			// begin_other_pc_block(pid), end_other_pc_block ���̸� other_pc_block�̶�� ����.
			// other_pc_block������ current_pc�� pid�� ����ȴ�.
			//						������ �ٽ� ������ current_pc�� ���ư���.
			/*		�̷� ���� ���� ����.
					for i, pid in next, pids, nil do
						q.begin_other_pc_block(pid)
						if pc.count_item(PASS_TICKET) < 1 then
							table.insert(criminalNames, pc.get_name())
							canPass = false
						end
						q.end_other_pc_block()
					end
			*/
			// ���� : other_pc_block ���ο����� ����� yield�� �Ͼ���� �ȵȴ�.(ex. wait, select, input, ...)
			{ "begin_other_pc_block",	quest_begin_other_pc_block	},
			{ "end_other_pc_block",		quest_end_other_pc_block	},
			{ "start_other_quest", quest_start_other_quest },
			{ "get_state_index", quest_get_state_index },

#ifdef ENABLE_NEWSTUFF
			{ "set_clock", quest_set_clock },
#endif

			{ nullptr,						nullptr						}
		};

		CQuestManager::Instance().AddLuaFunctionTable("q", quest_functions);
	}
}




