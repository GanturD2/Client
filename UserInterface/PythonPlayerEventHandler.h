#pragma once

#include "../GameLib/ActorInstance.h"
#include "../GameLib/FlyHandler.h"

#include "PythonNetworkStream.h"
#include "InstanceBase.h"

class CPythonPlayerEventHandler : public CActorInstance::IEventHandler
{
public:
	static CPythonPlayerEventHandler & GetSingleton();

public:
	~CPythonPlayerEventHandler();
	CLASS_DELETE_COPYMOVE(CPythonPlayerEventHandler);

	void OnSyncing(const SState & c_rkState) override;
	void OnWaiting(const SState & c_rkState) override;
	void OnMoving(const SState & c_rkState) override;
	void OnMove(const SState & c_rkState) override;
	void OnStop(const SState & c_rkState) override;
	void OnWarp(const SState & c_rkState) override;
	void OnClearAffects() override;
	void OnSetAffect(uint32_t uAffect) override;
	void OnResetAffect(uint32_t uAffect) override;
	void OnAttack(const SState & c_rkState, uint16_t wMotionIndex) override;
	void OnUseSkill(const SState & c_rkState, uint32_t uMotSkill, uint32_t uArg) override;
	virtual void OnUpdate();
	void OnChangeShape() override;
	void OnHit(uint32_t uSkill, CActorInstance & rkActorVictim, BOOL isSendPacket) override;

	void FlushVictimList();

protected:
	CPythonPlayerEventHandler();

protected:
	struct SVictim
	{
		uint32_t m_dwVID;
		long m_lPixelX;
		long m_lPixelY;
	};

protected:
	std::vector<SVictim> m_kVctkVictim;

	uint32_t m_dwPrevComboIndex;
	uint32_t m_dwNextWaitingNotifyTime;
	uint32_t m_dwNextMovingNotifyTime;
	TPixelPosition m_kPPosPrevWaiting;

private:
	class CNormalBowAttack_FlyEventHandler_AutoClear : public IFlyEventHandler
	{
	public:
		CNormalBowAttack_FlyEventHandler_AutoClear() = default;
		~CNormalBowAttack_FlyEventHandler_AutoClear() = default;
		CNormalBowAttack_FlyEventHandler_AutoClear(const CNormalBowAttack_FlyEventHandler_AutoClear &) = delete;
		CNormalBowAttack_FlyEventHandler_AutoClear(CNormalBowAttack_FlyEventHandler_AutoClear &&) = delete;
		CNormalBowAttack_FlyEventHandler_AutoClear & operator=(const CNormalBowAttack_FlyEventHandler_AutoClear &) = delete;
		CNormalBowAttack_FlyEventHandler_AutoClear & operator=(CNormalBowAttack_FlyEventHandler_AutoClear &&) = delete;

		void Set(CPythonPlayerEventHandler * pParent, CInstanceBase * pInstMain, CInstanceBase * pInstTarget);
		void SetTarget(CInstanceBase * pInstTarget);

		void OnSetFlyTarget() override;
		void OnShoot(uint32_t dwSkillIndex) override;

		void OnNoTarget() override {}

		void OnExplodingOutOfRange() override {}

		void OnExplodingAtBackground() override {}

		void OnExplodingAtAnotherTarget(uint32_t dwSkillIndex, uint32_t dwVID) override;
		void OnExplodingAtTarget(uint32_t dwSkillIndex) override;

	protected:
		CPythonPlayerEventHandler * m_pParent;
		CInstanceBase * m_pInstMain;
		CInstanceBase * m_pInstTarget;
	} m_NormalBowAttack_FlyEventHandler_AutoClear;

public:
	IFlyEventHandler * GetNormalBowAttackFlyEventHandler(CInstanceBase * pInstMain, CInstanceBase * pInstTarget)
	{
		m_NormalBowAttack_FlyEventHandler_AutoClear.Set(this, pInstMain, pInstTarget);
		return &m_NormalBowAttack_FlyEventHandler_AutoClear;
	}

	void ChangeFlyTarget(CInstanceBase * pInstTarget) { m_NormalBowAttack_FlyEventHandler_AutoClear.SetTarget(pInstTarget); }
};