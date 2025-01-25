#include "StdAfx.h"
#include "InstanceBase.h"
#include "PythonTextTail.h"
#include "AbstractApplication.h"
#include "AbstractPlayer.h"
#include "PythonPlayer.h"
#include "PythonSystem.h"

#include "../EffectLib/EffectManager.h"
#include "../EffectLib/ParticleSystemData.h"
#include "../EterLib/Camera.h"
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
#	include "PythonAchievement.h"
#endif

float CInstanceBase::ms_fDustGap;
float CInstanceBase::ms_fHorseDustGap;
uint32_t CInstanceBase::ms_adwCRCAffectEffect[CInstanceBase::EFFECT_NUM];
std::string CInstanceBase::ms_astAffectEffectAttachBone[EFFECT_NUM];

#define BYTE_COLOR_TO_D3DX_COLOR(r, g, b) D3DXCOLOR(float(r) / 255.0f, float(g) / 255.0f, float(b) / 255.0f, 1.0f)

D3DXCOLOR g_akD3DXClrTitle[CInstanceBase::TITLE_NUM];
D3DXCOLOR g_akD3DXClrName[CInstanceBase::NAMECOLOR_NUM];

std::map<int, std::string> g_TitleNameMap;
std::set<uint32_t> g_kSet_dwPVPReadyKey;
std::set<uint32_t> g_kSet_dwPVPKey;
std::set<uint32_t> g_kSet_dwGVGKey;
std::set<uint32_t> g_kSet_dwDUELKey;

bool g_isEmpireNameMode = false;

void CInstanceBase::SetEmpireNameMode(bool isEnable)
{
	g_isEmpireNameMode = isEnable;

	if (isEnable)
	{
		g_akD3DXClrName[NAMECOLOR_MOB] = g_akD3DXClrName[NAMECOLOR_EMPIRE_MOB];
		g_akD3DXClrName[NAMECOLOR_NPC] = g_akD3DXClrName[NAMECOLOR_EMPIRE_NPC];
		g_akD3DXClrName[NAMECOLOR_PC] = g_akD3DXClrName[NAMECOLOR_NORMAL_PC];

		for (uint32_t uEmpire = 1; uEmpire < EMPIRE_NUM; ++uEmpire)
			g_akD3DXClrName[NAMECOLOR_PC + uEmpire] = g_akD3DXClrName[NAMECOLOR_EMPIRE_PC + uEmpire];
	}
	else
	{
		g_akD3DXClrName[NAMECOLOR_MOB] = g_akD3DXClrName[NAMECOLOR_NORMAL_MOB];
		g_akD3DXClrName[NAMECOLOR_NPC] = g_akD3DXClrName[NAMECOLOR_NORMAL_NPC];

		for (uint32_t uEmpire = 0; uEmpire < EMPIRE_NUM; ++uEmpire)
			g_akD3DXClrName[NAMECOLOR_PC + uEmpire] = g_akD3DXClrName[NAMECOLOR_NORMAL_PC];
	}
}

const D3DXCOLOR & CInstanceBase::GetIndexedNameColor(uint32_t eNameColor)
{
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (eNameColor == NAMECOLOR_SHOP)
	{
		static D3DXCOLOR shopColor(D3DCOLOR_XRGB(255, 102, 0));
		return shopColor;
	}
#endif

	if (eNameColor >= NAMECOLOR_NUM)
	{
		static D3DXCOLOR s_kD3DXClrNameDefault(0xffffffff);
		return s_kD3DXClrNameDefault;
	}

	return g_akD3DXClrName[eNameColor];
}

void CInstanceBase::AddDamageEffect(uint32_t damage, uint8_t flag, BOOL bSelf, BOOL bTarget)
{
	if (CPythonSystem::Instance().IsShowDamage())
	{
		SEffectDamage sDamage;
		sDamage.bSelf = bSelf;
		sDamage.bTarget = bTarget;
		sDamage.damage = damage;
		sDamage.flag = flag;
		m_DamageQueue.emplace_back(sDamage);
	}
}

bool CInstanceBase::ProcessDamage()
{
	if (m_DamageQueue.empty())
		return false;

	SEffectDamage sDamage = m_DamageQueue.front();

	m_DamageQueue.pop_front();

	uint32_t damage = sDamage.damage;
	uint8_t flag = sDamage.flag;
	BOOL bSelf = sDamage.bSelf;
	BOOL bTarget = sDamage.bTarget;

	CCamera * pCamera = CCameraManager::Instance().GetCurrentCamera();
	float cameraAngle = GetDegreeFromPosition2(pCamera->GetTarget().x, pCamera->GetTarget().y, pCamera->GetEye().x, pCamera->GetEye().y);

	uint32_t FONT_WIDTH = 30;

	CEffectManager & rkEftMgr = CEffectManager::Instance();

	D3DXVECTOR3 v3Pos = m_GraphicThingInstance.GetPosition();
	v3Pos.z += float(m_GraphicThingInstance.GetHeight());

	D3DXVECTOR3 v3Rot = D3DXVECTOR3(0.0f, 0.0f, cameraAngle);

	if ((flag & DAMAGE_DODGE) || (flag & DAMAGE_BLOCK))
	{
		if (bSelf)
			rkEftMgr.CreateEffect(ms_adwCRCAffectEffect[EFFECT_DAMAGE_MISS], v3Pos, v3Rot);
		else
			rkEftMgr.CreateEffect(ms_adwCRCAffectEffect[EFFECT_DAMAGE_TARGETMISS], v3Pos, v3Rot);
		return false;
	}

	std::string strDamageType;
	uint32_t rdwCRCEft = 0;

	if (bSelf)
	{
#ifndef ENABLE_EXTENDED_DMG_EFFECT
		strDamageType = "damage_";
#endif
		if (m_bDamageEffectType == 0)
			rdwCRCEft = EFFECT_DAMAGE_SELFDAMAGE;
		else
			rdwCRCEft = EFFECT_DAMAGE_SELFDAMAGE2;

#ifdef ENABLE_EXTENDED_DMG_EFFECT
		if (IS_SET(flag, DAMAGE_CRITICAL) && IS_SET(flag, DAMAGE_PENETRATE))
			strDamageType = "damage_crit_pen_";
		else if (IS_SET(flag, DAMAGE_CRITICAL))
			strDamageType = "damage_crit_";
		else if (IS_SET(flag, DAMAGE_PENETRATE))
			strDamageType = "damage_pen_";
		else if (IS_SET(flag, DAMAGE_POISON))
			strDamageType = "damage_poison_";
		else if (IS_SET(flag, DAMAGE_BLEEDING))
			strDamageType = "damage_bleed_";
		else if (IS_SET(flag, DAMAGE_FIRE))
			strDamageType = "damage_burn_";
		else
			strDamageType = "damage_";
#endif

		m_bDamageEffectType = !m_bDamageEffectType;
	}
	else if (bTarget == false)
	{
		strDamageType = "nontarget_";
		rdwCRCEft = EFFECT_DAMAGE_NOT_TARGET;
		return false;
	}
	else
	{
#ifndef ENABLE_EXTENDED_DMG_EFFECT
		strDamageType = "target_";
#endif
		rdwCRCEft = EFFECT_DAMAGE_TARGET;

#ifdef ENABLE_EXTENDED_DMG_EFFECT
		if (IS_SET(flag, DAMAGE_CRITICAL) && IS_SET(flag, DAMAGE_PENETRATE))
			strDamageType = "target_crit_pen_";
		else if (IS_SET(flag, DAMAGE_CRITICAL))
			strDamageType = "target_crit_";
		else if (IS_SET(flag, DAMAGE_PENETRATE))
			strDamageType = "target_pen_";
		else if (IS_SET(flag, DAMAGE_POISON))
			strDamageType = "target_poison_";
		else if (IS_SET(flag, DAMAGE_BLEEDING))
			strDamageType = "target_bleed_";
		else if (IS_SET(flag, DAMAGE_FIRE))
			strDamageType = "target_burn_";
		else
			strDamageType = "target_";
#endif
	}

	uint32_t index = 0;
	uint32_t num = 0;
	std::vector<std::string> textures;
	while (damage > 0)
	{
		if (index > 15)
		{
			TraceError("ProcessDamage Possibility of endless loop");
			break;
		}
		num = damage % 10;
		damage /= 10;
#ifdef ENABLE_EXTENDED_DMG_EFFECT
		textures.emplace_back("d:/ymir work/effect/affect/damagevalue/"+strDamageType+std::to_string(num) + ".dds");
#else
		char numBuf[MAX_PATH];
		sprintf(numBuf, "%u.dds", num);
		textures.emplace_back("d:/ymir work/effect/affect/damagevalue/" + strDamageType + numBuf);
#endif

		rkEftMgr.SetEffectTextures(ms_adwCRCAffectEffect[rdwCRCEft], textures);

		D3DXMATRIX matrix, matTrans;
		D3DXMatrixIdentity(&matrix);
		matrix._41 = v3Pos.x;
		matrix._42 = v3Pos.y;
		matrix._43 = v3Pos.z;
		D3DXMatrixTranslation(&matrix, v3Pos.x, v3Pos.y, v3Pos.z);
		D3DXMatrixMultiply(&matrix, &pCamera->GetInverseViewMatrix(), &matrix);
		D3DXMatrixTranslation(&matTrans, FONT_WIDTH * index, 0, 0);
		matTrans._41 = -matTrans._41;
		matrix = matTrans * matrix;
		D3DXMatrixMultiply(&matrix, &pCamera->GetViewMatrix(), &matrix);

		rkEftMgr.CreateEffect(ms_adwCRCAffectEffect[rdwCRCEft], D3DXVECTOR3(matrix._41, matrix._42, matrix._43), v3Rot);

		textures.clear();

		index++;
	}
}

void CInstanceBase::AttachSpecialEffect(uint32_t effect
#ifdef ENABLE_SCALE_SYSTEM
	, float fScale
#endif
)
{
#ifdef ENABLE_GRAPHIC_ON_OFF
	if (!m_GraphicThingInstance.IsShowEffects() || !m_GraphicThingInstance.IsShowActor())
		return;
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	if (effect == CInstanceBase::EFFECT_ACCE_BACK)
		m_acceRefineEffect = __AttachEffect(CInstanceBase::EFFECT_ACCE_BACK);
	else
#endif
	{
		__AttachEffect(effect
#ifdef ENABLE_SCALE_SYSTEM
			, fScale
#endif
		);
	}
}

#ifdef ENABLE_12ZI
void CInstanceBase::AttachSpecialZodiacEffect(uint32_t eEftType, long GetX, long GetY, float fParticleScale)
{
	const D3DXMATRIX& c_rmatGlobal = m_GraphicThingInstance.GetTransform();

	D3DXMATRIX matrix;
	D3DXMatrixIdentity(&matrix);
	matrix._41 = float(GetX);
	matrix._42 = -float(GetY);
	matrix._43 = c_rmatGlobal._43;

	uint32_t dwEffectCRC = ms_adwCRCAffectEffect[eEftType];
	uint32_t dwEffectIndex = CEffectManager::Instance().GetEmptyIndex();
#	ifdef ENABLE_SCALE_SYSTEM
	CEffectManager::Instance().CreateEffectInstance(dwEffectIndex, dwEffectCRC, fParticleScale);
	#else
	CEffectManager::Instance().CreateEffectInstance(dwEffectIndex, dwEffectCRC);
#	endif
	CEffectManager::Instance().SelectEffectInstance(dwEffectIndex);
	CEffectManager::Instance().SetEffectInstanceGlobalMatrix(matrix);
}
#endif

void CInstanceBase::LevelUp()
{
	__AttachEffect(EFFECT_LEVELUP);
}

void CInstanceBase::SkillUp()
{
	__AttachEffect(EFFECT_SKILLUP);
}

void CInstanceBase::CreateSpecialEffect(uint32_t iEffectIndex
#ifdef ENABLE_SCALE_SYSTEM
	, float fScale
#endif
)
{
#ifdef ENABLE_GRAPHIC_ON_OFF
	if (!m_GraphicThingInstance.IsShowEffects() || !m_GraphicThingInstance.IsShowActor())
		return;
#endif

	const D3DXMATRIX & c_rmatGlobal = m_GraphicThingInstance.GetTransform();

	uint32_t dwEffectIndex = CEffectManager::Instance().GetEmptyIndex();
	uint32_t dwEffectCRC = ms_adwCRCAffectEffect[iEffectIndex];
#ifdef ENABLE_SCALE_SYSTEM
	CEffectManager::Instance().CreateEffectInstance(dwEffectIndex, dwEffectCRC, fScale);
#else
	CEffectManager::Instance().CreateEffectInstance(dwEffectIndex, dwEffectCRC);
#endif
	CEffectManager::Instance().SelectEffectInstance(dwEffectIndex);
	CEffectManager::Instance().SetEffectInstanceGlobalMatrix(c_rmatGlobal);
}

void CInstanceBase::__EffectContainer_Destroy()
{
	SEffectContainer::Dict & rkDctEftID = __EffectContainer_GetDict();

	for (auto & i : rkDctEftID)
		__DetachEffect(i.second);

	rkDctEftID.clear();
}

void CInstanceBase::__EffectContainer_Initialize()
{
	SEffectContainer::Dict & rkDctEftID = __EffectContainer_GetDict();
	rkDctEftID.clear();
}

CInstanceBase::SEffectContainer::Dict & CInstanceBase::__EffectContainer_GetDict()
{
	return m_kEffectContainer.m_kDct_dwEftID;
}


uint32_t CInstanceBase::__EffectContainer_AttachEffect(uint32_t dwEftKey)
{
	SEffectContainer::Dict & rkDctEftID = __EffectContainer_GetDict();
	auto f = rkDctEftID.find(dwEftKey);
	if (rkDctEftID.end() != f)
		return 0;

#ifdef ENABLE_SCALE_SYSTEM
	uint32_t dwEftID = __AttachEffect(dwEftKey, m_GraphicThingInstance.GetScale().x);
#else
	uint32_t dwEftID = __AttachEffect(dwEftKey);
#endif
	rkDctEftID.emplace(dwEftKey, dwEftID);
	return dwEftID;
}


void CInstanceBase::__EffectContainer_DetachEffect(uint32_t dwEftKey)
{
	SEffectContainer::Dict & rkDctEftID = __EffectContainer_GetDict();
	auto f = rkDctEftID.find(dwEftKey);
	if (rkDctEftID.end() == f)
		return;

	__DetachEffect(f->second);

	rkDctEftID.erase(f);
}

void CInstanceBase::__AttachEmpireEffect(uint32_t eEmpire)
{
	if (!__IsExistMainInstance())
		return;

#ifdef ENABLE_BATTLE_FIELD
	if (IsBattleFieldMap())
		return;
#endif

	CInstanceBase * pkInstMain = __GetMainInstancePtr();

	if (IsWarp())
		return;
	if (IsObject())
		return;
	if (IsFlag())
		return;
	if (IsResource())
		return;
#ifdef ENABLE_GROWTH_PET_SYSTEM
	if (IsGrowthPet())
		return;
#endif
#ifdef ENABLE_PROTO_RENEWAL
	if (IsPetPay())
		return;
	if (IsHorse())
		return;
#endif
#ifdef ENABLE_EVENT_BANNER_FLAG
	if (IsBannerFlag())
		return;
#endif

	if (pkInstMain->IsGameMaster())
	{
	}
#ifdef ENABLE_VIP_SYSTEM
	else if (pkInstMain->IsVIP())
	{
	}
#endif
	else
	{
		if (pkInstMain->IsSameEmpire(*this))
			return;

		// HIDE_OTHER_EMPIRE_EUNHYEONG_ASSASSIN
		if (IsAffect(AFFECT_EUNHYEONG))
			return;
		// END_OF_HIDE_OTHER_EMPIRE_EUNHYEONG_ASSASSIN
	}

	if (IsGameMaster())
		return;

#ifdef ENABLE_VIP_SYSTEM
	if (IsVIP())
		return;
#endif

	__EffectContainer_AttachEffect(EFFECT_EMPIRE + eEmpire);
}

void CInstanceBase::__AttachSelectEffect()
{
	__EffectContainer_AttachEffect(EFFECT_SELECT);
}

void CInstanceBase::__DetachSelectEffect()
{
	__EffectContainer_DetachEffect(EFFECT_SELECT);
}

void CInstanceBase::__AttachTargetEffect()
{
	__EffectContainer_AttachEffect(EFFECT_TARGET);
}

void CInstanceBase::__DetachTargetEffect()
{
	__EffectContainer_DetachEffect(EFFECT_TARGET);
}


void CInstanceBase::__StoneSmoke_Inialize()
{
	m_kStoneSmoke.m_dwEftID = 0;
}

void CInstanceBase::__StoneSmoke_Destroy()
{
	if (!m_kStoneSmoke.m_dwEftID)
		return;

	__DetachEffect(m_kStoneSmoke.m_dwEftID);
	m_kStoneSmoke.m_dwEftID = 0;
}

#ifdef ENABLE_TARGET_SELECT_COLOR
void CInstanceBase::__AttachSelectEffectMonster()
{
	__EffectContainer_AttachEffect(EFFECT_MONSTER);
}
void CInstanceBase::__DetachSelectEffectMonster()
{
	__EffectContainer_DetachEffect(EFFECT_MONSTER);
}
void CInstanceBase::__AttachTargetEffectMonster()
{
	__EffectContainer_AttachEffect(EFFECT_TARGET_MONSTER);
}
void CInstanceBase::__DetachTargetEffectMonster()
{
	__EffectContainer_DetachEffect(EFFECT_TARGET_MONSTER);
}

/////////////////////////////////////////////////////////////

void CInstanceBase::__AttachSelectEffectShinsoo()
{
	__EffectContainer_AttachEffect(EFFECT_SHINSOO);
}
void CInstanceBase::__DetachSelectEffectShinsoo()
{
	__EffectContainer_DetachEffect(EFFECT_SHINSOO);
}
void CInstanceBase::__AttachTargetEffectShinsoo()
{
	__EffectContainer_AttachEffect(EFFECT_TARGET_SHINSOO);
}
void CInstanceBase::__DetachTargetEffectShinsoo()
{
	__EffectContainer_DetachEffect(EFFECT_TARGET_SHINSOO);
}

/////////////////////////////////////////////////////////////

void CInstanceBase::__AttachSelectEffectChunjo()
{
	__EffectContainer_AttachEffect(EFFECT_CHUNJO);
}
void CInstanceBase::__DetachSelectEffectChunjo()
{
	__EffectContainer_DetachEffect(EFFECT_CHUNJO);
}
void CInstanceBase::__AttachTargetEffectChunjo()
{
	__EffectContainer_AttachEffect(EFFECT_TARGET_CHUNJO);
}
void CInstanceBase::__DetachTargetEffectChunjo()
{
	__EffectContainer_DetachEffect(EFFECT_TARGET_CHUNJO);
}

/////////////////////////////////////////////////////////////

void CInstanceBase::__AttachSelectEffectJinnos()
{
	__EffectContainer_AttachEffect(EFFECT_JINNOS);
}
void CInstanceBase::__DetachSelectEffectJinnos()
{
	__EffectContainer_DetachEffect(EFFECT_JINNOS);
}
void CInstanceBase::__AttachTargetEffectJinnos()
{
	__EffectContainer_AttachEffect(EFFECT_TARGET_JINNOS);
}
void CInstanceBase::__DetachTargetEffectJinnos()
{
	__EffectContainer_DetachEffect(EFFECT_TARGET_JINNOS);
}
#endif

void CInstanceBase::__StoneSmoke_Create(uint32_t eSmoke)
{
	m_kStoneSmoke.m_dwEftID = m_GraphicThingInstance.AttachSmokeEffect(eSmoke);
}

void CInstanceBase::SetAlpha(float fAlpha)
{
	__SetBlendRenderingMode();
	__SetAlphaValue(fAlpha);
}

bool CInstanceBase::UpdateDeleting()
{
	Update();
	Transform();

	IAbstractApplication & rApp = IAbstractApplication::GetSingleton();

	float fAlpha = __GetAlphaValue() - (rApp.GetGlobalElapsedTime() * 1.5f);
	__SetAlphaValue(fAlpha);

	if (fAlpha < 0.0f)
		return false;

	return true;
}

void CInstanceBase::DeleteBlendOut()
{
	__SetBlendRenderingMode();
	__SetAlphaValue(1.0f);
	DetachTextTail();

	IAbstractPlayer & rkPlayer = IAbstractPlayer::GetSingleton();
	rkPlayer.NotifyDeletingCharacterInstance(GetVirtualID());
}

void CInstanceBase::ClearPVPKeySystem()
{
	g_kSet_dwPVPReadyKey.clear();
	g_kSet_dwPVPKey.clear();
	g_kSet_dwGVGKey.clear();
	g_kSet_dwDUELKey.clear();
}

void CInstanceBase::InsertPVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
	uint32_t dwPVPKey = __GetPVPKey(dwVIDSrc, dwVIDDst);

	g_kSet_dwPVPKey.emplace(dwPVPKey);
}

void CInstanceBase::InsertPVPReadyKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
	uint32_t dwPVPReadyKey = __GetPVPKey(dwVIDSrc, dwVIDDst);

	g_kSet_dwPVPKey.emplace(dwPVPReadyKey);
}

void CInstanceBase::RemovePVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
	uint32_t dwPVPKey = __GetPVPKey(dwVIDSrc, dwVIDDst);

	g_kSet_dwPVPKey.erase(dwPVPKey);
}

void CInstanceBase::InsertGVGKey(uint32_t dwSrcGuildVID, uint32_t dwDstGuildVID)
{
	uint32_t dwGVGKey = __GetPVPKey(dwSrcGuildVID, dwDstGuildVID);
	g_kSet_dwGVGKey.emplace(dwGVGKey);
}

void CInstanceBase::RemoveGVGKey(uint32_t dwSrcGuildVID, uint32_t dwDstGuildVID)
{
	uint32_t dwGVGKey = __GetPVPKey(dwSrcGuildVID, dwDstGuildVID);
	g_kSet_dwGVGKey.erase(dwGVGKey);
}

void CInstanceBase::InsertDUELKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
	uint32_t dwPVPKey = __GetPVPKey(dwVIDSrc, dwVIDDst);

	g_kSet_dwDUELKey.emplace(dwPVPKey);
}

uint32_t CInstanceBase::__GetPVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
	if (dwVIDSrc > dwVIDDst)
		std::swap(dwVIDSrc, dwVIDDst);

	uint32_t awSrc[2];
	awSrc[0] = dwVIDSrc;
	awSrc[1] = dwVIDDst;

	const auto * s = reinterpret_cast<const uint8_t *>(awSrc);
	const uint8_t * end = s + sizeof(awSrc);
	unsigned long h = 0;

	while (s < end)
	{
		h *= 16777619;
		h ^= *const_cast<uint8_t *>(s++);
	}

	return h;
}

bool CInstanceBase::__FindPVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
	uint32_t dwPVPKey = __GetPVPKey(dwVIDSrc, dwVIDDst);

	if (g_kSet_dwPVPKey.end() == g_kSet_dwPVPKey.find(dwPVPKey))
		return false;

	return true;
}

bool CInstanceBase::__FindPVPReadyKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
	uint32_t dwPVPKey = __GetPVPKey(dwVIDSrc, dwVIDDst);

	if (g_kSet_dwPVPReadyKey.end() == g_kSet_dwPVPReadyKey.find(dwPVPKey))
		return false;

	return true;
}

bool CInstanceBase::__FindGVGKey(uint32_t dwSrcGuildID, uint32_t dwDstGuildID)
{
	uint32_t dwGVGKey = __GetPVPKey(dwSrcGuildID, dwDstGuildID);

	if (g_kSet_dwGVGKey.end() == g_kSet_dwGVGKey.find(dwGVGKey))
		return false;

	return true;
}

bool CInstanceBase::__FindDUELKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
	uint32_t dwDUELKey = __GetPVPKey(dwVIDSrc, dwVIDDst);

	if (g_kSet_dwDUELKey.end() == g_kSet_dwDUELKey.find(dwDUELKey))
		return false;

	return true;
}

bool CInstanceBase::IsPVPInstance(CInstanceBase & rkInstSel)
{
	uint32_t dwVIDSrc = GetVirtualID();
	uint32_t dwVIDDst = rkInstSel.GetVirtualID();

	uint32_t dwGuildIDSrc = GetGuildID();
	uint32_t dwGuildIDDst = rkInstSel.GetGuildID();

	if (GetDuelMode())
		return true;

	return __FindPVPKey(dwVIDSrc, dwVIDDst) || __FindGVGKey(dwGuildIDSrc, dwGuildIDDst);
	//__FindDUELKey(dwVIDSrc, dwVIDDst);
}

const D3DXCOLOR & CInstanceBase::GetNameColor()
{
	return GetIndexedNameColor(GetNameColorIndex());
}

uint32_t CInstanceBase::GetNameColorIndex()
{
	if (IsPC())
	{
		if (m_isKiller)
			return NAMECOLOR_PK;

		if (__IsExistMainInstance() && !__IsMainInstance())
		{
			CInstanceBase * pkInstMain = __GetMainInstancePtr();
			if (!pkInstMain)
			{
				TraceError("CInstanceBase::GetNameColorIndex - MainInstance is nullptr");
				return NAMECOLOR_PC;
			}
			uint32_t dwVIDMain = pkInstMain->GetVirtualID();
			uint32_t dwVIDSelf = GetVirtualID();

			if (pkInstMain->GetDuelMode())
			{
				switch (pkInstMain->GetDuelMode())
				{
				case DUEL_CANNOTATTACK:
					return NAMECOLOR_PC + GetEmpireID();
				case DUEL_START:
				{
					if (__FindDUELKey(dwVIDMain, dwVIDSelf))
						return NAMECOLOR_PVP;
					return NAMECOLOR_PC + GetEmpireID();
				}
				}
			}

			if (pkInstMain->IsSameEmpire(*this))
			{
				if (__FindPVPKey(dwVIDMain, dwVIDSelf))
					return NAMECOLOR_PVP;

				uint32_t dwGuildIDMain = pkInstMain->GetGuildID();
				uint32_t dwGuildIDSelf = GetGuildID();
				if (__FindGVGKey(dwGuildIDMain, dwGuildIDSelf))
					return NAMECOLOR_PVP;
				/*
				if (__FindDUELKey(dwVIDMain, dwVIDSelf))
				{
					return NAMECOLOR_PVP;
				}
				*/
			}
			else
				return NAMECOLOR_PVP;
		}

		IAbstractPlayer & rPlayer = IAbstractPlayer::GetSingleton();
		if (rPlayer.IsPartyMemberByVID(GetVirtualID()))
			return NAMECOLOR_PARTY;

		return NAMECOLOR_PC + GetEmpireID();
	}
	if (IsNPC())
		return NAMECOLOR_NPC;
	if (IsEnemy() && !IsBoss())
		return NAMECOLOR_MOB;
	if (IsPoly())
		return NAMECOLOR_MOB;
#ifdef ENABLE_GROWTH_PET_SYSTEM
	if (IsGrowthPet())
		return NAMECOLOR_NPC;
#endif
#ifdef ENABLE_PROTO_RENEWAL
	if (IsPetPay())
		return NAMECOLOR_NPC;
	if (IsHorse())
		return NAMECOLOR_NPC;
#endif
	if (IsStone())
		return NAMECOLOR_METIN;
	if (IsBoss())
		return NAMECOLOR_BOSS;
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (IsShop())
	{
#	ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
		CInstanceBase* pkInstMain = __GetMainInstancePtr();
		if (!pkInstMain)
			return NAMECOLOR_PC;

		if (pkInstMain->IsSameEmpire(*this))
			return NAMECOLOR_PC;
		else
			return NAMECOLOR_PVP;
#	else
		return NAMECOLOR_SHOP;
#	endif
	}
#endif

	return D3DXCOLOR(0xffffffff);
}

const D3DXCOLOR & CInstanceBase::GetTitleColor() const
{
	uint32_t uGrade = GetAlignmentGrade();
	if (uGrade >= TITLE_NUM)
	{
		static D3DXCOLOR s_kD3DXClrTitleDefault(0xffffffff);
		return s_kD3DXClrTitleDefault;
	}

	return g_akD3DXClrTitle[uGrade];
}

void CInstanceBase::AttachTextTail()
{
#ifdef ENABLE_RENDER_TARGET_EFFECT
	if (m_GraphicThingInstance.IsRenderTarget())
		return;
#endif

	if (m_isTextTail)
	{
		TraceError("CInstanceBase::AttachTextTail - VID [%d] ALREADY EXIST", GetVirtualID());
		return;
	}

	m_isTextTail = true;

	uint32_t dwVID = GetVirtualID();

	float fTextTailHeight = IsMountingHorse() ? 110.0f : 10.0f;

	static D3DXCOLOR s_kD3DXClrTextTail = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

#ifdef ENABLE_SHOW_GUILD_LEADER
	CPythonTextTail::Instance().RegisterCharacterTextTail(m_dwGuildID, m_dwNewIsGuildName, dwVID, s_kD3DXClrTextTail, fTextTailHeight);
#else
	CPythonTextTail::Instance().RegisterCharacterTextTail(m_dwGuildID, dwVID, s_kD3DXClrTextTail, fTextTailHeight);
#endif

	// CHARACTER_LEVEL
	if (m_dwLevel)
		UpdateTextTailLevel(m_dwLevel);
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	if (m_bLanguage)
		UpdateTextTailLanguage(m_bLanguage);
#endif
#ifdef ENABLE_PORTAL_LEVEL_LIMIT
	else if (IsWarp())
		UpdatePortalTextTailLevelLimit();
#endif
}

void CInstanceBase::DetachTextTail()
{
	if (!m_isTextTail)
		return;

#ifdef ENABLE_RENDER_TARGET_EFFECT
	if (m_GraphicThingInstance.IsRenderTarget())
		return;
#endif

	m_isTextTail = false;
	CPythonTextTail::Instance().DeleteCharacterTextTail(GetVirtualID());
}

void CInstanceBase::UpdateTextTailLevel(uint32_t level)
{
#ifdef ENABLE_RENDER_TARGET_EFFECT
	if (m_GraphicThingInstance.IsRenderTarget())
		return;
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	uint32_t conq_lev = GetConquerorLevel();
	level = conq_lev ? conq_lev : level;
#endif

#ifdef WJ_SHOW_MOB_INFO // EXTENDED
	static D3DXCOLOR s_kMobLevelColor = D3DXCOLOR(152.0f / 255.0f, 255.0f / 255.0f, 51.0f / 255.0f, 1.0f);
	//static D3DXCOLOR s_kMobLevelColor = D3DXCOLOR(119.0f / 255.0f, 246.0f / 255.0f, 168.0f / 255.0f, 1.0f);
#endif
	static D3DXCOLOR s_kPlayerLevelColor = D3DXCOLOR(152.0f / 255.0f, 255.0f / 255.0f, 51.0f / 255.0f, 1.0f);
	static D3DXCOLOR s_kLevelColor = D3DXCOLOR(152.0f / 255.0f, 255.0f / 255.0f, 51.0f / 255.0f, 1.0f);
	static D3DXCOLOR outline = D3DXCOLOR(0.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 1.0f);		//Outline normal
#ifdef ENABLE_GROWTH_PET_SYSTEM
	static D3DXCOLOR s_kPetLevelColor = D3DXCOLOR(255.0f / 255.0f, 255.0f / 255.0f, 0.0f, 1.0f);
#endif
#ifdef ENABLE_TEAM_GAMEMASTER
	static D3DXCOLOR s_kGameMasterColor = D3DXCOLOR(152.0f / 255.0f, 255.0f / 255.0f, 51.0f / 255.0f, 1.0f);
#endif
#ifdef ENABLE_VIP_SYSTEM
	static D3DXCOLOR s_kVIPColor = D3DXCOLOR(152.0f / 255.0f, 255.0f / 255.0f, 51.0f / 255.0f, 1.0f);
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	static D3DXCOLOR s_kConquerorLevelColor = D3DXCOLOR(191.0f / 255.0f, 217.0f / 255.0f, 255.0f / 255.0f, 1.0f);
	static D3DXCOLOR c_outline = D3DXCOLOR(0.0f / 255.0f, 102.0f / 255.0f, 255.0f / 255.0f, 1.0f );	//Outline Conqueror
#endif

	char szText[256];
	sprintf(szText, " Lv.%d ", level);

#if defined(ENABLE_PREMIUM_PRIVATE_SHOP) && defined(ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL)
	if (IsPC() || IsShop())
#else
	if (IsPC())
#endif
	{
#ifdef ENABLE_VIP_SYSTEM
		if (IsVIP() && !GetConquerorLevel()) {	// VIP
			sprintf(szText, " |cffffcc00 Lv.%d ", level);
			CPythonTextTail::Instance().AttachLevel(GetVirtualID(), szText, s_kVIPColor, outline);
		}
		else if (IsVIP() && GetConquerorLevel()) {	// VIP + Hongmoon
			sprintf(szText, " |cff00ffff HM %d ", level);
			CPythonTextTail::Instance().AttachLevel(GetVirtualID(), szText, conq_lev ? s_kConquerorLevelColor : s_kLevelColor, conq_lev ? c_outline : outline);
		}
#endif
#ifdef ENABLE_TEAM_GAMEMASTER
		else if (IsGameMaster()) {	// GameMaster
			sprintf(szText, " |cffff9900 [Team] ");
			CPythonTextTail::Instance().AttachLevel(GetVirtualID(), szText, s_kGameMasterColor, outline);
		}
#endif
#ifdef ENABLE_YOHARA_SYSTEM
		else if (GetConquerorLevel() && !IsVIP()) {	// Hongmoon
			sprintf(szText, "  HM %d ", level);
			CPythonTextTail::Instance().AttachLevel(GetVirtualID(), szText, conq_lev ? s_kConquerorLevelColor : s_kLevelColor, conq_lev ? c_outline : outline);
		}
#endif
#if defined(ENABLE_VIP_SYSTEM) || defined(ENABLE_TEAM_GAMEMASTER) || defined(ENABLE_YOHARA_SYSTEM)
		else
		CPythonTextTail::Instance().AttachLevel(GetVirtualID(), szText, s_kLevelColor, outline);
#endif
	}

#ifdef ENABLE_GROWTH_PET_SYSTEM
	else if (IsGrowthPet())
	{
		CPythonTextTail::Instance().AttachLevel(GetVirtualID(), szText, s_kPetLevelColor, outline);
	}
#endif
#ifdef WJ_SHOW_MOB_INFO // EXTENDED
	else if (IsEnemy() || IsStone())
	{
		CPythonTextTail::Instance().AttachLevel(GetVirtualID(), szText, s_kMobLevelColor, outline);
	}
#endif
	else
		CPythonTextTail::Instance().DetachLevel(GetVirtualID());
}

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
void CInstanceBase::UpdateTextTailLanguage(uint8_t bLanguage)
{
	if (IsPC())
		CPythonTextTail::Instance().AttachLanguageImage(GetVirtualID(), bLanguage);
}
#endif

void CInstanceBase::RefreshTextTail()
{
#ifdef ENABLE_RENDER_TARGET_EFFECT
	if (m_GraphicThingInstance.IsRenderTarget())
		return;
#endif
	CPythonTextTail::Instance().SetCharacterTextTailColor(GetVirtualID(), GetNameColor());

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	if (GetTitleID() != 0)
	{
		std::string szTitleName;
		if (CAchievementSystem::Instance().GetTitleName(GetTitleID(), szTitleName))
		{
			CPythonTextTail::Instance().DetachTitle(GetVirtualID());
			CPythonTextTail::Instance().AttachTitle(GetVirtualID(), szTitleName.c_str(), GetTitleColor());
			return;
		}
	}
#endif

	int iAlignmentGrade = GetAlignmentGrade();

#ifdef ENABLE_NEUTRAL_RANG
	if (!IsPC()
#	ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		&& !IsShop()
#	endif
#	ifdef ENABLE_BATTLE_FIELD
		|| IsBattleFieldMap()
#	endif
		)
#else
	if (TITLE_NONE == iAlignmentGrade)
#endif
		CPythonTextTail::Instance().DetachTitle(GetVirtualID());
	else
	{
		auto itor = g_TitleNameMap.find(iAlignmentGrade);
		if (g_TitleNameMap.end() != itor)
		{
			const std::string & c_rstrTitleName = itor->second;
			CPythonTextTail::Instance().AttachTitle(GetVirtualID(), c_rstrTitleName.c_str(), GetTitleColor());
		}
	}
}

void CInstanceBase::RefreshTextTailTitle()
{
	RefreshTextTail();
}

#ifdef ENABLE_PORTAL_LEVEL_LIMIT
#include "PythonNonPlayer.h"

void CInstanceBase::UpdatePortalTextTailLevelLimit()
{
	const std::pair<int, int>* pPortalLevelLimit = CPythonNonPlayer::Instance().GetPortalLevelLimit(GetRace());
	if (pPortalLevelLimit == nullptr)
		return;

	static const D3DXCOLOR s_kPortalLevelLimitColor = D3DXCOLOR(0.0f, 1.0f, 1.0f, 1.0f);
	static const D3DXCOLOR outline = D3DXCOLOR(0.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 1.0f);
	const int iMinLevel = pPortalLevelLimit->first;
	const int iMaxLevel = pPortalLevelLimit->second;

	char szText[32];
	snprintf(szText, sizeof(szText), "Lv Limit: [%d-%d]", iMinLevel, iMaxLevel);
	CPythonTextTail::Instance().AttachLevel(GetVirtualID(), szText, s_kPortalLevelLimitColor, outline);
}
#endif

/////////////////////////////////////////////////
void CInstanceBase::__ClearAffectFlagContainer()
{
	m_kAffectFlagContainer.Clear();
}

void CInstanceBase::__ClearAffects()
{
	if (IsStone()
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
		&& (GetVirtualNumber() != GUILD_MELEY_LAIR_DUNGEON_STATUE)
#	ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
		&& (GetVirtualNumber() != PARTY_MELEY_LAIR_DUNGEON_STATUE)
#	endif
#endif
		)
	{
		__StoneSmoke_Destroy();
	}
	else
	{
		for (auto & iAffect : m_adwCRCAffectEffect)
		{
			__DetachEffect(iAffect);
			iAffect = 0;
		}

		__ClearAffectFlagContainer();
	}

	m_GraphicThingInstance.__OnClearAffects();
}

/////////////////////////////////////////////////

void CInstanceBase::__SetNormalAffectFlagContainer(const CAffectFlagContainer & c_rkAffectFlagContainer)
{
	for (int i = 0; i < CAffectFlagContainer::BIT_SIZE; ++i)
	{
		bool isOldSet = m_kAffectFlagContainer.IsSet(i);
		bool isNewSet = c_rkAffectFlagContainer.IsSet(i);

		if (isOldSet != isNewSet)
		{
			__SetAffect(i, isNewSet);

			if (isNewSet)
				m_GraphicThingInstance.__OnSetAffect(i);
			else
				m_GraphicThingInstance.__OnResetAffect(i);
		}
	}

	m_kAffectFlagContainer.CopyInstance(c_rkAffectFlagContainer);
}

void CInstanceBase::__SetStoneSmokeFlagContainer(const CAffectFlagContainer & c_rkAffectFlagContainer)
{
	m_kAffectFlagContainer.CopyInstance(c_rkAffectFlagContainer);

	uint32_t eSmoke;
	if (m_kAffectFlagContainer.IsSet(STONE_SMOKE8))
		eSmoke = 3;
	else if (m_kAffectFlagContainer.IsSet(STONE_SMOKE5) | m_kAffectFlagContainer.IsSet(STONE_SMOKE6) |
			 m_kAffectFlagContainer.IsSet(STONE_SMOKE7))
		eSmoke = 2;
	else if (m_kAffectFlagContainer.IsSet(STONE_SMOKE2) | m_kAffectFlagContainer.IsSet(STONE_SMOKE3) |
			 m_kAffectFlagContainer.IsSet(STONE_SMOKE4))
		eSmoke = 1;
	else
		eSmoke = 0;

	__StoneSmoke_Destroy();
	__StoneSmoke_Create(eSmoke);
}

void CInstanceBase::SetAffectFlagContainer(const CAffectFlagContainer & c_rkAffectFlagContainer)
{
	if (IsBuilding())
		return;
	else if (IsStone()
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
		&& (GetVirtualNumber() != GUILD_MELEY_LAIR_DUNGEON_STATUE)
#	ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
		&& (GetVirtualNumber() != PARTY_MELEY_LAIR_DUNGEON_STATUE)
#	endif
#endif
		)
	{
		__SetStoneSmokeFlagContainer(c_rkAffectFlagContainer);
	}
	else
		__SetNormalAffectFlagContainer(c_rkAffectFlagContainer);
}


void CInstanceBase::SCRIPT_SetAffect(uint32_t eAffect, bool isVisible)
{
	__SetAffect(eAffect, isVisible);
}

void CInstanceBase::__SetReviveInvisibilityAffect(bool isVisible)
{
#ifdef ENABLE_GRAPHIC_ON_OFF
	if (IsAffect(AFFECT_INVISIBILITY))
		return;
#endif

	if (isVisible)
	{
		if (IsWearingDress())
			return;

		m_GraphicThingInstance.BlendAlphaValue(0.5f, 1.0f);
	}
	else
		m_GraphicThingInstance.BlendAlphaValue(1.0f, 1.0f);
}

void CInstanceBase::__Assassin_SetEunhyeongAffect(bool isVisible)
{
#ifdef ENABLE_GRAPHIC_ON_OFF
	if (IsAffect(AFFECT_INVISIBILITY))
		return;
#endif

	if (isVisible)
	{
		if (IsWearingDress())
			return;

#ifdef ENABLE_GRAPHIC_ON_OFF
		if (IsPoly())
			return;
#endif

		if (__IsMainInstance() || __MainCanSeeHiddenThing())
			m_GraphicThingInstance.BlendAlphaValue(0.5f, 1.0f);
		else
		{
			m_GraphicThingInstance.BlendAlphaValue(0.0f, 1.0f);
#ifndef ENABLE_GRAPHIC_ON_OFF
			m_GraphicThingInstance.HideAllAttachingEffect();
#endif
		}
	}
	else
	{
#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
		if (IsAffect(AFFECT_INVISIBILITY) && __MainCanSeeHiddenThing())
			return;
#endif
		m_GraphicThingInstance.BlendAlphaValue(1.0f, 1.0f);
#ifndef ENABLE_GRAPHIC_ON_OFF
		m_GraphicThingInstance.ShowAllAttachingEffect();
#endif
	}
}

void CInstanceBase::__Shaman_SetParalysis(bool isParalysis)
{
	m_GraphicThingInstance.SetParalysis(isParalysis);
}

void CInstanceBase::__Warrior_SetGeomgyeongAffect(bool isVisible)
{
	if (isVisible)
	{
		if (IsWearingDress())
			return;

		if (m_kWarrior.m_dwGeomgyeongEffect)
			__DetachEffect(m_kWarrior.m_dwGeomgyeongEffect);

		m_GraphicThingInstance.SetReachScale(1.5f);
		if (m_GraphicThingInstance.IsTwoHandMode())
			m_kWarrior.m_dwGeomgyeongEffect = __AttachEffect(EFFECT_WEAPON + WEAPON_TWOHAND);
		else
			m_kWarrior.m_dwGeomgyeongEffect = __AttachEffect(EFFECT_WEAPON + WEAPON_ONEHAND);
	}
	else
	{
		m_GraphicThingInstance.SetReachScale(1.0f);

		__DetachEffect(m_kWarrior.m_dwGeomgyeongEffect);
		m_kWarrior.m_dwGeomgyeongEffect = 0;
	}
}

void CInstanceBase::__Assassin_SetGyeongGongAffect(bool isVisible)
{
	if (isVisible)
	{
		if (IsWearingDress())
			return;

		if (m_kAssassin.m_dwGyeongGongEffect)
			__DetachEffect(m_kAssassin.m_dwGyeongGongEffect);

		m_GraphicThingInstance.SetReachScale(1.5f);
		m_kAssassin.m_dwGyeongGongEffect = __AttachEffect(EFFECT_GYEONGGONG_BOOM);
	}
	else
	{
		m_GraphicThingInstance.SetReachScale(1.0f);

		__DetachEffect(m_kAssassin.m_dwGyeongGongEffect);
		m_kAssassin.m_dwGyeongGongEffect = 0;
	}
}

void CInstanceBase::__SetAffect(uint32_t eAffect, bool isVisible)
{
	switch (eAffect)
	{
		case AFFECT_YMIR:
#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
			if (IsAffect(AFFECT_INVISIBILITY) && !__MainCanSeeHiddenThing())
				return;
#else
			if (IsAffect(AFFECT_INVISIBILITY))
				return;
#endif
			break;

#ifdef ENABLE_VIP_SYSTEM
		case AFFECT_VIP:
			if (IsAffect(AFFECT_INVISIBILITY))
				return;
			break;
#endif

		case AFFECT_CHEONGEUN:
			m_GraphicThingInstance.SetResistFallen(isVisible);
			break;

		case AFFECT_GEOMGYEONG:
			__Warrior_SetGeomgyeongAffect(isVisible);
			return;

		case AFFECT_REVIVE_INVISIBILITY:
#ifdef ENABLE_GRAPHIC_ON_OFF
			__SetReviveInvisibilityAffect(isVisible);
#else
			__Assassin_SetEunhyeongAffect(isVisible);
#endif
			break;

		case AFFECT_EUNHYEONG:
			__Assassin_SetEunhyeongAffect(isVisible);
			break;
		case AFFECT_GYEONGGONG:
			if (isVisible)
			{
				__Assassin_SetGyeongGongAffect(isVisible);
				if (!IsWalking())
					return;
			}
			break;

		case AFFECT_KWAESOK:
			if (isVisible)
				if (!IsWalking())
					return;
			break;

		case AFFECT_INVISIBILITY:
#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
			if (__MainCanSeeHiddenThing())
			{
				if (isVisible)
					m_GraphicThingInstance.BlendAlphaValue(0.5f, 1.0f);
				else
					m_GraphicThingInstance.BlendAlphaValue(1.0f, 1.0f);
				break;
			}
#endif

			if (isVisible)
			{
				m_GraphicThingInstance.ClearAttachingEffect();
				__EffectContainer_Destroy();
				DetachTextTail();
			}
			else
			{
				m_GraphicThingInstance.BlendAlphaValue(1.0f, 1.0f);
				AttachTextTail();
				RefreshTextTail();
			}
			return;

		case AFFECT_STUN:
			m_GraphicThingInstance.SetSleep(isVisible);
			break;
	}

	if (eAffect >= AFFECT_NUM)
	{
		TraceError("CInstanceBase[VID:%d]::SetAffect(eAffect:%d<AFFECT_NUM:%d, isVisible=%d)", GetVirtualID(), eAffect, isVisible);
		return;
	}

	if (isVisible)
	{
		if (!m_adwCRCAffectEffect[eAffect])
			m_adwCRCAffectEffect[eAffect] = __AttachEffect(EFFECT_AFFECT + eAffect);
	}
	else
	{
		if (m_adwCRCAffectEffect[eAffect])
		{
			__DetachEffect(m_adwCRCAffectEffect[eAffect]);
			m_adwCRCAffectEffect[eAffect] = 0;
		}
	}
}

bool CInstanceBase::IsPossibleEmoticon() const
{
	CEffectManager & rkEftMgr = CEffectManager::Instance();
	for (uint32_t eEmoticon = 0; eEmoticon < EMOTICON_NUM; eEmoticon++)
	{
		uint32_t effectID = ms_adwCRCAffectEffect[EFFECT_EMOTICON + eEmoticon];
		if (effectID && rkEftMgr.IsAliveEffect(effectID))
			return false;
	}

	if (ELTimer_GetMSec() - m_dwEmoticonTime < 1000)
	{
		TraceError("ELTimer_GetMSec() - m_dwEmoticonTime");
		return false;
	}

	return true;
}

void CInstanceBase::SetFishEmoticon()
{
	SetEmoticon(EMOTICON_FISH);
}

void CInstanceBase::SetEmoticon(uint32_t eEmoticon)
{
	if (eEmoticon >= EMOTICON_NUM)
	{
		TraceError("CInstanceBase[VID:%d]::SetEmoticon(eEmoticon:%d<EMOTICON_NUM:%d, isVisible=%d)", GetVirtualID(), eEmoticon);
		return;
	}
	if (IsPossibleEmoticon())
	{
		D3DXVECTOR3 v3Pos = m_GraphicThingInstance.GetPosition();
		v3Pos.z += float(m_GraphicThingInstance.GetHeight());

		CCamera * pCamera = CCameraManager::Instance().GetCurrentCamera();

		D3DXVECTOR3 v3Dir = (pCamera->GetEye() - v3Pos) * 9 / 10;
		v3Pos = pCamera->GetEye() - v3Dir;

		v3Pos = D3DXVECTOR3(0, 0, 0);
		v3Pos.z += float(m_GraphicThingInstance.GetHeight());

		m_GraphicThingInstance.AttachEffectByID(0, nullptr, ms_adwCRCAffectEffect[EFFECT_EMOTICON + eEmoticon], &v3Pos);
		m_dwEmoticonTime = ELTimer_GetMSec();
	}
}

void CInstanceBase::SetDustGap(float fDustGap)
{
	ms_fDustGap = fDustGap;
}

void CInstanceBase::SetHorseDustGap(float fDustGap)
{
	ms_fHorseDustGap = fDustGap;
}

void CInstanceBase::__DetachEffect(uint32_t dwEID)
{
	m_GraphicThingInstance.DettachEffect(dwEID);
}

uint32_t CInstanceBase::__AttachEffect(uint32_t eEftType
#ifdef ENABLE_SCALE_SYSTEM
	, float fScale
#endif
)
{
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	switch (eEftType)
	{
		case EFFECT_AFFECT + AFFECT_GONGPO:
			RegisterEffect(eEftType, "", "d:/ymir work/pc/sura/effect/fear_loop.mse", false, GetNameString());
			break;
		case EFFECT_AFFECT + AFFECT_JUMAGAP:
			RegisterEffect(eEftType, "", "d:/ymir work/pc/sura/effect/jumagap_loop.mse", false, GetNameString());
			break;
		case EFFECT_AFFECT + AFFECT_HOSIN:
			RegisterEffect(eEftType, "", "d:/ymir work/pc/shaman/effect/3hosin_loop.mse", false, GetNameString());
			break;
		case EFFECT_AFFECT + AFFECT_BOHO:
			RegisterEffect(eEftType, "", "d:/ymir work/pc/shaman/effect/boho_loop.mse", false, GetNameString());
			break;
		case EFFECT_AFFECT + AFFECT_KWAESOK:
			RegisterEffect(eEftType, "", "d:/ymir work/pc/shaman/effect/10kwaesok_loop.mse", false, GetNameString());
			break;
		case EFFECT_AFFECT + AFFECT_HEUKSIN:
			RegisterEffect(eEftType, "", "d:/ymir work/pc/sura/effect/heuksin_loop.mse", false, GetNameString());
			break;
		case EFFECT_AFFECT + AFFECT_MUYEONG:
			RegisterEffect(eEftType, "", "d:/ymir work/pc/sura/effect/muyeong_loop.mse", false, GetNameString());
			break;
		case EFFECT_AFFECT + AFFECT_FIRE:
			RegisterEffect(eEftType, "Bip01", "d:/ymir work/effect/hit/hwayeom_loop_1.mse", false, GetNameString());
			break;
		case EFFECT_AFFECT + AFFECT_GICHEON:
			RegisterEffect(eEftType, "Bip01 R Hand", "d:/ymir work/pc/shaman/effect/6gicheon_hand.mse", false, GetNameString());
			break;
		case EFFECT_AFFECT + AFFECT_JEUNGRYEOK:
			RegisterEffect(eEftType, "Bip01 L Hand", "d:/ymir work/pc/shaman/effect/jeungryeok_hand.mse", false, GetNameString());
			break;
		case EFFECT_AFFECT + AFFECT_PABEOP:
			RegisterEffect(eEftType, "Bip01 Head", "d:/ymir work/pc/sura/effect/pabeop_loop.mse", false, GetNameString());
			break;
		case EFFECT_AFFECT + AFFECT_CHEONGEUN:
		case EFFECT_AFFECT + AFFECT_FALLEN_CHEONGEUN:
			RegisterEffect(eEftType, "", "d:/ymir work/pc/warrior/effect/gyeokgongjang_loop.mse", false, GetNameString());
			break;
		case EFFECT_AFFECT + AFFECT_GWIGEOM:
			RegisterEffect(eEftType, "Bip01 R Finger2", "d:/ymir work/pc/sura/effect/gwigeom_loop.mse", false, GetNameString());
			break;
		case EFFECT_AFFECT + AFFECT_GYEONGGONG:
			RegisterEffect(eEftType, "", "d:/ymir work/pc/assassin/effect/gyeonggong_loop.mse", false, GetNameString());
			break;
		case EFFECT_WEAPON + WEAPON_ONEHAND:
				RegisterEffect(eEftType, "equip_right_hand", "d:/ymir work/pc/warrior/effect/geom_sword_loop.mse", false, GetNameString());
				break;
		case EFFECT_WEAPON + WEAPON_TWOHAND:
			RegisterEffect(eEftType, "equip_right_hand", "d:/ymir work/pc/warrior/effect/geom_spear_loop.mse", false, GetNameString());
			break;
#	ifdef ENABLE_WOLFMAN_CHARACTER
		case EFFECT_AFFECT + AFFECT_RED_POSSESSION:
			RegisterEffect(eEftType, "Bip01", "d:/ymir work/effect/hit/blow_flame/flame_loop_w.mse", false, GetNameString());
			break;
		case EFFECT_AFFECT + AFFECT_BLUE_POSSESSION:
			RegisterEffect(eEftType, "", "d:/ymir work/pc3/common/effect/gyeokgongjang_loop_w.mse", false, GetNameString());
			break;
#	endif
#	ifdef ENABLE_NINETH_SKILL
		case EFFECT_AFFECT + AFFECT_CHEONUN:
			RegisterEffect(eEftType, "", "d:/ymir work/pc/shaman/effect/chunwoon_4_target.mse", false, GetNameString());
			break;
#	endif
		default:
			break;
	}
#endif

#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
	if (IsAffect(AFFECT_INVISIBILITY) && !__MainCanSeeHiddenThing())
		return 0;
#else
	if (IsAffect(AFFECT_INVISIBILITY))
		return 0;
#endif

	if (eEftType >= EFFECT_NUM)
		return 0;

#ifdef ENABLE_SKILL_COLOR_SYSTEM
	uint32_t* dwSkillColor = m_GraphicThingInstance.GetSkillColorByEffectID(eEftType);
#endif

	if (ms_astAffectEffectAttachBone[eEftType].empty())
	{
		return m_GraphicThingInstance.AttachEffectByID(0, nullptr, ms_adwCRCAffectEffect[eEftType]
#ifdef ENABLE_SCALE_SYSTEM
			, (const D3DXVECTOR3*)nullptr, fScale
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
			, (const D3DXVECTOR3*)nullptr, dwSkillColor
#endif
#ifdef ENABLE_GRAPHIC_ON_OFF
			, eEftType, FALSE
#endif
		);
	}

	std::string & rstrBoneName = ms_astAffectEffectAttachBone[eEftType];
	const char * c_szBoneName;
	// Used when attaching to both hands.
	// The reason for this type of exception handling is that the Bone Name of Equip is different for each character.
	if ("PART_WEAPON" == rstrBoneName)
	{
		if (m_GraphicThingInstance.GetAttachingBoneName(CRaceData::PART_WEAPON, &c_szBoneName))
		{
			return m_GraphicThingInstance.AttachEffectByID(0, c_szBoneName, ms_adwCRCAffectEffect[eEftType]
#ifdef ENABLE_SCALE_SYSTEM
				, nullptr, 1.0f
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
				, nullptr, dwSkillColor
#endif
			);
		}
	}
	else if ("PART_WEAPON_LEFT" == rstrBoneName)
	{
		if (m_GraphicThingInstance.GetAttachingBoneName(CRaceData::PART_WEAPON_LEFT, &c_szBoneName))
		{
			return m_GraphicThingInstance.AttachEffectByID(0, c_szBoneName, ms_adwCRCAffectEffect[eEftType]
#ifdef ENABLE_SCALE_SYSTEM
				, (const D3DXVECTOR3*)nullptr, 1.0f
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
				, (const D3DXVECTOR3*)nullptr, dwSkillColor
#endif
			);
		}
	}
	else
	{
		return m_GraphicThingInstance.AttachEffectByID(0, rstrBoneName.c_str(), ms_adwCRCAffectEffect[eEftType]
#ifdef ENABLE_SCALE_SYSTEM
			, (const D3DXVECTOR3*)nullptr, fScale
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
			, (const D3DXVECTOR3*)nullptr, dwSkillColor
#endif
#ifdef ENABLE_GRAPHIC_ON_OFF
			, eEftType, FALSE
#endif
		);
	}

	return 0;
}

void CInstanceBase::__ComboProcess() const {}

#ifdef ENABLE_SKILL_COLOR_SYSTEM
bool CInstanceBase::RegisterEffect(uint32_t eEftType, const char* c_szEftAttachBone, const char* c_szEftName, bool isCache, const char* name)
#else
bool CInstanceBase::RegisterEffect(uint32_t eEftType, const char* c_szEftAttachBone, const char* c_szEftName, bool isCache)
#endif
{
	if (eEftType >= EFFECT_NUM)
		return false;

	ms_astAffectEffectAttachBone[eEftType] = c_szEftAttachBone;

	uint32_t & rdwCRCEft = ms_adwCRCAffectEffect[eEftType];
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	if (!CEffectManager::Instance().RegisterEffect2(c_szEftName, &rdwCRCEft, isCache, name))
#else
	if (!CEffectManager::Instance().RegisterEffect2(c_szEftName, &rdwCRCEft, isCache))
#endif
	{
		TraceError("CInstanceBase::RegisterEffect(eEftType=%d, c_szEftAttachBone=%s, c_szEftName=%s, isCache=%d) - Error", eEftType,
				   c_szEftAttachBone, c_szEftName, isCache);
		rdwCRCEft = 0;
		return false;
	}

	return true;
}

void CInstanceBase::RegisterTitleName(int iIndex, const char * c_szTitleName)
{
	g_TitleNameMap.emplace(iIndex, c_szTitleName);
}

D3DXCOLOR __RGBToD3DXColoru(uint32_t r, uint32_t g, uint32_t b)
{
	uint32_t dwColor = 0xff;
	dwColor <<= 8;
	dwColor |= r;
	dwColor <<= 8;
	dwColor |= g;
	dwColor <<= 8;
	dwColor |= b;

	return {dwColor};
}

bool CInstanceBase::RegisterNameColor(uint32_t uIndex, uint32_t r, uint32_t g, uint32_t b)
{
	if (uIndex >= NAMECOLOR_NUM)
		return false;

	g_akD3DXClrName[uIndex] = __RGBToD3DXColoru(r, g, b);
	return true;
}

bool CInstanceBase::RegisterTitleColor(uint32_t uIndex, uint32_t r, uint32_t g, uint32_t b)
{
	if (uIndex >= TITLE_NUM)
		return false;

	g_akD3DXClrTitle[uIndex] = __RGBToD3DXColoru(r, g, b);
	return true;
}

int CInstanceBase::HasAffect(uint32_t dwIndex)
{
	if (dwIndex >= AFFECT_NUM)
		return 0;

	return m_adwCRCAffectEffect[dwIndex];
}
