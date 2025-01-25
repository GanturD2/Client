#include "stdafx.h"

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
#include "constants.h"
#include "utils.h"
#include "desc.h"
#include "char.h"
#include "char_manager.h"
#include "mob_manager.h"
#include "db.h"
#include "config.h"
#include "questmanager.h"
#include "questlua.h"
#include "locale_service.h"

void CHARACTER::OpenLottoWindow()
{
	if(get_dword_time() < m_dwLastLottOpenMainTime)
		return;

	m_dwLastLottOpenMainTime = get_dword_time() + 2000;
	
	SendLottoBasicInfo();
	SendLottoTicketInfo();
	
	TPacketGCLotteryOpenings t;
	t.header = HEADER_GC_LOTTO_OPENINGS;
	t.subheader = SUBHEADER_GC_OPEN_LOTTERY_WINDOW;
	GetDesc()->Packet(&t, sizeof(t));

	return;
}

void CHARACTER::SendLottoBasicInfo()
{
	auto pMsg(DBManager::instance().DirectQuery("SELECT lotto_id, number1, number2, number3, number4, jackpot, UNIX_TIMESTAMP(next_numbers) FROM lotto_numbers ORDER BY lotto_id DESC LIMIT %d", 50));
	if (pMsg->uiSQLErrno)
		return;

	MYSQL_ROW row;
	
	int row_num = 0;
	while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
	{
		TPacketGCReciveLottoBaseInfo pack;
		pack.header = HEADER_GC_LOTTO_SEND_BASIC_INFO;
		pack.lottoSlot = row_num;
		pack.lottoNum = atoi(row[0]);
		pack.num1 = atoi(row[1]);
		pack.num2 = atoi(row[2]);
		pack.num3 = atoi(row[3]);
		pack.num4 = atoi(row[4]);
		pack.jackpot = atoll(row[5]);
		pack.nextRefresh = atol(row[6]);
		
		GetDesc()->Packet(&pack, sizeof(pack));
		row_num += 1;
	}
}

void CHARACTER::SendLottoTicketInfo()
{
	for (int i = 1; i <= 3; ++i)
	{
		char szQuery[1024];
		snprintf(szQuery, sizeof(szQuery), "SELECT ticket_id, number1, number2, number3, number4, for_lotto_id, buytime, state, win_numbers, money_win FROM player.lotto_tickets WHERE player_id = %d and slot = %d", GetPlayerID(), i );
		auto pMsg(DBManager::instance().DirectQuery(szQuery));
		
		if ( pMsg->Get()->uiNumRows > 0 )
		{
			MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
			
			TPacketGCReciveLottoTicketInfo pack;
			pack.header = HEADER_GC_LOTTO_SEND_TICKET_INFO;
			pack.tNum = i;
			pack.tID = atoi(row[0]);
			pack.num1 = atoi(row[1]);
			pack.num2 = atoi(row[2]);
			pack.num3 = atoi(row[3]);
			pack.num4 = atoi(row[4]);
			pack.lottoID = atoi(row[5]);
			strlcpy(pack.buytime, row[6], sizeof(pack.buytime));
			pack.state = atoi(row[7]);
			pack.winNumbers = atoi(row[8]);
			pack.MoneyWin = atoll(row[9]);
			
			GetDesc()->Packet(&pack, sizeof(pack));
		}
		else
		{
			TPacketGCReciveLottoTicketInfo pack;
			pack.header = HEADER_GC_LOTTO_SEND_TICKET_INFO;
			pack.tNum = i;
			pack.tID = 0;
			pack.num1 = 0;
			pack.num2 = 0;
			pack.num3 = 0;
			pack.num4 = 0;
			pack.lottoID = 0;
			strlcpy(pack.buytime, "", sizeof(pack.buytime));
			pack.state = 0;
			pack.winNumbers = 0;
			pack.MoneyWin = 0;
			
			GetDesc()->Packet(&pack, sizeof(pack));
		}
	}
}

void CHARACTER::SendLottoRankingInfo()
{
	if(get_dword_time() < m_dwLastLottoOpenRankingTime)
		return;

	m_dwLastLottoOpenRankingTime = get_dword_time() + 2000;
	
	auto pMsg_Jackpot(DBManager::instance().DirectQuery("SELECT player_name, lotto_ticket_id, money_win, date FROM log.lotto_log WHERE 1 = is_jackpot ORDER BY money_win DESC LIMIT 10"));
	if (pMsg_Jackpot->uiSQLErrno)
		return;

	MYSQL_ROW row_jackpot;
	
	while ((row_jackpot = mysql_fetch_row(pMsg_Jackpot->Get()->pSQLResult)))
	{
		TPacketGCSendRankingJackpotInfo pack;
		pack.header = HEADER_GC_LOTTO_SEND_RANKING_JACKPOT;
		strlcpy(pack.playername, row_jackpot[0], sizeof(pack.playername));
		pack.lottoID = atoi(row_jackpot[1]);
		pack.money = atoll(row_jackpot[2]);
		strlcpy(pack.date, row_jackpot[3], sizeof(pack.date));

		GetDesc()->Packet(&pack, sizeof(pack));
	}

	auto pMsg_Money(DBManager::instance().DirectQuery("SELECT account_id, name, level, lotto_totalmoneywin FROM player.player WHERE 0 < lotto_totalmoneywin ORDER BY lotto_totalmoneywin DESC LIMIT 10"));
	if (pMsg_Money->uiSQLErrno)
		return;

	MYSQL_ROW row_money;
	
	while ((row_money = mysql_fetch_row(pMsg_Money->Get()->pSQLResult)))
	{
		auto pMsgEmpire(DBManager::instance().DirectQuery("SELECT empire FROM player.player_index WHERE id = %d", atoi(row_money[0])));
		if (pMsgEmpire->uiSQLErrno)
			return;
		MYSQL_ROW row_empire = mysql_fetch_row(pMsgEmpire->Get()->pSQLResult);
		
		TPacketGCSendRankingMoneyInfo pack;
		pack.header = HEADER_GC_LOTTO_SEND_RANKING_MONEY;
		strlcpy(pack.playername, row_money[1], sizeof(pack.playername));
		pack.level = atoi(row_money[2]);
		pack.empire = atoi(row_empire[0]);
		pack.money = atoll(row_money[3]);
		
		GetDesc()->Packet(&pack, sizeof(pack));
	}
}
#endif
