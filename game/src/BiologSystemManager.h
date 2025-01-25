#pragma once
#include "stdafx.h"

#ifdef ENABLE_BIOLOG_SYSTEM
#include "../../common/tables.h"
#include "affect.h"

class CBiologSystem
{
public:
	CBiologSystem(LPCHARACTER m_pkChar);
	~CBiologSystem();

	// update functions
	void ResetMission();

	// general functions
	void SendBiologInformation(bool bUpdate = false);
	void SendBiologItem();

	// incoming packet functions
	int RecvClientPacket(uint8_t bSubHeader, const char* c_pData, size_t uiBytes);

	// outgoing packet functions
	void SendClientPacket(LPDESC pkDesc, uint8_t bSubHeader, const void* c_pvData, size_t iSize);
	void SendClientPacket(uint32_t dwPID, uint8_t bSubHeader, const void* c_pvData, size_t iSize);

private:
	LPCHARACTER m_pkChar;
};

class CBiologSystemManager : public singleton<CBiologSystemManager>
{
public:
	typedef std::map<uint8_t, TBiologMissionsProto> TMissionProtoMap;
	typedef std::map<uint8_t, TBiologRewardsProto> TRewardProtoMap;

public:
	CBiologSystemManager();
	~CBiologSystemManager();

	void InitializeMissions(TBiologMissionsProto* pData, size_t size);
	TBiologMissionsProto* GetMission(uint8_t bMission);

	void InitializeRewards(TBiologRewardsProto* pData, size_t size);
	TBiologRewardsProto* GetReward(uint8_t bMission);

	friend void CBiologSystem::SendBiologItem();

private:
	TMissionProtoMap m_mapMission_Proto;
	TRewardProtoMap m_mapReward_Proto;
};
#endif
