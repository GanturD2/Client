#include "stdafx.h"
#include <sstream>
#include "../../common/length.h"

#include "db.h"

#include "config.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "item.h"
#include "item_manager.h"
#include "p2p.h"
#include "log.h"
#include "login_data.h"
#include "locale_service.h"
#include "spam.h"
#include "shutdown_manager.h"

DBManager::DBManager() : m_bIsConnect(false)
{
}

DBManager::~DBManager()
{
}

bool DBManager::Connect(const char* host, const int port, const char* user, const char* pwd, const char* db)
{
	if (m_sql.Setup(host, user, pwd, db, g_stLocale.c_str(), false, port))
		m_bIsConnect = true;

	if (!m_sql_direct.Setup(host, user, pwd, db, g_stLocale.c_str(), true, port))
		sys_err("cannot open direct sql connection to host %s", host);

	if (m_bIsConnect && !g_bAuthServer)
	{
		LoadDBString();
	}

	return m_bIsConnect;
}

void DBManager::Query(const char* c_pszFormat, ...)
{
	char szQuery[4096];
	va_list args;

	va_start(args, c_pszFormat);
	vsnprintf(szQuery, sizeof(szQuery), c_pszFormat, args);
	va_end(args);

	//m_sql.AsyncQuery(szQuery);
	std::string sQuery(szQuery);	//@fixme456
	m_sql.AsyncQuery(sQuery.substr(0, sQuery.find_first_of(";") == -1 ? sQuery.length() : sQuery.find_first_of(";")).c_str());
}

std::unique_ptr<SQLMsg> DBManager::DirectQuery(const char* c_pszFormat, ...)
{
	char szQuery[4096];
	va_list args;

	va_start(args, c_pszFormat);
	vsnprintf(szQuery, sizeof(szQuery), c_pszFormat, args);
	va_end(args);

	//return m_sql_direct.DirectQuery(szQuery);
	std::string sQuery(szQuery);	//@fixme456
	return m_sql_direct.DirectQuery(sQuery.substr(0, sQuery.find_first_of(";") == -1 ? sQuery.length() : sQuery.find_first_of(";")).c_str());
}

void DBManager::FuncQuery(std::function<void(SQLMsg*)> f, const char* c_pszFormat, ...)
{
	char szQuery[4096];
	va_list args;

	va_start(args, c_pszFormat);
	vsnprintf(szQuery, 4096, c_pszFormat, args);
	va_end(args);

	CFuncQueryInfo* p = M2_NEW CFuncQueryInfo;

	p->iQueryType = QUERY_TYPE_FUNCTION;
	p->f = f;

	m_sql.ReturnQuery(szQuery, p);
}

void DBManager::FuncAfterQuery(std::function<void()> f, const char* c_pszFormat, ...)
{
	char szQuery[4096];
	va_list args;

	va_start(args, c_pszFormat);
	vsnprintf(szQuery, 4096, c_pszFormat, args);
	va_end(args);

	CFuncAfterQueryInfo* p = M2_NEW CFuncAfterQueryInfo;

	p->iQueryType = QUERY_TYPE_AFTER_FUNCTION;
	p->f = f;

	m_sql.ReturnQuery(szQuery, p);
}

bool DBManager::IsConnected()
{
	return m_bIsConnect;
}

void DBManager::ReturnQuery(int iType, uint32_t dwIdent, void* pvData, const char* c_pszFormat, ...)
{
	//sys_log(0, "ReturnQuery %s", c_pszQuery);
	char szQuery[4096];
	va_list args;

	va_start(args, c_pszFormat);
	vsnprintf(szQuery, sizeof(szQuery), c_pszFormat, args);
	va_end(args);

	CReturnQueryInfo* p = M2_NEW CReturnQueryInfo;

	p->iQueryType = QUERY_TYPE_RETURN;
	p->iType = iType;
	p->dwIdent = dwIdent;
	p->pvData = pvData;

	m_sql.ReturnQuery(szQuery, p);
}

SQLMsg* DBManager::PopResult()
{
	SQLMsg* p;

	if (m_sql.PopResult(&p))
		return p;

	return nullptr;
}

void DBManager::Process()
{
	SQLMsg* pMsg = nullptr;

	while ((pMsg = PopResult()))
	{
		if (nullptr != pMsg->pvUserData)
		{
			switch (reinterpret_cast<CQueryInfo*>(pMsg->pvUserData)->iQueryType)
			{
				case QUERY_TYPE_RETURN:
					AnalyzeReturnQuery(pMsg);
					break;

				case QUERY_TYPE_FUNCTION:
					{
						CFuncQueryInfo* qi = reinterpret_cast<CFuncQueryInfo*>(pMsg->pvUserData);
						qi->f(pMsg);
						M2_DELETE(qi);
					}
					break;

				case QUERY_TYPE_AFTER_FUNCTION:
					{
						CFuncAfterQueryInfo* qi = reinterpret_cast<CFuncAfterQueryInfo*>(pMsg->pvUserData);
						qi->f();
						M2_DELETE(qi);
					}
					break;
			}
		}

		delete pMsg;
	}
}

CLoginData* DBManager::GetLoginData(uint32_t dwKey)
{
	std::map<uint32_t, CLoginData*>::iterator it = m_map_pkLoginData.find(dwKey);

	if (it == m_map_pkLoginData.end())
		return nullptr;

	return it->second;
}

void DBManager::InsertLoginData(CLoginData* pkLD)
{
	m_map_pkLoginData.insert(std::make_pair(pkLD->GetKey(), pkLD));
}

void DBManager::DeleteLoginData(CLoginData* pkLD)
{
	std::map<uint32_t, CLoginData*>::iterator it = m_map_pkLoginData.find(pkLD->GetKey());

	if (it == m_map_pkLoginData.end())
		return;

	sys_log(0, "DeleteLoginData %s %p", pkLD->GetLogin(), pkLD);

	M2_DELETE(it->second);
	m_map_pkLoginData.erase(it);
}

void DBManager::SendLoginPing(const char* c_pszLogin)
{
	TPacketGGLoginPing ptog{};

	ptog.bHeader = HEADER_GG_LOGIN_PING;
	strlcpy(ptog.szLogin, c_pszLogin, sizeof(ptog.szLogin));

	if (!g_pkAuthMasterDesc) // If I am master, broadcast to others
	{
		P2P_MANAGER::Instance().Send(&ptog, sizeof(TPacketGGLoginPing));
	}
	else // If I am slave send login ping to master
	{
		g_pkAuthMasterDesc->Packet(&ptog, sizeof(TPacketGGLoginPing));
	}
}

void DBManager::SendAuthLogin(LPDESC d)
{
	if (!d)
		return;

	const TAccountTable& r = d->GetAccountTable();

	CLoginData* pkLD = GetLoginData(d->GetLoginKey());
	if (!pkLD)
		return;

	TPacketGDAuthLogin ptod{};
	ptod.dwID = r.id;

	trim_and_lower(r.login, ptod.szLogin, sizeof(ptod.szLogin));
#ifdef ENABLE_HWID_BAN
	strlcpy(ptod.hwid, r.hwid, sizeof(ptod.hwid));
#endif
	strlcpy(ptod.szSocialID, r.social_id, sizeof(ptod.szSocialID));
	ptod.dwLoginKey = d->GetLoginKey();
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	ptod.bLanguage = r.bLanguage;
#endif

	thecore_memcpy(ptod.iPremiumTimes, pkLD->GetPremiumPtr(), sizeof(ptod.iPremiumTimes));
	thecore_memcpy(&ptod.adwClientKey, pkLD->GetClientKey(), sizeof(uint32_t) * 4);

	db_clientdesc->DBPacket(HEADER_GD_AUTH_LOGIN, d->GetHandle(), &ptod, sizeof(TPacketGDAuthLogin));
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	sys_log(0, "SendAuthLogin %s language %d key %u", ptod.szLogin, ptod.bLanguage, ptod.dwID);
#else
	sys_log(0, "SendAuthLogin %s key %u", ptod.szLogin, ptod.dwID);
#endif

	SendLoginPing(r.login);
}

void DBManager::LoginPrepare(LPDESC d, uint32_t* pdwClientKey, int* paiPremiumTimes)
{
	if (!d)
		return;

	const TAccountTable& r = d->GetAccountTable();

	CLoginData* pkLD = M2_NEW CLoginData;

	pkLD->SetKey(d->GetLoginKey());
	pkLD->SetLogin(r.login);
	pkLD->SetIP(d->GetHostName());
	pkLD->SetClientKey(pdwClientKey);

	if (paiPremiumTimes)
		pkLD->SetPremium(paiPremiumTimes);

	InsertLoginData(pkLD);
	SendAuthLogin(d);
}

void DBManager::AnalyzeReturnQuery(SQLMsg* pMsg)
{
	CReturnQueryInfo* qi = (CReturnQueryInfo*)pMsg->pvUserData;

	switch (qi->iType)
	{
		case QID_AUTH_LOGIN:
			{
				TPacketCGLogin3* pinfo = (TPacketCGLogin3*)qi->pvData;
				LPDESC d = DESC_MANAGER::Instance().FindByLoginKey(qi->dwIdent);

				if (!d)
				{
					M2_DELETE(pinfo);
					break;
				}
				//Change location - By SeMinZ
				d->SetLogin(pinfo->login);

				sys_log(0, "QID_AUTH_LOGIN: START %u %p", qi->dwIdent, get_pointer(d));

				if (pMsg->Get()->uiNumRows == 0)
				{
					{
						sys_log(0, " NOID");
						LoginFailure(d, "NOID");
						M2_DELETE(pinfo);
					}
				}
				else
				{
					MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
					int col = 0;

					// PASSWORD('%s'), password, securitycode, social_id, id, status
					char szEncrytPassword[45 + 1] = {0, };
					char szPassword[45 + 1] = {0, };
#ifdef ENABLE_HWID_BAN
					char szHWID[HWID_MAX_LEN + 1] = { 0, };
					char szHWID_new[HWID_MAX_LEN + 1] = { 0, };
#endif
					char szSocialID[SOCIAL_ID_MAX_LEN + 1] = {0, };
					char szStatus[ACCOUNT_STATUS_MAX_LEN + 1] = {0, };
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
					uint8_t bLanguage = LANGUAGE_DEFAULT;
#endif
					uint32_t dwID = 0;

					if (!row[col])
					{
						sys_err("error column %d", col);
						M2_DELETE(pinfo);
						break;
					}

					strlcpy(szEncrytPassword, row[col++], sizeof(szEncrytPassword));

					if (!row[col])
					{
						sys_err("error column %d", col);
						M2_DELETE(pinfo);
						break;
					}

					strlcpy(szPassword, row[col++], sizeof(szPassword));

#ifdef ENABLE_HWID_BAN
					if (!row[col])
					{
						sys_err("error column %d", col);
						M2_DELETE(pinfo);
						break;
					}

					strlcpy(szHWID, row[col++], sizeof(szHWID));

					if (!row[col])
					{
						sys_err("error column %d", col);
						M2_DELETE(pinfo);
						break;
					}

					strlcpy(szHWID_new, row[col++], sizeof(szHWID_new));
#endif

					if (!row[col])
					{
						sys_err("error column %d", col);
						M2_DELETE(pinfo);
						break;
					}

					strlcpy(szSocialID, row[col++], sizeof(szSocialID));

					if (!row[col])
					{
						sys_err("error column %d", col);
						M2_DELETE(pinfo);
						break;
					}

					str_to_number(dwID, row[col++]);

					if (!row[col])
					{
						sys_err("error column %d", col);
						M2_DELETE(pinfo);
						break;
					}

					strlcpy(szStatus, row[col++], sizeof(szStatus));

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
					if (!row[col])
					{
						sys_err("error column %d", col);
						M2_DELETE(pinfo);
						break;
					}
					str_to_number(bLanguage, row[col++]);
#endif

					uint8_t bNotAvail = 0;
					str_to_number(bNotAvail, row[col++]);

					int aiPremiumTimes[PREMIUM_MAX_NUM];
					memset(&aiPremiumTimes, 0, sizeof(aiPremiumTimes));

					char szCreateDate[256] = "00000000";

					{
						str_to_number(aiPremiumTimes[PREMIUM_EXP], row[col++]);
						str_to_number(aiPremiumTimes[PREMIUM_ITEM], row[col++]);
						str_to_number(aiPremiumTimes[PREMIUM_SAFEBOX], row[col++]);
						str_to_number(aiPremiumTimes[PREMIUM_AUTOLOOT], row[col++]);
						str_to_number(aiPremiumTimes[PREMIUM_FISH_MIND], row[col++]);
						str_to_number(aiPremiumTimes[PREMIUM_MARRIAGE_FAST], row[col++]);
						str_to_number(aiPremiumTimes[PREMIUM_GOLD], row[col++]);
//#ifdef ENABLE_AUTO_SYSTEM
						str_to_number(aiPremiumTimes[PREMIUM_AUTO_USE], row[col++]);
//#endif
//#ifdef ENABLE_YOHARA_SYSTEM
						str_to_number(aiPremiumTimes[PREMIUM_SUNGMA], row[col++]);
//#endif
//#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
						str_to_number(aiPremiumTimes[PREMIUM_PREMIUM_PRIVATE_SHOP], row[col++]);
//#endif
						{
							long retValue = 0;
							str_to_number(retValue, row[col]);

							time_t create_time = retValue;
							struct tm* tm1;
							tm1 = localtime(&create_time);
							strftime(szCreateDate, 255, "%Y%m%d", tm1);

							sys_log(0, "Create_Time %d %s", retValue, szCreateDate);
							sys_log(0, "Block Time %d ", strncmp(szCreateDate, g_stBlockDate.c_str(), 8));
						}
					}

					int nPasswordDiff = strcmp(szEncrytPassword, szPassword);

					if (nPasswordDiff)
					{
						LoginFailure(d, "WRONGPWD");
						sys_log(0, " WRONGPWD");
						M2_DELETE(pinfo);
					}
#ifdef ENABLE_HWID_BAN
					else if (strncmp(szHWID, "", 2) != 0)
					{
						LoginFailure(d, "HWIDBAN");
						sys_log(0, "   HWIDBAN");
						M2_DELETE(pinfo);
					}
#endif
					else if (bNotAvail)
					{
						LoginFailure(d, "NOTAVAIL");
						sys_log(0, " NOTAVAIL");
						M2_DELETE(pinfo);
					}
					else if (DESC_MANAGER::Instance().FindByLoginName(pinfo->login))
					{
						LoginFailure(d, "ALREADY");
						sys_log(0, " ALREADY");
						M2_DELETE(pinfo);
					}
					else if (!CShutdownManager::Instance().CheckCorrectSocialID(szSocialID) && !test_server)
					{
						LoginFailure(d, "BADSCLID");
						sys_log(0, " BADSCLID");
						M2_DELETE(pinfo);
					}
					else if (CShutdownManager::Instance().CheckShutdownAge(szSocialID) && CShutdownManager::Instance().CheckShutdownTime())
					{
						LoginFailure(d, "AGELIMIT");
						sys_log(0, " AGELIMIT");
						M2_DELETE(pinfo);
					}
					else if (strcmp(szStatus, "OK"))
					{
						LoginFailure(d, szStatus);
						sys_log(0, " STATUS: %s", szStatus);
						M2_DELETE(pinfo);
					}
					else
					{
						{
							//stBlockData >= 0 == Date is future than BlockDate
							if (strncmp(szCreateDate, g_stBlockDate.c_str(), 8) >= 0)
							{
								LoginFailure(d, "BLKLOGIN");
								sys_log(0, " BLKLOGIN");
								M2_DELETE(pinfo);
								break;
							}

							char szQuery[1024];
#ifdef ENABLE_HWID_BAN
							if (strlen(szHWID_new) < 64)
							{
								TPacketGCLoginFailure failurePacket;

								failurePacket.header = HEADER_GC_LOGIN_FAILURE;
								strlcpy(failurePacket.szStatus, "HWID", sizeof(failurePacket.szStatus));

								d->Packet(&failurePacket, sizeof(failurePacket));
								return;
							}

							char szHWID_new2[(HWID_MAX_LEN +1) * 2 + 1];
							DBManager::Instance().EscapeString(szHWID_new2, sizeof(szHWID_new2), szHWID_new, strlen(szHWID_new));

							snprintf(szQuery, sizeof(szQuery), "UPDATE account SET last_play=NOW(), hwid='%s' WHERE id=%u", szHWID_new2, dwID);
#else
							snprintf(szQuery, sizeof(szQuery), "UPDATE account SET last_play=NOW() WHERE id=%u", dwID);
#endif
							auto msg(DBManager::Instance().DirectQuery(szQuery));
						}

						TAccountTable& r = d->GetAccountTable();

						r.id = dwID;
						trim_and_lower(pinfo->login, r.login, sizeof(r.login));
						strlcpy(r.passwd, pinfo->passwd, sizeof(r.passwd));
#ifdef ENABLE_HWID_BAN
						strlcpy(r.hwid, pinfo->hwid, sizeof(r.hwid));
#endif
						strlcpy(r.social_id, szSocialID, sizeof(r.social_id));

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
						if ((bLanguage != pinfo->bLanguage) && (pinfo->bLanguage != ELanguages::LANGUAGE_NONE))
						{
							r.bLanguage = pinfo->bLanguage;

							TPacketGDLangaugeChange packet;
							packet.dwAID = r.id;
							packet.bLanguage = pinfo->bLanguage;
							packet.bState = false;
							db_clientdesc->DBPacket(HEADER_GD_LANGUAGE_CHANGE, d->GetHandle(), &packet, sizeof(TPacketGDLangaugeChange));
						}
						else
							r.bLanguage = bLanguage;
#endif

						DESC_MANAGER::Instance().ConnectAccount(r.login, d);

						LoginPrepare(d, pinfo->adwClientKey, aiPremiumTimes);
						M2_DELETE(pinfo);

						sys_log(0, "QID_AUTH_LOGIN: SUCCESS %s", pinfo->login);
					}
				}
			}
			break;

		case QID_SAFEBOX_SIZE:
			{
				LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(qi->dwIdent);

				if (ch)
				{
					if (pMsg->Get()->uiNumRows > 0)
					{
						MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
						int	size = 0;
						str_to_number(size, row[0]);
						ch->SetSafeboxSize(SAFEBOX_PAGE_SIZE * size);
					}
				}
			}
			break;

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
		case QID_GUILDSTORAGE_SIZE:
			{
				LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(qi->dwIdent);

				if (ch)
				{
					if (pMsg->Get()->uiNumRows > 0)
					{
						MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
						int size = 0;
						str_to_number(size, row[0]);
						ch->SetGuildstorageSize(SAFEBOX_PAGE_SIZE * size);
					}
				}
			}
			break;
#endif

		case QID_DB_STRING:
			{
				m_map_dbstring.clear();
				m_vec_GreetMessage.clear();

				for (uint i = 0; i < pMsg->Get()->uiNumRows; ++i)
				{
					MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
					//ch->SetSafeboxSize(SAFEBOX_PAGE_SIZE * atoi(row[0]));
					if (row[0] && row[1])
					{
						m_map_dbstring.insert(make_pair(std::string(row[0]), std::string(row[1])));
						sys_log(0, "DBSTR '%s' '%s'", row[0], row[1]);
					}
				}
				if (m_map_dbstring.find("GREET") != m_map_dbstring.end())
				{
					std::istringstream is(m_map_dbstring["GREET"]);
					while (!is.eof())
					{
						std::string str;
						getline(is, str);
						m_vec_GreetMessage.emplace_back(str);
					}
				}
			}
			break;

		case QID_LOTTO:
			{
				LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(qi->dwIdent);
				uint32_t* pdw = (uint32_t*)qi->pvData;

				if (ch)
				{
					if (pMsg->Get()->uiAffectedRows == 0 || pMsg->Get()->uiAffectedRows == (uint32_t)-1)
					{
						sys_log(0, "GIVE LOTTO FAIL TO pid %u", ch->GetPlayerID());
					}
					else
					{
						LPITEM pkItem = ch->AutoGiveItem(pdw[0], pdw[1]);

						if (pkItem)
						{
							sys_log(0, "GIVE LOTTO SUCCESS TO %s (pid %u)", ch->GetName(), qi->dwIdent);
							//ch->ChatPacket(CHAT_TYPE_INFO, "[LS;444;[IN;%d]]", pkItem->GetVnum());

							pkItem->SetSocket(0, static_cast<long>(pMsg->Get()->uiInsertID));
							pkItem->SetSocket(1, pdw[2]);
						}
						else
							sys_log(0, "GIVE LOTTO FAIL2 TO pid %u", ch->GetPlayerID());
					}
				}

				M2_DELETE_ARRAY(pdw);
			}
			break;

		case QID_HIGHSCORE_REGISTER:
			{
				THighscoreRegisterQueryInfo* info = (THighscoreRegisterQueryInfo*)qi->pvData;
				bool bQuery = true;

				if (pMsg->Get()->uiNumRows)
				{
					MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);

					if (row && row[0])
					{
						int iCur = 0;
						str_to_number(iCur, row[0]);

						if ((info->bOrder && iCur >= info->iValue) ||
							(!info->bOrder && iCur <= info->iValue))
							bQuery = false;
					}
				}

				if (bQuery)
					Query("REPLACE INTO highscore%s VALUES('%s', %u, %d)",
						get_table_postfix(), info->szBoard, info->dwPID, info->iValue);

				M2_DELETE(info);
			}
			break;

		case QID_HIGHSCORE_SHOW:
			{
			}
			break;

			// BLOCK_CHAT
		case QID_BLOCK_CHAT_LIST:
			{
				LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(qi->dwIdent);

				if (ch == nullptr)
					break;
				if (pMsg->Get()->uiNumRows)
				{
					MYSQL_ROW row;
					while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "%s %s sec", row[0], row[1]);
					}
				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "No one currently blocked.");
				}
			}
			break;
			// END_OF_BLOCK_CHAT

		default:
			sys_err("FATAL ERROR!!! Unhandled return query id %d", qi->iType);
			break;
	}

	M2_DELETE(qi);
}

void DBManager::LoadDBString()
{
	ReturnQuery(QID_DB_STRING, 0, nullptr, "SELECT name, text FROM string%s", get_table_postfix());
}

const std::string& DBManager::GetDBString(const std::string& key)
{
	static std::string null_str = "";
	auto it = m_map_dbstring.find(key);
	if (it == m_map_dbstring.end())
		return null_str;
	return it->second;
}

const std::vector<std::string>& DBManager::GetGreetMessage()
{
	return m_vec_GreetMessage;
}

void DBManager::SendMoneyLog(uint8_t type, uint32_t vnum, int gold)
{
	if (!gold)
		return;

	TPacketMoneyLog p{};
	p.type = type;
	p.vnum = vnum;
	p.gold = gold;
	db_clientdesc->DBPacket(HEADER_GD_MONEY_LOG, 0, &p, sizeof(p));
}

void DBManager::RequestBlockException(const char* login, int cmd)
{
	TPacketBlockException packet{};

	packet.cmd = cmd;
	strlcpy(packet.login, login, sizeof(packet.login));
	db_clientdesc->DBPacket(HEADER_GD_BLOCK_EXCEPTION, 0, &packet, sizeof(packet));
}

size_t DBManager::EscapeString(char* dst, size_t dstSize, const char* src, size_t srcSize)
{
	return m_sql_direct.EscapeString(dst, dstSize, src, srcSize);
}

//
// Common SQL
//
AccountDB::AccountDB() :
	m_IsConnect(false)
{
}

bool AccountDB::IsConnected()
{
	return m_IsConnect;
}

bool AccountDB::Connect(const char* host, const int port, const char* user, const char* pwd, const char* db)
{
	m_IsConnect = m_sql_direct.Setup(host, user, pwd, db, "", true, port);

	if (false == m_IsConnect)
	{
		fprintf(stderr, "cannot open direct sql connection to host: %s user: %s db: %s\n", host, user, db);
		return false;
	}

	return m_IsConnect;
}

bool AccountDB::ConnectAsync(const char* host, const int port, const char* user, const char* pwd, const char* db, const char* locale)
{
	m_sql.Setup(host, user, pwd, db, locale, false, port);
	return true;
}

void AccountDB::SetLocale(const std::string& stLocale)
{
	m_sql_direct.SetLocale(stLocale);
	m_sql_direct.QueryLocaleSet();
}

std::unique_ptr<SQLMsg> AccountDB::DirectQuery(const char* query)
{
	return m_sql_direct.DirectQuery(query);
}

void AccountDB::AsyncQuery(const char* query)
{
	m_sql.AsyncQuery(query);
}

void AccountDB::ReturnQuery(int iType, uint32_t dwIdent, void* pvData, const char* c_pszFormat, ...)
{
	char szQuery[4096];
	va_list args;

	va_start(args, c_pszFormat);
	vsnprintf(szQuery, sizeof(szQuery), c_pszFormat, args);
	va_end(args);

	CReturnQueryInfo* p = M2_NEW CReturnQueryInfo;

	p->iQueryType = QUERY_TYPE_RETURN;
	p->iType = iType;
	p->dwIdent = dwIdent;
	p->pvData = pvData;

	m_sql.ReturnQuery(szQuery, p);
}

SQLMsg* AccountDB::PopResult()
{
	SQLMsg* p;

	if (m_sql.PopResult(&p))
		return p;

	return nullptr;
}

void AccountDB::Process()
{
	SQLMsg* pMsg = nullptr;

	while ((pMsg = PopResult()))
	{
		CQueryInfo* qi = (CQueryInfo*)pMsg->pvUserData;

		switch (qi->iQueryType)
		{
			case QUERY_TYPE_RETURN:
				AnalyzeReturnQuery(pMsg);
				break;
		}
	}

	delete pMsg;
}

extern uint32_t g_uiSpamReloadCycle;

enum EAccountQID
{
	QID_SPAM_DB,
};

// reload every 10 minutes
static LPEVENT s_pkReloadSpamEvent = nullptr;

EVENTINFO(reload_spam_event_info)
{
	// used to send command
	uint32_t empty;
};

EVENTFUNC(reload_spam_event)
{
	AccountDB::Instance().ReturnQuery(QID_SPAM_DB, 0, nullptr, "SELECT word, score FROM spam_db WHERE type='SPAM'");
	return PASSES_PER_SEC(g_uiSpamReloadCycle);
}

void LoadSpamDB()
{
	AccountDB::Instance().ReturnQuery(QID_SPAM_DB, 0, nullptr, "SELECT word, score FROM spam_db WHERE type='SPAM'");
#ifdef ENABLE_SPAMDB_REFRESH
	if (nullptr == s_pkReloadSpamEvent)
	{
		reload_spam_event_info* info = AllocEventInfo<reload_spam_event_info>();
		s_pkReloadSpamEvent = event_create(reload_spam_event, info, PASSES_PER_SEC(g_uiSpamReloadCycle));
	}
#endif
}

void CancelReloadSpamEvent()
{
	s_pkReloadSpamEvent = nullptr;
}

void AccountDB::AnalyzeReturnQuery(SQLMsg* pMsg)
{
	CReturnQueryInfo* qi = (CReturnQueryInfo*)pMsg->pvUserData;

	switch (qi->iType)
	{
		case QID_SPAM_DB:
			{
				if (pMsg->Get()->uiNumRows > 0)
				{
					MYSQL_ROW row;

					SpamManager::Instance().Clear();

					while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
						SpamManager::Instance().Insert(row[0], atoi(row[1]));
				}
			}
			break;
	}

	M2_DELETE(qi);
}
