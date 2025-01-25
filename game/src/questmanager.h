#ifndef __METIN2_SERVER_QUEST_MANAGER__
#define __METIN2_SERVER_QUEST_MANAGER__

#include <unordered_map>
#include <iostream>
#include <iterator>
#include "questnpc.h"

class ITEM;
class CHARACTER;
class CDungeon;

#ifdef ENABLE_NEWSTUFF
enum ETL { ETL_NIL, ETL_CFUN, ETL_LNUM, ETL_LSTR };

class lua_Any
{
public:
	// non-merged union data
	lua_CFunction cfVal;
	lua_Number lnVal;
	lua_String lsVal;
	// specified type
	ETL type;

	lua_Any() : type(ETL_NIL) {}
	lua_Any(lua_CFunction a1) : cfVal(a1), type(ETL_CFUN) {}
	lua_Any(lua_Number a1) : lnVal(a1), type(ETL_LNUM) {}
	lua_Any(lua_String a1) : lsVal(a1), type(ETL_LSTR) {}
};

typedef struct luaC_tab {
	const char* name;
	lua_Any val;
} _luaC_tab;
#endif

namespace quest
{
	using namespace std;

	bool IsScriptTrue(const char* code, int size);
	string ScriptToString(const string& str);

	class CQuestManager : public singleton<CQuestManager>
	{
	public:
		enum
		{
			QUEST_SKIN_NOWINDOW,
			QUEST_SKIN_NORMAL,
			//QUEST_SKIN_CINEMATIC,
			QUEST_SKIN_SCROLL = 4,
			QUEST_SKIN_CINEMATIC = 5,
			QUEST_SKIN_COUNT
		};

		typedef map<string, int> TEventNameMap;
		typedef map<unsigned int, PC> PCMap;

	public:
		CQuestManager();
		virtual ~CQuestManager();

		bool Initialize();
		void Destroy();

		bool InitializeLua();
		lua_State* GetLuaState() { return L; }
		void AddLuaFunctionTable(const char* c_pszName, luaL_reg* preg, bool bCheckIfExists = false);
		void AddLuaFunctionSubTable(const char* c_pszName, const char* c_pszSubName, luaL_reg* preg);
#ifdef ENABLE_NEWSTUFF
		void AppendLuaFunctionTable(const char* c_pszName, luaL_reg* preg, bool bForceCreation = false);
		void AddLuaConstantGlobal(const char* c_pszName, lua_Number lNumber, bool bOverwrite = false);
		void AddLuaConstantInTable(const char* c_pszName, const char* c_pszSubName, lua_Number lNumber, bool bForceCreation = false);
		void AddLuaConstantInTable(const char* c_pszName, const char* c_pszSubName, const char* szString, bool bForceCreation = false);
		void AddLuaConstantSubTable(const char* c_pszName, const char* c_pszSubName, luaC_tab* preg);
#endif

		TEventNameMap m_mapEventName;

		QuestState OpenState(const string& quest_name, int state_index);
		void CloseState(QuestState& qs);
		bool RunState(QuestState& qs);

		PC* GetPC(unsigned int pc);
		PC* GetPCForce(unsigned int pc); // 현재 PC를 바꾸지 않고 PC 포인터를 가져온다.

		unsigned int GetCurrentNPCRace();
		const string& GetCurrentQuestName();
		unsigned int FindNPCIDByName(const string& name);

		//void SetCurrentNPCCharacterPtr(LPCHARACTER ch) { m_pkCurrentNPC = ch; }
		LPCHARACTER GetCurrentNPCCharacterPtr();

		void SetCurrentEventIndex(int index) { m_iRunningEventIndex = index; }

		bool UseItem(unsigned int pc, LPITEM item, bool bReceiveAll);
		bool PickupItem(unsigned int pc, LPITEM item);
		bool SIGUse(unsigned int pc, uint32_t sig_vnum, LPITEM item, bool bReceiveAll);
		bool TakeItem(unsigned int pc, unsigned int npc, LPITEM item);
		LPITEM GetCurrentItem();
		void ClearCurrentItem();
		void SetCurrentItem(LPITEM item);
#ifdef ENABLE_QUEST_DND_EVENT
		LPITEM GetCurrentDNDItem();
		void ClearCurrentDNDItem();
		void SetCurrentDNDItem(LPITEM item);
#endif
		void AddServerTimer(const string& name, uint32_t arg, LPEVENT event);
		void ClearServerTimer(const string& name, uint32_t arg);
		void ClearServerTimerNotCancel(const string& name, uint32_t arg);
		void CancelServerTimers(uint32_t arg);

		void SetServerTimerArg(uint32_t dwArg);
		uint32_t GetServerTimerArg();

		// event over state and stae
		bool ServerTimer(unsigned int npc, unsigned int arg);

		void Login(unsigned int pc, const char* c_pszQuestName = nullptr);
		void Logout(unsigned int pc);
		bool Timer(unsigned int pc, unsigned int npc);
		bool Click(unsigned int pc, LPCHARACTER pkNPC);
		void Kill(unsigned int pc, unsigned int npc);
#ifdef ENABLE_QUEST_DIE_EVENT
		void Die(unsigned int pc, unsigned int npc);
#endif
#ifdef ENABLE_QUEST_BOOT_EVENT
		bool Boot();
#endif
#ifdef ENABLE_QUEST_DND_EVENT
		bool DND(uint32_t pc, LPITEM item_dnd, LPITEM item_victim, bool bReceiveAll);
#endif
		void LevelUp(unsigned int pc);
		void AttrIn(unsigned int pc, LPCHARACTER ch, int attr);
		void AttrOut(unsigned int pc, LPCHARACTER ch, int attr);
		bool Target(unsigned int pc, uint32_t dwQuestIndex, const char* c_pszTargetName, const char* c_pszVerb);
		bool GiveItemToPC(unsigned int pc, LPCHARACTER pkChr);
		void Unmount(unsigned int pc);

		void QuestButton(unsigned int pc, unsigned int quest_index);
		void QuestInfo(unsigned int pc, unsigned int quest_index);

		void EnterState(uint32_t pc, uint32_t quest_index, int state);
		void LeaveState(uint32_t pc, uint32_t quest_index, int state);

		void Letter(uint32_t pc);
		void Letter(uint32_t pc, uint32_t quest_index, int state);

		void ItemInformer(unsigned int pc, unsigned int vnum); //독일선물기능
#ifdef ENABLE_EVENT_MANAGER
		void EventBegin(uint32_t pc);
		void EventEnd(uint32_t pc);
		void RequestEventQuest(const string& quest_name, uint32_t pc);
#endif
		//

		bool CheckQuestLoaded(PC* pc) { return pc && pc->IsLoaded(); }

		// event occurs in one state
		void Select(unsigned int pc, unsigned int selection);
		void Resume(unsigned int pc);

		int ReadQuestCategoryFile(uint16_t q_index);
		void Input(unsigned int pc, const char* msg);
		void Confirm(unsigned int pc, EQuestConfirmType confirm, unsigned int pc2 = 0);
		void SelectItem(unsigned int pc, unsigned int selection);
#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
		void SelectItemEx(unsigned int pc, unsigned int selection);
#endif

		void LogoutPC(LPCHARACTER ch);
		void StopAllRunningQuests(); //@fixme509
		void DisconnectPC(LPCHARACTER ch);

		QuestState* GetCurrentState() { return m_CurrentRunningState; }

		void LoadStartQuest(const string& quest_name, unsigned int idx);
		//bool CanStartQuest(const string& quest_name, const PC& pc);
		bool CanStartQuest(unsigned int quest_index, const PC& pc);
		bool CanStartQuest(unsigned int quest_index);
		bool CanEndQuestAtState(const string& quest_name, const string& state_name);

		LPCHARACTER GetCurrentCharacterPtr() { return m_pCurrentCharacter; }
		LPCHARACTER GetCurrentPartyMember() { return m_pCurrentPartyMember; }
		PC* GetCurrentPC() { return m_pCurrentPC; }

		LPDUNGEON GetCurrentDungeon();
		void SelectDungeon(LPDUNGEON pDungeon);

#ifdef ENABLE_12ZI
		LPZODIAC GetCurrentZodiac();
		void SelectZodiac(LPZODIAC pZodiac);
#endif

		void ClearScript();
		void SendScript();
		void AddScript(const string& str);

		void BuildStateIndexToName(const char* questName);

		int GetQuestStateIndex(const string& quest_name, const string& state_name);
		const char* GetQuestStateName(const string& quest_name, const int state_index);

		void SetSkinStyle(int iStyle);

		void SetNoSend() { m_bNoSend = true; }

		unsigned int LoadTimerScript(const string& name);

		//unsigned int RegisterQuestName(const string& name);

		void RegisterQuest(const string& name, unsigned int idx);
		unsigned int GetQuestIndexByName(const string& name);
		const string& GetQuestNameByIndex(unsigned int idx);

		void RequestSetEventFlag(const string& name, int value);

		void SetEventFlag(const string& name, int value);
		int GetEventFlag(const string& name);
		void BroadcastEventFlagOnLogin(LPCHARACTER ch);

		void SendEventFlagList(LPCHARACTER ch);

		void Reload();

		//void CreateAllButton(const string& quest_name, const string& button_name);
		void SetError() { m_bError = true; }
		void ClearError() { m_bError = false; }
		bool IsError() { return m_bError; }
		void WriteRunningStateToSyserr();
		void QuestError(const char* func, int line, const char* fmt, ...);
		void RegisterNPCVnum(uint32_t dwVnum);
		bool IsServerTimerArg(const string& name, uint32_t arg);

	private:
		LPDUNGEON m_pSelectedDungeon;
#ifdef ENABLE_12ZI
		LPZODIAC m_pSelectedZodiac;
#endif
		uint32_t m_dwServerTimerArg;

		map<pair<string, uint32_t>, LPEVENT> m_mapServerTimer;

		int m_iRunningEventIndex;

		map<string, int> m_mapEventFlag;

		void GotoSelectState(QuestState& qs);
		void GotoPauseState(QuestState& qs);
		void GotoEndState(QuestState& qs);
		void GotoInputState(QuestState& qs);
		void GotoConfirmState(QuestState& qs);
		void GotoSelectItemState(QuestState& qs);
#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
		void GotoSelectItemExState(QuestState& qs);
#endif

		lua_State* L;

		bool m_bNoSend;

		set<unsigned int> m_registeredNPCVnum;
		map<unsigned int, NPC> m_mapNPC;
		map<string, unsigned int> m_mapNPCNameID;
		map<string, unsigned int> m_mapTimerID;

		QuestState* m_CurrentRunningState;

		PCMap m_mapPC;

		LPCHARACTER m_pCurrentCharacter;
		LPCHARACTER m_pCurrentNPCCharacter;
		LPCHARACTER m_pCurrentPartyMember;
		PC* m_pCurrentPC;

		string m_strScript;
		bool m_bQuestInfoFlag;

		int m_iCurrentSkin;

		struct stringhash
		{
			size_t operator () (const string& str) const
			{
				const unsigned char* s = (const unsigned char*)str.c_str();
				const unsigned char* end = s + str.size();
				size_t h = 0;

				while (s < end)
				{
					h *= 16777619;
					h ^= (unsigned char)*(unsigned char*)(s++);
				}

				return h;

			}
		};

		typedef std::unordered_map<string, int, stringhash> THashMapQuestName;
		typedef std::unordered_map<unsigned int, vector<char> > THashMapQuestStartScript;

		THashMapQuestName m_hmQuestName;
		THashMapQuestStartScript m_hmQuestStartScript;
		map<unsigned int, string> m_mapQuestNameByIndex;

		bool m_bError;

	public:
		static bool ExecuteQuestScript(PC& pc, const string& quest_name, const int state, const char* code, const int code_size, vector<AArgScript*>* pChatScripts = nullptr, bool bUseCache = true);
		static bool ExecuteQuestScript(PC& pc, uint32_t quest_index, const int state, const char* code, const int code_size, vector<AArgScript*>* pChatScripts = nullptr, bool bUseCache = true);


		// begin_other_pc_blcok, end_other_pc_block을 위한 객체들.
	public:
		void BeginOtherPCBlock(uint32_t pid);
		void EndOtherPCBlock();
		bool IsInOtherPCBlock();
		PC* GetOtherPCBlockRootPC();
	private:
		PC* m_pOtherPCBlockRootPC;
		std::vector <uint32_t> m_vecPCStack;

#ifdef ENABLE_QUEST_RENEWAL
	public:
		std::map<uint16_t, unsigned int> QuestCategoryIndexMap;
		void ReadQuestCategoryToDict();
		int GetQuestCategoryByQuestIndex(uint16_t q_index);
#endif

#ifdef ENABLE_SUNG_MAHI_TOWER
		public:
			LPEVENT m_sung_mahi_reward_event;
#endif
	};
};

#endif
