#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "NetworkActorManager.h"
#include "PythonBackground.h"

#include "PythonApplication.h"
#include "AbstractPlayer.h"
#include "../GameLib/ActorInstance.h"

void CPythonNetworkStream::__GlobalPositionToLocalPosition(LONG & rGlobalX, LONG & rGlobalY) const
{
	CPythonBackground & rkBgMgr = CPythonBackground::Instance();
	rkBgMgr.GlobalPositionToLocalPosition(rGlobalX, rGlobalY);
}

void CPythonNetworkStream::__LocalPositionToGlobalPosition(LONG & rLocalX, LONG & rLocalY) const
{
	CPythonBackground & rkBgMgr = CPythonBackground::Instance();
	rkBgMgr.LocalPositionToGlobalPosition(rLocalX, rLocalY);
}

bool CPythonNetworkStream::__CanActMainInstance() const
{
	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();
	CInstanceBase * pkInstMain = rkChrMgr.GetMainInstancePtr();
	if (!pkInstMain)
		return false;

	return pkInstMain->CanAct();
}

void CPythonNetworkStream::__ClearNetworkActorManager() const
{
	m_rokNetActorMgr->Destroy();
}

void __SetWeaponPower(IAbstractPlayer & rkPlayer, uint32_t dwWeaponID)
{
	uint32_t minPower = 0;
	uint32_t maxPower = 0;
	uint32_t minMagicPower = 0;
	uint32_t maxMagicPower = 0;
	uint32_t addPower = 0;

	CItemData * pkWeapon;
	if (CItemManager::Instance().GetItemDataPointer(CPythonPlayer::Instance().GetItemIndex(TItemPos(EQUIPMENT, c_Equipment_Weapon)), &pkWeapon))
	{
		if (pkWeapon->GetType() == CItemData::ITEM_TYPE_WEAPON)
		{
			minPower = pkWeapon->GetValue(3);
			maxPower = pkWeapon->GetValue(4);
			minMagicPower = pkWeapon->GetValue(1);
			maxMagicPower = pkWeapon->GetValue(2);
			addPower = pkWeapon->GetValue(5);
		}
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
		else if (pkWeapon->GetType() == CItemData::ITEM_TYPE_COSTUME && pkWeapon->GetSubType() == CItemData::COSTUME_WEAPON)
		{
			CItemData * pkRealWeapon;
			if (CItemManager::Instance().GetItemDataPointer(CPythonPlayer::Instance().GetItemIndex(TItemPos(EQUIPMENT, c_Equipment_Weapon)), &pkRealWeapon))
			{
				minPower = pkRealWeapon->GetValue(3);
				maxPower = pkRealWeapon->GetValue(4);
				minMagicPower = pkRealWeapon->GetValue(1);
				maxMagicPower = pkRealWeapon->GetValue(2);
				addPower = pkRealWeapon->GetValue(5);
			}
		}
#endif
	}

	rkPlayer.SetWeaponPower(minPower, maxPower, minMagicPower, maxMagicPower, addPower);
}

//테이블에서 이름이 "." 인 것들
//차후에 서버에서 보내주지 않게 되면 없어질 함수..(서버님께 꼭!!협박; )
bool IsInvisibleRace(uint16_t raceNum)
{
	switch (raceNum)
	{
		case 20025:
		case 20038:
		case 20039:
			return true;

		default:
			return false;
	}
}

static SNetworkActorData s_kNetActorData;

bool CPythonNetworkStream::RecvCharacterAppendPacket()
{
	TPacketGCCharacterAdd chrAddPacket;
	if (!Recv(sizeof(chrAddPacket), &chrAddPacket))
		return false;

	__GlobalPositionToLocalPosition(chrAddPacket.x, chrAddPacket.y);

	SNetworkActorData kNetActorData;
	kNetActorData.m_bType = chrAddPacket.bType;
	kNetActorData.m_dwMovSpd = chrAddPacket.bMovingSpeed;
	kNetActorData.m_dwAtkSpd = chrAddPacket.bAttackSpeed;
	kNetActorData.m_dwRace = chrAddPacket.wRaceNum;

	kNetActorData.m_dwStateFlags = chrAddPacket.bStateFlag;
	kNetActorData.m_dwVID = chrAddPacket.dwVID;
	kNetActorData.m_fRot = chrAddPacket.angle;

	kNetActorData.m_stName = "";

	kNetActorData.m_stName = "";
	kNetActorData.m_kAffectFlags.CopyData(0, sizeof(chrAddPacket.dwAffectFlag[0]), &chrAddPacket.dwAffectFlag[0]);
	kNetActorData.m_kAffectFlags.CopyData(32, sizeof(chrAddPacket.dwAffectFlag[1]), &chrAddPacket.dwAffectFlag[1]);

	kNetActorData.SetPosition(chrAddPacket.x, chrAddPacket.y);

	kNetActorData.m_sAlignment = 0; /*chrAddPacket.sAlignment*/
	kNetActorData.m_byPKMode = 0; /*chrAddPacket.bPKMode*/
	kNetActorData.m_dwGuildID = 0; /*chrAddPacket.dwGuild*/
#ifdef ENABLE_SHOW_GUILD_LEADER
	kNetActorData.m_dwNewIsGuildName=0;
#endif
	kNetActorData.m_dwEmpireID = 0; /*chrAddPacket.bEmpire*/
	kNetActorData.m_dwArmor = 0; /*chrAddPacket.awPart[CHR_EQUIPPART_ARMOR]*/
	kNetActorData.m_dwWeapon = 0; /*chrAddPacket.awPart[CHR_EQUIPPART_WEAPON]*/
	kNetActorData.m_dwHair = 0; /*chrAddPacket.awPart[CHR_EQUIPPART_HAIR]*/
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	kNetActorData.m_dwAcce = 0; /*chrAddPacket.awPart[CHR_EQUIPPART_ACCE]*/
#endif
#ifdef ENABLE_PENDANT
	kNetActorData.m_dwPendant = 0;/*chrAddPacket.awPart[CHR_EQUIPPART_PENDANT]*/
#endif
#ifdef ENABLE_REFINE_ELEMENT
	kNetActorData.m_dwElementsEffect = 0;/*chrAddPacket.dwElementsEffect*/;
#endif
	kNetActorData.m_dwMountVnum = 0; /*chrAddPacket.dwMountVnum*/
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	kNetActorData.m_dwTitle = 0;
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	kNetActorData.m_dwArrow = 0;/*chrAddPacket.m_dwArrow*/
#endif
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
	kNetActorData.m_lGroupWeapon = 0;
#endif
#ifdef WJ_SHOW_MOB_INFO
	kNetActorData.m_dwLevel = chrAddPacket.dwLevel;
	kNetActorData.m_dwAIFlag = chrAddPacket.dwAIFlag;
#else
	kNetActorData.m_dwLevel = 0; // 몬스터 레벨 표시 안함
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	kNetActorData.m_dwcLevel = 0;
#endif
#ifdef ENABLE_AURA_SYSTEM
	kNetActorData.m_dwAura = 0;/* chrAddPacket.awPart[CHR_EQUIPPART_AURA]*/
#endif
#ifdef ENABLE_PORTAL_LEVEL_LIMIT
	if (kNetActorData.m_bType == CActorInstance::TYPE_WARP)
		CPythonNonPlayer::Instance().AddPortalLevelLimit(kNetActorData.m_dwRace, chrAddPacket.iPortalMinLevelLimit, chrAddPacket.iPortalMaxLevelLimit);
#endif

	if(kNetActorData.m_bType != CActorInstance::TYPE_PC &&
		kNetActorData.m_bType != CActorInstance::TYPE_NPC
#ifdef ENABLE_PROTO_RENEWAL
		&& kNetActorData.m_bType != CActorInstance::TYPE_HORSE
		&& kNetActorData.m_bType != CActorInstance::TYPE_PET_PAY
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
		&& kNetActorData.m_bType != CActorInstance::TYPE_PET
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		&& kNetActorData.m_bType != CActorInstance::TYPE_SHOP
#endif
		)
	{
		const char * c_szName;
		CPythonNonPlayer & rkNonPlayer = CPythonNonPlayer::Instance();
		if (rkNonPlayer.GetName(kNetActorData.m_dwRace, &c_szName))
			kNetActorData.m_stName = c_szName;
		//else
		//	kNetActorData.m_stName=chrAddPacket.name;

		__RecvCharacterAppendPacket(&kNetActorData);
	}
	else
		s_kNetActorData = kNetActorData;

	return true;
}

bool CPythonNetworkStream::RecvCharacterAdditionalInfo()
{
	TPacketGCCharacterAdditionalInfo chrInfoPacket;
	if (!Recv(sizeof(chrInfoPacket), &chrInfoPacket))
		return false;

	SNetworkActorData kNetActorData = s_kNetActorData;
	if (IsInvisibleRace(kNetActorData.m_dwRace))
		return true;

	if (kNetActorData.m_dwVID == chrInfoPacket.dwVID)
	{
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		const char* c_szName;
		if (CPythonNonPlayer::Instance().GetName(kNetActorData.m_dwRace, &c_szName))
		{
			switch (kNetActorData.m_bType)
			{
				case CActorInstance::TYPE_NPC:
					kNetActorData.m_stName = c_szName;
					break;

				case CActorInstance::TYPE_PET_PAY:
					char szPetName[1024];
					sprintf(szPetName, "%s - %s", chrInfoPacket.name, c_szName);
					kNetActorData.m_stName = szPetName;
					break;

				default:
					kNetActorData.m_stName = chrInfoPacket.name;
					break;
			}
		}
		else
			kNetActorData.m_stName = chrInfoPacket.name;
#else
		kNetActorData.m_stName = chrInfoPacket.name;
#endif
		kNetActorData.m_dwGuildID = chrInfoPacket.dwGuildID;
		kNetActorData.m_dwLevel = chrInfoPacket.dwLevel;
#ifdef ENABLE_YOHARA_SYSTEM
		kNetActorData.m_dwcLevel = chrInfoPacket.dwcLevel;
#endif
		kNetActorData.m_sAlignment = chrInfoPacket.sAlignment;
		kNetActorData.m_byPKMode = chrInfoPacket.bPKMode;
		//kNetActorData.m_dwGuildID = chrInfoPacket.dwGuildID;	//why 2 times?
#ifdef ENABLE_SHOW_GUILD_LEADER
		kNetActorData.m_dwNewIsGuildName = chrInfoPacket.dwNewIsGuildName;
#endif
		kNetActorData.m_dwEmpireID = chrInfoPacket.bEmpire;
		kNetActorData.m_dwArmor = chrInfoPacket.adwPart[CHR_EQUIPPART_ARMOR];	//@fixme411 - uint16_t -> uint32_t
		kNetActorData.m_dwWeapon = chrInfoPacket.adwPart[CHR_EQUIPPART_WEAPON];	//@fixme411 - uint16_t -> uint32_t
		kNetActorData.m_dwHair = chrInfoPacket.adwPart[CHR_EQUIPPART_HAIR];	//@fixme411 - uint16_t -> uint32_t
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		kNetActorData.m_dwAcce = chrInfoPacket.adwPart[CHR_EQUIPPART_ACCE];
#endif
#ifdef ENABLE_PENDANT
		kNetActorData.m_dwPendant = chrInfoPacket.adwPart[CHR_EQUIPPART_PENDANT];
#endif
#ifdef ENABLE_REFINE_ELEMENT
		kNetActorData.m_dwElementsEffect = chrInfoPacket.dwElementsEffect;
#endif
		kNetActorData.m_dwMountVnum = chrInfoPacket.dwMountVnum;
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		kNetActorData.m_dwTitle = chrInfoPacket.dwTitle;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		kNetActorData.m_bLanguage = chrInfoPacket.bLanguage;
#endif
#ifdef ENABLE_AURA_SYSTEM
		kNetActorData.m_dwAura = chrInfoPacket.adwPart[CHR_EQUIPPART_AURA];
#endif
#ifdef ENABLE_QUIVER_SYSTEM
		kNetActorData.m_dwArrow = chrInfoPacket.dwArrow;
#endif
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
		kNetActorData.m_lGroupWeapon = chrInfoPacket.lGroupWeapon;
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
		memcpy(kNetActorData.m_dwSkillColor, chrInfoPacket.dwSkillColor, sizeof(kNetActorData.m_dwSkillColor));
#endif

		__RecvCharacterAppendPacket(&kNetActorData);
	}
	else
		TraceError("TPacketGCCharacterAdditionalInfo name=%s vid=%d race=%d Error", chrInfoPacket.name, chrInfoPacket.dwVID,
				   kNetActorData.m_dwRace);
	return true;
}

bool CPythonNetworkStream::RecvCharacterUpdatePacket()
{
	TPacketGCCharacterUpdate chrUpdatePacket;
	if (!Recv(sizeof(chrUpdatePacket), &chrUpdatePacket))
		return false;

	SNetworkUpdateActorData kNetUpdateActorData;
	kNetUpdateActorData.m_dwGuildID = chrUpdatePacket.dwGuildID;
#ifdef ENABLE_SHOW_GUILD_LEADER
	kNetUpdateActorData.m_dwNewIsGuildName = chrUpdatePacket.dwNewIsGuildName;
#endif
	kNetUpdateActorData.m_dwMovSpd = chrUpdatePacket.bMovingSpeed;
	kNetUpdateActorData.m_dwAtkSpd = chrUpdatePacket.bAttackSpeed;
	kNetUpdateActorData.m_dwArmor = chrUpdatePacket.adwPart[CHR_EQUIPPART_ARMOR];	//@fixme411 - uint16_t -> uint32_t
	kNetUpdateActorData.m_dwWeapon = chrUpdatePacket.adwPart[CHR_EQUIPPART_WEAPON];	//@fixme411 - uint16_t -> uint32_t
	kNetUpdateActorData.m_dwHair = chrUpdatePacket.adwPart[CHR_EQUIPPART_HAIR];	//@fixme411 - uint16_t -> uint32_t
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	kNetUpdateActorData.m_dwAcce = chrUpdatePacket.adwPart[CHR_EQUIPPART_ACCE];
#endif
#ifdef ENABLE_PENDANT
	kNetUpdateActorData.m_dwPendant = chrUpdatePacket.adwPart[CHR_EQUIPPART_PENDANT];
#endif
#ifdef ENABLE_REFINE_ELEMENT
	kNetUpdateActorData.m_dwElementsEffect = chrUpdatePacket.dwElementsEffect;
#endif
	kNetUpdateActorData.m_dwVID = chrUpdatePacket.dwVID;
	kNetUpdateActorData.m_kAffectFlags.CopyData(0, sizeof(chrUpdatePacket.dwAffectFlag[0]), &chrUpdatePacket.dwAffectFlag[0]);
	kNetUpdateActorData.m_kAffectFlags.CopyData(32, sizeof(chrUpdatePacket.dwAffectFlag[1]), &chrUpdatePacket.dwAffectFlag[1]);
	kNetUpdateActorData.m_sAlignment = chrUpdatePacket.sAlignment;
#ifdef WJ_SHOW_MOB_INFO
	kNetUpdateActorData.m_dwLevel = chrUpdatePacket.dwLevel;
#endif
	kNetUpdateActorData.m_byPKMode = chrUpdatePacket.bPKMode;
	kNetUpdateActorData.m_dwStateFlags = chrUpdatePacket.bStateFlag;
	kNetUpdateActorData.m_dwMountVnum = chrUpdatePacket.dwMountVnum;
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	kNetUpdateActorData.m_dwTitle = chrUpdatePacket.dwTitle;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	kNetUpdateActorData.m_bLanguage = chrUpdatePacket.bLanguage;
#endif
#ifdef ENABLE_AURA_SYSTEM
	kNetUpdateActorData.m_dwAura = chrUpdatePacket.adwPart[CHR_EQUIPPART_AURA];
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	kNetUpdateActorData.m_dwArrow = chrUpdatePacket.dwArrow;
#endif
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
	kNetUpdateActorData.m_lGroupWeapon = chrUpdatePacket.lGroupWeapon;
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	memcpy(kNetUpdateActorData.m_dwSkillColor, chrUpdatePacket.dwSkillColor, sizeof(kNetUpdateActorData.m_dwSkillColor));
#endif
	__RecvCharacterUpdatePacket(&kNetUpdateActorData);

	return true;
}

void CPythonNetworkStream::__RecvCharacterAppendPacket(const SNetworkActorData * pkNetActorData)
{
	// NOTE : 카메라가 땅에 묻히는 문제의 해결을 위해 메인 캐릭터가 지형에 올려지기
	//        전에 맵을 업데이트 해 높이를 구할 수 있도록 해놓아야 합니다.
	//        단, 게임이 들어갈때가 아닌 이미 캐릭터가 추가 된 이후에만 합니다.
	//        헌데 이동인데 왜 Move로 안하고 Append로 하는지..? - [levites]
	IAbstractPlayer & rkPlayer = IAbstractPlayer::GetSingleton();
	if (rkPlayer.IsMainCharacterIndex(pkNetActorData->m_dwVID))
	{
		rkPlayer.SetRace(pkNetActorData->m_dwRace);

		__SetWeaponPower(rkPlayer, pkNetActorData->m_dwWeapon);

		if (rkPlayer.NEW_GetMainActorPtr())
		{
			CPythonBackground::Instance().Update(pkNetActorData->m_lCurX, pkNetActorData->m_lCurY, 0.0f);
			CPythonCharacterManager::Instance().Update();

			// NOTE : 사귀 타워일 경우 GOTO 로 이동시에도 맵 이름을 출력하도록 처리
			{
				std::string strMapName = CPythonBackground::Instance().GetWarpMapName();
				if (strMapName == "metin2_map_deviltower1")
					__ShowMapName(pkNetActorData->m_lCurX, pkNetActorData->m_lCurY);
			}
		}
		else
			__ShowMapName(pkNetActorData->m_lCurX, pkNetActorData->m_lCurY);
	}

	m_rokNetActorMgr->AppendActor(*pkNetActorData);

	if (GetMainActorVID() == pkNetActorData->m_dwVID)
	{
		rkPlayer.SetTarget(0);
		if (m_bComboSkillFlag)
			rkPlayer.SetComboSkillFlag(m_bComboSkillFlag);

		__SetGuildID(pkNetActorData->m_dwGuildID);
		//CPythonApplication::Instance().SkipRenderBuffering(10000);
	}
}

void CPythonNetworkStream::__RecvCharacterUpdatePacket(const SNetworkUpdateActorData * pkNetUpdateActorData)
{
	m_rokNetActorMgr->UpdateActor(*pkNetUpdateActorData);

	IAbstractPlayer & rkPlayer = IAbstractPlayer::GetSingleton();
	if (rkPlayer.IsMainCharacterIndex(pkNetUpdateActorData->m_dwVID))
	{
		__SetGuildID(pkNetUpdateActorData->m_dwGuildID);
		__SetWeaponPower(rkPlayer, pkNetUpdateActorData->m_dwWeapon);

		__RefreshStatus();
		__RefreshAlignmentWindow();
		__RefreshEquipmentWindow();
		__RefreshInventoryWindow();
	}
	else
		rkPlayer.NotifyCharacterUpdate(pkNetUpdateActorData->m_dwVID);
}

bool CPythonNetworkStream::RecvCharacterDeletePacket()
{
	TPacketGCCharacterDelete chrDelPacket;

	if (!Recv(sizeof(chrDelPacket), &chrDelPacket))
	{
		TraceError("CPythonNetworkStream::RecvCharacterDeletePacket - Recv Error");
		return false;
	}

	m_rokNetActorMgr->RemoveActor(chrDelPacket.dwVID);

	// 캐릭터가 사라질때 개인 상점도 없애줍니다.
	// Key Check 를 하기때문에 없어도 상관은 없습니다.
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_PrivateShop_Disappear", Py_BuildValue("(i)", chrDelPacket.dwVID));

	return true;
}


bool CPythonNetworkStream::RecvCharacterMovePacket()
{
	TPacketGCMove kMovePacket;
	if (!Recv(sizeof(TPacketGCMove), &kMovePacket))
	{
		Tracen("CPythonNetworkStream::RecvCharacterMovePacket - PACKET READ ERROR");
		return false;
	}

	__GlobalPositionToLocalPosition(kMovePacket.lX, kMovePacket.lY);

	SNetworkMoveActorData kNetMoveActorData;
	kNetMoveActorData.m_dwArg = kMovePacket.bArg;
	kNetMoveActorData.m_dwFunc = kMovePacket.bFunc;
	kNetMoveActorData.m_dwTime = kMovePacket.dwTime;
	kNetMoveActorData.m_dwVID = kMovePacket.dwVID;
	kNetMoveActorData.m_fRot = kMovePacket.bRot * 5.0f;
	kNetMoveActorData.m_lPosX = kMovePacket.lX;
	kNetMoveActorData.m_lPosY = kMovePacket.lY;
	kNetMoveActorData.m_dwDuration = kMovePacket.dwDuration;

	m_rokNetActorMgr->MoveActor(kNetMoveActorData);

	return true;
}

bool CPythonNetworkStream::RecvOwnerShipPacket()
{
	TPacketGCOwnership kPacketOwnership;

	if (!Recv(sizeof(kPacketOwnership), &kPacketOwnership))
		return false;

	m_rokNetActorMgr->SetActorOwner(kPacketOwnership.dwOwnerVID, kPacketOwnership.dwVictimVID);

	return true;
}

bool CPythonNetworkStream::RecvSyncPositionPacket()
{
	TPacketGCSyncPosition kPacketSyncPos;
	if (!Recv(sizeof(kPacketSyncPos), &kPacketSyncPos))
		return false;

	TPacketGCSyncPositionElement kSyncPos;

	uint32_t uSyncPosCount = (kPacketSyncPos.wSize - sizeof(kPacketSyncPos)) / sizeof(kSyncPos);
	for (uint32_t iSyncPos = 0; iSyncPos < uSyncPosCount; ++iSyncPos)
	{
		if (!Recv(sizeof(TPacketGCSyncPositionElement), &kSyncPos))
			return false;

#ifdef __MOVIE_MODE__
		return true;
#endif //__MOVIE_MODE__

		__GlobalPositionToLocalPosition(kSyncPos.lX, kSyncPos.lY);
		m_rokNetActorMgr->SyncActor(kSyncPos.dwVID, kSyncPos.lX, kSyncPos.lY);
	}

	return true;
}
