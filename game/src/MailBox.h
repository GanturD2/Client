/*
* Blackdragonx61 / Mali
* 07.09.2021
*/

#pragma once

#ifdef ENABLE_MAILBOX
#include "../../common/length.h"
#include "../../common/tables.h"

class CMailBox
{
public:
	CMailBox(const LPCHARACTER m_ch, const TMailBoxTable* pTable, const uint16_t Size);
	~CMailBox();

public:
	void ServerProcess(const uint8_t SubHeader, const uint8_t arg1 = 0, const uint8_t arg2 = 0) const;
	void Write(const char* const szName, const char* const szTitle, const char* const szMessage, const TItemPos& pos, const int iYang, const int iWon);
	void WriteFromLua(const char* const szName, const char* const szTitle, const char* const szMessage, const char* const szFromName, uint32_t dwItem, uint8_t bCount);
	
	static void Open(const LPCHARACTER ch);
	static void Create(const LPCHARACTER ch, const TMailBoxTable* pTable, const uint16_t Size);
	static void UnreadData(const LPCHARACTER ch);
	static void ResultUnreadData(const LPCHARACTER ch, TMailBoxRespondUnreadData* data);
	
	void CheckPlayer(const char* const szName) const;
	void CheckPlayerResult(const TMailBox* t);
	
	void AddData(const uint8_t button_idx, const uint8_t data_idx);
	
	void GetAllItems();
	bool GetItem(const uint8_t data_idx, const bool bAll);
	
	void DeleteAllMails();
	bool DeleteMail(const uint8_t data_idx, const bool bAll);

public:
	enum EMAILBOX_CG : uint8_t
	{
		MAILBOX_CG_CLOSE,
		MAILBOX_CG_ALL_DELETE,
		MAILBOX_CG_ALL_GET_ITEMS,
		MAILBOX_CG_DELETE,
		MAILBOX_CG_GET_ITEMS,
		MAILBOX_CG_ADD_DATA,
	};
	
	using MailVec = std::vector <TMailBoxTable>;
	MailVec& GetMailVec() { return vecMailBox; }

private:
	MailVec vecMailBox;
	LPCHARACTER Owner;

private:
	enum EMAILBOX_GC : uint8_t
	{
		MAILBOX_GC_OPEN,
		MAILBOX_GC_POST_WRITE_CONFIRM,
		MAILBOX_GC_POST_WRITE,
		MAILBOX_GC_SET,
		MAILBOX_GC_ADD_DATA,
		MAILBOX_GC_POST_GET_ITEMS,
		MAILBOX_GC_POST_DELETE,
		MAILBOX_GC_POST_ALL_DELETE,
		MAILBOX_GC_POST_ALL_GET_ITEMS,
		MAILBOX_GC_UNREAD_DATA,
		MAILBOX_GC_ITEM_EXPIRE,
		MAILBOX_GC_CLOSE,
		MAILBOX_GC_SYSTEM_CLOSE,
	};

	enum EMAILBOX_POST_ALL_DELETE : uint8_t
	{
		POST_ALL_DELETE_FAIL,
		POST_ALL_DELETE_OK,
		POST_ALL_DELETE_FAIL_EMPTY,
		POST_ALL_DELETE_FAIL_DONT_EXIST,
	};

	enum EMAILBOX_POST_ALL_GET_ITEMS : uint8_t
	{
		POST_ALL_GET_ITEMS_FAIL,
		POST_ALL_GET_ITEMS_OK,
		POST_ALL_GET_ITEMS_EMPTY,
		POST_ALL_GET_ITEMS_FAIL_DONT_EXIST,
		POST_ALL_GET_ITEMS_FAIL_CANT_GET,
		POST_ALL_GET_ITEMS_FAIL_USE_TIME_LIMIT,
	};

	enum EMAILBOX_POST_DELETE_FAIL : uint8_t
	{
		POST_DELETE_FAIL,
		POST_DELETE_OK,
		POST_DELETE_FAIL_EXIST_ITEMS,
		POST_TIME_OUT_DELETE,
	};

	enum EMAILBOX_POST_GET_ITEMS : uint8_t
	{
		POST_GET_ITEMS_FAIL,
		POST_GET_ITEMS_OK,
		POST_GET_ITEMS_NONE,
		POST_GET_ITEMS_NOT_ENOUGHT_INVENTORY,
		POST_GET_ITEMS_YANG_OVERFLOW,
		POST_GET_ITEMS_WON_OVERFLOW,
		POST_GET_ITEMS_FAIL_BLOCK_CHAR,
		POST_GET_ITEMS_USE_TIME_LIMIT,
		POST_GET_ITEMS_RESULT_MAX,
	};

	enum EMAILBOX_POST_WRITE : uint8_t
	{
		POST_WRITE_FAIL,
		POST_WRITE_OK,
		POST_WRITE_INVALID_NAME,
		POST_WRITE_TARGET_BLOCKED,
		POST_WRITE_BLOCKED_ME,
		POST_WRITE_FULL_MAILBOX,
		POST_WRITE_WRONG_TITLE,
		POST_WRITE_YANG_NOT_ENOUGHT,
		POST_WRITE_WON_NOT_ENOUGHT,
		POST_WRITE_WRONG_MESSAGE,
		POST_WRITE_WRONG_ITEM,
		POST_WRITE_LEVEL_NOT_ENOUGHT,
		POST_WRITE_USE_TIME_LIMIT,
	};
};
#endif
