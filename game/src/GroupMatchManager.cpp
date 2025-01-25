#include "stdafx.h"

#ifdef ENABLE_PARTY_MATCH
#include "GroupMatchManager.h"
#include "char.h"
#include "packet.h"
#include "desc.h"
#include "party.h"
#include <unordered_set>

CGroupMatchManager::CGroupMatchManager()
{
}

CGroupMatchManager::~CGroupMatchManager()
{
	SearchMap.clear();
}

static auto& Coordinates() {
	struct MapObject {
		int x, y, level, player_count;
		std::vector<std::pair<uint32_t, uint8_t>> items;
		MapObject(int m_x, int m_y, int m_level, int pcount, std::vector<std::pair<uint32_t, uint8_t>> m_items)
			: x(m_x), y(m_y), level(m_level), player_count(pcount), items(std::move(m_items))
		{}
	};

	static std::map<int, std::unique_ptr<MapObject>> Map;

	auto AddObject = [&](int index, int x, int y, int level, int pcount, std::vector<std::pair<uint32_t, uint8_t>> items) {
		auto p = std::make_unique<MapObject>(x, y, level, pcount, items);
		Map.emplace(index, std::move(p));
	};

	if (Map.empty()) {
		//AddObject(351, X, Y, LEVEL, COUNT, { {VNUM1, COUNT1}, {VNUM2, COUNT2}, {VNUM3, COUNT3} });
		AddObject(351, 7424, 6144, 100, 2, { {71095, 1}, {71130, 1}, {76019, 1} });			// Rotdrachenfestung
		AddObject(352, 5120, 1536, 100, 2, {});												// Nemeres Warte
		AddObject(353, 7680, 14080, 95, 2, { {30613 , 1} });								// Verwunschener Wald
		AddObject(354, 8448, 14080, 95, 2, {});												// Tempel der Ochao
		AddObject(356, 3072, 15104, 75, 2, { {71095, 1}, {71130, 1}, {76019, 1} });			// Meleys Hort
	}

	return Map;
};

void CGroupMatchManager::SendPacket(LPCHARACTER ch, uint8_t SubHeader, uint8_t MSG, uint32_t index)
{
	if (!ch || !ch->GetDesc())
		return;

	TPacketGCPartyMatch p{};

	p.Header = HEADER_GC_PARTY_MATCH;
	p.SubHeader = SubHeader;
	p.MSG = MSG;
	p.index = index;

	ch->GetDesc()->Packet(&p, sizeof(TPacketGCPartyMatch));
}

void CGroupMatchManager::AddSearcher(LPCHARACTER ch, int index)
{
	if (!ch)
		return;

	if (IsSearching(ch) != SearchMap.end()) {
		StopSearching(ch, EPartyMatchMsg::PARTY_MATCH_HOLD, index);
		return;
	}

	if (!index) {
		StopSearching(ch, EPartyMatchMsg::PARTY_MATCH_FAIL_NONE_MAP_INDEX, index);
		return;
	}

	const auto& TCoordinates = Coordinates();
	if (TCoordinates.find(index) == TCoordinates.end()) {
		StopSearching(ch, EPartyMatchMsg::PARTY_MATCH_FAIL_IMPOSSIBLE_MAP, index);
		return;
	}

	if (TCoordinates.at(index)->level > ch->GetLevel()) {
		StopSearching(ch, EPartyMatchMsg::PARTY_MATCH_FAIL_LEVEL, index);
		return;
	}

	const auto vnum = CheckItems(ch, index);
	if (vnum) {
		sys_err("PARTY_MATCH_FAIL_NO_ITEM 1");
		StopSearching(ch, EPartyMatchMsg::PARTY_MATCH_FAIL_NO_ITEM, vnum);
		return;
	}

	/*if (IsRestricted(ch->GetMapIndex())) {
		StopSearching(ch, EPartyMatchMsg::PARTY_MATCH_FAIL_IMPOSSIBLE_MAP, index);
		return;
	}*/

	if (ch->GetParty()) {
		StopSearching(ch, EPartyMatchMsg::PARTY_MATCH_FAIL, index);
		return;
	}

	SearchMap.emplace(index, ch);
	SendPacket(ch, EPacketGCPartyMatchSubHeader::PARTY_MATCH_SEARCH, EPartyMatchMsg::PARTY_MATCH_INFO, index);
	CheckPlayers(index);
}

decltype(CGroupMatchManager::SearchMap)::const_iterator CGroupMatchManager::IsSearching(LPCHARACTER ch)
{
	if (ch)
		for (auto it = SearchMap.begin(); it != SearchMap.end(); ++it)
			if (it->second == ch)
				return it;
	return SearchMap.end();
}

void CGroupMatchManager::StopSearching(LPCHARACTER ch, uint8_t MSG, int index)
{
	if (ch) {
		auto it = IsSearching(ch);
		if (it != SearchMap.end())
			SearchMap.erase(it);

		SendPacket(ch, EPacketGCPartyMatchSubHeader::PARTY_MATCH_CANCEL, MSG, index);
	}
}

bool CGroupMatchManager::CheckParty(int index)
{
	bool Check = true;

	auto range = SearchMap.equal_range(index);

	std::unordered_set<LPCHARACTER> junk;
	for (auto i = range.first; i != range.second; ++i) {
		auto ch = i->second;
		if (ch && ch->GetParty()) {
			junk.insert(ch);
			Check = false;
		}
	}

	for (auto v : junk)
		StopSearching(v, EPartyMatchMsg::PARTY_MATCH_FAIL, index);

	return Check;
}

bool CGroupMatchManager::CheckItems(int index)
{
	bool check = true;
	auto range = SearchMap.equal_range(index);

	std::map<LPCHARACTER, int> junk;
	for (auto i = range.first; i != range.second; ++i) {
		auto ch = i->second;
		if (ch) {
			const auto vnum = CheckItems(ch, index);
			if (vnum) {
				junk.emplace(ch, vnum);
				check = false;
			}
		}
	}

	for (auto v : junk)
	{
		sys_err("PARTY_MATCH_FAIL_NO_ITEM 2");
		StopSearching(v.first, EPartyMatchMsg::PARTY_MATCH_FAIL_NO_ITEM, v.second);
	}

	return check;
}

uint32_t CGroupMatchManager::CheckItems(LPCHARACTER ch, int index) const
{
	if (!ch)
		return 0;

	for (const auto& v : Coordinates().at(index)->items)
	{
		if (ch->CountSpecifyItem(v.first) < v.second)
			return v.first;
	}

	return 0;
}

void CGroupMatchManager::EraseItems(LPCHARACTER ch, int index)
{
	if (ch)
		for (const auto& v : Coordinates().at(index)->items)
			ch->RemoveSpecifyItem(v.first, v.second);
}

void CGroupMatchManager::CheckPlayers(int index)
{
	const auto& TCoordinates = Coordinates();
	if (TCoordinates.find(index) == TCoordinates.end())
		return;

	if (static_cast<int>(GetPlayerCount(index)) < TCoordinates.at(index)->player_count)
		return;

	if (!CheckParty(index))
		return;

	if (!CheckItems(index))
		return;

	auto range = SearchMap.equal_range(index);

	LPPARTY party = nullptr;
	for (auto i = range.first; i != range.second; ++i) {
		auto ch = i->second;
		if (!ch)
			continue;

		EraseItems(ch, index);

		if (!party)
			party = CPartyManager::Instance().CreateParty(ch);
		else {
			party->Join(ch->GetPlayerID());
			party->Link(ch);
		}

		party->SendPartyInfoAllToOne(party->GetLeaderCharacter());
		SendPacket(ch, EPacketGCPartyMatchSubHeader::PARTY_MATCH_CANCEL, EPartyMatchMsg::PARTY_MATCH_SUCCESS, index);
	}

	for (auto i = range.first; i != range.second; ++i) {
		auto ch = i->second;
		if (ch)
			ch->WarpSet(TCoordinates.at(index)->x * 100, TCoordinates.at(index)->y * 100);
	}

	SearchMap.erase(index);
}

static std::unordered_set<int> s_RestrictedMaps;
void CGroupMatchManager::AddRestricted(int mapIndex)
{
	if (s_RestrictedMaps.size() || mapIndex <= 0)
		s_RestrictedMaps.clear();

	s_RestrictedMaps.emplace(mapIndex);
}

bool CGroupMatchManager::IsRestricted(int mapIndex) const
{
	return s_RestrictedMaps.find(mapIndex) != s_RestrictedMaps.end();
}
#endif
