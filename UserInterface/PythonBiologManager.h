#pragma once
#include "StdAfx.h"

#ifdef ENABLE_BIOLOG_SYSTEM
#include "Packet.h"

class CPythonBiologManager : public CSingleton<CPythonBiologManager>
{
public:
	CPythonBiologManager();
	~CPythonBiologManager();

	void Initialize();

	// biolog info
	void _LoadBiologInformation(const TPacketGCBiologManagerInfo* pBiologInfo);

	// getters
	uint8_t Biolog_GetCurrentLevel() const noexcept { return m_BiologInfo.bRequiredLevel; };
	uint32_t Biolog_GetCurrentItem() const noexcept { return m_BiologInfo.iRequiredItem; };
	uint16_t Biolog_GetCurrentGivenItems() const noexcept { return m_BiologInfo.wGivenItems; };
	uint16_t Biolog_GetCurrentRequiredCount() const noexcept { return m_BiologInfo.wRequiredItemCount; };
	time_t Biolog_GetCurrentGlobalCooldown() const noexcept { return m_BiologInfo.iGlobalCooldown; };
	time_t Biolog_GetCurrentCooldown() const noexcept { return m_BiologInfo.iCooldown; };
	bool Biolog_GetCurrentCooldownReminder() const noexcept { return m_BiologInfo.iCooldownReminder; }
	uint16_t Biolog_GetCurrentRewardType(int index) const noexcept { return m_BiologInfo.wApplyType[index]; }; //@fixme436
	long Biolog_GetCurrentRewardValue(int index) const noexcept { return m_BiologInfo.lApplyValue[index]; }
	uint32_t Biolog_GetCurrentRewardItem() const noexcept { return m_BiologInfo.dRewardItem; }
	uint16_t Biolog_GetCurrentRewardItemCount() const noexcept { return m_BiologInfo.wRewardItemCount; }
	bool Biolog_HasSubMission() const noexcept { return m_BiologInfo.bSubMission; }
	uint32_t Biolog_GetNpcVnum() const noexcept { return m_BiologInfo.dwNpcVnum; }
	uint32_t Biolog_GetCurrentSubItem() const noexcept { return m_BiologInfo.dwRequiredSubItem; };

private:
	TPacketGCBiologManagerInfo m_BiologInfo;
};

extern void initBiologManager() noexcept;
#endif
