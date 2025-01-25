#pragma once
#include "Packet.h"

#ifdef ENABLE_TICKET_SYSTEM
class CPythonTicketLogs
{
	public:
		CPythonTicketLogs(void);
		~CPythonTicketLogs(void);
		void AddLogDetails(TSubPacketTicketLogsData p);
		TSubPacketTicketLogs Request(int);
		static CPythonTicketLogs* instance();
	private:
		std::vector<TSubPacketTicketLogs> m_vecData_A;
		static CPythonTicketLogs * curInstance_A;
};

class CPythonTicketLogsReply
{
	public:
		CPythonTicketLogsReply(void);
		~CPythonTicketLogsReply(void);
		void AddLogDetails(TSubPacketTicketLogsDataReply p);
		TSubPacketTicketLogsReply Request(int);
		static CPythonTicketLogsReply* instance();
	private:
		std::vector<TSubPacketTicketLogsReply> m_vecData_B;
		static CPythonTicketLogsReply * curInstance_B;
};
#endif
