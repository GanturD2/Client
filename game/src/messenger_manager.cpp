#include "stdafx.h"
#include "constants.h"
#include "gm.h"
#include "messenger_manager.h"
#include "buffer_manager.h"
#include "desc_client.h"
#include "log.h"
#include "config.h"
#include "p2p.h"
#include "crc32.h"
#include "char.h"
#include "char_manager.h"
#include "questmanager.h"
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	#include "AchievementSystem.h"
#endif

// @fixme142 BEGIN
static char	__account[CHARACTER_NAME_MAX_LEN*2+1];
static char	__companion[CHARACTER_NAME_MAX_LEN*2+1];
// @fixme142 END

MessengerManager::MessengerManager()
{
}

MessengerManager::~MessengerManager()
{
}

void MessengerManager::Initialize()
{
}

void MessengerManager::Destroy()
{
}

void MessengerManager::P2PLogin(MessengerManager::keyA account)
{
	Login(account);
}

void MessengerManager::P2PLogout(MessengerManager::keyA account)
{
	Logout(account);
}

void MessengerManager::Login(MessengerManager::keyA account)
{
	if (m_set_loginAccount.find(account) != m_set_loginAccount.end())
		return;

	// @fixme142 BEGIN
	DBManager::Instance().EscapeString(__account, sizeof(__account), account.c_str(), account.size());
	if (account.compare(__account))
		return;
	// @fixme142 END

	DBManager::instance().FuncQuery(std::bind(&MessengerManager::LoadList, this, std::placeholders::_1),
			"SELECT account, companion FROM messenger_list%s WHERE account='%s'", get_table_postfix(), __account);

#ifdef ENABLE_GM_MESSENGER_LIST
	DBManager::instance().FuncQuery(std::bind(&MessengerManager::LoadGMList, this, std::placeholders::_1),
			"SELECT '%s' AS `account`, `mName` AS `companion`, '0000-00-00 00:00:00' AS `time` FROM common.gmlist%s WHERE `mAuthority` = 'IMPLEMENTOR' OR mAuthority = 'HIGH_WIZARD' OR mAuthority = 'GOD' OR mAuthority = 'LOW_WIZARD'", account.c_str(), get_table_postfix());
#endif

#ifdef ENABLE_MESSENGER_BLOCK
	DBManager::instance().FuncQuery(std::bind(&MessengerManager::LoadBlockList, this, std::placeholders::_1),
			"SELECT account, companion FROM messenger_block_list%s WHERE account='%s'", get_table_postfix(), __account);
#endif

	m_set_loginAccount.insert(account);
}

void MessengerManager::LoadList(SQLMsg * msg)
{
	if (nullptr == msg)
		return;

	if (nullptr == msg->Get())
		return;

	if (msg->Get()->uiNumRows == 0)
		return;

	std::string account;

	sys_log(1, "Messenger::LoadList");

	for (uint i = 0; i < msg->Get()->uiNumRows; ++i)
	{
		MYSQL_ROW row = mysql_fetch_row(msg->Get()->pSQLResult);

		if (row[0] && row[1])
		{
			if (account.length() == 0)
				account = row[0];

			m_Relation[row[0]].insert(row[1]);
			m_InverseRelation[row[1]].insert(row[0]);
		}
	}

	SendList(account);

	std::set<MessengerManager::keyT>::iterator it;

	for (it = m_InverseRelation[account].begin(); it != m_InverseRelation[account].end(); ++it)
		SendLogin(*it, account);
}

void MessengerManager::Logout(MessengerManager::keyA account)
{
	if (m_set_loginAccount.find(account) == m_set_loginAccount.end())
		return;

	m_set_loginAccount.erase(account);

	std::set<MessengerManager::keyT>::iterator it;

	for (it = m_InverseRelation[account].begin(); it != m_InverseRelation[account].end(); ++it)
	{
		SendLogout(*it, account);
	}

	std::map<keyT, std::set<keyT> >::iterator it2 = m_Relation.begin();

	while (it2 != m_Relation.end())
	{
		it2->second.erase(account);
		++it2;
	}
	m_Relation.erase(account);

#ifdef ENABLE_GM_MESSENGER_LIST
	std::set<MessengerManager::keyGM>::iterator it5;

	for (it5 = m_InverseGMRelation[account].begin(); it5 != m_InverseGMRelation[account].end(); ++it5)
	{
		SendGMLogout(*it5, account);
	}

	std::map<keyGM, std::set<keyGM> >::iterator it6 = m_GMRelation.begin();

	while (it6 != m_GMRelation.end())
	{
		it6->second.erase(account);
		++it6;
	}
	m_GMRelation.erase(account);
#endif

#ifdef ENABLE_MESSENGER_BLOCK
	std::set<MessengerManager::keyBL>::iterator it3;

	for (it3 = m_InverseBlockRelation[account].begin(); it3 != m_InverseBlockRelation[account].end(); ++it3)
	{
		SendBlockLogout(*it3, account);
	}

	std::map<keyBL, std::set<keyBL> >::iterator it4 = m_BlockRelation.begin();

	while (it4 != m_BlockRelation.end())
	{
		it4->second.erase(account);
		++it4;
	}
	m_BlockRelation.erase(account);
#endif
}

void MessengerManager::RequestToAdd(LPCHARACTER ch, LPCHARACTER target)
{
	if (!ch->IsPC() || !target->IsPC())
		return;

	if (quest::CQuestManager::Instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
	{
	    ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방이 친구 추가를 받을 수 없는 상태입니다."));
	    return;
	}

	if (quest::CQuestManager::Instance().GetPCForce(target->GetPlayerID())->IsRunning() == true)
		return;

	uint32_t dw1 = GetCRC32(ch->GetName(), strlen(ch->GetName()));
	uint32_t dw2 = GetCRC32(target->GetName(), strlen(target->GetName()));

	char buf[64];
	snprintf(buf, sizeof(buf), "%u:%u", dw1, dw2);
	uint32_t dwComplex = GetCRC32(buf, strlen(buf));

	m_set_requestToAdd.insert(dwComplex);

	target->ChatPacket(CHAT_TYPE_COMMAND, "messenger_auth %s", ch->GetName());
}

// @fixme130 void -> bool
bool MessengerManager::AuthToAdd(MessengerManager::keyA account, MessengerManager::keyA companion, bool bDeny)
{
	uint32_t dw1 = GetCRC32(companion.c_str(), companion.length());
	uint32_t dw2 = GetCRC32(account.c_str(), account.length());

	char buf[64];
	snprintf(buf, sizeof(buf), "%u:%u", dw1, dw2);
	uint32_t dwComplex = GetCRC32(buf, strlen(buf));

	if (m_set_requestToAdd.find(dwComplex) == m_set_requestToAdd.end())
	{
		sys_log(0, "MessengerManager::AuthToAdd : request not exist %s -> %s", companion.c_str(), account.c_str());
		return false;
	}

	m_set_requestToAdd.erase(dwComplex);

	if (!bDeny)
	{
		AddToList(companion, account);
		AddToList(account, companion);

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		{
			auto chA = CHARACTER_MANAGER::Instance().FindPC(account.c_str());
			auto chB = CHARACTER_MANAGER::Instance().FindPC(companion.c_str());

			if (chA)
				CAchievementSystem::Instance().OnSocial(chA, achievements::ETaskTypes::TYPE_ADD_FRIEND);

			if (chB)
				CAchievementSystem::Instance().OnSocial(chB, achievements::ETaskTypes::TYPE_ADD_FRIEND);
		}
#endif
	}
	return true;
}

void MessengerManager::__AddToList(MessengerManager::keyA account, MessengerManager::keyA companion)
{
	m_Relation[account].insert(companion);
	m_InverseRelation[companion].insert(account);

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindPC(account.c_str());
	LPDESC d = ch ? ch->GetDesc() : nullptr;

	if (d)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;735;%s]", companion.c_str());
	}

	LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(companion.c_str());

	if (tch)
		SendLogin(account, companion);
	else
		SendLogout(account, companion);
}

void MessengerManager::AddToList(MessengerManager::keyA account, MessengerManager::keyA companion)
{
	if (companion.size() == 0)
		return;

	if (m_Relation[account].find(companion) != m_Relation[account].end())
		return;

	// @fixme142 BEGIN
	DBManager::Instance().EscapeString(__account, sizeof(__account), account.c_str(), account.size());
	DBManager::Instance().EscapeString(__companion, sizeof(__companion), companion.c_str(), companion.size());
	if (account.compare(__account) || companion.compare(__companion))
		return;
	// @fixme142 END

	sys_log(0, "Messenger Add %s %s", account.c_str(), companion.c_str());
	DBManager::Instance().Query("INSERT INTO messenger_list%s VALUES ('%s', '%s')",
			get_table_postfix(), __account, __companion);

	__AddToList(account, companion);

	TPacketGGMessenger p2ppck{};

	p2ppck.bHeader = HEADER_GG_MESSENGER_ADD;
	strlcpy(p2ppck.szAccount, account.c_str(), sizeof(p2ppck.szAccount));
	strlcpy(p2ppck.szCompanion, companion.c_str(), sizeof(p2ppck.szCompanion));
	P2P_MANAGER::Instance().Send(&p2ppck, sizeof(TPacketGGMessenger));
}

void MessengerManager::__RemoveFromList(MessengerManager::keyA account, MessengerManager::keyA companion)
{
	m_Relation[account].erase(companion);
	m_InverseRelation[companion].erase(account);

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindPC(account.c_str());
	LPDESC d = ch ? ch->GetDesc() : nullptr;

	if (d)
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;737;%s]", companion.c_str());
}

bool MessengerManager::IsInList(const std::string& account, const std::string& companion)
{
	if (m_Relation.find(account) == m_Relation.end())
		return false;

	if (m_Relation[account].empty())
		return false;

	return m_Relation[account].find(companion) != m_Relation[account].end();
}

void MessengerManager::RemoveFromList(MessengerManager::keyA account, MessengerManager::keyA companion)
{
	if (companion.size() == 0)
		return;

	// @fixme142 BEGIN
	DBManager::Instance().EscapeString(__account, sizeof(__account), account.c_str(), account.size());
	DBManager::Instance().EscapeString(__companion, sizeof(__companion), companion.c_str(), companion.size());
	if (account.compare(__account) || companion.compare(__companion))
		return;
	// @fixme142 END

	sys_log(1, "Messenger Remove %s %s", account.c_str(), companion.c_str());
	DBManager::Instance().Query("DELETE FROM messenger_list%s WHERE account='%s' AND companion = '%s'",
			get_table_postfix(), __account, __companion);

	__RemoveFromList(account, companion);

	TPacketGGMessenger p2ppck{};

	p2ppck.bHeader = HEADER_GG_MESSENGER_REMOVE;
	strlcpy(p2ppck.szAccount, account.c_str(), sizeof(p2ppck.szAccount));
	strlcpy(p2ppck.szCompanion, companion.c_str(), sizeof(p2ppck.szCompanion));
	P2P_MANAGER::Instance().Send(&p2ppck, sizeof(TPacketGGMessenger));
}

void MessengerManager::RemoveAllList(keyA account)
{
	std::set<keyT>	company(m_Relation[account]);

	// @fixme142 BEGIN
	DBManager::Instance().EscapeString(__account, sizeof(__account), account.c_str(), account.size());
	if (account.compare(__account))
		return;
	// @fixme142 END

	/* SQL Data 삭제 */
	DBManager::Instance().Query("DELETE FROM messenger_list%s WHERE account='%s' OR companion='%s'",
			get_table_postfix(), __account, __account);

	for (std::set<keyT>::iterator iter = company.begin(); iter != company.end(); ++iter)	//@fixme541
	{
		this->RemoveFromList(account, *iter);
		this->RemoveFromList(*iter, account); //@fixme183
	}

	for (std::set<keyT>::iterator iter = company.begin(); iter != company.end();)
	{
		company.erase(iter++);
	}

	company.clear();
}


void MessengerManager::SendList(MessengerManager::keyA account)
{
	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindPC(account.c_str());

	if (!ch)
		return;

	LPDESC d = ch->GetDesc();

	if (!d)
		return;

	if (m_Relation.find(account) == m_Relation.end())
		return;

	if (m_Relation[account].empty())
		return;

	TPacketGCMessenger pack{};

	pack.header		= HEADER_GC_MESSENGER;
	pack.subheader	= MESSENGER_SUBHEADER_GC_LIST;
	pack.size		= sizeof(TPacketGCMessenger);

	TPacketGCMessengerListOffline pack_offline{};
	TPacketGCMessengerListOnline pack_online{};

	TEMP_BUFFER buf(128 * 1024); // 128k

	itertype(m_Relation[account]) it = m_Relation[account].begin(), eit = m_Relation[account].end();

	while (it != eit)
	{
		if (m_set_loginAccount.find(*it) != m_set_loginAccount.end())
		{
			pack_online.connected = 1;

			// Online
			pack_online.length = it->size();

			buf.write(&pack_online, sizeof(TPacketGCMessengerListOnline));
			buf.write(it->c_str(), it->size());
		}
		else
		{
			pack_offline.connected = 0;

			// Offline
			pack_offline.length = it->size();

			buf.write(&pack_offline, sizeof(TPacketGCMessengerListOffline));
			buf.write(it->c_str(), it->size());
		}

		++it;
	}

	pack.size += buf.size();

	d->BufferedPacket(&pack, sizeof(TPacketGCMessenger));
	d->Packet(buf.read_peek(), buf.size());
}

void MessengerManager::SendLogin(MessengerManager::keyA account, MessengerManager::keyA companion)
{
	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindPC(account.c_str());
	LPDESC d = ch ? ch->GetDesc() : nullptr;

	if (!d)
		return;

	if (!d->GetCharacter())
		return;

	if (ch->GetGMLevel() == GM_PLAYER && gm_get_level(companion.c_str()) != GM_PLAYER)
		return;

	uint8_t bLen = companion.size();

	TPacketGCMessenger pack{};

	pack.header			= HEADER_GC_MESSENGER;
	pack.subheader		= MESSENGER_SUBHEADER_GC_LOGIN;
	pack.size			= sizeof(TPacketGCMessenger) + sizeof(uint8_t) + bLen;

	d->BufferedPacket(&pack, sizeof(TPacketGCMessenger));
	d->BufferedPacket(&bLen, sizeof(uint8_t));
	d->Packet(companion.c_str(), companion.size());
}

void MessengerManager::SendLogout(MessengerManager::keyA account, MessengerManager::keyA companion)
{
	if (!companion.size())
		return;

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindPC(account.c_str());
	LPDESC d = ch ? ch->GetDesc() : nullptr;

	if (!d)
		return;

	uint8_t bLen = companion.size();

	TPacketGCMessenger pack{};

	pack.header		= HEADER_GC_MESSENGER;
	pack.subheader	= MESSENGER_SUBHEADER_GC_LOGOUT;
	pack.size		= sizeof(TPacketGCMessenger) + sizeof(uint8_t) + bLen;

	d->BufferedPacket(&pack, sizeof(TPacketGCMessenger));
	d->BufferedPacket(&bLen, sizeof(uint8_t));
	d->Packet(companion.c_str(), companion.size());
}

#ifdef ENABLE_GM_MESSENGER_LIST
void MessengerManager::SendGMList(MessengerManager::keyG account)
{
	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindPC(account.c_str());

	if (!ch)
		return;

	LPDESC d = ch->GetDesc();

	if (!d)
		return;

	TPacketGCMessenger pack{};

	pack.header = HEADER_GC_MESSENGER;
	pack.subheader = MESSENGER_SUBHEADER_GC_GM_LIST;
	pack.size = sizeof(TPacketGCMessenger);

	TPacketGCMessengerGMListOnline pack_online{};
	TPacketGCMessengerGMListOffline pack_offline{};

	TEMP_BUFFER buf(128 * 1024);

	itertype(m_GMRelation[account]) it = m_GMRelation[account].begin(), eit = m_GMRelation[account].end();

	while (it != eit)
	{
		if (m_set_loginAccount.find(*it) != m_set_loginAccount.end())
		{
			pack_online.connected = 1;
			pack_online.length = it->size();

			buf.write(&pack_online, sizeof(TPacketGCMessengerGMListOnline));
			buf.write(it->c_str(), it->size());
		}
		else
		{
			pack_offline.connected = 0;
			pack_offline.length = it->size();

			buf.write(&pack_offline, sizeof(TPacketGCMessengerGMListOffline));
			buf.write(it->c_str(), it->size());
		}

		++it;
	}

	pack.size += buf.size();

	d->BufferedPacket(&pack, sizeof(TPacketGCMessenger));
	d->Packet(buf.read_peek(), buf.size());
}

void MessengerManager::SendGMLogin(MessengerManager::keyG account, MessengerManager::keyG companion)
{
	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindPC(account.c_str());
	LPDESC d = ch ? ch->GetDesc() : nullptr;

	if (!d)
		return;

	if (!d->GetCharacter())
		return;

	uint8_t bLen = companion.size();

	TPacketGCMessenger pack{};

	pack.header = HEADER_GC_MESSENGER;
	pack.subheader = MESSENGER_SUBHEADER_GC_GM_LOGIN;
	pack.size = sizeof(TPacketGCMessenger) + sizeof(uint8_t) + bLen;

	d->BufferedPacket(&pack, sizeof(TPacketGCMessenger));
	d->BufferedPacket(&bLen, sizeof(uint8_t));
	d->Packet(companion.c_str(), companion.size());
}

void MessengerManager::SendGMLogout(MessengerManager::keyG account, MessengerManager::keyG companion)
{
	if (!companion.size())
		return;

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindPC(account.c_str());
	LPDESC d = ch ? ch->GetDesc() : nullptr;

	if (!d)
		return;

	uint8_t bLen = companion.size();

	TPacketGCMessenger pack{};

	pack.header = HEADER_GC_MESSENGER;
	pack.subheader = MESSENGER_SUBHEADER_GC_GM_LOGOUT;
	pack.size = sizeof(TPacketGCMessenger) + sizeof(uint8_t) + bLen;

	d->BufferedPacket(&pack, sizeof(TPacketGCMessenger));
	d->BufferedPacket(&bLen, sizeof(uint8_t));
	d->Packet(companion.c_str(), companion.size());
}

void MessengerManager::LoadGMList(SQLMsg* msg)
{
	if (nullptr == msg || nullptr == msg->Get() || msg->Get()->uiNumRows == 0)
		return;

	std::string account;

	for (uint i = 0; i < msg->Get()->uiNumRows; ++i)
	{
		MYSQL_ROW row = mysql_fetch_row(msg->Get()->pSQLResult);

		if (row[0] && row[1])
		{
			if (account.length() == 0)
				account = row[0];

			m_GMRelation[row[0]].insert(row[1]);
			m_InverseGMRelation[row[1]].insert(row[0]);
		}
	}

	SendGMList(account);

	std::set<MessengerManager::keyGM>::iterator it;

	for (it = m_InverseGMRelation[account].begin(); it != m_InverseGMRelation[account].end(); ++it)
		SendGMLogin(*it, account);
}
#endif

#ifdef ENABLE_MESSENGER_BLOCK
void MessengerManager::SendBlockList(MessengerManager::keyB account)
{
	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindPC(account.c_str());

	if (!ch)
		return;

	LPDESC d = ch->GetDesc();

	if (!d)
		return;

	TPacketGCMessenger pack{};

	pack.header = HEADER_GC_MESSENGER;
	pack.subheader = MESSENGER_SUBHEADER_GC_BLOCK_LIST;
	pack.size = sizeof(TPacketGCMessenger);

	TPacketGCMessengerBlockListOnline pack_online{};
	TPacketGCMessengerBlockListOffline pack_offline{};

	TEMP_BUFFER buf(128 * 1024);

	itertype(m_BlockRelation[account]) it = m_BlockRelation[account].begin(), eit = m_BlockRelation[account].end();

	while (it != eit)
	{
		if (m_set_loginAccount.find(*it) != m_set_loginAccount.end())
		{
			pack_online.connected = 1;	// Online
			pack_online.length = it->size();

			buf.write(&pack_online, sizeof(TPacketGCMessengerBlockListOnline));
			buf.write(it->c_str(), it->size());
		}
		else
		{
			pack_offline.connected = 0;	// Offline
			pack_offline.length = it->size();

			buf.write(&pack_offline, sizeof(TPacketGCMessengerBlockListOffline));
			buf.write(it->c_str(), it->size());
		}

		++it;
	}

	pack.size += buf.size();

	d->BufferedPacket(&pack, sizeof(TPacketGCMessenger));
	d->Packet(buf.read_peek(), buf.size());
}

void MessengerManager::SendBlockLogin(MessengerManager::keyB account, MessengerManager::keyB companion)
{
	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindPC(account.c_str());
	LPDESC d = ch ? ch->GetDesc() : nullptr;

	if (!d)
		return;

	if (!d->GetCharacter())
		return;

	if (ch->GetGMLevel() == GM_PLAYER && gm_get_level(companion.c_str()) != GM_PLAYER)
		return;

	uint8_t bLen = companion.size();

	TPacketGCMessenger pack{};

	pack.header = HEADER_GC_MESSENGER;
	pack.subheader = MESSENGER_SUBHEADER_GC_BLOCK_LOGIN;
	pack.size = sizeof(TPacketGCMessenger) + sizeof(uint8_t) + bLen;

	d->BufferedPacket(&pack, sizeof(TPacketGCMessenger));
	d->BufferedPacket(&bLen, sizeof(uint8_t));
	d->Packet(companion.c_str(), companion.size());
}

void MessengerManager::SendBlockLogout(MessengerManager::keyB account, MessengerManager::keyB companion)
{
	if (!companion.size())
		return;

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindPC(account.c_str());
	LPDESC d = ch ? ch->GetDesc() : nullptr;

	if (!d)
		return;

	if (m_BlockRelation.find(account) == m_BlockRelation.end())
		return;

	if (m_BlockRelation[account].empty())
		return;

	uint8_t bLen = companion.size();

	TPacketGCMessenger pack{};

	pack.header = HEADER_GC_MESSENGER;
	pack.subheader = MESSENGER_SUBHEADER_GC_BLOCK_LOGOUT;
	pack.size = sizeof(TPacketGCMessenger) + sizeof(uint8_t) + bLen;

	d->BufferedPacket(&pack, sizeof(TPacketGCMessenger));
	d->BufferedPacket(&bLen, sizeof(uint8_t));
	d->Packet(companion.c_str(), companion.size());
}

void MessengerManager::LoadBlockList(SQLMsg* msg)
{
	if (nullptr == msg || nullptr == msg->Get() || msg->Get()->uiNumRows == 0)
		return;

	std::string account;

	for (uint i = 0; i < msg->Get()->uiNumRows; ++i)
	{
		MYSQL_ROW row = mysql_fetch_row(msg->Get()->pSQLResult);

		if (row[0] && row[1])
		{
			if (account.length() == 0)
				account = row[0];

			m_BlockRelation[row[0]].insert(row[1]);
			m_InverseBlockRelation[row[1]].insert(row[0]);
		}
	}

	SendBlockList(account);	//missing

	std::set<MessengerManager::keyBL>::iterator it;

	for (it = m_InverseBlockRelation[account].begin(); it != m_InverseBlockRelation[account].end(); ++it)
		SendBlockLogin(*it, account);
}

void MessengerManager::__AddToBlockList(MessengerManager::keyB account, MessengerManager::keyB companion)
{
	m_BlockRelation[account].insert(companion);
	m_InverseBlockRelation[companion].insert(account);

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindPC(account.c_str());
	LPDESC d = ch ? ch->GetDesc() : nullptr;

	if (d)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s is now blocked."), companion.c_str());
	}

	LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(companion.c_str());

	if (tch)
		SendBlockLogin(account, companion);
	else
		SendBlockLogout(account, companion);
}

void MessengerManager::AddToBlockList(MessengerManager::keyB account, MessengerManager::keyB companion)
{
	if (companion.size() == 0)
		return;

	if (m_BlockRelation[account].find(companion) != m_BlockRelation[account].end())
		return;

	// @fixme142 BEGIN
	DBManager::Instance().EscapeString(__account, sizeof(__account), account.c_str(), account.size());
	DBManager::Instance().EscapeString(__companion, sizeof(__companion), companion.c_str(), companion.size());
	if (account.compare(__account) || companion.compare(__companion))
		return;
	// @fixme142 END

	sys_log(0, "Messenger_Block Add %s %s", account.c_str(), companion.c_str());
	DBManager::Instance().Query("INSERT INTO messenger_block_list%s VALUES ('%s', '%s', NOW())",
		get_table_postfix(), __account, __companion);

	__AddToBlockList(account, companion);

	TPacketGGMessenger p2ppck{};

	p2ppck.bHeader = HEADER_GG_MESSENGER_BLOCK_ADD;
	strlcpy(p2ppck.szAccount, account.c_str(), sizeof(p2ppck.szAccount));
	strlcpy(p2ppck.szCompanion, companion.c_str(), sizeof(p2ppck.szCompanion));
	P2P_MANAGER::Instance().Send(&p2ppck, sizeof(TPacketGGMessenger));
}

void MessengerManager::__RemoveFromBlockList(MessengerManager::keyB account, MessengerManager::keyB companion)
{
	m_BlockRelation[account].erase(companion);
	m_InverseBlockRelation[companion].erase(account);

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindPC(account.c_str());
	LPDESC d = ch ? ch->GetDesc() : nullptr;

	if (d)
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s is no longer blocked."), companion.c_str());
}

/*bool MessengerManager::IsBlocked(const std::string& c_szAccount, const std::string& c_szName)
{
    if (m_BlockRelation.empty())
        return false;

    std::string strAccount(c_szAccount), strName(c_szName);
	
	std::set<MessengerManager::keyBL>::iterator it;
    for (it = m_BlockRelation[strAccount].begin(); it != m_BlockRelation[strAccount].end(); ++it)
    {
        if (it->compare(strName) == 0)
            return true;
    }

    return false;
}*/

bool MessengerManager::IsBlocked(MessengerManager::keyB account, MessengerManager::keyB companion)
{
	if (m_BlockRelation.find(account) == m_BlockRelation.end())
		return false;

	if (m_BlockRelation[account].empty())
		return false;

	return m_BlockRelation[account].find(companion) != m_BlockRelation[account].end();
}

void MessengerManager::RemoveFromBlockList(MessengerManager::keyB account, MessengerManager::keyB companion)
{
	if (companion.size() == 0)
		return;

	// @fixme142 BEGIN
	DBManager::Instance().EscapeString(__account, sizeof(__account), account.c_str(), account.size());
	DBManager::Instance().EscapeString(__companion, sizeof(__companion), companion.c_str(), companion.size());
	if (account.compare(__account) || companion.compare(__companion))
		return;
	// @fixme142 END

	sys_log(1, "Messenger Remove %s %s", account.c_str(), companion.c_str());
	DBManager::Instance().Query("DELETE FROM messenger_block_list%s WHERE account='%s' AND companion = '%s'",
		get_table_postfix(), __account, __companion);

	__RemoveFromBlockList(account, companion);

	TPacketGGMessenger p2ppck{};

	p2ppck.bHeader = HEADER_GG_MESSENGER_BLOCK_REMOVE;
	strlcpy(p2ppck.szAccount, account.c_str(), sizeof(p2ppck.szAccount));
	strlcpy(p2ppck.szCompanion, companion.c_str(), sizeof(p2ppck.szCompanion));
	P2P_MANAGER::Instance().Send(&p2ppck, sizeof(TPacketGGMessenger));
}

void MessengerManager::RemoveAllBlockList(keyB account)
{
	std::set<keyBL>	company(m_BlockRelation[account]);

	// @fixme142 BEGIN
	DBManager::Instance().EscapeString(__account, sizeof(__account), account.c_str(), account.size());
	if (account.compare(__account))
		return;
	// @fixme142 END

	DBManager::Instance().Query("DELETE FROM messenger_block_list%s WHERE account='%s' OR companion='%s'",
		get_table_postfix(), __account, __account);

	for (std::set<keyBL>::iterator iter = company.begin(); iter != company.end(); ++iter)	//@fixme541
	{
		this->RemoveFromBlockList(account, *iter);
	}

	for (std::set<keyBL>::iterator iter = company.begin(); iter != company.end();)
	{
		company.erase(iter++);
	}

	company.clear();
}
#endif

