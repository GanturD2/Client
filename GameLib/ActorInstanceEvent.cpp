#include "StdAfx.h"
#include "ActorInstance.h"
#ifdef ENABLE_CSHIELD
#	include "../UserInterface/CShield.h"
#endif

void CActorInstance::__OnSyncing()
{
	IEventHandler & rkEventHandler = __GetEventHandlerRef();

	IEventHandler::SState kState;
	kState.kPPosSelf = NEW_GetCurPixelPositionRef();
	kState.fAdvRotSelf = GetAdvancingRotation();
	rkEventHandler.OnSyncing(kState);
}

void CActorInstance::__OnWaiting()
{
	assert(!IsPushing());

	IEventHandler & rkEventHandler = __GetEventHandlerRef();

	IEventHandler::SState kState;
	kState.kPPosSelf = NEW_GetCurPixelPositionRef();
	kState.fAdvRotSelf = GetAdvancingRotation();
	rkEventHandler.OnWaiting(kState);
}

void CActorInstance::__OnMoving()
{
	assert(!IsPushing());

	IEventHandler & rkEventHandler = __GetEventHandlerRef();

	const TPixelPosition & c_rkPPosCur = NEW_GetCurPixelPositionRef();
	const TPixelPosition & c_rkPPosDst = NEW_GetDstPixelPositionRef();

	TPixelPosition kPPosDir = c_rkPPosDst - c_rkPPosCur;
	float distance = sqrt(kPPosDir.x * kPPosDir.x + kPPosDir.y * kPPosDir.y);

	IEventHandler::SState kState;

	if (distance > 1000.0f)
	{
		D3DXVec3Normalize(&kPPosDir, &kPPosDir);
		D3DXVec3Scale(&kPPosDir, &kPPosDir, 1000.0f);
		D3DXVec3Add(&kState.kPPosSelf, &kPPosDir, &c_rkPPosCur);
	}
	else
		kState.kPPosSelf = c_rkPPosDst;
	kState.fAdvRotSelf = GetAdvancingRotation();
	rkEventHandler.OnMoving(kState);
}

void CActorInstance::__OnMove()
{
#if defined(ENABLE_CHECK_MOVESPEED_HACK) && !defined(_DEBUG)
	if (!CShield::Instance().CheckMovespeed(m_fMovSpd))
	{
		hackFound = TRUE;
		CShield::Instance().Close();
	}
#endif

	IEventHandler & rkEventHandler = __GetEventHandlerRef();

	IEventHandler::SState kState;
	kState.kPPosSelf = NEW_GetCurPixelPositionRef();
	kState.fAdvRotSelf = GetAdvancingRotation();
	rkEventHandler.OnMove(kState);
}

void CActorInstance::__OnStop()
{
	IEventHandler & rkEventHandler = __GetEventHandlerRef();

	IEventHandler::SState kState;
	kState.kPPosSelf = NEW_GetCurPixelPositionRef();
	kState.fAdvRotSelf = GetAdvancingRotation();
	rkEventHandler.OnStop(kState);
}

void CActorInstance::__OnWarp()
{
	IEventHandler & rkEventHandler = __GetEventHandlerRef();

	IEventHandler::SState kState;
	kState.kPPosSelf = NEW_GetCurPixelPositionRef();
	kState.fAdvRotSelf = GetAdvancingRotation();
	rkEventHandler.OnWarp(kState);
}

void CActorInstance::__OnAttack(uint16_t wMotionIndex)
{
	IEventHandler & rkEventHandler = __GetEventHandlerRef();

	IEventHandler::SState kState;
	kState.kPPosSelf = NEW_GetCurPixelPositionRef();
	kState.fAdvRotSelf = GetTargetRotation();
	rkEventHandler.OnAttack(kState, wMotionIndex);
}

void CActorInstance::__OnUseSkill(uint32_t uMotSkill, uint32_t uLoopCount, bool isMovingSkill)
{
	IEventHandler & rkEventHandler = __GetEventHandlerRef();

	IEventHandler::SState kState;
	kState.kPPosSelf = NEW_GetCurPixelPositionRef();
	kState.fAdvRotSelf = GetAdvancingRotation();

	uint32_t uArg = uLoopCount;
	if (isMovingSkill)
		uArg |= 1 << 4;

	rkEventHandler.OnUseSkill(kState, uMotSkill, uArg);
}

#ifdef ENABLE_EARTHQUAKE_EVENT
uint32_t CActorInstance::GetEarthQuakeState(uint32_t dwMobVnum)
{
	if (!dwMobVnum)
		return EARTHQUAKE_TYPE_NONE;

	switch (dwMobVnum)
	{
		case 691:	// Orc Boss
		case 1091:	// Dämonenkönig
		case 1092:	// Stolzer Dämonenkönig
		case 1093:	// Sensenmann
		case 1094:	// Gemeiner Dämonenkönig
		case 1095:	// Blauer Tod
		case 1901:	// Neunschwanz
		case 2091:	// Königinnenspinne
		case 2094:	// Spinnenbaron
		case 2491:	// Hauptmann Yonghan
		case 2492:	// General Yonghan
		case 2493:	// Beran-Setaou
		case 2591:	// Tartaros
		case 3190:	// Arges
		case 3191:	// Polyphemos
		case 3290:	// Rakshasa
		case 3390:	// Lemuren - Fürst
		case 3490:	// General Kappa
		case 3591:	// Roter Häuptling
		case 3596:	// Brutaler Roter Häuptling
		case 3790:	// Gargoyle
		case 3890:	// Kapitän Shrack
		case 3891:	// Der Große Oger
			return EARTHQUAKE_TYPE_MINIMAL;
			break;

		case 2092:	// Spinnenbaroness
		case 3391:	// Lemurische Leibgarde
		case 3690:	// General Lobster
		case 3791:	// König Wobba
		case 3911:	// Aku - Boku
		case 3912:	// Arboretum
		case 3913:	// Sycomor
		case 4101:	// Endzeit - Wächter Zi
		case 4121:	// Endzeit - Wächter Zi
		case 6191:	// Nemere
		case 6091:	// Razador
			return EARTHQUAKE_TYPE_MIDDLE;
			break;

		case 2291:	// Roter Drache
			return EARTHQUAKE_TYPE_STRONG;
			break;

		default:
			return EARTHQUAKE_TYPE_NONE;
			break;
	}
}

void CActorInstance::__IsEarthQuake()
{
	uint32_t eqType = GetEarthQuakeState(GetRace());

	if (eqType == EARTHQUAKE_TYPE_NONE)
		return;

	if (eqType == EARTHQUAKE_TYPE_MINIMAL)
		SetScreenEffectWaving(50.00f, 50);
	else if (eqType == EARTHQUAKE_TYPE_MIDDLE)
		SetScreenEffectWaving(50.00f, 100);
	else if (eqType == EARTHQUAKE_TYPE_STRONG)
		SetScreenEffectWaving(50.00f, 200);
}
#endif

void CActorInstance::__OnHit(uint32_t uSkill, CActorInstance & rkActorVictm, BOOL isSendPacket)
{
	IEventHandler & rkEventHandler = __GetEventHandlerRef();
	rkEventHandler.OnHit(uSkill, rkActorVictm, isSendPacket);
}

void CActorInstance::__OnClearAffects()
{
	IEventHandler & rkEventHandler = __GetEventHandlerRef();
	rkEventHandler.OnClearAffects();
}

void CActorInstance::__OnSetAffect(uint32_t uAffect)
{
	IEventHandler & rkEventHandler = __GetEventHandlerRef();
	rkEventHandler.OnSetAffect(uAffect);
}

void CActorInstance::__OnResetAffect(uint32_t uAffect)
{
	IEventHandler & rkEventHandler = __GetEventHandlerRef();
	rkEventHandler.OnResetAffect(uAffect);
}


CActorInstance::IEventHandler & CActorInstance::__GetEventHandlerRef()
{
	assert(m_pkEventHandler != nullptr && "CActorInstance::GetEventHandlerRef");
	return *m_pkEventHandler;
}

CActorInstance::IEventHandler * CActorInstance::__GetEventHandlerPtr()
{
	return m_pkEventHandler;
}

void CActorInstance::SetEventHandler(IEventHandler * pkEventHandler)
{
	m_pkEventHandler = pkEventHandler;
}

CActorInstance::IEventHandler * CActorInstance::IEventHandler::GetEmptyPtr()
{
	static class CEmptyEventHandler : public IEventHandler
	{
	public:
		CEmptyEventHandler() = default;
		~CEmptyEventHandler() override = default;

		void OnSyncing(const SState & c_rkState) override {}
		void OnWaiting(const SState & c_rkState) override {}
		void OnMoving(const SState & c_rkState) override {}
		void OnMove(const SState & c_rkState) override {}
		void OnStop(const SState & c_rkState) override {}
		void OnWarp(const SState & c_rkState) override {}

		void OnClearAffects() override {}
		void OnSetAffect(uint32_t uAffect) override {}
		void OnResetAffect(uint32_t uAffect) override {}

		void OnAttack(const SState & c_rkState, uint16_t wMotionIndex) override {}
		void OnUseSkill(const SState & c_rkState, uint32_t uMotSkill, uint32_t uMotLoopCount) override {}

		void OnHit(uint32_t uMotAttack, CActorInstance & rkActorVictim, BOOL isSendPacket) override {}

		void OnChangeShape() override {}

	} s_kEmptyEventHandler;

	return &s_kEmptyEventHandler;
}
