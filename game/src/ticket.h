#ifndef __INC_METIN_II_GAME_TICKET_SYSTEM_H__
#define __INC_METIN_II_GAME_TICKET_SYSTEM_H__

#ifdef ENABLE_TICKET_SYSTEM
#define RECV_MODE_SORT "TICKET.MODE"
#define RECV_MODE_TIME "TICKET.TIMER"
#define RECV_UNKNOWN ""
#define RECV_NULL "BRAK"

enum ETicketSettings
{
	MAX_LOGS_PER_PAGE = 20,
	MAX_LOGS_FROM_REPLY = 40,
	MAX_LOGS_GENERAL = 40,
	MAX_LEN_TICKET_ID = 10,
	MAX_PAGE_ADMIN = 500,
	MAX_LEN_CONTENT_REPLY = 200,
	MAX_LEN_CONTENT = 320,
	MAX_LEN_TITLE = 32,
	MAX_LEN_REASON = 32,
	MAX_LEN_CHAR_NAME = 12,
};

enum ETicketAction
{
	ACTION_BAN = 1,
	ACTION_UNBAN = 2,
	ACTION_STATUS_OPEN = 3,
	ACTION_STATUS_CLOSED = 4,
};
enum ETicketStatus
{
	STATUS_OPENED = 1,
	STATUS_CLOSED = 2,
};
enum ETicketOpen
{
	PAGE_HOME = 1,
	PAGE_REPLY = 2,
	PAGE_SORT_ADMIN = 3,
};
enum ETicketTypeLogs
{
	LOGS_GENERAL = 1,
	LOGS_FROM_REPLY = 2,
	LOGS_ADMIN = 3,
};
enum ETicketMessageType
{
	MSG_REPLY,
	MSG_CLOSE
};

#pragma once
class CTicketSystem : public singleton<CTicketSystem>
{
public:
	CTicketSystem();
	~CTicketSystem();

	int GetFilter(LPCHARACTER ch);

	bool GetLastTime(LPCHARACTER ch);
	bool GetIsOpened(const char* strID);
	bool GetExistID(const char* strID);
	bool GetAccountBanned(LPCHARACTER ch);
	bool GetIsMemberStaff(LPCHARACTER ch);
	bool GetOwner(LPCHARACTER ch, const char* strID);
	bool GetLastTimeUsed(LPCHARACTER ch, int mode);

	bool IsDenied(LPCHARACTER ch, const char* argument);

	void SetLastTimeUsed(LPCHARACTER ch, int mode);
	bool SendTicketLogs(LPCHARACTER ch, int _kwargsMode, const char* szTicketID, int iStartPage, int iSortMode);

	void Open(LPCHARACTER ch, int action, int mode, const char* ticked_id);
	void Create(LPCHARACTER ch, const char* title, const char* content, int priority);
	void Reply(LPCHARACTER ch, const char* ticked_id, const char* reply);
	void TicketMessage(LPCHARACTER ch, const char* ticked_id, int type);
	void Action(LPCHARACTER ch, int action, const char* ticked_id, const char* char_name, const char* reason);
	void ChangePage(LPCHARACTER ch, int iStartPage);
};
#endif
#endif
