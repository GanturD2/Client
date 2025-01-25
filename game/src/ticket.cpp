#include "stdafx.h"

#ifdef ENABLE_TICKET_SYSTEM
#include "desc.h"
#include "utils.h"
#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "affect.h"
#include "start_position.h"
#include "p2p.h"
#include "db.h"
#include "dungeon.h"
#include "castle.h"
#include <string>
#include <sstream>
#include "ticket.h"
#include <string>
#include <algorithm>
#include <boost/algorithm/string/replace.hpp>

using namespace std;
using namespace boost;
using namespace algorithm;

#define WAIT_TIMER_FOR_OPEN		10		// You can open ticket window once every 10 second.
#define WAIT_TIMER_FOR_CREATE	1*60*60	// You can create a ticket once every 1 hour.
#define WAIT_TIMER_FOR_REPLY	10*60	// You can reply to a ticket once every 10 minutes.
		
CTicketSystem::CTicketSystem()
{}
CTicketSystem::~CTicketSystem()
{}

const char* LC_TRANSLATE(const char* front_inserter)
{
	typedef map<const char *, const char *> TMapTranslate;
	TMapTranslate LC_TRANSLATE;

	LC_TRANSLATE["USER_ERROR_ACCOUNT_IS_BANNED"]				= "<<User>> You cannot use the Ticket System, your account is now blocked. Reason: %s";
	LC_TRANSLATE["USER_ERROR_LAST_TIME"]						= "<<User>> You have to wait [%d Day(s)] [%d Hour(s)] [%d Min] [%d Sec] to do that action.";
	LC_TRANSLATE["USER_ERROR_OWNER"]							= "<<User>> You are not allowed to use that function.";
	LC_TRANSLATE["USER_ERROR_TICKET_ID"]						= "<<User>> ID Ticket: %s, is no more in database to reply.";
	LC_TRANSLATE["USER_ERROR_STATUS_NOT_OPENED"]				= "<<User>> Curent status for TicketID: %s is closed !";
	
	LC_TRANSLATE["STAFF_ERROR_OPEN_SORT"]						= "<<STAFF>> Error sending argument, report that to -> Vegas.";
	LC_TRANSLATE["STAFF_ERROR_LOGS_ARGUMENTS_LOW"]				= "<<STAFF>> Error argument sended are smaller as 0.";
	LC_TRANSLATE["STAFF_ERROR_LOGS_ARGUMENTS_HIGH"]				= "<<STAFF>> Error argument sended. %d | %d, maxim is: %d";
	
	LC_TRANSLATE["STAFF_ERROR_LOGS_LOADED_MYSQL"]				= "<<STAFF>> I can not take the logs from the mysql ticket.list table.";	
	LC_TRANSLATE["STAFF_ERROR_CREATE"]							= "<<STAFF>> You are a staff member, you can not create a ticket.";
	LC_TRANSLATE["STAFF_ERROR_BAN_WITH_REASON"]					= "<<STAFF>> You have to put a reason to ban this user.";	
	LC_TRANSLATE["STAFF_ERROR_SEARCH_USER"]						= "<<STAFF>> Target not found.";
	LC_TRANSLATE["STAFF_ERROR_CANNOT_BAN_SELF"]					= "<<STAFF>> You cannot ban yourself.";
	LC_TRANSLATE["STAFF_ERROR_USER_ALREADY_IS_BANNED"]			= "<<STAFF>> Target is already banned.";
	LC_TRANSLATE["STAFF_ERROR_CANNOT_UNBAN_SELF"]				= "<<STAFF>> You cannot unban yourself.";	
	LC_TRANSLATE["STAFF_ERROR_USER_IS_NOT_BANNED"]				= "<<STAFF>> That account is not blocked.";		
	LC_TRANSLATE["STAFF_ERROR_CHANGE_STATUS_NOID"]				= "<<STAFF>> TicketID: %s, does not exist in database, you cannot change its status.";		
	LC_TRANSLATE["STAFF_ERROR_CHANGE_STATUS_ALREADY_IS_OPENED"]	= "<<STAFF>> TicketID: %s, already have its status open.";	
	LC_TRANSLATE["STAFF_ERROR_CHANGE_STATUS_ALREADY_IS_CLOSED"]	= "<<STAFF>> TicketID: %s, already have its status closed.";

	LC_TRANSLATE["STAFF_SUCCES_UNBANNED_USER"]					= "<<STAFF>> Player %s was successfully unbanned.";		
	LC_TRANSLATE["STAFF_SUCCES_BANNED_USER"]					= "<<STAFF>> Player %s is now blocked with the reason: %s.";		
	LC_TRANSLATE["STAFF_SUCCES_CHANGE_STATUS_TO_OPEN"]			= "<<STAFF>> Status for TicketID: %s was changed to open.";
	LC_TRANSLATE["STAFF_SUCCES_CHANGE_STATUS_TO_CLOSED"]		= "<<STAFF>> Status for TicketID: %s was changed to closed.";

	return LC_TRANSLATE[front_inserter];
}

static char ar_Elements[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
int ar_SizeElements = sizeof(ar_Elements) - 1;

char CTicketRandom()
{
    return ar_Elements[rand() % ar_SizeElements];
}

bool CTicketSystem::IsDenied(LPCHARACTER ch, const char* argument)
{
    static const char * dwListDenied[] =	// Add here characters that cannot be used in title / replys.
	{
		// '?',
		// '*',
		//'$',
		//'!',
		// '/',
		//'>',
		//'<',
		// '|',
		// ';',
		//':',
		//'}',
		//'{',
		//'[',
		//']',
		"%", // DO NOT REMOVE
		"#", // DO NOT REMOVE
		"@", // DO NOT REMOVE
		"^", // DO NOT REMOVE
		"&", // DO NOT REMOVE
		"DROP", // DO NOT REMOVE
		"drop", // DO NOT REMOVE
		// '"',
		// '\'',
		// '\\',
		"TRUNCATE" // DO NOT REMOVE
	};

	std::string strLine = argument;
	
	for (int i = 0; i < _countof(dwListDenied); i++)
	{
		if (strLine.find(dwListDenied[i]) != std::string::npos)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "You can not use special characters in the message.");
			return true;
        }
	}

	return false;
}

bool CTicketSystem::GetIsMemberStaff(LPCHARACTER ch)
{
	if (ch->GetGMLevel() >= GM_GOD)
		return true;
	else
		return false;

	/*if ((!strcmp("[DEV]Nativ", ch->GetName())) && ch->GetGMLevel() > GM_PLAYER)
		return true;
	else if ((!strcmp("Admin2", ch->GetName())) && ch->GetGMLevel() > GM_PLAYER)
		return true;
	else if ((!strcmp("Admin3", ch->GetName())) && ch->GetGMLevel() > GM_PLAYER)
		return true;
	else if ((!strcmp("Admin4", ch->GetName())) && ch->GetGMLevel() > GM_PLAYER)
		return true;
	else if ((!strcmp("Admin5", ch->GetName())) && ch->GetGMLevel() > GM_PLAYER)
		return true;
	else
		return false;*/
}

int CTicketSystem::GetFilter(LPCHARACTER ch)
{
	int iResultIdx = ch->GetQuestFlag(RECV_MODE_SORT);
	return (iResultIdx);
}

bool CTicketSystem::GetOwner(LPCHARACTER ch, const char* strID)
{
	char szQuery[QUERY_MAX_LEN + 1];
	snprintf(szQuery, sizeof(szQuery), "SELECT ticked_id FROM ticket.list WHERE ticked_id = '%s' and char_name = '%s'", strID, ch->GetName());
	auto dwExtractID(DBManager::Instance().DirectQuery(szQuery));

	return (dwExtractID->Get()->uiNumRows > 0 || (CTicketSystem::Instance().GetIsMemberStaff(ch))) ? true : false;
}

bool CTicketSystem::GetIsOpened(const char* strID)
{
	char szQuery[QUERY_MAX_LEN + 1];
	snprintf(szQuery, sizeof(szQuery), "SELECT status FROM ticket.list WHERE ticked_id = '%s'", strID);
	auto dwExtractStatus(DBManager::Instance().DirectQuery(szQuery));
	MYSQL_ROW row = mysql_fetch_row(dwExtractStatus->Get()->pSQLResult);
	
	int result = 0;
	str_to_number(result, row[0]);
	
	return (result == STATUS_OPENED) ? true : false;
}

bool CTicketSystem::GetExistID(const char* strID)
{
	char szQuery[QUERY_MAX_LEN + 1];
	snprintf(szQuery, sizeof(szQuery), "SELECT ticked_id FROM ticket.list WHERE ticked_id = '%s'", strID);
	auto dwExtractID(DBManager::Instance().DirectQuery(szQuery));
	
	return (dwExtractID->Get()->uiNumRows > 0) ? true : false;
}

bool CTicketSystem::GetAccountBanned(LPCHARACTER ch)
{
	char szQuery[QUERY_MAX_LEN + 1];
	snprintf(szQuery, sizeof(szQuery), "SELECT account_id,reason FROM ticket.user_restricted WHERE account_id = '%d'", ch->GetDesc()->GetAccountTable().id);
	auto dwExtractID(DBManager::Instance().DirectQuery(szQuery));
	MYSQL_ROW row = mysql_fetch_row(dwExtractID->Get()->pSQLResult);
	
	if (dwExtractID->Get()->uiNumRows > 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("USER_ERROR_ACCOUNT_IS_BANNED"), row[1]);
		return true;
	}

	return false;
}

bool CTicketSystem::GetLastTimeUsed(LPCHARACTER ch, int iGetMode)
{
	string strGetFlag;
	ostringstream str;

	str << RECV_MODE_TIME << iGetMode;
	strGetFlag = str.str();
	
	int vecTime = ch->GetQuestFlag(strGetFlag);

	if (vecTime && get_global_time() < vecTime)
	{	
		int total = vecTime - get_global_time();
		int sec = total % 60;
			total /= 60;
		int mins = total % 60;
			total /= 60;
		int hours = total % 24;
		int days = total / 24;
		
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("USER_ERROR_LAST_TIME"), days, hours, mins, sec);
		return false;
	}

	return true;
}

void CTicketSystem::SetLastTimeUsed(LPCHARACTER ch, int iRegMode)
{
	int iDam = 0;
	for (int row = 1; row <= 3; row++)
	{
		if (row == iRegMode && !CTicketSystem::Instance().GetIsMemberStaff(ch))
		{
			string strSaveFlag;
			ostringstream str;

			str << RECV_MODE_TIME << iRegMode;
			strSaveFlag = str.str();

			if (iRegMode == 1)
				iDam = WAIT_TIMER_FOR_OPEN;
			if (iRegMode == 2)
				iDam = WAIT_TIMER_FOR_CREATE;
			if (iRegMode == 3)
				iDam = WAIT_TIMER_FOR_REPLY;

			ch->SetQuestFlag(strSaveFlag, get_global_time() + iDam);
		}
	}
}

void TicketReplaceOld(string& line, string& oldString, string& newString)
{
	const size_t oldSize = oldString.length();

	if (oldSize > line.length())
		return;

	const size_t newSize = newString.length();
	
	for(size_t pos = 0; ; pos += newSize)
	{
		pos = line.find(oldString, pos);
		
		if (pos == string::npos)
			return;
		
		if (oldSize == newSize)
		{
			line.replace(pos, oldSize, newString);
		}
		else
		{
			line.erase(pos, oldSize);
			line.insert(pos, newString);
		}
	}
}

bool CTicketSystem::SendTicketLogs(LPCHARACTER ch, int dwModeLogs, const char* szTicketID, int iStartPage, int iSortMode)
{
	switch(dwModeLogs)
	{
		case LOGS_GENERAL:
		{
			TEMP_BUFFER buf;
			TPacketGCTicketSystem pack;
			TSubPacketTicketLogsData sub{};
			pack.wSize = sizeof(TPacketGCTicketSystem) + sizeof(TSubPacketTicketLogsData);
			pack.bSubHeader = TICKET_SUBHEADER_GC_LOGS;

			auto dwExtractList = DBManager::Instance().DirectQuery("SELECT * FROM ticket.list WHERE char_name = '%s' ORDER BY create_date DESC LIMIT %d", ch->GetName(), MAX_LOGS_GENERAL);
			if(dwExtractList->Get()->uiNumRows == 0)
				return false;

			MYSQL_ROW iCurRow;
			int idx = 0;
			
			if(dwExtractList->uiSQLErrno != 0)
				return false;

			while ((iCurRow = mysql_fetch_row(dwExtractList->Get()->pSQLResult)))
			{
				sub.logs[idx] = TSubPacketTicketLogs();
				strncpy(sub.logs[idx].ticked_id, iCurRow[1], sizeof(sub.logs[idx].ticked_id));
				strncpy(sub.logs[idx].title, iCurRow[3], sizeof(sub.logs[idx].title));
				strncpy(sub.logs[idx].content, iCurRow[4], sizeof(sub.logs[idx].content));
				str_to_number(sub.logs[idx].priority, iCurRow[5]);
				strncpy(sub.logs[idx].create_date, iCurRow[6], sizeof(sub.logs[idx].create_date) - 1);
				str_to_number(sub.logs[idx].status, iCurRow[7]);
				idx++;
			}

			if(dwExtractList->Get()->uiNumRows < MAX_LOGS_GENERAL)
			{
				while (idx < MAX_LOGS_GENERAL)
				{
					sub.logs[idx] = TSubPacketTicketLogs();
					strncpy(sub.logs[idx].ticked_id, RECV_UNKNOWN, sizeof(sub.logs[idx].ticked_id));
					strncpy(sub.logs[idx].title, RECV_UNKNOWN, sizeof(sub.logs[idx].title));
					strncpy(sub.logs[idx].content, RECV_UNKNOWN, sizeof(sub.logs[idx].content));
					sub.logs[idx].priority = 999;
					strncpy(sub.logs[idx].create_date, RECV_UNKNOWN, sizeof(sub.logs[idx].create_date) - 1);
					sub.logs[idx].status = 999;
					idx++;
				}
			}

			LPDESC desc = ch->GetDesc();
			if (!desc)
			{
				sys_err("User(%s)'s DESC is nullptr POINT.", ch->GetName());
				return false;
			}

			buf.write(&pack, sizeof(TPacketGCTicketSystem));
			buf.write(&sub, sizeof(TSubPacketTicketLogsData));
			desc->Packet(buf.read_peek(), buf.size());
		}
		break;
		
		case LOGS_FROM_REPLY:
		{
			TEMP_BUFFER buf;
			TPacketGCTicketSystem pack;
			TSubPacketTicketLogsDataReply sub{};
			pack.wSize = sizeof(TPacketGCTicketSystem) + sizeof(TSubPacketTicketLogsDataReply);
			pack.bSubHeader = TICKET_SUBHEADER_GC_LOGS_REPLY;

			auto dwExtractList = DBManager::Instance().DirectQuery("SELECT * FROM ticket.reply WHERE ticked_id = '%s' ORDER BY reply_date DESC LIMIT %d", szTicketID, MAX_LOGS_FROM_REPLY);
			MYSQL_ROW iCurRow;
			int idx = 0;
			
			if(dwExtractList->uiSQLErrno != 0)
				return false;

			while ((iCurRow = mysql_fetch_row(dwExtractList->Get()->pSQLResult)))
			{
				sub.logs[idx] = TSubPacketTicketLogsReply();
				strncpy(sub.logs[idx].reply_from, iCurRow[2], sizeof(sub.logs[idx].reply_from));
				strncpy(sub.logs[idx].reply_content, iCurRow[3], sizeof(sub.logs[idx].reply_content));
				strncpy(sub.logs[idx].reply_date, iCurRow[4], sizeof(sub.logs[idx].reply_date) - 1);
				idx++;
			}

			if(dwExtractList->Get()->uiNumRows < MAX_LOGS_FROM_REPLY)
			{
				while (idx < MAX_LOGS_FROM_REPLY)
				{
					sub.logs[idx] = TSubPacketTicketLogsReply();
					strncpy(sub.logs[idx].reply_from, RECV_NULL, sizeof(sub.logs[idx].reply_from));
					strncpy(sub.logs[idx].reply_content, RECV_UNKNOWN, sizeof(sub.logs[idx].reply_content));
					strncpy(sub.logs[idx].reply_date, RECV_UNKNOWN, sizeof(sub.logs[idx].reply_date) - 1);
					idx++;
				}
			}

			LPDESC desc = ch->GetDesc();
			if (!desc)
			{
				sys_err("User(%s)'s DESC is nullptr POINT.", ch->GetName());
				return false;
			}

			buf.write(&pack, sizeof(TPacketGCTicketSystem));
			buf.write(&sub, sizeof(TSubPacketTicketLogsDataReply));
			desc->Packet(buf.read_peek(), buf.size());
		}
		break;
		
		case LOGS_ADMIN:
		{
			char szQuery[QUERY_MAX_LEN + 1];
			int iStartIdx = 0;
			int iEndIdx = 0;
			
			for (int iCurIndex = 1; iCurIndex <= MAX_PAGE_ADMIN; iCurIndex++)
			{
				if (iCurIndex == iStartPage)
				{
					if (iStartPage == 1)
					{
						iStartIdx = 0;
						iEndIdx = MAX_LOGS_PER_PAGE;
						break;
					}
					else
					{
						iStartIdx = MAX_LOGS_PER_PAGE * (iCurIndex - 1);
						iEndIdx = MAX_LOGS_PER_PAGE * (iCurIndex - 1) + MAX_LOGS_PER_PAGE;
						break;
					}
				}
			}

			auto dwExtract(DBManager::Instance().DirectQuery("SELECT COUNT(*) FROM ticket.list"));
			if (dwExtract->Get()->uiNumRows == 0)
				return false;

			MYSQL_ROW maxRow = mysql_fetch_row(dwExtract->Get()->pSQLResult);
			
			int	iMaxRows = 0;
			str_to_number(iMaxRows, maxRow[0]);
			
			if (iStartIdx < 0 || iEndIdx < 0)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_LOGS_ARGUMENTS_LOW"));
				return false;
			}
			
			if (iEndIdx > (iMaxRows + MAX_LOGS_PER_PAGE))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_LOGS_ARGUMENTS_HIGH"), iStartIdx, iEndIdx, iMaxRows);
				return false;
			}

			typedef map<int, string> TMapString;
			typedef map<int, int> TMapInt;
			TMapString ticked_id, char_name, title, content, create_date;
			TMapInt priority, status;
			
			for (int idx = 0; idx < MAX_LOGS_PER_PAGE; ++idx)
			{
				ticked_id[idx] = RECV_NULL;
				char_name[idx] = RECV_NULL;
				title[idx] = RECV_NULL;
				content[idx] = RECV_NULL;
				create_date[idx] = RECV_NULL;
				
				priority[idx] = 0;
				status[idx] = 0;
			}

			if (iSortMode == 1)
				snprintf(szQuery, sizeof(szQuery), "SELECT * FROM ticket.list ORDER BY create_date DESC LIMIT %d, %d", iStartIdx, iEndIdx);
			else if (iSortMode == 2)
				snprintf(szQuery, sizeof(szQuery), "SELECT * FROM ticket.list WHERE status = %d ORDER BY create_date DESC LIMIT %d, %d", STATUS_OPENED, iStartIdx, iEndIdx);
			else if (iSortMode == 3)
				snprintf(szQuery, sizeof(szQuery), "SELECT * FROM ticket.list WHERE status = %d ORDER BY create_date DESC LIMIT %d, %d", STATUS_CLOSED, iStartIdx, iEndIdx);
			else if (iSortMode == 4)
				snprintf(szQuery, sizeof(szQuery), "SELECT * FROM ticket.list WHERE priority > 1 ORDER BY create_date DESC LIMIT %d, %d", iStartIdx, iEndIdx);

			auto dwExtractList = DBManager::Instance().DirectQuery(szQuery);

			if (dwExtractList->Get()->uiNumRows == 0)
				return false;

			MYSQL_ROW iCurRow;
			int idx = 0;
			int dwPriority = 0, dwStatus = 0;

			if(dwExtractList->uiSQLErrno != 0)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_LOGS_LOADED_MYSQL"));
				return false;
			}
					
			if (dwExtractList->Get()->uiNumRows > 0)
			{
				while ((iCurRow = mysql_fetch_row(dwExtractList->Get()->pSQLResult)))
				{
					const char* ar_DataRows[8] =
					{
						iCurRow[0],
						iCurRow[1],
						iCurRow[2],
						iCurRow[3],
						iCurRow[4],
						iCurRow[5],
						iCurRow[6],
						iCurRow[7]
					};				
							
					ticked_id[idx] = ar_DataRows[1];
					char_name[idx] = ar_DataRows[2];
					title[idx] = ar_DataRows[3];
					content[idx] = ar_DataRows[4];
					create_date[idx] = ar_DataRows[6];

					str_to_number(dwPriority, ar_DataRows[5]);
					str_to_number(dwStatus, ar_DataRows[7]);

					priority[idx] = dwPriority, status[idx] = dwStatus;
					idx++;
				}
			}
		
			// if(dwExtractList->Get()->uiNumRows > 0 && dwExtractList->Get()->uiNumRows < MAX_LOGS_PER_PAGE)
			// {
				// while (idx < MAX_LOGS_PER_PAGE)
				// {
					// ticked_id[idx] = RECV_NULL, char_name[idx] = RECV_NULL, title[idx] = RECV_NULL, content[idx] = RECV_NULL, create_date[idx] = RECV_NULL;
					// priority[idx] = 0, status[idx] = 0;
					// idx++;
				// }
			// }

			int index = 0;
			while (index < MAX_LOGS_PER_PAGE)
			{
				boost::replace_all(ticked_id[index], " ", "#");
				boost::replace_all(char_name[index], " ", "#");
				boost::replace_all(title[index], " ", "#");
				boost::replace_all(content[index], " ", "#");
				boost::replace_all(create_date[index], " ", "#");

				char szBuf[QUERY_MAX_LEN + 1];
				snprintf(szBuf, sizeof(szBuf), "ticket team_logs %d %s %s %s %s %d %s %d", index, ticked_id[index].c_str(), char_name[index].c_str(), title[index].c_str(), content[index].c_str(), priority[index], create_date[index].c_str(), status[index]);
				ch->ChatPacket(CHAT_TYPE_COMMAND, szBuf);
				index++;
			}
		}
		break;

		default:
			break;
	}
	
	return true;
}

void CTicketSystem::ChangePage(LPCHARACTER ch, int iStartPage)
{
	if (!ch)
		return;
	
	if (iStartPage < 0 || iStartPage > MAX_PAGE_ADMIN)
		return;
	
	if (!CTicketSystem::Instance().GetIsMemberStaff(ch))
		return;

	CTicketSystem::Instance().SendTicketLogs(ch, LOGS_ADMIN, RECV_UNKNOWN, iStartPage, CTicketSystem::Instance().GetFilter(ch));
}

void CTicketSystem::Open(LPCHARACTER ch, int action, int mode, const char * ticked_id)
{
	if (!ch)
		return;
	
	if (!(CTicketSystem::Instance().GetLastTimeUsed(ch, 1)))	
		return;
	
	if ((CTicketSystem::Instance().GetAccountBanned(ch)))
		return;
	
	if (action == 2 && strlen(ticked_id) <= 0)
		return;
	
	switch(action)
	{
		case PAGE_SORT_ADMIN:
		{
			if (mode > 0 && CTicketSystem::Instance().GetIsMemberStaff(ch))
			{
				ch->SetQuestFlag(RECV_MODE_SORT, mode);
				CTicketSystem::Instance().SendTicketLogs(ch, LOGS_ADMIN, RECV_UNKNOWN, 1, mode);
				ch->ChatPacket(CHAT_TYPE_COMMAND, "ticket elevate o %d", 1);
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_OPEN_SORT"));
			}
		}
		break;
		
		case PAGE_HOME:
		{
			if (CTicketSystem::Instance().GetIsMemberStaff(ch))
			{
				ch->ChatPacket(CHAT_TYPE_COMMAND, "ticket elevate r %d", 1);
			}
			
			else if (!CTicketSystem::Instance().GetIsMemberStaff(ch))
			{
				CTicketSystem::Instance().SendTicketLogs(ch, LOGS_GENERAL, RECV_UNKNOWN, 0, 0);
				ch->ChatPacket(CHAT_TYPE_COMMAND, "ticket elevate o %d", 0);
			}
		}
		break;

		case PAGE_REPLY:
		{
			// if (CTicketSystem::Instance().IsDenied(ch, p->ticked_id))
				// return;
			
			if (!CTicketSystem::Instance().GetExistID(ticked_id))
				return;
			
			CTicketSystem::Instance().SendTicketLogs(ch, LOGS_FROM_REPLY, ticked_id, 0, 0);
		}
		break;

		default:
			break;
	}
}

void CTicketSystem::Create(LPCHARACTER ch, const char * title, const char * content, int priority)
{
	if (!ch)
		return;
	
	if (strlen(title) <= 0 || strlen(content) <= 0 || strlen(content) > MAX_LEN_CONTENT || strlen(title) > MAX_LEN_TITLE)
		return;
	
	if (CTicketSystem::Instance().GetIsMemberStaff(ch))
	{
		ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TRANSLATE("STAFF_ERROR_CREATE"));
		return;
	}
	
	if (!(CTicketSystem::Instance().GetLastTimeUsed(ch, 2)))	
		return;
	
	if (CTicketSystem::Instance().GetAccountBanned(ch))
		return;
	
	if (CTicketSystem::Instance().IsDenied(ch, title) || CTicketSystem::Instance().IsDenied(ch, content))
		return;

	string strID;

    srand(time(0));
    for(unsigned int i = 0; i < MAX_LEN_TICKET_ID; ++i) {
		strID += CTicketRandom();
	}

	auto dwExtract(DBManager::Instance().DirectQuery("SELECT ticked_id FROM ticket.list WHERE ticked_id = '%s'", strID.c_str()));

	while (dwExtract->Get()->uiNumRows > 0)
	{
		srand(time(0));
		for(unsigned int i = 0; i < MAX_LEN_TICKET_ID; ++i) {
			strID += CTicketRandom();
		}
	}
	
	char szQuery[QUERY_MAX_LEN + 1];
	snprintf(szQuery, sizeof(szQuery), "INSERT INTO ticket.list (ticked_id,char_name,title,content,priority,create_date,status) VALUES('%s','%s','%s','%s','%d',NOW(),'%d')", strID.c_str(), ch->GetName(), title, content, priority, STATUS_OPENED);
	DBManager::Instance().DirectQuery(szQuery);
	
	CTicketSystem::Instance().SetLastTimeUsed(ch, 2);
	CTicketSystem::Instance().SendTicketLogs(ch, LOGS_GENERAL, RECV_UNKNOWN, 0, 0);		
}

void CTicketSystem::Reply(LPCHARACTER ch, const char* ticked_id, const char* reply)
{
	if (!ch)
		return;
	
	if (strlen(ticked_id) <= 0 || strlen(reply) <= 0 || strlen(reply) > MAX_LEN_CONTENT_REPLY)
		return;
	
	if (!CTicketSystem::Instance().GetLastTimeUsed(ch, 3))	
		return;
	
	if ((CTicketSystem::Instance().GetAccountBanned(ch)))
		return;
	
	if (/*CTicketSystem::Instance().IsDenied(ch, p->ticked_id) || */CTicketSystem::Instance().IsDenied(ch, reply))
		return;

	string strReply = reply;
	
	if (!CTicketSystem::Instance().GetOwner(ch, ticked_id))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("USER_ERROR_OWNER"));
		return;
	}

	if (!CTicketSystem::Instance().GetExistID(ticked_id))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("USER_ERROR_TICKET_ID"), ticked_id);
		return;
	}
	
	if (!CTicketSystem::Instance().GetIsOpened(ticked_id))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("USER_ERROR_STATUS_NOT_OPENED"), ticked_id);
		return;
	}
	
	char szQuery[QUERY_MAX_LEN + 1];
	snprintf(szQuery, sizeof(szQuery), "INSERT INTO ticket.reply (ticked_id,reply_from,reply_content,reply_date) VALUES('%s','%s','%s',NOW())", ticked_id, ch->GetName(), strReply.c_str());
	DBManager::Instance().DirectQuery(szQuery);

	TicketMessage(ch, ticked_id, MSG_REPLY);

	CTicketSystem::Instance().SendTicketLogs(ch, LOGS_FROM_REPLY, ticked_id, 0, 0);
	CTicketSystem::Instance().SetLastTimeUsed(ch, 3);
}

void CTicketSystem::TicketMessage(LPCHARACTER ch, const char* ticked_id, int type)
{
	if (!ch->IsGM())
		return;

	auto pMsg(DBManager::Instance().DirectQuery("SELECT char_name FROM ticket.list WHERE ticked_id = '%s'", ticked_id));
	if (pMsg->Get()->uiNumRows == 0)
		return;

	MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
	const char * char_name = "";
	if (row[0] != 0)
		char_name = row[0];

	LPCHARACTER pkChr = CHARACTER_MANAGER::Instance().FindPC(char_name);
	if (!pkChr)
		return;

	LPDESC pkDesc = nullptr;
	pkDesc = pkChr->GetDesc();

	char msg[CHAT_MAX_LEN + 1];
	if (type == MSG_REPLY)
		snprintf(msg, sizeof(msg), LC_TEXT("[Ticket] You received an answer!"));
	else if (type == MSG_CLOSE)
		snprintf(msg, sizeof(msg), LC_TEXT("[Ticket] Your ticket has been closed!"));
	int len = MIN(CHAT_MAX_LEN, strlen(msg) + 1);

	TPacketGCWhisper pack{};
	pack.bHeader = HEADER_GC_WHISPER;
	pack.wSize = sizeof(TPacketGCWhisper) + len;
	pack.bType = WHISPER_TYPE_SYSTEM;
	strlcpy(pack.szNameFrom, "[Ticket]", sizeof(pack.szNameFrom));

	TEMP_BUFFER buf;
	buf.write(&pack, sizeof(TPacketGCWhisper));
	buf.write(msg, len);
	pkDesc->Packet(buf.read_peek(), buf.size());
}

void CTicketSystem::Action(LPCHARACTER ch, int action, const char * ticked_id, const char * char_name, const char * reason)
{
	if (strlen(ticked_id) > MAX_LEN_TICKET_ID || strlen(char_name) > MAX_LEN_CHAR_NAME || strlen(reason) > MAX_LEN_REASON)
		return;
	
	if (!CTicketSystem::Instance().GetIsMemberStaff(ch))
		return;

	switch(action)
	{
		case ACTION_BAN:
		{
			if (CTicketSystem::Instance().IsDenied(ch, char_name) || CTicketSystem::Instance().IsDenied(ch, reason))
				return;
			
			if (strlen(reason) <= 0)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_BAN_WITH_REASON"));		
				return;
			}
				
			auto dwExtractTarget(DBManager::Instance().DirectQuery("SELECT name FROM player.player WHERE name = '%s'", char_name));
				
			if (dwExtractTarget->uiSQLErrno != 0 || !dwExtractTarget->Get()->uiNumRows)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_SEARCH_USER"));
				return;
			}
				
			if (!strcmp(char_name, ch->GetName()))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_CANNOT_BAN_SELF"));
				return;
			}
				
			auto dwExtractName(DBManager::Instance().DirectQuery("SELECT account_id FROM player.player WHERE name = '%s'", char_name));
			MYSQL_ROW row = mysql_fetch_row(dwExtractName->Get()->pSQLResult);
				
			int account_id = 0;
			str_to_number(account_id, row[0]);

			auto dwExtractID(DBManager::Instance().DirectQuery("SELECT account_id FROM ticket.user_restricted WHERE account_id = '%d'", account_id));
				
			if (dwExtractID->Get()->uiNumRows > 0)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_USER_ALREADY_IS_BANNED"));
				return;
			}

			char szQuery[QUERY_MAX_LEN + 1];
			snprintf(szQuery, sizeof(szQuery), "INSERT INTO ticket.user_restricted (account_id, reason) VALUES('%d', '%s')", account_id, reason);
			DBManager::Instance().DirectQuery(szQuery);
				
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_SUCCES_BANNED_USER"), char_name, reason);
		}
		break;
		
		case ACTION_UNBAN:
		{
			if (CTicketSystem::Instance().IsDenied(ch, char_name))
				return;
				
			auto dwExtractTarget(DBManager::Instance().DirectQuery("SELECT name FROM player.player WHERE name = '%s'", char_name));
				
			if (dwExtractTarget->uiSQLErrno != 0 || !dwExtractTarget->Get()->uiNumRows)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_SEARCH_USER"));
				return;
			}
				
			if (!strcmp(char_name, ch->GetName()))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_CANNOT_UNBAN_SELF"));
				return;
			}
				
			auto dwExtractName(DBManager::Instance().DirectQuery("SELECT account_id FROM player.player WHERE name = '%s'", char_name));
			MYSQL_ROW row = mysql_fetch_row(dwExtractName->Get()->pSQLResult);
				
			int account_id = 0;
			str_to_number(account_id, row[0]);

			auto dwExtractID(DBManager::Instance().DirectQuery("SELECT account_id FROM ticket.user_restricted WHERE account_id = '%d'", account_id));
			
			if (dwExtractID->uiSQLErrno != 0 || !dwExtractID->Get()->uiNumRows)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_USER_IS_NOT_BANNED"));	
				return;
			}
				
			if (dwExtractID->Get()->uiNumRows > 0)
			{
				char szQuery[QUERY_MAX_LEN];
				snprintf(szQuery, sizeof(szQuery), "DELETE FROM ticket.user_restricted WHERE account_id = '%d'", account_id);
				auto pMsg(DBManager::Instance().DirectQuery(szQuery));	
			}
				
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_SUCCES_UNBANNED_USER"), char_name);
		}
		break;
		
		case ACTION_STATUS_OPEN:
		{
			// if (CTicketSystem::Instance().IsDenied(ch, p->ticked_id))
				// return;

			if (!CTicketSystem::Instance().GetExistID(ticked_id))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_CHANGE_STATUS_NOID"), ticked_id);
				return;
			}
			
			if (CTicketSystem::Instance().GetIsOpened(ticked_id))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_CHANGE_STATUS_ALREADY_IS_OPENED"), ticked_id);
				return;				
			}
			
			char szQuery[CHAT_MAX_LEN * 2 + 1];
			snprintf(szQuery, sizeof(szQuery), "UPDATE ticket.list SET status = '%d' WHERE ticked_id = '%s'", STATUS_OPENED, ticked_id);
			auto update(DBManager::Instance().DirectQuery(szQuery));
			
			CTicketSystem::Instance().SendTicketLogs(ch, LOGS_ADMIN, RECV_UNKNOWN, 0, CTicketSystem::Instance().GetFilter(ch));
			
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_SUCCES_CHANGE_STATUS_TO_OPEN"), ticked_id);
		}
		break;
		
		case ACTION_STATUS_CLOSED:
		{
			// if (CTicketSystem::Instance().IsDenied(ch, p->ticked_id))
				// return;

			if (!CTicketSystem::Instance().GetExistID(ticked_id))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_CHANGE_STATUS_NOID"), ticked_id);
				return;
			}
			
			if (!CTicketSystem::Instance().GetIsOpened(ticked_id))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_ERROR_CHANGE_STATUS_ALREADY_IS_CLOSED"), ticked_id);
				return;				
			}
			
			char szQuery[CHAT_MAX_LEN * 2 + 1];
			snprintf(szQuery, sizeof(szQuery), "UPDATE ticket.list SET status = '%d' WHERE ticked_id = '%s'", STATUS_CLOSED, ticked_id);
			auto update(DBManager::Instance().DirectQuery(szQuery));
			
			CTicketSystem::Instance().SendTicketLogs(ch, LOGS_ADMIN, RECV_UNKNOWN, 0, CTicketSystem::Instance().GetFilter(ch));
			
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TRANSLATE("STAFF_SUCCES_CHANGE_STATUS_TO_CLOSED"), ticked_id);
			TicketMessage(ch, ticked_id, MSG_CLOSE);
		}
		break;
		default:
			break;
	}
}
#endif
