#include "stdafx.h"
#include "constants.h"
#include "pvp.h"
#include "crc32.h"
#include "packet.h"
#include "desc.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "config.h"
#include "sectree_manager.h"
#include "buffer_manager.h"
#include "locale_service.h"

using namespace std;

CPVP::CPVP(uint32_t dwPID1, uint32_t dwPID2)
{
	if (dwPID1 > dwPID2)
	{
		m_players[0].dwPID = dwPID1;
		m_players[1].dwPID = dwPID2;
		m_players[0].bAgree = true;
	}
	else
	{
		m_players[0].dwPID = dwPID2;
		m_players[1].dwPID = dwPID1;
		m_players[1].bAgree = true;
	}

	uint32_t adwID[2];
	adwID[0] = m_players[0].dwPID;
	adwID[1] = m_players[1].dwPID;
	m_dwCRC = GetFastHash((const char *) &adwID, 8);
	m_bRevenge = false;

	SetLastFightTime();
}

CPVP::CPVP(CPVP & k)
{
	m_players[0] = k.m_players[0];
	m_players[1] = k.m_players[1];

	m_dwCRC = k.m_dwCRC;
	m_bRevenge = k.m_bRevenge;

	SetLastFightTime();
}

CPVP::~CPVP()
{
}

void CPVP::Packet(bool bDelete)
{
	if (!m_players[0].dwVID || !m_players[1].dwVID)
	{
		if (bDelete)
			sys_err("null vid when removing %u %u", m_players[0].dwVID, m_players[0].dwVID);

		return;
	}

	TPacketGCPVP pack{};

	pack.bHeader = HEADER_GC_PVP;

	if (bDelete)
	{
		pack.bMode = PVP_MODE_NONE;
		pack.dwVIDSrc = m_players[0].dwVID;
		pack.dwVIDDst = m_players[1].dwVID;
	}
	else if (IsFight())
	{
		pack.bMode = PVP_MODE_FIGHT;
		pack.dwVIDSrc = m_players[0].dwVID;
		pack.dwVIDDst = m_players[1].dwVID;
	}
	else
	{
		pack.bMode = m_bRevenge ? PVP_MODE_REVENGE : PVP_MODE_AGREE;

		if (m_players[0].bAgree)
		{
			pack.dwVIDSrc = m_players[0].dwVID;
			pack.dwVIDDst = m_players[1].dwVID;
		}
		else
		{
			pack.dwVIDSrc = m_players[1].dwVID;
			pack.dwVIDDst = m_players[0].dwVID;
		}
	}

	const DESC_MANAGER::DESC_SET & c_rSet = DESC_MANAGER::Instance().GetClientSet();
	DESC_MANAGER::DESC_SET::const_iterator it = c_rSet.begin();

	while (it != c_rSet.end())
	{
		LPDESC d = *it++;

		if (d->IsPhase(PHASE_GAME) || d->IsPhase(PHASE_DEAD))
			d->Packet(&pack, sizeof(pack));
	}
}

bool CPVP::Agree(uint32_t dwPID)
{
	m_players[m_players[0].dwPID != dwPID ? 1 : 0].bAgree = true;

	if (IsFight())
	{
		Packet();
		return true;
	}

	return false;
}

bool CPVP::IsFight()
{
	return (m_players[0].bAgree == m_players[1].bAgree) && m_players[0].bAgree;
}

void CPVP::Win(uint32_t dwPID)
{
	int iSlot = m_players[0].dwPID != dwPID ? 1 : 0;

	m_bRevenge = true;

	m_players[iSlot].bAgree = true; // 자동으로 동의
	m_players[!iSlot].bCanRevenge = true;
	m_players[!iSlot].bAgree = false;

	Packet();
}

bool CPVP::CanRevenge(uint32_t dwPID)
{
	return m_players[m_players[0].dwPID != dwPID ? 1 : 0].bCanRevenge;
}

void CPVP::SetVID(uint32_t dwPID, uint32_t dwVID)
{
	if (m_players[0].dwPID == dwPID)
		m_players[0].dwVID = dwVID;
	else
		m_players[1].dwVID = dwVID;
}

void CPVP::SetLastFightTime()
{
	m_dwLastFightTime = get_dword_time();
}

uint32_t CPVP::GetLastFightTime()
{
	return m_dwLastFightTime;
}

CPVPManager::CPVPManager()
{
}

CPVPManager::~CPVPManager()
{
}

void CPVPManager::Insert(LPCHARACTER pkChr, LPCHARACTER pkVictim)
{
	if (pkChr->IsDead() || pkVictim->IsDead())
		return;

	CPVP kPVP(pkChr->GetPlayerID(), pkVictim->GetPlayerID());

	CPVP * pkPVP;

	if ((pkPVP = Find(kPVP.m_dwCRC)))
	{
		// 복수할 수 있으면 바로 싸움!
		if (pkPVP->Agree(pkChr->GetPlayerID()))
		{
			pkVictim->ChatPacket(CHAT_TYPE_INFO, "[LS;492;%s]", pkChr->GetName());
			pkChr->ChatPacket(CHAT_TYPE_INFO, "[LS;492;%s]", pkVictim->GetName());
		}
		return;
	}

	pkPVP = M2_NEW CPVP(kPVP);

	pkPVP->SetVID(pkChr->GetPlayerID(), pkChr->GetVID());
	pkPVP->SetVID(pkVictim->GetPlayerID(), pkVictim->GetVID());

	m_map_pkPVP.insert(map<uint32_t, CPVP *>::value_type(pkPVP->m_dwCRC, pkPVP));

	m_map_pkPVPSetByID[pkChr->GetPlayerID()].insert(pkPVP);
	m_map_pkPVPSetByID[pkVictim->GetPlayerID()].insert(pkPVP);

	pkPVP->Packet();

	char msg[CHAT_MAX_LEN + 1];
	snprintf(msg, sizeof(msg), LC_TEXT("%s님이 대결신청을 했습니다. 승낙하려면 대결동의를 하세요."), pkChr->GetName());
	pkVictim->ChatPacket(CHAT_TYPE_INFO, msg);
	pkChr->ChatPacket(CHAT_TYPE_INFO, "[LS;514;%s]", pkVictim->GetName());

	// NOTIFY_PVP_MESSAGE
	LPDESC pkVictimDesc = pkVictim->GetDesc();
	if (pkVictimDesc)
	{
		TPacketGCWhisper pack{};

		int len = MIN(CHAT_MAX_LEN, strlen(msg) + 1);

		pack.bHeader = HEADER_GC_WHISPER;
		pack.wSize = sizeof(TPacketGCWhisper) + len;
		pack.bType = WHISPER_TYPE_SYSTEM;
		strlcpy(pack.szNameFrom, pkChr->GetName(), sizeof(pack.szNameFrom));

		TEMP_BUFFER buf;

		buf.write(&pack, sizeof(TPacketGCWhisper));
		buf.write(msg, len);

		pkVictimDesc->Packet(buf.read_peek(), buf.size());
	}
	// END_OF_NOTIFY_PVP_MESSAGE
}

#ifdef ENABLE_NEWSTUFF
bool CPVPManager::IsFighting(LPCHARACTER pkChr)
{
	if (!pkChr)
		return false;

	return IsFighting(pkChr->GetPlayerID());
}

bool CPVPManager::IsFighting(uint32_t dwPID)
{
	CPVPSetMap::iterator it = m_map_pkPVPSetByID.find(dwPID);

	if (it == m_map_pkPVPSetByID.end())
		return false;

	std::unordered_set<CPVP*>::iterator it2 = it->second.begin();

	while (it2 != it->second.end())
	{
		CPVP * pkPVP = *it2++;
		if (pkPVP->IsFight())
			return true;
	}

	return false;
}
#endif

void CPVPManager::ConnectEx(LPCHARACTER pkChr, bool bDisconnect)
{
	CPVPSetMap::iterator it = m_map_pkPVPSetByID.find(pkChr->GetPlayerID());

	if (it == m_map_pkPVPSetByID.end())
		return;

	uint32_t dwVID = bDisconnect ? 0 : pkChr->GetVID();

	std::unordered_set<CPVP*>::iterator it2 = it->second.begin();

	while (it2 != it->second.end())
	{
		CPVP * pkPVP = *it2++;
		pkPVP->SetVID(pkChr->GetPlayerID(), dwVID);
	}
}

void CPVPManager::Connect(LPCHARACTER pkChr)
{
	ConnectEx(pkChr, false);
}

void CPVPManager::Disconnect(LPCHARACTER pkChr)
{
	//ConnectEx(pkChr, true);
}

void CPVPManager::GiveUp(LPCHARACTER pkChr, uint32_t dwKillerPID) // This method is calling from no where yet.
{
	CPVPSetMap::iterator it = m_map_pkPVPSetByID.find(pkChr->GetPlayerID());

	if (it == m_map_pkPVPSetByID.end())
		return;

	sys_log(1, "PVPManager::Dead %d", pkChr->GetPlayerID());
	std::unordered_set<CPVP*>::iterator it2 = it->second.begin();

	while (it2 != it->second.end())
	{
		CPVP * pkPVP = *it2++;

		uint32_t dwCompanionPID;

		if (pkPVP->m_players[0].dwPID == pkChr->GetPlayerID())
			dwCompanionPID = pkPVP->m_players[1].dwPID;
		else
			dwCompanionPID = pkPVP->m_players[0].dwPID;

		if (dwCompanionPID != dwKillerPID)
			continue;

		pkPVP->SetVID(pkChr->GetPlayerID(), 0);

		m_map_pkPVPSetByID.erase(dwCompanionPID);

		it->second.erase(pkPVP);

		if (it->second.empty())
			m_map_pkPVPSetByID.erase(it);

		m_map_pkPVP.erase(pkPVP->m_dwCRC);

		pkPVP->Packet(true);
		M2_DELETE(pkPVP);
		break;
	}
}

bool CPVPManager::IsDuelingInstance(LPCHARACTER pkChr)	//custom000
{
	CPVPSetMap::iterator it = m_map_pkPVPSetByID.find(pkChr->GetPlayerID());

	if (it == m_map_pkPVPSetByID.end())
		return false;

	return true;
}

// 리턴값: 0 = PK, 1 = PVP
// PVP를 리턴하면 경험치나 아이템을 떨구고 PK면 떨구지 않는다.
bool CPVPManager::Dead(LPCHARACTER pkChr, uint32_t dwKillerPID)
{
	CPVPSetMap::iterator it = m_map_pkPVPSetByID.find(pkChr->GetPlayerID());

	if (it == m_map_pkPVPSetByID.end())
		return false;

	bool found = false;

	sys_log(1, "PVPManager::Dead %d", pkChr->GetPlayerID());
	std::unordered_set<CPVP*>::iterator it2 = it->second.begin();

	while (it2 != it->second.end())
	{
		CPVP * pkPVP = *it2++;

		uint32_t dwCompanionPID;

		if (pkPVP->m_players[0].dwPID == pkChr->GetPlayerID())
			dwCompanionPID = pkPVP->m_players[1].dwPID;
		else
			dwCompanionPID = pkPVP->m_players[0].dwPID;

		if (dwCompanionPID == dwKillerPID)
		{
			if (pkPVP->IsFight())
			{
				pkPVP->SetLastFightTime();
				pkPVP->Win(dwKillerPID);
				found = true;
				break;
			}
			else if (get_dword_time() - pkPVP->GetLastFightTime() <= 15000)
			{
				found = true;
				break;
			}
		}
	}

	return found;
}

bool CPVPManager::CanAttack(LPCHARACTER pkChr, LPCHARACTER pkVictim)
{
	switch (pkVictim->GetCharType())
	{
		case CHAR_TYPE_NPC:
		case CHAR_TYPE_WARP:
		case CHAR_TYPE_GOTO:
#ifdef ENABLE_PROTO_RENEWAL
		case CHAR_TYPE_PET:	//ENABLE_GROWTH_PET_SYSTEM
		case CHAR_TYPE_PET_PAY:
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		case CHAR_TYPE_SHOP:
#endif
			return false;
	}

	if (pkChr->GetWaitHackCounter() == 1) {	//@fixme503
		pkChr->ClearWaitHackCounter();
		return false;
	}

	if (pkChr == pkVictim)
		return false;

	if (pkVictim->IsNPC() && pkChr->IsNPC() && !pkChr->IsGuardNPC())
		return false;

#ifdef ENABLE_AFK_MODE_SYSTEM
	if (pkVictim->IsPC() && pkVictim->IsAway())
		return false;
#endif

	if(pkChr->IsHorseRiding()) {
		if (pkChr->GetHorseLevel() > 0 && 1 == pkChr->GetHorseGrade())
			return false;
	}
	else {
		eMountType eIsMount = GetMountLevelByVnum(pkChr->GetMountVnum(), false);
		switch (eIsMount)
		{
			case MOUNT_TYPE_NONE:
			case MOUNT_TYPE_COMBAT:
			case MOUNT_TYPE_MILITARY:
				break;

			case MOUNT_TYPE_NORMAL:
				return false;
				break;

			default:
				if (test_server)
					sys_log(0, "CanUseSkill: Mount can't attack. vnum(%u) type(%d)", pkChr->GetMountVnum(), static_cast<int>(eIsMount));
				return false;
				break;
		}
	}

	if (pkVictim->IsNPC() || pkChr->IsNPC()) {
		return true;
	}

	if (pkVictim->IsObserverMode() || pkChr->IsObserverMode())
		return false;

	{
		uint8_t bMapEmpire = SECTREE_MANAGER::Instance().GetEmpireFromMapIndex(pkChr->GetMapIndex());

		if ( ((pkChr->GetPKMode() == PK_MODE_PROTECT) && (pkChr->GetEmpire() == bMapEmpire)) ||
				((pkVictim->GetPKMode() == PK_MODE_PROTECT) && (pkVictim->GetEmpire() == bMapEmpire)) )
		{
			return false;
		}
	}

	if (pkChr->GetEmpire() != pkVictim->GetEmpire())
	{
		// @warme005
		{
			if ( pkChr->GetPKMode() == PK_MODE_PROTECT || pkVictim->GetPKMode() == PK_MODE_PROTECT )
			{
				return false;
			}
		}

		return true;
	}

	bool beKillerMode = false;

	if (pkVictim->GetParty() && pkVictim->GetParty() == pkChr->GetParty())
	{
		return false;
		// Cannot attack same party on any pvp model
	}
	else
	{
		if (pkVictim->IsKillerMode())
		{
			return true;
		}

		if (pkChr->GetAlignment() < 0 && pkVictim->GetAlignment() >= 0)
		{
		    if (g_protectNormalPlayer)
		    {
			// 범법자는 평화모드인 착한사람을 공격할 수 없다.
			if (PK_MODE_PEACE == pkVictim->GetPKMode())
			    return false;
		    }
		}


		switch (pkChr->GetPKMode())
		{
			case PK_MODE_PEACE:
			case PK_MODE_REVENGE:
				// Cannot attack same guild
				if (pkVictim->GetGuild() && pkVictim->GetGuild() == pkChr->GetGuild())
					break;

				if (pkChr->GetPKMode() == PK_MODE_REVENGE)
				{
					if (pkChr->GetAlignment() < 0 && pkVictim->GetAlignment() >= 0)
					{
						pkChr->SetKillerMode(true);
						return true;
					}
					else if (pkChr->GetAlignment() >= 0 && pkVictim->GetAlignment() < 0)
						return true;
				}
				break;

			case PK_MODE_GUILD:
				// Same implementation from PK_MODE_FREE except for attacking same guild
				if (!pkChr->GetGuild() || (pkVictim->GetGuild() != pkChr->GetGuild()))
				{
					if (pkVictim->GetAlignment() >= 0)
						pkChr->SetKillerMode(true);
					else if (pkChr->GetAlignment() < 0 && pkVictim->GetAlignment() < 0)
						pkChr->SetKillerMode(true);

					return true;
				}
				break;

			case PK_MODE_FREE:
				if (pkVictim->GetAlignment() >= 0)
					pkChr->SetKillerMode(true);
				else if (pkChr->GetAlignment() < 0 && pkVictim->GetAlignment() < 0)
					pkChr->SetKillerMode(true);
				return true;
				break;

#ifdef ENABLE_BATTLE_FIELD
			case PK_MODE_BATTLE:
				pkChr->SetKillerMode(true);
				return true;
				break;
#endif
		}
	}

	CPVP kPVP(pkChr->GetPlayerID(), pkVictim->GetPlayerID());
	CPVP * pkPVP = Find(kPVP.m_dwCRC);

	if (!pkPVP || !pkPVP->IsFight())
	{
		if (beKillerMode)
			pkChr->SetKillerMode(true);

		return (beKillerMode);
	}

	pkPVP->SetLastFightTime();
	return true;
}

CPVP * CPVPManager::Find(uint32_t dwCRC)
{
	map<uint32_t, CPVP *>::iterator it = m_map_pkPVP.find(dwCRC);

	if (it == m_map_pkPVP.end())
		return nullptr;

	return it->second;
}

void CPVPManager::Delete(CPVP * pkPVP)
{
	map<uint32_t, CPVP *>::iterator it = m_map_pkPVP.find(pkPVP->m_dwCRC);

	if (it == m_map_pkPVP.end())
		return;

	m_map_pkPVP.erase(it);
	m_map_pkPVPSetByID[pkPVP->m_players[0].dwPID].erase(pkPVP);
	m_map_pkPVPSetByID[pkPVP->m_players[1].dwPID].erase(pkPVP);

	M2_DELETE(pkPVP);
}

void CPVPManager::SendList(LPDESC d)
{
	map<uint32_t, CPVP *>::iterator it = m_map_pkPVP.begin();

	uint32_t dwVID = d->GetCharacter()->GetVID();

	TPacketGCPVP pack{};

	pack.bHeader = HEADER_GC_PVP;

	while (it != m_map_pkPVP.end())
	{
		CPVP * pkPVP = (it++)->second;

		if (!pkPVP->m_players[0].dwVID || !pkPVP->m_players[1].dwVID)
			continue;

		// VID가 둘다 있을 경우에만 보낸다.
		if (pkPVP->IsFight())
		{
			pack.bMode = PVP_MODE_FIGHT;
			pack.dwVIDSrc = pkPVP->m_players[0].dwVID;
			pack.dwVIDDst = pkPVP->m_players[1].dwVID;
		}
		else
		{
			pack.bMode = pkPVP->m_bRevenge ? PVP_MODE_REVENGE : PVP_MODE_AGREE;

			if (pkPVP->m_players[0].bAgree)
			{
				pack.dwVIDSrc = pkPVP->m_players[0].dwVID;
				pack.dwVIDDst = pkPVP->m_players[1].dwVID;
			}
			else
			{
				pack.dwVIDSrc = pkPVP->m_players[1].dwVID;
				pack.dwVIDDst = pkPVP->m_players[0].dwVID;
			}
		}

		d->Packet(&pack, sizeof(pack));
		sys_log(1, "PVPManager::SendList %d %d", pack.dwVIDSrc, pack.dwVIDDst);

		if (pkPVP->m_players[0].dwVID == dwVID)
		{
			LPCHARACTER ch = CHARACTER_MANAGER::Instance().Find(pkPVP->m_players[1].dwVID);
			if (ch && ch->GetDesc())
			{
				LPDESC d = ch->GetDesc();
				d->Packet(&pack, sizeof(pack));
			}
		}
		else if (pkPVP->m_players[1].dwVID == dwVID)
		{
			LPCHARACTER ch = CHARACTER_MANAGER::Instance().Find(pkPVP->m_players[0].dwVID);
			if (ch && ch->GetDesc())
			{
				LPDESC d = ch->GetDesc();
				d->Packet(&pack, sizeof(pack));
			}
		}
	}
}

void CPVPManager::Process()
{
	map<uint32_t, CPVP *>::iterator it = m_map_pkPVP.begin();

	while (it != m_map_pkPVP.end())
	{
		CPVP * pvp = (it++)->second;

		if (get_dword_time() - pvp->GetLastFightTime() > 600000) // 10분 이상 싸움이 없었으면
		{
			pvp->Packet(true);
			Delete(pvp);
		}
	}
}

#ifdef ENABLE_FIGHT_RECORD
std::unordered_set<CPVP*> CPVPManager::GetMyDuels(LPCHARACTER pkChr)
{
	std::unordered_set<CPVP*> set;

	auto it = m_map_pkPVPSetByID.find(pkChr->GetPlayerID());
	if (it != m_map_pkPVPSetByID.end())
		set = it->second;

	return set;
}

/************************************************************************/
/*							LOGGER                                      */
/************************************************************************/

PVP::CLogger::CLogger(uint32_t player1, uint32_t player2)
	: m_winner(0)
{
	m_memberPIDs[0] = player1;
	m_memberPIDs[1] = player2;
	m_dwStartTime = PVP::getMillis();

	//Get character pointers and collect a bit of information!
	LPCHARACTER ch;

	ch = CHARACTER_MANAGER::Instance().FindByPID(player1);
	if (ch)
		CollectStartInformation(ch, 1);
	else
		sys_err("CPvPLogger could not find player1 (%lu)", player1);

	ch = CHARACTER_MANAGER::Instance().FindByPID(player2);
	if (ch)
		CollectStartInformation(ch, 2);
	else
		sys_err("CPvPLogger could not find player2 (%lu)", player2);
}

void PVP::CLogger::CollectStartInformation(const LPCHARACTER ch, uint8_t player)
{
	if (player > 2)
		return;

	TDuelStartInfo startInfo;
	startInfo.level = ch->GetLevel();
	startInfo.name = ch->GetName();
	startInfo.job = ch->GetJob();
	startInfo.pid = ch->GetPlayerID();
	for (size_t i = 0; i < WEAR_MAX; ++i)
	{
		LPITEM item = ch->GetWear(i);
		if (!item)
			continue;

		startInfo.eq[i] = item->GetVnum();
	}

	for (size_t i = 0; i < POINT_MAX_NUM; ++i)
	{
		startInfo.points[i] = ch->GetPoint(i);
	}

	for (size_t i = 0; i < SKILL_MAX_NUM; ++i)
	{
		startInfo.skills[i] = ch->GetSkillLevel(i);
	}

	//Fill in the array pos that hasn't been already
	m_startInfo[player - 1] = startInfo;
}

void PVP::CLogger::RecordEquipmentChange(const LPITEM item)
{
	m_events.emplace_back(std::make_tuple(getMillis(), item->GetOwner()->GetPlayerID(), EQUIPMENT_CHANGE, item->GetVnum()));
}

void PVP::CLogger::RecordItemUse(const LPITEM item)
{
	m_events.emplace_back(std::make_tuple(getMillis(), item->GetOwner()->GetPlayerID(), ITEM_USE, item->GetVnum()));
}

void PVP::CLogger::RecordHit(LPCHARACTER victim, int attackType, uint32_t infoBitflag, int baseDmg, int finalDmg, int skillVnum)
{
	if (!victim->IsPC())
		return;

	m_events.emplace_back(std::make_tuple(getMillis(), 0, HIT, m_hits.size()));

	THitInfo info;
	info.victimPID = victim->GetPlayerID();
	info.hp = victim->GetHP();
	info.sp = victim->GetSP();
	info.stamina = victim->GetStamina();
	info.baseDamage = baseDmg;
	info.processedDamage = finalDmg;
	info.attackType = attackType;
	info.dmgProcessingBitflag = infoBitflag;
	info.skillVnum = skillVnum;

	m_hits.emplace_back(info);
}

void PVP::CLogger::Win(uint32_t pid)
{
	m_winner = pid;
	Save();
}

void PVP::CLogger::Save()
{
	std::chrono::milliseconds now = getMillis();
	if ((m_dwStartTime - now).count() / 1000 < 20 && (m_startInfo[0].level < 50 || m_startInfo[1].level < 50))
		return;

	std::ostringstream log;
	//First log initial state
	log << "{";
	for (int i = 0; i < 2; ++i)
	{
		TDuelStartInfo startInfo = m_startInfo[i];

		log << "\"" << i << "\":" << "{\"name\":\"" << startInfo.name << "\",\"lv\":" << startInfo.level;
		log << ",\"pid\":" << startInfo.pid << ",\"race\":" << startInfo.job;
		log << ",\"points\":{";
		for (size_t k = 0, applied = 0; k < POINT_MAX_NUM; ++k)
		{
			if (startInfo.points[k] == 0)
				continue;

			if (applied > 0)
				log << ",";

			log << "\"" << k << "\":" << startInfo.points[k];
			++applied;
		}
		log << "},\"skills\":{";
		for (size_t k = 0, applied = 0; k < SKILL_MAX_NUM; ++k)
		{
			if (startInfo.skills[k] == 0)
				continue;

			if (applied > 0)
				log << ",";

			log << "\"" << k << "\":" << startInfo.skills[k];
			++applied;
		}
		log << "},\"eq\":{";
		for (size_t k = 0, applied = 0; k < WEAR_MAX; ++k)
		{
			if (startInfo.eq[k] == 0)
				continue;

			if (applied > 0)
				log << ",";

			log << "\"" << k << "\":" << startInfo.eq[k];
			++applied;
		}
		log << "}},";
	}

	log << "\"winner\":" << m_winner;
	log << ",\"hits\":" << m_hits.size();
	log << ",\"events\":" << m_events.size();

	if (m_winner > 0) {
		log << ",\"duration\":" << (now - m_dwStartTime).count();
	}

	log << "}";
	//End json (too repetitive for the events/hits)

	log << "|";

	bool separate = false;
	for (const auto ev : m_events)
	{
		std::chrono::milliseconds millis = std::get<0>(ev);
		uint32_t pid = std::get<1>(ev);
		PVP::EventType eventType = std::get<2>(ev);
		uint32_t value = std::get<3>(ev);

		if (separate)
			log << ",";
		else
			separate = true;

		log << "[" << (millis - m_dwStartTime).count() << ",";
		if (pid) //Not relevant for hits (already stored as hit information)
			log << pid << ",";

		log << eventType;

		if (eventType != HIT) //We can already know the hit ID through id increases
			log << "," << value;

		log << "]";
	}

	log << "|";
	int hitID = 0;
	THitInfo previousHit;

	//Sort of header for the data that comes afterwards
	log << "[victim,hp,sp,stamina,type,baseDmg,dmgBitflag,finalDmg],";

	separate = false;
	for (const THitInfo hit : m_hits)
	{
		if (separate)
		{
			//We already have a hit that we can compare with
			log << ",[";
			if (previousHit.victimPID != hit.victimPID)
				log << hit.victimPID;

			log << ",";

			if (previousHit.hp != hit.hp)
				log << hit.hp;

			log << ",";

			if (previousHit.sp != hit.sp)
				log << hit.sp;

			log << ",";

			if (previousHit.stamina != hit.stamina)
				log << hit.stamina;

			log << ",";

			if (previousHit.attackType != hit.attackType)
				log << hit.attackType;

			log << ",";

			if (previousHit.baseDamage != hit.baseDamage)
				log << hit.baseDamage;

			log << ",";

			if (previousHit.dmgProcessingBitflag != hit.dmgProcessingBitflag)
				log << hit.dmgProcessingBitflag;

			log << ",";

			if (previousHit.processedDamage != hit.processedDamage)
				log << hit.processedDamage;

			log << ",";

			if (previousHit.skillVnum != hit.skillVnum)
				log << hit.skillVnum;
		}
		else
		{
			log << "[" << hit.victimPID;
			log << "," << hit.hp << "," << hit.sp << "," << hit.stamina << "," << hit.attackType;
			log << "," << hit.baseDamage << "," << hit.dmgProcessingBitflag << "," << hit.processedDamage << "," << hit.skillVnum;
			separate = true;
		}

		log << "]";
		previousHit = hit;
	}

	log << "\n";

	std::ofstream f("pvp_log", fstream::out | fstream::app);
	if (!f) {
		sys_err("Could not open pvp_log. Trying to log: %s", log.str().c_str());
		return;
	}

	f.write(log.str().c_str(), log.str().length());
	f.close();
}
#endif
