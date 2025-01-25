#pragma once
#include "stdafx.h"

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
class CSoulRoulette
{
public:
	CSoulRoulette(LPCHARACTER ch);
	~CSoulRoulette();
	void TurnWheel();
	void GiveGift();
	uint32_t GetGiftVnum() const;
	void SendPacket(uint8_t option, int arg1 = 0 , int arg2 = 0);
	
	enum Error { LOAD, ADD, GIVE, SHUTDOWN };
	enum Packet { OPEN, CLOSE, TURN };

	static bool ReadRouletteData(bool NoMoreItem = false);
	static void StateError(uint8_t, LPCHARACTER);

	struct SRoulette
	{
		uint32_t vnum;
		uint8_t count;
		uint8_t chance; // max 255
		SRoulette(uint32_t m_vnum, uint8_t m_count, uint8_t m_chance)
			: vnum(m_vnum), count(m_count), chance(m_chance) {}

		struct ByChance
		{
			bool operator ()(const CSoulRoulette::SRoulette* a, const CSoulRoulette::SRoulette* b) const
			{
				return a->chance < b->chance;
			}
		};
	};

private:
	struct ErrorData
	{
		uint32_t vnum;
		uint8_t count;
		std::string name;
		ErrorData(uint32_t m_vnum, uint8_t m_count, const char* m_name)
			: vnum(m_vnum), count(m_count), name(m_name)
		{}
		struct FindName
		{
			std::string s_Name;
			FindName(const char* c_name) : s_Name(c_name) {}
			bool operator () (const ErrorData* p) { return !p->name.compare(s_Name); }
		};
	};
	
	void SetGift(const uint32_t vnum, const uint8_t count);
	int PickAGift();
	uint8_t GetGiftCount() const;
	uint16_t GetTurnCount() const;
	uint8_t GetChance() const;

	LPCHARACTER ch;
	uint32_t gift_vnum;
	uint8_t gift_count;
	uint16_t turn_count;
};
#endif
