#include "StdAfx.h"
#include "InstanceBase.h"
#include "PythonBackground.h"
#include "PythonNonPlayer.h"
#include "PythonPlayer.h"
#include "PythonCharacterManager.h"
#include "AbstractPlayer.h"
#include "AbstractApplication.h"
#include "Packet.h"

#include "../EterLib/StateManager.h"
#include "../GameLib/ItemManager.h"
#include "../GameLib/GameLibDefines.h"

#ifdef WJ_SHOW_MOB_INFO
#	include "PythonSystem.h"
#	include "PythonTextTail.h"
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
# include "PythonLocale.h"
#endif

BOOL HAIR_COLOR_ENABLE = FALSE;
BOOL USE_ARMOR_SPECULAR = TRUE;	// Original -> FALSE
BOOL RIDE_HORSE_ENABLE = TRUE;
#ifndef ENABLE_SET_NO_CHARACTER_ROTATION
const float c_fDefaultRotationSpeed = 1200.0f;
#else
const float c_fDefaultRotationSpeed = 2400.0f;
#endif
#ifndef ENABLE_SET_NO_MOUNT_ROTATION
const float c_fDefaultHorseRotationSpeed = 300.0f;
#else
const float c_fDefaultHorseRotationSpeed = 1500.0f;
#endif

bool IsWall(unsigned race)
{
	switch (race)
	{
		case 14201:
		case 14202:
		case 14203:
		case 14204:
			return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////

CInstanceBase::SHORSE::SHORSE()
{
	__Initialize();
}

CInstanceBase::SHORSE::~SHORSE()
{
	assert(m_pkActor == nullptr);
}

void CInstanceBase::SHORSE::__Initialize()
{
	m_isMounting = false;
	m_pkActor.reset();
}

void CInstanceBase::SHORSE::SetAttackSpeed(uint32_t uAtkSpd) const
{
	if (!IsMounting())
		return;

	CActorInstance & rkActor = GetActorRef();
	rkActor.SetAttackSpeed(uAtkSpd / 100.0f);
}

void CInstanceBase::SHORSE::SetMoveSpeed(uint32_t uMovSpd) const
{
	if (!IsMounting())
		return;

	CActorInstance & rkActor = GetActorRef();
	rkActor.SetMoveSpeed(uMovSpd / 100.0f);
}

void CInstanceBase::SHORSE::Create(const TPixelPosition & c_rkPPos, uint32_t eRace, uint32_t eHitEffect)
{
	assert(nullptr == m_pkActor && "CInstanceBase::SHORSE::Create - ALREADY MOUNT");

	m_pkActor = std::make_unique<CActorInstance>();

	CActorInstance & rkActor = GetActorRef();
	rkActor.SetEventHandler(CActorInstance::IEventHandler::GetEmptyPtr());
	if (!rkActor.SetRace(eRace))
	{
		m_pkActor.reset();
		return;
	}

	rkActor.SetShape(0);
	rkActor.SetBattleHitEffect(eHitEffect);
	rkActor.SetAlphaValue(0.0f);
	rkActor.BlendAlphaValue(1.0f, 0.5f);
	rkActor.SetMoveSpeed(1.0f);
	rkActor.SetAttackSpeed(1.0f);
	rkActor.SetMotionMode(CRaceMotionData::MODE_GENERAL);
	rkActor.Stop();
	rkActor.RefreshActorInstance();

	rkActor.SetCurPixelPosition(c_rkPPos);

	m_isMounting = true;
}

void CInstanceBase::SHORSE::Destroy()
{
	if (m_pkActor)
	{
		m_pkActor->Destroy();
		m_pkActor.reset();
	}

	__Initialize();
}

CActorInstance & CInstanceBase::SHORSE::GetActorRef() const
{
	assert(nullptr != m_pkActor && "CInstanceBase::SHORSE::GetActorRef");
	return *m_pkActor;
}

CActorInstance * CInstanceBase::SHORSE::GetActorPtr() const
{
	return m_pkActor.get();
}

enum eMountType
{
	MOUNT_TYPE_NONE = 0,
	MOUNT_TYPE_NORMAL = 1,
	MOUNT_TYPE_COMBAT = 2,
	MOUNT_TYPE_MILITARY = 3
};
eMountType GetMountLevelByVnum(uint32_t dwMountVnum, bool IsNew)
{
	if (!dwMountVnum)
		return MOUNT_TYPE_NONE;

	switch (dwMountVnum)
	{
		// ### YES SKILL
		// @fixme116 begin
		case 20107: // normal military horse (no guild)
		case 20108: // normal military horse (guild member)
		case 20109: // normal military horse (guild master)
			if (IsNew)
				return MOUNT_TYPE_NONE;
		// @fixme116 end

		// Classic
		case 20110: // Classic Boar
		case 20111: // Classic Wolf
		case 20112: // Classic Tiger
		case 20113: // Classic Lion
		case 20114: // White Lion
		// Special Lv2
		case 20115: // Wild Battle Boar
		case 20116: // Fight Wolf
		case 20117: // Storm Tiger
		case 20118: // Battle Lion (bugged)
		case 20205: // Wild Battle Boar (alternative)
		case 20206: // Fight Wolf (alternative)
		case 20207: // Storm Tiger (alternative)
		case 20208: // Battle Lion (bugged) (alternative)
		// Royal Tigers
		case 20120: // blue
		case 20121: // dark red
		case 20122: // gold
		case 20123: // green
		case 20124: // pied
		case 20125: // white
		// Royal mounts (Special Lv3)
		case 20209: // Royal Boar
		case 20210: // Royal Wolf
		case 20211: // Royal Tiger
		case 20212: // Royal Lion
		//
		case 20215: // Rudolph m Lv3 (yes skill, yes atk)
		case 20218: // Rudolph f Lv3 (yes skill, yes atk)
		case 20225: // Dyno Lv3 (yes skill, yes atk)
		case 20230: // Turkey Lv3 (yes skill, yes atk)

		//NewMonuts [Date: 2020]
		case 20233:	// Muffin (Zubehör braun)
		case 20234:	// Nugget (Zubehör braun)
		case 20235:	// Muffin (Zubehör blau)
		case 20236:	// Nugget (Zubehör blau)
		case 20237:	// Muffin (Zubehör grün)
		case 20238:	// Nugget (Zubehör grün)
		case 20239:	// Blitzen
		case 20240:	// Donner
		case 20241:	// Nugget (marine)
		case 20242:	// Muffin (rosa)
		case 20243:	// Kyzaghans Pferd
		case 20244:	// Nirwana-Drache
		case 20245:	// Samsara-Drache
		case 20246:	// Manni
		case 20247:	// Manu
		case 20248:	// Braunes Kampf-Tulu
		case 20249:	// Schwarzes Kampf-Tulu
		case 20250:	// Lavahöllenechse
		case 20251:	// Eishöllenechse
		case 20252:	// Magmadrache
		case 20254:	// Weißes Kaninchen
		case 20255:	// Mondhase
		case 20257:	// Bernie
		case 20258:	// Blue Barry
		case 20259:	// Zerberus (gold)
		case 20260:	// Zerberus (silber)
		case 20261:	// Spektralross
		case 20262:	// Ross Nocturnos
		case 20263:	// Nachtflügel
		case 20264:	// Finsterschwinge
		case 20265:	// Mondschatten
		case 20266:	// Silberstern
		case 20267:	// Goldener Kriegsstier
		case 20268:	// Silberner Kriegsstier
		case 20269:	// Weiße Sphinx
		case 20270:	// Steinerne Sphinx
		case 20271:	// Eisrabe
		case 20272:	// Nachtrabe
			return MOUNT_TYPE_MILITARY;

		// ### NO SKILL YES ATK
		// @fixme116 begin
		case 20104: // normal combat horse (no guild)
		case 20105: // normal combat horse (guild member)
		case 20106: // normal combat horse (guild master)
			if (IsNew)
				return MOUNT_TYPE_NONE;
		// @fixme116 end

		case 20119: // Black Horse (no skill, yes atk)
		case 20214: // Rudolph m Lv2 (no skill, yes atk)
		case 20217: // Rudolph f Lv2 (no skill, yes atk)
		case 20219: // Equus Porphyreus (no skill, yes atk)
		case 20220: // Comet (no skill, yes atk)
		case 20221: // Polar Predator (no skill, yes atk)
		case 20222: // Armoured Panda (no skill, yes atk)
		case 20224: // Dyno Lv2 (no skill, yes atk)
		case 20226: // Nightmare (no skill, yes atk)
		case 20227: // Unicorn (no skill, yes atk)
		case 20229: // Turkey Lv2 (no skill, yes atk)
		case 20231: // Leopard (no skill, yes atk)
		case 20232: // Black Panther (no skill, yes atk)
			return MOUNT_TYPE_COMBAT;

		// ### NO SKILL NO ATK
		// @fixme116 begin
		case 20101: // normal beginner horse (no guild)
		case 20102: // normal beginner horse (guild member)
		case 20103: // normal beginner horse (guild master)
			if (IsNew)
				return MOUNT_TYPE_NONE;

		// @fixme116 end
		case 20213: // Rudolph m Lv1 (no skill, no atk)
		case 20216: // Rudolph f Lv1 (no skill, no atk)
		// Special Lv1
		case 20201: // Boar Lv1 (no skill, no atk)
		case 20202: // Wolf Lv1 (no skill, no atk)
		case 20203: // Tiger Lv1 (no skill, no atk)
		case 20204: // Lion Lv1 (no skill, no atk)
		//
		case 20223: // Dyno Lv1 (no skill, no atk)
		case 20228: // Turkey Lv1 (no skill, no atk)

		//NewMonuts [Date: 2020]
		case 20253:	// Appaloosa
			return MOUNT_TYPE_NORMAL;
		default:
			return MOUNT_TYPE_NONE;
	}
}

uint32_t CInstanceBase::SHORSE::GetLevel() const
{
	if (m_pkActor)
	{
#ifndef ENABLE_NO_MOUNT_CHECK
		return static_cast<uint32_t>(GetMountLevelByVnum(m_pkActor->GetRace(), false));
#else
		return (m_pkActor->GetRace()) ? MOUNT_TYPE_MILITARY : MOUNT_TYPE_NONE;
#endif
	}
	return 0;
}

bool CInstanceBase::SHORSE::IsNewMount() const
{
#ifndef ENABLE_NO_MOUNT_CHECK
	if (m_pkActor)
	{
		eMountType mountType = GetMountLevelByVnum(m_pkActor->GetRace(), true);
		return (mountType != MOUNT_TYPE_NONE) && (mountType != MOUNT_TYPE_NORMAL);
	}
#endif
	return false;
}
bool CInstanceBase::SHORSE::CanUseSkill() const
{
	if (IsMounting())
		return MOUNT_TYPE_MILITARY == GetLevel();

	return true;
}

bool CInstanceBase::SHORSE::CanAttack() const
{
	if (IsMounting())
		if (GetLevel() < MOUNT_TYPE_COMBAT)
			return false;

	return true;
}

bool CInstanceBase::SHORSE::IsMounting() const
{
	return m_isMounting;
}

void CInstanceBase::SHORSE::Deform() const
{
	if (!IsMounting())
		return;

	CActorInstance & rkActor = GetActorRef();
	rkActor.INSTANCEBASE_Deform();
}

void CInstanceBase::SHORSE::Render() const
{
	if (!IsMounting())
		return;

	CActorInstance & rkActor = GetActorRef();
	rkActor.Render();
}

void CInstanceBase::__AttachHorseSaddle()
{
	if (!IsMountingHorse())
		return;
	m_kHorse.m_pkActor->AttachModelInstance(CRaceData::PART_MAIN, "saddle", m_GraphicThingInstance, CRaceData::PART_MAIN);
}

void CInstanceBase::__DetachHorseSaddle()
{
	if (!IsMountingHorse())
		return;
	m_kHorse.m_pkActor->DetachModelInstance(CRaceData::PART_MAIN, m_GraphicThingInstance, CRaceData::PART_MAIN);
}

//////////////////////////////////////////////////////////////////////////////////////

void CInstanceBase::BlockMovement()
{
	m_GraphicThingInstance.BlockMovement();
}

bool CInstanceBase::IsBlockObject(const CGraphicObjectInstance & c_rkBGObj)
{
	return m_GraphicThingInstance.IsBlockObject(c_rkBGObj);
}

bool CInstanceBase::AvoidObject(const CGraphicObjectInstance & c_rkBGObj)
{
	return m_GraphicThingInstance.AvoidObject(c_rkBGObj);
}

///////////////////////////////////////////////////////////////////////////////////

bool __ArmorVnumToShape(int iVnum, uint32_t * pdwShape)
{
	*pdwShape = iVnum;

	/////////////////////////////////////////

	if (0 == iVnum || 1 == iVnum)
		return false;

	if (!USE_ARMOR_SPECULAR)
		return false;

	CItemData * pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(iVnum, &pItemData))
		return false;

	enum
	{
		SHAPE_VALUE_SLOT_INDEX = 3
	};

	*pdwShape = pItemData->GetValue(SHAPE_VALUE_SLOT_INDEX);

	return true;
}

// 2004.07.05.myevan.±Ã½ÅÅº¿µ ³¢ÀÌ´Â ¹®Á¦
class CActorInstanceBackground : public IBackground
{
public:
	CActorInstanceBackground() = default;
	~CActorInstanceBackground() = default;

	bool IsBlock(int x, int y) override
	{
		CPythonBackground & rkBG = CPythonBackground::Instance();
		return rkBG.isAttrOn(x, y, CTerrainImpl::ATTRIBUTE_BLOCK);
	}
};

static CActorInstanceBackground gs_kActorInstBG;

bool CInstanceBase::LessRenderOrder(CInstanceBase * pkInst)
{
	int nMainAlpha = (__GetAlphaValue() < 1.0f) ? 1 : 0;
	int nTestAlpha = (pkInst->__GetAlphaValue() < 1.0f) ? 1 : 0;
	if (nMainAlpha < nTestAlpha)
		return true;
	if (nMainAlpha > nTestAlpha)
		return false;

	if (GetRace() < pkInst->GetRace())
		return true;
	if (GetRace() > pkInst->GetRace())
		return false;

	if (GetShape() < pkInst->GetShape())
		return true;

	if (GetShape() > pkInst->GetShape())
		return false;

	uint32_t uLeftLODLevel = __LessRenderOrder_GetLODLevel();
	uint32_t uRightLODLevel = pkInst->__LessRenderOrder_GetLODLevel();
	if (uLeftLODLevel < uRightLODLevel)
		return true;
	if (uLeftLODLevel > uRightLODLevel)
		return false;

	if (m_awPart[CRaceData::PART_WEAPON] < pkInst->m_awPart[CRaceData::PART_WEAPON])
		return true;

	return false;
}

uint32_t CInstanceBase::__LessRenderOrder_GetLODLevel()
{
	CGrannyLODController * pLODCtrl = m_GraphicThingInstance.GetLODControllerPointer(0);
	if (!pLODCtrl)
		return 0;

	return pLODCtrl->GetLODLevel();
}

bool CInstanceBase::__Background_GetWaterHeight(const TPixelPosition & c_rkPPos, float * pfHeight) const
{
	long lHeight;
	if (!CPythonBackground::Instance().GetWaterHeight(int(c_rkPPos.x), int(c_rkPPos.y), &lHeight))
		return false;

	*pfHeight = float(lHeight);

	return true;
}

bool CInstanceBase::__Background_IsWaterPixelPosition(const TPixelPosition & c_rkPPos) const
{
	return CPythonBackground::Instance().isAttrOn(c_rkPPos.x, c_rkPPos.y, CTerrainImpl::ATTRIBUTE_WATER);
}

const float PC_DUST_RANGE = 2000.0f;
const float NPC_DUST_RANGE = 1000.0f;

uint32_t CInstanceBase::ms_dwUpdateCounter = 0;
uint32_t CInstanceBase::ms_dwRenderCounter = 0;
uint32_t CInstanceBase::ms_dwDeformCounter = 0;

CDynamicPool<CInstanceBase> CInstanceBase::ms_kPool;

bool CInstanceBase::__IsInDustRange()
{
	if (!__IsExistMainInstance())
		return false;

	CInstanceBase * pkInstMain = __GetMainInstancePtr();

	float fDistance = NEW_GetDistanceFromDestInstance(*pkInstMain);

	if (IsPC())
	{
		if (fDistance <= PC_DUST_RANGE)
			return true;
	}

	if (fDistance <= NPC_DUST_RANGE)
		return true;

	return false;
}

void CInstanceBase::__EnableSkipCollision()
{
	if (__IsMainInstance())
	{
		TraceError("CInstanceBase::__EnableSkipCollision - You should not skip your own collisions!!");
		return;
	}
	m_GraphicThingInstance.EnableSkipCollision();
}

void CInstanceBase::__DisableSkipCollision()
{
	m_GraphicThingInstance.DisableSkipCollision();
}

uint32_t CInstanceBase::__GetShadowMapColor(float x, float y)
{
	CPythonBackground & rkBG = CPythonBackground::Instance();
	return rkBG.GetShadowMapColor(x, y);
}

float CInstanceBase::__GetBackgroundHeight(float x, float y)
{
	CPythonBackground & rkBG = CPythonBackground::Instance();
	return rkBG.GetHeight(x, y);
}

#ifdef __MOVIE_MODE__

BOOL CInstanceBase::IsMovieMode()
{
#	ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
	if (IsAffect(AFFECT_INVISIBILITY) && !__MainCanSeeHiddenThing())
		return true;
#	else
	if (IsAffect(AFFECT_INVISIBILITY))
		return true;
#	endif

	return false;
}

#endif

BOOL CInstanceBase::IsInvisibility()
{
#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
	if (IsAffect(AFFECT_INVISIBILITY) && !__MainCanSeeHiddenThing())
		return true;
#else
	if (IsAffect(AFFECT_INVISIBILITY))
		return true;
#endif

	if (IsAffect(AFFECT_EUNHYEONG))	//@fixme410
		return true;

	return false;
}

BOOL CInstanceBase::IsParalysis()
{
	return m_GraphicThingInstance.IsParalysis();
}

BOOL CInstanceBase::IsGameMaster() const
{
	if (m_kAffectFlagContainer.IsSet(AFFECT_YMIR))
		return true;
	return false;
}

#ifdef ENABLE_BATTLE_FIELD
BOOL CInstanceBase::IsTargetVictim() const
{
	if (m_kAffectFlagContainer.IsSet(AFFECT_TARGET_VICTIM))
		return true;
	return false;
}
#endif

#ifdef ENABLE_VIP_SYSTEM
BOOL CInstanceBase::IsVIP() const
{
	if (m_kAffectFlagContainer.IsSet(AFFECT_VIP))
		return true;
	return false;
}
#endif

BOOL CInstanceBase::IsSameEmpire(const CInstanceBase & rkInstDst) const
{
	if (0 == rkInstDst.m_dwEmpireID)
		return TRUE;

	if (IsGameMaster())
		return TRUE;

#ifdef ENABLE_VIP_SYSTEM
	if (IsVIP())
		return TRUE;
#endif

	if (rkInstDst.IsGameMaster())
		return TRUE;

#ifdef ENABLE_VIP_SYSTEM
	if (rkInstDst.IsVIP())
		return TRUE;
#endif

	if (rkInstDst.m_dwEmpireID == m_dwEmpireID)
		return TRUE;

	return FALSE;
}

uint32_t CInstanceBase::GetEmpireID() const
{
	return m_dwEmpireID;
}

#ifdef WJ_SHOW_MOB_INFO
uint32_t CInstanceBase::GetAIFlag() const
{
	return m_dwAIFlag;
}
#endif

uint32_t CInstanceBase::GetGuildID() const
{
	return m_dwGuildID;
}

#ifdef ENABLE_SHOW_GUILD_LEADER
uint8_t CInstanceBase::GetNewIsGuildName()
{
	return m_dwNewIsGuildName;
}
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
uint32_t* CInstanceBase::GetSkillColor(uint32_t dwSkillIndex)
{
	uint32_t dwSkillSlot = dwSkillIndex + 1;
	CPythonSkill::SSkillData* c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(dwSkillSlot, &c_pSkillData))
		return 0;

	uint16_t dwEffectID = c_pSkillData->GradeData[CPythonSkill::SKILL_GRADE_COUNT].wMotionIndex - CRaceMotionData::NAME_SKILL - (1 * 25);

	return m_GraphicThingInstance.GetSkillColorByMotionID(dwEffectID);
}
#endif

int CInstanceBase::GetAlignment() const
{
	return m_sAlignment;
}

uint32_t CInstanceBase::GetAlignmentGrade() const
{
	if (m_sAlignment >= 12000)
		return 0;
	if (m_sAlignment >= 8000)
		return 1;
	if (m_sAlignment >= 4000)
		return 2;
	if (m_sAlignment >= 1000)
		return 3;
	if (m_sAlignment >= 0)
		return 4;
	if (m_sAlignment > -4000)
		return 5;
	if (m_sAlignment > -8000)
		return 6;
	if (m_sAlignment > -12000)
		return 7;

	return 8;
}

int CInstanceBase::GetAlignmentType() const
{
	switch (GetAlignmentGrade())
	{
		case 0:
		case 1:
		case 2:
		case 3:
		{
			return ALIGNMENT_TYPE_WHITE;
		}

		case 5:
		case 6:
		case 7:
		case 8:
		{
			return ALIGNMENT_TYPE_DARK;
		}
	}

	return ALIGNMENT_TYPE_NORMAL;
}

uint8_t CInstanceBase::GetPKMode() const
{
	return m_byPKMode;
}

bool CInstanceBase::IsKiller() const
{
	return m_isKiller;
}

bool CInstanceBase::IsPartyMember() const
{
	return m_isPartyMember;
}

BOOL CInstanceBase::IsInSafe()
{
	const TPixelPosition & c_rkPPosCur = m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
	if (CPythonBackground::Instance().isAttrOn(c_rkPPosCur.x, c_rkPPosCur.y, CTerrainImpl::ATTRIBUTE_BANPK))
		return TRUE;

	return FALSE;
}

float CInstanceBase::CalculateDistanceSq3d(const TPixelPosition & c_rkPPosDst)
{
	const TPixelPosition & c_rkPPosSrc = m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
	return SPixelPosition_CalculateDistanceSq3d(c_rkPPosSrc, c_rkPPosDst);
}

void CInstanceBase::OnSelected()
{
#ifdef __MOVIE_MODE__
	if (!__IsExistMainInstance())
		return;
#endif

	if (IsStoneDoor())
		return;

	if (IsDead())
		return;

#ifdef ENABLE_TARGET_SELECT_COLOR
	if (IsEnemy() || IsStone())
	{
		__AttachSelectEffectMonster();
	}
	else if (IsPC() || IsNPC()
#ifdef ENABLE_GROWTH_PET_SYSTEM
		|| IsGrowthPet()
#endif
		)
	{
		if (m_dwEmpireID == 1) // Shinshoo
		{
			__AttachSelectEffectShinsoo();
		}
		else if (m_dwEmpireID == 2) // chunjo
		{
			__AttachSelectEffectChunjo();
		}
		else if (m_dwEmpireID == 3)//Jinnos
		{
			__AttachSelectEffectJinnos();
		}
	}
	else
#ifdef ENABLE_RENDER_LOGIN_EFFECTS
	if (!m_GraphicThingInstance.IsLoginRender())
		__AttachSelectEffect();
#else
	__AttachSelectEffect();
#endif

#else
	
#ifdef ENABLE_RENDER_LOGIN_EFFECTS
	if (!m_GraphicThingInstance.IsLoginRender())
		__AttachSelectEffect();
#else
	__AttachSelectEffect();
#endif

#endif
}

void CInstanceBase::OnUnselected()
{
#ifdef ENABLE_TARGET_SELECT_COLOR
	if (IsPC() || IsNPC()
#ifdef ENABLE_GROWTH_PET_SYSTEM
		|| IsGrowthPet()
#endif
		)
	{
		if (m_dwEmpireID == 1) //Shinsoo
		{
			__DetachSelectEffectShinsoo();
		}
		else if (m_dwEmpireID == 2)//Chunjo
		{
			__DetachSelectEffectChunjo();
		}
		else if (m_dwEmpireID == 3)//Jinnnos
		{
			__DetachSelectEffectJinnos();
		}
	}
	else if (IsEnemy() || IsStone())
	{
		__DetachSelectEffectMonster();
	}
	else
		__DetachSelectEffect();
#else
	__DetachSelectEffect();
#endif
}

void CInstanceBase::OnTargeted()
{
#ifdef __MOVIE_MODE__
	if (!__IsExistMainInstance())
		return;
#endif

	if (IsStoneDoor())
		return;

	if (IsDead())
		return;

#ifdef ENABLE_TARGET_SELECT_COLOR
	if (IsEnemy() || IsStone())
	{
		__AttachTargetEffectMonster();
	}
	else if (IsPC() || IsNPC()
#ifdef ENABLE_GROWTH_PET_SYSTEM
		|| IsGrowthPet()
#endif
		)
	{
		if (m_dwEmpireID == 1) //Shinsoo
		{
			__AttachTargetEffectShinsoo();
		}
		else if (m_dwEmpireID == 2)//Chunjo
		{
			__AttachTargetEffectChunjo();
		}
		else if (m_dwEmpireID == 3)//Jinnnos
		{
			__AttachTargetEffectJinnos();
		}
	}
	else
		__AttachTargetEffect();
#else
	__AttachTargetEffect();
#endif
}

void CInstanceBase::OnUntargeted()
{
#ifdef ENABLE_TARGET_SELECT_COLOR
	if (IsPC() || IsNPC()
#ifdef ENABLE_GROWTH_PET_SYSTEM
		|| IsGrowthPet()
#endif
		)
	{
		if (m_dwEmpireID == 1) //Shinsoo
		{
			__DetachTargetEffectShinsoo();
		}
		else if (m_dwEmpireID == 2)//Chunjo
		{
			__DetachTargetEffectChunjo();
		}
		else if (m_dwEmpireID == 3)//Jinnnos
		{
			__DetachTargetEffectJinnos();
		}
	}
	else if (IsEnemy() || IsStone())
	{
		__DetachTargetEffectMonster();
	}
	else
		__DetachTargetEffect();
#else
	__DetachTargetEffect();
#endif
}

void CInstanceBase::DestroySystem()
{
	ms_kPool.Clear();
}

void CInstanceBase::CreateSystem(uint32_t uCapacity)
{
	ms_kPool.Create(uCapacity);

	msl::refill(ms_adwCRCAffectEffect);

	ms_fDustGap = 250.0f;
	ms_fHorseDustGap = 500.0f;
}

CInstanceBase * CInstanceBase::New()
{
	return ms_kPool.Alloc();
}

void CInstanceBase::Delete(CInstanceBase * pkInst)
{
	pkInst->Destroy();
	ms_kPool.Free(pkInst);
}

void CInstanceBase::SetMainInstance()
{
	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();

	uint32_t dwVID = GetVirtualID();
	rkChrMgr.SetMainInstance(dwVID);

	m_GraphicThingInstance.SetMainInstance();
}

CInstanceBase * CInstanceBase::__GetMainInstancePtr() const
{
	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();
	return rkChrMgr.GetMainInstancePtr();
}

void CInstanceBase::__ClearMainInstance() const
{
	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.ClearMainInstance();
}

/* ½ÇÁ¦ ÇÃ·¹ÀÌ¾î Ä³¸¯ÅÍÀÎÁö Á¶»ç.*/
bool CInstanceBase::__IsMainInstance()
{
	if (this == __GetMainInstancePtr())
		return true;

	return false;
}

bool CInstanceBase::__IsExistMainInstance() const
{
	if (__GetMainInstancePtr())
		return true;
	return false;
}

bool CInstanceBase::__MainCanSeeHiddenThing() const
{
#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
	CInstanceBase * pInstance = __GetMainInstancePtr();
	return (pInstance) ? TRUE == pInstance->IsGameMaster() : false;
#else
	return false;
#endif
}

float CInstanceBase::__GetBowRange()
{
	float fRange = 2500.0f - 100.0f;

	if (__IsMainInstance())
	{
		IAbstractPlayer & rPlayer = IAbstractPlayer::GetSingleton();
		fRange += float(rPlayer.GetStatus(POINT_BOW_DISTANCE));
	}

	return fRange;
}

CInstanceBase * CInstanceBase::__FindInstancePtr(uint32_t dwVID) const
{
	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();
	return rkChrMgr.GetInstancePtr(dwVID);
}

bool CInstanceBase::__FindRaceType(uint32_t dwRace, uint8_t * pbType) const
{
	CPythonNonPlayer & rkNonPlayer = CPythonNonPlayer::Instance();
	return rkNonPlayer.GetInstanceType(dwRace, pbType);
}

bool CInstanceBase::Create(const SCreateData & c_rkCreateData)
{
	//IAbstractApplication::GetSingleton().SkipRenderBuffering(300);	//@fixme440

	SetInstanceType(c_rkCreateData.m_bType);

	if (!SetRace(c_rkCreateData.m_dwRace))
		return false;

	SetVirtualID(c_rkCreateData.m_dwVID);

#ifdef ENABLE_RENDER_TARGET_EFFECT
	if (c_rkCreateData.m_isRenderTarget)
		m_GraphicThingInstance.SetRenderTarget();
#endif

	SetInstanceFallState(CPythonNonPlayer::Instance().IsImmuneFlagByVnum(c_rkCreateData.m_dwRace, CPythonNonPlayer::IMMUNE_FALL));

	if (c_rkCreateData.m_isMain)
		SetMainInstance();

	if (IsGuildWall())
	{
		unsigned center_x;
		unsigned center_y;

		c_rkCreateData.m_kAffectFlags.ConvertToPosition(&center_x, &center_y);

		float center_z = __GetBackgroundHeight(center_x, center_y);
		NEW_SetPixelPosition(TPixelPosition(float(c_rkCreateData.m_lPosX), float(c_rkCreateData.m_lPosY), center_z));
	}
	else
		SCRIPT_SetPixelPosition(float(c_rkCreateData.m_lPosX), float(c_rkCreateData.m_lPosY));

	if (0 != c_rkCreateData.m_dwMountVnum)
		MountHorse(c_rkCreateData.m_dwMountVnum);

	SetArmor(c_rkCreateData.m_dwArmor);

	if (IsPC())
	{
		SetHair(c_rkCreateData.m_dwHair);
#ifdef ENABLE_REFINE_ELEMENT
		SetElementEffect(c_rkCreateData.m_dwElementsEffect);
#endif
		SetWeapon(c_rkCreateData.m_dwWeapon
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
			, c_rkCreateData.m_lGroupWeapon
#endif
		);
#ifdef ENABLE_QUIVER_SYSTEM
		SetArrow(c_rkCreateData.m_dwArrow);
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		SetAcce(c_rkCreateData.m_dwAcce);
#endif
#ifdef ENABLE_AURA_SYSTEM
		SetAura(c_rkCreateData.m_dwAura);
#endif
#ifdef ENABLE_PENDANT
		SetPendant(c_rkCreateData.m_dwPendant);
#endif
	}

	__Create_SetName(c_rkCreateData);

#ifdef ENABLE_SKILL_COLOR_SYSTEM
	ChangeSkillColor(*c_rkCreateData.m_dwSkillColor);
	memcpy(m_dwSkillColor, *c_rkCreateData.m_dwSkillColor, sizeof(m_dwSkillColor));
#endif

	m_dwLevel = c_rkCreateData.m_dwLevel;
#ifdef ENABLE_YOHARA_SYSTEM
	m_dwcLevel = c_rkCreateData.m_dwcLevel;
#endif
#ifdef WJ_SHOW_MOB_INFO
	m_dwAIFlag = c_rkCreateData.m_dwAIFlag;
#endif
	m_dwGuildID = c_rkCreateData.m_dwGuildID;
#ifdef ENABLE_SHOW_GUILD_LEADER
	m_dwNewIsGuildName = c_rkCreateData.m_dwNewIsGuildName;
#endif
	m_dwEmpireID = c_rkCreateData.m_dwEmpireID;
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
	m_lGroupWeapon = c_rkCreateData.m_lGroupWeapon;
#endif
	SetVirtualNumber(c_rkCreateData.m_dwRace);
	SetRotation(c_rkCreateData.m_fRot);

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	SetTitleID(c_rkCreateData.m_dwTitle);
#endif

	SetAlignment(c_rkCreateData.m_sAlignment);
	SetPKMode(c_rkCreateData.m_byPKMode);

	SetMoveSpeed(c_rkCreateData.m_dwMovSpd);
	SetAttackSpeed(c_rkCreateData.m_dwAtkSpd);
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	SetLanguage(c_rkCreateData.m_bLanguage);
#endif

#ifdef ENABLE_GRAPHIC_ON_OFF
	SetRank(CPythonNonPlayer::Instance().GetMonsterRank(c_rkCreateData.m_dwRace));
#endif

#ifdef ENABLE_SCALE_SYSTEM
	float fScale = 1.0f;

	uint8_t bScale = CPythonNonPlayer::Instance().GetMonsterScalePercent(c_rkCreateData.m_dwRace);
	if (bScale != 100)
		fScale = static_cast<float>(bScale / 100.0f);

#ifdef ENABLE_GROWTH_PET_SYSTEM
	if (c_rkCreateData.m_bType == CActorInstance::TYPE_PET)
		fScale = c_rkCreateData.m_dwLevel * 0.005f + 0.75f;
#endif

	/*if (IsPC()) {	//Player Scale Test
		float pScale = 2.0f;
		m_GraphicThingInstance.SetScaleWorld(pScale, pScale, pScale);
	}*/

	m_GraphicThingInstance.SetScaleWorld(fScale, fScale, fScale);
#endif

	if (!IsWearingDress())
	{
		m_GraphicThingInstance.SetAlphaValue(0.0f);
		m_GraphicThingInstance.BlendAlphaValue(1.0f, 0.5f);
	}

	if (!IsGuildWall())
		SetAffectFlagContainer(c_rkCreateData.m_kAffectFlags);

	// NOTE : ¹Ýµå½Ã Affect ¼ÂÆÃ ÈÄ¿¡ ÇØ¾ß ÇÔ
	AttachTextTail();
	RefreshTextTail();

	if (c_rkCreateData.m_dwStateFlags & ADD_CHARACTER_STATE_SPAWN)
	{
		if (IsAffect(AFFECT_SPAWN))
			__AttachEffect(EFFECT_SPAWN_APPEAR);

		if (IsPC())
			Refresh(CRaceMotionData::NAME_WAIT, true);
		else
			Refresh(CRaceMotionData::NAME_SPAWN, false);
	}
	else
		Refresh(CRaceMotionData::NAME_WAIT, true);

	__AttachEmpireEffect(c_rkCreateData.m_dwEmpireID);

	RegisterBoundingSphere();

	if (c_rkCreateData.m_dwStateFlags & ADD_CHARACTER_STATE_DEAD)
		m_GraphicThingInstance.DieEnd();

	SetStateFlags(c_rkCreateData.m_dwStateFlags);

	m_GraphicThingInstance.SetBattleHitEffect(ms_adwCRCAffectEffect[EFFECT_HIT]);

	if (!IsPC())
	{
		uint32_t dwBodyColor = CPythonNonPlayer::Instance().GetMonsterColor(c_rkCreateData.m_dwRace);
		if (0 != dwBodyColor)
		{
			SetModulateRenderMode();
			SetAddColor(dwBodyColor);
		}
	}

	__AttachHorseSaddle();

	// ±æµå ½Éº¼À» À§ÇÑ ÀÓ½Ã ÄÚµå, ÀûÁ¤ À§Ä¡¸¦ Ã£´Â Áß
	const int c_iGuildSymbolRace = 14200;
	if (c_iGuildSymbolRace == GetRace())
	{
		std::string strFileName = GetGuildSymbolFileName(m_dwGuildID);
		if (IsFile(strFileName.c_str()))
			m_GraphicThingInstance.ChangeMaterial(strFileName.c_str());
	}

#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
	if (IsAffect(AFFECT_INVISIBILITY) && __MainCanSeeHiddenThing())
		m_GraphicThingInstance.BlendAlphaValue(0.5f, 0.5f);
#endif

	return true;
}

#ifdef ENABLE_SKILL_COLOR_SYSTEM
void CInstanceBase::ChangeSkillColor(const uint32_t *dwSkillColor)
{
	uint32_t tmpdwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
	memcpy(tmpdwSkillColor, dwSkillColor, sizeof(tmpdwSkillColor));

	uint32_t skill[CRaceMotionData::SKILL_NUM][ESkillColorLength::MAX_EFFECT_COUNT];
	memset(skill, 0, sizeof(skill));

#ifdef ENABLE_WOLFMAN_CHARACTER
	for (int i = 0; i < CRaceData::RACE_WOLFMAN_M; ++i)
#else
	for (int i = 0; i < CRaceData::RACE_MAX_NUM; ++i)
#endif
	{
#ifdef ENABLE_NINETH_SKILL
		for (int t = 0; t < ESkillColorLength::MAX_NORMAL_SKILL_COUNT; ++t)
		{
			for (int x = 0; x < ESkillColorLength::MAX_EFFECT_COUNT; ++x)
			{
				skill[i * 10 + i * (ESkillColorLength::MAX_NORMAL_SKILL_COUNT - 1) + t + 1][x] = *(dwSkillColor++);
			}
		}
		dwSkillColor -= ESkillColorLength::MAX_NORMAL_SKILL_COUNT * ESkillColorLength::MAX_EFFECT_COUNT;
#else
		for (int t = 0; t < ESkillColorLength::MAX_SKILL_COUNT; ++t)
		{
			for (int x = 0; x < ESkillColorLength::MAX_EFFECT_COUNT; ++x)
			{
				skill[i * 10 + i * (ESkillColorLength::MAX_SKILL_COUNT - 1) + t + 1][x] = *(dwSkillColor++);
			}
		}
		dwSkillColor -= ESkillColorLength::MAX_SKILL_COUNT * ESkillColorLength::MAX_EFFECT_COUNT;
#endif
	}

#ifdef ENABLE_WOLFMAN_CHARACTER
#ifdef ENABLE_NINETH_SKILL
	for (int i = 170; i < 170 + ESkillColorLength::MAX_NORMAL_SKILL_COUNT; i++)
#else
	for (int i = 170; i < 170 + ESkillColorLength::MAX_SKILL_COUNT; i++)
#endif
	{
		for (int x = 0; x < ESkillColorLength::MAX_EFFECT_COUNT; ++x)
			skill[i][x] = tmpdwSkillColor[i - 170][x];
	}
#endif

#ifdef ENABLE_NINETH_SKILL
	for (int i = 176; i < 176 + 8; i++) // 8 = count of new skills
	{
		for (int x = 0; x < ESkillColorLength::MAX_EFFECT_COUNT; ++x)
			skill[i][x] = tmpdwSkillColor[BUFF_BEGIN - 1][x];
	}
#endif

	for (int i = BUFF_BEGIN; i < MAX_SKILL_COUNT + MAX_BUFF_COUNT; i++)
	{
		uint8_t id = 0;
		switch (i)
		{
			case BUFF_BEGIN+0:
				id = 94;
				break;
			case BUFF_BEGIN + 1:
				id = 95;
				break;
			case BUFF_BEGIN + 2:
				id = 96;
				break;
			case BUFF_BEGIN + 3:
				id = 110;
				break;
			case BUFF_BEGIN + 4:
				id = 111;
				break;
#ifdef ENABLE_NINETH_SKILL
			case BUFF_BEGIN + 5:
				id = 182;
				break;
#endif
#ifdef ENABLE_WOLFMAN_CHARACTER
			case BUFF_BEGIN + 6:
				id = 175;
				break;
#endif
			default:
				break;
		}

		if (id == 0)
			continue;

		for (int x = 0; x < ESkillColorLength::MAX_EFFECT_COUNT; ++x)
			skill[id][x] = tmpdwSkillColor[i][x];
	}

	m_GraphicThingInstance.ChangeSkillColor(*skill);
}
#endif

void CInstanceBase::__Create_SetName(const SCreateData & c_rkCreateData)
{
	if (IsGoto())
	{
		SetNameString("", 0);
		return;
	}
	if (IsWarp())
	{
		__Create_SetWarpName(c_rkCreateData);
		return;
	}
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (IsShop())
	{
		std::string newName = c_rkCreateData.m_stName;
		SetNameString(newName.c_str(), newName.length());
		return;
	}
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	if (IsNPC())
	{
		const char* c_szName;
		if (CPythonNonPlayer::Instance().GetName(c_rkCreateData.m_dwRace, &c_szName))
		{
			SetNameString(c_szName, strlen(c_szName));
			return;
		}

		SetNameString(c_rkCreateData.m_stName.c_str(), c_rkCreateData.m_stName.length());
		return;
	}
	else if (IsHorse())
	{
		std::string strHorseName = c_rkCreateData.m_stName;
		strHorseName += CPythonLocale::Instance().GetLocaleString(888); // "'s Horse"

		SetNameString(strHorseName.c_str(), strHorseName.length());
		return;
	}

	SetNameString(c_rkCreateData.m_stName.c_str(), c_rkCreateData.m_stName.length());
#else
	SetNameString(c_rkCreateData.m_stName.c_str(), c_rkCreateData.m_stName.length());
#endif
}

void CInstanceBase::__Create_SetWarpName(const SCreateData & c_rkCreateData)
{
	const char * c_szName;
	if (CPythonNonPlayer::Instance().GetName(c_rkCreateData.m_dwRace, &c_szName))
	{
		std::string strName = c_szName;
		int iFindingPos = strName.find_first_of(' ', 0);
		if (iFindingPos > 0)
			strName.resize(iFindingPos);

		SetNameString(strName.c_str(), strName.length());
	}
	else
		SetNameString(c_rkCreateData.m_stName.c_str(), c_rkCreateData.m_stName.length());
}

void CInstanceBase::SetNameString(const char * c_szName, int len)
{
	m_stName.assign(c_szName, len);
}

bool CInstanceBase::SetRace(uint32_t eRace)
{
	m_dwRace = eRace;

	if (!m_GraphicThingInstance.SetRace(eRace))
		return false;

	if (!__FindRaceType(m_dwRace, &m_eRaceType))
		m_eRaceType = CActorInstance::TYPE_PC;

	return true;
}

BOOL CInstanceBase::__IsChangableWeapon(int iWeaponID) const
{
	if (IsWearingDress())
	{
		const int c_iBouquets[] = {
			50201,	// Bouquet for Assassin
			50202,	// Bouquet for Shaman
			50203,
			50204,
			0,
		};

		for (int i = 0; c_iBouquets[i] != 0; ++i)
			if (iWeaponID == c_iBouquets[i])
				return true;

		return false;
	}

	return true;
}

BOOL CInstanceBase::IsWearingDress() const
{
	const int c_iWeddingDressShape = 201;
	return c_iWeddingDressShape == m_eShape;
}

BOOL CInstanceBase::IsHoldingPickAxe()
{
	const int c_iPickAxeStart = 29101;
	const int c_iPickAxeEnd = 29110;
	return m_awPart[CRaceData::PART_WEAPON] >= c_iPickAxeStart && m_awPart[CRaceData::PART_WEAPON] <= c_iPickAxeEnd;
}

BOOL CInstanceBase::IsNewMount() const
{
	return m_kHorse.IsNewMount();
}

BOOL CInstanceBase::IsMountingHorse() const
{
	return m_kHorse.IsMounting();
}

BOOL CInstanceBase::IsMountingHorseOnly()
{
	if (m_kHorse.IsMounting())
	{
		uint32_t dwMountVnum = m_kHorse.m_pkActor->GetRace();
		if ((dwMountVnum >= 20101 && dwMountVnum <= 20109) ||
			(dwMountVnum == 20029 || dwMountVnum == 20030))
			return TRUE;
	}

	return FALSE;
}

void CInstanceBase::MountHorse(uint32_t eRace)
{
	m_kHorse.Destroy();
	m_kHorse.Create(m_GraphicThingInstance.NEW_GetCurPixelPositionRef(), eRace, ms_adwCRCAffectEffect[EFFECT_HIT]);

	SetMotionMode(CRaceMotionData::MODE_HORSE);
	SetRotationSpeed(c_fDefaultHorseRotationSpeed);

	m_GraphicThingInstance.MountHorse(m_kHorse.GetActorPtr());
	m_GraphicThingInstance.Stop();
	m_GraphicThingInstance.RefreshActorInstance();
}

void CInstanceBase::DismountHorse()
{
	m_kHorse.Destroy();
}

void CInstanceBase::GetInfo(std::string * pstInfo)
{
	char szInfo[256];
	sprintf(szInfo, "Inst - UC %u, RC %u Pool - %u ", ms_dwUpdateCounter, ms_dwRenderCounter, ms_kPool.GetCapacity());

	pstInfo->append(szInfo);
}

void CInstanceBase::ResetPerformanceCounter()
{
	ms_dwUpdateCounter = 0;
	ms_dwRenderCounter = 0;
	ms_dwDeformCounter = 0;
}

bool CInstanceBase::NEW_IsLastPixelPosition()
{
	return m_GraphicThingInstance.IsPushing();
}

const TPixelPosition & CInstanceBase::NEW_GetLastPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetLastPixelPositionRef();
}

void CInstanceBase::NEW_SetDstPixelPositionZ(FLOAT z)
{
	m_GraphicThingInstance.NEW_SetDstPixelPositionZ(z);
}

void CInstanceBase::NEW_SetDstPixelPosition(const TPixelPosition & c_rkPPosDst)
{
	m_GraphicThingInstance.NEW_SetDstPixelPosition(c_rkPPosDst);
}

void CInstanceBase::NEW_SetSrcPixelPosition(const TPixelPosition & c_rkPPosSrc)
{
	m_GraphicThingInstance.NEW_SetSrcPixelPosition(c_rkPPosSrc);
}

const TPixelPosition & CInstanceBase::NEW_GetCurPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
}

const TPixelPosition & CInstanceBase::NEW_GetDstPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetDstPixelPositionRef();
}

const TPixelPosition & CInstanceBase::NEW_GetSrcPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetSrcPixelPositionRef();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void CInstanceBase::OnSyncing()
{
	m_GraphicThingInstance.__OnSyncing();
}

void CInstanceBase::OnWaiting()
{
	m_GraphicThingInstance.__OnWaiting();
}

void CInstanceBase::OnMoving()
{
	m_GraphicThingInstance.__OnMoving();
}

#ifdef ENABLE_SHOW_GUILD_LEADER
void CInstanceBase::ChangeGuild(uint32_t dwGuildID, uint32_t dwNewIsGuildName)
#else
void CInstanceBase::ChangeGuild(uint32_t dwGuildID)
#endif
{
	m_dwGuildID = dwGuildID;
#ifdef ENABLE_SHOW_GUILD_LEADER
	m_dwNewIsGuildName = dwNewIsGuildName;
#endif

	DetachTextTail();
	AttachTextTail();
	RefreshTextTail();
}

uint32_t CInstanceBase::GetPart(CRaceData::EParts part)
{
	assert(part >= 0 && part < CRaceData::PART_MAX_NUM);
	return m_awPart[part];
}

uint32_t CInstanceBase::GetShape() const
{
	return m_eShape;
}

bool CInstanceBase::CanAct()
{
	return m_GraphicThingInstance.CanAct();
}

bool CInstanceBase::CanMove()
{
	return m_GraphicThingInstance.CanMove();
}

bool CInstanceBase::CanUseSkill()
{
	if (IsPoly())
		return false;

	if (IsWearingDress())
		return false;

	if (IsHoldingPickAxe())
		return false;

	if (!m_kHorse.CanUseSkill())
		return false;

	if (!m_GraphicThingInstance.CanUseSkill())
		return false;

	return true;
}

bool CInstanceBase::CanAttack()
{
	if (!m_kHorse.CanAttack())
		return false;

	if (IsWearingDress())
		return false;

	if (IsHoldingPickAxe())
		return false;

	return m_GraphicThingInstance.CanAttack();
}

bool CInstanceBase::CanFishing()
{
	return m_GraphicThingInstance.CanFishing();
}


BOOL CInstanceBase::IsBowMode()
{
	return m_GraphicThingInstance.IsBowMode();
}

BOOL CInstanceBase::IsHandMode()
{
	return m_GraphicThingInstance.IsHandMode();
}

BOOL CInstanceBase::IsFishingMode()
{
	if (CRaceMotionData::MODE_FISHING == m_GraphicThingInstance.GetMotionMode())
		return true;

	return false;
}

BOOL CInstanceBase::IsFishing()
{
	return m_GraphicThingInstance.IsFishing();
}

BOOL CInstanceBase::IsDead()
{
	return m_GraphicThingInstance.IsDead();
}

BOOL CInstanceBase::IsStun()
{
	return m_GraphicThingInstance.IsStun();
}

BOOL CInstanceBase::IsSleep()
{
	return m_GraphicThingInstance.IsSleep();
}


BOOL CInstanceBase::__IsSyncing()
{
	return m_GraphicThingInstance.__IsSyncing();
}

void CInstanceBase::NEW_SetOwner(uint32_t dwVIDOwner)
{
	m_GraphicThingInstance.SetOwner(dwVIDOwner);
}

float CInstanceBase::GetLocalTime() const
{
	return m_GraphicThingInstance.GetLocalTime();
}

void CInstanceBase::PushUDPState(uint32_t dwCmdTime, const TPixelPosition & c_rkPPosDst, float fDstRot, uint32_t eFunc, uint32_t uArg) const
{
}

uint32_t ELTimer_GetServerFrameMSec();

void CInstanceBase::PushTCPStateExpanded(uint32_t dwCmdTime, const TPixelPosition & c_rkPPosDst, float fDstRot, uint32_t eFunc,
										 uint32_t uArg, uint32_t uTargetVID)
{
	SCommand kCmdNew;
	kCmdNew.m_kPPosDst = c_rkPPosDst;
	kCmdNew.m_dwChkTime = dwCmdTime + 100;
	kCmdNew.m_dwCmdTime = dwCmdTime;
	kCmdNew.m_fDstRot = fDstRot;
	kCmdNew.m_eFunc = eFunc;
	kCmdNew.m_uArg = uArg;
	kCmdNew.m_uTargetVID = uTargetVID;
	m_kQue_kCmdNew.emplace_back(kCmdNew);
}

void CInstanceBase::PushTCPState(uint32_t dwCmdTime, const TPixelPosition & c_rkPPosDst, float fDstRot, uint32_t eFunc, uint32_t uArg)
{
	if (__IsMainInstance())
	{
		TraceError("CInstanceBase::PushTCPState You can't send move packets to yourself!");
		return;
	}

	int nNetworkGap = ELTimer_GetServerFrameMSec() - dwCmdTime;

	m_nAverageNetworkGap = (m_nAverageNetworkGap * 70 + nNetworkGap * 30) / 100;

	SCommand kCmdNew;
	kCmdNew.m_kPPosDst = c_rkPPosDst;
	kCmdNew.m_dwChkTime = dwCmdTime + m_nAverageNetworkGap; //m_dwBaseChkTime + (dwCmdTime - m_dwBaseCmdTime);// + nNetworkGap;
	kCmdNew.m_dwCmdTime = dwCmdTime;
	kCmdNew.m_fDstRot = fDstRot;
	kCmdNew.m_eFunc = eFunc;
	kCmdNew.m_uArg = uArg;
	m_kQue_kCmdNew.emplace_back(kCmdNew);
}

BOOL CInstanceBase::__CanProcessNetworkStatePacket()
{
	if (m_GraphicThingInstance.IsDead())
		return FALSE;
	if (m_GraphicThingInstance.IsKnockDown())
		return FALSE;
	if (m_GraphicThingInstance.IsUsingSkill())
		if (!m_GraphicThingInstance.CanCancelSkill())
			return FALSE;

	return TRUE;
}

BOOL CInstanceBase::__IsEnableTCPProcess(uint32_t eCurFunc)
{
	if (m_GraphicThingInstance.IsActEmotion())
		return FALSE;

	if (!m_bEnableTCPState)
	{
		if (FUNC_EMOTION != eCurFunc)
			return FALSE;
	}

	return TRUE;
}

void CInstanceBase::StateProcess()
{
	while (true)
	{
		if (m_kQue_kCmdNew.empty())
			return;

		uint32_t dwDstChkTime = m_kQue_kCmdNew.front().m_dwChkTime;
		uint32_t dwCurChkTime = ELTimer_GetServerFrameMSec();

		if (dwCurChkTime < dwDstChkTime)
			return;

		SCommand kCmdTop = m_kQue_kCmdNew.front();
		m_kQue_kCmdNew.pop_front();

		TPixelPosition kPPosDst = kCmdTop.m_kPPosDst;
		FLOAT fRotDst = kCmdTop.m_fDstRot;
		uint32_t eFunc = kCmdTop.m_eFunc;
		uint32_t uArg = kCmdTop.m_uArg;
		uint32_t uVID = GetVirtualID();
		uint32_t uTargetVID = kCmdTop.m_uTargetVID;

		TPixelPosition kPPosCur;
		NEW_GetPixelPosition(&kPPosCur);

		TPixelPosition kPPosDir = kPPosDst - kPPosCur;
		auto fDirLen = sqrt(kPPosDir.x * kPPosDir.x + kPPosDir.y * kPPosDir.y);

		if (!__CanProcessNetworkStatePacket())
		{
			Lognf(0, "vid=%d Skip State as unable to process IsDead=%d, IsKnockDown=%d", uVID, m_GraphicThingInstance.IsDead(),
				  m_GraphicThingInstance.IsKnockDown());
			return;
		}

		if (!__IsEnableTCPProcess(eFunc))
			return;

		switch (eFunc)
		{
			case FUNC_WAIT:
			{
				if (fDirLen > 1.0f)
				{
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);

					__EnableSkipCollision();

					m_fDstRot = fRotDst;
					m_isGoing = TRUE;

					m_kMovAfterFunc.eFunc = FUNC_WAIT;

					if (!IsWalking())
						StartWalking();
				}
				else
				{
					m_isGoing = FALSE;

					if (!IsWaiting())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					SetAdvancingRotation(fRotDst);
					SetRotation(fRotDst);
				}
				break;
			}

			case FUNC_MOVE:
			{
#ifdef ENABLE_SECOND_GUILDRENEWAL_SYSTEM
				if (CPythonPlayer::Instance().GetParalysis())
					return;
#endif

				NEW_SetSrcPixelPosition(kPPosCur);
				NEW_SetDstPixelPosition(kPPosDst);
				m_fDstRot = fRotDst;
				m_isGoing = TRUE;
				__EnableSkipCollision();

				m_kMovAfterFunc.eFunc = FUNC_MOVE;

				if (!IsWalking())
					StartWalking();
				break;
			}

			case FUNC_COMBO:
			{
				if (fDirLen >= 50.0f)
				{
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot = fRotDst;
					m_isGoing = TRUE;
					__EnableSkipCollision();

					m_kMovAfterFunc.eFunc = FUNC_COMBO;
					m_kMovAfterFunc.uArg = uArg;

					if (!IsWalking())
						StartWalking();
				}
				else
				{
					m_isGoing = FALSE;

					if (IsWalking())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					RunComboAttack(fRotDst, uArg);
				}
				break;
			}

			case FUNC_ATTACK:
			{
				if (fDirLen >= 50.0f)
				{
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot = fRotDst;
					m_isGoing = TRUE;
					__EnableSkipCollision();

					m_kMovAfterFunc.eFunc = FUNC_ATTACK;

					if (!IsWalking())
						StartWalking();
				}
				else
				{
					m_isGoing = FALSE;

					if (IsWalking())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					BlendRotation(fRotDst);

					RunNormalAttack(fRotDst);
				}
				break;
			}

			case FUNC_MOB_SKILL:
			{
				if (fDirLen >= 50.0f)
				{
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot = fRotDst;
					m_isGoing = TRUE;
					__EnableSkipCollision();

					m_kMovAfterFunc.eFunc = FUNC_MOB_SKILL;
					m_kMovAfterFunc.uArg = uArg;

					if (!IsWalking())
						StartWalking();
				}
				else
				{
					m_isGoing = FALSE;

					if (IsWalking())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					BlendRotation(fRotDst);

					m_GraphicThingInstance.InterceptOnceMotion(CRaceMotionData::NAME_SPECIAL_1 + uArg);
				}
				break;
			}

			case FUNC_EMOTION:
			{
				if (fDirLen > 100.0f)
				{
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot = fRotDst;
					m_isGoing = TRUE;

					if (__IsMainInstance())
						__EnableSkipCollision();

					m_kMovAfterFunc.eFunc = FUNC_EMOTION;
					m_kMovAfterFunc.uArg = uArg;
					m_kMovAfterFunc.uArgExpanded = uTargetVID;
					m_kMovAfterFunc.kPosDst = kPPosDst;

					if (!IsWalking())
						StartWalking();
				}
				else
					__ProcessFunctionEmotion(uArg, uTargetVID, kPPosDst);
				break;
			}

			default:
			{
				if (eFunc & FUNC_SKILL)
				{
					if (fDirLen >= 50.0f)
					{
						NEW_SetSrcPixelPosition(kPPosCur);
						NEW_SetDstPixelPosition(kPPosDst);
						m_fDstRot = fRotDst;
						m_isGoing = TRUE;
						__EnableSkipCollision();

						m_kMovAfterFunc.eFunc = eFunc;
						m_kMovAfterFunc.uArg = uArg;

						if (!IsWalking())
							StartWalking();
					}
					else
					{
						m_isGoing = FALSE;

						if (IsWalking())
							EndWalking();

						SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
						SetAdvancingRotation(fRotDst);
						SetRotation(fRotDst);

						NEW_UseSkill(0, eFunc & 0x7f, uArg & 0x0f, (uArg >> 4) ? true : false);
					}
				}
				break;
			}
		}
	}
}


void CInstanceBase::MovementProcess()
{
	TPixelPosition kPPosCur;
	NEW_GetPixelPosition(&kPPosCur);

	// ·»´õ¸µ ÁÂÇ¥°èÀÌ¹Ç·Î y¸¦ -È­ÇØ¼­ ´õÇÑ´Ù.

	TPixelPosition kPPosNext;
	{
		const D3DXVECTOR3 & c_rkV3Mov = m_GraphicThingInstance.GetMovementVectorRef();

		kPPosNext.x = kPPosCur.x + (+c_rkV3Mov.x);
		kPPosNext.y = kPPosCur.y + (-c_rkV3Mov.y);
		kPPosNext.z = kPPosCur.z + (+c_rkV3Mov.z);
	}

	TPixelPosition kPPosDeltaSC = kPPosCur - NEW_GetSrcPixelPositionRef();
	TPixelPosition kPPosDeltaSN = kPPosNext - NEW_GetSrcPixelPositionRef();
	TPixelPosition kPPosDeltaSD = NEW_GetDstPixelPositionRef() - NEW_GetSrcPixelPositionRef();

	float fCurLen = sqrtf(kPPosDeltaSC.x * kPPosDeltaSC.x + kPPosDeltaSC.y * kPPosDeltaSC.y);
	float fNextLen = sqrtf(kPPosDeltaSN.x * kPPosDeltaSN.x + kPPosDeltaSN.y * kPPosDeltaSN.y);
	float fTotalLen = sqrtf(kPPosDeltaSD.x * kPPosDeltaSD.x + kPPosDeltaSD.y * kPPosDeltaSD.y);
	float fRestLen = fTotalLen - fCurLen;

	if (__IsMainInstance())
	{
		if (m_isGoing && IsWalking())
		{
			float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(NEW_GetSrcPixelPositionRef(), NEW_GetDstPixelPositionRef());

			SetAdvancingRotation(fDstRot);

			if (fRestLen <= 0.0)
			{
				if (IsWalking())
					EndWalking();

				//Tracen("¸ñÇ¥ µµ´Þ Á¤Áö");

				m_isGoing = FALSE;

				BlockMovement();

				if (FUNC_EMOTION == m_kMovAfterFunc.eFunc)
				{
					uint32_t dwMotionNumber = m_kMovAfterFunc.uArg;
					uint32_t dwTargetVID = m_kMovAfterFunc.uArgExpanded;
					__ProcessFunctionEmotion(dwMotionNumber, dwTargetVID, m_kMovAfterFunc.kPosDst);
					m_kMovAfterFunc.eFunc = FUNC_WAIT;
					return;
				}
			}
		}
	}
	else
	{
		if (m_isGoing && IsWalking())
		{
			float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(NEW_GetSrcPixelPositionRef(), NEW_GetDstPixelPositionRef());

			SetAdvancingRotation(fDstRot);

			// ¸¸¾à ·»ÅÏ½Ã°¡ ´Ê¾î ³Ê¹« ¸¹ÀÌ ÀÌµ¿Çß´Ù¸é..
			if (fRestLen < -100.0f)
			{
				NEW_SetSrcPixelPosition(kPPosCur);

				float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(kPPosCur, NEW_GetDstPixelPositionRef());
				SetAdvancingRotation(fDstRot);
				//Tracenf("VID %d ¿À¹ö ¹æÇâ¼³Á¤ (%f, %f) %f rest %f", GetVirtualID(), kPPosCur.x, kPPosCur.y, fDstRot, fRestLen);

				// ÀÌµ¿ÁßÀÌ¶ó¸é ´ÙÀ½¹ø¿¡ ¸ØÃß°Ô ÇÑ´Ù
				if (FUNC_MOVE == m_kMovAfterFunc.eFunc)
					m_kMovAfterFunc.eFunc = FUNC_WAIT;
			}
			// µµÂøÇß´Ù¸é...
			else if (fCurLen <= fTotalLen && fTotalLen <= fNextLen)
			{
				if (m_GraphicThingInstance.IsDead() || m_GraphicThingInstance.IsKnockDown())
				{
					__DisableSkipCollision();

					//Tracen("»ç¸Á »óÅÂ¶ó µ¿ÀÛ ½ºÅµ");

					m_isGoing = FALSE;

					//Tracen("Çàµ¿ ºÒ´É »óÅÂ¶ó ÀÌÈÄ µ¿ÀÛ ½ºÅµ");
				}
				else
				{
					switch (m_kMovAfterFunc.eFunc)
					{
						case FUNC_ATTACK:
						{
							if (IsWalking())
								EndWalking();

							__DisableSkipCollision();
							m_isGoing = FALSE;

							BlockMovement();
							SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
							SetAdvancingRotation(m_fDstRot);
							SetRotation(m_fDstRot);

							RunNormalAttack(m_fDstRot);
							break;
						}

						case FUNC_COMBO:
						{
							if (IsWalking())
								EndWalking();

							__DisableSkipCollision();
							m_isGoing = FALSE;

							BlockMovement();
							SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
							RunComboAttack(m_fDstRot, m_kMovAfterFunc.uArg);
							break;
						}

						case FUNC_EMOTION:
						{
							m_isGoing = FALSE;
							m_kMovAfterFunc.eFunc = FUNC_WAIT;
							__DisableSkipCollision();
							BlockMovement();

							uint32_t dwMotionNumber = m_kMovAfterFunc.uArg;
							uint32_t dwTargetVID = m_kMovAfterFunc.uArgExpanded;
							__ProcessFunctionEmotion(dwMotionNumber, dwTargetVID, m_kMovAfterFunc.kPosDst);
							break;
						}

						case FUNC_MOVE:
						{
							break;
						}

						case FUNC_MOB_SKILL:
						{
							if (IsWalking())
								EndWalking();

							__DisableSkipCollision();
							m_isGoing = FALSE;

							BlockMovement();
							SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
							SetAdvancingRotation(m_fDstRot);
							SetRotation(m_fDstRot);

							m_GraphicThingInstance.InterceptOnceMotion(CRaceMotionData::NAME_SPECIAL_1 + m_kMovAfterFunc.uArg);
							break;
						}

						default:
						{
							if (m_kMovAfterFunc.eFunc & FUNC_SKILL)
							{
								SetAdvancingRotation(m_fDstRot);
								BlendRotation(m_fDstRot);
								NEW_UseSkill(0, m_kMovAfterFunc.eFunc & 0x7f, m_kMovAfterFunc.uArg & 0x0f,
											 (m_kMovAfterFunc.uArg >> 4) ? true : false);
							}
							else
							{
								//Tracenf("VID %d ½ºÅ³ °ø°Ý (%f, %f) rot %f", GetVirtualID(), NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y, m_fDstRot);

								__DisableSkipCollision();
								m_isGoing = FALSE;

								BlockMovement();
								SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
								SetAdvancingRotation(m_fDstRot);
								BlendRotation(m_fDstRot);
								if (!IsWaiting())
									EndWalking();

								//Tracenf("VID %d Á¤Áö (%f, %f) rot %f IsWalking %d", GetVirtualID(), NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y, m_fDstRot, IsWalking());
							}
							break;
						}
					}
				}
			}
		}
	}

	if (IsWalking() || m_GraphicThingInstance.IsUsingMovingSkill())
	{
		float fRotation = m_GraphicThingInstance.GetRotation();
		float fAdvancingRotation = m_GraphicThingInstance.GetAdvancingRotation();
		int iDirection = GetRotatingDirection(fRotation, fAdvancingRotation);

		if (DEGREE_DIRECTION_SAME != m_iRotatingDirection)
		{
			if (DEGREE_DIRECTION_LEFT == iDirection)
				fRotation = fmodf(fRotation + m_fRotSpd * m_GraphicThingInstance.GetSecondElapsed(), 360.0f);
			else if (DEGREE_DIRECTION_RIGHT == iDirection)
				fRotation = fmodf(fRotation - m_fRotSpd * m_GraphicThingInstance.GetSecondElapsed() + 360.0f, 360.0f);

			if (m_iRotatingDirection != GetRotatingDirection(fRotation, fAdvancingRotation))
			{
				m_iRotatingDirection = DEGREE_DIRECTION_SAME;
				fRotation = fAdvancingRotation;
			}

			m_GraphicThingInstance.SetRotation(fRotation);
		}

		if (__IsInDustRange())
		{
			float fDustDistance = NEW_GetDistanceFromDestPixelPosition(m_kPPosDust);
			if (IsMountingHorse())
			{
				if (fDustDistance > ms_fHorseDustGap)
				{
					NEW_GetPixelPosition(&m_kPPosDust);
					__AttachEffect(EFFECT_HORSE_DUST);
				}
			}
			else
			{
				if (fDustDistance > ms_fDustGap)
				{
					NEW_GetPixelPosition(&m_kPPosDust);
					__AttachEffect(EFFECT_DUST);
				}
			}
		}
	}
}

void CInstanceBase::__ProcessFunctionEmotion(uint32_t dwMotionNumber, uint32_t dwTargetVID, const TPixelPosition & c_rkPosDst)
{
	if (IsWalking())
		EndWalkingWithoutBlending();

	__EnableChangingTCPState();
	SCRIPT_SetPixelPosition(c_rkPosDst.x, c_rkPosDst.y);

	CInstanceBase * pTargetInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwTargetVID);
	if (pTargetInstance)
	{
		pTargetInstance->__EnableChangingTCPState();

		if (pTargetInstance->IsWalking())
			pTargetInstance->EndWalkingWithoutBlending();

		uint16_t wMotionNumber1 = HIWORD(dwMotionNumber);
		uint16_t wMotionNumber2 = LOWORD(dwMotionNumber);

		int src_job = RaceToJob(GetRace());
		int dst_job = RaceToJob(pTargetInstance->GetRace());

		NEW_LookAtDestInstance(*pTargetInstance);
		m_GraphicThingInstance.InterceptOnceMotion(wMotionNumber1 + dst_job);
		m_GraphicThingInstance.SetRotation(m_GraphicThingInstance.GetTargetRotation());
		m_GraphicThingInstance.SetAdvancingRotation(m_GraphicThingInstance.GetTargetRotation());

		pTargetInstance->NEW_LookAtDestInstance(*this);
		pTargetInstance->m_GraphicThingInstance.InterceptOnceMotion(wMotionNumber2 + src_job);
		pTargetInstance->m_GraphicThingInstance.SetRotation(pTargetInstance->m_GraphicThingInstance.GetTargetRotation());
		pTargetInstance->m_GraphicThingInstance.SetAdvancingRotation(pTargetInstance->m_GraphicThingInstance.GetTargetRotation());

		if (pTargetInstance->__IsMainInstance())
		{
			IAbstractPlayer & rPlayer = IAbstractPlayer::GetSingleton();
			rPlayer.EndEmotionProcess();
		}
	}

	if (__IsMainInstance())
	{
		IAbstractPlayer & rPlayer = IAbstractPlayer::GetSingleton();
		rPlayer.EndEmotionProcess();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Update & Deform & Render

int g_iAccumulationTime = 0;

void CInstanceBase::Update()
{
	++ms_dwUpdateCounter;

	StateProcess();
	m_GraphicThingInstance.PhysicsProcess();
	m_GraphicThingInstance.RotationProcess();
	m_GraphicThingInstance.ComboProcess();
	m_GraphicThingInstance.AccumulationMovement();

	if (m_GraphicThingInstance.IsMovement())
	{
		TPixelPosition kPPosCur;
		NEW_GetPixelPosition(&kPPosCur);

		uint32_t dwCurTime = ELTimer_GetFrameMSec();
		//if (m_dwNextUpdateHeightTime<dwCurTime)
		{
			m_dwNextUpdateHeightTime = dwCurTime;
			kPPosCur.z = __GetBackgroundHeight(kPPosCur.x, kPPosCur.y);
			NEW_SetPixelPosition(kPPosCur);
		}

		// SetMaterialColor
		{
			uint32_t dwMtrlColor = __GetShadowMapColor(kPPosCur.x, kPPosCur.y);
			m_GraphicThingInstance.SetMaterialColor(dwMtrlColor);
		}
	}

	m_GraphicThingInstance.UpdateAdvancingPointInstance();

	AttackProcess();
	MovementProcess();

#ifdef ENABLE_GRAPHIC_ON_OFF
	if (!m_GraphicThingInstance.IsMainInstance() && HasPrivateShopSign())
	{
		m_GraphicThingInstance.SetDistanceFromMainCharacter(NEW_GetDistanceFromMainInstance());
	}

	if (IsPC())
		m_GraphicThingInstance.SetActorInvisibility(IsInvisibility());
#endif

	m_GraphicThingInstance.MotionProcess(IsPC());

#ifdef ENABLE_HIDE_WEAPON_WHEN_USE_EMOTION
	// Hide weapon: GetCurrentMotionIndex public version of the __GetCurrentMotionIndex in gamelib\\ActorInstance.h
	if (m_GraphicThingInstance.GetCurrentMotionIndex() < CRaceMotionData::NAME_CLAP || m_GraphicThingInstance.GetCurrentMotionIndex() == CRaceMotionData::NAME_DIG)
	{
		if (m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON) != m_awPart[CRaceData::PART_WEAPON])
		{
			m_GraphicThingInstance.AttachWeapon(m_awPart[CRaceData::PART_WEAPON]);
			CItemData* pItemData;
			if (CItemManager::Instance().GetItemDataPointer(m_awPart[CRaceData::PART_WEAPON], &pItemData))
				__GetRefinedEffect(pItemData);
		}
	}
	else if (m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON))
	{
		m_GraphicThingInstance.AttachWeapon(0);
		__ClearWeaponRefineEffect();
	}
	// EOF Hide weapon
#endif

	if (IsMountingHorse())
		m_kHorse.m_pkActor->HORSE_MotionProcess(FALSE);	//@custom008	FALSE -> TRUE

	if (IsAffect(AFFECT_INVISIBILITY))
		m_GraphicThingInstance.HideAllAttachingEffect();

	__ComboProcess();
	
	while(ProcessDamage());
}

void CInstanceBase::Transform()
{
	if (__IsSyncing())
	{
		//OnSyncing();
	}
	else
	{
		if (IsWalking() || m_GraphicThingInstance.IsUsingMovingSkill())
		{
			const D3DXVECTOR3 & c_rv3Movment = m_GraphicThingInstance.GetMovementVectorRef();

			float len = (c_rv3Movment.x * c_rv3Movment.x) + (c_rv3Movment.y * c_rv3Movment.y);
			if (len > 1.0f)
				OnMoving();
			else
				OnWaiting();
		}
	}

	m_GraphicThingInstance.INSTANCEBASE_Transform();
}


void CInstanceBase::Deform()
{
	// 2004.07.17.levites.isShow¸¦ ViewFrustumCheck·Î º¯°æ
	if (!__CanRender())
		return;

	++ms_dwDeformCounter;

	m_GraphicThingInstance.INSTANCEBASE_Deform();

	m_kHorse.Deform();
}

void CInstanceBase::RenderTrace()
{
	if (!__CanRender())
		return;

	m_GraphicThingInstance.RenderTrace();
}

void CInstanceBase::Render()
{
	// 2004.07.17.levites.isShow¸¦ ViewFrustumCheck·Î º¯°æ
	if (!__CanRender())
		return;

	++ms_dwRenderCounter;

	m_kHorse.Render();
	m_GraphicThingInstance.Render();

	if (CActorInstance::IsDirLine())
	{
		if (NEW_GetDstPixelPositionRef().x != 0.0f)
		{
			static CScreen s_kScreen;

			STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
			STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			STATEMANAGER.SaveRenderState(D3DRS_ZENABLE, FALSE);
			STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);
			STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

			TPixelPosition px;
			m_GraphicThingInstance.GetPixelPosition(&px);
			D3DXVECTOR3 kD3DVt3Cur(px.x, px.y, px.z);
			//D3DXVECTOR3 kD3DVt3Cur(NEW_GetSrcPixelPositionRef().x, -NEW_GetSrcPixelPositionRef().y, NEW_GetSrcPixelPositionRef().z);
			D3DXVECTOR3 kD3DVt3Dest(NEW_GetDstPixelPositionRef().x, -NEW_GetDstPixelPositionRef().y, NEW_GetDstPixelPositionRef().z);

			//printf("%s %f\n", GetNameString(), kD3DVt3Cur.y - kD3DVt3Dest.y);
			//float fdx = NEW_GetDstPixelPositionRef().x - NEW_GetSrcPixelPositionRef().x;
			//float fdy = NEW_GetDstPixelPositionRef().y - NEW_GetSrcPixelPositionRef().y;

			s_kScreen.SetDiffuseColor(0.0f, 0.0f, 1.0f);
			s_kScreen.RenderLine3d(kD3DVt3Cur.x, kD3DVt3Cur.y, px.z, kD3DVt3Dest.x, kD3DVt3Dest.y, px.z);
			STATEMANAGER.RestoreRenderState(D3DRS_ZENABLE);
			STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, TRUE);
			STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);
		}
	}
}

void CInstanceBase::RenderToShadowMap()
{
	if (IsDoor())
		return;

	if (IsBuilding())
		return;

	if (!__CanRender())
		return;

	if (!__IsExistMainInstance())
		return;

	CInstanceBase * pkInstMain = __GetMainInstancePtr();

#ifndef ENABLE_SHADOW_RENDER_QUALITY_OPTION
	const float SHADOW_APPLY_DISTANCE = 2500.0f;
#endif

	float fDistance = NEW_GetDistanceFromDestInstance(*pkInstMain);
#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
	float fDistanceMax = CPythonBackground::Instance().GetShadowDistance();
	if (fDistance >= fDistanceMax)
#else
	if (fDistance>=SHADOW_APPLY_DISTANCE)
#endif
		return;

	m_GraphicThingInstance.RenderToShadowMap();
}

void CInstanceBase::RenderCollision()
{
	m_GraphicThingInstance.RenderCollisionData();
}

void CInstanceBase::RenderCollisionNew()
{
	m_GraphicThingInstance.RenderCollisionDataNew();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Setting & Getting Data

void CInstanceBase::SetVirtualID(uint32_t dwVirtualID)
{
	m_GraphicThingInstance.SetVirtualID(dwVirtualID);
}

void CInstanceBase::SetVirtualNumber(uint32_t dwVirtualNumber)
{
	m_dwVirtualNumber = dwVirtualNumber;
}

void CInstanceBase::SetInstanceType(int iInstanceType)
{
	m_GraphicThingInstance.SetActorType(iInstanceType);
}

void CInstanceBase::SetInstanceFallState(bool bIsFall)
{
	m_GraphicThingInstance.SetActorFallState(bIsFall);
}

void CInstanceBase::SetAlignment(int16_t sAlignment)
{
	m_sAlignment = sAlignment;
	RefreshTextTailTitle();
}

void CInstanceBase::SetPKMode(uint8_t byPKMode)
{
	if (m_byPKMode == byPKMode)
		return;

	m_byPKMode = byPKMode;

	if (__IsMainInstance())
	{
		IAbstractPlayer & rPlayer = IAbstractPlayer::GetSingleton();
		rPlayer.NotifyChangePKMode();
	}
}

void CInstanceBase::SetKiller(bool bFlag)
{
	if (m_isKiller == bFlag)
		return;

	m_isKiller = bFlag;
	RefreshTextTail();
}

void CInstanceBase::SetPartyMemberFlag(bool bFlag)
{
	m_isPartyMember = bFlag;
}

void CInstanceBase::SetStateFlags(uint32_t dwStateFlags)
{
	if (dwStateFlags & ADD_CHARACTER_STATE_KILLER)
		SetKiller(TRUE);
	else
		SetKiller(FALSE);

	if (dwStateFlags & ADD_CHARACTER_STATE_PARTY)
		SetPartyMemberFlag(TRUE);
	else
		SetPartyMemberFlag(FALSE);
}

void CInstanceBase::SetComboType(uint32_t uComboType)
{
	m_GraphicThingInstance.SetComboType(uComboType);
}

const char * CInstanceBase::GetNameString() const
{
	return m_stName.c_str();
}

#ifdef ENABLE_LEVEL_IN_TRADE
uint32_t CInstanceBase::GetLevel() const
{
	return m_dwLevel;
}
#endif

#ifdef ENABLE_TEXT_LEVEL_REFRESH
void CInstanceBase::SetLevel(uint32_t dwLevel)
{
	m_dwLevel = dwLevel;
#ifdef WJ_SHOW_MOB_INFO
	if (dwLevel)
		UpdateTextTailLevel(dwLevel);
#endif
}
#endif

#if defined(ENABLE_YOHARA_SYSTEM) && defined(ENABLE_TEXT_LEVEL_REFRESH)
uint32_t CInstanceBase::GetConquerorLevel()
{
	return m_dwcLevel;
}

void CInstanceBase::SetConquerorLevel(uint32_t dwcLevel)
{
	m_dwcLevel = dwcLevel;
}
#endif

uint32_t CInstanceBase::GetRace() const
{
	return m_dwRace;
}


bool CInstanceBase::IsConflictAlignmentInstance(const CInstanceBase & rkInstVictim) const
{
	if (PK_MODE_PROTECT == rkInstVictim.GetPKMode())
		return false;

	switch (GetAlignmentType())
	{
		case ALIGNMENT_TYPE_NORMAL:
		case ALIGNMENT_TYPE_WHITE:
			if (ALIGNMENT_TYPE_DARK == rkInstVictim.GetAlignmentType())
				return true;
			break;
		case ALIGNMENT_TYPE_DARK:
			if (GetAlignmentType() != rkInstVictim.GetAlignmentType())
				return true;
			break;
	}

	return false;
}

void CInstanceBase::SetDuelMode(uint32_t type)
{
	m_dwDuelMode = type;
}

uint32_t CInstanceBase::GetDuelMode() const
{
	return m_dwDuelMode;
}

bool CInstanceBase::IsAttackableInstance(CInstanceBase & rkInstVictim)
{
	if (__IsMainInstance())
	{
		CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
		if (rkPlayer.IsObserverMode())
			return false;
	}

	if (GetVirtualID() == rkInstVictim.GetVirtualID())
		return false;

	if (IsStone())
	{
		if (rkInstVictim.IsPC())
			return true;
	}
	else if (IsPC())
	{
		if (rkInstVictim.IsStone())
			return true;

		if (rkInstVictim.IsPC())
		{
			if (GetDuelMode())
			{
				switch (GetDuelMode())
				{
				case DUEL_CANNOTATTACK:
					return false;
				case DUEL_START:
				{
					if (__FindDUELKey(GetVirtualID(), rkInstVictim.GetVirtualID()))
						return true;
					return false;
				}
				}
			}
			if (PK_MODE_GUILD == GetPKMode())
				if (GetGuildID() == rkInstVictim.GetGuildID())
					return false;

			if (rkInstVictim.IsKiller())
				if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
					return true;

			if (PK_MODE_PROTECT != GetPKMode())
			{
				if (PK_MODE_FREE == GetPKMode())
				{
					if (PK_MODE_PROTECT != rkInstVictim.GetPKMode())
						if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
							return true;
				}
				if (PK_MODE_GUILD == GetPKMode())
				{
					if (PK_MODE_PROTECT != rkInstVictim.GetPKMode())
						if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
							if (GetGuildID() != rkInstVictim.GetGuildID())
								return true;
				}
#ifdef ENABLE_BATTLE_FIELD
				if (PK_MODE_BATTLE == GetPKMode())
				{
					if (PK_MODE_PROTECT != rkInstVictim.GetPKMode() && rkInstVictim.GetPKMode() == PK_MODE_BATTLE)
						return true;
				}
#endif
			}

			if (IsSameEmpire(rkInstVictim))
			{
				if (IsPVPInstance(rkInstVictim))
					return true;

				if (PK_MODE_REVENGE == GetPKMode())
					if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
						if (IsConflictAlignmentInstance(rkInstVictim))
							return true;
			}
			else
				return true;
		}

		if (rkInstVictim.IsEnemy())
			return true;

		if (rkInstVictim.IsWoodenDoor())
			return true;
	}
	else if (IsEnemy())
	{
		if (rkInstVictim.IsPC())
			return true;

		if (rkInstVictim.IsBuilding())
			return true;
	}
	else if (IsPoly())
	{
		if (rkInstVictim.IsPC())
			return true;

		if (rkInstVictim.IsEnemy())
			return true;
	}
	return false;
}

bool CInstanceBase::IsTargetableInstance(CInstanceBase & rkInstVictim) const
{
	return rkInstVictim.CanPickInstance();
}

// 2004. 07. 07. [levites] - ½ºÅ³ »ç¿ëÁß Å¸°ÙÀÌ ¹Ù²î´Â ¹®Á¦ ÇØ°áÀ» À§ÇÑ ÄÚµå
bool CInstanceBase::CanChangeTarget()
{
	return m_GraphicThingInstance.CanChangeTarget();
}

// 2004.07.17.levites.isShow¸¦ ViewFrustumCheck·Î º¯°æ
bool CInstanceBase::CanPickInstance()
{
#ifdef ENABLE_GRAPHIC_ON_OFF
	if (!IsShowActor())
		return false;
#endif

	if (!__IsInViewFrustum())
		return false;

	if (IsDoor())
	{
		if (IsDead())
			return false;
	}

	if (IsPC())
	{
		if (IsAffect(AFFECT_EUNHYEONG))
		{
			if (!__MainCanSeeHiddenThing())
				return false;
		}
#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
		if (IsAffect(AFFECT_REVIVE_INVISIBILITY) && !__MainCanSeeHiddenThing())
			return false;
#else
		if (IsAffect(AFFECT_REVIVE_INVISIBILITY))
			return false;
#endif
#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
		if (IsAffect(AFFECT_INVISIBILITY) && !__MainCanSeeHiddenThing())
			return false;
#else
		if (IsAffect(AFFECT_INVISIBILITY))
			return false;
#endif
	}

	if (IsDead())
		return false;

	return true;
}

bool CInstanceBase::CanViewTargetHP(CInstanceBase & rkInstVictim) const
{
	if (rkInstVictim.IsStone())
		return true;
	if (rkInstVictim.IsWoodenDoor())
		return true;
	if (rkInstVictim.IsEnemy())
		return true;

	return false;
}

BOOL CInstanceBase::IsPoly()
{
	return m_GraphicThingInstance.IsPoly();
}

BOOL CInstanceBase::IsPC()
{
	return m_GraphicThingInstance.IsPC();
}

BOOL CInstanceBase::IsNPC()
{
	return m_GraphicThingInstance.IsNPC();
}

#ifdef ENABLE_GROWTH_PET_SYSTEM
BOOL CInstanceBase::IsGrowthPet()
{
	return m_GraphicThingInstance.IsGrowthPet();
}
#endif

BOOL CInstanceBase::IsEnemy()
{
	return m_GraphicThingInstance.IsEnemy();
}

BOOL CInstanceBase::IsStone()
{
	return m_GraphicThingInstance.IsStone();
}

#ifdef ENABLE_PROTO_RENEWAL
BOOL CInstanceBase::IsPetPay()
{
	return m_GraphicThingInstance.IsPetPay();
}

BOOL CInstanceBase::IsHorse()
{
	return m_GraphicThingInstance.IsHorse();
}
#endif

BOOL CInstanceBase::IsBoss()
{
	const CPythonNonPlayer::TMobTable* pkTab = CPythonNonPlayer::Instance().GetTable(GetRace());

	if (pkTab != nullptr) {
		if (pkTab->bRank >= CPythonNonPlayer::MOB_RANK_BOSS && pkTab->bType == CActorInstance::TYPE_ENEMY)
			return TRUE;
	}

	return FALSE;
}

BOOL CInstanceBase::IsGuildWall() const
{
	return IsWall(m_dwRace);
}

#ifdef ENABLE_BATTLE_FIELD
BOOL CInstanceBase::IsBattleFieldMap()
{
	return CPythonBackground::Instance().IsBattleFieldMap();
}
#endif

BOOL CInstanceBase::IsResource() const
{
	switch (m_dwVirtualNumber)
	{
		case 20047:
		case 20048:
		case 20049:
		case 20050:
		case 20051:
		case 20052:
		case 20053:
		case 20054:
		case 20055:
		case 20056:
		case 20057:
		case 20058:
		case 20059:
		case 30301:
		case 30302:
		case 30303:
		case 30304:
		case 30305:
		case 30306:
#ifdef ENABLE_YOHARA_SYSTEM
		case 30310:
		case 30311:
		case 30312:
		case 30313:
#endif
			return TRUE;
	}

	return FALSE;
}

BOOL CInstanceBase::IsWarp()
{
	return m_GraphicThingInstance.IsWarp();
}

BOOL CInstanceBase::IsGoto()
{
	return m_GraphicThingInstance.IsGoto();
}

BOOL CInstanceBase::IsObject()
{
	return m_GraphicThingInstance.IsObject();
}

BOOL CInstanceBase::IsShop()
{
	return m_GraphicThingInstance.IsShop();
}

BOOL CInstanceBase::IsBuilding()
{
	return m_GraphicThingInstance.IsBuilding();
}

BOOL CInstanceBase::IsDoor()
{
	return m_GraphicThingInstance.IsDoor();
}

BOOL CInstanceBase::IsWoodenDoor()
{
	if (m_GraphicThingInstance.IsDoor())
	{
		int vnum = GetVirtualNumber();
		if (vnum == 13000)
			return true;
		if (vnum >= 30111 && vnum <= 30119)
			return true;

		return false;
	}
	return false;
}

BOOL CInstanceBase::IsStoneDoor()
{
	return m_GraphicThingInstance.IsDoor() && 13001 == GetVirtualNumber();
}

BOOL CInstanceBase::IsFlag() const
{
	if (GetRace() == 20035)
		return TRUE;
	if (GetRace() == 20036)
		return TRUE;
	if (GetRace() == 20037)
		return TRUE;

	return FALSE;
}

#ifdef ENABLE_EXTENDED_CONFIGS
/*BOOL CInstanceBase::IsPet()
{
	if (GetRace() >= 34000 && GetRace() <= 34999)
		return true;
	return false;
}*/

BOOL CInstanceBase::IsMount()
{
	if (GetRace() >= 20110 && GetRace() <= 20299)
		return true;
	return false;
}
#endif

BOOL CInstanceBase::IsForceVisible()
{
	if (IsAffect(AFFECT_SHOW_ALWAYS))
		return TRUE;

	if (IsObject() || IsBuilding() || IsDoor())
		return TRUE;

	return FALSE;
}

#ifdef ENABLE_EVENT_BANNER_FLAG
BOOL CInstanceBase::IsBannerFlag()
{
	switch (GetRace())
	{
		case 20127: // Anniversary Banner 1
		case 20128: // Anniversary Banner 2
		case 20129: // Anniversary Banner 3
		case 20130: // Anniversary Banner 4
		case 20131: // Anniversary Banner 5
		case 20132: // Anniversary Banner 6
		case 20133: // Anniversary Banner 7
		case 20134: // Solar Banner
		case 20135: // Christmas Banner
		case 20138: // Ramadan Banner
		case 20142: // Halloween Banner
		case 20143: // Easter Banner
			return TRUE;
	}

	return FALSE;
}
#endif

int CInstanceBase::GetInstanceType() const
{
	return m_GraphicThingInstance.GetActorType();
}

uint32_t CInstanceBase::GetVirtualID()
{
	return m_GraphicThingInstance.GetVirtualID();
}

uint32_t CInstanceBase::GetVirtualNumber() const
{
	return m_dwVirtualNumber;
}

// 2004.07.17.levites.isShow¸¦ ViewFrustumCheck·Î º¯°æ
bool CInstanceBase::__IsInViewFrustum()
{
	return m_GraphicThingInstance.isShow();
}

bool CInstanceBase::__CanRender()
{
#ifdef ENABLE_RENDER_TARGET
	if (IsAlwaysRender())
		return true;
#endif

	if (!__IsInViewFrustum())
		return false;

#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
	if (IsAffect(AFFECT_INVISIBILITY) && !__MainCanSeeHiddenThing())
		return false;
#else
	if (IsAffect(AFFECT_INVISIBILITY))
		return false;
#endif

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Graphic Control

bool CInstanceBase::IntersectBoundingBox()
{
	float u, v, t;
	return m_GraphicThingInstance.Intersect(&u, &v, &t);
}

bool CInstanceBase::IntersectDefendingSphere()
{
	return m_GraphicThingInstance.IntersectDefendingSphere();
}

float CInstanceBase::GetDistance(CInstanceBase * pkTargetInst)
{
	TPixelPosition TargetPixelPosition;
	pkTargetInst->m_GraphicThingInstance.GetPixelPosition(&TargetPixelPosition);
	return GetDistance(TargetPixelPosition);
}

float CInstanceBase::GetDistance(const TPixelPosition & c_rPixelPosition)
{
	TPixelPosition PixelPosition;
	m_GraphicThingInstance.GetPixelPosition(&PixelPosition);

	float fdx = PixelPosition.x - c_rPixelPosition.x;
	float fdy = PixelPosition.y - c_rPixelPosition.y;

	return sqrtf((fdx * fdx) + (fdy * fdy));
}

CActorInstance & CInstanceBase::GetGraphicThingInstanceRef()
{
	return m_GraphicThingInstance;
}

CActorInstance * CInstanceBase::GetGraphicThingInstancePtr()
{
	return &m_GraphicThingInstance;
}

void CInstanceBase::RefreshActorInstance()
{
	m_GraphicThingInstance.RefreshActorInstance();
}

void CInstanceBase::Refresh(uint32_t dwMotIndex, bool isLoop)
{
	RefreshState(dwMotIndex, isLoop);
}

void CInstanceBase::RestoreRenderMode()
{
	m_GraphicThingInstance.RestoreRenderMode();
}

void CInstanceBase::SetAddRenderMode()
{
	m_GraphicThingInstance.SetAddRenderMode();
}

void CInstanceBase::SetModulateRenderMode()
{
	m_GraphicThingInstance.SetModulateRenderMode();
}

void CInstanceBase::SetRenderMode(int iRenderMode)
{
	m_GraphicThingInstance.SetRenderMode(iRenderMode);
}

void CInstanceBase::SetAddColor(const D3DXCOLOR & c_rColor)
{
	m_GraphicThingInstance.SetAddColor(c_rColor);
}

void CInstanceBase::__SetBlendRenderingMode()
{
	m_GraphicThingInstance.SetBlendRenderMode();
}

void CInstanceBase::__SetAlphaValue(float fAlpha)
{
	m_GraphicThingInstance.SetAlphaValue(fAlpha);
}

float CInstanceBase::__GetAlphaValue()
{
	return m_GraphicThingInstance.GetAlphaValue();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Part

void CInstanceBase::SetHair(uint32_t eHair)
{
	if (!IsPC() || IsPoly() || __IsShapeAnimalWear())
		return;

	if (!HAIR_COLOR_ENABLE)
		return;

	if (IsPC() == false)
		return;
	m_awPart[CRaceData::PART_HAIR] = eHair;
	m_GraphicThingInstance.SetHair(eHair);
}

void CInstanceBase::ChangeHair(uint32_t eHair)
{
	if (!HAIR_COLOR_ENABLE)
		return;

	if (IsPC() == false)
		return;

	if (GetPart(CRaceData::PART_HAIR) == eHair)
		return;

	SetHair(eHair);

	//int type = m_GraphicThingInstance.GetMotionMode();

	RefreshState(CRaceMotionData::NAME_WAIT, true);
	//RefreshState(type, true);
}

void CInstanceBase::SetArmor(uint32_t dwArmor)
{
	uint32_t dwShape;
	if (__ArmorVnumToShape(dwArmor, &dwShape))
	{
		CItemData * pItemData;
		if (CItemManager::Instance().GetItemDataPointer(dwArmor, &pItemData))
		{
			float fSpecularPower = pItemData->GetSpecularPowerf();
			SetShape(dwShape, fSpecularPower);
			__GetRefinedEffect(pItemData);
#ifdef ENABLE_SHINING_SYSTEM
			__GetShiningEffect(pItemData);
#endif
			return;
		}
		__ClearArmorRefineEffect();
#ifdef ENABLE_SHINING_SYSTEM
		__ClearShiningEffect(CItemData::ITEM_TYPE_ARMOR);
#endif
	}

	SetShape(dwArmor);
}

void CInstanceBase::SetShape(uint32_t eShape, float fSpecular)
{
	if (IsPoly())
		m_GraphicThingInstance.SetShape(0);
	else
		m_GraphicThingInstance.SetShape(eShape, fSpecular);

	m_eShape = eShape;
}

uint32_t CInstanceBase::GetWeaponType()
{
	uint32_t dwWeapon = GetPart(CRaceData::PART_WEAPON);
	CItemData * pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(dwWeapon, &pItemData))
		return CItemData::WEAPON_NONE;

	return pItemData->GetWeaponType();
}

/*
void CInstanceBase::SetParts(const uint16_t * c_pParts)
{
	if (IsPoly())
		return;

	if (__IsShapeAnimalWear())
		return;

	uint32_t eWeapon=c_pParts[CRaceData::PART_WEAPON];

	if (__IsChangableWeapon(eWeapon) == false)
			eWeapon = 0;

	if (eWeapon != m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON))
	{
		m_GraphicThingInstance.AttachPart(CRaceData::PART_MAIN, CRaceData::PART_WEAPON, eWeapon);
		m_awPart[CRaceData::PART_WEAPON] = eWeapon;
	}

	__AttachHorseSaddle();
}
*/

void CInstanceBase::__ClearWeaponRefineEffect()
{
	if (m_swordRefineEffectRight)
	{
		__DetachEffect(m_swordRefineEffectRight);
		m_swordRefineEffectRight = 0;
	}
	if (m_swordRefineEffectLeft)
	{
		__DetachEffect(m_swordRefineEffectLeft);
		m_swordRefineEffectLeft = 0;
	}
}

void CInstanceBase::__ClearArmorRefineEffect()
{
	if (m_armorRefineEffect[0])
	{
		__DetachEffect(m_armorRefineEffect[0]);
		m_armorRefineEffect[0] = 0;
	}

	if (m_armorRefineEffect[1])
	{
		__DetachEffect(m_armorRefineEffect[1]);
		m_armorRefineEffect[1] = 0;
	}
}

uint32_t CInstanceBase::__GetRefinedEffect(CItemData* pItem
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
	, long lGroupWeapon
#endif
)
{
#if defined(ENABLE_WEAPON_COSTUME_SYSTEM) && !defined(USE_WEAPON_COSTUME_WITH_EFFECT) && defined(ENABLE_WEAPON_COSTUME_RENEWAL)
	if (pItem->GetType() == CItemData::ITEM_TYPE_COSTUME && pItem->GetSubType() == CItemData::COSTUME_WEAPON) {
		uint32_t weapon_vnum = CPythonPlayer::Instance().GetItemIndex(TItemPos(EQUIPMENT, c_Equipment_Weapon));
		CItemData* pItemData;
		if (CItemManager::Instance().GetItemDataPointer(weapon_vnum, &pItemData)) {
			pItem = pItemData;
		}
	}
#endif

#ifdef ENABLE_SIMPLE_REFINED_EFFECT_CHECK
	uint32_t refine = pItem->GetRefine();
#else
#	ifdef ENABLE_PROTO_RENEWAL
	auto refine = std::max<size_t>(pItem->GetRefine() + pItem->GetSocketCount(), METIN_SOCKET_MAX_NUM) - METIN_SOCKET_MAX_NUM;
#	else
	auto refine = std::max<size_t>(pItem->GetRefine() + pItem->GetSocketCount(), CItemData::ITEM_SOCKET_MAX_NUM) - CItemData::ITEM_SOCKET_MAX_NUM;
#	endif
#endif
	uint32_t vnum = pItem->GetIndex();

	switch (pItem->GetType())
	{
		case CItemData::ITEM_TYPE_WEAPON:
		{
			__ClearWeaponRefineEffect();
#ifdef ENABLE_REFINE_ELEMENT
			AttachElementEffect(pItem);
#endif

			bool enable_refine = true;

			switch (pItem->GetSubType())
			{
				case CItemData::WEAPON_SWORD:
				case CItemData::WEAPON_TWO_HANDED:
				{
					bool default_effect = true;

#ifdef ENABLE_LVL96_WEAPON_EFFECT
					if ((330 <= vnum && vnum <= 335) ||		// Dunkeldrachenschwert
						(350 <= vnum && vnum <= 355) ||		// Dunkeldrachenschneide
						(3240 <= vnum && vnum <= 3245) ||	// Dunkeldrachenklinge

						(370 <= vnum && vnum <= 375) ||		// Schlangenschwert
						(390 <= vnum && vnum <= 395) ||		// Schlangenklinge
						(3260 <= vnum && vnum <= 3265))		// Schlangengroßschwert
					{
						default_effect = false;
						enable_refine = false;
						m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_REFINED7TH;
					}
#endif
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
					if (lGroupWeapon > 0 && pItem->IsFlag(CItemData::ITEM_FLAG_GROUP_DMG_WEAPON))
					{
						default_effect = false;
						m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_RARITY3;
					}
#endif

					if (default_effect)
					{
						m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_REFINED7 + refine - 7;
					}

					break;
				}//WEAPON_SWORD

				case CItemData::WEAPON_DAGGER:
				{
					bool default_effect = true;

#ifdef ENABLE_LVL96_WEAPON_EFFECT
					if ((1200 <= vnum && vnum <= 1205) ||	// Dunkeldrachendolch
						(1220 <= vnum && vnum <= 1225))		// Schlangendolch
					{
						default_effect = false;
						enable_refine = false;
						m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED7TH;
						m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED7TH_LEFT;
					}
#endif
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
					if (lGroupWeapon > 0 && pItem->IsFlag(CItemData::ITEM_FLAG_GROUP_DMG_WEAPON))
					{
						default_effect = false;
						m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY3;
						m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY3_LEFT;
					}
#endif

					if (default_effect)
					{
						m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED7 + refine - 7;
						m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED7_LEFT + refine - 7;
					}

					break;
				}//WEAPON_DAGGER

				case CItemData::WEAPON_BOW:
				{
					bool default_effect = true;

#ifdef ENABLE_LVL96_WEAPON_EFFECT
					if ((2220 <= vnum && vnum <= 2225) ||	// Dunkeldrachenbogen
						(2240 <= vnum && vnum <= 2245))		// Schlangenbogen
					{
						default_effect = false;
						enable_refine = false;
						m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_BOW_REFINED7TH;
					}
#endif
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
					if (lGroupWeapon > 0 && pItem->IsFlag(CItemData::ITEM_FLAG_GROUP_DMG_WEAPON))
					{
						default_effect = false;
						m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_BOW_RARITY3;
					}
#endif

					if (default_effect)
					{
						m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_BOW_REFINED7 + refine - 7;
					}

					break;
				}//WEAPON_BOW

				case CItemData::WEAPON_BELL:
				case CItemData::WEAPON_ARROW:
#ifdef ENABLE_QUIVER_SYSTEM
				case CItemData::WEAPON_QUIVER:
#endif
				{
					bool default_effect = true;

#ifdef ENABLE_LVL96_WEAPON_EFFECT
					if ((5180 <= vnum && vnum <= 5185) ||	// Dunkeldrachenglocke
						(5210 <= vnum && vnum <= 5215))		// Schlangenglocke
					{
						default_effect = false;
						enable_refine = false;
						m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED7TH;
					}
#endif
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
					if (lGroupWeapon > 0 && pItem->IsFlag(CItemData::ITEM_FLAG_GROUP_DMG_WEAPON))
					{
						default_effect = false;
						m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_RARITY3;
					}
#endif

					if (default_effect)
					{
						m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED7 + refine - 7;
					}

					break;
				}//WEAPON_TWO_HANDED | WEAPON_BELL

				case CItemData::WEAPON_FAN:
				{
					bool default_effect = true;

#ifdef ENABLE_LVL96_WEAPON_EFFECT
					if ((7320 <= vnum && vnum <= 7325) ||	// Dunkeldrachenfächer
						(7340 <= vnum && vnum <= 7345))		// Schlangenfächer
					{
						default_effect = false;
						enable_refine = false;
						m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_FANBELL_REFINED7TH;
					}
#endif
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
					if (lGroupWeapon > 0 && pItem->IsFlag(CItemData::ITEM_FLAG_GROUP_DMG_WEAPON))
					{
						default_effect = false;
						m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_FANBELL_RARITY3;
					}
#endif

					if (default_effect)
					{
						m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_FANBELL_REFINED7 + refine - 7;
					}

					break;
				}//WEAPON_FAN

#ifdef ENABLE_WOLFMAN_CHARACTER
				case CItemData::WEAPON_CLAW:	//8
				{
					bool default_effect = true;

#	ifdef ENABLE_LVL96_WEAPON_EFFECT
					if ((6140 <= vnum && vnum <= 6145) ||	// Dunkeldrachenkralle
						(6160 <= vnum && vnum <= 6165))		// Schlangenkralle
					{
						default_effect = false;
						enable_refine = false;
						m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_REFINED7TH_W;
						m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SWORD_REFINED7TH_W_LEFT;
					}
#	endif
#	ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
					//Groupdmg-Effect
#	endif

					if (default_effect)
					{
						m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_REFINED7_W + refine - 7;
						m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SWORD_REFINED7_W_LEFT + refine - 7;
					}

					break;
				}//WEAPON_CLAW
#endif

				default:
					TraceError("Weapon set Default!");
					m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_REFINED7 + refine - 7;
					break;
			}

			if ((refine < 7) && enable_refine)
				return 0;

			if (m_swordRefineEffectRight)
				m_swordRefineEffectRight = __AttachEffect(m_swordRefineEffectRight);
			if (m_swordRefineEffectLeft)
				m_swordRefineEffectLeft = __AttachEffect(m_swordRefineEffectLeft);

			break;
		}//END ITEM_TYPE_WEAPON

		case CItemData::ITEM_TYPE_ARMOR:
		{
			__ClearArmorRefineEffect();

			if (pItem->GetSubType() == CItemData::ARMOR_BODY)
			{
				bool default_effect = true;
				bool enable_refine = true;

	/*----------Bubble-Armor----------*/
				if ((12010 <= vnum && vnum <= 12019) ||	// Blaustahl-Panzer
					(12020 <= vnum && vnum <= 12029) ||	// Blauer Drachenanzug
					(12030 <= vnum && vnum <= 12039) ||	// Auraplattenpanzer
					(12040 <= vnum && vnum <= 12049)	// Kleidung des Drachen
#ifdef ENABLE_WOLFMAN_CHARACTER
					|| (21080 <= vnum && vnum <= 21089)	// Waldläuferpanzer
#endif
					)
				{
					default_effect = false;
					m_armorRefineEffect[0] = EFFECT_REFINED + EFFECT_BODYARMOR_SPECIAL;	// Bubbles-Effekt
					m_armorRefineEffect[1] = EFFECT_REFINED + EFFECT_BODYARMOR_SPECIAL2;	// Blue-Shining-Effekt
				}

#ifdef ENABLE_LVL96_ARMOR_EFFECT
	/*----------96-Armor----------*/
				if ((21220 <= vnum && vnum <= 21225) ||	// Dunkeldrachenrüstung		Warrior
					(21240 <= vnum && vnum <= 21245) ||	// Winddrachenrüstung		Assassin
					(21260 <= vnum && vnum <= 21265) ||	// Magiedrachenrüstung		Sura
					(21280 <= vnum && vnum <= 21285)	// Götterdrachenrüstung		Shaman
#	ifdef ENABLE_WOLFMAN_CHARACTER
					|| (21300 <= vnum && vnum <= 21305)	// Himmelsdrachenrüstung	Wolfman
#	endif
	/*----------120-Armor----------*/
					|| (21320 <= vnum && vnum <= 21325)	// Schlangenmantel	Warrior
					|| (21340 <= vnum && vnum <= 21345)	// Schlangenwams	Assassin
					|| (21360 <= vnum && vnum <= 21365)	// Boeser Schlangenmantel	Sura
					|| (21380 <= vnum && vnum <= 21385)	// Rein. Schlangenmantel	Shaman
#	ifdef ENABLE_WOLFMAN_CHARACTER
					|| (21400 <= vnum && vnum <= 21405)	// Him. Schlangenmantel	Wolfman
#	endif
					)
				{
					default_effect = false;
					enable_refine = false;
					m_armorRefineEffect[0] = EFFECT_REFINED + EFFECT_BODYARMOR_SPECIAL_TH;
				}
#endif

#ifdef ENABLE_LVL115_ARMOR_EFFECT
	/*----------Blitz[105-115]-Armor----------*/
				if (
					// Warrior
					(20760 <= vnum && vnum <= 20769) ||	// Polarlichtpanzer
					(20860 <= vnum && vnum <= 20869) ||	// Walknochenpanzer
					(20810 <= vnum && vnum <= 20819) ||	// Magmaglutpanzer
					(20910 <= vnum && vnum <= 20919) ||	// Ascheregenpanzer

					// Sura
					(20780 <= vnum && vnum <= 20789) ||	// Teufelskrallenpanzer
					(20880 <= vnum && vnum <= 20889) ||	// Irrlichterpanzer
					(20830 <= vnum && vnum <= 20839) ||	// Höllenrittpanzer
					(20930 <= vnum && vnum <= 20939) ||	// Schwarzfeuerpanzer

					// Assassin
					(20770 <= vnum && vnum <= 20779) ||	// Anzug des Hinterhalts
					(20870 <= vnum && vnum <= 20879) ||	// Verwandlungsanzug
					(20820 <= vnum && vnum <= 20829) ||	// Blutmondanzug
					(20920 <= vnum && vnum <= 20929) ||	// Todesnachtanzug

					// Shaman
					(20790 <= vnum && vnum <= 20799) ||	// Kleidung des Glaubens
					(20890 <= vnum && vnum <= 20899) ||	// Kleidung der Harmonie
					(20840 <= vnum && vnum <= 20849) ||	// Kleidung des Feuers
					(20940 <= vnum && vnum <= 20949)	// Kleidung des Mondes

#	ifdef ENABLE_WOLFMAN_CHARACTER
				// Wolfman
					|| (20800 <= vnum && vnum <= 20809)	// Schattenkämpferpanzer
					|| (20900 <= vnum && vnum <= 20909)	// Mondsteinplattenpanzer
					|| (20850 <= vnum && vnum <= 20859)	// Achatplattenpanzer
					|| (20950 <= vnum && vnum <= 20959)	// Onyxplattenpanzer
#	endif
					)
				{
					enable_refine = false;
					default_effect = false;
					m_armorRefineEffect[0] = EFFECT_REFINED + EFFECT_BODYARMOR_SPECIAL3;
					if (refine > 6)
						m_armorRefineEffect[1] = EFFECT_REFINED + EFFECT_BODYARMOR_REFINED7 + refine - 7;
				}
#endif

				if (default_effect)
				{
					m_armorRefineEffect[0] = EFFECT_REFINED + EFFECT_BODYARMOR_REFINED7 + refine - 7;
				}

				if ((refine < 7) && enable_refine)
					return 0;

				if (m_armorRefineEffect[0])
					m_armorRefineEffect[0] = __AttachEffect(m_armorRefineEffect[0]);
				if (m_armorRefineEffect[1])
					m_armorRefineEffect[1] = __AttachEffect(m_armorRefineEffect[1]);
			}

			break;
		}	//END ITEM_TYPE_ARMOR

#if defined(ENABLE_WEAPON_COSTUME_SYSTEM) && (defined(USE_WEAPON_COSTUME_WITH_EFFECT) || defined(USE_BODY_COSTUME_WITH_EFFECT)) && defined(ENABLE_WEAPON_COSTUME_RENEWAL)
		case CItemData::ITEM_TYPE_COSTUME:
		{
#	ifdef USE_WEAPON_COSTUME_WITH_EFFECT
			if (pItem->GetSubType() == CItemData::COSTUME_WEAPON)
			{
				__ClearWeaponRefineEffect();
#	ifdef ENABLE_REFINE_ELEMENT
				ClearElementEffect();
#	endif
				//int costumeWeaponEffect = 0;

				//switch(pItem->GetValue(3))
				//{
				//	case CItemData::WEAPON_DAGGER:
				//		m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_SMALLSWORD_REFINED9;
				//		m_swordRefineEffectLeft = EFFECT_REFINED+EFFECT_SMALLSWORD_REFINED9_LEFT;
				//		break;

				//	case CItemData::WEAPON_FAN:
				//		m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_FANBELL_REFINED9;
				//		break;

				//	case CItemData::WEAPON_ARROW:
				//	case CItemData::WEAPON_BELL:
				//		m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_SMALLSWORD_REFINED9;
				//		break;

				//	case CItemData::WEAPON_BOW:
				//		m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_BOW_REFINED9;
				//		break;

				//	default:
				//		m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_SWORD_REFINED9;
				//}

				if (m_swordRefineEffectRight)
					m_swordRefineEffectRight = __AttachEffect(m_swordRefineEffectRight);
				if (m_swordRefineEffectLeft)
					m_swordRefineEffectLeft = __AttachEffect(m_swordRefineEffectLeft);
			}
#	endif

#	ifdef USE_BODY_COSTUME_WITH_EFFECT
			if (pItem->GetSubType() == CItemData::COSTUME_BODY)
			{
				__ClearArmorRefineEffect();

				int costumeBodyEffect = 0;

				//if (41001 <= vnum && vnum <= 41002)			//Hasenkostüm
				//	costumeBodyEffect = EFFECT_COSTUMEBODY_YELLOW;
				//else if (41003 <= vnum && vnum <= 41004)	// Wüstenkämpfer
				//	costumeBodyEffect = EFFECT_COSTUMEBODY_ORANGE;
				//else if (41005 <= vnum && vnum <= 41012)	// Fechtanzug
				//	costumeBodyEffect = EFFECT_COSTUMEBODY_BLUE;
				//else if (41013 <= vnum && vnum <= 41020)	// Fußballtrikot
				//	costumeBodyEffect = EFFECT_COSTUMEBODY_RED;
				//else if (41021 <= vnum && vnum <= 41028)	// Box-Outfit
				//	costumeBodyEffect = EFFECT_COSTUMEBODY_GREEN;
				//else if (41472 <= vnum && vnum <= 41473)	//Zombieanzug
				//	costumeBodyEffect = EFFECT_COSTUMEBODY_VIOLETT;
				//else if (41496 <= vnum && vnum <= 41503)
				//	costumeBodyEffect = EFFECT_COSTUMEBODY_WHITE;

				m_armorRefineEffect[0] = EFFECT_REFINED + costumeBodyEffect;
				__AttachEffect(m_armorRefineEffect[0]);

			}
#	endif

			break;
		}	//END ITEM_TYPE_COSTUME
#endif
			
	}

	return 0;
}

bool CInstanceBase::SetWeapon(uint32_t eWeapon
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
	, long lGroupWeapon
#endif
)
{
	if (IsPoly())
		return false;

	if (__IsShapeAnimalWear())
		return false;

	if (__IsChangableWeapon(eWeapon) == false)
		eWeapon = 0;

	m_GraphicThingInstance.AttachWeapon(eWeapon);
	m_awPart[CRaceData::PART_WEAPON] = eWeapon;

	//Weapon Effect
	CItemData * pItemData;
	if (CItemManager::Instance().GetItemDataPointer(eWeapon, &pItemData))
	{
		__GetRefinedEffect(pItemData
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
			, lGroupWeapon
#endif
		);
#ifdef ENABLE_HEROIC_EFFECT
		__GetHeroicEffect(pItemData);
#endif
#ifdef ENABLE_SHINING_SYSTEM
		__GetShiningEffect(pItemData);
#endif
	}
	else
	{
		__ClearWeaponRefineEffect();
#ifdef ENABLE_HEROIC_EFFECT
		__ClearHeroicEffect();
#endif
#ifdef ENABLE_REFINE_ELEMENT
		ClearElementEffect();
#endif
#ifdef ENABLE_SHINING_SYSTEM
		__ClearShiningEffect(CItemData::ITEM_TYPE_WEAPON);
#endif
	}

	return true;
}

#ifdef ENABLE_QUIVER_SYSTEM
bool CInstanceBase::SetArrow(uint32_t eArrow)
{
	if (IsPoly())
		return false;

	if (__IsShapeAnimalWear())
		return false;

	if (eArrow)
	{
		CItemData * pItemData;
		if (!CItemManager::Instance().GetItemDataPointer(eArrow, &pItemData))
			return false;

		if (pItemData->GetType() == CItemData::ITEM_TYPE_WEAPON)
		{
			if (pItemData->GetSubType() == CItemData::WEAPON_ARROW)
			{
				m_GraphicThingInstance.SetQuiverEquipped(false);
				m_GraphicThingInstance.SetQuiverEffectID(0);
				return true;
			}

			if (pItemData->GetSubType() == CItemData::WEAPON_QUIVER)
			{
				m_GraphicThingInstance.SetQuiverEquipped(true);
				m_GraphicThingInstance.SetQuiverEffectID(pItemData->GetValue(0));
				return true;
			}
		}
	}

	m_GraphicThingInstance.SetQuiverEquipped(false);
	m_GraphicThingInstance.SetQuiverEffectID(0);
	return false;
}
#endif

#ifdef ENABLE_PENDANT
bool CInstanceBase::SetPendant(uint32_t ePendant)
{
	if (IsPoly() || __IsShapeAnimalWear())
		return false;

	if (!ePendant)
	{
		m_awPart[CRaceData::PART_PENDANT] = 0;
		return true;
	}

	CItemData* pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(ePendant, &pItemData)) {
		m_awPart[CRaceData::PART_PENDANT] = ePendant;
		return true;
	}

	m_awPart[CRaceData::PART_PENDANT] = ePendant;
	return true;
}
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
bool CInstanceBase::SetAcce(uint32_t eAcce)
{
	if (!IsPC() || IsPoly() || IsWearingDress() || __IsShapeAnimalWear())
		return false;

	m_GraphicThingInstance.AttachAcce(eAcce, 0, CRaceData::PART_ACCE);

	if (!eAcce)
	{
		__ClearAcceRefineEffect();
#	ifdef ENABLE_SHINING_SYSTEM
		__ClearShiningEffect(CItemData::ITEM_TYPE_COSTUME, CItemData::COSTUME_ACCE);
#	endif
#	ifdef ENABLE_SCALE_SYSTEM
		m_GraphicThingInstance.SetAcceScale(1.0f, 1.0f, 1.0f);
#	endif
		m_awPart[CRaceData::PART_ACCE] = 0;
		return true;
	}

	CItemData* pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(eAcce, &pItemData))
	{
#	ifdef ENABLE_SCALE_SYSTEM
		__ClearAcceRefineEffect();
#	ifdef ENABLE_SHINING_SYSTEM
		__ClearShiningEffect(CItemData::ITEM_TYPE_COSTUME, CItemData::COSTUME_ACCE);
#	endif
		m_GraphicThingInstance.SetAcceScale(1.0f, 1.0f, 1.0f);
		m_awPart[CRaceData::PART_ACCE] = 0;
#	endif
		return true;
	}

	if (m_acceRefineEffect)
		m_acceRefineEffect = __AttachEffect(EFFECT_ACCE_BACK);

#	ifdef ENABLE_SHINING_SYSTEM
	__GetShiningEffect(pItemData);
#	endif

#	ifdef ENABLE_SCALE_SYSTEM
	const uint8_t bRace = (uint8_t)GetRace();
	const uint8_t bJob = (uint8_t)RaceToJob(bRace);
	const uint8_t bSex = (uint8_t)RaceToSex(bRace);

	m_GraphicThingInstance.SetAcceScale(pItemData->GetItemScaleVector(bJob, bSex), bRace);
	m_awPart[CRaceData::PART_ACCE] = eAcce;
#	endif
	return true;
}

void CInstanceBase::ChangeAcce(uint32_t eAcce)
{
	if (!IsPC())
		return;

	if (eAcce == m_GraphicThingInstance.GetPartItemID(CRaceData::PART_ACCE))
		return;

	if (SetAcce(eAcce))
		RefreshState(CRaceMotionData::NAME_WAIT, true);
}

void CInstanceBase::__ClearAcceRefineEffect()
{
	if (!m_acceRefineEffect)
		return;

	__DetachEffect(m_acceRefineEffect);
	m_acceRefineEffect = 0;
}
#endif

#ifdef ENABLE_AURA_SYSTEM
void CInstanceBase::ChangeAura(uint32_t eAura)
{
	if (m_GraphicThingInstance.GetPartItemID(CRaceData::PART_AURA) != eAura)
		SetAura(eAura);
}

bool CInstanceBase::SetAura(uint32_t eAura)
{
	if (!IsPC() || IsPoly() || __IsShapeAnimalWear())
		return false;

	m_GraphicThingInstance.ChangePart(CRaceData::PART_AURA, eAura);
	if (!eAura)
	{
		if (m_auraRefineEffect)
		{
			__DetachEffect(m_auraRefineEffect);
			m_auraRefineEffect = 0;
		}
		m_awPart[CRaceData::PART_AURA] = 0;
		return true;
	}

	CItemData* pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(eAura, &pItemData))
	{
		if (m_auraRefineEffect)
		{
			__DetachEffect(m_auraRefineEffect);
			m_auraRefineEffect = 0;
		}
		m_awPart[CRaceData::PART_AURA] = 0;
		return true;
	}

#ifdef ENABLE_SCALE_SYSTEM
	uint8_t byRace = (uint8_t)GetRace();
	uint8_t byJob = (uint8_t)RaceToJob(byRace);
	uint8_t bySex = (uint8_t)RaceToSex(byRace);

	D3DXVECTOR3 v3MeshScale = pItemData->GetItemScaleVector(byJob, bySex);
	float fParticleScale = pItemData->GetItemParticleScale(byJob, bySex);
	m_auraRefineEffect = m_GraphicThingInstance.AttachEffectByID(0, "Bip01 Spine2", pItemData->GetAuraEffectID(), nullptr, fParticleScale, &v3MeshScale);
#else
	m_auraRefineEffect = m_GraphicThingInstance.AttachEffectByID(nullptr, "Bip01 Spine2", pItemData->GetAuraEffectID(), nullptr);
#endif
	m_awPart[CRaceData::PART_AURA] = eAura;
	return true;
}
#endif

void CInstanceBase::ChangeWeapon(uint32_t eWeapon
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
	, long lGroupWeapon
#endif
)
{
#if defined(ENABLE_WEAPON_COSTUME_SYSTEM) && !defined(USE_WEAPON_COSTUME_WITH_EFFECT) && defined(ENABLE_WEAPON_COSTUME_RENEWAL)
	const uint32_t weapon_vnum = m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON);
	CItemData* pItemData;
	bool itemgood = CItemManager::Instance().GetItemDataPointer(weapon_vnum, &pItemData);
	if (eWeapon == m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON) && (itemgood && pItemData->GetType() != CItemData::ITEM_TYPE_COSTUME && pItemData->GetSubType() != CItemData::COSTUME_WEAPON))
		return;
#else
	if (eWeapon == m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON))
		return;
#endif

	if (SetWeapon(eWeapon
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
		, lGroupWeapon
#endif
	))
		RefreshState(CRaceMotionData::NAME_WAIT, true);
}

#ifdef ENABLE_PENDANT
void CInstanceBase::ChangePendant(uint32_t ePendant)
{
	if (!IsPC())
		return;

	if (ePendant == m_GraphicThingInstance.GetPartItemID(CRaceData::PART_PENDANT))
		return;

	SetPendant(ePendant);
}
#endif

bool CInstanceBase::ChangeArmor(uint32_t dwArmor)
{
	uint32_t eShape;
	__ArmorVnumToShape(dwArmor, &eShape);

	if (GetShape() == eShape)
		return false;

	CAffectFlagContainer kAffectFlagContainer;
	kAffectFlagContainer.CopyInstance(m_kAffectFlagContainer);

	uint32_t dwVID = GetVirtualID();
	uint32_t dwRace = GetRace();
	uint32_t eHair = GetPart(CRaceData::PART_HAIR);
	uint32_t eAcce = GetPart(CRaceData::PART_ACCE);	//ENABLE_ACCE_COSTUME_SYSTEM
#ifdef ENABLE_PENDANT
	uint32_t ePendant = GetPart(CRaceData::PART_PENDANT);
#endif
	uint32_t eAura = GetPart(CRaceData::PART_AURA);	//ENABLE_AURA_SYSTEM
	uint32_t eWeapon = GetPart(CRaceData::PART_WEAPON);
	float fRot = GetRotation();
	float fAdvRot = GetAdvancingRotation();

	if (IsWalking())
		EndWalking();

	// 2004.07.25.myevan.ÀÌÆåÆ® ¾È ºÙ´Â ¹®Á¦
	//////////////////////////////////////////////////////
	__ClearAffects();
	//////////////////////////////////////////////////////

	if (!SetRace(dwRace))
	{
		TraceError("CPythonCharacterManager::ChangeArmor - SetRace VID[%d] Race[%d] ERROR", dwVID, dwRace);
		return false;
	}

	SetArmor(dwArmor);
	SetHair(eHair);
	SetWeapon(eWeapon
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
	, m_lGroupWeapon
#endif
	);
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	SetAcce(eAcce);
#endif
#ifdef ENABLE_AURA_SYSTEM
	SetAura(eAura);
#endif
#ifdef ENABLE_PENDANT
	SetPendant(ePendant);
#endif

	SetRotation(fRot);
	SetAdvancingRotation(fAdvRot);

	__AttachHorseSaddle();

	RefreshState(CRaceMotionData::NAME_WAIT, TRUE);

	// 2004.07.25.myevan.ÀÌÆåÆ® ¾È ºÙ´Â ¹®Á¦
	/////////////////////////////////////////////////
	SetAffectFlagContainer(kAffectFlagContainer);
	/////////////////////////////////////////////////

	CActorInstance::IEventHandler & rkEventHandler = GetEventHandlerRef();
	rkEventHandler.OnChangeShape();

	return true;
}

#ifdef ENABLE_HEROIC_EFFECT
void CInstanceBase::__GetHeroicEffect(CItemData* pItem)
{
	if (!pItem)
	{
		TraceError("no item data");
		return;
	}

	bool twoSidedWeapon = pItem->GetSubType() == CItemData::WEAPON_DAGGER || (IsMountingHorse() && pItem->GetSubType() == CItemData::WEAPON_FAN)
#ifdef ENABLE_WOLFMAN_CHARACTER
		|| pItem->GetSubType() == CItemData::WEAPON_CLAW
#endif
		;

	if (IsAffect(AFFECT_INVISIBILITY))
		return;

	bool check = false;

	if (twoSidedWeapon)
	{
		const char* c_szName;
		if (pItem->GetIndex() == 97003) {	//Dolch
			check = true;
			c_szName = "d:/ymir work/effect/mehok/da_1.mse";
		}
		else if (pItem->GetIndex() == 97007) {	//Kralle
			check = true;
			c_szName = "d:/ymir work/effect/mehok/cw_1.mse";
		}
		else if (pItem->GetIndex() == 97008) {	//Fächer
			check = true;
			c_szName = "d:/ymir work/effect/mehok/fa_1.mse";
		}

		if (check)
		{
			CEffectManager::Instance().RegisterEffect(c_szName, false, false);

			const char* c_szLeftBoneName;
			m_GraphicThingInstance.GetAttachingBoneName(CRaceData::PART_WEAPON_LEFT, &c_szLeftBoneName);
			if (strcmp(c_szLeftBoneName, ""))
				m_heroicEffectsLeft = m_GraphicThingInstance.AttachEffectByName(0, c_szLeftBoneName, c_szName);

			const char* c_szRightBoneName;
			m_GraphicThingInstance.GetAttachingBoneName(CRaceData::PART_WEAPON, &c_szRightBoneName);
			if (strcmp(c_szRightBoneName, ""))
				m_heroicEffectsRight = m_GraphicThingInstance.AttachEffectByName(0, c_szRightBoneName, c_szName);
		}
	}
	else
	{
		const char* c_szName;
		if (pItem->GetIndex() == 97001) {	//Sword
			check = true;
			c_szName = "d:/ymir work/effect/mehok/sw_1.mse";
		}
		else if (pItem->GetIndex() == 97002) {	//Sura-Sword
			check = true;
			c_szName = "d:/ymir work/effect/mehok/su_1.mse";
		}
		/*else if (pItem->GetIndex() == 97004)	//Bogen
			c_szName = "d:/ymir work/effect/mehok/.mse";*/
		else if (pItem->GetIndex() == 97005) {	//2Hand
			check = true;
			c_szName = "d:/ymir work/effect/mehok/sp_1.mse";
		}
		/*else if (pItem->GetIndex() == 97006)	//Glocke
			c_szName = "d:/ymir work/effect/mehok/.mse";*/
		else if (pItem->GetIndex() == 97008) {	//Fächer
			check = true;
			c_szName = "d:/ymir work/effect/mehok/fa_1.mse";
		}

		if (check)
		{
			CEffectManager::Instance().RegisterEffect(c_szName, false, false);

			const char* c_szRightBoneName;
			m_GraphicThingInstance.GetAttachingBoneName(CRaceData::PART_WEAPON, &c_szRightBoneName);
			if (strcmp(c_szRightBoneName, ""))
				m_heroicEffectsRight = m_GraphicThingInstance.AttachEffectByName(0, c_szRightBoneName, c_szName);
		}
	}
}

void CInstanceBase::__ClearHeroicEffect()
{
	if (m_heroicEffectsRight)
	{
		__DetachEffect(m_heroicEffectsRight);
		m_heroicEffectsRight = 0;
	}
	if (m_heroicEffectsLeft)
	{
		__DetachEffect(m_heroicEffectsLeft);
		m_heroicEffectsLeft = 0;
	}
}
#endif

#ifdef ENABLE_SHINING_SYSTEM
//This method checks if the item has shinings and will attach them.
//Weapons are heaving special checks for two sided weapons like daggers or fans while mounting.
//Every itemtype is supported. If no explicit logic is implemented it will be attached to "Bip01".
void CInstanceBase::__GetShiningEffect(CItemData* pItem)
{
	if (!pItem)
		return;

	//Set this to true if you want to hide the refine effect if there's a shining for this item.
	const bool removeRefineEffect = false;
	CItemData::TItemShiningTable shiningTable = pItem->GetItemShiningTable();

	// Weapon //
	if (pItem->GetType() == CItemData::ITEM_TYPE_WEAPON
#if defined(ENABLE_COSTUME_SYSTEM) && defined(ENABLE_WEAPON_COSTUME_SYSTEM)
		|| (pItem->GetType() == CItemData::ITEM_TYPE_COSTUME && pItem->GetSubType() == CItemData::COSTUME_WEAPON)
#endif
		)
	{
#if defined(ENABLE_COSTUME_SYSTEM) && defined(ENABLE_WEAPON_COSTUME_SYSTEM)
		const uint8_t bSubType = pItem->GetType() == CItemData::ITEM_TYPE_COSTUME && pItem->GetSubType() == CItemData::COSTUME_WEAPON ? pItem->GetValue(3) : pItem->GetSubType();
#else
		const uint8_t bSubType = pItem->GetSubType();
#endif

		__ClearShiningEffect(CItemData::ITEM_TYPE_WEAPON);

		if (shiningTable.Any() && removeRefineEffect)
			__ClearWeaponRefineEffect();

		for (int i = 0; i < CItemData::ITEM_SHINING_MAX_COUNT; i++)
		{
			if (strcmp(shiningTable.szShinings[i], ""))
			{
				if (bSubType == CItemData::WEAPON_BOW)
					__AttachShiningEffect(i, shiningTable.szShinings[i], "PART_WEAPON_LEFT", CItemData::ITEM_TYPE_WEAPON);
				else
				{
					//Check for double sided weapons or fan which is attached on both sides when mounted.
#ifdef ENABLE_WOLFMAN_CHARACTER
					bool twoSidedWeapon = bSubType == CItemData::WEAPON_DAGGER || (IsMountingHorse() && bSubType == CItemData::WEAPON_FAN) || bSubType == CItemData::WEAPON_CLAW;
#else
					bool twoSidedWeapon = bSubType == CItemData::WEAPON_DAGGER || (IsMountingHorse() && bSubType == CItemData::WEAPON_FAN);
#endif
					if (twoSidedWeapon)
						__AttachShiningEffect(i, shiningTable.szShinings[i], "PART_WEAPON_LEFT", CItemData::ITEM_TYPE_WEAPON);

					__AttachShiningEffect(i, shiningTable.szShinings[i], "PART_WEAPON", CItemData::ITEM_TYPE_WEAPON);
				}
			}
		}
	}

	// Armor //
#ifdef ENABLE_COSTUME_SYSTEM
	if ((pItem->GetType() == CItemData::ITEM_TYPE_ARMOR && pItem->GetSubType() == CItemData::ARMOR_BODY) || (pItem->GetType() == CItemData::ITEM_TYPE_COSTUME && pItem->GetSubType() == CItemData::COSTUME_BODY))
#else
	if (pItem->GetType() == CItemData::ITEM_TYPE_ARMOR && pItem->GetSubType() == CItemData::ARMOR_BODY)
#endif
	{
		__ClearShiningEffect(CItemData::ITEM_TYPE_ARMOR);

		if (shiningTable.Any() && removeRefineEffect)
			__ClearArmorRefineEffect();

		for (int i = 0; i < CItemData::ITEM_SHINING_MAX_COUNT; i++)
		{
			if (strcmp(shiningTable.szShinings[i], ""))
				__AttachShiningEffect(i, shiningTable.szShinings[i], "Bip01", CItemData::ITEM_TYPE_ARMOR);
		}
	}

# ifdef ENABLE_ACCE_COSTUME_SYSTEM
	// Acce //
	if (pItem->GetSubType() == CItemData::COSTUME_ACCE)
	{
		__ClearShiningEffect(CItemData::ITEM_TYPE_COSTUME, CItemData::COSTUME_ACCE);

		if (shiningTable.Any() && removeRefineEffect)
			__ClearAcceRefineEffect();

		for (int i = 0; i < CItemData::ITEM_SHINING_MAX_COUNT; i++)
		{
			if (strcmp(shiningTable.szShinings[i], ""))
				__AttachShiningEffect(i, shiningTable.szShinings[i], "Bip01", CItemData::ITEM_TYPE_COSTUME, CItemData::COSTUME_ACCE);
		}
	}
# endif
}

//Attaching the shining effect. 
//If boneName is not given "Bip01" is used as boneName.
void CInstanceBase::__AttachShiningEffect(int effectIndex, const char* effectFileName, const char* boneName, uint8_t bType, uint8_t sub)
{
	if (IsAffect(AFFECT_INVISIBILITY))
		return;

	if (effectIndex >= CItemData::ITEM_SHINING_MAX_COUNT)
		return;

	switch (bType)
	{
		case CItemData::ITEM_TYPE_WEAPON:
		{ //BoneName can be "PART_WEAPON" or "PART_WEAPON_LEFT" to get the attaching bone name dynamically.
			CEffectManager::Instance().RegisterEffect(effectFileName, false, false);

			if (!strcmp(boneName, "PART_WEAPON"))
			{
				const char* c_szRightBoneName;
				m_GraphicThingInstance.GetAttachingBoneName(CRaceData::PART_WEAPON, &c_szRightBoneName);

				if (strcmp(c_szRightBoneName, ""))
					m_weaponShiningEffects[0][effectIndex] = m_GraphicThingInstance.AttachEffectByName(0, c_szRightBoneName, effectFileName);
			}
			else if (!strcmp(boneName, "PART_WEAPON_LEFT"))
			{
				const char* c_szLeftBoneName;
				m_GraphicThingInstance.GetAttachingBoneName(CRaceData::PART_WEAPON_LEFT, &c_szLeftBoneName);

				if (strcmp(c_szLeftBoneName, ""))
					m_weaponShiningEffects[1][effectIndex] = m_GraphicThingInstance.AttachEffectByName(0, c_szLeftBoneName, effectFileName);
			}
		}
		break;

		case CItemData::ITEM_TYPE_ARMOR:
		{ //Attaching the armor shining effect. 
			if (!strcmp(boneName, ""))
			{
				Tracef("Empty bone name for attaching armor shining. Effect Index: %i, EffectFileName: %s", effectIndex, effectFileName);
				return;
			}

			CEffectManager::Instance().RegisterEffect(effectFileName, false, false);
			m_armorShiningEffects[effectIndex] = m_GraphicThingInstance.AttachEffectByName(0, boneName, effectFileName);
		}
		break;

		case CItemData::ITEM_TYPE_COSTUME:
		{
# ifdef ENABLE_ACCE_COSTUME_SYSTEM
			if (sub == CItemData::COSTUME_ACCE)
			{ //Attaching the acce shining effect.
				if (!strcmp(boneName, ""))
				{
					Tracef("Empty bone name for attaching armor shining. Effect Index: %i, EffectFileName: %s", effectIndex, effectFileName);
					return;
				}

				CEffectManager::Instance().RegisterEffect(effectFileName, false, false);
				m_acceShiningEffects[effectIndex] = m_GraphicThingInstance.AttachEffectByName(0, boneName, effectFileName);
			}
# endif
		}
		break;

		default:
			Tracef("Invalid partname for getting attaching bone name. %s - %s", effectFileName, boneName);
			break;
	}
}

void CInstanceBase::__ClearShiningEffect(uint8_t bType, uint8_t sub)
{
	switch (bType)
	{
		case CItemData::ITEM_TYPE_WEAPON:
		{
			//Clears all weapon shining effects - Left & Right if set
			for (int i = 0; i < CItemData::ITEM_SHINING_MAX_COUNT; i++)
			{
				if (m_weaponShiningEffects[0][i])
					__DetachEffect(m_weaponShiningEffects[0][i]);

				if (m_weaponShiningEffects[1][i])
					__DetachEffect(m_weaponShiningEffects[1][i]);
			}

			memset(&m_weaponShiningEffects, 0, sizeof(m_weaponShiningEffects));
		}
		break;

		case CItemData::ITEM_TYPE_ARMOR:
		{
			//Clears all armor shining effects
			for (int i = 0; i < CItemData::ITEM_SHINING_MAX_COUNT; i++)
			{
				if (m_armorShiningEffects[i])
					__DetachEffect(m_armorShiningEffects[i]);
			}

			memset(&m_armorShiningEffects, 0, sizeof(m_armorShiningEffects));
		}
		break;

		case CItemData::ITEM_TYPE_COSTUME:
		{
# ifdef ENABLE_ACCE_COSTUME_SYSTEM
			//Clears all Acce shining effects
			if (sub == CItemData::COSTUME_ACCE)
			{
				for (int i = 0; i < CItemData::ITEM_SHINING_MAX_COUNT; i++)
				{
					if (m_acceShiningEffects[i])
						__DetachEffect(m_acceShiningEffects[i]);
				}

				memset(&m_acceShiningEffects, 0, sizeof(m_acceShiningEffects));
			}
#endif
		}
		break;

		default:
			return;
	}
}
#endif

bool CInstanceBase::__IsShapeAnimalWear() const
{
	if (100 == GetShape() || 101 == GetShape() || 102 == GetShape() || 103 == GetShape())
		return true;

	return false;
}

uint32_t CInstanceBase::__GetRaceType() const
{
	return m_eRaceType;
}


void CInstanceBase::RefreshState(uint32_t dwMotIndex, bool isLoop)
{
	uint32_t dwPartItemID = m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON);

	uint8_t byItemType = 0xff;
	uint8_t bySubType = 0xff;

	CItemManager & rkItemMgr = CItemManager::Instance();
	CItemData * pItemData;

	if (rkItemMgr.GetItemDataPointer(dwPartItemID, &pItemData))
	{
		byItemType = pItemData->GetType();
		bySubType = pItemData->GetWeaponType();
	}

	if (IsPoly())
		SetMotionMode(CRaceMotionData::MODE_GENERAL);
	else if (IsWearingDress())
		SetMotionMode(CRaceMotionData::MODE_WEDDING_DRESS);
	else if (IsHoldingPickAxe())
	{
		if (m_kHorse.IsMounting())
			SetMotionMode(CRaceMotionData::MODE_HORSE);
		else
			SetMotionMode(CRaceMotionData::MODE_GENERAL);
	}
	else if (CItemData::ITEM_TYPE_ROD == byItemType)
	{
		if (m_kHorse.IsMounting())
			SetMotionMode(CRaceMotionData::MODE_HORSE);
		else
			SetMotionMode(CRaceMotionData::MODE_FISHING);
	}
	else if (m_kHorse.IsMounting())
	{
		switch (bySubType)
		{
			case CItemData::WEAPON_SWORD:
				SetMotionMode(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD);
				break;

			case CItemData::WEAPON_TWO_HANDED:
				SetMotionMode(CRaceMotionData::MODE_HORSE_TWOHAND_SWORD); // Only Warrior
				break;

			case CItemData::WEAPON_DAGGER:
				SetMotionMode(CRaceMotionData::MODE_HORSE_DUALHAND_SWORD); // Only Assassin
				break;

			case CItemData::WEAPON_FAN:
				SetMotionMode(CRaceMotionData::MODE_HORSE_FAN); // Only Shaman
				break;

			case CItemData::WEAPON_BELL:
				SetMotionMode(CRaceMotionData::MODE_HORSE_BELL); // Only Shaman
				break;

			case CItemData::WEAPON_BOW:
				SetMotionMode(CRaceMotionData::MODE_HORSE_BOW); // Only Shaman
				break;
#ifdef ENABLE_WOLFMAN_CHARACTER
			case CItemData::WEAPON_CLAW:
				SetMotionMode(CRaceMotionData::MODE_HORSE_CLAW); // Only Wolfman
				break;
#endif
			default:
				SetMotionMode(CRaceMotionData::MODE_HORSE);
				break;
		}
	}
	else
	{
		switch (bySubType)
		{
			case CItemData::WEAPON_SWORD:
				SetMotionMode(CRaceMotionData::MODE_ONEHAND_SWORD);
				break;

			case CItemData::WEAPON_TWO_HANDED:
				SetMotionMode(CRaceMotionData::MODE_TWOHAND_SWORD); // Only Warrior
				break;

			case CItemData::WEAPON_DAGGER:
				SetMotionMode(CRaceMotionData::MODE_DUALHAND_SWORD); // Only Assassin
				break;

			case CItemData::WEAPON_BOW:
				SetMotionMode(CRaceMotionData::MODE_BOW); // Only Assassin
				break;

			case CItemData::WEAPON_FAN:
				SetMotionMode(CRaceMotionData::MODE_FAN); // Only Shaman
				break;

			case CItemData::WEAPON_BELL:
				SetMotionMode(CRaceMotionData::MODE_BELL); // Only Shaman
				break;
#ifdef ENABLE_WOLFMAN_CHARACTER
			case CItemData::WEAPON_CLAW:
				SetMotionMode(CRaceMotionData::MODE_CLAW); // Only Wolfman
				break;
#endif
			case CItemData::WEAPON_ARROW:
			default:
				SetMotionMode(CRaceMotionData::MODE_GENERAL);
				break;
		}
	}

	if (isLoop)
		m_GraphicThingInstance.InterceptLoopMotion(dwMotIndex);
	else
		m_GraphicThingInstance.InterceptOnceMotion(dwMotIndex);

	RefreshActorInstance();
}

#include "../GameLib/RaceManager.h"
float CInstanceBase::GetBaseHeight()
{
	CActorInstance* pkHorse = m_kHorse.GetActorPtr();
	if (!m_kHorse.IsMounting() || !pkHorse)
		return 0.0f;

	uint32_t dwHorseVnum = m_kHorse.m_pkActor->GetRace();
	if ((dwHorseVnum >= 20101 && dwHorseVnum <= 20109) ||
		(dwHorseVnum == 20029 || dwHorseVnum == 20030))
		return 100.0f;

	float fRaceHeight = CRaceManager::Instance().GetRaceHeight(dwHorseVnum);
	if (fRaceHeight == 0.0f)
		return 100.0f;
	else
		return fRaceHeight;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Device

#ifdef ENABLE_GRAPHIC_ON_OFF
void CInstanceBase::SetRank(uint8_t bRank)
{
	m_bRank = bRank;
	m_GraphicThingInstance.SetRank(bRank);
}

bool CInstanceBase::IsShowEffects()
{
	return m_GraphicThingInstance.IsShowEffects();
}

bool CInstanceBase::HasPrivateShopSign()
{
	return m_GraphicThingInstance.HasPrivateShopSign();
}

bool CInstanceBase::IsShowActor()
{
	return m_GraphicThingInstance.IsShowActor();
}
#endif

void CInstanceBase::RegisterBoundingSphere()
{
	// Stone ÀÏ °æ¿ì DeforomNoSkin À» ÇÏ¸é
	// ³«ÇÏÇÏ´Â ¾Ö´Ï¸ÞÀÌ¼Ç °°Àº °æ¿ì ¾Ö´Ï¸ÞÀÌ¼ÇÀÌ
	// ¹Ù¿îµå ¹Ú½º¿¡ ¿µÇâÀ» ¹ÌÃÄ ÄÃ¸µÀÌ Á¦´ë·Î ÀÌ·ç¾îÁöÁö ¾Ê´Â´Ù.
	if (!IsStone())
		m_GraphicThingInstance.DeformNoSkin();

	m_GraphicThingInstance.RegisterBoundingSphere();
}

bool CInstanceBase::CreateDeviceObjects()
{
	return m_GraphicThingInstance.CreateDeviceObjects();
}

void CInstanceBase::DestroyDeviceObjects()
{
	m_GraphicThingInstance.DestroyDeviceObjects();
}

void CInstanceBase::Destroy()
{
	DetachTextTail();

	DismountHorse();

	m_kQue_kCmdNew.clear();

	__EffectContainer_Destroy();
	__StoneSmoke_Destroy();

	if (__IsMainInstance())
		__ClearMainInstance();

	m_GraphicThingInstance.Destroy();

	__Initialize();
}

void CInstanceBase::__InitializeRotationSpeed()
{
	SetRotationSpeed(c_fDefaultRotationSpeed);
}

void CInstanceBase::__Warrior_Initialize()
{
	m_kWarrior.m_dwGeomgyeongEffect = 0;
}

void CInstanceBase::__Assassin_Initialize()
{
	m_kAssassin.m_dwGyeongGongEffect = 0;
}

void CInstanceBase::__Initialize()
{
	__Warrior_Initialize();
	__Assassin_Initialize();
	__StoneSmoke_Inialize();
	__EffectContainer_Initialize();
	__InitializeRotationSpeed();

	SetEventHandler(CActorInstance::IEventHandler::GetEmptyPtr());

	m_kAffectFlagContainer.Clear();

	m_dwLevel = 0;
#ifdef ENABLE_YOHARA_SYSTEM
	m_dwcLevel = 0;
#endif
#ifdef WJ_SHOW_MOB_INFO
	m_dwAIFlag = 0;
#endif
	m_dwGuildID = 0;
#ifdef ENABLE_SHOW_GUILD_LEADER
	m_dwNewIsGuildName = 0;
#endif
	m_dwEmpireID = 0;
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	m_bLanguage = 0;
#endif

	m_eType = 0;
	m_eRaceType = 0;
	m_eShape = 0;
	m_dwRace = 0;
	m_dwVirtualNumber = 0;

	m_dwBaseCmdTime = 0;
	m_dwBaseChkTime = 0;
	m_dwSkipTime = 0;

	m_GraphicThingInstance.Initialize();

	m_dwAdvActorVID = 0;
	m_dwLastDmgActorVID = 0;

	m_nAverageNetworkGap = 0;
	m_dwNextUpdateHeightTime = 0;

	// Moving by keyboard
	m_iRotatingDirection = DEGREE_DIRECTION_SAME;

	// Moving by mouse
	m_isTextTail = FALSE;
	m_isGoing = FALSE;
	NEW_SetSrcPixelPosition(TPixelPosition(0, 0, 0));
	NEW_SetDstPixelPosition(TPixelPosition(0, 0, 0));

	m_kPPosDust = TPixelPosition(0, 0, 0);


	m_kQue_kCmdNew.clear();

	m_dwLastComboIndex = 0;

	m_swordRefineEffectRight = 0;
	m_swordRefineEffectLeft = 0;
	m_armorRefineEffect[0] = 0;
	m_armorRefineEffect[1] = 0;
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	m_acceRefineEffect = 0;
#endif
#ifdef ENABLE_AURA_SYSTEM
	m_auraRefineEffect = 0;
#endif
#ifdef ENABLE_HEROIC_EFFECT
	m_heroicEffectsRight = 0;
	m_heroicEffectsLeft = 0;
#endif
#ifdef ENABLE_SHINING_SYSTEM
	memset(&m_weaponShiningEffects, 0, sizeof(m_weaponShiningEffects));
#endif

	m_sAlignment = 0;
	m_byPKMode = 0;
	m_isKiller = false;
	m_isPartyMember = false;

	m_bEnableTCPState = TRUE;

	m_stName = "";

	msl::refill(m_awPart);
	msl::refill(m_adwCRCAffectEffect);
	m_kMovAfterFunc = {};

	m_bDamageEffectType = false;
	m_dwDuelMode = DUEL_NONE;
	m_dwEmoticonTime = 0;
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	m_dwTitle = 0;
#endif
#ifdef ENABLE_RENDER_TARGET
	m_IsAlwaysRender = false;
#endif
#ifdef ENABLE_REFINE_ELEMENT
	m_dwElementsEffect = 0;
	m_swordElementEffectRight = 0;
	m_swordElementEffectLeft = 0;
#endif
#ifdef ENABLE_GRAPHIC_ON_OFF
	m_bRank = CPythonNonPlayer::MOB_RANK_PAWN;
#endif
}

CInstanceBase::CInstanceBase()
{
	__Initialize();
}

CInstanceBase::~CInstanceBase()
{
	Destroy();
}

#ifdef ENABLE_RENDER_TARGET
bool CInstanceBase::IsAlwaysRender()
{
	return m_IsAlwaysRender;
}

void CInstanceBase::SetAlwaysRender(bool val)
{
	m_IsAlwaysRender = val;
}
#endif

void CInstanceBase::GetBoundBox(D3DXVECTOR3 * vtMin, D3DXVECTOR3 * vtMax)
{
	m_GraphicThingInstance.GetBoundBox(vtMin, vtMax);
}

#ifdef ENABLE_REFINE_ELEMENT
void CInstanceBase::SetElementEffect(uint8_t wElementsEffect)
{
	if (m_dwElementsEffect < 0)
		return;

	m_dwElementsEffect = wElementsEffect;
}

uint8_t CInstanceBase::GetElementEffect()
{
	if (m_dwElementsEffect < 0)
		return 0;

	return m_dwElementsEffect;
}

void CInstanceBase::ClearElementEffect()
{
	if (m_swordElementEffectRight) {
		__DetachEffect(m_swordElementEffectRight);
		m_swordElementEffectRight = 0;
	}

	if (m_swordElementEffectLeft) {
		__DetachEffect(m_swordElementEffectLeft);
		m_swordElementEffectLeft = 0;
	}
}

void CInstanceBase::AttachElementEffect(const CItemData* pItem)
{
	ClearElementEffect();
#ifdef ENABLE_SIMPLE_REFINED_EFFECT_CHECK
	uint32_t refine = pItem->GetRefine();
#else
#ifdef ENABLE_YOHARA_SYSTEM
	auto refine = std::max<size_t>(pItem->GetRefine() + pItem->GetSocketCount(), METIN_SOCKET_MAX_NUM) - METIN_SOCKET_MAX_NUM;
#else
	auto refine = std::max<size_t>(pItem->GetRefine() + pItem->GetSocketCount(), CItemData::ITEM_SOCKET_MAX_NUM) - CItemData::ITEM_SOCKET_MAX_NUM;
#endif
#endif

	if (GetElementEffect() != 0)
	{
		switch (pItem->GetSubType())
		{
			case CItemData::WEAPON_DAGGER:
				m_swordElementEffectRight = EFFECT_ELEMENTS_WEAPON+EFFECT_ELEMENTS_SMALLSWORD+GetElementEffect();
				m_swordElementEffectLeft = EFFECT_ELEMENTS_WEAPON+EFFECT_ELEMENTS_SMALLSWORD_LEFT+GetElementEffect();
				break;

			case CItemData::WEAPON_FAN:
				m_swordElementEffectRight = EFFECT_ELEMENTS_WEAPON+EFFECT_ELEMENTS_FANBELL+GetElementEffect();
				break;

			case CItemData::WEAPON_ARROW:
			case CItemData::WEAPON_BELL:
				m_swordElementEffectRight = EFFECT_ELEMENTS_WEAPON+EFFECT_ELEMENTS_SMALLSWORD+GetElementEffect();
				break;

			case CItemData::WEAPON_BOW:
				m_swordElementEffectRight = EFFECT_ELEMENTS_WEAPON+EFFECT_ELEMENTS_BOW+GetElementEffect();
				break;

#ifdef ENABLE_WOLFMAN_CHARACTER
			case CItemData::WEAPON_CLAW:
				m_swordElementEffectRight = EFFECT_ELEMENTS_WEAPON + EFFECT_ELEMENTS_CLAW + GetElementEffect();
				m_swordElementEffectLeft = EFFECT_ELEMENTS_WEAPON + EFFECT_ELEMENTS_CLAW_LEFT + GetElementEffect();
				break;
#endif

			default:
				m_swordElementEffectRight = EFFECT_ELEMENTS_WEAPON+EFFECT_ELEMENTS_SWORD+GetElementEffect();
		}

		if (m_swordElementEffectRight)
			m_swordElementEffectRight = __AttachEffect(m_swordElementEffectRight);
		if (m_swordElementEffectLeft)
			m_swordElementEffectLeft = __AttachEffect(m_swordElementEffectLeft);
	}
}
#endif

DWORD CInstanceBase::FindNearestVictimVID(DWORD dwCurrentVID)
{
	// int VICTIM_TARGET_SELECT_RANGE_MAX = 0;
	// CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
	// if (!rkPlayer.GetAutoRangeOnOff())
		// VICTIM_TARGET_SELECT_RANGE_MAX = 30000;
	// else
	float passDistance = 0.0f;
	int VICTIM_TARGET_SELECT_RANGE_MAX = 3000;
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	CInstanceBase* pkInstSelf = rkChrMgr.GetMainInstancePtr();
	typedef std::map<DWORD, int> TNearestVictimMap;
	TNearestVictimMap mapVictimsVIDDistance;
	for (CPythonCharacterManager::CharacterIterator i = rkChrMgr.CharacterInstanceBegin(); i != rkChrMgr.CharacterInstanceEnd(); ++i)
	{
		CInstanceBase* pkInstEach = *i;
		if (pkInstEach == pkInstSelf || pkInstEach == this)
			continue;
		if (pkInstEach->GetVirtualID() == dwCurrentVID)
			continue;

		int iDistance = int(pkInstEach->NEW_GetDistanceFromDestInstance(*pkInstSelf));
		if (iDistance < VICTIM_TARGET_SELECT_RANGE_MAX && (pkInstEach->__GetRaceType() == CActorInstance::TYPE_ENEMY || pkInstEach->__GetRaceType() == CActorInstance::TYPE_STONE) && pkInstEach->CanPickInstance())
		{
			CActorInstance& rkActorSelf = m_GraphicThingInstance;
			CActorInstance& rkActorEach = pkInstEach->GetGraphicThingInstanceRef();

			CPythonBackground& rkBG = CPythonBackground::Instance();
			const D3DXVECTOR3& rv3PositionSelf = rkActorSelf.GetPosition();
			const D3DXVECTOR3& rv3PositionEach = rkActorEach.GetPosition();

			int iX = 1, iY = 1;
			int iStep = 100, iRange = 50;

			float fDistanceX = rv3PositionSelf.x - rv3PositionEach.x;
			float fDistanceY = rv3PositionSelf.y - rv3PositionEach.y;

			if (rv3PositionEach.x >= rv3PositionSelf.x)
				fDistanceX = rv3PositionEach.x - rv3PositionSelf.x;
			else
				iX = -iX;

			if (rv3PositionEach.y >= rv3PositionSelf.y)
				fDistanceY = rv3PositionEach.y - rv3PositionSelf.y;
			else
				iY = -iY;

			if (fDistanceX <= fDistanceY)
				iStep = static_cast<int>(fDistanceY);
			else
				iStep = static_cast<int>(fDistanceX);

			float fRangeX = static_cast<float>(iX * iRange);
			float fRangeY = static_cast<float>(iY * iRange);
			bool bIsBlocked = false;

			for (int j = 0; j < iStep; ++j)
			{
				D3DXVECTOR3 v3CheckPosition = D3DXVECTOR3(fRangeX + rv3PositionSelf.x, fRangeY + rv3PositionSelf.y, 0.0f);
				if (rkBG.isAttrOn(v3CheckPosition.x, -v3CheckPosition.y, CTerrainImpl::ATTRIBUTE_BLOCK))
				{
					bIsBlocked = true;
					break;
				}

				bool bCheck = false;
				if (iX >= 0)
					bCheck = rv3PositionEach.x > v3CheckPosition.x;
				else
					bCheck = rv3PositionEach.x < v3CheckPosition.x;

				if (bCheck)
					fRangeX += iX * iRange;

				if (iY >= 0)
					bCheck = rv3PositionEach.y > v3CheckPosition.y;
				else
					bCheck = rv3PositionEach.y < v3CheckPosition.y;

				if (bCheck)
					fRangeY += static_cast<float>(iY * iRange);

				if (iX >= 0)
					bCheck = rv3PositionEach.x > v3CheckPosition.x;
				else
					bCheck = rv3PositionEach.x < v3CheckPosition.x;

				if (!bCheck)
				{
					if (iY >= 0)
						bCheck = rv3PositionEach.y > v3CheckPosition.y;
					else
						bCheck = rv3PositionEach.y < v3CheckPosition.y;

					if (!bCheck)
						break;
				}
			}

			if (!bIsBlocked)
				if (mapVictimsVIDDistance.find(pkInstEach->GetVirtualID()) == mapVictimsVIDDistance.end())
					mapVictimsVIDDistance.insert(TNearestVictimMap::value_type(pkInstEach->GetVirtualID(), iDistance));
		}
	}

	DWORD dwNearestVictimVID = 0;
	int iLowestDistance = VICTIM_TARGET_SELECT_RANGE_MAX;
	for (TNearestVictimMap::iterator it = mapVictimsVIDDistance.begin(); it != mapVictimsVIDDistance.end(); it++)
	{
		CInstanceBase* pkTarget = __FindInstancePtr(it->first);
		if (!pkTarget || pkTarget->IsDead() || pkTarget->GetVirtualID() == dwCurrentVID)
			continue;

		if (it->second < iLowestDistance)
		{
			dwNearestVictimVID = it->first;
			iLowestDistance = it->second;
		}
	}

	mapVictimsVIDDistance.clear();
	return dwNearestVictimVID;
}
