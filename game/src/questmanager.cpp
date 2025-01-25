#include "stdafx.h"
#include <fstream>
#include "constants.h"
#include "buffer_manager.h"
#include "packet.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "questmanager.h"
#include "text_file_loader.h"
#include "lzo_manager.h"
#include "item.h"
#include "config.h"
#include "xmas_event.h"
#include "target.h"
#include "party.h"
#include "locale_service.h"
#include "dungeon.h"
#ifdef ENABLE_12ZI
#	include "zodiac_temple.h"
#endif
#ifdef ENABLE_QUEST_RENEWAL
	#include <vector>
	#include <iostream>
	#include <sstream>
#endif
#ifdef ENABLE_MONSTER_BACK
#	include "minigame_manager.h"
#endif
#if defined(ENABLE_EVENT_BANNER_FLAG) && defined(ENABLE_EVENT_MANAGER)
#	include "event_manager.h"
#endif
#ifdef ENABLE_SUNG_MAHI_TOWER
# include "db.h"
#endif

uint32_t g_GoldDropTimeLimitValue = 0;
#ifdef ENABLE_NEWSTUFF
uint32_t g_ItemDropTimeLimitValue = 0;
uint32_t g_BoxUseTimeLimitValue = 0;
uint32_t g_BuySellTimeLimitValue = 0;
bool g_NoDropMetinStone = false;
bool g_NoMountAtGuildWar = false;
bool g_NoPotionsOnPVP = false;
#endif
extern bool DropEvent_CharStone_SetValue(const std::string& name, int value);
extern bool DropEvent_RefineBox_SetValue (const std::string& name, int value);

namespace quest
{
	using namespace std;

	CQuestManager::CQuestManager()
		: m_pSelectedDungeon(nullptr),
#ifdef ENABLE_12ZI
		m_pSelectedZodiac(nullptr),
#endif
		m_dwServerTimerArg(0), m_iRunningEventIndex(0), L(nullptr), m_bNoSend (false),
		m_CurrentRunningState(nullptr), m_pCurrentCharacter(nullptr), m_pCurrentNPCCharacter(nullptr), m_pCurrentPartyMember(nullptr),
		m_pCurrentPC(nullptr),  m_iCurrentSkin(0), m_bError(false), m_pOtherPCBlockRootPC(nullptr)
	{
	}

	CQuestManager::~CQuestManager()
	{
		Destroy();
	}

	void CQuestManager::Destroy()
	{
		if (L)
		{
			lua_close(L);
			L = nullptr;
		}

#ifdef ENABLE_SUNG_MAHI_TOWER
		if (m_sung_mahi_reward_event)
		{
			event_cancel(&m_sung_mahi_reward_event);
			m_sung_mahi_reward_event = nullptr;
		}
#endif
	}

#ifdef ENABLE_SUNG_MAHI_TOWER
	EVENTINFO(SungMahiMonthlyRewardInfo)
	{
		int iLastMonth;
	};

	EVENTFUNC(SungMahiMonthRewardTimer)
	{
		SungMahiMonthlyRewardInfo* info = dynamic_cast<SungMahiMonthlyRewardInfo*>(event->info);
		if (info == NULL)
		{
			sys_err("SungMahiMonthRewardTimer> <Factor> Null pointer");
			return 0;
		}

		time_t cur_Time = time(NULL);
		struct tm vKey = *localtime(&cur_Time);
		int currentMonth = vKey.tm_mon;

		if (quest::CQuestManager::instance().GetEventFlag("sungMahiLastMonth") != currentMonth)
		{
			for (auto index = 1; index <= SUNG_MAHI_MAX_LEVEL; index++)
			{
				auto pMsg(DBManager::instance().DirectQuery("SELECT player_login FROM `sung_mahi_ranking` WHERE tower_level = %d ORDER BY tower_time ASC LIMIT 1", index));
				if (pMsg->Get()->uiNumRows == 0)
					continue;

				MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
				bool isElementStage = (index % 10 < 1);
				int iCount = (isElementStage ? (5 * (index / 10)) : (1 + (index / 10)));

#ifdef ENABLE_MAILBOX
				TMailBoxTable p;
				std::memcpy(p.szName, row[0], sizeof(p.szName));
				p.bIsDeleted = false;

				// Message
				std::memcpy(p.Message.szTitle, "Sung Mahi Tower Month Reward", sizeof(p.Message.szTitle));
				p.Message.bIsGMPost = true;
				p.Message.bIsConfirm = false;
				p.Message.SendTime = time(nullptr);
				p.Message.DeleteTime = p.Message.SendTime + EMAILBOX::MAILBOX_REMAIN_REWARD * 60 * 60 * 24;

				// ItemData
				p.AddData.bHeader = 0;
				p.AddData.Index = 0;
				std::memcpy(p.AddData.szFrom, "Sung Mahi Tower", sizeof(p.AddData.szFrom));
				std::memcpy(p.AddData.szMessage, "", sizeof(p.AddData.szMessage));
				p.AddData.iYang = 0;
				p.AddData.iWon = 0;
				p.AddData.ItemVnum = 50249;
				p.AddData.ItemCount = iCount;

				memset(p.AddData.alSockets, 0, sizeof(p.AddData.alSockets));
				memset(p.AddData.aAttr, 0, sizeof(p.AddData.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
				memset(p.AddData.aApplyRandom, 0, sizeof(p.AddData.aApplyRandom));
				memset(p.AddData.alRandomValues, 0, sizeof(p.AddData.alRandomValues));
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
				p.AddData.dwTransmutationVnum = 0;
#endif
#ifdef ENABLE_REFINE_ELEMENT
				p.AddData.grade_element = 0;
				memset(p.AddData.attack_element, 0, sizeof(p.AddData.attack_element));
				p.AddData.element_type_bonus = 0;
				memset(p.AddData.elements_value_bonus, 0, sizeof(p.AddData.elements_value_bonus));
#endif
#ifdef ENABLE_SET_ITEM
				p.AddData.set_value = 0;
#endif

				p.Message.bIsItemExist = p.AddData.ItemVnum > 0;

				db_clientdesc->DBPacket(HEADER_GD_MAILBOX_WRITE, 0, &p, sizeof(p));
#else
				DBManager::instance().Query("INSERT INTO item_award (login, vnum, count, given_time, why, mall) VALUES ('%s', %d, %d, now(), '%s', 1);", row[0], 50249, iCount, "month_sung_mahi_reward");
#endif
			}

			DBManager::instance().Query("TRUNCATE TABLE `sung_mahi_ranking`;");
			quest::CQuestManager::instance().RequestSetEventFlag("sungMahiLastMonth", currentMonth);

			// dofile to clear the set
			{
				char questDungeonInformation[256];
				snprintf(questDungeonInformation, sizeof(questDungeonInformation), "%s/Questlibs/dungeonInfoLibrary.lua", LocaleService_GetQuestPath().c_str());
				lua_dofile(quest::CQuestManager::instance().GetLuaState(), questDungeonInformation);
			}

			BroadcastNotice(LC_TEXT("Sungma Tower Ranking Season has ended!"));
		}

		return PASSES_PER_SEC(60);
	}
#endif

	bool CQuestManager::Initialize()
	{
		if (g_bAuthServer)
			return true;

		if (!InitializeLua())
			return false;

		m_pSelectedDungeon = nullptr;
#ifdef ENABLE_12ZI
		m_pSelectedZodiac = nullptr;
#endif

		m_mapEventName.insert(TEventNameMap::value_type("click", QUEST_CLICK_EVENT));		// NPC를 클릭
		m_mapEventName.insert(TEventNameMap::value_type("kill", QUEST_KILL_EVENT));		// Mob을 사냥
		m_mapEventName.insert(TEventNameMap::value_type("timer", QUEST_TIMER_EVENT));		// 미리 지정해둔 시간이 지남
		m_mapEventName.insert(TEventNameMap::value_type("levelup", QUEST_LEVELUP_EVENT));	// 레벨업을 함
		m_mapEventName.insert(TEventNameMap::value_type("login", QUEST_LOGIN_EVENT));		// 로그인 시
		m_mapEventName.insert(TEventNameMap::value_type("logout", QUEST_LOGOUT_EVENT));		// 로그아웃 시
		m_mapEventName.insert(TEventNameMap::value_type("button", QUEST_BUTTON_EVENT));		// 퀘스트 버튼을 누름
		m_mapEventName.insert(TEventNameMap::value_type("info", QUEST_INFO_EVENT));		// 퀘스트 정보창을 염
		m_mapEventName.insert(TEventNameMap::value_type("chat", QUEST_CHAT_EVENT));		// 특정 키워드로 대화를 함
		m_mapEventName.insert(TEventNameMap::value_type("in", QUEST_ATTR_IN_EVENT));		// 맵의 특정 속성에 들어감
		m_mapEventName.insert(TEventNameMap::value_type("out", QUEST_ATTR_OUT_EVENT));		// 맵의 특정 속성에서 나옴
		m_mapEventName.insert(TEventNameMap::value_type("use", QUEST_ITEM_USE_EVENT));		// 퀘스트 아이템을 사용
		m_mapEventName.insert(TEventNameMap::value_type("server_timer", QUEST_SERVER_TIMER_EVENT));	// 서버 타이머 (아직 테스트 안됐음)
		m_mapEventName.insert(TEventNameMap::value_type("enter", QUEST_ENTER_STATE_EVENT));	// 현재 스테이트가 됨
		m_mapEventName.insert(TEventNameMap::value_type("leave", QUEST_LEAVE_STATE_EVENT));	// 현재 스테이트에서 다른 스테이트로 바뀜
		m_mapEventName.insert(TEventNameMap::value_type("letter", QUEST_LETTER_EVENT));		// 로긴 하거나 스테이트가 바껴 새로 정보를 세팅해줘야함
		m_mapEventName.insert(TEventNameMap::value_type("take", QUEST_ITEM_TAKE_EVENT));	// 아이템을 받음
		m_mapEventName.insert(TEventNameMap::value_type("target", QUEST_TARGET_EVENT));		// 타겟
		m_mapEventName.insert(TEventNameMap::value_type("party_kill", QUEST_PARTY_KILL_EVENT));	// 파티 멤버가 몬스터를 사냥 (리더에게 옴)
		m_mapEventName.insert(TEventNameMap::value_type("unmount", QUEST_UNMOUNT_EVENT));
		m_mapEventName.insert(TEventNameMap::value_type("pick", QUEST_ITEM_PICK_EVENT));	// 떨어져있는 아이템을 습득함.
		m_mapEventName.insert(TEventNameMap::value_type("sig_use", QUEST_SIG_USE_EVENT));		// Special item group에 속한 아이템을 사용함.
		m_mapEventName.insert(TEventNameMap::value_type("item_informer", QUEST_ITEM_INFORMER_EVENT));	// 독일선물기능테스트
#ifdef ENABLE_QUEST_DIE_EVENT
		m_mapEventName.insert(TEventNameMap::value_type("die", QUEST_DIE_EVENT));
#endif
#ifdef ENABLE_QUEST_BOOT_EVENT
		m_mapEventName.insert(TEventNameMap::value_type("boot", QUEST_BOOT_EVENT));
#endif
#ifdef ENABLE_QUEST_DND_EVENT
		m_mapEventName.insert(TEventNameMap::value_type("dnd", QUEST_DND_EVENT));
#endif
#ifdef ENABLE_EVENT_MANAGER
		m_mapEventName.insert(TEventNameMap::value_type("event_begin", QUEST_GAME_EVENT_BEGIN)); // Event for when a game event is started
		m_mapEventName.insert(TEventNameMap::value_type("event_end", QUEST_GAME_EVENT_END)); // Event for when a game event is ended
#endif

		m_bNoSend = false;

		m_iCurrentSkin = QUEST_SKIN_NORMAL;

		{
			ifstream inf((g_stQuestDir + "/questnpc.txt").c_str());
			int line = 0;

			if (!inf.is_open())
				sys_err( "QUEST Cannot open 'questnpc.txt'");
			else
				sys_log(0, "QUEST can open 'questnpc.txt' (%s)", g_stQuestDir.c_str() );

			while (1)
			{
				unsigned int vnum;

				inf >> vnum;

				line++;

				if (inf.fail())
					break;

				string s;
				getline(inf, s);
				unsigned int li = 0, ri = s.size()-1;
				while (li < s.size() && isspace(s[li])) li++;
				while (ri > 0 && isspace(s[ri])) ri--;

				if (ri < li)
				{
					sys_err("QUEST questnpc.txt:%d:npc name error",line);
					continue;
				}

				s = s.substr(li, ri-li+1);

				int	n = 0;
				str_to_number(n, s.c_str());
				if (n)
					continue;

				//cout << '-' << s << '-' << endl;
				if (test_server)
					sys_log(0, "QUEST reading script of %s(%d)", s.c_str(), vnum);
				m_mapNPC[vnum].Set(vnum, s);
				m_mapNPCNameID[s] = vnum;
			}

			// notarget quest
			m_mapNPC[0].Set(0, "notarget");
		}

		SetEventFlag("guild_withdraw_delay", 1);
		SetEventFlag("guild_disband_delay", 1);
#ifdef ENABLE_QUEST_RENEWAL
		ReadQuestCategoryToDict();
#endif

#ifdef ENABLE_SUNG_MAHI_TOWER
		if (strcmp(g_stHostname.c_str(), "game-ch99-core99") == 0)
		{
			SungMahiMonthlyRewardInfo* info = AllocEventInfo<SungMahiMonthlyRewardInfo>();
			info->iLastMonth = 0;
			m_sung_mahi_reward_event = event_create(SungMahiMonthRewardTimer, info, PASSES_PER_SEC(60));
		}
#endif

		return true;
	}

	unsigned int CQuestManager::FindNPCIDByName(const string& name)
	{
		map<string, unsigned int>::iterator it = m_mapNPCNameID.find(name);
		return it != m_mapNPCNameID.end() ? it->second : 0;
	}

	void CQuestManager::SelectItem(unsigned int pc, unsigned int selection)
	{
		PC* pPC = GetPC(pc);

		if (pPC && pPC->IsRunning() && pPC->GetRunningQuestState()->suspend_state == SUSPEND_STATE_SELECT_ITEM)
		{
			pPC->SetSendDoneFlag();
			pPC->GetRunningQuestState()->args = 1;
			lua_pushnumber(pPC->GetRunningQuestState()->co, selection);

			if (!RunState(*pPC->GetRunningQuestState()))
			{
				CloseState(*pPC->GetRunningQuestState());
				pPC->EndRunning();
			}
		}
	}

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	void CQuestManager::SelectItemEx(unsigned int pc, unsigned int selection)
	{
		PC* pPC = GetPC(pc);
		if (pPC && pPC->IsRunning() && pPC->GetRunningQuestState()->suspend_state == SUSPEND_STATE_SELECT_ITEM_EX)
		{
			pPC->SetSendDoneFlag();
			pPC->GetRunningQuestState()->args = 1;
			lua_pushnumber(pPC->GetRunningQuestState()->co, selection);

			if (!RunState(*pPC->GetRunningQuestState()))
			{
				CloseState(*pPC->GetRunningQuestState());
				pPC->EndRunning();
			}
		}
	}
#endif

	void CQuestManager::Confirm(unsigned int pc, EQuestConfirmType confirm, unsigned int pc2)
	{
		PC* pPC = GetPC(pc);

		if (!pPC->IsRunning())
		{
			sys_err("no quest running for pc, cannot process input : %u", pc);
			return;
		}

		if (pPC->GetRunningQuestState()->suspend_state != SUSPEND_STATE_CONFIRM)
		{
			sys_err("not wait for a confirm : %u %d", pc, pPC->GetRunningQuestState()->suspend_state);
			return;
		}

		if (pc2 && !pPC->IsConfirmWait(pc2))
		{
			sys_err("not wait for a confirm : %u %d", pc, pPC->GetRunningQuestState()->suspend_state);
			return;
		}

		pPC->ClearConfirmWait();

		pPC->SetSendDoneFlag();

		pPC->GetRunningQuestState()->args=1;
		lua_pushnumber(pPC->GetRunningQuestState()->co, confirm);

		AddScript("[END_CONFIRM_WAIT]");
		SetSkinStyle(QUEST_SKIN_NOWINDOW);
		SendScript();

		if (!RunState(*pPC->GetRunningQuestState()))
		{
			CloseState(*pPC->GetRunningQuestState());
			pPC->EndRunning();
		}

	}

	int CQuestManager::ReadQuestCategoryFile(uint16_t q_index)
	{

		ifstream inf((g_stQuestDir + "/questcategory.txt").c_str());
		int line = 0;
		int c_qi = 99;

		if (!inf.is_open())
			sys_err( "QUEST Cannot open 'questcategory.txt'");
		else
			sys_log(0, "QUEST can open 'questcategory.txt' (%s)", g_stQuestDir.c_str() );

		while (1)
		{
			//받은 quest_index를 quest_name로 변환 후 비교
			string qn = CQuestManager::Instance().GetQuestNameByIndex(q_index);

			unsigned int category_num;

			//enum
			//{
			//	MAIN_QUEST,		//0
			//	SUB_QUEST,		//1
			//	COLLECT_QUEST,	//2
			//	LEVELUP_QUEST,	//3
			//	SCROLL_QUEST,	//4
			//	SYSTEM_QUEST,	//5
			//};

			inf >> category_num;

			line++;

			if (inf.fail())
				break;

			string s;
			getline(inf, s);
			unsigned int li = 0, ri = s.size()-1;
			while (li < s.size() && isspace(s[li])) li++;
			while (ri > 0 && isspace(s[ri])) ri--;

			if (ri < li)
			{
				sys_err("QUEST questcategory.txt:%d:npc name error",line);
				continue;
			}

			s = s.substr(li, ri-li+1);

			int	n = 0;
			str_to_number(n, s.c_str());
			if (n)
				continue;

			//cout << '-' << s << '-' << endl;
			if (test_server)
				sys_log(0, "QUEST reading script of %s(%d)", s.c_str(), category_num);

			if (qn == s)
			{
				c_qi = category_num;
				break;
			}
		}

		// notarget quest
		//m_mapNPC[0].Set(0, "notarget");


		//enum 순서대로 카테고리 인덱스를 리턴
		return c_qi;
	}

#ifdef ENABLE_QUEST_RENEWAL
	int CQuestManager::GetQuestCategoryByQuestIndex(uint16_t q_index)
	{
		if (QuestCategoryIndexMap.find(q_index) != QuestCategoryIndexMap.end())
			return QuestCategoryIndexMap[q_index];
		else
			return 0; /* DEFAULT_QUEST_CATEGORY */
	}

	void CQuestManager::ReadQuestCategoryToDict()
	{
		if (!QuestCategoryIndexMap.empty())
			QuestCategoryIndexMap.clear();

		ifstream inf((g_stQuestDir + "/questcategory.txt").c_str());

		if (!inf.is_open())
		{
			sys_err("QUEST Cannot open 'questcategory.txt'");
			return;
		}

		string lineFromFile;
		while (getline(inf, lineFromFile))
		{
			if (lineFromFile.empty())
				continue;

			std::stringstream strstr(lineFromFile);
			std::istream_iterator<std::string> it(strstr);
			std::istream_iterator<std::string> end;
			std::vector<std::string> data(it, end);

			std::ostream_iterator<std::string> oit(std::cout);
			std::copy(data.begin(), data.end(), oit);

			int category_num = atoi(data[0].c_str());
			string quest_name = data[1];

			unsigned int quest_index = CQuestManager::Instance().GetQuestIndexByName(quest_name);

			if (test_server)
				sys_log(0, "QUEST_CATEGORY_LINE: %s => %s, %s", lineFromFile.c_str(), data[0].c_str(), quest_name.c_str());

			if (quest_index != 0)
				QuestCategoryIndexMap[quest_index] = category_num;
			else
				sys_err("QUEST coult not find QuestIndex for name Quest: %s(%d)", quest_name.c_str(), category_num);
		}
	}
#endif

	void CQuestManager::Input(unsigned int pc, const char* msg)
	{
		PC* pPC = GetPC(pc);
		if (!pPC)
		{
			sys_err("no pc! : %u",pc);
			return;
		}

		if (!pPC->IsRunning())
		{
			sys_err("no quest running for pc, cannot process input : %u", pc);
			return;
		}

		if (pPC->GetRunningQuestState()->suspend_state != SUSPEND_STATE_INPUT)
		{
			sys_err("not wait for a input : %u %d", pc, pPC->GetRunningQuestState()->suspend_state);
			return;
		}

		pPC->SetSendDoneFlag();

		pPC->GetRunningQuestState()->args=1;
		lua_pushstring(pPC->GetRunningQuestState()->co,msg);

		if (!RunState(*pPC->GetRunningQuestState()))
		{
			CloseState(*pPC->GetRunningQuestState());
			pPC->EndRunning();
		}
	}

	void CQuestManager::Select(unsigned int pc, unsigned int selection)
	{
		PC* pPC;

		if ((pPC = GetPC(pc)) && pPC->IsRunning() && pPC->GetRunningQuestState()->suspend_state==SUSPEND_STATE_SELECT)
		{
			pPC->SetSendDoneFlag();

			if (!pPC->GetRunningQuestState()->chat_scripts.empty())
			{
				// 채팅 이벤트인 경우
				// 현재 퀘스트는 어느 퀘스트를 실행할 것인가를 고르는 퀘스트 이므로
				// 끝내고 선택된 퀘스트를 실행한다.
				QuestState& old_qs = *pPC->GetRunningQuestState();
				CloseState(old_qs);

				if (selection >= pPC->GetRunningQuestState()->chat_scripts.size())
				{
					pPC->SetSendDoneFlag();
					GotoEndState(old_qs);
					pPC->EndRunning();
				}
				else
				{
					AArgScript* pas = pPC->GetRunningQuestState()->chat_scripts[selection];
					ExecuteQuestScript(*pPC, pas->quest_index, pas->state_index, pas->script.GetCode(), pas->script.GetSize());
				}
			}
			else
			{
				// on default
				pPC->GetRunningQuestState()->args=1;
				lua_pushnumber(pPC->GetRunningQuestState()->co,selection+1);

				if (!RunState(*pPC->GetRunningQuestState()))
				{
					CloseState(*pPC->GetRunningQuestState());
					pPC->EndRunning();
				}
			}
		}
		else
		{
			sys_err("wrong QUEST_SELECT request! : %d",pc);
		}
	}

	void CQuestManager::Resume(unsigned int pc)
	{
		PC * pPC;

		if ((pPC = GetPC(pc)) && pPC->IsRunning() && pPC->GetRunningQuestState()->suspend_state == SUSPEND_STATE_PAUSE)
		{
			pPC->SetSendDoneFlag();
			pPC->GetRunningQuestState()->args = 0;

			if (!RunState(*pPC->GetRunningQuestState()))
			{
				CloseState(*pPC->GetRunningQuestState());
				pPC->EndRunning();
			}
		}
		else
		{
			//cerr << pPC << endl;
			//cerr << pPC->IsRunning() << endl;
			//cerr << pPC->GetRunningQuestState()->suspend_state;
			//cerr << SUSPEND_STATE_WAIT << endl;
			//cerr << "wrong QUEST_WAIT request! : " << pc << endl;
			sys_err("wrong QUEST_WAIT request! : %d",pc);
		}
	}

	void CQuestManager::EnterState(uint32_t pc, uint32_t quest_index, int state)
	{
		PC* pPC;
		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
				return;

			m_mapNPC[QUEST_NO_NPC].OnEnterState(*pPC, quest_index, state);
		}
		else
			sys_err("QUEST no such pc id : %d", pc);
	}

	void CQuestManager::LeaveState(uint32_t pc, uint32_t quest_index, int state)
	{
		PC* pPC;
		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
				return;

			m_mapNPC[QUEST_NO_NPC].OnLeaveState(*pPC, quest_index, state);
		}
		else
			sys_err("QUEST no such pc id : %d", pc);
	}

	void CQuestManager::Letter(uint32_t pc, uint32_t quest_index, int state)
	{
		PC* pPC;
		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
				return;

			m_mapNPC[QUEST_NO_NPC].OnLetter(*pPC, quest_index, state);
		}
		else
			sys_err("QUEST no such pc id : %d", pc);
	}

	void CQuestManager::LogoutPC(LPCHARACTER ch)
	{
		PC * pPC = GetPC(ch->GetPlayerID());

		if (pPC && pPC->IsRunning())
		{
			CloseState(*pPC->GetRunningQuestState());
			pPC->CancelRunning();
		}

		// 지우기 전에 로그아웃 한다.
		Logout(ch->GetPlayerID());

		if (ch == m_pCurrentCharacter)
		{
			m_pCurrentCharacter = nullptr;
			m_pCurrentPC = nullptr;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	//
	// Quest Event 관련
	//
	///////////////////////////////////////////////////////////////////////////////////////////
	void CQuestManager::Login(unsigned int pc, const char * c_pszQuest)
	{
		PC * pPC;

		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
				return;

			m_mapNPC[QUEST_NO_NPC].OnLogin(*pPC, c_pszQuest);
		}
		else
		{
			sys_err("QUEST no such pc id : %d", pc);
		}
	}

	void CQuestManager::Logout(unsigned int pc)
	{
		PC * pPC;

		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
				return;

			m_mapNPC[QUEST_NO_NPC].OnLogout(*pPC);
		}
		else
			sys_err("QUEST no such pc id : %d", pc);
	}

	void CQuestManager::Kill(unsigned int pc, unsigned int npc)
	{
		//m_CurrentNPCRace = npc;
		PC * pPC;

		sys_log(0, "CQuestManager::Kill QUEST_KILL_EVENT (pc=%d, npc=%d)", pc, npc);

		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
				return;

			/* [hyo] 몹 kill시 중복 카운팅 이슈 관련한 수정사항
			   quest script에 when 171.kill begin ... 등의 코드로 인하여 스크립트가 처리되었더라도
			   바로 return하지 않고 다른 검사도 수행하도록 변경함. (2011/07/21)
			*/
			// kill call script
			if (npc >= MAIN_RACE_MAX_NUM) //@fixme109
				m_mapNPC[npc].OnKill(*pPC); //@warme004
			m_mapNPC[QUEST_NO_NPC].OnKill(*pPC);

#ifdef ENABLE_PARTYKILL
			// party_kill call script
			LPCHARACTER ch = GetCurrentCharacterPtr();
			LPPARTY pParty = ch->GetParty();
			LPCHARACTER leader = pParty ? pParty->GetLeaderCharacter() : ch;
			if (leader)
			{
				m_pCurrentPartyMember = ch;
				if (npc >= MAIN_RACE_MAX_NUM) //@fixme109
					m_mapNPC[npc].OnPartyKill(*GetPC(leader->GetPlayerID())); //@warme004
				m_mapNPC[QUEST_NO_NPC].OnPartyKill(*GetPC(leader->GetPlayerID()));

				pPC = GetPC(pc);
			}
#endif
		}
		else
			sys_err("QUEST: no such pc id : %d", pc);
	}

#ifdef ENABLE_QUEST_DIE_EVENT
	void CQuestManager::Die(unsigned int pc, unsigned int npc)
	{
		PC * pPC;

		sys_log(0, "CQuestManager::Kill QUEST_DIE_EVENT (pc=%d, npc=%d)", pc, npc);

		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
				return;

			m_mapNPC[QUEST_NO_NPC].OnDie(*pPC);

		}
		else
			sys_err("QUEST: no such pc id : %d", pc);
	}
#endif

#ifdef ENABLE_QUEST_BOOT_EVENT
	bool CQuestManager::Boot()
	{
		sys_log(0, "XXX Boot Call NPC %p", GetPCForce(0));
		m_pCurrentPC = GetPCForce(0);
		m_pCurrentCharacter = nullptr;
		m_pSelectedDungeon = nullptr;
		return m_mapNPC[QUEST_NO_NPC].OnBoot(*m_pCurrentPC);
	}
#endif

#ifdef ENABLE_QUEST_DND_EVENT
	bool CQuestManager::DND(uint32_t pc, LPITEM item_dnd, LPITEM item_victim, bool bReceiveAll)
	{
		if (test_server)
			sys_log(0, "CQuestManager::DND_EVENT Start: item_dnd(%d), item_victim(%d), PC(%d)", item_dnd->GetOriginalVnum(), item_victim->GetOriginalVnum(), pc);
		PC* pPC;
		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
			{
				LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(pc);
				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("???"));
				return false;
			}
			// call script
			SetCurrentItem(item_victim);
			SetCurrentDNDItem(item_dnd);
			return m_mapNPC[item_dnd->GetVnum()].OnDND(*pPC, bReceiveAll);
		}
		else
		{
			//cout << "no such pc id : " << pc;
			sys_err("QUEST DND_EVENT no such pc id : %d", pc);
			return false;
		}
	}
#endif

	bool CQuestManager::ServerTimer(unsigned int npc, unsigned int arg)
	{
		SetServerTimerArg(arg);
		sys_log(0, "XXX ServerTimer Call NPC %p vnum %u arg %u", GetPCForce(0), npc, arg);
		m_pCurrentPC = GetPCForce(0);
		m_pCurrentCharacter = nullptr;
		m_pSelectedDungeon = nullptr;
#ifdef ENABLE_12ZI
		m_pSelectedZodiac = nullptr;
#endif
		return m_mapNPC[npc].OnServerTimer(*m_pCurrentPC);
	}

	bool CQuestManager::Timer(unsigned int pc, unsigned int npc)
	{
		PC* pPC;

		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
			{
				return false;
			}
			// call script
			return m_mapNPC[npc].OnTimer(*pPC);
		}
		else
		{
			//cout << "no such pc id : " << pc;
			sys_err("QUEST TIMER_EVENT no such pc id : %d", pc);
			return false;
		}
		//cerr << "QUEST TIMER" << endl;
	}

	void CQuestManager::LevelUp(unsigned int pc)
	{
		PC * pPC;

		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
				return;

			m_mapNPC[QUEST_NO_NPC].OnLevelUp(*pPC);
		}
		else
		{
			sys_err("QUEST LEVELUP_EVENT no such pc id : %d", pc);
		}
	}

	void CQuestManager::AttrIn(unsigned int pc, LPCHARACTER ch, int attr)
	{
		PC* pPC;
		if ((pPC = GetPC(pc)))
		{
			m_pCurrentPartyMember = ch;
			if (!CheckQuestLoaded(pPC))
				return;

			// call script
			m_mapNPC[attr+QUEST_ATTR_NPC_START].OnAttrIn(*pPC);
		}
		else
		{
			//cout << "no such pc id : " << pc;
			sys_err("QUEST no such pc id : %d", pc);
		}
	}

	void CQuestManager::AttrOut(unsigned int pc, LPCHARACTER ch, int attr)
	{
		PC* pPC;
		if ((pPC = GetPC(pc)))
		{
			//m_pCurrentCharacter = ch;
			m_pCurrentPartyMember = ch;
			if (!CheckQuestLoaded(pPC))
				return;

			// call script
			m_mapNPC[attr+QUEST_ATTR_NPC_START].OnAttrOut(*pPC);
		}
		else
		{
			//cout << "no such pc id : " << pc;
			sys_err("QUEST no such pc id : %d", pc);
		}
	}

	bool CQuestManager::Target(unsigned int pc, uint32_t dwQuestIndex, const char * c_pszTargetName, const char * c_pszVerb)
	{
		PC * pPC;

		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
				return false;

			bool bRet;
			return m_mapNPC[QUEST_NO_NPC].OnTarget(*pPC, dwQuestIndex, c_pszTargetName, c_pszVerb, bRet);
		}

		return false;
	}

	void CQuestManager::QuestInfo(unsigned int pc, unsigned int quest_index)
	{
		PC* pPC;

		if ((pPC = GetPC(pc)))
		{
			// call script
			if (!CheckQuestLoaded(pPC))
			{
				LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(pc);

				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;772]");

				return;
			}

			//퀘스트 창에서 퀘스트 클릭과 NPC 클릭시의 구분을 위한 플래그
			m_bQuestInfoFlag = 1;
			m_mapNPC[QUEST_NO_NPC].OnInfo(*pPC, quest_index);
		}
		else
		{
			//cout << "no such pc id : " << pc;
			sys_err("QUEST INFO_EVENT no such pc id : %d", pc);
		}
	}

	void CQuestManager::QuestButton(unsigned int pc, unsigned int quest_index)
	{
		PC* pPC;
		if ((pPC = GetPC(pc)))
		{
			// call script
			if (!CheckQuestLoaded(pPC))
			{
				LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(pc);
				if (ch)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;772]");
				}
				return;
			}
			m_mapNPC[QUEST_NO_NPC].OnButton(*pPC, quest_index);
		}
		else
		{
			//cout << "no such pc id : " << pc;
			sys_err("QUEST CLICK_EVENT no such pc id : %d", pc);
		}
	}

	bool CQuestManager::TakeItem(unsigned int pc, unsigned int npc, LPITEM item)
	{
		//m_CurrentNPCRace = npc;
		PC* pPC;

		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
			{
				LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(pc);
				if (ch)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;772]");
				}
				return false;
			}
			// call script
			SetCurrentItem(item);
			return m_mapNPC[npc].OnTakeItem(*pPC);
		}
		else
		{
			//cout << "no such pc id : " << pc;
			sys_err("QUEST USE_ITEM_EVENT no such pc id : %d", pc);
			return false;
		}
	}

	bool CQuestManager::UseItem(unsigned int pc, LPITEM item, bool bReceiveAll)
	{
		if (test_server)
			sys_log( 0, "questmanager::UseItem Start : itemVnum : %d PC : %d", item->GetOriginalVnum(), pc);
		PC* pPC;
		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
			{
				LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(pc);
				if (ch)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;772]");
				}
				return false;
			}
			// call script
			SetCurrentItem(item);
			/*
			if (test_server)
			{
				sys_log( 0, "Quest UseItem Start : itemVnum : %d PC : %d", item->GetOriginalVnum(), pc);
				itertype(m_mapNPC) it = m_mapNPC.begin();
				itertype(m_mapNPC) end = m_mapNPC.end();
				for( ; it != end ; ++it)
				{
					sys_log( 0, "Quest UseItem : vnum : %d item Vnum : %d", it->first, item->GetOriginalVnum());
				}
			}
			if(test_server)
			sys_log( 0, "questmanager:useItem: mapNPCVnum : %d\n", m_mapNPC[item->GetVnum()].GetVnum());
			*/

			return m_mapNPC[item->GetVnum()].OnUseItem(*pPC, bReceiveAll);
		}
		else
		{
			//cout << "no such pc id : " << pc;
			sys_err("QUEST USE_ITEM_EVENT no such pc id : %d", pc);
			return false;
		}
	}

	// Speical Item Group에 정의된 Group Use
	bool CQuestManager::SIGUse(unsigned int pc, uint32_t sig_vnum, LPITEM item, bool bReceiveAll)
	{
		if (test_server)
			sys_log( 0, "questmanager::SIGUse Start : itemVnum : %d PC : %d", item->GetOriginalVnum(), pc);
		PC* pPC;
		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
			{
				LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(pc);
				if (ch)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;772]");
				}
				return false;
			}
			// call script
			SetCurrentItem(item);

			return m_mapNPC[sig_vnum].OnSIGUse(*pPC, bReceiveAll);
		}
		else
		{
			//cout << "no such pc id : " << pc;
			sys_err("QUEST USE_ITEM_EVENT no such pc id : %d", pc);
			return false;
		}
	}

	bool CQuestManager::GiveItemToPC(unsigned int pc, LPCHARACTER pkChr)
	{
		if (!pkChr->IsPC())
			return false;

		PC * pPC = GetPC(pc);

		if (pPC)
		{
			if (!CheckQuestLoaded(pPC))
				return false;

			TargetInfo * pInfo = CTargetManager::Instance().GetTargetInfo(pc, TARGET_TYPE_VID, pkChr->GetVID());

			if (pInfo)
			{
				bool bRet;

				if (m_mapNPC[QUEST_NO_NPC].OnTarget(*pPC, pInfo->dwQuestIndex, pInfo->szTargetName, "click", bRet))
					return true;
			}
		}

		return false;
	}

	bool CQuestManager::Click(unsigned int pc, LPCHARACTER pkChrTarget)
	{
		PC * pPC = GetPC(pc);

		if (pPC)
		{
			if (!CheckQuestLoaded(pPC))
			{
				LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(pc);

				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;772]");

				return false;
			}

			TargetInfo * pInfo = CTargetManager::Instance().GetTargetInfo(pc, TARGET_TYPE_VID, pkChrTarget->GetVID());
			if (test_server)
			{
				sys_log(0, "CQuestManager::Click(pid=%d, npc_name=%s) - target_info(%x)", pc, pkChrTarget->GetName(), pInfo);
			}

			if (pInfo)
			{
				bool bRet;
				if (m_mapNPC[QUEST_NO_NPC].OnTarget(*pPC, pInfo->dwQuestIndex, pInfo->szTargetName, "click", bRet))
					return bRet;
			}

			uint32_t dwCurrentNPCRace = pkChrTarget->GetRaceNum();

			if (pkChrTarget->IsNPC())
			{
				map<uint32_t, NPC>::iterator it = m_mapNPC.find(dwCurrentNPCRace);

				if (it == m_mapNPC.end())
				{
					sys_log(0, "CQuestManager::Click(pid=%d, target_npc_name=%s) - NOT EXIST NPC RACE VNUM[%d]",
						pc,
						pkChrTarget->GetName(),
						dwCurrentNPCRace); // @warme012
					return false;
				}

				// call script
				if (it->second.HasChat())
				{
					// if have chat, give chat
					if (test_server)
						sys_log(0, "CQuestManager::Click->OnChat");

					if (!it->second.OnChat(*pPC))
					{
						if (test_server)
							sys_log(0, "CQuestManager::Click->OnChat Failed");

						return it->second.OnClick(*pPC);
					}

					return true;
				}
				else
				{
					// else click
					return it->second.OnClick(*pPC);
				}
			}
			return false;
		}
		else
		{
			//cout << "no such pc id : " << pc;
			sys_err("QUEST CLICK_EVENT no such pc id : %d", pc);
			return false;
		}
		//cerr << "QUEST CLICk" << endl;
	}

	void CQuestManager::Unmount(unsigned int pc)
	{
		PC * pPC;

		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
				return;

			m_mapNPC[QUEST_NO_NPC].OnUnmount(*pPC);
		}
		else
			sys_err("QUEST no such pc id : %d", pc);
	}
	//독일 선물 기능 테스트
	void CQuestManager::ItemInformer(unsigned int pc,unsigned int vnum)
	{

		PC* pPC;
		pPC = GetPC(pc);

		m_mapNPC[QUEST_NO_NPC].OnItemInformer(*pPC,vnum);
	}

#ifdef ENABLE_EVENT_MANAGER
	void CQuestManager::EventBegin(uint32_t pc)
	{
		PC* pPC;

		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
				return;

			m_mapNPC[QUEST_NO_NPC].OnEventBegin(*pPC);
		}
		else
			sys_err("QUEST no such pc id : %d", pc);
	}

	void CQuestManager::EventEnd(uint32_t pc)
	{
		PC* pPC;

		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
				return;

			m_mapNPC[QUEST_NO_NPC].OnEventEnd(*pPC);
		}
		else
			sys_err("QUEST no such pc id : %d", pc);
	}

	void CQuestManager::RequestEventQuest(const std::string& quest_name, uint32_t pc)
	{
		if (GetPCForce(pc)->IsRunning())
			return;

		const int questIdx = GetQuestIndexByName(quest_name);
		if (questIdx)
			QuestButton(pc, questIdx);
	}
#endif
	///////////////////////////////////////////////////////////////////////////////////////////
	// END OF 퀘스트 이벤트 처리
	///////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////
	void CQuestManager::LoadStartQuest(const string& quest_name, unsigned int idx)
	{
		for (itertype(g_setQuestObjectDir) it = g_setQuestObjectDir.begin(); it != g_setQuestObjectDir.end(); ++it)
		{
			const string& stQuestObjectDir = *it;
			string full_name = stQuestObjectDir + "/begin_condition/" + quest_name;
			ifstream inf(full_name.c_str());

			if (inf.is_open())
			{
				sys_log(0, "QUEST loading begin condition for %s", quest_name.c_str());

				istreambuf_iterator<char> ib(inf), ie;
				copy(ib, ie, back_inserter(m_hmQuestStartScript[idx]));
			}
		}
	}

	bool CQuestManager::CanStartQuest(unsigned int quest_index, const PC& pc)
	{
		return CanStartQuest(quest_index);
	}

	bool CQuestManager::CanStartQuest(unsigned int quest_index)
	{
		THashMapQuestStartScript::iterator it;

		if ((it = m_hmQuestStartScript.find(quest_index)) == m_hmQuestStartScript.end())
			return true;
		else
		{
			int x = lua_gettop(L);
			lua_dobuffer(L, &(it->second[0]), it->second.size(), "StartScript");
			int bStart = lua_toboolean(L, -1);
			lua_settop(L, x);
			return bStart != 0;
		}
	}

	bool CQuestManager::CanEndQuestAtState(const string& quest_name, const string& state_name)
	{
		return false;
	}

	void CQuestManager::DisconnectPC(LPCHARACTER ch)
	{
		m_mapPC.erase(ch->GetPlayerID());
	}

	void CQuestManager::StopAllRunningQuests()	//@fixme509
	{
		for (PCMap::iterator it = m_mapPC.begin(); it != m_mapPC.end(); it++)
		{
			it->second.CancelRunning();
			LPCHARACTER pkChr = CHARACTER_MANAGER::Instance().FindByPID(it->first);
			if (!pkChr || !(pkChr->GetDesc()))
				continue;
			TPacketGCScript packet_script;

			packet_script.header = HEADER_GC_SCRIPT;
			packet_script.skin = QUEST_SKIN_NOWINDOW;
			string data = "[DESTROY_ALL]";
			packet_script.src_size = data.size();
			packet_script.size = packet_script.src_size + sizeof(TPacketGCScript);

			TEMP_BUFFER buf;
			buf.write(&packet_script, sizeof(TPacketGCScript));
			buf.write(&data[0], data.size());

			pkChr->GetDesc()->Packet(buf.read_peek(), buf.size());
		}
	}


	PC * CQuestManager::GetPCForce(unsigned int pc)
	{
		PCMap::iterator it;

		if ((it = m_mapPC.find(pc)) == m_mapPC.end())
		{
			PC * pPC = &m_mapPC[pc];
			pPC->SetID(pc);
			return pPC;
		}

		return &it->second;
	}

	PC * CQuestManager::GetPC(unsigned int pc)
	{
		PCMap::iterator it;

		LPCHARACTER pkChr = CHARACTER_MANAGER::Instance().FindByPID(pc);

		if (!pkChr)
			return nullptr;

		m_pCurrentPC = GetPCForce(pc);
		m_pCurrentCharacter = pkChr;
		m_pSelectedDungeon = nullptr;
#ifdef ENABLE_12ZI
		m_pSelectedZodiac = nullptr;
#endif
		return (m_pCurrentPC);
	}

	void CQuestManager::ClearScript()
	{
		m_strScript.clear();
		m_iCurrentSkin = QUEST_SKIN_NORMAL;
	}

	void CQuestManager::AddScript(const string& str)
	{
		m_strScript+=str;
	}

	void CQuestManager::SendScript()
	{
		if (m_bNoSend || !GetCurrentCharacterPtr() || !GetCurrentCharacterPtr()->GetDesc()) //@fixme174
		{
			m_bNoSend = false;
			ClearScript();
			return;
		}

		if (m_strScript=="[DONE]" || m_strScript == "[NEXT]")
		{
			if (m_pCurrentPC && !m_pCurrentPC->GetAndResetDoneFlag() && m_strScript=="[DONE]" && m_iCurrentSkin == QUEST_SKIN_NORMAL && !IsError())
			{
				ClearScript();
				return;
			}
			m_iCurrentSkin = QUEST_SKIN_NOWINDOW;
		}

		//sys_log(0, "Send Quest Script to %s", GetCurrentCharacterPtr()->GetName());
		//send -_-!
		TPacketGCScript packet_script;

		packet_script.header = HEADER_GC_SCRIPT;
		packet_script.skin = m_iCurrentSkin;
		packet_script.src_size = m_strScript.size();
		packet_script.size = packet_script.src_size + sizeof(TPacketGCScript);

		TEMP_BUFFER buf;
		buf.write(&packet_script, sizeof(TPacketGCScript));
		buf.write(&m_strScript[0], m_strScript.size());

		GetCurrentCharacterPtr()->GetDesc()->Packet(buf.read_peek(), buf.size());

		if (test_server)
			sys_log(0, "m_strScript %s size %d", m_strScript.c_str(), buf.size());

		ClearScript();
		m_bQuestInfoFlag = 0;
	}

	const char* CQuestManager::GetQuestStateName(const string& quest_name, const int state_index)
	{
		int x = lua_gettop(L);
		lua_getglobal(L, quest_name.c_str());
		if (lua_isnil(L,-1))
		{
			sys_err("QUEST wrong quest state file %s.%d", quest_name.c_str(), state_index);
			lua_settop(L,x);
			return "";
		}
		lua_pushnumber(L, state_index);
		lua_gettable(L, -2);

		const char* str = lua_tostring(L, -1);
		lua_settop(L, x);
		return str;
	}

	int CQuestManager::GetQuestStateIndex(const string& quest_name, const string& state_name)
	{
		int x = lua_gettop(L);
		lua_getglobal(L, quest_name.c_str());
		if (lua_isnil(L,-1))
		{
			sys_err("QUEST wrong quest state file %s.%s",quest_name.c_str(),state_name.c_str()  );
			lua_settop(L,x);
			return 0;
		}
		lua_pushstring(L, state_name.c_str());
		lua_gettable(L, -2);

		int v = (int)rint(lua_tonumber(L,-1));
		lua_settop(L, x);
		if (test_server)
			sys_log( 0,"[QUESTMANAGER] GetQuestStateIndex x(%d) v(%d) %s %s", v,x, quest_name.c_str(), state_name.c_str() );
		return v;
	}

	void CQuestManager::SetSkinStyle(int iStyle)
	{
		if (iStyle<0 || iStyle >= QUEST_SKIN_COUNT)
		{
			m_iCurrentSkin = QUEST_SKIN_NORMAL;
		}
		else
			m_iCurrentSkin = iStyle;
	}

	unsigned int CQuestManager::LoadTimerScript(const string& name)
	{
		map<string, unsigned int>::iterator it;
		if ((it = m_mapTimerID.find(name)) != m_mapTimerID.end())
		{
			return it->second;
		}
		else
		{
			unsigned int new_id = UINT_MAX - m_mapTimerID.size();

			m_mapNPC[new_id].Set(new_id, name);
			m_mapTimerID.insert(make_pair(name, new_id));

			return new_id;
		}
	}

	unsigned int CQuestManager::GetCurrentNPCRace()
	{
		return GetCurrentNPCCharacterPtr() ? GetCurrentNPCCharacterPtr()->GetRaceNum() : 0;
	}

	LPITEM CQuestManager::GetCurrentItem()
	{
		return GetCurrentCharacterPtr() ? GetCurrentCharacterPtr()->GetQuestItemPtr() : nullptr;
	}

	void CQuestManager::ClearCurrentItem()
	{
		if (GetCurrentCharacterPtr())
			GetCurrentCharacterPtr()->ClearQuestItemPtr();
	}

	void CQuestManager::SetCurrentItem(LPITEM item)
	{
		if (GetCurrentCharacterPtr())
			GetCurrentCharacterPtr()->SetQuestItemPtr(item);
	}

#ifdef ENABLE_QUEST_DND_EVENT
	LPITEM CQuestManager::GetCurrentDNDItem()
	{
		return GetCurrentCharacterPtr() ? GetCurrentCharacterPtr()->GetQuestDNDItemPtr() : nullptr;
	}

	void CQuestManager::ClearCurrentDNDItem()
	{
		if (GetCurrentCharacterPtr())
			GetCurrentCharacterPtr()->ClearQuestDNDItemPtr();
	}

	void CQuestManager::SetCurrentDNDItem(LPITEM item)
	{
		if (GetCurrentCharacterPtr())
			GetCurrentCharacterPtr()->SetQuestDNDItemPtr(item);
	}
#endif

	LPCHARACTER CQuestManager::GetCurrentNPCCharacterPtr()
	{
		return GetCurrentCharacterPtr() ? GetCurrentCharacterPtr()->GetQuestNPC() : nullptr;
	}

	const string & CQuestManager::GetCurrentQuestName()
	{
		return GetCurrentPC()->GetCurrentQuestName();
	}

	LPDUNGEON CQuestManager::GetCurrentDungeon()
	{
		LPCHARACTER ch = GetCurrentCharacterPtr();

		if (!ch)
		{
			if (m_pSelectedDungeon)
				return m_pSelectedDungeon;
			return nullptr;
		}

		return ch->GetDungeonForce();
	}

#ifdef ENABLE_12ZI
	LPZODIAC CQuestManager::GetCurrentZodiac()
	{
		LPCHARACTER ch = GetCurrentCharacterPtr();

		if (!ch)
		{
			if (m_pSelectedZodiac)
				return m_pSelectedZodiac;

			return nullptr;
		}

		return ch->GetZodiacForce();
	}
#endif

	void CQuestManager::RegisterQuest(const string & stQuestName, unsigned int idx)
	{
		assert(idx > 0);

		itertype(m_hmQuestName) it;

		if ((it = m_hmQuestName.find(stQuestName)) != m_hmQuestName.end())
			return;

		m_hmQuestName.insert(make_pair(stQuestName, idx));
		LoadStartQuest(stQuestName, idx);
		m_mapQuestNameByIndex.insert(make_pair(idx, stQuestName));

		sys_log(0, "QUEST: Register %4u %s", idx, stQuestName.c_str());
	}

	unsigned int CQuestManager::GetQuestIndexByName(const string& name)
	{
		THashMapQuestName::iterator it = m_hmQuestName.find(name);

		if (it == m_hmQuestName.end())
			return 0; // RESERVED

		return it->second;
	}

	const string & CQuestManager::GetQuestNameByIndex(unsigned int idx)
	{
		itertype(m_mapQuestNameByIndex) it;

		if ((it = m_mapQuestNameByIndex.find(idx)) == m_mapQuestNameByIndex.end())
		{
			sys_err("cannot find quest name by index %u", idx);
			assert(!"cannot find quest name by index");

			static std::string st = "";
			return st;
		}

		return it->second;
	}

	void CQuestManager::SendEventFlagList(LPCHARACTER ch)
	{
		itertype(m_mapEventFlag) it;
		for (it = m_mapEventFlag.begin(); it != m_mapEventFlag.end(); ++it)
		{
			const string& flagname = it->first;
			int value = it->second;

			if (!test_server && value == 1 && flagname == "valentine_drop")
				ch->ChatPacket(CHAT_TYPE_INFO, "%s %d prob 800", flagname.c_str(), value);
			else if (!test_server && value == 1 && flagname == "newyear_wonso")
				ch->ChatPacket(CHAT_TYPE_INFO, "%s %d prob 500", flagname.c_str(), value);
			else if (!test_server && value == 1 && flagname == "newyear_fire")
				ch->ChatPacket(CHAT_TYPE_INFO, "%s %d prob 1000", flagname.c_str(), value);
			else
				ch->ChatPacket(CHAT_TYPE_INFO, "%s %d", flagname.c_str(), value);
		}
	}

	void CQuestManager::RequestSetEventFlag(const string& name, int value)
	{
		TPacketSetEventFlag p;
		strlcpy(p.szFlagName, name.c_str(), sizeof(p.szFlagName));
		p.lValue = value;
		db_clientdesc->DBPacket(HEADER_GD_SET_EVENT_FLAG, 0, &p, sizeof(TPacketSetEventFlag));
	}

	void CQuestManager::SetEventFlag(const string& name, int value)
	{
		static const char*	DROPEVENT_CHARTONE_NAME		= "drop_char_stone";
		static const int	DROPEVENT_CHARTONE_NAME_LEN = strlen(DROPEVENT_CHARTONE_NAME);

		int prev_value = m_mapEventFlag[name];

		sys_log(0, "QUEST eventflag %s %d prev_value %d", name.c_str(), value, m_mapEventFlag[name]);
		m_mapEventFlag[name] = value;

		if (name == "mob_item")
		{
			CHARACTER_MANAGER::Instance().SetMobItemRate(value);
		}
		else if (name == "mob_dam")
		{
			CHARACTER_MANAGER::Instance().SetMobDamageRate(value);
		}
		else if (name == "mob_gold")
		{
			CHARACTER_MANAGER::Instance().SetMobGoldAmountRate(value);
		}
		else if (name == "mob_gold_pct")
		{
			CHARACTER_MANAGER::Instance().SetMobGoldDropRate(value);
		}
		else if (name == "user_dam")
		{
			CHARACTER_MANAGER::Instance().SetUserDamageRate(value);
		}
		else if (name == "user_dam_buyer")
		{
			CHARACTER_MANAGER::Instance().SetUserDamageRatePremium(value);
		}
		else if (name == "mob_exp")
		{
			CHARACTER_MANAGER::Instance().SetMobExpRate(value);
		}
		else if (name == "mob_item_buyer")
		{
			CHARACTER_MANAGER::Instance().SetMobItemRatePremium(value);
		}
		else if (name == "mob_exp_buyer")
		{
			CHARACTER_MANAGER::Instance().SetMobExpRatePremium(value);
		}
		else if (name == "mob_gold_buyer")
		{
			CHARACTER_MANAGER::Instance().SetMobGoldAmountRatePremium(value);
		}
		else if (name == "mob_gold_pct_buyer")
		{
			CHARACTER_MANAGER::Instance().SetMobGoldDropRatePremium(value);
		}
		else if (name == "crcdisconnect")
		{
			DESC_MANAGER::Instance().SetDisconnectInvalidCRCMode(value != 0);
		}
		else if (!name.compare(0,5,"xmas_"))
		{
			xmas::ProcessEventFlag(name, prev_value, value);
		}
		else if (name == "newyear_boom")
		{
			const DESC_MANAGER::DESC_SET & c_ref_set = DESC_MANAGER::Instance().GetClientSet();

			for (itertype(c_ref_set) it = c_ref_set.begin(); it != c_ref_set.end(); ++it)
			{
				LPCHARACTER ch = (*it)->GetCharacter();

				if (!ch)
					continue;

				ch->ChatPacket(CHAT_TYPE_COMMAND, "newyear_boom %d", value);
			}
		}
		else if ( name == "eclipse" )
		{
			std::string mode("");

			if (value == DayMode::DAY)
				mode = "light";
			else if (value == DayMode::NIGHT)
				mode = "dark";
#ifdef ENABLE_WEATHER_INFO
			else if (value == DayMode::MORNING)
				mode = "morning";
			else if (value == DayMode::EVENING)
				mode = "evening";
			else if (value == DayMode::RAIN)
				mode = "rain";
#endif
			else
				mode = "light";

			const DESC_MANAGER::DESC_SET & c_ref_set = DESC_MANAGER::Instance().GetClientSet();

			for (itertype(c_ref_set) it = c_ref_set.begin(); it != c_ref_set.end(); ++it)
			{
				LPCHARACTER ch = (*it)->GetCharacter();
				if (!ch)
					continue;

				ch->ChatPacket(CHAT_TYPE_COMMAND, "DayMode %s", mode.c_str());
			}
		}
		else if (name == "day")
		{
			const DESC_MANAGER::DESC_SET & c_ref_set = DESC_MANAGER::Instance().GetClientSet();

			for (itertype(c_ref_set) it = c_ref_set.begin(); it != c_ref_set.end(); ++it)
			{
				LPCHARACTER ch = (*it)->GetCharacter();
				if (!ch)
					continue;
				if (value)
				{
					// 밤
					ch->ChatPacket(CHAT_TYPE_COMMAND, "DayMode dark");
				}
				else
				{
					// 낮
					ch->ChatPacket(CHAT_TYPE_COMMAND, "DayMode light");
				}
			}

			if (value && !prev_value)
			{
				// 없으면 만들어준다
				struct SNPCSellFireworkPosition
				{
					long lMapIndex;
					int x;
					int y;
				} positions[] = {
					{	1,	615,	618 },
					{	3,	500,	625 },
					{	21,	598,	665 },
					{	23,	476,	360 },
					{	41,	318,	629 },
					{	43,	478,	375 },
					{	0,	0,	0   },
				};

				SNPCSellFireworkPosition* p = positions;
				while (p->lMapIndex)
				{
					if (map_allow_find(p->lMapIndex))
					{
						PIXEL_POSITION posBase;
						if (!SECTREE_MANAGER::Instance().GetMapBasePositionByMapIndex(p->lMapIndex, posBase))
						{
							sys_err("cannot get map base position %d", p->lMapIndex);
							++p;
							continue;
						}

						CHARACTER_MANAGER::Instance().SpawnMob(xmas::MOB_XMAS_FIRWORK_SELLER_VNUM, p->lMapIndex, posBase.x + p->x * 100, posBase.y + p->y * 100, 0, false, -1);
					}
					p++;
				}
			}
			else if (!value && prev_value)
			{
				// 있으면 지워준다
				CharacterVectorInteractor i;

				auto snapshot = CHARACTER_MANAGER::Instance().GetCharactersByRaceNum(xmas::MOB_XMAS_FIRWORK_SELLER_VNUM);
				if (!snapshot.empty())
				{
					auto it = snapshot.begin();
					while (it != snapshot.end())
					{
						M2_DESTROY_CHARACTER(*it++);
					}
				}
			}
		}
		else if (name == "pre_event_hc")
		{
			const uint32_t EventNPC = 20090;

			struct SEventNPCPosition
			{
				long lMapIndex;
				int x;
				int y;
			} positions[] = {
				{ 3, 588, 617 },
				{ 23, 397, 250 },
				{ 43, 567, 426 },
				{ 0, 0, 0 },
			};

			if (value && !prev_value)
			{
				SEventNPCPosition* pPosition = positions;

				while (pPosition->lMapIndex)
				{
					if (map_allow_find(pPosition->lMapIndex))
					{
						PIXEL_POSITION pos;

						if (!SECTREE_MANAGER::Instance().GetMapBasePositionByMapIndex(pPosition->lMapIndex, pos))
						{
							sys_err("cannot get map base position %d", pPosition->lMapIndex);
							++pPosition;
							continue;
						}

						CHARACTER_MANAGER::Instance().SpawnMob(EventNPC, pPosition->lMapIndex, pos.x+pPosition->x*100, pos.y+pPosition->y*100, 0, false, -1);
					}
					pPosition++;
				}
			}
			else if (!value && prev_value)
			{
				auto snapshot = CHARACTER_MANAGER::Instance().GetCharactersByRaceNum(EventNPC);
				if (!snapshot.empty())
				{
					auto it = snapshot.begin();
					while (it != snapshot.end())
					{
						LPCHARACTER ch = *it++;

						switch (ch->GetMapIndex())
						{
							case MapTypes::MAP_A3:
							case MapTypes::MAP_B3:
							case MapTypes::MAP_C3:
								M2_DESTROY_CHARACTER(ch);
								break;
						}
					}
				}
			}
		}
		else if (name.compare(0, DROPEVENT_CHARTONE_NAME_LEN, DROPEVENT_CHARTONE_NAME)== 0)
		{
			DropEvent_CharStone_SetValue(name, value);
		}
		else if (name.compare(0, strlen("refine_box"), "refine_box")== 0)
		{
			DropEvent_RefineBox_SetValue(name, value);
		}
		else if (name == "gold_drop_limit_time")
		{
			g_GoldDropTimeLimitValue = value * 1000;
		}
#ifdef ENABLE_NEWSTUFF
		else if (name == "item_drop_limit_time")
		{
			g_ItemDropTimeLimitValue = value * 1000;
		}
		else if (name == "box_use_limit_time")
		{
			g_BoxUseTimeLimitValue = value * 1000;
		}
		else if (name == "buysell_limit_time")
		{
			g_BuySellTimeLimitValue = value * 1000;
		}
		else if (name == "no_drop_metin_stone")
		{
			g_NoDropMetinStone = !!value;
		}
		else if (name == "no_mount_at_guild_war")
		{
			g_NoMountAtGuildWar = !!value;
		}
		else if (name == "no_potions_on_pvp")
		{
			g_NoPotionsOnPVP = !!value;
		}
#endif
#ifdef ENABLE_EVENT_BANNER_FLAG
		else if (name == "banner")
		{
			CMiniGameManager::Instance().InitializeBanners();
		}
#endif
#ifdef ENABLE_MONSTER_BACK
#	ifdef ENABLE_10TH_EVENT
        else if (name == "e_monsterback")
#	else
        else if (name == "e_easter_monsterback")
#	endif
        {
            CMiniGameManager::Instance().InitializeAttendance(value);
        }
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
		else if (name == "enable_catch_king_event")
		{
			CMiniGameManager::Instance().InitializeMiniGameCatchKing(value);
		}
		else if (name == "catch_king_event_end_day")
		{
			CMiniGameManager::Instance().InitializeMiniGameCatchKingEndTime(value);
		}
#endif
#ifdef ENABLE_MINI_GAME_FINDM
		else if (name == "mini_game_findm_event")
		{
			CMiniGameManager::instance().InitializeEvent(FIND_M_EVENT, value);
			CMiniGameManager::instance().InitializeMiniGameFindM(value);
		}
		else if (name == "findm_event_end_day")
		{
			CMiniGameManager::instance().InitializeMiniGameFindMEndTime(value);
		}
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
		else if (name == "mini_game_yutnori_event" || name == "enable_yut_nori_event_get_reward")
		{
			if (name == "mini_game_yutnori_event")
			{
				CMiniGameManager::instance().InitializeEvent(YUT_NORI_EVENT, value);
			}

			CMiniGameManager::instance().InitializeMiniGameYutNori(value);
		}
		else if (name == "yut_nori_event_end_day" || name == "yut_nori_event_get_reward_end_day")
		{
			CMiniGameManager::instance().InitializeMiniGameYutNoriEndTime(value);
		}
#endif
#ifdef ENABLE_EASTER_EVENT
		else if (name == "enable_easter_event")
		{
			CMiniGameManager::Instance().InitializeEasterEvent(value);
		}
#endif
		else if (name == "new_xmas_event")
		{
			// 20126 new산타.
			static uint32_t new_santa = 20126;
			if (value != 0)
			{
				CharacterVectorInteractor i;
				bool map1_santa_exist = false;
				bool map21_santa_exist = false;
				bool map41_santa_exist = false;

				auto snapshot = CHARACTER_MANAGER::Instance().GetCharactersByRaceNum(new_santa);
				if (!snapshot.empty())
				{
					auto it = snapshot.begin();
					while (it != snapshot.end())
					{
						LPCHARACTER tch = *(it++);

						if (tch->GetMapIndex() == 1)
						{
							map1_santa_exist = true;
						}
						else if (tch->GetMapIndex() == 21)
						{
							map21_santa_exist = true;
						}
						else if (tch->GetMapIndex() == 41)
						{
							map41_santa_exist = true;
						}
					}
				}

				if (map_allow_find(1) && !map1_santa_exist)
				{
					LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::Instance().GetMap(1);
					CHARACTER_MANAGER::Instance().SpawnMob(new_santa, 1, pkSectreeMap->m_setting.iBaseX + 60800, pkSectreeMap->m_setting.iBaseY + 61700, 0, false, 90, true);
				}
				if (map_allow_find(21) && !map21_santa_exist)
				{
					LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::Instance().GetMap(21);
					CHARACTER_MANAGER::Instance().SpawnMob(new_santa, 21, pkSectreeMap->m_setting.iBaseX + 59600, pkSectreeMap->m_setting.iBaseY + 61000, 0, false, 110, true);
				}
				if (map_allow_find(41) && !map41_santa_exist)
				{
					LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::Instance().GetMap(41);
					CHARACTER_MANAGER::Instance().SpawnMob(new_santa, 41, pkSectreeMap->m_setting.iBaseX + 35700, pkSectreeMap->m_setting.iBaseY + 74300, 0, false, 140, true);
				}
			}
			else
			{
				CharacterVectorInteractor i;
				CHARACTER_MANAGER::Instance().GetCharactersByRaceNum(new_santa);

				for (CharacterVectorInteractor::iterator it = i.begin(); it != i.end(); it++)
				{
					M2_DESTROY_CHARACTER(*it);
				}
			}
		}
	}

	int	CQuestManager::GetEventFlag(const string& name)
	{
		map<string,int>::iterator it = m_mapEventFlag.find(name);

		if (it == m_mapEventFlag.end())
			return 0;

		return it->second;
	}

	void CQuestManager::BroadcastEventFlagOnLogin(LPCHARACTER ch)
	{
		int iEventFlagValue;

		if ((iEventFlagValue = quest::CQuestManager::Instance().GetEventFlag("xmas_snow")))
			ch->ChatPacket(CHAT_TYPE_COMMAND, "xmas_snow %d", iEventFlagValue);
		if ((iEventFlagValue = quest::CQuestManager::Instance().GetEventFlag("xmas_boom")))
			ch->ChatPacket(CHAT_TYPE_COMMAND, "xmas_boom %d", iEventFlagValue);
		if ((iEventFlagValue = quest::CQuestManager::Instance().GetEventFlag("xmas_tree")))
			ch->ChatPacket(CHAT_TYPE_COMMAND, "xmas_tree %d", iEventFlagValue);
		if ((iEventFlagValue = quest::CQuestManager::Instance().GetEventFlag("day")))
			ch->ChatPacket(CHAT_TYPE_COMMAND, "DayMode dark");
		if ((iEventFlagValue = quest::CQuestManager::Instance().GetEventFlag("newyear_boom")))
			ch->ChatPacket(CHAT_TYPE_COMMAND, "newyear_boom %d", iEventFlagValue);
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
		if ((iEventFlagValue = quest::CQuestManager::Instance().GetEventFlag("xmas_soul")))
			ch->ChatPacket(CHAT_TYPE_COMMAND, "xmas_soul %d", iEventFlagValue);
#endif
#ifndef ENABLE_WEATHER_INFO
		if ((iEventFlagValue = quest::CQuestManager::Instance().GetEventFlag("eclipse")))
		{
			std::string mode;

			if (iEventFlagValue == DayMode::DAY)
				mode = "light";
			else if (iEventFlagValue == DayMode::NIGHT)
				mode = "dark";
			else
				mode = "light";

			ch->ChatPacket(CHAT_TYPE_COMMAND, "DayMode %s", mode.c_str());
		}
#endif
	}

	void CQuestManager::Reload()
	{
		StopAllRunningQuests();	//@fixme509
		lua_close(L);
		m_mapNPC.clear();
		m_mapNPCNameID.clear();
		m_hmQuestName.clear();
		m_mapTimerID.clear();

#ifdef __UNIMPLEMENTED__
		//Clear all server timers
		for (auto it = m_mapServerTimer.begin(); it != m_mapServerTimer.end();) {
			LPEVENT event = it->second;
			event_cancel(&event);
			it = m_mapServerTimer.erase(it);
		}
#endif

		m_hmQuestStartScript.clear();
		m_mapEventName.clear();
		L = nullptr;
		Initialize();

		for (itertype(m_registeredNPCVnum) it = m_registeredNPCVnum.begin(); it != m_registeredNPCVnum.end(); ++it)
		{
			char buf[256];
			uint32_t dwVnum = *it;
			snprintf(buf, sizeof(buf), "%u", dwVnum);
			m_mapNPC[dwVnum].Set(dwVnum, buf);
		}
	}

	bool CQuestManager::ExecuteQuestScript(PC& pc, uint32_t quest_index, const int state, const char* code, const int code_size, vector<AArgScript*>* pChatScripts, bool bUseCache)
	{
		return ExecuteQuestScript(pc, CQuestManager::Instance().GetQuestNameByIndex(quest_index), state, code, code_size, pChatScripts, bUseCache);
	}

	bool CQuestManager::ExecuteQuestScript(PC& pc, const string& quest_name, const int state, const char* code, const int code_size, vector<AArgScript*>* pChatScripts, bool bUseCache)
	{
		// 실행공간을 생성
		QuestState qs = CQuestManager::Instance().OpenState(quest_name, state);
		if (pChatScripts)
			qs.chat_scripts.swap(*pChatScripts);

		// 코드를 읽어들임
		if (bUseCache)
		{
			lua_getglobal(qs.co, "__codecache");
			// stack : __codecache
			lua_pushnumber(qs.co, (long)code);
			// stack : __codecache (codeptr)
			lua_rawget(qs.co, -2);
			// stack : __codecache (compiled-code)
			if (lua_isnil(qs.co, -1))
			{
				// cache miss

				// load code to lua,
				// save it to cache
				// and only function remain in stack
				lua_pop(qs.co, 1);
				// stack : __codecache
				luaL_loadbuffer(qs.co, code, code_size, quest_name.c_str());
				// stack : __codecache (compiled-code)
				lua_pushnumber(qs.co, (long)code);
				// stack : __codecache (compiled-code) (codeptr)
				lua_pushvalue(qs.co, -2);
				// stack : __codecache (compiled-code) (codeptr) (compiled_code)
				lua_rawset(qs.co, -4);
				// stack : __codecache (compiled-code)
				lua_remove(qs.co, -2);
				// stack : (compiled-code)
			}
			else
			{
				// cache hit
				lua_remove(qs.co, -2);
				// stack : (compiled-code)
			}
		}
		else
			luaL_loadbuffer(qs.co, code, code_size, quest_name.c_str());

		// 플레이어와 연결
		pc.SetQuest(quest_name, qs);

		// 실행
		QuestState& rqs = *pc.GetRunningQuestState();
		if (!CQuestManager::Instance().RunState(rqs))
		{
			CQuestManager::Instance().CloseState(rqs);
			pc.EndRunning();
			return false;
		}
		return true;
	}

	void CQuestManager::RegisterNPCVnum(uint32_t dwVnum)
	{
		if (m_registeredNPCVnum.find(dwVnum) != m_registeredNPCVnum.end())
			return;

		m_registeredNPCVnum.insert(dwVnum);

		char buf[256];
		DIR* dir;

		for (itertype(g_setQuestObjectDir) it = g_setQuestObjectDir.begin(); it != g_setQuestObjectDir.end(); ++it)
		{
			const string& stQuestObjectDir = *it;
			snprintf(buf, sizeof(buf), "%s/%u", stQuestObjectDir.c_str(), dwVnum);
			sys_log(0, "%s", buf);

			if ((dir = opendir(buf)))
			{
				closedir(dir);
				snprintf(buf, sizeof(buf), "%u", dwVnum);
				sys_log(0, "%s", buf);

				m_mapNPC[dwVnum].Set(dwVnum, buf);
			}
		}
	}

	void CQuestManager::WriteRunningStateToSyserr()
	{
		const char * state_name = GetQuestStateName(GetCurrentQuestName(), GetCurrentState()->st);

		string event_index_name = "";
		for (auto it : m_mapEventName)
		{
			if (it.second == m_iRunningEventIndex)
			{
				event_index_name = it.first;
				break;
			}
		}

		sys_err("LUA_ERROR: quest %s.%s %s", GetCurrentQuestName().c_str(), state_name, event_index_name.c_str() );
		if (GetCurrentCharacterPtr() && test_server)
			GetCurrentCharacterPtr()->ChatPacket(CHAT_TYPE_PARTY, "LUA_ERROR: quest %s.%s %s", GetCurrentQuestName().c_str(), state_name, event_index_name.c_str() );
	}

	void CQuestManager::QuestError(const char* func, int line, const char* fmt, ...)
	{
		char szMsg[4096];
		va_list args;

		va_start(args, fmt);
		vsnprintf(szMsg, sizeof(szMsg), fmt, args);
		va_end(args);

		_sys_err(func, line, "%s", szMsg);
		if (test_server)
		{
			LPCHARACTER ch = GetCurrentCharacterPtr();
			if (ch)
			{
				ch->ChatPacket(CHAT_TYPE_PARTY, "error occurred on [%s:%d]", func,line);
				ch->ChatPacket(CHAT_TYPE_PARTY, "%s", szMsg);
			}
		}
	}

	void CQuestManager::AddServerTimer(const std::string& name, uint32_t arg, LPEVENT event)
	{
		sys_log(0, "XXX AddServerTimer %s %d %p", name.c_str(), arg, get_pointer(event));
		if (m_mapServerTimer.find(make_pair(name, arg)) != m_mapServerTimer.end())
		{
			sys_err("already registered server timer name:%s arg:%u", name.c_str(), arg);
			return;
		}
		m_mapServerTimer.insert(make_pair(make_pair(name, arg), event));
	}

	void CQuestManager::ClearServerTimerNotCancel(const std::string& name, uint32_t arg)
	{
		m_mapServerTimer.erase(make_pair(name, arg));
	}

	void CQuestManager::ClearServerTimer(const std::string& name, uint32_t arg)
	{
		itertype(m_mapServerTimer) it = m_mapServerTimer.find(make_pair(name, arg));
		if (it != m_mapServerTimer.end())
		{
			LPEVENT event = it->second;
			event_cancel(&event);
			m_mapServerTimer.erase(it);
		}
	}

	void CQuestManager::CancelServerTimers(uint32_t arg)
	{
		itertype(m_mapServerTimer) it = m_mapServerTimer.begin();
		for ( ; it != m_mapServerTimer.end();) {
			if (it->first.second == arg) {
				LPEVENT event = it->second;
				event_cancel(&event);
				m_mapServerTimer.erase(it++);
			}
			else {
				++it;
			}
		}
	}

	void CQuestManager::SetServerTimerArg(uint32_t dwArg)
	{
		m_dwServerTimerArg = dwArg;
	}

	uint32_t CQuestManager::GetServerTimerArg()
	{
		return m_dwServerTimerArg;
	}

	void CQuestManager::SelectDungeon(LPDUNGEON pDungeon)
	{
		m_pSelectedDungeon = pDungeon;
	}

#ifdef ENABLE_12ZI
	void CQuestManager::SelectZodiac(LPZODIAC pZodiac)
	{
		m_pSelectedZodiac = pZodiac;
	}
#endif

	bool CQuestManager::PickupItem(unsigned int pc, LPITEM item)
	{
		if (test_server)
			sys_log( 0, "questmanager::PickupItem Start : itemVnum : %d PC : %d", item->GetOriginalVnum(), pc);
		PC* pPC;
		if ((pPC = GetPC(pc)))
		{
			if (!CheckQuestLoaded(pPC))
			{
				LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(pc);
				if (ch)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;772]");
				}
				return false;
			}
			// call script
			SetCurrentItem(item);

			return m_mapNPC[item->GetVnum()].OnPickupItem(*pPC);
		}
		else
		{
			sys_err("QUEST PICK_ITEM_EVENT no such pc id : %d", pc);
			return false;
		}
	}

	void CQuestManager::BeginOtherPCBlock(uint32_t pid)
	{
		LPCHARACTER ch = GetCurrentCharacterPtr();
		if (nullptr == ch)
		{
			sys_err("nullptr?");
			return;
		}
		/*
		# 1. current pid = pid0 <- It will be m_pOtherPCBlockRootPC.
		begin_other_pc_block(pid1)
			# 2. current pid = pid1
			begin_other_pc_block(pid2)
				# 3. current_pid = pid2
			end_other_pc_block()
		end_other_pc_block()
		*/
		// when begin_other_pc_block(pid1)
		if (m_vecPCStack.empty())
		{
			m_pOtherPCBlockRootPC = GetCurrentPC();
		}
		m_vecPCStack.emplace_back(GetCurrentCharacterPtr()->GetPlayerID());
		GetPC(pid);
	}

	void CQuestManager::EndOtherPCBlock()
	{
		if (m_vecPCStack.size() == 0)
		{
			sys_err("m_vecPCStack is alread empty. CurrentQuest{Name(%s), State(%s)}", GetCurrentQuestName().c_str(), GetCurrentState()->_title.c_str());
			return;
		}
		uint32_t pc = m_vecPCStack.back();
		m_vecPCStack.pop_back();
		GetPC(pc);

		if (m_vecPCStack.empty())
		{
			m_pOtherPCBlockRootPC = nullptr;
		}
	}

	bool CQuestManager::IsInOtherPCBlock()
	{
		return !m_vecPCStack.empty();
	}

	PC*	CQuestManager::GetOtherPCBlockRootPC()
	{
		return m_pOtherPCBlockRootPC;
	}

	bool CQuestManager::IsServerTimerArg(const std::string& name, uint32_t arg)
	{
		if (m_mapServerTimer.find(make_pair(name, arg)) != m_mapServerTimer.end())
			return true;

		return false;
	}
}
