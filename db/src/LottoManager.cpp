#include "stdafx.h"

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
#include <map>
#include "ClientManager.h"
#include "DBManager.h"
#include "LottoManager.h"
#include "Main.h"
#include "CsvReader.h"

CLottoManager::CLottoManager()
{
	//Initialize();
}

CLottoManager::~CLottoManager()
{
	//Destroy();
}

void CLottoManager::CheckRefreshTime()
{
	
	int lotto_rows = 0;
	
	char szQueryRows[512];
	snprintf(szQueryRows, sizeof(szQueryRows), "SELECT COUNT(*) FROM player.lotto_numbers");
	auto pMsgRow = CDBManager::instance().DirectQuery(szQueryRows);
	MYSQL_ROW row_count = mysql_fetch_row(pMsgRow->Get()->pSQLResult);

	if (atoi(row_count[0]) == 0 and NEW_LOTTERY_NUMBERS_ACTIVATE == 1)
	{
		long long next_jackpot = 100000000;
		
		int number_1 = number(1, 30);
		
		int number_2 = number(1, 30);
		while (number_2 == number_1) {
			number_2 = number(1, 30);
		}
		int number_3 = number(1, 30);
		while (number_3 == number_1 or number_3 == number_2 ) {
			number_3 = number(1, 30);
		}
		
		int number_4 = number(1, 30);
		while (number_4 == number_1 or number_4 == number_2 or number_4 == number_3) {
			number_4 = number(1, 30);
		}

		sys_log(0,"CLottoManager::New Numbers = %d , %d , %d , %d", number_1, number_2, number_3, number_4);
		
		char szInsertQuery[512];
		snprintf(szInsertQuery, sizeof(szInsertQuery), "INSERT INTO lotto_numbers SET number1=%d, number2=%d, number3=%d, number4=%d, create_time=NOW(), jackpot=%lld, next_jackpot=%lld, next_numbers=NOW() + INTERVAL %d SECOND", number_1, number_2, number_3, number_4, next_jackpot, next_jackpot, 60*2);
		CDBManager::instance().AsyncQuery(szInsertQuery);
	}
	else
	{
		char szQueryInfo[512];
		snprintf(szQueryInfo, sizeof(szQueryInfo), "SELECT jackpot, next_jackpot, UNIX_TIMESTAMP(next_numbers) FROM player.lotto_numbers WHERE lotto_id=%d", atoi(row_count[0]));
		auto pMsgInfo = CDBManager::instance().DirectQuery(szQueryInfo);
		
		if (pMsgInfo->Get()->uiNumRows == 0)
			return;

		MYSQL_ROW rows_info = mysql_fetch_row(pMsgInfo->Get()->pSQLResult);
		
		long long jackpot = atoll(rows_info[0]);
		long long next_jackpot = atoll(rows_info[1]);
		long next_time = atol(rows_info[2]);

		if(NEW_LOTTERY_NUMBERS_ACTIVATE == 1 and (next_time-10) < time(0))
		{
			int number_1 = number(1, 30);
			
			int number_2 = number(1, 30);
			while (number_2 == number_1) {
				number_2 = number(1, 30);
			}
			
			int number_3 = number(1, 30);
			while (number_3 == number_1 or number_3 == number_2 ) {
				number_3 = number(1, 30);
			}
			
			int number_4 = number(1, 30);
			while (number_4 == number_1 or number_4 == number_2 or number_4 == number_3) {
				number_4 = number(1, 30);
			}
			
			long long new_jackpot_wins = 0;
			
			char szTicketDatas[1024];
			snprintf(szTicketDatas, sizeof(szTicketDatas), "SELECT ticket_id, number1, number2, number3, number4, win_numbers, player_name FROM lotto_tickets WHERE for_lotto_id=%d", (atoi(row_count[0])+1));
			auto pMsgTickets = CDBManager::instance().DirectQuery(szTicketDatas);

			MYSQL_ROW	ticket_data;

			while (ticket_data = mysql_fetch_row(pMsgTickets->Get()->pSQLResult))
			{
				int win_numbers = 0;
				long long win_money = 0;

				// -----------------------------------------------------------------------

				if(atoi(ticket_data[1]) == number_1 or atoi(ticket_data[1]) == number_2 or atoi(ticket_data[1]) == number_3 or atoi(ticket_data[1]) == number_4)
					win_numbers += 1;
				
				if(atoi(ticket_data[2]) == number_1 or atoi(ticket_data[2]) == number_2 or atoi(ticket_data[2]) == number_3 or atoi(ticket_data[2]) == number_4)
					win_numbers += 1;
				
				if(atoi(ticket_data[3]) == number_1 or atoi(ticket_data[3]) == number_2 or atoi(ticket_data[3]) == number_3 or atoi(ticket_data[3]) == number_4)
					win_numbers += 1;
				
				if(atoi(ticket_data[4]) == number_1 or atoi(ticket_data[4]) == number_2 or atoi(ticket_data[4]) == number_3 or atoi(ticket_data[4]) == number_4)
					win_numbers += 1;

				// -----------------------------------------------------------------------

				if(win_numbers == 0)
					win_money = 0;
				
				if(win_numbers == 1) {
					win_money = jackpot / 1000 * PERC_REWARD_FOR_1_SAME_NUMBER;
					if(MAX_REWARD_FOR_1_SAME_NUMBER > 0 and win_money > MAX_REWARD_FOR_1_SAME_NUMBER)
						win_money = MAX_REWARD_FOR_1_SAME_NUMBER;
				}	
				
				if(win_numbers == 2) {
					win_money = jackpot / 1000 * PERC_REWARD_FOR_2_SAME_NUMBER;
					if(MAX_REWARD_FOR_2_SAME_NUMBER > 0 and win_money > MAX_REWARD_FOR_2_SAME_NUMBER)
						win_money = MAX_REWARD_FOR_2_SAME_NUMBER;
				}	
				
				if(win_numbers == 3) {
					win_money = jackpot / 1000 * PERC_REWARD_FOR_3_SAME_NUMBER;
					if(MAX_REWARD_FOR_3_SAME_NUMBER > 0 and win_money > MAX_REWARD_FOR_3_SAME_NUMBER)
						win_money = MAX_REWARD_FOR_3_SAME_NUMBER;
				}	
				
				if(win_numbers == 4)
					win_money = jackpot;

				// -----------------------------------------------------------------------
				
				new_jackpot_wins += win_money;
				
				int state = 0;
				if (win_money > 0)
					state = 1;
				if (win_money == 0)
					state = 2;
				
				char szInsertQuery[512];
				snprintf(szInsertQuery, sizeof(szInsertQuery), "UPDATE player.lotto_tickets SET state=%d, win_numbers=%d, money_win=%lld WHERE ticket_id=%d", state, win_numbers, win_money, atoi(ticket_data[0]));
				CDBManager::instance().AsyncQuery(szInsertQuery);
				
				int is_jackpot = 0;
				if(win_numbers == 4)
					is_jackpot = 1;

				char szInsertLogQuery[512];
				snprintf(szInsertLogQuery, sizeof(szInsertLogQuery), "INSERT INTO log.lotto_log SET player_name='%s', lotto_id=%d, lotto_ticket_id=%d, money_win=%lld, coins_win=%d, is_jackpot=%d, date=NOW()", ticket_data[6], atoi(row_count[0]), atoi(ticket_data[0]), win_money, 0, is_jackpot);
				CDBManager::instance().AsyncQuery(szInsertLogQuery);
			}
				
			sys_log(0,"CLottoManager::New Numbers = %d , %d , %d , %d", number_1, number_2, number_3, number_4);
			
			next_jackpot -= new_jackpot_wins;

			if(next_jackpot < MIN_JACKPOT_MONEY)
				next_jackpot = MIN_JACKPOT_MONEY;
			
			//int next_refresh = 60 * GENERATE_NEW_LOTTO_NUMBERS_PULSE_MIN;
			int next_refresh = 30;
			
			char szInsertQuery[512];
			snprintf(szInsertQuery, sizeof(szInsertQuery), "INSERT INTO lotto_numbers SET number1=%d, number2=%d, number3=%d, number4=%d, create_time=NOW(), jackpot=%lld, next_jackpot=%lld, next_numbers=NOW() + INTERVAL %d SECOND", number_1, number_2, number_3, number_4, next_jackpot, next_jackpot, next_refresh);
			//snprintf(szInsertQuery, sizeof(szInsertQuery), "INSERT INTO lotto_numbers SET number1=%d, number2=%d, number3=%d, number4=%d, create_time=NOW(), jackpot=%lld, next_jackpot=%lld, next_numbers=NOW() + INTERVAL %d SECOND", 1, 2, 3, 4, next_jackpot, next_jackpot, 60*2);
			CDBManager::instance().AsyncQuery(szInsertQuery);
		}
	}
}
#endif
