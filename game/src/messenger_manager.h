#ifndef __INC_MESSENGER_MANAGER_H
#define __INC_MESSENGER_MANAGER_H

#include "db.h"

class MessengerManager : public singleton<MessengerManager>
{
public:
	typedef std::string keyT;
	typedef const std::string& keyA;

#ifdef ENABLE_GM_MESSENGER_LIST
	typedef std::string keyGM;
	typedef const std::string& keyG;
#endif

#ifdef ENABLE_MESSENGER_BLOCK
	typedef std::string keyBL;
	typedef const std::string& keyB;
#endif

	MessengerManager();
	virtual ~MessengerManager();

public:
	void P2PLogin(keyA account);
	void P2PLogout(keyA account);

	void Login(keyA account);
	void Logout(keyA account);

	void RequestToAdd(LPCHARACTER ch, LPCHARACTER target);
	bool AuthToAdd(keyA account, keyA companion, bool bDeny); // @fixme130 void -> bool

	void __AddToList(keyA account, keyA companion); // 실제 m_Relation, m_InverseRelation 수정하는 메소드
	void AddToList(keyA account, keyA companion);

	void __RemoveFromList(keyA account, keyA companion); // 실제 m_Relation, m_InverseRelation 수정하는 메소드
	bool IsInList(const std::string& account, const std::string& companion);
	void RemoveFromList(keyA account, keyA companion);

	void RemoveAllList(keyA account);


#ifdef ENABLE_MESSENGER_BLOCK
	void __AddToBlockList(MessengerManager::keyB account, MessengerManager::keyB companion);
	void AddToBlockList(MessengerManager::keyB account, MessengerManager::keyB companion);

	void __RemoveFromBlockList(MessengerManager::keyB account, MessengerManager::keyB companion);
	bool IsBlocked(MessengerManager::keyB account, MessengerManager::keyB companion);
	void RemoveFromBlockList(MessengerManager::keyB account, MessengerManager::keyB companion);
	void RemoveAllBlockList(keyB account);
#endif

	void Initialize();

private:
	void SendList(keyA account);
	void SendLogin(keyA account, keyA companion);
	void SendLogout(keyA account, keyA companion);

	void LoadList(SQLMsg* pmsg);

#ifdef ENABLE_GM_MESSENGER_LIST
	void SendGMList(MessengerManager::keyG account);
	void SendGMLogin(MessengerManager::keyG account, MessengerManager::keyG companion);
	void SendGMLogout(MessengerManager::keyG account, MessengerManager::keyG companion);

	void LoadGMList(SQLMsg* pmsg);
#endif

#ifdef ENABLE_MESSENGER_BLOCK
	void SendBlockList(MessengerManager::keyB account);
	void SendBlockLogin(MessengerManager::keyB account, MessengerManager::keyB companion);
	void SendBlockLogout(MessengerManager::keyB account, MessengerManager::keyB companion);

	void LoadBlockList(SQLMsg* pmsg);
#endif

	void Destroy();

	std::set<keyT> m_set_loginAccount;
	std::map<keyT, std::set<keyT> > m_Relation;
	std::map<keyT, std::set<keyT> > m_InverseRelation;
	std::set<uint32_t> m_set_requestToAdd;

#ifdef ENABLE_GM_MESSENGER_LIST
	std::map<keyGM, std::set<keyGM> > m_GMRelation;
	std::map<keyGM, std::set<keyGM> > m_InverseGMRelation;
#endif

#ifdef ENABLE_MESSENGER_BLOCK
	std::map<keyBL, std::set<keyBL> > m_BlockRelation;
	std::map<keyBL, std::set<keyBL> > m_InverseBlockRelation;
	std::set<uint32_t> m_set_requestToBlockAdd;
#endif
};

#endif
