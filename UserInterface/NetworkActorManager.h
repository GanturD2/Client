#pragma once

#include "InstanceBase.h"

struct SNetworkActorData
{
	std::string m_stName;

	CAffectFlagContainer m_kAffectFlags;

	uint8_t m_bType;
	uint32_t m_dwVID;
	uint32_t m_dwStateFlags;
	uint32_t m_dwEmpireID;
	uint32_t m_dwRace;
	uint32_t m_dwMovSpd;
	uint32_t m_dwAtkSpd;
	FLOAT m_fRot;
	LONG m_lCurX;
	LONG m_lCurY;
	LONG m_lSrcX;
	LONG m_lSrcY;
	LONG m_lDstX;
	LONG m_lDstY;


	uint32_t m_dwServerSrcTime;
	uint32_t m_dwClientSrcTime;
	uint32_t m_dwDuration;

	uint32_t m_dwArmor;
	uint32_t m_dwWeapon;
	uint32_t m_dwHair;
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	uint32_t	m_dwAcce;
#endif
#ifdef ENABLE_PENDANT
	uint32_t m_dwPendant;
#endif
#ifdef ENABLE_REFINE_ELEMENT
	uint8_t 	m_dwElementsEffect;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	uint8_t	m_bLanguage;
#endif
	uint32_t m_dwOwnerVID;

	int16_t m_sAlignment;
	uint8_t m_byPKMode;
	uint32_t m_dwMountVnum;
#ifdef ENABLE_AURA_SYSTEM
	uint32_t	m_dwAura;
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	uint32_t	m_dwArrow;
#endif
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
	long	m_lGroupWeapon;
#endif
	uint32_t m_dwGuildID;
#ifdef ENABLE_SHOW_GUILD_LEADER
	uint8_t	m_dwNewIsGuildName;
#endif
	uint32_t m_dwLevel;
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	uint32_t m_dwTitle;
#endif
#ifdef WJ_SHOW_MOB_INFO
	uint32_t m_dwAIFlag;
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	uint32_t	m_dwcLevel;
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	uint32_t	m_dwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
#endif

	SNetworkActorData();

	void SetDstPosition(uint32_t dwServerTime, LONG lDstX, LONG lDstY, uint32_t dwDuration);
	void SetPosition(LONG lPosX, LONG lPosY);
	void UpdatePosition();

	// NETWORK_ACTOR_DATA_COPY
	SNetworkActorData(const SNetworkActorData & src);
	void operator=(const SNetworkActorData & src);
	void __copy__(const SNetworkActorData & src);
	// END_OF_NETWORK_ACTOR_DATA_COPY
};

struct SNetworkMoveActorData
{
	uint32_t m_dwVID;
	uint32_t m_dwTime;
	LONG m_lPosX;
	LONG m_lPosY;
	float m_fRot;
	uint32_t m_dwFunc;
	uint32_t m_dwArg;
	uint32_t m_dwDuration;

	SNetworkMoveActorData()
	{
		m_dwVID = 0;
		m_dwTime = 0;
		m_fRot = 0.0f;
		m_lPosX = 0;
		m_lPosY = 0;
		m_dwFunc = 0;
		m_dwArg = 0;
		m_dwDuration = 0;
	}
};

struct SNetworkUpdateActorData
{
	uint32_t m_dwVID;
	uint32_t m_dwGuildID;
#ifdef ENABLE_SHOW_GUILD_LEADER
	uint8_t m_dwNewIsGuildName;
#endif
	uint32_t m_dwArmor;
	uint32_t m_dwWeapon;
	uint32_t m_dwHair;
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	uint32_t m_dwAcce;
#endif
#ifdef ENABLE_PENDANT
	uint32_t m_dwPendant;
#endif
#ifdef ENABLE_REFINE_ELEMENT
	uint8_t 	m_dwElementsEffect;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	uint8_t m_bLanguage;
#endif
	uint32_t m_dwMovSpd;
	uint32_t m_dwAtkSpd;
	int16_t m_sAlignment;
#ifdef WJ_SHOW_MOB_INFO
	uint32_t m_dwLevel;
#endif
	uint8_t m_byPKMode;
	uint32_t m_dwMountVnum;
#ifdef ENABLE_AURA_SYSTEM
	uint32_t m_dwAura;
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	uint32_t m_dwArrow;
#endif
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
	long m_lGroupWeapon;
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	uint32_t m_dwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	uint32_t m_dwTitle;
#endif
	uint32_t m_dwStateFlags; // 본래 Create 때만 쓰이는 변수임
	CAffectFlagContainer m_kAffectFlags;

	SNetworkUpdateActorData()
	{
		m_dwGuildID = 0;
#ifdef ENABLE_SHOW_GUILD_LEADER
		m_dwNewIsGuildName=0;
#endif
		m_dwVID = 0;
		m_dwArmor = 0;
		m_dwWeapon = 0;
		m_dwHair = 0;
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		m_dwAcce = 0;
#endif
#ifdef ENABLE_PENDANT
		m_dwPendant = 0;
#endif
#ifdef ENABLE_REFINE_ELEMENT
		m_dwElementsEffect = 0;
#endif
		m_dwMovSpd = 0;
		m_dwAtkSpd = 0;
		m_sAlignment = 0;
#ifdef WJ_SHOW_MOB_INFO
		m_dwLevel=0;
#endif
		m_byPKMode = 0;
		m_dwMountVnum = 0;
#ifdef ENABLE_AURA_SYSTEM
		m_dwAura = 0;
#endif
#ifdef ENABLE_QUIVER_SYSTEM
		m_dwArrow = 0;
#endif
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
		m_lGroupWeapon = 0;
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
		memset(m_dwSkillColor, 0, sizeof(m_dwSkillColor));
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		m_dwTitle = 0;
#endif
		m_dwStateFlags = 0;
		m_kAffectFlags.Clear();
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		m_bLanguage = LANGUAGE_NONE;
#endif
	}
};

class CPythonCharacterManager;

class CNetworkActorManager : public CReferenceObject
{
public:
	CNetworkActorManager();
	~CNetworkActorManager();
	CLASS_DELETE_COPYMOVE(CNetworkActorManager);

	void Destroy();

	void SetMainActorVID(uint32_t dwVID);

	void RemoveActor(uint32_t dwVID);
	void AppendActor(const SNetworkActorData & c_rkNetActorData);
	void UpdateActor(const SNetworkUpdateActorData & c_rkNetUpdateActorData);
	void MoveActor(const SNetworkMoveActorData & c_rkNetMoveActorData);

	void SyncActor(uint32_t dwVID, LONG lPosX, LONG lPosY);
	void SetActorOwner(uint32_t dwOwnerVID, uint32_t dwVictimVID);

	void Update();

protected:
	void __OLD_Update();

	void __UpdateMainActor();

	bool __IsVisiblePos(LONG lPosX, LONG lPosY) const;
	bool __IsVisibleActor(const SNetworkActorData & c_rkNetActorData) const;
	bool __IsMainActorVID(uint32_t dwVID) const;

	void __RemoveAllGroundItems() const;
	void __RemoveAllActors();
	void __RemoveDynamicActors();
	void __RemoveCharacterManagerActor(const SNetworkActorData & rkNetActorData) const;

	SNetworkActorData * __FindActorData(uint32_t dwVID);

	CInstanceBase * __AppendCharacterManagerActor(const SNetworkActorData & rkNetActorData);
	CInstanceBase * __FindActor(SNetworkActorData & rkNetActorData);
	CInstanceBase * __FindActor(SNetworkActorData & rkNetActorData, LONG lDstX, LONG lDstY);

	CPythonCharacterManager & __GetCharacterManager() const;

protected:
	uint32_t m_dwMainVID;

	LONG m_lMainPosX;
	LONG m_lMainPosY;

	std::map<uint32_t, SNetworkActorData> m_kNetActorDict;
};
