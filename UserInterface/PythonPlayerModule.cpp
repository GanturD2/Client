#include "StdAfx.h"
#include "PythonPlayer.h"
#include "PythonApplication.h"
#include "../GameLib/GameLibDefines.h"
#ifdef ENABLE_LOADING_TIP
#	include "PythonBackground.h"
#endif
#include "../GameLib/GameType.h"
#ifdef ENABLE_MINI_GAME_YUTNORI
#	include "PythonYutnoriManager.h"
#endif

extern const uint32_t c_iSkillIndex_Tongsol = 121;
extern const uint32_t c_iSkillIndex_Combo = 122;
extern const uint32_t c_iSkillIndex_Fishing = 123;
extern const uint32_t c_iSkillIndex_Mining = 124;
extern const uint32_t c_iSkillIndex_Making = 125;
extern const uint32_t c_iSkillIndex_Language1 = 126;
extern const uint32_t c_iSkillIndex_Language2 = 127;
extern const uint32_t c_iSkillIndex_Language3 = 128;
extern const uint32_t c_iSkillIndex_Polymorph = 129;
extern const uint32_t c_iSkillIndex_Riding = 130;
extern const uint32_t c_iSkillIndex_Summon = 131;
#ifdef ENABLE_PASSIVE_ATTR
extern const uint32_t c_iSkillIndex_AutoAttack = 132;
#endif

enum ERefineFailType
{
	REFINE_FAIL_GRADE_DOWN,
	REFINE_FAIL_DEL_ITEM,
	REFINE_FAIL_KEEP_GRADE,
	REFINE_FAIL_MAX,
};

enum
{
	EMOTION_CLAP = 1,
	EMOTION_CHEERS_1,
	EMOTION_CHEERS_2,
	EMOTION_DANCE_1,
	EMOTION_DANCE_2,
	EMOTION_DANCE_3,
	EMOTION_DANCE_4,
	EMOTION_DANCE_5,
	EMOTION_DANCE_6,
	EMOTION_CONGRATULATION,
	EMOTION_FORGIVE,
	EMOTION_ANGRY,

	EMOTION_ATTRACTIVE,
	EMOTION_SAD,
	EMOTION_SHY,
	EMOTION_CHEERUP,
	EMOTION_BANTER,
	EMOTION_JOY,

#ifdef ENABLE_EXPRESSING_EMOTION
	EMOTION_PUSHUP 			= 60,
	EMOTION_DANCE_7			= 61,
	EMOTION_EXERCISE		= 62,
	EMOTION_DOZE			= 63,
	EMOTION_SELFIE			= 64,
#endif

	EMOTION_KISS = 51,
	EMOTION_FRENCH_KISS,
	EMOTION_SLAP,
};

std::map<int, CGraphicImage *> m_kMap_iEmotionIndex_pkIconImage;

extern int TWOHANDED_WEWAPON_ATT_SPEED_DECREASE_VALUE;


//ENABLE_NEW_EQUIPMENT_SYSTEM
class CBeltInventoryHelper
{
public:
	using TGradeUnit = unsigned char;

	static TGradeUnit GetBeltGradeByRefineLevel(int refineLevel)
	{
		static TGradeUnit beltGradeByLevelTable[] = {
			0, // ∫ß∆Æ+0
			1, // +1
			1, // +2
			2, // +3
			2, // +4,
			3, // +5
			4, // +6,
			5, // +7,
			6, // +8,
			7, // +9
		};

		return beltGradeByLevelTable[refineLevel];
	}

	static const TGradeUnit * GetAvailableRuleTableByGrade()
	{
		static TGradeUnit availableRuleByGrade[c_Belt_Inventory_Slot_Count] =
		{
			1, 2, 4, 6, 3, 3, 4, 6, 5, 5, 5, 6, 7, 7, 7, 7
#ifdef ENABLE_BELT_INVENTORY_RENEWAL
			, 8, 8, 8, 8, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11
#endif
		};

		return availableRuleByGrade;
	}

	static bool IsAvailableCell(uint16_t cell, int beltGrade /*int beltLevel*/)
	{
		// ±‚»π ∂« πŸ≤Ò.. æ∆≥ˆ...
		//const TGradeUnit beltGrade = GetBeltGradeByRefineLevel(beltLevel);
		const TGradeUnit * ruleTable = GetAvailableRuleTableByGrade();

		return ruleTable[cell] <= beltGrade;
	}
};
//END_ENABLE_NEW_EQUIPMENT_SYSTEM

PyObject * playerPickCloseItem(PyObject * poSelf, PyObject * poArgs)
{
	CPythonPlayer::Instance().PickCloseItem();
	return Py_BuildNone();
}


PyObject * playerSetGameWindow(PyObject * poSelf, PyObject * poArgs)
{
	PyObject * pyHandle;
	if (!PyTuple_GetObject(poArgs, 0, &pyHandle))
		return Py_BadArgument();

	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
	rkPlayer.SetGameWindow(pyHandle);
	return Py_BuildNone();
}


PyObject * playerSetQuickCameraMode(PyObject * poSelf, PyObject * poArgs)
{
	int nIsEnable;
	if (!PyTuple_GetInteger(poArgs, 0, &nIsEnable))
		return Py_BadArgument();

	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
	rkPlayer.SetQuickCameraMode(nIsEnable);

	return Py_BuildNone();
}

// Test Code
PyObject * playerSetMainCharacterIndex(PyObject * poSelf, PyObject * poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	CPythonPlayer::Instance().SetMainCharacterIndex(iVID);
	CPythonCharacterManager::Instance().SetMainInstance(iVID);

	return Py_BuildNone();
}
// Test Code

PyObject * playerGetMainCharacterIndex(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetMainCharacterIndex());
}

PyObject * playerGetMainCharacterName(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("s", CPythonPlayer::Instance().GetName());
}

PyObject * playerGetMainCharacterPosition(PyObject * poSelf, PyObject * poArgs)
{
	TPixelPosition kPPosMainActor;
	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
	rkPlayer.NEW_GetMainActorPosition(&kPPosMainActor);
	return Py_BuildValue("fff", kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);
}

PyObject * playerIsMainCharacterIndex(PyObject * poSelf, PyObject * poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonPlayer::Instance().IsMainCharacterIndex(iVID));
}

#ifdef ENABLE_SPECIAL_INVENTORY
PyObject* playerGetSpecialInventoryTypeByGlobalSlot(PyObject* poSelf, PyObject* poArgs)
{
	int iSlot;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlot))
	{
		return Py_BadArgument();
	}

	int invenType = CPythonPlayer::Instance().GetSpecialInventoryTypeByGlobalSlot(iSlot);
	return Py_BuildValue("i", invenType);
}

PyObject* playerGetSpecialInventoryRange(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t invenType;
	if (!PyTuple_GetByte(poArgs, 0, &invenType))
	{
		return Py_BadArgument();
	}

	int start, end;
	std::tie(start, end) = CPythonPlayer::Instance().GetSpecialInventoryRange(invenType);
	return Py_BuildValue("(ii)", start, end);
}
#endif

PyObject * playerCanAttackInstance(PyObject * poSelf, PyObject * poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	CInstanceBase * pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
	CInstanceBase * pTargetInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVID);
	if (!pMainInstance)
		return Py_BuildValue("i", 0);
	if (!pTargetInstance)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pMainInstance->IsAttackableInstance(*pTargetInstance));
}

PyObject * playerIsActingEmotion(PyObject * poSelf, PyObject * poArgs)
{
	CInstanceBase * pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
	if (!pMainInstance)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pMainInstance->IsActingEmotion());
}

PyObject * playerIsPVPInstance(PyObject * poSelf, PyObject * poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	CInstanceBase * pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
	CInstanceBase * pTargetInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVID);
	if (!pMainInstance)
		return Py_BuildValue("i", 0);
	if (!pTargetInstance)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pMainInstance->IsPVPInstance(*pTargetInstance));
}

PyObject * playerIsSameEmpire(PyObject * poSelf, PyObject * poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	CInstanceBase * pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
	CInstanceBase * pTargetInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVID);
	if (!pMainInstance)
		return Py_BuildValue("i", FALSE);
	if (!pTargetInstance)
		return Py_BuildValue("i", FALSE);

	return Py_BuildValue("i", pMainInstance->IsSameEmpire(*pTargetInstance));
}

PyObject * playerIsChallengeInstance(PyObject * poSelf, PyObject * poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonPlayer::Instance().IsChallengeInstance(iVID));
}

PyObject * playerIsRevengeInstance(PyObject * poSelf, PyObject * poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonPlayer::Instance().IsRevengeInstance(iVID));
}

PyObject * playerIsCantFightInstance(PyObject * poSelf, PyObject * poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonPlayer::Instance().IsCantFightInstance(iVID));
}

PyObject * playerGetCharacterDistance(PyObject * poSelf, PyObject * poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	CInstanceBase * pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
	CInstanceBase * pTargetInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVID);
	if (!pMainInstance)
		return Py_BuildValue("f", -1.0f);
	if (!pTargetInstance)
		return Py_BuildValue("f", -1.0f);

	return Py_BuildValue("f", pMainInstance->GetDistance(pTargetInstance));
}

PyObject * playerIsInSafeArea(PyObject * poSelf, PyObject * poArgs)
{
	CInstanceBase * pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
	if (!pMainInstance)
		return Py_BuildValue("i", FALSE);

	return Py_BuildValue("i", pMainInstance->IsInSafe());
}

PyObject * playerIsMountingHorse(PyObject * poSelf, PyObject * poArgs)
{
	CInstanceBase * pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
	if (!pMainInstance)
		return Py_BuildValue("i", FALSE);

	return Py_BuildValue("i", pMainInstance->IsMountingHorse());
}

PyObject * playerIsObserverMode(PyObject * poSelf, PyObject * poArgs)
{
	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
	return Py_BuildValue("i", rkPlayer.IsObserverMode());
}

PyObject * playerActEmotion(PyObject * poSelf, PyObject * poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
	rkPlayer.ActEmotion(iVID);
	return Py_BuildNone();
}

PyObject * playerShowPlayer(PyObject * poSelf, PyObject * poArgs)
{
	CInstanceBase * pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
	if (pMainInstance)
		pMainInstance->GetGraphicThingInstanceRef().Show();
	return Py_BuildNone();
}

PyObject * playerHidePlayer(PyObject * poSelf, PyObject * poArgs)
{
	CInstanceBase * pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
	if (pMainInstance)
		pMainInstance->GetGraphicThingInstanceRef().Hide();
	return Py_BuildNone();
}


PyObject * playerComboAttack(PyObject * poSelf, PyObject * poArgs)
{
	CPythonPlayer::Instance().NEW_Attack();
	return Py_BuildNone();
}

PyObject * playerIsAttacking(PyObject* poSelf, PyObject* poArgs)
{
	CInstanceBase * pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();

	if (!pMainInstance)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pMainInstance->IsAttacking() ? 1 : 0);
}

PyObject * playerRegisterEffect(PyObject * poSelf, PyObject * poArgs)
{
	int iEft;
	if (!PyTuple_GetInteger(poArgs, 0, &iEft))
		return Py_BadArgument();

	char * szFileName;
	if (!PyTuple_GetString(poArgs, 1, &szFileName))
		return Py_BadArgument();

	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
	if (!rkPlayer.RegisterEffect(iEft, szFileName, false))
		return Py_BuildException("CPythonPlayer::RegisterEffect(eEft=%d, szFileName=%s", iEft, szFileName);

	return Py_BuildNone();
}

PyObject * playerRegisterCacheEffect(PyObject * poSelf, PyObject * poArgs)
{
	int iEft;
	if (!PyTuple_GetInteger(poArgs, 0, &iEft))
		return Py_BadArgument();

	char * szFileName;
	if (!PyTuple_GetString(poArgs, 1, &szFileName))
		return Py_BadArgument();

	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
	if (!rkPlayer.RegisterEffect(iEft, szFileName, true))
		return Py_BuildException("CPythonPlayer::RegisterEffect(eEft=%d, szFileName=%s", iEft, szFileName);

	return Py_BuildNone();
}

PyObject * playerSetAttackKeyState(PyObject * poSelf, PyObject * poArgs)
{
	int isPressed;
	if (!PyTuple_GetInteger(poArgs, 0, &isPressed))
		return Py_BuildException("playerSetAttackKeyState(isPressed) - There is no first argument");

	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
	rkPlayer.SetAttackKeyState(isPressed);
	return Py_BuildNone();
}

PyObject * playerSetSingleDIKKeyState(PyObject * poSelf, PyObject * poArgs)
{
	int eDIK;
	if (!PyTuple_GetInteger(poArgs, 0, &eDIK))
		return Py_BuildException("playerSetSingleDIKKeyState(eDIK, isPressed) - There is no first argument");

	int isPressed;
	if (!PyTuple_GetInteger(poArgs, 1, &isPressed))
		return Py_BuildException("playerSetSingleDIKKeyState(eDIK, isPressed) - There is no second argument");

	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
	rkPlayer.NEW_SetSingleDIKKeyState(eDIK, isPressed);
	return Py_BuildNone();
}

PyObject * playerEndKeyWalkingImmediately(PyObject * poSelf, PyObject * poArgs)
{
	CPythonPlayer::Instance().NEW_Stop();
	return Py_BuildNone();
}


PyObject * playerStartMouseWalking(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildNone();
}

PyObject * playerEndMouseWalking(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildNone();
}

PyObject * playerResetCameraRotation(PyObject * poSelf, PyObject * poArgs)
{
	CPythonPlayer::Instance().NEW_ResetCameraRotation();
	return Py_BuildNone();
}

PyObject * playerSetAutoCameraRotationSpeed(PyObject * poSelf, PyObject * poArgs)
{
	float fCmrRotSpd;
	if (!PyTuple_GetFloat(poArgs, 0, &fCmrRotSpd))
		return Py_BuildException();

	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
	rkPlayer.NEW_SetAutoCameraRotationSpeed(fCmrRotSpd);
	return Py_BuildNone();
}

PyObject * playerSetMouseState(PyObject * poSelf, PyObject * poArgs)
{
	int eMBT;
	if (!PyTuple_GetInteger(poArgs, 0, &eMBT))
		return Py_BuildException();

	int eMBS;
	if (!PyTuple_GetInteger(poArgs, 1, &eMBS))
		return Py_BuildException();

	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
	rkPlayer.NEW_SetMouseState(eMBT, eMBS);

	return Py_BuildNone();
}

PyObject * playerSetMouseFunc(PyObject * poSelf, PyObject * poArgs)
{
	int eMBT;
	if (!PyTuple_GetInteger(poArgs, 0, &eMBT))
		return Py_BuildException();

	int eMBS;
	if (!PyTuple_GetInteger(poArgs, 1, &eMBS))
		return Py_BuildException();

	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
	rkPlayer.NEW_SetMouseFunc(eMBT, eMBS);

	return Py_BuildNone();
}

PyObject * playerGetMouseFunc(PyObject * poSelf, PyObject * poArgs)
{
	int eMBT;
	if (!PyTuple_GetInteger(poArgs, 0, &eMBT))
		return Py_BuildException();

	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
	return Py_BuildValue("i", rkPlayer.NEW_GetMouseFunc(eMBT));
}

PyObject * playerSetMouseMiddleButtonState(PyObject * poSelf, PyObject * poArgs)
{
	int eMBS;
	if (!PyTuple_GetInteger(poArgs, 0, &eMBS))
		return Py_BuildException();

	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
	rkPlayer.NEW_SetMouseMiddleButtonState(eMBS);

	return Py_BuildNone();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

PyObject * playerGetName(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("s", CPythonPlayer::Instance().GetName());
}

PyObject * playerGetRace(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetRace());
}

PyObject * playerGetJob(PyObject * poSelf, PyObject * poArgs)
{
	int race = CPythonPlayer::Instance().GetRace();
	int job = RaceToJob(race);
	return Py_BuildValue("i", job);
}

PyObject * playerGetPlayTime(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetPlayTime());
}

PyObject * playerSetPlayTime(PyObject * poSelf, PyObject * poArgs)
{
	int iTime;
	if (!PyTuple_GetInteger(poArgs, 0, &iTime))
		return Py_BuildException();

	CPythonPlayer::Instance().SetPlayTime(iTime);
	return Py_BuildNone();
}

PyObject * playerIsSkillCoolTime(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonPlayer::Instance().IsSkillCoolTime(iSlotIndex));
}

PyObject * playerGetSkillCoolTime(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	float fCoolTime = CPythonPlayer::Instance().GetSkillCoolTime(iSlotIndex);
	float fElapsedCoolTime = CPythonPlayer::Instance().GetSkillElapsedCoolTime(iSlotIndex);
	return Py_BuildValue("ff", fCoolTime, fElapsedCoolTime);
}

PyObject * playerIsSkillActive(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonPlayer::Instance().IsSkillActive(iSlotIndex));
}

PyObject * playerUseGuildSkill(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillSlotIndex))
		return Py_BuildException();

	CPythonPlayer::Instance().UseGuildSkill(iSkillSlotIndex);
	return Py_BuildNone();
}

PyObject * playerAffectIndexToSkillIndex(PyObject * poSelf, PyObject * poArgs)
{
	int iAffectIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iAffectIndex))
		return Py_BuildException();

	uint32_t dwSkillIndex;
	if (!CPythonPlayer::Instance().AffectIndexToSkillIndex(iAffectIndex, &dwSkillIndex))
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", dwSkillIndex);
}

PyObject * playerGetEXP(PyObject * poSelf, PyObject * poArgs)
{
	uint32_t dwEXP = CPythonPlayer::Instance().GetStatus(POINT_EXP);
	return Py_BuildValue("l", dwEXP);
}

PyObject * playerGetStatus(PyObject * poSelf, PyObject * poArgs)
{
	int iType;
	if (!PyTuple_GetInteger(poArgs, 0, &iType))
		return Py_BuildException();

	long iValue = CPythonPlayer::Instance().GetStatus(iType);

	if (POINT_ATT_SPEED == iType)
	{
		CInstanceBase * pInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
		if (pInstance && (CItemData::WEAPON_TWO_HANDED == pInstance->GetWeaponType()))
			iValue -= TWOHANDED_WEWAPON_ATT_SPEED_DECREASE_VALUE;
	}

	return Py_BuildValue("i", iValue);
}

PyObject * playerSetStatus(PyObject * poSelf, PyObject * poArgs)
{
	int iType;
	if (!PyTuple_GetInteger(poArgs, 0, &iType))
		return Py_BuildException();

	int iValue;
	if (!PyTuple_GetInteger(poArgs, 1, &iValue))
		return Py_BuildException();

	CPythonPlayer::Instance().SetStatus(iType, iValue);
	return Py_BuildNone();
}

PyObject * playerGetElk(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetStatus(POINT_GOLD));
}

#ifdef ENABLE_CHEQUE_SYSTEM
PyObject * playerGetCheque(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetStatus(POINT_CHEQUE));
}
#endif

#ifdef ENABLE_GEM_SYSTEM
PyObject* playerGetGem(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetStatus(POINT_GEM));
}

#	if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
PyObject* playerGetGemShopItemID(PyObject* poSelf, PyObject* poArgs)
{
	int iPos;
	if (!PyTuple_GetInteger(poArgs, 0, &iPos))
		return Py_BadArgument();

	const TGemShopItem* pItemData;
	CPythonPlayer::Instance().GetGemShopItemData(iPos, &pItemData);

	return Py_BuildValue("iiii", pItemData->vnum, pItemData->price, pItemData->status, pItemData->count);
}

PyObject* playerGetGemShopOpenSlotCount(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetGemShopOpenSlotCount());
}

PyObject* playerGetGemShopOpenSlotItemCount(PyObject* poSelf, PyObject* poArgs)
{
	int iPos;
	if (!PyTuple_GetInteger(poArgs, 0, &iPos))
		return Py_BadArgument();

	return Py_BuildValue("i", CPythonPlayer::Instance().GetGemShopOpenSlotItemCount(iPos));
}

PyObject* playerGetGemShopRefreshTime(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetGemShopRefreshTime());
}

PyObject* playerIsGemShopWindowOpen(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().IsOpenGemShopWindow());
}

PyObject* playerSetGemShopWindowOpen(PyObject* poSelf, PyObject* poArgs)
{
	bool bisOpenGemShopWindow = false;
	if (!PyTuple_GetBoolean(poArgs, 0, &bisOpenGemShopWindow))
		return Py_BadArgument();

	CPythonPlayer::Instance().SetOpenGemShopWindow(bisOpenGemShopWindow);

	return Py_BuildNone();
}
#	endif
#endif

PyObject * playerGetGuildID(PyObject * poSelf, PyObject * poArgs)
{
	CInstanceBase * pInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
	if (!pInstance)
		return Py_BuildValue("i", 0);
	return Py_BuildValue("i", pInstance->GetGuildID());
}

PyObject * playerGetGuildName(PyObject * poSelf, PyObject * poArgs)
{
	CInstanceBase * pInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
	if (pInstance)
	{
		uint32_t dwID = pInstance->GetGuildID();
		std::string strName;
		if (CPythonGuild::Instance().GetGuildName(dwID, &strName))
			return Py_BuildValue("s", strName.c_str());
	}
	return Py_BuildValue("s", "");
}

PyObject * playerGetAlignmentData(PyObject * poSelf, PyObject * poArgs)
{
	CInstanceBase * pInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
	int iAlignmentPoint = 0;
	int iAlignmentGrade = 4;
	if (pInstance)
	{
		iAlignmentPoint = pInstance->GetAlignment();
		iAlignmentGrade = pInstance->GetAlignmentGrade();
	}
	return Py_BuildValue("ii", iAlignmentPoint, iAlignmentGrade);
}

PyObject * playerSetSkill(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSkillIndex))
		return Py_BuildException();

	CPythonPlayer::Instance().SetSkill(iSlotIndex, iSkillIndex);
	return Py_BuildNone();
}

PyObject * playerGetSkillIndex(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonPlayer::Instance().GetSkillIndex(iSlotIndex));
}

PyObject * playerGetSkillSlotIndex(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BuildException();

	uint32_t dwSlotIndex;
	if (!CPythonPlayer::Instance().GetSkillSlotIndex(iSkillIndex, &dwSlotIndex))
		return Py_BuildException();

	return Py_BuildValue("i", dwSlotIndex);
}

PyObject * playerGetSkillGrade(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonPlayer::Instance().GetSkillGrade(iSlotIndex));
}

PyObject * playerGetSkillLevel(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonPlayer::Instance().GetSkillLevel(iSlotIndex));
}

PyObject * playerGetSkillCurrentEfficientPercentage(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	return Py_BuildValue("f", CPythonPlayer::Instance().GetSkillCurrentEfficientPercentage(iSlotIndex));
}
PyObject * playerGetSkillNextEfficientPercentage(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	return Py_BuildValue("f", CPythonPlayer::Instance().GetSkillNextEfficientPercentage(iSlotIndex));
}

PyObject * playerClickSkillSlot(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillSlot;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillSlot))
		return Py_BadArgument();

	CPythonPlayer::Instance().ClickSkillSlot(iSkillSlot);

	return Py_BuildNone();
}

PyObject * playerChangeCurrentSkillNumberOnly(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();

	CPythonPlayer::Instance().ChangeCurrentSkillNumberOnly(iSlotIndex);

	return Py_BuildNone();
}

PyObject * playerClearSkillDict(PyObject * poSelf, PyObject * poArgs)
{
	CPythonPlayer::Instance().ClearSkillDict();
	return Py_BuildNone();
}

PyObject * playerMoveItem(PyObject * poSelf, PyObject * poArgs)
{
	TItemPos srcCell;
	TItemPos dstCell;
	switch (PyTuple_Size(poArgs))
	{
	case 2:
		// int iSourceSlotIndex;
		if (!PyTuple_GetInteger(poArgs, 0, &srcCell.cell))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &dstCell.cell))
			return Py_BuildException();
		break;
	case 4:
		if (!PyTuple_GetByte(poArgs, 0, &srcCell.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &srcCell.cell))
			return Py_BuildException();
		if (!PyTuple_GetByte(poArgs, 2, &dstCell.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 3, &dstCell.cell))
			return Py_BuildException();
	default:
		return Py_BuildException();
	}

	CPythonPlayer::Instance().MoveItemData(srcCell, dstCell);
	return Py_BuildNone();
}

PyObject * playerSendClickItemPacket(PyObject * poSelf, PyObject * poArgs)
{
	int ivid;
	if (!PyTuple_GetInteger(poArgs, 0, &ivid))
		return Py_BuildException();

	CPythonPlayer::Instance().SendClickItemPacket(ivid);
	return Py_BuildNone();
}

PyObject * playerGetItemIndex(PyObject * poSelf, PyObject * poArgs)
{
	switch (PyTuple_Size(poArgs))
	{
	case 1:
	{
		int iSlotIndex;
		if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
			return Py_BuildException();

		int ItemIndex = CPythonPlayer::Instance().GetItemIndex(TItemPos(INVENTORY, iSlotIndex));
		return Py_BuildValue("i", ItemIndex);
	}
	case 2:
	{
		TItemPos Cell;
		if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
			return Py_BuildException();

		int ItemIndex = CPythonPlayer::Instance().GetItemIndex(Cell);
		return Py_BuildValue("i", ItemIndex);
	}
	default:
		return Py_BuildException();
	}
}

PyObject * playerGetItemFlags(PyObject * poSelf, PyObject * poArgs)
{
	switch (PyTuple_Size(poArgs))
	{
	case 1:
	{
		int iSlotIndex;
		if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
			return Py_BuildException();

		uint32_t flags = CPythonPlayer::Instance().GetItemFlags(TItemPos(INVENTORY, iSlotIndex));
		return Py_BuildValue("i", flags);
	}
	case 2:
	{
		TItemPos Cell;
		if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
			return Py_BuildException();

		if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
			return Py_BuildException();

		uint32_t flags = CPythonPlayer::Instance().GetItemFlags(Cell);
		return Py_BuildValue("i", flags);
	}
	default:
		return Py_BuildException();
	}
}


PyObject * playerGetItemCount(PyObject * poSelf, PyObject * poArgs)
{
	switch (PyTuple_Size(poArgs))
	{
	case 1:
	{
		int iSlotIndex;
		if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
			return Py_BuildException();

		int ItemNum = CPythonPlayer::Instance().GetItemCount(TItemPos(INVENTORY, iSlotIndex));
		return Py_BuildValue("i", ItemNum);
	}
	case 2:
	{
		TItemPos Cell;
		if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
			return Py_BuildException();

		if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
			return Py_BuildException();

		int ItemNum = CPythonPlayer::Instance().GetItemCount(Cell);

		return Py_BuildValue("i", ItemNum);
	}
	default:
		return Py_BuildException();
	}
}

PyObject * playerSetItemCount(PyObject * poSelf, PyObject * poArgs)
{
	switch (PyTuple_Size(poArgs))
	{
	case 2:
	{
		int iSlotIndex;
		if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
			return Py_BuildException();

		uint8_t bCount;
		if (!PyTuple_GetInteger(poArgs, 1, &bCount))
			return Py_BuildException();

		if (0 == bCount)
			return Py_BuildException();

		CPythonPlayer::Instance().SetItemCount(TItemPos(INVENTORY, iSlotIndex), bCount);
		return Py_BuildNone();
	}
	case 3:
	{
		TItemPos Cell;
		if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
			return Py_BuildException();

		if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
			return Py_BuildException();

		uint8_t bCount;
		if (!PyTuple_GetInteger(poArgs, 2, &bCount))
			return Py_BuildException();

		CPythonPlayer::Instance().SetItemCount(Cell, bCount);

		return Py_BuildNone();
	}
	default:
		return Py_BuildException();
	}
}

PyObject * playerGetItemCountByVnum(PyObject * poSelf, PyObject * poArgs)
{
	int ivnum;
	if (!PyTuple_GetInteger(poArgs, 0, &ivnum))
		return Py_BuildException();

#if defined(ENABLE_CUBE_RENEWAL) && defined(ENABLE_SET_ITEM)
	int ignore_set_value;
	if (!PyTuple_GetInteger(poArgs, 1, &ignore_set_value))
		ignore_set_value = 0;

	int ItemNum = CPythonPlayer::Instance().GetItemCountByVnum(ivnum, ignore_set_value);
#else
	int ItemNum = CPythonPlayer::Instance().GetItemCountByVnum(ivnum);
#endif
	return Py_BuildValue("i", ItemNum);
}

PyObject * playerGetItemMetinSocket(PyObject * poSelf, PyObject * poArgs)
{
	TItemPos Cell;
	int iMetinSocketIndex;

	switch (PyTuple_Size(poArgs))
	{
	case 2:
		if (!PyTuple_GetInteger(poArgs, 0, &Cell.cell))
			return Py_BuildException();

		if (!PyTuple_GetInteger(poArgs, 1, &iMetinSocketIndex))
			return Py_BuildException();

		break;
	case 3:
		if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 2, &iMetinSocketIndex))
			return Py_BuildException();

		break;

	default:
		return Py_BuildException();
	}
	int nMetinSocketValue = CPythonPlayer::Instance().GetItemMetinSocket(Cell, iMetinSocketIndex);
	return Py_BuildValue("i", nMetinSocketValue);
}

PyObject * playerGetItemAttribute(PyObject * poSelf, PyObject * poArgs)
{
	TItemPos Cell;
	// int iSlotPos;
	int iAttributeSlotIndex;
	switch (PyTuple_Size(poArgs))
	{
	case 2:
		if (!PyTuple_GetInteger(poArgs, 0, &Cell.cell))
			return Py_BuildException();

		if (!PyTuple_GetInteger(poArgs, 1, &iAttributeSlotIndex))
			return Py_BuildException();

		break;
	case 3:
		if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
			return Py_BuildException();

		if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
			return Py_BuildException();

		if (!PyTuple_GetInteger(poArgs, 2, &iAttributeSlotIndex))
			return Py_BuildException();
		break;
	default:
		return Py_BuildException();
	}
	uint16_t wType;	//@fixme436
	int16_t sValue;
	CPythonPlayer::Instance().GetItemAttribute(Cell, iAttributeSlotIndex, &wType, &sValue);

	return Py_BuildValue("ii", wType, sValue);
}

#ifdef ENABLE_YOHARA_SYSTEM
PyObject* playerGetConquerorEXP(PyObject* poSelf, PyObject* poArgs)
{
	uint32_t dwEXP = CPythonPlayer::Instance().GetStatus(POINT_CONQUEROR_EXP);
	return Py_BuildValue("l", dwEXP);
}

PyObject* playerGetItemApplyRandom(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos Cell;
	// int iSlotPos;
	int iAttributeSlotIndex;
	switch (PyTuple_Size(poArgs))
	{
		case 2:
			if (!PyTuple_GetInteger(poArgs, 0, &Cell.cell))
				return Py_BuildException();

			if (!PyTuple_GetInteger(poArgs, 1, &iAttributeSlotIndex))
				return Py_BuildException();
			break;

		case 3:
			if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
				return Py_BuildException();

			if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
				return Py_BuildException();

			if (!PyTuple_GetInteger(poArgs, 2, &iAttributeSlotIndex))
				return Py_BuildException();
			break;

		default:
			return Py_BuildException();
	}

	uint16_t wType;	//@fixme436
	int16_t sValue;
	CPythonPlayer::Instance().GetItemApplyRandom(Cell, iAttributeSlotIndex, &wType, &sValue);
	return Py_BuildValue("ii", wType, sValue);
}

PyObject* playerGetRandomValue(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos Cell;
	int iRandomValueIndex;

	switch (PyTuple_Size(poArgs))
	{
		case 2:
			if (!PyTuple_GetInteger(poArgs, 0, &Cell.cell))
				return Py_BuildException();

			if (!PyTuple_GetInteger(poArgs, 1, &iRandomValueIndex))
				return Py_BuildException();

			break;
		case 3:
			if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
				return Py_BuildException();
			if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
				return Py_BuildException();
			if (!PyTuple_GetInteger(poArgs, 2, &iRandomValueIndex))
				return Py_BuildException();

			break;

		default:
			return Py_BuildException();
	}
	int nRandomValue = CPythonPlayer::Instance().GetRandomDefaultAttrs(Cell, iRandomValueIndex);
	return Py_BuildValue("i", nRandomValue);
}
#endif

PyObject* playerGetItemLink(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos Cell;

	switch (PyTuple_Size(poArgs))
	{
		case 1:
		{
			if (!PyTuple_GetInteger(poArgs, 0, &Cell.cell))
				return Py_BuildException();
		}
		break;

		case 2:
		{
			if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
				return Py_BuildException();
			if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
				return Py_BuildException();
		}
		break;

		default:
			return Py_BuildException();
	}

	const TItemData* pPlayerItem = CPythonPlayer::Instance().GetItemData(Cell);
	CItemData* pItemData = nullptr;
	char buf[1024];

	if (pPlayerItem && CItemManager::Instance().GetItemDataPointer(pPlayerItem->vnum, &pItemData))
	{
		char itemlink[256];
		bool isAttr = false;

		// "item:%x:%d:%x:%x:%x:%x:%x:%x:%x",
		int len = snprintf(itemlink, sizeof(itemlink),
			"item"
			":%x"	// HYPER_LINK_ITEM_VNUM
#ifdef ENABLE_SET_ITEM
			":%d"	// HYPER_LINK_ITEM_PRE_SET_VALUE
#endif
			":%x"	// HYPER_LINK_ITEM_FLAGS
			":%x"	// HYPER_LINK_ITEM_SOCKET0
			":%x"	// HYPER_LINK_ITEM_SOCKET1
			":%x",	// HYPER_LINK_ITEM_SOCKET2
			pPlayerItem->vnum,
#ifdef ENABLE_SET_ITEM
			0, // UNKNOWN
#endif
			pPlayerItem->flags,
			pPlayerItem->alSockets[0],
			pPlayerItem->alSockets[1],
			pPlayerItem->alSockets[2]);

#if (defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_YOHARA_SYSTEM)) || (defined(ENABLE_PROTO_RENEWAL))
		len += snprintf(itemlink + len, sizeof(itemlink) - len,
			":%x"	// HYPER_LINK_ITEM_SOCKET3
			":%x"	// HYPER_LINK_ITEM_SOCKET4
			":%x",	// HYPER_LINK_ITEM_SOCKET5
			pPlayerItem->alSockets[3],
			pPlayerItem->alSockets[4],
			pPlayerItem->alSockets[5]);
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		len += snprintf(itemlink + len, sizeof(itemlink) - len, ":%x", pPlayerItem->dwTransmutationVnum); // HYPER_LINK_ITEM_CHANGE_ITEM_VNUM
#endif

#ifdef ENABLE_REFINE_ELEMENT
		len += snprintf(itemlink + len, sizeof(itemlink) - len, ":%x", pPlayerItem->grade_element); // NOT_OFFICIAL

		len += snprintf(itemlink + len, sizeof(itemlink) - len, ":%x", pPlayerItem->element_type_bonus); // HYPER_LINK_ITEM_REFINE_ELEMENT_APPLY_TYPE

		len += snprintf(itemlink + len, sizeof(itemlink) - len, ":%x", pPlayerItem->attack_element[pPlayerItem->grade_element - 1]); // HYPER_LINK_ITEM_REFINE_ELEMENT_VALUE0
		len += snprintf(itemlink + len, sizeof(itemlink) - len, ":%x", 0); // HYPER_LINK_ITEM_REFINE_ELEMENT_VALUE1
		len += snprintf(itemlink + len, sizeof(itemlink) - len, ":%x", 0); // HYPER_LINK_ITEM_REFINE_ELEMENT_VALUE2

		len += snprintf(itemlink + len, sizeof(itemlink) - len, ":%x", pPlayerItem->elements_value_bonus[pPlayerItem->grade_element - 1]); // HYPER_LINK_ITEM_REFINE_ELEMENT_BONUS_VALUE0
		len += snprintf(itemlink + len, sizeof(itemlink) - len, ":%x", 0); // HYPER_LINK_ITEM_REFINE_ELEMENT_BONUS_VALUE1
		len += snprintf(itemlink + len, sizeof(itemlink) - len, ":%x", 0); // HYPER_LINK_ITEM_REFINE_ELEMENT_BONUS_VALUE2
#endif

#ifdef ENABLE_YOHARA_SYSTEM
		for (auto& s : pPlayerItem->aApplyRandom)
		{
			len += snprintf(itemlink + len, sizeof(itemlink) - len,
				":%x" // HYPER_LINK_ITEM_APPLY_RANDOM_TYPE0~3
				":%d", // HYPER_LINK_ITEM_APPLY_RANDOM_VALUE0~3
				s.wType, s.sValue);
		}
#endif

		for (auto& i : pPlayerItem->aAttr)
		{
			if (i.wType != 0)
			{
				len += snprintf(itemlink + len, sizeof(itemlink) - len,
					":%x"	// HYPER_LINK_ITEM_ATTRIBUTE_TYPE
					":%d",	// HYPER_LINK_ITEM_ATTRIBUTE_VALUE
					i.wType, i.sValue);

				isAttr = true;
			}
		}

#ifdef ENABLE_YOHARA_SYSTEM
		len += snprintf(itemlink + len, sizeof(itemlink) - len,
			":%x"	// HYPER_LINK_ITEM_RANDOM_VALUE0
			":%x"	// HYPER_LINK_ITEM_RANDOM_VALUE1
			":%x",	// HYPER_LINK_ITEM_RANDOM_VALUE2
			":%x",	// HYPER_LINK_ITEM_RANDOM_VALUE3
			pPlayerItem->alRandomValues[0],
			pPlayerItem->alRandomValues[1],
			pPlayerItem->alRandomValues[2],
			pPlayerItem->alRandomValues[3]);
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
		if ((pItemData->GetType() == CItemData::ITEM_TYPE_PET) &&
			(pItemData->GetSubType() == CItemData::PET_UPBRINGING || pItemData->GetSubType() == CItemData::PET_BAG))
		{
			memset(itemlink, 0, sizeof(itemlink));

			const auto petInfo = CPythonPlayer::Instance().GetPetInfo(pPlayerItem->alSockets[2]);
			if (petInfo.pet_id == 0)
			{
				int len = snprintf(itemlink, sizeof(itemlink),
					"item:%x"
#ifdef ENABLE_SET_ITEM
					":%d"	// HYPER_LINK_ITEM_PRE_SET_VALUE
#endif
					":%x:%x:%x:%x",
					pPlayerItem->vnum,
#ifdef ENABLE_SET_ITEM
					0, // UNKNOWN
#endif
					pPlayerItem->flags, 0, 0, 0);

#if (defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_YOHARA_SYSTEM)) || (defined(ENABLE_PROTO_RENEWAL))
				len += snprintf(itemlink + len, sizeof(itemlink) - len, ":%x:%x:%x", 0, 0, 0);
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
				len += snprintf(itemlink + len, sizeof(itemlink) - len, ":%x", 0);
#endif

#ifdef ENABLE_REFINE_ELEMENT
				len += snprintf(itemlink + len, sizeof(itemlink) - len, ":%x", 0); // NOT_OFFICIAL
				len += snprintf(itemlink + len, sizeof(itemlink) - len, ":%x:%x:%x:%x:%x:%x:%x", 0, 0, 0, 0, 0, 0, 0);
#endif

#ifdef ENABLE_YOHARA_SYSTEM
				for (auto& s : pPlayerItem->aApplyRandom)
					len += snprintf(itemlink + len, sizeof(itemlink) - len, ":%x:%d", 0, 0);
#endif

				for (auto& i : pPlayerItem->aAttr)
					len += snprintf(itemlink + len, sizeof(itemlink) - len, ":%x:%d", 0, 0);

#ifdef ENABLE_YOHARA_SYSTEM
				len += snprintf(itemlink + len, sizeof(itemlink) - len, ":%x:%x:%x:%x", 0, 0, 0, 0);
#endif
			}
			else
			{
				len = snprintf(itemlink, sizeof(itemlink),
					"itempet"
					":%x"	// HYPER_LINK_ITEM_PET_VNUM
#ifdef ENABLE_SET_ITEM
					":%d"	// HYPER_LINK_ITEM_PET_PRE_SET_VALUE
#endif
					":%x"	// HYPER_LINK_ITEM_PET_FLAGS
					":%x"	// HYPER_LINK_ITEM_PET_SOCKET0
					":%x"	// HYPER_LINK_ITEM_PET_SOCKET1
					":%x"	// HYPER_LINK_ITEM_PET_SOCKET2
					":%x"	// HYPER_LINK_ITEM_PET_SOCKET3
					":%x"	// HYPER_LINK_ITEM_PET_SOCKET4
					":%x"	// HYPER_LINK_ITEM_PET_SOCKET5
					":%x"	// HYPER_LINK_ITEM_PET_LEVEL
					":%x"	// HYPER_LINK_ITEM_PET_AGE
					":%x"	// HYPER_LINK_ITEM_PET_EVOL_LEVEL
					":%f"	// HYPER_LINK_ITEM_PET_HP
					":%f"	// HYPER_LINK_ITEM_PET_DEF
					":%f"	// HYPER_LINK_ITEM_PET_SP
					":%ld"	// HYPER_LINK_ITEM_PET_LIFE 
					":%ld"	// HYPER_LINK_ITEM_PET_LIFE_MAX
					":%x"	// HYPER_LINK_ITEM_PET_SKILL_COUNT
					":%x"	// HYPER_LINK_ITEM_PET_BAG_PET_VNUM
					":%s",	// HYPER_LINK_ITEM_PET_NICKNAME
					pPlayerItem->vnum,
#ifdef ENABLE_SET_ITEM
					0,
#endif
					pPlayerItem->flags,
					pPlayerItem->alSockets[0],
					pPlayerItem->alSockets[1],
					pPlayerItem->alSockets[2],
					pPlayerItem->alSockets[3],
					pPlayerItem->alSockets[4],
					pPlayerItem->alSockets[5],
					petInfo.pet_level,
					petInfo.pet_birthday,
					petInfo.evol_level,
					petInfo.pet_hp,
					petInfo.pet_def,
					petInfo.pet_sp,
					0,
					0,
					petInfo.skill_count,
					petInfo.pet_vnum,
					petInfo.pet_nick
				);

				for (int j = 0; j < 3; ++j)
				{
					len += snprintf(itemlink + len, sizeof(itemlink) - len, ":%x:%x",
						petInfo.skill_vnum[j], petInfo.skill_level[j]);
				}
			}
		}
#endif

		if (GetDefaultCodePage() == CP_ARABIC)
		{
			if (isAttr) //"item:Number: Flag: Socket 0: Socket 1: Socket 2"
				snprintf(buf, sizeof(buf), " |h|r[%s]|cffffc700|H%s|h", pItemData->GetName(), itemlink);
			else
				snprintf(buf, sizeof(buf), " |h|r[%s]|cfff1e6c0|H%s|h", pItemData->GetName(), itemlink);
		}
		else
		{
			if (isAttr) //"item:Number: Flag: Socket 0: Socket 1: Socket 2"
				snprintf(buf, sizeof(buf), "|cffffc700|H%s|h[%s]|h|r", itemlink, pItemData->GetName());
			else
				snprintf(buf, sizeof(buf), "|cfff1e6c0|H%s|h[%s]|h|r", itemlink, pItemData->GetName());
		}
	}
	else
		buf[0] = '\0';

	return Py_BuildValue("s", buf);
}


PyObject * playerGetISellItemPrice(PyObject * poSelf, PyObject * poArgs)
{
	TItemPos Cell;

	switch (PyTuple_Size(poArgs))
	{
	case 1:
		if (!PyTuple_GetInteger(poArgs, 0, &Cell.cell))
			return Py_BuildException();
		break;
	case 2:
		if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
			return Py_BuildException();
		break;
	default:
		return Py_BuildException();
	}
	CItemData * pItemData;

	if (!CItemManager::Instance().GetItemDataPointer(CPythonPlayer::Instance().GetItemIndex(Cell), &pItemData))
		return Py_BuildValue("i", 0);

	int iPrice;

	if (pItemData->IsFlag(CItemData::ITEM_FLAG_COUNT_PER_1GOLD))
		iPrice = CPythonPlayer::Instance().GetItemCount(Cell) / pItemData->GetIBuyItemPrice();
	else
		iPrice = pItemData->GetIBuyItemPrice() * CPythonPlayer::Instance().GetItemCount(Cell);

	return Py_BuildValue("i", iPrice);
}

PyObject * playerGetQuickPage(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetQuickPage());
}

PyObject * playerSetQuickPage(PyObject * poSelf, PyObject * poArgs)
{
	int iPageIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iPageIndex))
		return Py_BuildException();

	CPythonPlayer::Instance().SetQuickPage(iPageIndex);
	return Py_BuildNone();
}

PyObject * playerLocalQuickSlotIndexToGlobalQuickSlotIndex(PyObject * poSelf, PyObject * poArgs)
{
	int iLocalSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iLocalSlotIndex))
		return Py_BuildException();

	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
	return Py_BuildValue("i", rkPlayer.LocalQuickSlotIndexToGlobalQuickSlotIndex(iLocalSlotIndex));
}


PyObject * playerGetLocalQuickSlot(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	uint32_t dwWndType;
	uint32_t dwWndItemPos;

	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
	rkPlayer.GetLocalQuickSlotData(iSlotIndex, &dwWndType, &dwWndItemPos);

	return Py_BuildValue("ii", dwWndType, dwWndItemPos);
}

PyObject * playerGetGlobalQuickSlot(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	uint32_t dwWndType;
	uint32_t dwWndItemPos;

	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
	rkPlayer.GetGlobalQuickSlotData(iSlotIndex, &dwWndType, &dwWndItemPos);

	return Py_BuildValue("ii", dwWndType, dwWndItemPos);
}


PyObject * playerRequestAddLocalQuickSlot(PyObject * poSelf, PyObject * poArgs)
{
	int nSlotIndex;
	int nWndType;
	int nWndItemPos;

	if (!PyTuple_GetInteger(poArgs, 0, &nSlotIndex))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &nWndType))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 2, &nWndItemPos))
		return Py_BuildException();

	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
	rkPlayer.RequestAddLocalQuickSlot(nSlotIndex, nWndType, nWndItemPos);

	return Py_BuildNone();
}

PyObject * playerRequestAddToEmptyLocalQuickSlot(PyObject * poSelf, PyObject * poArgs)
{
	int nWndType;
	if (!PyTuple_GetInteger(poArgs, 0, &nWndType))
		return Py_BuildException();

	int nWndItemPos;
	if (!PyTuple_GetInteger(poArgs, 1, &nWndItemPos))
		return Py_BuildException();

	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
	rkPlayer.RequestAddToEmptyLocalQuickSlot(nWndType, nWndItemPos);

	return Py_BuildNone();
}

PyObject * playerRequestDeleteGlobalQuickSlot(PyObject * poSelf, PyObject * poArgs)
{
	int nGlobalSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &nGlobalSlotIndex))
		return Py_BuildException();

	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
	rkPlayer.RequestDeleteGlobalQuickSlot(nGlobalSlotIndex);
	return Py_BuildNone();
}

PyObject * playerRequestMoveGlobalQuickSlotToLocalQuickSlot(PyObject * poSelf, PyObject * poArgs)
{
	int nGlobalSrcSlotIndex;
	int nLocalDstSlotIndex;

	if (!PyTuple_GetInteger(poArgs, 0, &nGlobalSrcSlotIndex))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &nLocalDstSlotIndex))
		return Py_BuildException();

	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
	rkPlayer.RequestMoveGlobalQuickSlotToLocalQuickSlot(nGlobalSrcSlotIndex, nLocalDstSlotIndex);
	return Py_BuildNone();
}

PyObject * playerRequestUseLocalQuickSlot(PyObject * poSelf, PyObject * poArgs)
{
	int iLocalPosition;
	if (!PyTuple_GetInteger(poArgs, 0, &iLocalPosition))
		return Py_BuildException();

	CPythonPlayer::Instance().RequestUseLocalQuickSlot(iLocalPosition);

	return Py_BuildNone();
}

PyObject * playerRemoveQuickSlotByValue(PyObject * poSelf, PyObject * poArgs)
{
	int iType;
	if (!PyTuple_GetInteger(poArgs, 0, &iType))
		return Py_BuildException();

	int iPosition;
	if (!PyTuple_GetInteger(poArgs, 1, &iPosition))
		return Py_BuildException();

	CPythonPlayer::Instance().RemoveQuickSlotByValue(iType, iPosition);

	return Py_BuildNone();
}

PyObject* playerisItem(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonPlayer::Instance().IsItem(TItemPos(INVENTORY, iSlotIndex)));
}

//ENABLE_NEW_EQUIPMENT_SYSTEM
PyObject* playerIsBeltInventorySlot(PyObject* poSelf, PyObject* poArgs) // Deprecated since uiNewInventory.py
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonPlayer::Instance().IsBeltInventorySlot(TItemPos(INVENTORY, iSlotIndex)));
}
//END_ENABLE_NEW_EQUIPMENT_SYSTEM

PyObject* playerIsEquipmentSlot(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bWindowType;
	if (!PyTuple_GetInteger(poArgs, 0, &bWindowType))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	if (bWindowType != EQUIPMENT || iSlotIndex >= c_Equipment_Slot_Count)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", 1);
}

PyObject* playerIsDSEquipmentSlot(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bWindowType;
	if (!PyTuple_GetInteger(poArgs, 0, &bWindowType))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	if (bWindowType != EQUIPMENT || iSlotIndex < c_Wear_Max
		|| iSlotIndex >= c_Equipment_Slot_Count)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", 1);
}

PyObject* playerIsCostumeSlot(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	if (iSlotIndex < c_Costume_Slot_Start || iSlotIndex > c_Costume_Slot_End)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", 1);
}

PyObject * playerIsOpenPrivateShop(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().IsOpenPrivateShop());
}

PyObject * playerIsValuableItem(PyObject * poSelf, PyObject * poArgs)
{
	TItemPos SlotIndex;

	switch (PyTuple_Size(poArgs))
	{
	case 1:
		if (!PyTuple_GetInteger(poArgs, 0, &SlotIndex.cell))
			return Py_BuildException();
		break;
	case 2:
		if (!PyTuple_GetInteger(poArgs, 0, &SlotIndex.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &SlotIndex.cell)) // @fixme013 arg idx from 0 to 1
			return Py_BuildException();
		break;
	default:
		return Py_BuildException();
	}

	uint32_t dwItemIndex = CPythonPlayer::Instance().GetItemIndex(SlotIndex);
	CItemManager::Instance().SelectItemData(dwItemIndex);
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find item data");

	BOOL hasMetinSocket = FALSE;
	BOOL isHighPrice = FALSE;

	for (int i = 0; i < METIN_SOCKET_COUNT; ++i)
		if (CPythonPlayer::METIN_SOCKET_TYPE_NONE != CPythonPlayer::Instance().GetItemMetinSocket(SlotIndex, i))
			hasMetinSocket = TRUE;

	uint32_t dwValue = pItemData->GetISellItemPrice();
	if (dwValue > 5000)
		isHighPrice = TRUE;

	return Py_BuildValue("i", hasMetinSocket || isHighPrice);
}

int GetItemGrade(const char * c_szItemName)
{
	std::string strName = c_szItemName;
	if (strName.empty())
		return 0;

	char chGrade = strName[strName.length() - 1];
	if (chGrade < '0' || chGrade > '9')
		chGrade = '0';

#ifdef ENABLE_REFINE_ELEMENT
	char chGrade_ex = strName[strName.length() - 2];

	int iGrade;
	if (chGrade >= '0' && chGrade <= '9' && chGrade_ex != '1')
		iGrade = chGrade - '0';
	else if (chGrade_ex == '1' && (chGrade >= '0' && chGrade <= '5'))
		iGrade = 10 + (chGrade - '0');
#else
	int iGrade = chGrade - '0';
#endif
	return iGrade;
}

PyObject * playerGetItemGrade(PyObject * poSelf, PyObject * poArgs)
{
	TItemPos SlotIndex;
	switch (PyTuple_Size(poArgs))
	{
	case 1:
		if (!PyTuple_GetInteger(poArgs, 0, &SlotIndex.cell))
			return Py_BuildException();
		break;
	case 2:
		if (!PyTuple_GetInteger(poArgs, 0, &SlotIndex.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &SlotIndex.cell))
			return Py_BuildException();
		break;
	default:
		return Py_BuildException();
	}

	int iItemIndex = CPythonPlayer::Instance().GetItemIndex(SlotIndex);
	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find item data");

	return Py_BuildValue("i", GetItemGrade(pItemData->GetName()));
}

enum
{
	REFINE_SCROLL_TYPE_MAKE_SOCKET = 1,
	REFINE_SCROLL_TYPE_UP_GRADE = 2
};

enum
{
	REFINE_CANT,
	REFINE_OK,
	REFINE_ALREADY_MAX_SOCKET_COUNT,
	REFINE_NEED_MORE_GOOD_SCROLL,
	REFINE_CANT_MAKE_SOCKET_ITEM,
	REFINE_NOT_NEXT_GRADE_ITEM,
	REFINE_CANT_REFINE_METIN_TO_EQUIPMENT,
	REFINE_CANT_REFINE_ROD,
#ifdef ENABLE_AURA_SYSTEM
	REFINE_CANT_REFINE_AURA_ITEM,
#endif
#ifdef __UNIMPLEMENTED_EXTEND__
	REFINE_CANT_DD_ITEM,
	REFINE_CANT_NO_DD_ITEM,
#endif
};

PyObject * playerCanRefine(PyObject * poSelf, PyObject * poArgs)
{
	int iScrollItemIndex;
	TItemPos TargetSlotIndex;

	switch (PyTuple_Size(poArgs))
	{
	case 2:
		if (!PyTuple_GetInteger(poArgs, 0, &iScrollItemIndex))
			return Py_BadArgument();
		if (!PyTuple_GetInteger(poArgs, 1, &TargetSlotIndex.cell))
			return Py_BadArgument();
		break;
	case 3:
		if (!PyTuple_GetInteger(poArgs, 0, &iScrollItemIndex))
			return Py_BadArgument();
		if (!PyTuple_GetInteger(poArgs, 1, &TargetSlotIndex.window_type))
			return Py_BadArgument();
		if (!PyTuple_GetInteger(poArgs, 2, &TargetSlotIndex.cell))
			return Py_BadArgument();
		break;
	default:
		return Py_BadArgument();
	}

	if (CPythonPlayer::Instance().IsEquipmentSlot(TargetSlotIndex))
		return Py_BuildValue("i", REFINE_CANT_REFINE_METIN_TO_EQUIPMENT);

	// Scroll
	CItemManager::Instance().SelectItemData(iScrollItemIndex);
	CItemData * pScrollItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pScrollItemData)
		return Py_BuildValue("i", REFINE_CANT);
	int iScrollType = pScrollItemData->GetType();
	int iScrollSubType = pScrollItemData->GetSubType();
	if (iScrollType != CItemData::ITEM_TYPE_USE)
		return Py_BuildValue("i", REFINE_CANT);
	if (iScrollSubType != CItemData::USE_TUNING)
		return Py_BuildValue("i", REFINE_CANT);

	// Target Item
	int iTargetItemIndex = CPythonPlayer::Instance().GetItemIndex(TargetSlotIndex);
	CItemManager::Instance().SelectItemData(iTargetItemIndex);
	CItemData * pTargetItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pTargetItemData)
		return Py_BuildValue("i", REFINE_CANT);
	int iTargetType = pTargetItemData->GetType();
	//int iTargetSubType = pTargetItemData->GetSubType();
	if (CItemData::ITEM_TYPE_ROD == iTargetType)
		return Py_BuildValue("i", REFINE_CANT_REFINE_ROD);

#ifdef ENABLE_AURA_SYSTEM
	if (pTargetItemData->GetType() == CItemData::ITEM_TYPE_COSTUME && pTargetItemData->GetSubType() == CItemData::COSTUME_AURA)
		return Py_BuildValue("i", REFINE_CANT_REFINE_AURA_ITEM);
#endif

#ifdef ENABLE_SOUL_SYSTEM
	if ((pScrollItemData->GetValue(0) == 8 || pScrollItemData->GetValue(0) == 9) && (CItemData::ITEM_TYPE_SOUL != iTargetType))
		return Py_BuildValue("i", REFINE_CANT);

	if ((pScrollItemData->GetValue(0) != 8 && pScrollItemData->GetValue(0) != 9) && (CItemData::ITEM_TYPE_SOUL == iTargetType))
		return Py_BuildValue("i", REFINE_CANT);
#endif

	if (pTargetItemData->HasNextGrade())
		return Py_BuildValue("i", REFINE_OK);
	return Py_BuildValue("i", REFINE_NOT_NEXT_GRADE_ITEM);
}

enum
{
	ATTACH_METIN_CANT,
	ATTACH_METIN_OK,
	ATTACH_METIN_NOT_MATCHABLE_ITEM,
	ATTACH_METIN_NO_MATCHABLE_SOCKET,
	ATTACH_METIN_NOT_EXIST_GOLD_SOCKET,
	ATTACH_METIN_CANT_ATTACH_TO_EQUIPMENT
};

PyObject* playerCanAttachMetin(PyObject* poSelf, PyObject* poArgs)
{
	int iMetinItemID;
	TItemPos TargetSlotIndex;

	switch (PyTuple_Size(poArgs))
	{
	case 2:
		if (!PyTuple_GetInteger(poArgs, 0, &iMetinItemID))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &TargetSlotIndex.cell))
			return Py_BuildException();
		break;
	case 3:
		if (!PyTuple_GetInteger(poArgs, 0, &iMetinItemID))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &TargetSlotIndex.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 2, &TargetSlotIndex.cell))
			return Py_BuildException();
		break;
	default:
		return Py_BuildException();
	}
	if (CPythonPlayer::Instance().IsEquipmentSlot(TargetSlotIndex))
		return Py_BuildValue("i", ATTACH_METIN_CANT_ATTACH_TO_EQUIPMENT);

	CItemData* pMetinItemData;
	if (!CItemManager::Instance().GetItemDataPointer(iMetinItemID, &pMetinItemData))
		return Py_BuildException("can't find item data");

	const uint32_t dwTargetItemIndex = CPythonPlayer::Instance().GetItemIndex(TargetSlotIndex);
	CItemData* pTargetItemData;
	if (!CItemManager::Instance().GetItemDataPointer(dwTargetItemIndex, &pTargetItemData))
		return Py_BuildException("can't find item data");

	const uint32_t dwMetinWearFlags = pMetinItemData->GetWearFlags();
	const uint32_t dwTargetWearFlags = pTargetItemData->GetWearFlags();
	if (0 == (dwMetinWearFlags & dwTargetWearFlags))
		return Py_BuildValue("i", ATTACH_METIN_NOT_MATCHABLE_ITEM);
	if (CItemData::ITEM_TYPE_ROD == pTargetItemData->GetType())
		return Py_BuildValue("i", ATTACH_METIN_CANT);

	BOOL bNotExistGoldSocket = FALSE;

	int iSubType = pMetinItemData->GetSubType();
#ifdef ENABLE_PROTO_RENEWAL
	for (int i = 0; i < METIN_SOCKET_MAX_NUM; ++i)
#else
	for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
#endif
	{
		const uint32_t dwSocketType = CPythonPlayer::Instance().GetItemMetinSocket(TargetSlotIndex, i);
		if (CItemData::METIN_NORMAL == iSubType)
		{
			if (CPythonPlayer::METIN_SOCKET_TYPE_SILVER == dwSocketType || CPythonPlayer::METIN_SOCKET_TYPE_GOLD == dwSocketType)
				return Py_BuildValue("i", ATTACH_METIN_OK);
		}
		else if (CItemData::METIN_GOLD == iSubType)
		{
			if (CPythonPlayer::METIN_SOCKET_TYPE_GOLD == dwSocketType)
				return Py_BuildValue("i", ATTACH_METIN_OK);
			if (CPythonPlayer::METIN_SOCKET_TYPE_SILVER == dwSocketType)
				bNotExistGoldSocket = TRUE;
		}
#ifdef ENABLE_YOHARA_SYSTEM
		else if (CItemData::METIN_SUNGMA == iSubType)
		{
			if (CPythonPlayer::METIN_SOCKET_TYPE_SILVER == dwSocketType || CPythonPlayer::METIN_SOCKET_TYPE_GOLD == dwSocketType)
				return Py_BuildValue("i", ATTACH_METIN_OK);
		}
#endif
	}

	if (bNotExistGoldSocket)
		return Py_BuildValue("i", ATTACH_METIN_NOT_EXIST_GOLD_SOCKET);

	return Py_BuildValue("i", ATTACH_METIN_NO_MATCHABLE_SOCKET);
}

enum
{
	DETACH_METIN_CANT,
	DETACH_METIN_OK
};

PyObject * playerCanDetach(PyObject * poSelf, PyObject * poArgs)
{
	int iScrollItemIndex;
	TItemPos TargetSlotIndex;
	switch (PyTuple_Size(poArgs))
	{
	case 2:
		if (!PyTuple_GetInteger(poArgs, 0, &iScrollItemIndex))
			return Py_BadArgument();
		if (!PyTuple_GetInteger(poArgs, 1, &TargetSlotIndex.cell))
			return Py_BadArgument();
		break;
	case 3:
		if (!PyTuple_GetInteger(poArgs, 0, &iScrollItemIndex))
			return Py_BadArgument();
		if (!PyTuple_GetInteger(poArgs, 1, &TargetSlotIndex.window_type))
			return Py_BadArgument();
		if (!PyTuple_GetInteger(poArgs, 2, &TargetSlotIndex.cell))
			return Py_BadArgument();
		break;
	default:
		return Py_BadArgument();
	}

	// Scroll
	CItemManager::Instance().SelectItemData(iScrollItemIndex);
	CItemData * pScrollItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pScrollItemData)
		return Py_BuildException("Can't find item data");
	int iScrollType = pScrollItemData->GetType();
	int iScrollSubType = pScrollItemData->GetSubType();
	if (iScrollType != CItemData::ITEM_TYPE_USE)
		return Py_BuildValue("i", DETACH_METIN_CANT);
	if (iScrollSubType != CItemData::USE_DETACHMENT)
		return Py_BuildValue("i", DETACH_METIN_CANT);

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	if (pScrollItemData->GetIndex() == (uint32_t)ETRANSMUTATIONSETTINGS::TRANSMUTATION_CLEAR_SCROLL) {
		const TItemData* pPlayerItem = CPythonPlayer::Instance().GetItemData(TargetSlotIndex);
		if (pPlayerItem) {
			if (pPlayerItem->dwTransmutationVnum == 0)
				return Py_BuildValue("i", DETACH_METIN_CANT);
			else
				return Py_BuildValue("i", DETACH_METIN_OK);
		}
		else
			return Py_BuildValue("i", DETACH_METIN_OK);
	}
#endif

#if defined(ENABLE_ACCE_COSTUME_SYSTEM) || defined(ENABLE_AURA_SYSTEM)
	if (pScrollItemData->GetIndex() == 39046 || pScrollItemData->GetIndex() == 90000) {
		const TItemData* pPlayerItem = CPythonPlayer::Instance().GetItemData(TargetSlotIndex);
		if (pPlayerItem) {
			if (pPlayerItem->alSockets[0] == 0)
				return Py_BuildValue("i", DETACH_METIN_CANT);
			else
				return Py_BuildValue("i", DETACH_METIN_OK);
		}
		else
			return Py_BuildValue("i", DETACH_METIN_OK);
	}
#endif

#ifdef ENABLE_SET_ITEM
	if (pScrollItemData->GetIndex() == 39115) {
		const TItemData* pPlayerItem = CPythonPlayer::Instance().GetItemData(TargetSlotIndex);
		if (pPlayerItem) {
			if (pPlayerItem->set_value == 0)
				return Py_BuildValue("i", DETACH_METIN_CANT);
			else
				return Py_BuildValue("i", DETACH_METIN_OK);
		}
		else
			return Py_BuildValue("i", DETACH_METIN_OK);
	}
#endif

	// Target Item
	int iTargetItemIndex = CPythonPlayer::Instance().GetItemIndex(TargetSlotIndex);
	CItemManager::Instance().SelectItemData(iTargetItemIndex);
	CItemData * pTargetItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pTargetItemData)
		return Py_BuildException("Can't find item data");
	//int iTargetType = pTargetItemData->GetType();
	//int iTargetSubType = pTargetItemData->GetSubType();

	if (pTargetItemData->IsFlag(CItemData::ITEM_FLAG_REFINEABLE))
	{
		for (int iSlotCount = 0; iSlotCount < METIN_SOCKET_COUNT; ++iSlotCount)
			if (CPythonPlayer::Instance().GetItemMetinSocket(TargetSlotIndex, iSlotCount) > 2
#ifdef ENABLE_GLOVE_SYSTEM
				&& CPythonPlayer::Instance().GetItemMetinSocket(TargetSlotIndex, iSlotCount) != 28960
#endif
				)
				return Py_BuildValue("i", DETACH_METIN_OK);
	}

	return Py_BuildValue("i", DETACH_METIN_CANT);
}

PyObject * playerCanUnlock(PyObject * poSelf, PyObject * poArgs)
{
	int iKeyItemIndex;
	TItemPos TargetSlotIndex;
	switch (PyTuple_Size(poArgs))
	{
	case 2:
		if (!PyTuple_GetInteger(poArgs, 0, &iKeyItemIndex))
			return Py_BadArgument();
		if (!PyTuple_GetInteger(poArgs, 1, &TargetSlotIndex.cell))
			return Py_BadArgument();
		break;
	case 3:
		if (!PyTuple_GetInteger(poArgs, 0, &iKeyItemIndex))
			return Py_BadArgument();
		if (!PyTuple_GetInteger(poArgs, 1, &TargetSlotIndex.window_type))
			return Py_BadArgument();
		if (!PyTuple_GetInteger(poArgs, 2, &TargetSlotIndex.cell))
			return Py_BadArgument();
		break;
	default:
		return Py_BadArgument();
	}

	// Key
	CItemManager::Instance().SelectItemData(iKeyItemIndex);
	CItemData * pKeyItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pKeyItemData)
		return Py_BuildException("Can't find item data");
	int iKeyType = pKeyItemData->GetType();
	if (iKeyType != CItemData::ITEM_TYPE_TREASURE_KEY)
		return Py_BuildValue("i", FALSE);

	// Target Item
	int iTargetItemIndex = CPythonPlayer::Instance().GetItemIndex(TargetSlotIndex);
	CItemManager::Instance().SelectItemData(iTargetItemIndex);
	CItemData * pTargetItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pTargetItemData)
		return Py_BuildException("Can't find item data");
	int iTargetType = pTargetItemData->GetType();
	if (iTargetType != CItemData::ITEM_TYPE_TREASURE_BOX)
		return Py_BuildValue("i", FALSE);

	return Py_BuildValue("i", TRUE);
}

PyObject * playerIsRefineGradeScroll(PyObject * poSelf, PyObject * poArgs)
{
	TItemPos ScrollSlotIndex;
	switch (PyTuple_Size(poArgs))
	{
	case 1:
		if (!PyTuple_GetInteger(poArgs, 0, &ScrollSlotIndex.cell))
			return Py_BuildException();
	case 2:
		if (!PyTuple_GetInteger(poArgs, 0, &ScrollSlotIndex.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &ScrollSlotIndex.cell))
			return Py_BuildException();
	default:
		return Py_BuildException();
	}
}

PyObject * playerUpdate(PyObject * poSelf, PyObject * poArgs)
{
	CPythonPlayer::Instance().Update();
	return Py_BuildNone();
}

PyObject * playerRender(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildNone();
}

PyObject * playerClear(PyObject * poSelf, PyObject * poArgs)
{
	CPythonPlayer::Instance().Clear();
	return Py_BuildNone();
}

PyObject * playerClearTarget(PyObject * poSelf, PyObject * poArgs)
{
	CPythonPlayer::Instance().SetTarget(0);
	return Py_BuildNone();
}

PyObject * playerSetTarget(PyObject * poSelf, PyObject * poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	CPythonPlayer::Instance().SetTarget(iVID);
	return Py_BuildNone();
}

PyObject * playerOpenCharacterMenu(PyObject * poSelf, PyObject * poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	CPythonPlayer::Instance().OpenCharacterMenu(iVID);
	return Py_BuildNone();
}

PyObject * playerIsPartyMember(PyObject * poSelf, PyObject * poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonPlayer::Instance().IsPartyMemberByVID(iVID));
}

PyObject * playerIsPartyLeader(PyObject * poSelf, PyObject * poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	uint32_t dwPID;
	if (!CPythonPlayer::Instance().PartyMemberVIDToPID(iVID, &dwPID))
		return Py_BuildValue("i", FALSE);

	CPythonPlayer::TPartyMemberInfo * pPartyMemberInfo;
	if (!CPythonPlayer::Instance().GetPartyMemberPtr(dwPID, &pPartyMemberInfo))
		return Py_BuildValue("i", FALSE);

	return Py_BuildValue("i", CPythonPlayer::PARTY_ROLE_LEADER == pPartyMemberInfo->byState);
}

PyObject * playerIsPartyLeaderByPID(PyObject * poSelf, PyObject * poArgs)
{
	int iPID;
	if (!PyTuple_GetInteger(poArgs, 0, &iPID))
		return Py_BuildException();

	CPythonPlayer::TPartyMemberInfo * pPartyMemberInfo;
	if (!CPythonPlayer::Instance().GetPartyMemberPtr(iPID, &pPartyMemberInfo))
		return Py_BuildValue("i", FALSE);

	return Py_BuildValue("i", CPythonPlayer::PARTY_ROLE_LEADER == pPartyMemberInfo->byState);
}

PyObject * playerGetPartyMemberHPPercentage(PyObject * poSelf, PyObject * poArgs)
{
	int iPID;
	if (!PyTuple_GetInteger(poArgs, 0, &iPID))
		return Py_BuildException();

	CPythonPlayer::TPartyMemberInfo * pPartyMemberInfo;
	if (!CPythonPlayer::Instance().GetPartyMemberPtr(iPID, &pPartyMemberInfo))
		return Py_BuildValue("i", FALSE);

	return Py_BuildValue("i", pPartyMemberInfo->byHPPercentage);
}

PyObject * playerGetPartyMemberState(PyObject * poSelf, PyObject * poArgs)
{
	int iPID;
	if (!PyTuple_GetInteger(poArgs, 0, &iPID))
		return Py_BuildException();

	CPythonPlayer::TPartyMemberInfo * pPartyMemberInfo;
	if (!CPythonPlayer::Instance().GetPartyMemberPtr(iPID, &pPartyMemberInfo))
		return Py_BuildValue("i", FALSE);

	return Py_BuildValue("i", pPartyMemberInfo->byState);
}

PyObject * playerGetPartyMemberAffects(PyObject * poSelf, PyObject * poArgs)
{
	int iPID;
	if (!PyTuple_GetInteger(poArgs, 0, &iPID))
		return Py_BuildException();

	CPythonPlayer::TPartyMemberInfo * pPartyMemberInfo;
	if (!CPythonPlayer::Instance().GetPartyMemberPtr(iPID, &pPartyMemberInfo))
		return Py_BuildValue("i", FALSE);

	return Py_BuildValue("iiiiiii", pPartyMemberInfo->sAffects[0], pPartyMemberInfo->sAffects[1], pPartyMemberInfo->sAffects[2],
						 pPartyMemberInfo->sAffects[3], pPartyMemberInfo->sAffects[4], pPartyMemberInfo->sAffects[5],
						 pPartyMemberInfo->sAffects[6]);
}

PyObject * playerRemovePartyMember(PyObject * poSelf, PyObject * poArgs)
{
	int iPID;
	if (!PyTuple_GetInteger(poArgs, 0, &iPID))
		return Py_BuildException();

	CPythonPlayer::Instance().RemovePartyMember(iPID);
	return Py_BuildNone();
}

PyObject * playerExitParty(PyObject * poSelf, PyObject * poArgs)
{
	CPythonPlayer::Instance().ExitParty();
	return Py_BuildNone();
}

PyObject * playerGetPKMode(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetPKMode());
}

PyObject * playerHasMobilePhoneNumber(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().HasMobilePhoneNumber());
}

PyObject * playerSetWeaponAttackBonusFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	return Py_BuildNone();
}

PyObject * playerToggleCoolTime(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().__ToggleCoolTime());
}

PyObject * playerToggleLevelLimit(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().__ToggleLevelLimit());
}

PyObject * playerGetTargetVID(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetTargetVID());
}

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
PyObject* playerGetRouletteData(PyObject* poSelf, PyObject* poArgs)
{
	int iOption;
	if (!PyTuple_GetInteger(poArgs, 0, &iOption))
		return Py_BuildException();

	enum { Speed, Position };
	PyObject* SList = PyList_New(0);

	if (iOption == Speed) {
		const std::vector<int> lSpeedList = { 325, 105, 75, 40, 25, 10 };
		for (auto it = lSpeedList.begin(); it != lSpeedList.end(); ++it)
			PyList_Append(SList, Py_BuildValue("i", (*it)));
	}
	else if (iOption == Position) {
		const std::vector<std::pair<int, int>> lPositionList = { {25, 46}, {69, 46}, {113, 46}, {157, 46}, {201, 46}, {245, 46}, {245, 90}, {245, 134}, {245, 178}, {245, 222}, {245, 266}, {201, 266}, {157, 266}, {113, 266}, {69, 266}, {25, 266}, {25, 222}, {25, 178}, {25, 134}, {25, 90} };
		for (auto it = lPositionList.begin(); it != lPositionList.end(); ++it)
			PyList_Append(SList, Py_BuildValue("[ii]", it->first, it->second));
	}

	return SList;
}

PyObject* playerGetSoul(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetStatus(POINT_SOUL));
}
#endif

PyObject * playerRegisterEmotionIcon(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();
	char * szFileName;
	if (!PyTuple_GetString(poArgs, 1, &szFileName))
		return Py_BuildException();

	auto * pImage = msl::inherit_cast<CGraphicImage *>(CResourceManager::Instance().GetResourcePointer(szFileName));
	m_kMap_iEmotionIndex_pkIconImage.emplace(iIndex, pImage);

	return Py_BuildNone();
}

PyObject * playerGetEmotionIconImage(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	if (m_kMap_iEmotionIndex_pkIconImage.end() == m_kMap_iEmotionIndex_pkIconImage.find(iIndex))
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", m_kMap_iEmotionIndex_pkIconImage[iIndex]);
}

PyObject * playerSetItemData(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 1, &iVirtualID))
		return Py_BuildException();

	int iNum;
	if (!PyTuple_GetInteger(poArgs, 2, &iNum))
		return Py_BuildException();

	TItemData kItemInst;
	ZeroMemory(&kItemInst, sizeof(kItemInst));
	kItemInst.vnum = iVirtualID;
	kItemInst.count = iNum;
	CPythonPlayer::Instance().SetItemData(TItemPos(INVENTORY, iSlotIndex), kItemInst);
	return Py_BuildNone();
}

PyObject * playerSetItemMetinSocket(PyObject * poSelf, PyObject * poArgs)
{
	TItemPos ItemPos;
	int iMetinSocketNumber;
	int iNum;

	switch (PyTuple_Size(poArgs))
	{
	case 3:
		if (!PyTuple_GetInteger(poArgs, 0, &ItemPos.cell))
			return Py_BuildException();

		if (!PyTuple_GetInteger(poArgs, 1, &iMetinSocketNumber))
			return Py_BuildException();

		if (!PyTuple_GetInteger(poArgs, 2, &iNum))
			return Py_BuildException();

		break;
	case 4:
		if (!PyTuple_GetInteger(poArgs, 0, &ItemPos.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &ItemPos.cell))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 2, &iMetinSocketNumber))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 3, &iNum))
			return Py_BuildException();

		break;
	default:
		return Py_BuildException();
	}

	CPythonPlayer::Instance().SetItemMetinSocket(ItemPos, iMetinSocketNumber, iNum);
	return Py_BuildNone();
}

PyObject * playerSetItemAttribute(PyObject * poSelf, PyObject * poArgs)
{
	TItemPos ItemPos;
	int iAttributeSlotIndex;
	int iAttributeType;
	int iAttributeValue;

	switch (PyTuple_Size(poArgs))
	{
	case 4:
		if (!PyTuple_GetInteger(poArgs, 0, &ItemPos.cell))
			return Py_BuildException();

		if (!PyTuple_GetInteger(poArgs, 1, &iAttributeSlotIndex))
			return Py_BuildException();

		if (!PyTuple_GetInteger(poArgs, 2, &iAttributeType))
			return Py_BuildException();

		if (!PyTuple_GetInteger(poArgs, 3, &iAttributeValue))
			return Py_BuildException();
		break;
	case 5:
		if (!PyTuple_GetInteger(poArgs, 0, &ItemPos.window_type))
			return Py_BuildException();

		if (!PyTuple_GetInteger(poArgs, 1, &ItemPos.cell))
			return Py_BuildException();

		if (!PyTuple_GetInteger(poArgs, 2, &iAttributeSlotIndex))
			return Py_BuildException();

		if (!PyTuple_GetInteger(poArgs, 3, &iAttributeType))
			return Py_BuildException();

		if (!PyTuple_GetInteger(poArgs, 4, &iAttributeValue))
			return Py_BuildException();
		break;
	}
	CPythonPlayer::Instance().SetItemAttribute(ItemPos, iAttributeSlotIndex, iAttributeType, iAttributeValue);
	return Py_BuildNone();
}

PyObject * playerSetAutoPotionInfo(PyObject * poSelf, PyObject * poArgs)
{
	int potionType = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &potionType))
		return Py_BadArgument();

	CPythonPlayer * player = CPythonPlayer::InstancePtr();

	CPythonPlayer::SAutoPotionInfo & potionInfo = player->GetAutoPotionInfo(potionType);

	if (!PyTuple_GetBoolean(poArgs, 1, &potionInfo.bActivated))
		return Py_BadArgument();

	if (!PyTuple_GetLong(poArgs, 2, &potionInfo.currentAmount))
		return Py_BadArgument();

	if (!PyTuple_GetLong(poArgs, 3, &potionInfo.totalAmount))
		return Py_BadArgument();

	if (!PyTuple_GetLong(poArgs, 4, &potionInfo.inventorySlotIndex))
		return Py_BadArgument();

	return Py_BuildNone();
}

PyObject * playerGetAutoPotionInfo(PyObject * poSelf, PyObject * poArgs)
{
	CPythonPlayer * player = CPythonPlayer::InstancePtr();

	int potionType = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &potionType))
		return Py_BadArgument();

	CPythonPlayer::SAutoPotionInfo & potionInfo = player->GetAutoPotionInfo(potionType);

	return Py_BuildValue("biii", potionInfo.bActivated, int(potionInfo.currentAmount), int(potionInfo.totalAmount),
						 int(potionInfo.inventorySlotIndex));
}

PyObject * playerSlotTypeToInvenType(PyObject * poSelf, PyObject * poArgs)
{
	int slotType = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &slotType))
		return Py_BadArgument();

	return Py_BuildValue("i", SlotTypeToInvenType(static_cast<uint8_t>(slotType)));
}

//ENABLE_NEW_EQUIPMENT_SYSTEM
PyObject * playerIsEquippingBelt(PyObject * poSelf, PyObject * poArgs)
{
	const CPythonPlayer * player = CPythonPlayer::InstancePtr();
	if (!player)
		return Py_BadArgument();

	bool bEquipping = false;

	const TItemData * data = player->GetItemData(TItemPos(EQUIPMENT, c_Equipment_Belt));

	if (nullptr != data)
		bEquipping = 0 < data->count;

	return Py_BuildValue("b", bEquipping);
}

PyObject* playerIsAvailableBeltInventoryCell(PyObject* poSelf, PyObject* poArgs)
{
	const CPythonPlayer* player = CPythonPlayer::InstancePtr();
	if (!player)
		return Py_BadArgument();

	const TItemData* pData = player->GetItemData(TItemPos(EQUIPMENT, c_Equipment_Belt));

	if (nullptr == pData || 0 == pData->count)
		return Py_BuildValue("b", false);

	CItemManager::Instance().SelectItemData(pData->vnum);
	const CItemData* pItem = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItem)
		return Py_BadArgument();

	const long beltGrade = pItem->GetValue(0);

	int pos = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &pos))
		return Py_BadArgument();

	//return Py_BuildValue("b", CBeltInventoryHelper::IsAvailableCell(pos - c_Belt_Inventory_Slot_Start, GetItemGrade(pItem->GetName())));
	return Py_BuildValue("b", CBeltInventoryHelper::IsAvailableCell(pos - c_Belt_Inventory_Slot_Start, beltGrade));
}
//END_ENABLE_NEW_EQUIPMENT_SYSTEM


// øÎ»•ºÆ ∞≠»≠
PyObject * playerSendDragonSoulRefine(PyObject * poSelf, PyObject * poArgs)
{
	uint8_t bSubHeader;
	PyObject * pDic;
	TItemPos RefineItemPoses[DS_REFINE_WINDOW_MAX_NUM];
	if (!PyTuple_GetByte(poArgs, 0, &bSubHeader))
		return Py_BuildException();
	switch (bSubHeader)
	{
	case DS_SUB_HEADER_CLOSE:
		break;
	case DS_SUB_HEADER_DO_UPGRADE:
	case DS_SUB_HEADER_DO_IMPROVEMENT:
	case DS_SUB_HEADER_DO_REFINE:
#ifdef ENABLE_DS_CHANGE_ATTR
	case DS_SUB_HEADER_DO_CHANGE_ATTR:
#endif
	{
		if (!PyTuple_GetObject(poArgs, 1, &pDic))
			return Py_BuildException();
		int pos = 0;
		PyObject *key, *value;
		int size = PyDict_Size(pDic);

		while (PyDict_Next(pDic, &pos, &key, &value))
		{
			int i = PyInt_AsLong(key);
			if (i > DS_REFINE_WINDOW_MAX_NUM)
				return Py_BuildException();

			if (!PyTuple_GetByte(value, 0, &RefineItemPoses[i].window_type) || !PyTuple_GetInteger(value, 1, &RefineItemPoses[i].cell))
				return Py_BuildException();
		}
	}
	break;
	}

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendDragonSoulRefinePacket(bSubHeader, RefineItemPoses);

	return Py_BuildNone();
}

#ifdef ENABLE_SOULBIND_SYSTEM
PyObject* playerCanSealItem(PyObject* poSelf, PyObject* poArgs)	//50263
{
	int iSealScrollVNum;
	TItemPos Cell;
	switch (PyTuple_Size(poArgs))
	{
		case 2:
			if (!PyTuple_GetInteger(poArgs, 0, &iSealScrollVNum))
				return Py_BadArgument();
			if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
				return Py_BadArgument();
			break;

		case 3:
			if (!PyTuple_GetInteger(poArgs, 0, &iSealScrollVNum))
				return Py_BadArgument();
			if (!PyTuple_GetInteger(poArgs, 1, &Cell.window_type))
				return Py_BadArgument();
			if (!PyTuple_GetInteger(poArgs, 2, &Cell.cell))
				return Py_BadArgument();
			break;

		default:
			return Py_BuildException();
	}

	//SealScroll
	CItemManager::Instance().SelectItemData(iSealScrollVNum);
	CItemData* pSealScrollItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pSealScrollItemData)
		return Py_BuildException("Can't find item data");

	//Destination ItemData
	CItemManager::Instance().SelectItemData(CPythonPlayer::Instance().GetItemIndex(Cell));
	CItemData* pDestItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pDestItemData)
		return Py_BuildException("Can't find item data");

	//Destination Item
	const TItemData* pDestItem = CPythonPlayer::Instance().GetItemData(Cell);
	if (!pDestItem)
		return Py_BuildException("CanSealItem Cell(%d, %d) item is null", Cell.window_type, Cell.cell);

	//Bind-Item
	if (pSealScrollItemData->GetType() == CItemData::ITEM_TYPE_USE && pSealScrollItemData->GetSubType() == CItemData::USE_BIND)
	{
		if (pDestItemData->CanSealItem() && !pDestItem->nSealDate)
			return Py_BuildValue("i", TRUE);
	}

	//Unbind-Item
	else if (pSealScrollItemData->GetType() == CItemData::ITEM_TYPE_USE && pSealScrollItemData->GetSubType() == CItemData::USE_UNBIND)
	{
		if (pDestItem->nSealDate)
			return Py_BuildValue("i", TRUE);
	}

	return Py_BuildValue("i", FALSE);
}

PyObject* playerGetItemSealDate(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos Cell;
	if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
		return Py_BuildException();

	TItemData* pPlayerItem = nullptr;
	if (Cell.window_type == SAFEBOX)
		CPythonSafeBox::Instance().GetItemDataPtr(Cell.cell, &pPlayerItem);	//safebox???
	else
		pPlayerItem = const_cast<TItemData*>(CPythonPlayer::Instance().GetItemData(Cell));

	if (!pPlayerItem)
		return Py_BuildException("GetItemSealDate Cell(%d, %d) item is null", Cell.window_type, Cell.cell);

	long sealTime = pPlayerItem->nSealDate;

	if (-1 != sealTime && sealTime < CPythonApplication::Instance().GetServerTimeStamp())
		sealTime = CItemData::SEAL_DATE_DEFAULT_TIMESTAMP;

	return Py_BuildValue("i", sealTime);
}

PyObject* playerGetItemUnSealLeftTime(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos Cell;
	if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
		return Py_BuildException();

	TItemData* pPlayerItem = nullptr;
	if (Cell.window_type == SAFEBOX)
		CPythonSafeBox::Instance().GetItemDataPtr(Cell.cell, &pPlayerItem);	//safebox???
	else
		pPlayerItem = const_cast<TItemData*>(CPythonPlayer::Instance().GetItemData(Cell));

	if (!pPlayerItem)
		return Py_BuildException("GetItemUnSealLeftTime Cell(%d, %d) item is null", Cell.window_type, Cell.cell);

	long sealTime = pPlayerItem->nSealDate;
	if (sealTime != -1 && sealTime != 0 && sealTime > CPythonApplication::Instance().GetServerTimeStamp())
		return Py_BuildValue("ii", ((sealTime - CPythonApplication::Instance().GetServerTimeStamp()) / 3600), (sealTime - CPythonApplication::Instance().GetServerTimeStamp()) % 3600 / 60);
	else
		return Py_BuildValue("ii", 0, 0);
}
#endif

PyObject * playerIsAntiFlagBySlot(PyObject * poSelf, PyObject * poArgs)
{
	TItemPos Cell;
	int antiflag;
	switch (PyTuple_Size(poArgs))
	{
	case 2:
		if (!PyTuple_GetInteger(poArgs, 0, &Cell.cell))
			return Py_BadArgument();
		if (!PyTuple_GetInteger(poArgs, 1, &antiflag))
			return Py_BadArgument();
		break;

	case 3:
		if (!PyTuple_GetInteger(poArgs, 0, &Cell.window_type))
			return Py_BadArgument();
		if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
			return Py_BadArgument();
		if (!PyTuple_GetInteger(poArgs, 2, &antiflag))
			return Py_BadArgument();
		break;
	default:
		return Py_BuildException();
	}

	uint32_t itemAntiFlags = CPythonPlayer::Instance().GetItemAntiFlags(Cell);
	return Py_BuildValue("i", IS_SET(itemAntiFlags, antiflag) != 0 ? TRUE : FALSE);
}

PyObject * playerGetItemTypeBySlot(PyObject * poSelf, PyObject * poArgs)
{
	TItemPos Cell;
	switch (PyTuple_Size(poArgs))
	{
	case 1:
		if (!PyTuple_GetInteger(poArgs, 0, &Cell.cell))
			return Py_BadArgument();
		break;
	case 2:
		if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
			return Py_BadArgument();
		if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
			return Py_BadArgument();
		break;
	default:
		return Py_BuildException();
	}

	return Py_BuildValue("i", CPythonPlayer::Instance().GetItemTypeBySlot(Cell));
}

PyObject * playerGetItemSubTypeBySlot(PyObject * poSelf, PyObject * poArgs)
{
	TItemPos Cell;
	switch (PyTuple_Size(poArgs))
	{
	case 1:
		if (!PyTuple_GetInteger(poArgs, 0, &Cell.cell))
			return Py_BadArgument();
		break;
	case 2:
		if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
			return Py_BadArgument();
		if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
			return Py_BadArgument();
		break;
	default:
		return Py_BuildException();
	}

	return Py_BuildValue("i", CPythonPlayer::Instance().GetItemSubTypeBySlot(Cell));
}

PyObject * playerIsSameItemVnum(PyObject * poSelf, PyObject * poArgs)
{
	TItemPos Cell;
	int iItemVNum;
	switch (PyTuple_Size(poArgs))
	{
	case 2:
		if (!PyTuple_GetInteger(poArgs, 0, &iItemVNum))
			return Py_BadArgument();
		if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
			return Py_BadArgument();
		break;
	case 3:
		if (!PyTuple_GetInteger(poArgs, 0, &iItemVNum))
			return Py_BadArgument();
		if (!PyTuple_GetInteger(poArgs, 1, &Cell.window_type))
			return Py_BadArgument();
		if (!PyTuple_GetInteger(poArgs, 2, &Cell.cell))
			return Py_BadArgument();
		break;
	default:
		return Py_BuildException();
	}

	return Py_BuildValue("i", (iItemVNum == CPythonPlayer::Instance().GetItemIndex(Cell)) ? TRUE : FALSE);
}

PyObject* playerPartyMemberVIDToPID(PyObject* poSelf, PyObject* poArgs)	//@fixme402
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	uint32_t dwPID;
	if (!CPythonPlayer::Instance().PartyMemberVIDToPID(iVID, &dwPID))
		return Py_BuildValue("i", FALSE);

	return Py_BuildValue("i", dwPID);
}

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
PyObject* playerGetChangeLookVnum(PyObject* poSelf, PyObject* poArgs)
{
	switch (PyTuple_Size(poArgs))
	{
		case 1:
		{
			int iSlotIndex;
			if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
				return Py_BuildException();

			uint32_t dwTransmutation = CPythonPlayer::Instance().GetItemTransmutationVnum(TItemPos(INVENTORY, iSlotIndex));
			return Py_BuildValue("i", dwTransmutation);
		}

		case 2:
		{
			TItemPos Cell;
			if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
				return Py_BuildException();

			if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
				return Py_BuildException();

			uint32_t dwTransmutation = CPythonPlayer::Instance().GetItemTransmutationVnum(Cell);
			return Py_BuildValue("i", dwTransmutation);
		}

		default:
			return Py_BuildException();
	}
}

#	ifdef ENABLE_ACCE_COSTUME_SYSTEM
PyObject* playerGetAcceWindowChangeLookVnum(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();

	int pAcceItem = CPythonPlayer::Instance().GetItemTransmutationVnum(TItemPos(INVENTORY, CPythonPlayer::Instance().GetAcceActivedItemSlot(iSlotIndex)));
	if (pAcceItem)
		return Py_BuildValue("i", pAcceItem);

	return Py_BuildValue("i", 0);
}
#	endif

#	ifdef ENABLE_REFINE_ELEMENT
PyObject* playerGetChangeLookElement(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bSlotType;
	if (!PyTuple_GetByte(poArgs, 0, &bSlotType))
		return Py_BadArgument();

	if (bSlotType >= static_cast<uint8_t>(ETRANSMUTATIONSLOTTYPE::TRANSMUTATION_SLOT_MAX))
		return Py_BuildException();

	const TSubPacketGCChangeLookItemSet* item = CPythonPlayer::Instance().GetChangeLookItemData(bSlotType);
	if (!item)
		return Py_BuildValue("i", 0);

	const TItemData* pPlayerItem = CPythonPlayer::Instance().GetItemData(TItemPos(INVENTORY, item->wCell));
	if (!pPlayerItem)
		return Py_BuildValue("i", 0);

	if (pPlayerItem->grade_element > 0)
		return Py_BuildValue("iiii", pPlayerItem->grade_element, pPlayerItem->attack_element[pPlayerItem->grade_element - 1], pPlayerItem->element_type_bonus, pPlayerItem->elements_value_bonus[pPlayerItem->grade_element - 1]);

	return Py_BuildValue("iiii", 0, 0, 0, 0);
}
#	endif

PyObject* playerSetChangeLookWindow(PyObject* poSelf, PyObject* poArgs)
{
	bool blOpen;
	if (!PyTuple_GetBoolean(poArgs, 0, &blOpen))
		return Py_BadArgument();

	CPythonPlayer::Instance().SetChangeLookWindowOpen(blOpen);

	return Py_BuildNone();
}

PyObject* playerGetChangeLookWindowOpen(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("b", CPythonPlayer::Instance().GetChangeLookWindowOpen());
}

PyObject* playerSetChangeLookWindowType(PyObject* poSelf, PyObject* poArgs)
{
	bool blType;
	if (!PyTuple_GetBoolean(poArgs, 0, &blType))
		return Py_BadArgument();

	CPythonPlayer::Instance().SetChangeLookWindowType(blType);

	return Py_BuildNone();
}

PyObject* playerGetChangeLookItemID(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bSlotType;
	if (!PyTuple_GetByte(poArgs, 0, &bSlotType))
		return Py_BadArgument();

	if (bSlotType >= static_cast<uint8_t>(ETRANSMUTATIONSLOTTYPE::TRANSMUTATION_SLOT_MAX))
		return Py_BuildException();

	const TSubPacketGCChangeLookItemSet* item = CPythonPlayer::Instance().GetChangeLookItemData(bSlotType);
	if (!item)
		return Py_BuildValue("i", 0);

	const TItemData* pPlayerItem = CPythonPlayer::Instance().GetItemData(TItemPos(INVENTORY, item->wCell));
	if (!pPlayerItem)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pPlayerItem->vnum);
}

PyObject* playerGetChangeLookItemInvenSlot(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bSlotType;
	if (!PyTuple_GetByte(poArgs, 0, &bSlotType))
		return Py_BadArgument();

	if (bSlotType >= static_cast<uint8_t>(ETRANSMUTATIONSLOTTYPE::TRANSMUTATION_SLOT_MAX))
		return Py_BuildException();

	const TSubPacketGCChangeLookItemSet* item = CPythonPlayer::Instance().GetChangeLookItemData(bSlotType);
	if (item)
		return Py_BuildValue("i", item->wCell);

	return Py_BuildValue("i", 0);
}

PyObject* playerGetChangeLookFreeYangItemID(PyObject* poSelf, PyObject* poArgs)
{
	const TSubPacketGCChangeLookItemSet* item = CPythonPlayer::Instance().GetChangeLookFreeItemData();
	if (!item)
		return Py_BuildValue("i", 0);

	const TItemData* pPlayerItem = CPythonPlayer::Instance().GetItemData(TItemPos(INVENTORY, item->wCell));
	if (!pPlayerItem)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pPlayerItem->vnum);
}

PyObject* playerGetChangeLookFreeYangInvenSlotPos(PyObject* poSelf, PyObject* poArgs)
{
	const TSubPacketGCChangeLookItemSet* item = CPythonPlayer::Instance().GetChangeLookFreeItemData();
	if (item)
		return Py_BuildValue("i", item->wCell);

	return Py_BuildValue("i", 0);
}

PyObject* playerCanChangeLookClearItem(PyObject* poSelf, PyObject* poArgs)
{
	int iItemVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemVID))
		return Py_BadArgument();

	TItemPos Cell;
	if (!PyTuple_GetByte(poArgs, 1, &Cell.window_type))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 2, &Cell.cell))
		return Py_BuildException();

	CItemManager::Instance().SelectItemData(iItemVID);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();

	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	const TItemData* pPlayerItem = CPythonPlayer::Instance().GetItemData(Cell);
	if (pPlayerItem)
		return Py_BuildValue("b", pPlayerItem->dwTransmutationVnum != 0);

	return Py_BuildValue("b", false);
}

PyObject* playerGetChangeWIndowChangeLookVnum(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bSlotType;
	if (!PyTuple_GetByte(poArgs, 0, &bSlotType))
		return Py_BadArgument();

	if (bSlotType >= static_cast<uint8_t>(ETRANSMUTATIONSLOTTYPE::TRANSMUTATION_SLOT_MAX))
		return Py_BuildException();

	const TSubPacketGCChangeLookItemSet* item = CPythonPlayer::Instance().GetChangeLookItemData(bSlotType);
	if (!item)
		return Py_BuildValue("i", 0);

	const TItemData* pPlayerItem = CPythonPlayer::Instance().GetItemData(TItemPos(INVENTORY, item->wCell));
	if (!pPlayerItem)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pPlayerItem->dwTransmutationVnum);
}

PyObject* playerGetChangeChangeLookPrice(PyObject* poSelf, PyObject* poArgs)
{
	const bool bType = CPythonPlayer::Instance().GetChangeLookWindowType();
	switch (static_cast<ETRANSMUTATIONTYPE>(bType))
	{
		case ETRANSMUTATIONTYPE::TRANSMUTATION_TYPE_MOUNT:
			return Py_BuildValue("i", static_cast<uint32_t>(ETRANSMUTATIONSETTINGS::TRANSMUTATION_MOUNT_PRICE));
		case ETRANSMUTATIONTYPE::TRANSMUTATION_TYPE_ITEM:
			return Py_BuildValue("i", static_cast<uint32_t>(ETRANSMUTATIONSETTINGS::TRANSMUTATION_ITEM_PRICE));
	}

	return Py_BuildValue("i", 0);
}
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
PyObject* playerGetPetItem(PyObject* poSelf, PyObject* poArgs)
{
	int pet_item_id;
	if (!PyTuple_GetInteger(poArgs, 0, &pet_item_id))
		return Py_BadArgument();

	const auto petInfo = CPythonPlayer::Instance().GetPetInfo(pet_item_id);
	if (!petInfo.pet_id)
		return Py_BadArgument();

	char szPetName[CItemData::PET_NAME_MAX_SIZE + 1];
	strncpy(szPetName, petInfo.pet_nick, sizeof(szPetName));

	char szPetHP[10];
	sprintf(szPetHP, "%.1f", petInfo.pet_hp);

	char szPetDEF[10];
	sprintf(szPetDEF, "%.1f", petInfo.pet_def);

	char szPetSP[10];
	sprintf(szPetSP, "%.1f", petInfo.pet_sp);

	return Py_BuildValue("iiisssss", petInfo.pet_level, petInfo.evol_level, petInfo.pet_birthday,
		szPetName, szPetHP, szPetDEF, szPetSP, "");
}

PyObject* playerGetActivePetItemId(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetActivePetItemID());
}

PyObject* playerGetActivePetItemVNum(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetActivePetItemVnum());
}

PyObject* playerGetPetExpPoints(PyObject* poSelf, PyObject* poArgs)
{
	int pet_item_id;
	if (!PyTuple_GetInteger(poArgs, 0, &pet_item_id))
		return Py_BadArgument();

	const auto petInfo = CPythonPlayer::Instance().GetPetInfo(pet_item_id);
	if (!petInfo.pet_id)
		return Py_BadArgument();

	const int curExp = petInfo.exp_monster;
	const int nextEXP = petInfo.next_exp_monster;

	const int itemEXP = petInfo.exp_item;
	const int itemMaxExp = petInfo.next_exp_item;

	return Py_BuildValue("iiii", curExp, nextEXP, itemEXP, itemMaxExp);
}

PyObject* playerGetPetItemVNumInBag(PyObject* poSelf, PyObject* poArgs)
{
	int pet_item_id;
	if (!PyTuple_GetInteger(poArgs, 0, &pet_item_id))
		return Py_BadArgument();

	const auto petInfo = CPythonPlayer::Instance().GetPetInfo(pet_item_id);
	if (!petInfo.pet_id)
		return Py_BadArgument();

	return Py_BuildValue("i", petInfo.pet_vnum);
}

PyObject* playerGetPetSkill(PyObject* poSelf, PyObject* poArgs)
{
	int pet_item_id;
	if (!PyTuple_GetInteger(poArgs, 0, &pet_item_id))
		return Py_BadArgument();

	const auto petInfo = CPythonPlayer::Instance().GetPetInfo(pet_item_id);
	if (!petInfo.pet_id)
		return Py_BadArgument();

	return Py_BuildValue("iiiiiiiiii", petInfo.skill_count,
		petInfo.skill_vnum[0], petInfo.skill_level[0], petInfo.skill_cool[0],
		petInfo.skill_vnum[1], petInfo.skill_level[1], petInfo.skill_cool[1],
		petInfo.skill_vnum[2], petInfo.skill_level[2], petInfo.skill_cool[2]);
}

PyObject* playerGetPetLifeTime(PyObject* poSelf, PyObject* poArgs)
{
	int pet_item_id;
	if (!PyTuple_GetInteger(poArgs, 0, &pet_item_id))
		return Py_BadArgument();

	const auto petInfo = CPythonPlayer::Instance().GetPetInfo(pet_item_id);
	if (!petInfo.pet_id)
		return Py_BadArgument();

	return Py_BuildValue("ii", petInfo.pet_end_time, petInfo.pet_max_time);
}

PyObject* playerGetPetSkillByIndex(PyObject* poSelf, PyObject* poArgs)
{
	int pet_item_id;
	if (!PyTuple_GetInteger(poArgs, 0, &pet_item_id))
		return Py_BadArgument();

	int pet_skill_slot;
	if (!PyTuple_GetInteger(poArgs, 1, &pet_skill_slot))
		return Py_BadArgument();

	if (pet_skill_slot < 0 || pet_skill_slot >= 3)
		return Py_BuildException();

	const auto petInfo = CPythonPlayer::Instance().GetPetInfo(pet_item_id);
	if (!petInfo.pet_id)
		return Py_BadArgument();

	const auto pet_skill_vnum = petInfo.skill_vnum[pet_skill_slot];
	const auto pet_skill_level = petInfo.skill_level[pet_skill_slot];

	CPythonSkillPet::SSkillDataPet* c_pSkillData;
	if (!CPythonSkillPet::Instance().GetSkillData(pet_skill_vnum, &c_pSkillData))
		return Py_BuildException();

	const int formula1 = petInfo.skill_formula1[pet_skill_slot];
	const int next_formula1 = petInfo.next_skill_formula1[pet_skill_slot];

	const float formula2 = petInfo.skill_formula2[pet_skill_slot];
	const float next_formula2 = petInfo.next_skill_formula2[pet_skill_slot];

	const bool bonus_value = (pet_skill_level == 20) ? true : false; // need rework

	return Py_BuildValue("iiififi", pet_skill_vnum, pet_skill_level, formula1, formula2, next_formula1, next_formula2, bonus_value);
}

PyObject* playerCanUsePetCoolTimeCheck(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", 1);
}

PyObject* playerSetOpenPetHatchingWindow(PyObject* poSelf, PyObject* poArgs)
{
	bool bisOpenPetHatchingWindow = false;
	if (!PyTuple_GetBoolean(poArgs, 0, &bisOpenPetHatchingWindow))
		return Py_BadArgument();

	CPythonPlayer::Instance().SetOpenPetHatchingWindow(bisOpenPetHatchingWindow);

	return Py_BuildNone();
}

PyObject* playerIsOpenPetHatchingWindow(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().IsOpenPetHatchingWindow());
}

PyObject* playerSetOpenPetFeedWindow(PyObject* poSelf, PyObject* poArgs)
{
	bool bisOpenPetFeedWindow = false;
	if (!PyTuple_GetBoolean(poArgs, 0, &bisOpenPetFeedWindow))
		return Py_BadArgument();

	CPythonPlayer::Instance().SetOpenPetFeedWindow(bisOpenPetFeedWindow);

	return Py_BuildNone();
}

PyObject* playerIsOpenPetFeedWindow(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().IsOpenPetFeedWindow());
}

PyObject* playerCanUseGrowthPetQuickSlot(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", 1);
}

PyObject* playerSetOpenPetNameChangeWindow(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildNone();
}

#ifdef ENABLE_PET_ATTR_DETERMINE
PyObject* playerCanAttachToPetAttrChangeSlot(PyObject* poSelf, PyObject* poArgs)
{
	if (PyTuple_Size(poArgs) != 3)
		return Py_BuildException();

	int selectedSlotPos;
	if (!PyTuple_GetInteger(poArgs, 0, &selectedSlotPos))
		return Py_BuildException();

	int attachedSlotType;
	if (!PyTuple_GetInteger(poArgs, 1, &attachedSlotType))
		return Py_BuildException();

	int attachedSlotPos;
	if (!PyTuple_GetInteger(poArgs, 2, &attachedSlotPos))
		return Py_BuildException();

	if (selectedSlotPos < 0 || selectedSlotPos > 1)
		return Py_BuildException("SlotIndex Error");

	TItemPos Cell;
	if (!PyTuple_GetByte(poArgs, 1, &Cell.window_type))
		return Py_BadArgument();

	if (!PyTuple_GetInteger(poArgs, 2, &Cell.cell))
		return Py_BadArgument();

	const int ItemType = CPythonPlayer::Instance().GetItemTypeBySlot(Cell);
	const int ItemSubType = CPythonPlayer::Instance().GetItemSubTypeBySlot(Cell);

	if (CItemData::ITEM_TYPE_PET != ItemType || (CItemData::PET_UPBRINGING != ItemSubType && CItemData::PET_ATTR_CHANGE != ItemSubType))
		return Py_BuildValue("i", FALSE);

	if (selectedSlotPos == 0)
	{
		if (CItemData::PET_UPBRINGING != ItemSubType)
			return Py_BuildValue("i", FALSE);
	}
	else if (selectedSlotPos == 1)
	{
		if (CPythonPlayer::Instance().GetAttrChangeWindowSlotByAttachedInvenSlot(0) < 0)
			return Py_BuildValue("i", FALSE);

		if (CItemData::PET_ATTR_CHANGE != ItemSubType)
			return Py_BuildValue("i", FALSE);
	}

	const int iValue = CPythonPlayer::Instance().GetAttrChangeWindowSlotByAttachedInvenSlot(selectedSlotPos);
	if (iValue < 0)
		return Py_BuildValue("i", TRUE);

	return Py_BuildValue("i", FALSE);
}

PyObject* playerGetInvenSlotAttachedToPetAttrChangeWindowSlot(PyObject* poSelf, PyObject* poArgs)
{
	int attachedSlotPos;
	if (!PyTuple_GetInteger(poArgs, 0, &attachedSlotPos))
		return Py_BuildException();

	const int iValue = CPythonPlayer::Instance().GetAttrChangeWindowSlotByAttachedInvenSlot(attachedSlotPos);
	if (iValue < 0)
		return Py_BuildValue("i", -1);

	return Py_BuildValue("i", iValue);
}

PyObject* playerGetPetAttrChangeWindowSlotByAttachedInvenSlot(PyObject* poSelf, PyObject* poArgs)
{
	int attachedSlotPos;
	if (!PyTuple_GetInteger(poArgs, 0, &attachedSlotPos))
		return Py_BuildException();

	for (int i = 0; i < PET_WND_SLOT_ATTR_CHANGE_MAX; ++i)
	{
		const int selectedSlotPos = CPythonPlayer::Instance().GetAttrChangeWindowSlotByAttachedInvenSlot(i);
		if (attachedSlotPos == selectedSlotPos)
			return Py_BuildValue("i", 0);
	}

	return Py_BuildValue("i", PET_WND_SLOT_ATTR_CHANGE_MAX);
}

PyObject* playerSetItemPetAttrChangeWindowActivedItemSlot(PyObject* poSelf, PyObject* poArgs)
{
	int iSlot;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlot))
		return Py_BuildException();

	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iIndex))
		return Py_BuildException();

	CPythonPlayer::Instance().SetItemAttrChangeWindowActivedItemSlot(iSlot, iIndex);
	return Py_BuildNone();
}
#endif
#endif

PyObject * playerWindowTypeToSlotType(PyObject * poSelf, PyObject * poArgs)
{
	int iWindowType = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &iWindowType))
		return Py_BuildException();

	return Py_BuildValue("i", WindowTypeToSlotType(iWindowType));
}

#ifdef ENABLE_GIVE_BASIC_ITEM
PyObject * playerIsBasicItem(PyObject * poSelf, PyObject* poArgs)
{
	TItemPos Cell;
	switch (PyTuple_Size(poArgs))
	{
		case 1:
			if (!PyTuple_GetInteger(poArgs, 0, &Cell.cell))
				return Py_BadArgument();
			break;
		case 2:
			if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
				return Py_BadArgument();
			if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
				return Py_BadArgument();
			break;
		default:
			return Py_BuildException();
	}

	//TraceError("i", CPythonPlayer::Instance().IsBasicItem(Cell));
	return Py_BuildValue("i", CPythonPlayer::Instance().IsBasicItem(Cell));	
}
#endif

#ifdef ENABLE_SET_ITEM
static int GetSetItemApplyType(uint8_t type)
{
	std::map<int, int> applyTypeMap
	{
		{ POINT_NONE, CItemData::APPLY_NONE },
		{ POINT_MAX_HP, CItemData::APPLY_MAX_HP },
		{ POINT_MALL_EXPBONUS, CItemData::APPLY_MALL_EXPBONUS },
		{ POINT_MOV_SPEED, CItemData::APPLY_MOV_SPEED },
		{ POINT_ATTBONUS_MONSTER, CItemData::APPLY_ATT_BONUS_TO_MONSTER },
		{ POINT_MELEE_MAGIC_ATT_BONUS_PER, CItemData::APPLY_MELEE_MAGIC_ATTBONUS_PER },
	};

	const auto it = applyTypeMap.find(type);
	return it->second;
}

PyObject* playerGetSetItemEffect(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* item = PyDict_New();
	for (int i = CInstanceBase::NEW_AFFECT_SET_ITEM_SET_VALUE_1; i <= CInstanceBase::NEW_AFFECT_SET_ITEM_SET_VALUE_3; ++i)
	{
		const TPacketAffectElement elem = CPythonPlayer::Instance().GetAffectData(i, 0);
		//TraceError("Type %d value %d", GetSetItemApplyType(elem.wPointIdxApplyOn), elem.lApplyValue);
		PyDict_SetItem(item, Py_BuildValue("i", GetSetItemApplyType(elem.wPointIdxApplyOn)), Py_BuildValue("i", elem.lApplyValue));
	}

	return item;
}

PyObject * playerGetItemSetValue(PyObject * poSelf, PyObject* poArgs)
{
	switch (PyTuple_Size(poArgs))
	{
		case 1:
		{
			int iSlotIndex;
			if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
				return Py_BuildException();

			uint8_t bSetItem = CPythonPlayer::Instance().GetItemSetValue(TItemPos(INVENTORY, iSlotIndex));
			return Py_BuildValue("i", bSetItem);
		}
		case 2:
		{
			TItemPos Cell;
			if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
				return Py_BuildException();

			if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
				return Py_BuildException();

			uint8_t bSetItem = CPythonPlayer::Instance().GetItemSetValue(Cell);
			return Py_BuildValue("i", bSetItem);
		}
		default:
			return Py_BuildException();
	}	
}

PyObject* playerCanSetItemClearItem(PyObject* poSelf, PyObject* poArgs)	//50263
{
	int iScrollVNum;
	TItemPos Cell;
	switch (PyTuple_Size(poArgs))
	{
	case 2:
		if (!PyTuple_GetInteger(poArgs, 0, &iScrollVNum))
			return Py_BadArgument();
		if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
			return Py_BadArgument();
		break;

	case 3:
		if (!PyTuple_GetInteger(poArgs, 0, &iScrollVNum))
			return Py_BadArgument();
		if (!PyTuple_GetInteger(poArgs, 1, &Cell.window_type))
			return Py_BadArgument();
		if (!PyTuple_GetInteger(poArgs, 2, &Cell.cell))
			return Py_BadArgument();
		break;

	default:
		return Py_BuildException();
	}

	//Scroll
	CItemManager::Instance().SelectItemData(iScrollVNum);
	CItemData* pSealScrollItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pSealScrollItemData)
		return Py_BuildException("Can't find item data");

	//Destination ItemData
	CItemManager::Instance().SelectItemData(CPythonPlayer::Instance().GetItemIndex(Cell));
	CItemData* pDestItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pDestItemData)
		return Py_BuildException("Can't find item data");

	//Destination Item
	const TItemData* pDestItem = CPythonPlayer::Instance().GetItemData(Cell);
	if (!pDestItem)
		return Py_BuildException("Cell(%d, %d) item is null", Cell.window_type, Cell.cell);

	//Bind-Item
	if ((pSealScrollItemData->GetType() == CItemData::ITEM_TYPE_USE) || (pSealScrollItemData->GetSubType() == CItemData::USE_SPECIAL) || (pSealScrollItemData->GetIndex() == 39115))
	{
		if (pDestItem->set_value)
			return Py_BuildValue("i", TRUE);
	}

	return Py_BuildValue("i", FALSE);
}
#endif

#ifdef ENABLE_MYSHOP_DECO
PyObject* playerSelectShopModel(PyObject* poSelf, PyObject* poArgs)
{
	int iPolyVnum = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &iPolyVnum))
		return Py_BuildException();

	CRenderTargetManager::Instance().GetRenderTarget(CRenderTargetManager::MODEL_PRIVATE_SHOP)->CreateBackground("d:/ymir work/ui/game/myshop_deco/model_view_bg.sub", CPythonApplication::Instance().GetWidth(), CPythonApplication::Instance().GetHeight());
	CRenderTargetManager::Instance().GetRenderTarget(CRenderTargetManager::MODEL_PRIVATE_SHOP)->SetVisibility(true);
	CRenderTargetManager::Instance().GetRenderTarget(CRenderTargetManager::MODEL_PRIVATE_SHOP)->SelectModel(iPolyVnum);
	return Py_BuildNone();
}

PyObject* playerMyShopDecoShow(PyObject* poSelf, PyObject* poArgs)
{
	bool bIsMyShopDecoShow = false;
	if (!PyTuple_GetBoolean(poArgs, 0, &bIsMyShopDecoShow))
		return Py_BadArgument();

	CPythonPlayer::Instance().SetMyShopDecoShow(bIsMyShopDecoShow);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_BATTLE_FIELD
PyObject* playerGetBattlePoint(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("b", CPythonPlayer::Instance().GetStatus(POINT_BATTLE_FIELD));
}

PyObject* playerIsBattleButtonFlush(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("b", CPythonPlayer::Instance().IsBattleButtonFlush());
}

PyObject* playerSetBattleButtonFlush(PyObject* poSelf, PyObject* poArgs)
{
	if (PyTuple_Size(poArgs) != 1)
		return Py_BuildException();

	int isButtonFlush = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &isButtonFlush))
		return Py_BadArgument();

	CPythonPlayer::Instance().SetBattleButtonFlush(isButtonFlush);
	return Py_BuildNone();
}

PyObject* playerIsBattleFieldOpen(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("b", CPythonPlayer::Instance().IsBattleFieldOpen());
}

PyObject* playerSetBattleFieldOpen(PyObject* poSelf, PyObject* poArgs)
{
	if (PyTuple_Size(poArgs) != 1)
		return Py_BuildException();

	int isOpen = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &isOpen))
		return Py_BadArgument();

	CPythonPlayer::Instance().SetBattleFieldOpen(isOpen);
	return Py_BuildNone();
}

PyObject* playerIsBattleFieldEventOpen(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("b", CPythonPlayer::Instance().IsBattleFieldEventOpen());
}

PyObject* playerSetBattleFieldEventOpen(PyObject* poSelf, PyObject* poArgs)
{
	if (PyTuple_Size(poArgs) != 1)
		return Py_BuildException();

	int isEventOpen = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &isEventOpen))
		return Py_BadArgument();

	CPythonPlayer::Instance().SetBattleFieldEventOpen(isEventOpen);
	return Py_BuildNone();
}

PyObject* playerSetBattleFieldInfo(PyObject* poSelf, PyObject* poArgs)
{
	if (PyTuple_Size(poArgs) != 1)
		return Py_BuildException();

	int isEnable = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &isEnable))
		return Py_BadArgument();

	CPythonPlayer::Instance().SetBattleFieldInfo(isEnable);
	return Py_BuildNone();
}

PyObject* playerSetBattleFieldEventInfo(PyObject* poSelf, PyObject* poArgs)
{
	bool bIsOpen = false;
	if (PyTuple_Size(poArgs) == 1)
	{
		if (!PyTuple_GetBoolean(poArgs, 0, &bIsOpen))
			return Py_BadArgument();
		return Py_BadArgument();
	}

	if (PyTuple_Size(poArgs) != 3)
		return Py_BuildException();

	if (!PyTuple_GetBoolean(poArgs, 0, &bIsOpen))
		return Py_BadArgument();

	int iStartTime;
	if (!PyTuple_GetInteger(poArgs, 1, &iStartTime))
		return Py_BadArgument();

	int iEndTime;
	if (!PyTuple_GetInteger(poArgs, 2, &iEndTime))
		return Py_BadArgument();

	CPythonPlayer::Instance().SetBattleFieldEventInfo(bIsOpen, iStartTime, iEndTime);
	return Py_BuildNone();
}

PyObject* playerGetBattleFieldEnable(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("b", CPythonPlayer::Instance().GetBattleFieldEnable());
}

PyObject* playerGetBattleFieldEventEnable(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("b", CPythonPlayer::Instance().GetBattleFieldEventEnable());
}
#endif

PyObject* playerIsGameMaster(PyObject* poSelf, PyObject* poArgs)
{
	CInstanceBase* pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
	if (!pMainInstance)
		return Py_BuildValue("i", 0);

	if (pMainInstance->IsGameMaster())
		return Py_BuildValue("i", 1);

	return Py_BuildValue("i", 0);
}

#ifdef ENABLE_LOADING_TIP
PyObject* playerGetLoadingTip(PyObject* poSelf, PyObject* poArgs)
{
	const char* szLoadingTip;
	long lMapIndex;

	std::string stringMapName = CPythonBackground::Instance().GetWarpMapName();

	if (stringMapName == "metin2_12zi_stage") // 12zi
		lMapIndex = 358;
	else
		lMapIndex = CPythonNetworkStream::Instance().GetMapTarget();
	szLoadingTip = CPythonNetworkStream::Instance().GetLoadingTipVnum(lMapIndex);

	return Py_BuildValue("s", szLoadingTip);
}
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
PyObject* playerSetSkillColor(PyObject* poSelf, PyObject* poArgs)
{
	int iSkillSlot;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillSlot))
		return Py_BadArgument();

	int iColor1;
	if (!PyTuple_GetInteger(poArgs, 1, &iColor1))
		return Py_BadArgument();

	int iColor2;
	if (!PyTuple_GetInteger(poArgs, 2, &iColor2))
		return Py_BadArgument();

	int iColor3;
	if (!PyTuple_GetInteger(poArgs, 3, &iColor3))
		return Py_BadArgument();

	int iColor4;
	if (!PyTuple_GetInteger(poArgs, 4, &iColor4))
		return Py_BadArgument();

	int iColor5;
	if (!PyTuple_GetInteger(poArgs, 5, &iColor5))
		return Py_BadArgument();

	if (iSkillSlot >= 255 || iSkillSlot < 0 || iColor1 < 0 || iColor2 < 0 || iColor3 < 0 || iColor4 < 0 || iColor5 < 0)
		return Py_BadArgument();

	CPythonNetworkStream& nstr = CPythonNetworkStream::Instance();
	nstr.SendSkillColorPacket(iSkillSlot, iColor1, iColor2, iColor3, iColor4, iColor5);

	return Py_BuildNone();
}

PyObject* playerGetSkillColor(PyObject* poSelf, PyObject* poArgs)
{
	int iSkillSlot;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillSlot))
		return Py_BadArgument();

	if (iSkillSlot >= 255 || iSkillSlot < 0)
		return Py_BadArgument();

	CInstanceBase* pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
	uint32_t* dwSkillColor = pMainInstance->GetSkillColor(iSkillSlot);
	if (!dwSkillColor)
		return Py_BuildValue("iiiii", 0, 0, 0, 0, 0);

	return Py_BuildValue("iiiii", dwSkillColor[0], dwSkillColor[1], dwSkillColor[2], dwSkillColor[3], dwSkillColor[4]);
}
#endif

#ifdef ENABLE_ELEMENT_ADD
PyObject* playerGetElementByVID(PyObject* poSelf, PyObject* poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	uint32_t dwElement = CPythonPlayer::Instance().GetElementByVID(iVID);
	//TraceError("dwElement: %d", dwElement);
	return Py_BuildValue("i", dwElement);
}
#endif

#ifdef ENABLE_MEDAL_OF_HONOR
PyObject* playerGetMedalOfHonor(PyObject* poSelf, PyObject* poArgs) // this not exist on official, but is neccesary for info shopEx
{
	return Py_BuildValue("b", CPythonPlayer::Instance().GetStatus(POINT_MEDAL_OF_HONOR));
}
#endif

#ifdef ENABLE_REFINE_ELEMENT
enum
{
	ELEMENT_UPGRADE_CANT_ADD,
	ELEMENT_UPGRADE_ADD,

	ELEMENT_DOWNGRADE_CANT_ADD,
	ELEMENT_DOWNGRADE_ADD,

	ELEMENT_CHANGE_CANT_ADD,
	ELEMENT_CHANGE_ADD,

	ELEMENT_CANT,
};

PyObject* playerElements(PyObject* poSelf, PyObject* poArgs)
{
	int VnumItemElement;
	TItemPos SlotIndexItemWeapon;
	TItemPos SlotIndexItemElements;

	switch (PyTuple_Size(poArgs))
	{
	case 2:
		if (!PyTuple_GetInteger(poArgs, 0, &VnumItemElement))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &SlotIndexItemWeapon.cell))
			return Py_BuildException();
		break;
	default:
		return Py_BuildException();
	}

	if (CPythonPlayer::Instance().IsEquipmentSlot(SlotIndexItemWeapon))
		return Py_BuildValue("i", ELEMENT_CANT);

	CItemData* pItemElementData;
	if (!CItemManager::Instance().GetItemDataPointer(VnumItemElement, &pItemElementData))
		return Py_BuildException("can't find item data");

	uint32_t dwSlotIndexItemWeapon = CPythonPlayer::Instance().GetItemIndex(SlotIndexItemWeapon);
	CItemData* pSlotIndexItemWeapon;
	if (!CItemManager::Instance().GetItemDataPointer(dwSlotIndexItemWeapon, &pSlotIndexItemWeapon))
		return Py_BuildException("can't find item data");

	int iSubTypeItemElement = pItemElementData->GetSubType();
	int iTypeWeapon = pSlotIndexItemWeapon->GetType();

	if (CItemData::USE_ELEMENT_UPGRADE == iSubTypeItemElement && CItemData::ITEM_TYPE_WEAPON == iTypeWeapon)
	{
		uint8_t dwGradeItem = CPythonPlayer::Instance().GetElementGrade(SlotIndexItemWeapon);

		uint8_t byType_Item_Weapon = CPythonPlayer::Instance().GetElementType(SlotIndexItemWeapon);

		CItemManager::Instance().SelectItemData(dwSlotIndexItemWeapon);

		CItemData* pItemWeaponData = CItemManager::Instance().GetSelectedItemDataPointer();
		if (!pItemWeaponData)
			return Py_BuildException("Can't find item data");

		if ((dwGradeItem >= 0 && dwGradeItem < 3) && (GetItemGrade(pItemWeaponData->GetName()) >= 7))
		{
			if (dwGradeItem > 0)
			{
				if (byType_Item_Weapon != pItemElementData->GetValue(0))
					return Py_BuildValue("i", ELEMENT_UPGRADE_CANT_ADD);

				return Py_BuildValue("i", ELEMENT_UPGRADE_ADD);
			}

			return Py_BuildValue("i", ELEMENT_UPGRADE_ADD);
		}

		return Py_BuildValue("i", ELEMENT_UPGRADE_CANT_ADD);
	}
	else if (CItemData::USE_ELEMENT_DOWNGRADE == iSubTypeItemElement && CItemData::ITEM_TYPE_WEAPON == iTypeWeapon)
	{
		uint8_t dwGradeItem = CPythonPlayer::Instance().GetElementGrade(SlotIndexItemWeapon);
		uint8_t byType_Item_Weapon = CPythonPlayer::Instance().GetElementType(SlotIndexItemWeapon);

		CItemManager::Instance().SelectItemData(dwSlotIndexItemWeapon);

		CItemData* pItemWeaponData = CItemManager::Instance().GetSelectedItemDataPointer();
		if (!pItemWeaponData)
			return Py_BuildException("Can't find item data");

		if ((dwGradeItem > 0 && dwGradeItem <= 3) && (GetItemGrade(pItemWeaponData->GetName()) >= 7))
			return Py_BuildValue("i", ELEMENT_DOWNGRADE_ADD);

		return Py_BuildValue("i", ELEMENT_DOWNGRADE_CANT_ADD);
	}
	else if (CItemData::USE_ELEMENT_CHANGE == iSubTypeItemElement && CItemData::ITEM_TYPE_WEAPON == iTypeWeapon)
	{
		uint8_t dwGradeItem = CPythonPlayer::Instance().GetElementGrade(SlotIndexItemWeapon);
		uint8_t byType_Item_Weapon = CPythonPlayer::Instance().GetElementType(SlotIndexItemWeapon);

		CItemManager::Instance().SelectItemData(dwSlotIndexItemWeapon);

		CItemData* pItemWeaponData = CItemManager::Instance().GetSelectedItemDataPointer();
		if (!pItemWeaponData)
			return Py_BuildException("Can't find item data");

		if ((dwGradeItem > 0 && dwGradeItem <= 3) && (GetItemGrade(pItemWeaponData->GetName()) >= 7))
			return Py_BuildValue("i", ELEMENT_CHANGE_ADD);

		return Py_BuildValue("i", ELEMENT_CHANGE_CANT_ADD);
	}

	return Py_BuildValue("i", ELEMENT_CANT);
}

PyObject * playerGetItemElementGrade(PyObject* poSelf, PyObject* poArgs)
{
	switch (PyTuple_Size(poArgs))
	{
		case 1:
		{
			int iSlotIndex;
			if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
				return Py_BuildException();

			uint32_t dwElementGrade = CPythonPlayer::Instance().GetElementGrade(TItemPos(INVENTORY, iSlotIndex));
			return Py_BuildValue("i", dwElementGrade);
		}
		case 2:
		{
			TItemPos Cell;
			if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
				return Py_BuildException();

			if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
				return Py_BuildException();

			uint32_t dwElementGrade = CPythonPlayer::Instance().GetElementGrade(Cell);
			return Py_BuildValue("i", dwElementGrade);
		}

		default:
			return Py_BuildException();
	}
}

PyObject * playerGetItemElementAttack(PyObject* poSelf, PyObject* poArgs)
{
	int iAttackIndex;

	switch (PyTuple_Size(poArgs))
	{
		case 2:
		{
			int iSlotIndex;
			if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
				return Py_BuildException();
			if (!PyTuple_GetInteger(poArgs, 1, &iAttackIndex))
				return Py_BuildException();

			uint32_t dwElementAttack = CPythonPlayer::Instance().GetElementAttack(TItemPos(INVENTORY, iSlotIndex),iAttackIndex);
			return Py_BuildValue("i", dwElementAttack);
		}
		case 3:
		{
			TItemPos Cell;
			if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
				return Py_BuildException();

			if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
				return Py_BuildException();

			if (!PyTuple_GetInteger(poArgs, 2, &iAttackIndex))
				return Py_BuildException();

			uint32_t dwElementAttack = CPythonPlayer::Instance().GetElementAttack(Cell,iAttackIndex);
			return Py_BuildValue("i", dwElementAttack);
		}

		default:
			return Py_BuildException();
	}
}

PyObject * playerGetItemElementType(PyObject* poSelf, PyObject* poArgs)
{
	switch (PyTuple_Size(poArgs))
	{
		case 1:
		{
			int iSlotIndex;
			if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
				return Py_BuildException();

			uint8_t dwElementType = CPythonPlayer::Instance().GetElementType(TItemPos(INVENTORY, iSlotIndex));
			return Py_BuildValue("i", dwElementType);
		}
		case 2:
		{
			TItemPos Cell;
			if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
				return Py_BuildException();

			if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
				return Py_BuildException();

			uint8_t dwElementType = CPythonPlayer::Instance().GetElementType(Cell);
			return Py_BuildValue("i", dwElementType);
		}

		default:
			return Py_BuildException();
	}
}

PyObject * playerGetItemElementValue(PyObject* poSelf, PyObject* poArgs)
{
	int iValueIndex;

	switch (PyTuple_Size(poArgs))
	{
		case 2:
		{
			int iSlotIndex;
			if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
				return Py_BuildException();
			if (!PyTuple_GetInteger(poArgs, 1, &iValueIndex))
				return Py_BuildException();

			int16_t dwElementValue = CPythonPlayer::Instance().GetElementValue(TItemPos(INVENTORY, iSlotIndex),iValueIndex);
			return Py_BuildValue("i", dwElementValue);
		}
		case 3:
		{
			TItemPos Cell;
			if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
				return Py_BuildException();

			if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
				return Py_BuildException();

			if (!PyTuple_GetInteger(poArgs, 2, &iValueIndex))
				return Py_BuildException();

			int16_t dwElementValue = CPythonPlayer::Instance().GetElementValue(Cell,iValueIndex);
			return Py_BuildValue("i", dwElementValue);
		}

		default:
			return Py_BuildException();
	}
}
#endif

#ifdef ENABLE_RENDER_TARGET
PyObject* playerGetEquippedWeapon(PyObject* poSelf, PyObject* poArgs)
{
	const CPythonPlayer* player = CPythonPlayer::InstancePtr();
	const TItemData* data = player->GetItemData(TItemPos(EQUIPMENT, c_Equipment_Weapon));
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	const TItemData* data_costume = player->GetItemData(TItemPos(EQUIPMENT, c_Costume_Slot_Weapon));

	if (nullptr != data_costume) {
		if (data_costume->vnum != 0)
			return Py_BuildValue("i", data_costume->vnum);
	}
#endif

	if (nullptr != data) {
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		if (data->dwTransmutationVnum != 0)
			return Py_BuildValue("i", data->dwTransmutationVnum);
#endif
		if (data->vnum != 0)
			return Py_BuildValue("i", data->vnum);
	}

	if (!data)
		return Py_BuildValue("i", 0);

	return Py_BuildNone();
}

PyObject* playerGetEquippedArmor(PyObject* poSelf, PyObject* poArgs)
{
	const CPythonPlayer* player = CPythonPlayer::InstancePtr();
	const TItemData* data = player->GetItemData(TItemPos(EQUIPMENT, c_Equipment_Body));
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	const TItemData* data_costume = player->GetItemData(TItemPos(EQUIPMENT, c_Costume_Slot_Body));

	if (nullptr != data_costume) {
		if (data_costume->vnum != 0)
			return Py_BuildValue("i", data_costume->vnum);
	}
#endif

	if (nullptr != data) {
		if (data->vnum != 0)
			return Py_BuildValue("i", data->vnum);
	}

	if (!data)
		return Py_BuildValue("i", 0);

	return Py_BuildNone();
}

PyObject* playerGetEquippedHair(PyObject* poSelf, PyObject* poArgs)
{
	const CPythonPlayer* player = CPythonPlayer::InstancePtr();
	const TItemData* data = player->GetItemData(TItemPos(EQUIPMENT, c_Costume_Slot_Hair));

	if (nullptr != data) {
		if (data->vnum != 0)
			return Py_BuildValue("i", data->vnum);
	}

	if (!data)
		return Py_BuildValue("i", 0);

	return Py_BuildNone();
}

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
PyObject* playerGetEquippedAcce(PyObject* poSelf, PyObject* poArgs)
{
	const CPythonPlayer* player = CPythonPlayer::InstancePtr();
	const TItemData* data = player->GetItemData(TItemPos(EQUIPMENT, c_Costume_Slot_Acce));

	if (nullptr != data)
	{
		if (data->vnum != 0)
			return Py_BuildValue("i", data->vnum);
	}

	if (!data)
		return Py_BuildValue("i", 0);

	return Py_BuildNone();
}
#endif

#ifdef ENABLE_AURA_SYSTEM
PyObject* playerGetEquippedAura(PyObject* poSelf, PyObject* poArgs)
{
	const CPythonPlayer* player = CPythonPlayer::InstancePtr();
	const TItemData* data = player->GetItemData(TItemPos(EQUIPMENT, c_Costume_Slot_Aura));

	if (nullptr != data)
	{
		if (data->vnum != 0)
			return Py_BuildValue("i", data->vnum);
	}

	if (!data)
		return Py_BuildValue("i", 0);

	return Py_BuildNone();
}
#endif
#endif

#ifdef ENABLE_OFFICAL_FEATURES
PyObject* playerIsPoly(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().IsPoly());
}

PyObject* playerIsOpenSafeBox(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().IsOpenSafeBox());
}

PyObject* playerSetOpenSafeBox(PyObject* poSelf, PyObject* poArgs)
{
	int openState;
	if (!PyTuple_GetInteger(poArgs, 0, &openState))
		return Py_BadArgument();

	CPythonPlayer::Instance().SetOpenSafeBox(openState);
	return Py_BuildValue("i", 0);
}

PyObject* playerIsOpenMall(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().IsOpenMall());
}

PyObject* playerSetOpenMall(PyObject* poSelf, PyObject* poArgs)
{
	int openState;
	if (!PyTuple_GetInteger(poArgs, 0, &openState))
		return Py_BadArgument();

	CPythonPlayer::Instance().SetOpenMall(openState);
	return Py_BuildValue("i", 0);
}
#endif

#ifdef ENABLE_EXTEND_INVEN_SYSTEM
PyObject* playerGetExtendInvenStage(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetExtendInvenStage());
}

PyObject* playerGetExtendInvenMax(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetExtendInvenMax());
}

# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
PyObject* playerGetExtendSpecialInvenStage(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bWindow;
	if (!PyTuple_GetInteger(poArgs, 0, &bWindow))
		return Py_BadArgument();

	return Py_BuildValue("i", CPythonPlayer::Instance().GetExtendSpecialInvenStage(bWindow));
}

PyObject* playerGetExtendSpecialInvenMax(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bWindow;
	if (!PyTuple_GetInteger(poArgs, 0, &bWindow))
		return Py_BadArgument();

	return Py_BuildValue("i", CPythonPlayer::Instance().GetExtendSpecialInvenMax(bWindow));
}
# endif
#endif

#ifdef ENABLE_MOVE_COSTUME_ATTR
PyObject* playerCanAttachToCombMediumSlot(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos Cell;
	if (!PyTuple_GetByte(poArgs, 1, &Cell.window_type))
		return Py_BadArgument();

	if (!PyTuple_GetInteger(poArgs, 2, &Cell.cell))
		return Py_BadArgument();

	int ItemType = CPythonPlayer::Instance().GetItemTypeBySlot(Cell);
	if (CItemData::ITEM_TYPE_MEDIUM != ItemType)
		return Py_BuildValue("i", FALSE);

	return Py_BuildValue("i", TRUE);
}

PyObject* playerCanAttachToCombItemSlot(PyObject* poSelf, PyObject* poArgs)
{
	if (PyTuple_Size(poArgs) != 3)
		return Py_BuildException();

	int selectedSlotPos;
	if (!PyTuple_GetInteger(poArgs, 0, &selectedSlotPos))
		return Py_BuildException();

	int attachedSlotType;
	if (!PyTuple_GetInteger(poArgs, 1, &attachedSlotType))
		return Py_BuildException();

	int attachedSlotPos;
	if (!PyTuple_GetInteger(poArgs, 2, &attachedSlotPos))
		return Py_BuildException();

	if (selectedSlotPos <= 0 || selectedSlotPos > 2)
		return Py_BuildException("SlotIndex Error");

	TItemPos Cell;
	if (!PyTuple_GetByte(poArgs, 1, &Cell.window_type))
		return Py_BadArgument();

	if (!PyTuple_GetInteger(poArgs, 2, &Cell.cell))
		return Py_BadArgument();

	if (CPythonPlayer::Instance().GetItemMagicPctBySlot(Cell) != 100)
		return Py_BuildValue("i", FALSE);

	int ItemType = CPythonPlayer::Instance().GetItemTypeBySlot(Cell);
	int ItemSubType = CPythonPlayer::Instance().GetItemSubTypeBySlot(Cell);

	if (selectedSlotPos == 2)
	{
		if (CPythonPlayer::Instance().GetConbWindowSlotByAttachedInvenSlot(1) < 0)
			return Py_BuildValue("i", FALSE);

		if (ItemSubType != CPythonPlayer::Instance().GetItemSubTypeBySlot(TItemPos(INVENTORY, CPythonPlayer::Instance().GetConbWindowSlotByAttachedInvenSlot(1))))
			return Py_BuildValue("i", FALSE);
	}

	if (CItemData::ITEM_TYPE_COSTUME != ItemType ||
		(CItemData::COSTUME_BODY != ItemSubType && CItemData::COSTUME_HAIR != ItemSubType && CItemData::COSTUME_WEAPON != ItemSubType))
		return Py_BuildValue("i", FALSE);

	int iValue = CPythonPlayer::Instance().GetConbWindowSlotByAttachedInvenSlot(selectedSlotPos);
	if (iValue < 0)
		return Py_BuildValue("i", TRUE);

	return Py_BuildValue("i", FALSE);
}

PyObject* playerGetInvenSlotAttachedToConbWindowSlot(PyObject* poSelf, PyObject* poArgs)
{
	int attachedSlotPos;
	if (!PyTuple_GetInteger(poArgs, 0, &attachedSlotPos))
		return Py_BuildException();

	int iValue = CPythonPlayer::Instance().GetConbWindowSlotByAttachedInvenSlot(attachedSlotPos);
	if (iValue < 0)
		return Py_BuildValue("i", -1);

	return Py_BuildValue("i", iValue);
}

PyObject* playerGetConbWindowSlotByAttachedInvenSlot(PyObject* poSelf, PyObject* poArgs)
{
	int attachedSlotPos;
	if (!PyTuple_GetInteger(poArgs, 0, &attachedSlotPos))
		return Py_BuildException();

	for (int i = 0; i < COMB_WND_SLOT_MAX; ++i)
	{
		int selectedSlotPos = CPythonPlayer::Instance().GetConbWindowSlotByAttachedInvenSlot(i);
		if (attachedSlotPos == selectedSlotPos)
			return Py_BuildValue("i", 0);
	}

	return Py_BuildValue("i", COMB_WND_SLOT_MAX);
}

PyObject* playerSetItemCombinationWindowActivedItemSlot(PyObject* poSelf, PyObject* poArgs)
{
	int iSlot;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlot))
		return Py_BuildException();

	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iIndex))
		return Py_BuildException();

	CPythonPlayer::Instance().SetItemCombinationWindowActivedItemSlot(iSlot, iIndex);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_ACCE_SECOND_COSTUME_SYSTEM
PyObject* playerGetAcceRefineWindowOpen(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetAcceRefineWindowOpen());
}

PyObject* playerSetAcceRefineWindowOpen(PyObject* poSelf, PyObject* poArgs)
{
	int iWindowType;
	if (!PyTuple_GetInteger(poArgs, 0, &iWindowType))
		return Py_BadArgument();

	bool isOpen = (CPythonPlayer::Instance().GetAcceRefineWindowOpen() ? false : true);
	CPythonPlayer::Instance().SetAcceRefineWindowOpen(isOpen);
	CPythonPlayer::Instance().SetAcceRefineWindowType(iWindowType); // set window type

	return Py_BuildValue("i", 0);
}

PyObject* playerGetAcceRefineWindowType(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetAcceRefineWindowType());
}

PyObject* playerFineMoveAcceItemSlot(PyObject* poSelf, PyObject* poArgs)
{
	for (unsigned int i = 0; i < 3; ++i)
	{
		if (CPythonPlayer::Instance().GetAcceActivedItemSlot(i) == 180)
			return Py_BuildValue("i", i);
	}

	return Py_BuildValue("i", 3);
}

PyObject* playerGetCurrentItemCount(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bCount = 1;
	for (unsigned int i = 0; i < 3; ++i)
	{
		if (CPythonPlayer::Instance().GetAcceActivedItemSlot(i) != 180)
			bCount++;
	}

	return Py_BuildValue("i", bCount);
}

PyObject* playerSetAcceActivedItemSlot(PyObject* poSelf, PyObject* poArgs)
{
	int iAcceSlot;
	if (!PyTuple_GetInteger(poArgs, 0, &iAcceSlot))
		return Py_BadArgument();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BadArgument();

	if (iAcceSlot < 0 || iAcceSlot > 1)
		return Py_BuildNone();

	if (iAcceSlot == 1)
	{
		if (CPythonPlayer::Instance().GetAcceActivedItemSlot(0) == 180)
			return Py_BuildNone();
	}

	CPythonPlayer::Instance().SetAcceActivedItemSlot(iAcceSlot, iSlotIndex);
	return Py_BuildNone();
}

PyObject* playerFindActivedAcceSlot(PyObject* poSelf, PyObject* poArgs)
{
	int iAttachedSlotPos;
	if (!PyTuple_GetInteger(poArgs, 0, &iAttachedSlotPos))
		return Py_BadArgument();

	for (signed int i = 0; i < 3; ++i)
	{
		if (CPythonPlayer::Instance().GetAcceActivedItemSlot(i) == iAttachedSlotPos)
			return Py_BuildValue("i", i);
	}

	return Py_BuildValue("i", 3);
}

PyObject* playerFindUsingAcceSlot(PyObject* poSelf, PyObject* poArgs)
{
	int usingSlot;
	if (!PyTuple_GetInteger(poArgs, 0, &usingSlot))
		return Py_BadArgument();

	int result = 180; // 180 = ITEM_SLOT_COUNT
	if (usingSlot < 3)
		result = CPythonPlayer::Instance().GetAcceActivedItemSlot(usingSlot);

	return Py_BuildValue("i", result);
}

PyObject* playerIsAcceWindowEmpty(PyObject* poSelf, PyObject* poArgs)
{
	bool bIsAcceEmpty = true;
	for (uint8_t i = 0; i < 3; ++i)
	{
		if (CPythonPlayer::Instance().GetAcceActivedItemSlot(i) != 180)
		{
			bIsAcceEmpty = false;
			break;
		}
	}

	return Py_BuildValue("i", bIsAcceEmpty);
}

PyObject* playerGetAcceItemID(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bAcceSlot;
	if (!PyTuple_GetInteger(poArgs, 0, &bAcceSlot))
		return Py_BadArgument();

	if (bAcceSlot < 2 && CPythonPlayer::Instance().GetAcceActivedItemSlot(bAcceSlot) == 180)
		return Py_BuildValue("i", 0);

	uint16_t wCell = CPythonPlayer::Instance().GetAcceActivedItemSlot(bAcceSlot);
	int ItemIndex = CPythonPlayer::Instance().GetItemIndex(TItemPos(INVENTORY, wCell));

	if (bAcceSlot == 2)
	{
		uint8_t bWindowType = CPythonPlayer::Instance().GetAcceRefineWindowType();

		if (bWindowType == 0 && CPythonPlayer::Instance().GetAcceActivedItemSlot(0) == 180)
			return Py_BuildValue("i", 0);

		if (bWindowType == 1 && CPythonPlayer::Instance().GetAcceActivedItemSlot(1) == 180)
			return Py_BuildValue("i", 0);

		uint16_t wAcceItemCell = CPythonPlayer::Instance().GetAcceActivedItemSlot(0);

		CItemManager::Instance().SelectItemData(CPythonPlayer::Instance().GetItemIndex(TItemPos(INVENTORY, wAcceItemCell)));
		CItemData* pAcceItemData = CItemManager::Instance().GetSelectedItemDataPointer();

		if (!pAcceItemData)
			return Py_BuildException("Can't find acce select item data");

		if (CPythonPlayer::Instance().GetAcceRefineWindowType() == 0)
		{
			ItemIndex = pAcceItemData->GetRefinedVnum();
			if (ItemIndex == 0)
				ItemIndex = pAcceItemData->GetIndex();
		}
		else
			ItemIndex = pAcceItemData->GetIndex();
	}

	return Py_BuildValue("i", ItemIndex);
}

PyObject* playerGetAcceItemSize(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", 3);
}

PyObject* playerGetAcceItemFlags(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();

	int flags = CPythonPlayer::Instance().GetItemFlags(TItemPos(INVENTORY,
		CPythonPlayer::Instance().GetAcceActivedItemSlot(iSlotIndex)));
	return Py_BuildValue("i", flags);
}

PyObject* playerGetAcceItemMetinSocket(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();

	int iMetinSocketIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iMetinSocketIndex))
		return Py_BadArgument();

	if (iMetinSocketIndex >= 3)
		return Py_BuildValue("i", 0);

	int iMetinSocketValue = 0;
	if (iSlotIndex == ACCE_SLOT_LEFT || iSlotIndex == ACCE_SLOT_RIGHT)
		iMetinSocketValue = CPythonPlayer::Instance().GetItemMetinSocket(TItemPos(INVENTORY,
			CPythonPlayer::Instance().GetAcceActivedItemSlot(iSlotIndex)), iMetinSocketIndex);
	else if (iSlotIndex == ACCE_SLOT_RESULT)
	{
		if (ACCE_SLOT_TYPE_COMBINE == CPythonPlayer::Instance().GetAcceRefineWindowType())
		{
			const TItemData* pAcceItem = CPythonPlayer::Instance().GetItemData(TItemPos(INVENTORY,
				CPythonPlayer::Instance().GetAcceActivedItemSlot(ACCE_SLOT_LEFT)));
			if (!pAcceItem)
				return Py_BuildException("Can't find acce item");

			if (iMetinSocketIndex == 0)
				iMetinSocketValue = pAcceItem->alSockets[iMetinSocketIndex];
		}
		else
		{
			const TItemData* pAcceItem = CPythonPlayer::Instance().GetItemData(TItemPos(INVENTORY,
				CPythonPlayer::Instance().GetAcceActivedItemSlot(ACCE_SLOT_LEFT)));
			if (!pAcceItem)
				return Py_BuildException("Can't find acce item");

			if (pAcceItem->alSockets[iMetinSocketIndex] == 0)
				iMetinSocketValue = CPythonPlayer::Instance().GetItemIndex(TItemPos(INVENTORY,
					CPythonPlayer::Instance().GetAcceActivedItemSlot(ACCE_SLOT_RIGHT)));
			else
				iMetinSocketValue = pAcceItem->alSockets[iMetinSocketIndex];
		}
	}

	return Py_BuildValue("i", iMetinSocketValue);
}

PyObject* playerGetAcceItemAttribute(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();

	int iAttributeSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAttributeSlotIndex))
		return Py_BadArgument();

	if (iAttributeSlotIndex < 0 || iAttributeSlotIndex >= 7)
		return Py_BuildValue("ii", 0, 0);

	uint16_t wType;
	int16_t sValue;
	if (iSlotIndex == ACCE_SLOT_LEFT || iSlotIndex == ACCE_SLOT_RIGHT)
		CPythonPlayer::Instance().GetItemAttribute(TItemPos(INVENTORY,
			CPythonPlayer::Instance().GetAcceActivedItemSlot(iSlotIndex)), iAttributeSlotIndex, &wType, &sValue);
	else if (iSlotIndex == ACCE_SLOT_RESULT)
	{
		if (ACCE_SLOT_TYPE_COMBINE == CPythonPlayer::Instance().GetAcceRefineWindowType())
		{
			CPythonPlayer::Instance().GetItemAttribute(TItemPos(INVENTORY,
				CPythonPlayer::Instance().GetAcceActivedItemSlot(ACCE_SLOT_LEFT)), iAttributeSlotIndex, &wType, &sValue);
			if (sValue < 0)
				return Py_BuildValue("ii", 0, 0);
		}
		else
		{
			CPythonPlayer::Instance().GetItemAttribute(TItemPos(INVENTORY,
				CPythonPlayer::Instance().GetAcceActivedItemSlot(ACCE_SLOT_RIGHT)), iAttributeSlotIndex, &wType, &sValue);
			if (sValue < 0)
				return Py_BuildValue("ii", 0, 0);
		}
	}

	return Py_BuildValue("ii", wType, sValue);
}

PyObject* playerCanAcceClearItem(PyObject* poSelf, PyObject* poArgs)
{
	if (PyTuple_Size(poArgs) != 3)
		return Py_BuildException();

	int iScrollItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iScrollItemIndex))
		return Py_BadArgument();
	TItemPos TargetSlotIndex;
	if (!PyTuple_GetByte(poArgs, 1, &TargetSlotIndex.window_type))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 2, &TargetSlotIndex.cell))
		return Py_BuildException();

	if (iScrollItemIndex != 90000 && iScrollItemIndex != 39046)
		return Py_BuildValue("i", 0);

	// Scroll
	CItemManager::Instance().SelectItemData(iScrollItemIndex);
	CItemData* pScrollItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pScrollItemData)
		return Py_BuildException("Can't find item data");
	if (pScrollItemData->GetType() != CItemData::ITEM_TYPE_USE || pScrollItemData->GetSubType() != CItemData::USE_SPECIAL)
		return Py_BuildValue("i", 0);

	// Target Item
	int iTargetItemIndex = CPythonPlayer::Instance().GetItemIndex(TargetSlotIndex);
	CItemManager::Instance().SelectItemData(iTargetItemIndex);
	CItemData* pTargetItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pTargetItemData)
		return Py_BuildException("Can't find item data");
	if (pTargetItemData->GetType() != CItemData::ITEM_TYPE_COSTUME || pTargetItemData->GetSubType() != CItemData::COSTUME_ACCE)
		return Py_BuildValue("i", 0);

	uint32_t dwSocketValue = CPythonPlayer::Instance().GetItemMetinSocket(TargetSlotIndex, 0);
	if (dwSocketValue != 0)
		return Py_BuildValue("i", 1);

	return Py_BuildValue("i", 0);
}

#	ifdef ENABLE_YOHARA_SYSTEM
PyObject* playerGetAcceRandomItemAttribute(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();

	int iAttributeSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAttributeSlotIndex))
		return Py_BadArgument();

	if (iAttributeSlotIndex < 0 || iAttributeSlotIndex >= APPLY_RANDOM_SLOT_MAX_NUM)
		return Py_BuildValue("ii", 0, 0);

	uint16_t wType;
	int16_t sValue;
	if (iSlotIndex == ACCE_SLOT_LEFT || iSlotIndex == ACCE_SLOT_RIGHT)
		CPythonPlayer::Instance().GetItemApplyRandom(TItemPos(INVENTORY, CPythonPlayer::Instance().GetAcceActivedItemSlot(iSlotIndex)), iAttributeSlotIndex, &wType, &sValue);
	else if (iSlotIndex == ACCE_SLOT_RESULT)
	{
		if (ACCE_SLOT_TYPE_COMBINE == CPythonPlayer::Instance().GetAcceRefineWindowType())
		{
			CPythonPlayer::Instance().GetItemApplyRandom(TItemPos(INVENTORY, CPythonPlayer::Instance().GetAcceActivedItemSlot(ACCE_SLOT_LEFT)), iAttributeSlotIndex, &wType, &sValue);
			if (sValue < 0)
				return Py_BuildValue("ii", 0, 0);
		}
		else
		{
			CPythonPlayer::Instance().GetItemApplyRandom(TItemPos(INVENTORY, CPythonPlayer::Instance().GetAcceActivedItemSlot(ACCE_SLOT_RIGHT)), iAttributeSlotIndex, &wType, &sValue);
			if (sValue < 0)
				return Py_BuildValue("ii", 0, 0);
		}
	}

	return Py_BuildValue("ii", wType, sValue);
}

PyObject* playerGetAcceItemRandomValue(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();

	int iRandomValueIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iRandomValueIndex))
		return Py_BadArgument();

	if (iRandomValueIndex >= ITEM_RANDOM_VALUES_MAX_NUM)
		return Py_BuildValue("i", 0);

	int iDefaultRandomValue = 0;
	if (iSlotIndex == ACCE_SLOT_LEFT || iSlotIndex == ACCE_SLOT_RIGHT)
		iDefaultRandomValue = CPythonPlayer::Instance().GetItemMetinSocket(TItemPos(INVENTORY, CPythonPlayer::Instance().GetAcceActivedItemSlot(iSlotIndex)), iRandomValueIndex);
	else if (iSlotIndex == ACCE_SLOT_RESULT)
	{
		if (ACCE_SLOT_TYPE_COMBINE == CPythonPlayer::Instance().GetAcceRefineWindowType())
		{
			const TItemData* pAcceItem = CPythonPlayer::Instance().GetItemData(TItemPos(INVENTORY, CPythonPlayer::Instance().GetAcceActivedItemSlot(ACCE_SLOT_LEFT)));
			if (!pAcceItem)
				return Py_BuildException("Can't find acce item");

			if (iRandomValueIndex == 0)
				iDefaultRandomValue = pAcceItem->alRandomValues[iRandomValueIndex];
		}
		else
		{
			const TItemData* pAcceItem = CPythonPlayer::Instance().GetItemData(TItemPos(INVENTORY, CPythonPlayer::Instance().GetAcceActivedItemSlot(ACCE_SLOT_LEFT)));
			if (!pAcceItem)
				return Py_BuildException("Can't find acce item");

			if (pAcceItem->alRandomValues[iRandomValueIndex] == 0)
				iDefaultRandomValue = CPythonPlayer::Instance().GetItemIndex(TItemPos(INVENTORY, CPythonPlayer::Instance().GetAcceActivedItemSlot(ACCE_SLOT_RIGHT)));
			else
				iDefaultRandomValue = pAcceItem->alRandomValues[iRandomValueIndex];
		}
	}

	return Py_BuildValue("i", iRandomValueIndex);
}
#	endif

#	ifdef ENABLE_SET_ITEM
PyObject* playerGetAcceSetValue(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();

	int setValue = CPythonPlayer::Instance().GetItemSetValue(TItemPos(INVENTORY, CPythonPlayer::Instance().GetAcceActivedItemSlot(iSlotIndex)));
	return Py_BuildValue("i", setValue);
}
#	endif
#endif

#ifdef ENABLE_AURA_SYSTEM
PyObject* playerGetAuraItemID(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotPos;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotPos))
		return Py_BadArgument();

	TItemData* pAuraItemInstance;
	if (!CPythonPlayer::Instance().GetAuraItemDataPtr(iSlotPos, &pAuraItemInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pAuraItemInstance->vnum);
}

PyObject* playerGetAuraItemCount(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotPos;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotPos))
		return Py_BadArgument();

	TItemData* pAuraItemInstance;
	if (!CPythonPlayer::Instance().GetAuraItemDataPtr(iSlotPos, &pAuraItemInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pAuraItemInstance->count);
}

PyObject* playerGetAuraItemMetinSocket(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotPos;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotPos))
		return Py_BadArgument();
	int iMetinSocketIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iMetinSocketIndex))
		return Py_BadArgument();

	TItemData* pAuraItemInstance;
	if (!CPythonPlayer::Instance().GetAuraItemDataPtr(iSlotPos, &pAuraItemInstance))
		return Py_BuildException();

	if (iMetinSocketIndex >= ITEM_SOCKET_SLOT_MAX_NUM || iMetinSocketIndex < 0)
		return Py_BuildException();

	return Py_BuildValue("i", pAuraItemInstance->alSockets[iMetinSocketIndex]);
}

PyObject* playerGetAuraItemAttribute(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotPos;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotPos))
		return Py_BadArgument();
	int iAttributeSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAttributeSlotIndex))
		return Py_BadArgument();

	TItemData* pAuraItemInstance;
	if (!CPythonPlayer::Instance().GetAuraItemDataPtr(iSlotPos, &pAuraItemInstance) || (iAttributeSlotIndex >= ITEM_ATTRIBUTE_SLOT_MAX_NUM || iAttributeSlotIndex < 0))
		return Py_BuildValue("ii", 0, 0);

	TPlayerItemAttribute kAttr = pAuraItemInstance->aAttr[iAttributeSlotIndex];
	return Py_BuildValue("ii", kAttr.wType, kAttr.sValue);
}

PyObject* playerIsAuraRefineWindowEmpty(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().IsAuraRefineWindowEmpty());
}

PyObject* playerGetAuraCurrentItemSlotCount(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetAuraCurrentItemSlotCount());
}

PyObject* playerIsAuraRefineWindowOpen(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().IsAuraRefineWindowOpen());
}

PyObject* playerGetAuraRefineWindowType(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetAuraRefineWindowType());
}

PyObject* playerFineMoveAuraItemSlot(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().FineMoveAuraItemSlot());
}

PyObject* playerSetAuraActivatedItemSlot(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bAuraSlotPos;
	if (!PyTuple_GetByte(poArgs, 0, &bAuraSlotPos))
		return Py_BuildException();

	TItemPos ItemCell;
	if (!PyTuple_GetByte(poArgs, 1, &ItemCell.window_type))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 2, &ItemCell.cell))
		return Py_BuildException();

	CPythonPlayer::Instance().SetActivatedAuraSlot(bAuraSlotPos, ItemCell);
	return Py_BuildNone();
}

PyObject* playerFindActivatedAuraSlot(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos ItemCell;
	if (!PyTuple_GetByte(poArgs, 0, &ItemCell.window_type))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 1, &ItemCell.cell))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonPlayer::Instance().FindActivatedAuraSlot(ItemCell));
}

PyObject* playerFindUsingAuraSlot(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bAuraSlotPos;
	if (!PyTuple_GetInteger(poArgs, 0, &bAuraSlotPos))
		return Py_BuildException();

	TItemPos ItemCell = CPythonPlayer::Instance().FindUsingAuraSlot(bAuraSlotPos);
	return Py_BuildValue("(ii)", ItemCell.window_type, ItemCell.cell);
}

PyObject* playerGetAuraRefineInfo(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bLevel;
	if (!PyTuple_GetByte(poArgs, 0, &bLevel))
		return Py_BuildException();

	uint8_t bRefineInfoIndex;
	if (!PyTuple_GetByte(poArgs, 1, &bRefineInfoIndex))
		return Py_BuildException();

	const int* info = GetAuraRefineInfo(bLevel);
	if (!info || bRefineInfoIndex < AURA_REFINE_INFO_STEP || bRefineInfoIndex >= AURA_REFINE_INFO_MAX)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", info[bRefineInfoIndex]);
}

PyObject* playerGetAuraRefineInfoExpPer(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bAuraRefineInfoSlot;
	if (!PyTuple_GetInteger(poArgs, 0, &bAuraRefineInfoSlot))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonPlayer::Instance().GetAuraRefineInfoExpPct(bAuraRefineInfoSlot));
}

PyObject* playerGetAuraRefineInfoLevel(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bAuraRefineInfoSlot;
	if (!PyTuple_GetInteger(poArgs, 0, &bAuraRefineInfoSlot))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonPlayer::Instance().GetAuraRefineInfoLevel(bAuraRefineInfoSlot));
}

#	ifdef ENABLE_YOHARA_SYSTEM
PyObject* playerGetAuraSlotItemApplyRandom(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotPos;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotPos))
		return Py_BadArgument();
	int iAttributeSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAttributeSlotIndex))
		return Py_BadArgument();

	TItemData* pAuraItemInstance;
	if (!CPythonPlayer::Instance().GetAuraItemDataPtr(iSlotPos, &pAuraItemInstance) || (iAttributeSlotIndex >= APPLY_RANDOM_SLOT_MAX_NUM || iAttributeSlotIndex < 0))
		return Py_BuildValue("ii", 0, 0);

	TPlayerItemApplyRandom kAttr = pAuraItemInstance->aApplyRandom[iAttributeSlotIndex];
	return Py_BuildValue("ii", kAttr.wType, kAttr.sValue);
}

PyObject* playerGetAuraRandomValue(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotPos;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotPos))
		return Py_BadArgument();

	int iRandomValueIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iRandomValueIndex))
		return Py_BadArgument();

	TItemData* pAuraItemInstance;
	if (!CPythonPlayer::Instance().GetAuraItemDataPtr(iSlotPos, &pAuraItemInstance))
		return Py_BuildException();

	if (iRandomValueIndex >= ITEM_RANDOM_VALUES_MAX_NUM || iRandomValueIndex < 0)
		return Py_BuildException();

	return Py_BuildValue("i", pAuraItemInstance->alRandomValues[iRandomValueIndex]);
}
#	endif

#	ifdef ENABLE_SET_ITEM
PyObject* playerGetAuraSetValue(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotPos;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotPos))
		return Py_BadArgument();

	TItemData* pAuraItemInstance;
	if (!CPythonPlayer::Instance().GetAuraItemDataPtr(iSlotPos, &pAuraItemInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pAuraItemInstance->set_value);
}
#	endif
#endif

PyObject* playerNPOS(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("(ii)", NPOS.window_type, NPOS.cell);
}

#ifdef ENABLE_CHANGED_ATTR
PyObject* playerGetItemChangedAttribute(PyObject* poSelf, PyObject* poArgs)
{
	int iAttributeSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iAttributeSlotIndex))
		return Py_BuildException();

	uint16_t wType;
	short sValue;
	CPythonPlayer::Instance().GetItemChangedAttribute(iAttributeSlotIndex, &wType, &sValue);

	return Py_BuildValue("ii", wType, sValue);
}
#endif

#ifdef ENABLE_CUBE_RENEWAL
PyObject* playerIsCubeRenewalOpen(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().IsOpenCubeRenewal());
}
#endif

#ifdef ENABLE_AUTO_SYSTEM
PyObject* playerSetAutoSkillSlotIndex(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();

	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iIndex))
		return Py_BadArgument();

	CPythonPlayer::Instance().SetAutoSkillSlotIndex(iSlotIndex, iIndex);
	return Py_BuildNone();
}

PyObject* playerSetAutoPositionSlotIndex(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();

	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iIndex))
		return Py_BadArgument();

	CPythonPlayer::Instance().SetAutoPositionSlotIndex(iSlotIndex, iIndex);
	return Py_BuildNone();
}

PyObject* playerGetAutoSlotIndex(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BadArgument();

	uint32_t dwVnum, fillingTime;
	CPythonPlayer::Instance().GetAutoSlotIndex(iIndex, &dwVnum, &fillingTime);
	return Py_BuildValue("i", dwVnum);
}

PyObject* playerSetAutoSlotCoolTime(PyObject* poSelf, PyObject* poArgs)
{
	int ÌIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &ÌIndex))
		return Py_BadArgument();

	int iCoolTime;
	if (!PyTuple_GetInteger(poArgs, 1, &iCoolTime))
		return Py_BadArgument();

	CPythonPlayer::Instance().SetAutoSlotCoolTime(ÌIndex, iCoolTime);
	return Py_BuildNone();
}

PyObject* playerGetAutoSlotCoolTime(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BadArgument();

	//return Py_BuildValue("i", CPythonPlayer::Instance().GetAutoSlotCoolTime(bIndex));
	uint32_t dwVnum, fillingTime;
	CPythonPlayer::Instance().GetAutoSlotIndex(iIndex, &dwVnum, &fillingTime);	//GetAutoSlotCoolTime
	return Py_BuildValue("i", fillingTime);
}

PyObject* playerCheckSkillSlotCoolTime(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bIndex;
	if (!PyTuple_GetByte(poArgs, 0, &bIndex))
		return Py_BadArgument();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BadArgument();

	int iCoolTime;
	if (!PyTuple_GetInteger(poArgs, 2, &iCoolTime))
		return Py_BadArgument();

	return Py_BuildValue("i", CPythonPlayer::Instance().CheckSkillSlotCoolTime(bIndex, iSlotIndex, iCoolTime));
}

PyObject* playerCheckPositionSlotCoolTime(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bIndex;
	if (!PyTuple_GetByte(poArgs, 0, &bIndex))
		return Py_BadArgument();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BadArgument();

	int iCoolTime;
	if (!PyTuple_GetInteger(poArgs, 2, &iCoolTime))
		return Py_BadArgument();

	return Py_BuildValue("i", CPythonPlayer::Instance().CheckPositionSlotCoolTime(bIndex, iSlotIndex, iCoolTime));
}

PyObject* playerClearAutoSKillSlot(PyObject* poSelf, PyObject* poArgs)
{
	CPythonPlayer::Instance().ClearAutoSKillSlot();
	return Py_BuildNone();
}

PyObject* playerClearAutoPositionSlot(PyObject* poSelf, PyObject* poArgs)
{
	CPythonPlayer::Instance().ClearAutoPositionSlot();
	return Py_BuildNone();
}

PyObject* playerClearAutoAllSlot(PyObject* poSelf, PyObject* poArgs)
{
	CPythonPlayer::Instance().ClearAutoAllSlot();
	return Py_BuildNone();
}

PyObject* playerCanStartAuto(PyObject* poSelf, PyObject* poArgs)
{
	CInstanceBase* pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
	if (pMainInstance)
	{
		CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
		return Py_BuildValue("i", rkPlayer.CanStartAuto());
	}

	return Py_BuildValue("i", 0);
}

/*~| New Modules |~*/
PyObject* playerAutoStartOnOff(PyObject* poSelf, PyObject* poArgs)
{
	bool onoff;
	if (!PyTuple_GetBoolean(poArgs, 0, &onoff))
		return Py_BuildValue("i", 0);

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetSelectedInstancePtr();
	CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
	if (pInstance)
	{
		if (rkPlayer.AutoStatus() != onoff)
			rkPlayer.AutoStartOnOff(onoff);
	}

	return Py_BuildNone();
}

PyObject* playerAutoAttackOnOff(PyObject* poSelf, PyObject* poArgs)
{
	int onoff;
	if (!PyTuple_GetInteger(poArgs, 0, &onoff))
		return Py_BuildException();

	CPythonPlayer::Instance().AutoAttackOnOff(onoff);
	return Py_BuildNone();
}

PyObject* playerAutoSkillOnOff(PyObject* poSelf, PyObject* poArgs)
{
	int onoff;
	if (!PyTuple_GetInteger(poArgs, 0, &onoff))
		return Py_BuildException();

	CPythonPlayer::Instance().AutoSkillOnOff(onoff);
	return Py_BuildNone();
}

PyObject* playerAutoPositionOnOff(PyObject* poSelf, PyObject* poArgs)
{
	int onoff;
	if (!PyTuple_GetInteger(poArgs, 0, &onoff))
		return Py_BuildException();

	CPythonPlayer::Instance().AutoPositionOnOff(onoff);
	return Py_BuildNone();
}

PyObject* playerAutoRangeOnOff(PyObject* poSelf, PyObject* poArgs)
{
	int onoff;
	if (!PyTuple_GetInteger(poArgs, 0, &onoff))
		return Py_BuildException();

	CPythonPlayer::Instance().AutoRangeOnOff(onoff);
	return Py_BuildNone();
}

PyObject* playerSetAutoRestart(PyObject* poSelf, PyObject* poArgs)
{
	bool state;
	if (!PyTuple_GetBoolean(poArgs, 0, &state))
		return Py_BuildException();

	CPythonPlayer::Instance().SetAutoRestart(state);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_MONSTER_BACK
PyObject* playerGetAttendanceRewardList(PyObject* poSelf, PyObject* poArgs)
{
	std::vector<TRewardItem> m_rewardItems = CPythonPlayer::Instance().GetRewardVec();

	PyObject* dict = PyDict_New();

	if (!m_rewardItems.empty())
	{
		for (uint32_t i = 0; i < m_rewardItems.size(); i++)
		{
			PyDict_SetItem(dict, Py_BuildValue("i", m_rewardItems[i].dwDay), Py_BuildValue("ii", m_rewardItems[i].dwVnum, m_rewardItems[i].dwCount));
		}
	}

	return dict;
}
#endif

#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
PyObject* playerGetAccumulateDamageByVID(PyObject* poSelf, PyObject* poArgs)
{
	int iVid;
	if (!PyTuple_GetInteger(poArgs, 0, &iVid))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonPlayer::Instance().GetAccumulateDamage(iVid));
}
#endif

#ifdef ENABLE_KEYCHANGE_SYSTEM
PyObject* playerIsOpenKeySettingWindow(PyObject* poSelf, PyObject* poArgs)
{
	BOOL bSet = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &bSet))
		return Py_BadArgument();

	CPythonPlayer::Instance().IsOpenKeySettingWindow(bSet);
	return Py_BuildNone();
}

PyObject* playerKeySettingClear(PyObject* poSelf, PyObject* poArgs)
{
	CPythonPlayer::Instance().KeySettingClear();
	return Py_BuildNone();
}

PyObject* playerKeySetting(PyObject* poSelf, PyObject* poArgs)
{
	int iKey = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &iKey))
		return Py_BadArgument();

	int iKeyFunction = 0;
	if (!PyTuple_GetInteger(poArgs, 1, &iKeyFunction))
		return Py_BadArgument();

	CPythonPlayer::Instance().KeySetting(iKey, iKeyFunction);
	return Py_BuildNone();
}

PyObject* playerOnKeyDown(PyObject* poSelf, PyObject* poArgs)
{
	int iKey = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &iKey))
		return Py_BadArgument();

	CPythonPlayer::Instance().OnKeyDown(iKey);
	return Py_BuildNone();
}

PyObject* playerOnKeyUp(PyObject* poSelf, PyObject* poArgs)
{
	int iKey = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &iKey))
		return Py_BadArgument();

	CPythonPlayer::Instance().OnKeyUp(iKey);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_PARTY_MATCH
PyObject* playerGetPartyMatchInfoMap(PyObject* poSelf, PyObject* poArgs)
{
	const auto& PartyMatchInfo = CPythonNetworkStream::Instance().GetPartyMatchInfo();

	PyObject* items, * dict = PyDict_New();

	for (const auto& i : PartyMatchInfo)
	{
		const auto& ItemVector = i.second->items;
		items = PyTuple_New(ItemVector.size());

		for (size_t j = 0; j < ItemVector.size(); j++)
			PyTuple_SetItem(items, j, Py_BuildValue("ii", ItemVector.at(j).first, ItemVector.at(j).second));

		PyDict_SetItem(dict, PyInt_FromLong(i.first), Py_BuildValue("iiO", i.first, i.second->limit_level, items));
	}

	return dict;
}

PyObject* playerIsPartyMatchLoaded(PyObject* poSelf, PyObject* poArgs)
{
	const auto& PartyMatchInfo = CPythonNetworkStream::Instance().GetPartyMatchInfo();
	return Py_BuildValue("i", !PartyMatchInfo.empty());
}

PyObject* playerIsPartyMatchEnoughItem(PyObject* poSelf, PyObject* poArgs)	//missing
{
	int iMapIndex = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &iMapIndex))
		return Py_BadArgument();

	return Py_BuildNone();
}
#endif

#ifdef ENABLE_SECOND_GUILDRENEWAL_SYSTEM
PyObject* playerSetParalysis(PyObject* poSelf, PyObject* poArgs)
{
	BOOL bState = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &bState))
	return Py_BadArgument();

	CPythonPlayer::Instance().SetParalysis(bState);
	return Py_BuildNone();
}
#endif

PyObject* playerGetLevel(PyObject* poSelf, PyObject* poArgs)
{
	uint32_t dwLevel = CPythonPlayer::Instance().GetStatus(POINT_LEVEL);
	return Py_BuildValue("l", dwLevel);
}

#ifdef ENABLE_MINI_GAME_YUTNORI
PyObject* playerYutnoriShow(PyObject* poSelf, PyObject* poArgs)
{
	bool isShow = false;
	if (!PyTuple_GetBoolean(poArgs, 0, &isShow))
		return Py_BadArgument();

	CPythonYutnoriManager::Instance().SetShow(isShow);
	return Py_BuildNone();
}

PyObject* playerYutnoriChangeMotion(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bMotion;
	if (!PyTuple_GetByte(poArgs, 0, &bMotion))
		return Py_BuildException();

	CPythonYutnoriManager::Instance().ChangeMotion(bMotion);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_FLOWER_EVENT
PyObject* playerGetFlower(PyObject* poSelf, PyObject* poArgs)
{
	int id = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BadArgument();

	for (int i = POINT_FLOWER_TYPE_1; i < POINT_FLOWER_TYPE_6 + 1; ++i)
	{
		if (id == (i - POINT_FLOWER_TYPE_1))
		{
			return Py_BuildValue("i", CPythonPlayer::Instance().GetStatus(i));
		}
	}

	return Py_BuildNone();
}
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
PyObject* playerGetPremiumBattlePassID(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("b", CPythonPlayer::Instance().GetStatus(POINT_BATTLE_PASS_PREMIUM_ID));
}
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
PyObject* playerGetPageEquipment(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetStatus(POINT_NEW_EQUIPMENT_ACTUAL));
}

PyObject* playerGetPageEquipmentTotal(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetStatus(POINT_NEW_EQUIPMENT_TOTAL));
}
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
PyObject* playerGetLottoMoney(PyObject* poSelf, PyObject* poArgs)
{
	return PyLong_FromLongLong(CPythonPlayer::Instance().GetStatus(POINT_LOTTO_MONEY));
	//return Py_BuildValue("b", CPythonPlayer::Instance().GetStatus(POINT_LOTTO_MONEY));
}

PyObject* playerGetLottoTotalMoney(PyObject* poSelf, PyObject* poArgs)
{
	return PyLong_FromLongLong(CPythonPlayer::Instance().GetStatus(POINT_LOTTO_TOTAL_MONEY));
	//return Py_BuildValue("b", CPythonPlayer::Instance().GetStatus(POINT_LOTTO_TOTAL_MONEY));
}
#endif

void initPlayer()
{
	static PyMethodDef s_methods[] = {
		{"GetAutoPotionInfo", playerGetAutoPotionInfo, METH_VARARGS},
		{"SetAutoPotionInfo", playerSetAutoPotionInfo, METH_VARARGS},

		{"PickCloseItem", playerPickCloseItem, METH_VARARGS},
		{"SetGameWindow", playerSetGameWindow, METH_VARARGS},
		{"RegisterEffect", playerRegisterEffect, METH_VARARGS},
		{"RegisterCacheEffect", playerRegisterCacheEffect, METH_VARARGS},
		{"SetMouseState", playerSetMouseState, METH_VARARGS},
		{"SetMouseFunc", playerSetMouseFunc, METH_VARARGS},
		{"GetMouseFunc", playerGetMouseFunc, METH_VARARGS},
		{"SetMouseMiddleButtonState", playerSetMouseMiddleButtonState, METH_VARARGS},
		{"SetMainCharacterIndex", playerSetMainCharacterIndex, METH_VARARGS},
		{"GetMainCharacterIndex", playerGetMainCharacterIndex, METH_VARARGS},
		{"GetMainCharacterName", playerGetMainCharacterName, METH_VARARGS},
		{"GetMainCharacterPosition", playerGetMainCharacterPosition, METH_VARARGS},
		{"IsMainCharacterIndex", playerIsMainCharacterIndex, METH_VARARGS},
#ifdef ENABLE_SPECIAL_INVENTORY
		{ "GetSpecialInventoryTypeByGlobalSlot", playerGetSpecialInventoryTypeByGlobalSlot, METH_VARARGS },
		{ "GetSpecialInventoryRange", playerGetSpecialInventoryRange, METH_VARARGS },
#endif
		{"CanAttackInstance", playerCanAttackInstance, METH_VARARGS},
		{"IsActingEmotion", playerIsActingEmotion, METH_VARARGS},
		{"IsPVPInstance", playerIsPVPInstance, METH_VARARGS},
		{"IsSameEmpire", playerIsSameEmpire, METH_VARARGS},
		{"IsChallengeInstance", playerIsChallengeInstance, METH_VARARGS},
		{"IsRevengeInstance", playerIsRevengeInstance, METH_VARARGS},
		{"IsCantFightInstance", playerIsCantFightInstance, METH_VARARGS},
		{"GetCharacterDistance", playerGetCharacterDistance, METH_VARARGS},
		{"IsInSafeArea", playerIsInSafeArea, METH_VARARGS},
		{"IsMountingHorse", playerIsMountingHorse, METH_VARARGS},
		{"IsObserverMode", playerIsObserverMode, METH_VARARGS},
		{"ActEmotion", playerActEmotion, METH_VARARGS},

		{"ShowPlayer", playerShowPlayer, METH_VARARGS},
		{"HidePlayer", playerHidePlayer, METH_VARARGS},

		{"ComboAttack", playerComboAttack, METH_VARARGS},
		{ "IsAttacking", playerIsAttacking, METH_VARARGS },

		{"SetAutoCameraRotationSpeed", playerSetAutoCameraRotationSpeed, METH_VARARGS},
		{"SetAttackKeyState", playerSetAttackKeyState, METH_VARARGS},
		{"SetSingleDIKKeyState", playerSetSingleDIKKeyState, METH_VARARGS},
		{"EndKeyWalkingImmediately", playerEndKeyWalkingImmediately, METH_VARARGS},
		{"StartMouseWalking", playerStartMouseWalking, METH_VARARGS},
		{"EndMouseWalking", playerEndMouseWalking, METH_VARARGS},
		{"ResetCameraRotation", playerResetCameraRotation, METH_VARARGS},
		{"SetQuickCameraMode", playerSetQuickCameraMode, METH_VARARGS},

		///////////////////////////////////////////////////////////////////////////////////////////

		{"SetSkill", playerSetSkill, METH_VARARGS},
		{"GetSkillIndex", playerGetSkillIndex, METH_VARARGS},
		{"GetSkillSlotIndex", playerGetSkillSlotIndex, METH_VARARGS},
		{"GetSkillGrade", playerGetSkillGrade, METH_VARARGS},
		{"GetSkillLevel", playerGetSkillLevel, METH_VARARGS},
		{"GetSkillCurrentEfficientPercentage", playerGetSkillCurrentEfficientPercentage, METH_VARARGS},
		{"GetSkillNextEfficientPercentage", playerGetSkillNextEfficientPercentage, METH_VARARGS},
		{"ClickSkillSlot", playerClickSkillSlot, METH_VARARGS},
		{"ChangeCurrentSkillNumberOnly", playerChangeCurrentSkillNumberOnly, METH_VARARGS},
		{"ClearSkillDict", playerClearSkillDict, METH_VARARGS},

		{"GetItemIndex", playerGetItemIndex, METH_VARARGS},
		{"GetItemFlags", playerGetItemFlags, METH_VARARGS},
		{"GetItemCount", playerGetItemCount, METH_VARARGS},
		{"GetItemCountByVnum", playerGetItemCountByVnum, METH_VARARGS},
		{"GetItemMetinSocket", playerGetItemMetinSocket, METH_VARARGS},
		{"GetItemAttribute", playerGetItemAttribute, METH_VARARGS},
#ifdef ENABLE_YOHARA_SYSTEM
		{ "GetItemApplyRandom", playerGetItemApplyRandom, METH_VARARGS },
		{ "GetRandomValue", playerGetRandomValue, METH_VARARGS },
		{ "GetConquerorEXP", playerGetConquerorEXP, METH_VARARGS },
#endif
		{"GetISellItemPrice", playerGetISellItemPrice, METH_VARARGS},
		{"MoveItem", playerMoveItem, METH_VARARGS},
		{"SendClickItemPacket", playerSendClickItemPacket, METH_VARARGS},

		///////////////////////////////////////////////////////////////////////////////////////////

		{"GetName", playerGetName, METH_VARARGS},
		{"GetJob", playerGetJob, METH_VARARGS},
		{"GetRace", playerGetRace, METH_VARARGS},
		{"GetPlayTime", playerGetPlayTime, METH_VARARGS},
		{"SetPlayTime", playerSetPlayTime, METH_VARARGS},

		{"IsSkillCoolTime", playerIsSkillCoolTime, METH_VARARGS},
		{"GetSkillCoolTime", playerGetSkillCoolTime, METH_VARARGS},
		{"IsSkillActive", playerIsSkillActive, METH_VARARGS},
		{"UseGuildSkill", playerUseGuildSkill, METH_VARARGS},
		{"AffectIndexToSkillIndex", playerAffectIndexToSkillIndex, METH_VARARGS},
		{"GetEXP", playerGetEXP, METH_VARARGS},
		{"GetStatus", playerGetStatus, METH_VARARGS},
		{"SetStatus", playerSetStatus, METH_VARARGS},
		{"GetElk", playerGetElk, METH_VARARGS},
#ifdef ENABLE_CHEQUE_SYSTEM
		{ "GetCheque",					playerGetCheque,					METH_VARARGS },
#endif
#ifdef ENABLE_GEM_SYSTEM
		{ "GetGem",						playerGetGem,						METH_VARARGS },
#	if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
		{"GetGemShopItemID", playerGetGemShopItemID, METH_VARARGS},
		{"GetGemShopOpenSlotCount", playerGetGemShopOpenSlotCount, METH_VARARGS},
		{"GetGemShopOpenSlotItemCount", playerGetGemShopOpenSlotItemCount, METH_VARARGS},
		{"GetGemShopRefreshTime", playerGetGemShopRefreshTime, METH_VARARGS},

		{"IsGemShopWindowOpen", playerIsGemShopWindowOpen, METH_VARARGS},
		{"SetGemShopWindowOpen", playerSetGemShopWindowOpen, METH_VARARGS},
#	endif
#endif
		{"GetMoney", playerGetElk, METH_VARARGS},
		{"GetGuildID", playerGetGuildID, METH_VARARGS},
		{"GetGuildName", playerGetGuildName, METH_VARARGS},
		{"GetAlignmentData", playerGetAlignmentData, METH_VARARGS},
		{"RequestAddLocalQuickSlot", playerRequestAddLocalQuickSlot, METH_VARARGS},
		{"RequestAddToEmptyLocalQuickSlot", playerRequestAddToEmptyLocalQuickSlot, METH_VARARGS},
		{"RequestDeleteGlobalQuickSlot", playerRequestDeleteGlobalQuickSlot, METH_VARARGS},
		{"RequestMoveGlobalQuickSlotToLocalQuickSlot", playerRequestMoveGlobalQuickSlotToLocalQuickSlot, METH_VARARGS},
		{"RequestUseLocalQuickSlot", playerRequestUseLocalQuickSlot, METH_VARARGS},
		{"LocalQuickSlotIndexToGlobalQuickSlotIndex", playerLocalQuickSlotIndexToGlobalQuickSlotIndex, METH_VARARGS},

		{"GetQuickPage", playerGetQuickPage, METH_VARARGS},
		{"SetQuickPage", playerSetQuickPage, METH_VARARGS},
		{"GetLocalQuickSlot", playerGetLocalQuickSlot, METH_VARARGS},
		{"GetGlobalQuickSlot", playerGetGlobalQuickSlot, METH_VARARGS},
		{"RemoveQuickSlotByValue", playerRemoveQuickSlotByValue, METH_VARARGS},

		{"isItem", playerisItem, METH_VARARGS},
		{"IsEquipmentSlot", playerIsEquipmentSlot, METH_VARARGS},
		{"IsDSEquipmentSlot", playerIsDSEquipmentSlot, METH_VARARGS},
		{"IsCostumeSlot", playerIsCostumeSlot, METH_VARARGS},
		{"IsValuableItem", playerIsValuableItem, METH_VARARGS},
		{"IsOpenPrivateShop", playerIsOpenPrivateShop, METH_VARARGS},

		//ENABLE_NEW_EQUIPMENT_SYSTEM
		{"IsBeltInventorySlot", playerIsBeltInventorySlot, METH_VARARGS},
		{"IsEquippingBelt", playerIsEquippingBelt, METH_VARARGS},
		{"IsAvailableBeltInventoryCell", playerIsAvailableBeltInventoryCell, METH_VARARGS},
		//END_ENABLE_NEW_EQUIPMENT_SYSTEM

		// Refine
		{"GetItemGrade", playerGetItemGrade, METH_VARARGS},
		{"CanRefine", playerCanRefine, METH_VARARGS},
		{"CanDetach", playerCanDetach, METH_VARARGS},
		{"CanUnlock", playerCanUnlock, METH_VARARGS},
		{"CanAttachMetin", playerCanAttachMetin, METH_VARARGS},
		{"IsRefineGradeScroll", playerIsRefineGradeScroll, METH_VARARGS},

		{"ClearTarget", playerClearTarget, METH_VARARGS},
		{"SetTarget", playerSetTarget, METH_VARARGS},
		{"OpenCharacterMenu", playerOpenCharacterMenu, METH_VARARGS},

		{"Update", playerUpdate, METH_VARARGS},
		{"Render", playerRender, METH_VARARGS},
		{"Clear", playerClear, METH_VARARGS},

		// Party
		{"IsPartyMember", playerIsPartyMember, METH_VARARGS},
		{"IsPartyLeader", playerIsPartyLeader, METH_VARARGS},
		{"IsPartyLeaderByPID", playerIsPartyLeaderByPID, METH_VARARGS},
		{"GetPartyMemberHPPercentage", playerGetPartyMemberHPPercentage, METH_VARARGS},
		{"GetPartyMemberState", playerGetPartyMemberState, METH_VARARGS},
		{"GetPartyMemberAffects", playerGetPartyMemberAffects, METH_VARARGS},
		{"RemovePartyMember", playerRemovePartyMember, METH_VARARGS},
		{"ExitParty", playerExitParty, METH_VARARGS},

		// PK Mode
		{"GetPKMode", playerGetPKMode, METH_VARARGS},

		// Mobile
		{"HasMobilePhoneNumber", playerHasMobilePhoneNumber, METH_VARARGS},

		// Emotion
		{"RegisterEmotionIcon", playerRegisterEmotionIcon, METH_VARARGS},
		{"GetEmotionIconImage", playerGetEmotionIconImage, METH_VARARGS },

		// For System
		{"SetWeaponAttackBonusFlag", playerSetWeaponAttackBonusFlag, METH_VARARGS},
		{"ToggleCoolTime", playerToggleCoolTime, METH_VARARGS},
		{"ToggleLevelLimit", playerToggleLevelLimit, METH_VARARGS},
		{"GetTargetVID", playerGetTargetVID, METH_VARARGS},
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
		{ "GetRouletteData",			playerGetRouletteData,				METH_VARARGS },
		{ "GetEventSoul",				playerGetSoul,						METH_VARARGS },
#endif

		{"SetItemData", playerSetItemData, METH_VARARGS},
		{"SetItemMetinSocket", playerSetItemMetinSocket, METH_VARARGS},
		{"SetItemAttribute", playerSetItemAttribute, METH_VARARGS},
		{"SetItemCount", playerSetItemCount, METH_VARARGS},

		{"GetItemLink", playerGetItemLink, METH_VARARGS},
		{"SlotTypeToInvenType", playerSlotTypeToInvenType, METH_VARARGS},
		{"SendDragonSoulRefine", playerSendDragonSoulRefine, METH_VARARGS},

#ifdef ENABLE_SOULBIND_SYSTEM
		// Item Sealing
		{ "GetItemSealDate", playerGetItemSealDate, METH_VARARGS },
		{ "GetItemUnSealLeftTime", playerGetItemUnSealLeftTime, METH_VARARGS },
		{ "CanSealItem", playerCanSealItem, METH_VARARGS },
#endif

		{ "IsAntiFlagBySlot", playerIsAntiFlagBySlot, METH_VARARGS },
		{ "GetItemTypeBySlot", playerGetItemTypeBySlot, METH_VARARGS },
		{ "GetItemSubTypeBySlot", playerGetItemSubTypeBySlot, METH_VARARGS },
		{ "IsSameItemVnum", playerIsSameItemVnum, METH_VARARGS },
		{ "ExitParty", playerExitParty, METH_VARARGS },
		{ "PartyMemberVIDToPID", playerPartyMemberVIDToPID, METH_VARARGS },	//@fixme402

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		{ "GetChangeLookVnum", playerGetChangeLookVnum, METH_VARARGS },
#	ifdef ENABLE_ACCE_COSTUME_SYSTEM
		{ "GetAcceWindowChangeLookVnum", playerGetAcceWindowChangeLookVnum, METH_VARARGS },
#	endif
#	ifdef ENABLE_REFINE_ELEMENT
		{ "GetChangeLookElement", playerGetChangeLookElement, METH_VARARGS },
#	endif

		{ "SetChangeLookWindow", playerSetChangeLookWindow, METH_VARARGS },
		{ "GetChangeLookWindowOpen", playerGetChangeLookWindowOpen, METH_VARARGS },

		{ "SetChangeLookWindowType", playerSetChangeLookWindowType, METH_VARARGS },

		{ "GetChangeLookItemID", playerGetChangeLookItemID, METH_VARARGS },
		{ "GetChangeLookItemInvenSlot", playerGetChangeLookItemInvenSlot, METH_VARARGS },

		{ "GetChangeLookFreeYangItemID", playerGetChangeLookFreeYangItemID, METH_VARARGS },
		{ "GetChangeLookFreeYangInvenSlotPos", playerGetChangeLookFreeYangInvenSlotPos, METH_VARARGS },

		{ "CanChangeLookClearItem", playerCanChangeLookClearItem, METH_VARARGS },
		{ "GetChangeWIndowChangeLookVnum", playerGetChangeWIndowChangeLookVnum, METH_VARARGS },
		{ "GetChangeChangeLookPrice", playerGetChangeChangeLookPrice, METH_VARARGS },
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
		{ "GetPetItem", playerGetPetItem, METH_VARARGS },
		{ "GetActivePetItemId", playerGetActivePetItemId, METH_VARARGS },
		{ "GetActivePetItemVNum", playerGetActivePetItemVNum, METH_VARARGS },
		{ "GetPetExpPoints", playerGetPetExpPoints, METH_VARARGS },
		{ "GetPetItemVNumInBag", playerGetPetItemVNumInBag, METH_VARARGS },
		{ "GetPetSkill", playerGetPetSkill, METH_VARARGS },
		{ "GetPetLifeTime", playerGetPetLifeTime, METH_VARARGS },
		{ "GetPetSkillByIndex", playerGetPetSkillByIndex, METH_VARARGS },
		{ "CanUsePetCoolTimeCheck", playerCanUsePetCoolTimeCheck, METH_VARARGS },
		{ "SetOpenPetHatchingWindow", playerSetOpenPetHatchingWindow, METH_VARARGS },
		{ "IsOpenPetHatchingWindow", playerIsOpenPetHatchingWindow, METH_VARARGS },
		{ "SetOpenPetFeedWindow", playerSetOpenPetFeedWindow, METH_VARARGS },
		{ "IsOpenPetFeedWindow", playerIsOpenPetFeedWindow, METH_VARARGS },
		{ "CanUseGrowthPetQuickSlot", playerCanUseGrowthPetQuickSlot, METH_VARARGS },
		{ "SetOpenPetNameChangeWindow", playerSetOpenPetNameChangeWindow, METH_VARARGS },
#	ifdef ENABLE_PET_ATTR_DETERMINE
		{ "CanAttachToPetAttrChangeSlot", playerCanAttachToPetAttrChangeSlot, METH_VARARGS },
		{ "GetInvenSlotAttachedToPetAttrChangeWindowSlot", playerGetInvenSlotAttachedToPetAttrChangeWindowSlot, METH_VARARGS },
		{ "GetPetAttrChangeWindowSlotByAttachedInvenSlot", playerGetPetAttrChangeWindowSlotByAttachedInvenSlot, METH_VARARGS },
		{ "SetItemPetAttrChangeWindowActivedItemSlot", playerSetItemPetAttrChangeWindowActivedItemSlot, METH_VARARGS },
#	endif
#endif

		{ "WindowTypeToSlotType",		playerWindowTypeToSlotType,			METH_VARARGS },

#ifdef ENABLE_GIVE_BASIC_ITEM
		{ "IsBasicItem",	playerIsBasicItem,	METH_VARARGS },
#endif
#ifdef ENABLE_SET_ITEM
		{ "GetItemSetValue",	playerGetItemSetValue,	METH_VARARGS },
		{ "GetSetItemEffect",	playerGetSetItemEffect,	METH_VARARGS },
		{ "CanSetItemClearItem",	playerCanSetItemClearItem,	METH_VARARGS },
#endif
		{ "IsGameMaster", playerIsGameMaster, METH_VARARGS },

#ifdef ENABLE_LOADING_TIP
		{ "GetLoadingTip", playerGetLoadingTip, METH_VARARGS },
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
		{ "SetSkillColor",					playerSetSkillColor,				METH_VARARGS },
		{ "GetSkillColor",					playerGetSkillColor,				METH_VARARGS },
#endif

#ifdef ENABLE_ELEMENT_ADD
		{"GetElementByVID", playerGetElementByVID, METH_VARARGS},
#endif

#ifdef ENABLE_MEDAL_OF_HONOR
		{ "GetMedalOfHonor", playerGetMedalOfHonor, METH_VARARGS },
#endif

#ifdef ENABLE_BATTLE_FIELD
		{ "GetBattlePoint", playerGetBattlePoint, METH_VARARGS },

		{ "IsBattleButtonFlush", playerIsBattleButtonFlush, METH_VARARGS },
		{ "SetBattleButtonFlush", playerSetBattleButtonFlush, METH_VARARGS },

		{ "IsBattleFieldOpen", playerIsBattleFieldOpen, METH_VARARGS },
		{ "SetBattleFieldOpen", playerSetBattleFieldOpen, METH_VARARGS },

		{ "IsBattleFieldEventOpen", playerIsBattleFieldEventOpen, METH_VARARGS },
		{ "SetBattleFieldEventOpen", playerSetBattleFieldEventOpen, METH_VARARGS },

		{ "SetBattleFieldInfo", playerSetBattleFieldInfo, METH_VARARGS },
		{ "SetBattleFieldEventInfo", playerSetBattleFieldEventInfo, METH_VARARGS },

		{ "GetBattleFieldEnable", playerGetBattleFieldEnable, METH_VARARGS },
		{ "GetBattleFieldEventEnable", playerGetBattleFieldEventEnable, METH_VARARGS },
#endif

#ifdef ENABLE_REFINE_ELEMENT
		{ "GetItemElementGrade",				playerGetItemElementGrade,					METH_VARARGS },
		{ "GetItemElementAttack",				playerGetItemElementAttack,					METH_VARARGS },
		{ "GetItemElementType",					playerGetItemElementType,					METH_VARARGS },
		{ "GetItemElementValue",				playerGetItemElementValue,						METH_VARARGS },
		{ "GetElements", 						playerElements,								METH_VARARGS },
#endif

#ifdef ENABLE_RENDER_TARGET
		{ "GetEquippedWeapon", playerGetEquippedWeapon, METH_VARARGS },
		{ "GetEquippedArmor", playerGetEquippedArmor, METH_VARARGS },
		{ "GetEquippedHair", playerGetEquippedHair, METH_VARARGS },
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		{ "GetEquippedAcce", playerGetEquippedAcce, METH_VARARGS },
#endif
#ifdef ENABLE_AURA_SYSTEM
		{ "GetEquippedAura", playerGetEquippedAura, METH_VARARGS },
#endif
#ifdef ENABLE_MYSHOP_DECO
		{ "SelectShopModel", playerSelectShopModel, METH_VARARGS },
		{ "MyShopDecoShow", playerMyShopDecoShow, METH_VARARGS },
#endif
#endif

#ifdef ENABLE_OFFICAL_FEATURES
		{ "IsPoly", playerIsPoly, METH_VARARGS },
		{ "IsOpenSafeBox", playerIsOpenSafeBox, METH_VARARGS },
		{ "SetOpenSafeBox", playerSetOpenSafeBox, METH_VARARGS },
		{ "IsOpenMall", playerIsOpenMall, METH_VARARGS },
		{ "SetOpenMall", playerSetOpenMall, METH_VARARGS },
#endif

#ifdef ENABLE_EXTEND_INVEN_SYSTEM
		{ "GetExtendInvenStage", playerGetExtendInvenStage, METH_VARARGS },
		{ "GetExtendInvenMax", playerGetExtendInvenMax, METH_VARARGS },

# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
		{ "GetExtendSpecialInvenStage", playerGetExtendSpecialInvenStage, METH_VARARGS },
		{ "GetExtendSpecialInvenMax", playerGetExtendSpecialInvenMax, METH_VARARGS },
# endif
#endif

#ifdef ENABLE_MOVE_COSTUME_ATTR
		{ "GetInvenSlotAttachedToConbWindowSlot", playerGetInvenSlotAttachedToConbWindowSlot, METH_VARARGS },
		{ "GetConbWindowSlotByAttachedInvenSlot", playerGetConbWindowSlotByAttachedInvenSlot, METH_VARARGS },
		{ "SetItemCombinationWindowActivedItemSlot", playerSetItemCombinationWindowActivedItemSlot, METH_VARARGS },
		{ "CanAttachToCombMediumSlot", playerCanAttachToCombMediumSlot, METH_VARARGS },
		{ "CanAttachToCombItemSlot", playerCanAttachToCombItemSlot, METH_VARARGS },
#endif

#ifdef ENABLE_ACCE_SECOND_COSTUME_SYSTEM
		{ "SetAcceRefineWindowOpen", playerSetAcceRefineWindowOpen, METH_VARARGS },
		{ "GetAcceRefineWindowType", playerGetAcceRefineWindowType, METH_VARARGS },
		{ "SetAcceActivedItemSlot", playerSetAcceActivedItemSlot, METH_VARARGS },
		{ "FindActivedAcceSlot", playerFindActivedAcceSlot, METH_VARARGS },
		{ "GetAcceItemMetinSocket", playerGetAcceItemMetinSocket, METH_VARARGS },
		{ "GetAcceItemID", playerGetAcceItemID, METH_VARARGS },
		{ "GetAcceItemSize", playerGetAcceItemSize, METH_VARARGS },
		{ "GetAcceItemAttribute", playerGetAcceItemAttribute, METH_VARARGS },
		{ "GetAcceItemFlags", playerGetAcceItemFlags, METH_VARARGS },
		{ "IsAcceWindowEmpty", playerIsAcceWindowEmpty, METH_VARARGS },
		{ "FindUsingAcceSlot", playerFindUsingAcceSlot, METH_VARARGS },
		{ "GetAcceRefineWindowOpen", playerGetAcceRefineWindowOpen, METH_VARARGS },
		{ "FineMoveAcceItemSlot", playerFineMoveAcceItemSlot, METH_VARARGS },
		{ "GetCurrentItemCount", playerGetCurrentItemCount, METH_VARARGS },
		{ "CanAcceClearItem", playerCanAcceClearItem, METH_VARARGS },
#	ifdef ENABLE_YOHARA_SYSTEM
		{ "GetAcceRandomItemAttribute", playerGetAcceRandomItemAttribute, METH_VARARGS },
		{ "GetAcceItemRandomValue", playerGetAcceItemRandomValue, METH_VARARGS },
#	endif
#	ifdef ENABLE_SET_ITEM
		{ "GetAcceSetValue",						playerGetAcceSetValue,					METH_VARARGS },
#	endif
#endif

#ifdef ENABLE_AURA_SYSTEM
		{ "GetAuraItemID",							playerGetAuraItemID,						METH_VARARGS },
		{ "GetAuraItemCount",						playerGetAuraItemCount,						METH_VARARGS },
		{ "GetAuraItemMetinSocket",					playerGetAuraItemMetinSocket,				METH_VARARGS },
		{ "GetAuraItemAttribute",					playerGetAuraItemAttribute,					METH_VARARGS },

		{ "IsAuraRefineWindowEmpty",				playerIsAuraRefineWindowEmpty,				METH_VARARGS },
		{ "GetAuraCurrentItemSlotCount",			playerGetAuraCurrentItemSlotCount,			METH_VARARGS },
		{ "IsAuraRefineWindowOpen",					playerIsAuraRefineWindowOpen,				METH_VARARGS },
		{ "GetAuraRefineWindowType",				playerGetAuraRefineWindowType,				METH_VARARGS },
		{ "FineMoveAuraItemSlot",					playerFineMoveAuraItemSlot,					METH_VARARGS },
		{ "SetAuraActivatedItemSlot",				playerSetAuraActivatedItemSlot,				METH_VARARGS },
		{ "FindActivatedAuraSlot",					playerFindActivatedAuraSlot,				METH_VARARGS },
		{ "FindUsingAuraSlot",						playerFindUsingAuraSlot,					METH_VARARGS },

		{ "GetAuraRefineInfo",						playerGetAuraRefineInfo,					METH_VARARGS },
		{ "GetAuraRefineInfoExpPer",				playerGetAuraRefineInfoExpPer,				METH_VARARGS },
		{ "GetAuraRefineInfoLevel",					playerGetAuraRefineInfoLevel,				METH_VARARGS },
#	ifdef ENABLE_YOHARA_SYSTEM
		{ "GetAuraSlotItemApplyRandom",				playerGetAuraSlotItemApplyRandom,			METH_VARARGS },
		{ "GetAuraRandomValue",						playerGetAuraRandomValue,					METH_VARARGS },
#	endif
#	ifdef ENABLE_SET_ITEM
		{ "GetAuraSetValue",						playerGetAuraSetValue,						METH_VARARGS },
#	endif
#endif

		{ "NPOS",									playerNPOS,									METH_VARARGS },

#ifdef ENABLE_CHANGED_ATTR
		{ "GetItemChangedAttribute",				playerGetItemChangedAttribute,				METH_VARARGS },
#endif

#ifdef ENABLE_CUBE_RENEWAL
		{ "IsCubeRenewalOpen",						playerIsCubeRenewalOpen,					METH_VARARGS },
#endif

#ifdef ENABLE_AUTO_SYSTEM
		{ "SetAutoSkillSlotIndex", playerSetAutoSkillSlotIndex, METH_VARARGS },
		{ "SetAutoPositionSlotIndex", playerSetAutoPositionSlotIndex, METH_VARARGS },
		{ "GetAutoSlotIndex", playerGetAutoSlotIndex,METH_VARARGS },
		{ "SetAutoSlotCoolTime", playerSetAutoSlotCoolTime, METH_VARARGS },
		{ "GetAutoSlotCoolTime",playerGetAutoSlotCoolTime,METH_VARARGS },
		{ "CheckSkillSlotCoolTime", playerCheckSkillSlotCoolTime, METH_VARARGS },
		{ "CheckPositionSlotCoolTime", playerCheckPositionSlotCoolTime, METH_VARARGS },
		{ "ClearAutoSKillSlot",playerClearAutoSKillSlot,METH_VARARGS },
		{ "ClearAutoPositionSlot",playerClearAutoPositionSlot,METH_VARARGS },
		{ "ClearAutoAllSlot",playerClearAutoAllSlot,METH_VARARGS },
		{ "CanStartAuto", playerCanStartAuto,METH_VARARGS },

		{ "AutoStartOnOff", playerAutoStartOnOff, METH_VARARGS },
		{ "AutoAttackOnOff", playerAutoAttackOnOff, METH_VARARGS },
		{ "AutoSkillOnOff", playerAutoSkillOnOff, METH_VARARGS },
		{ "AutoPositionOnOff", playerAutoPositionOnOff, METH_VARARGS },
		{ "AutoRangeOnOff", playerAutoRangeOnOff, METH_VARARGS },
		{ "SetAutoRestart", playerSetAutoRestart, METH_VARARGS },
#endif

#ifdef ENABLE_MONSTER_BACK
		{ "GetAttendanceRewardList",	playerGetAttendanceRewardList,		METH_VARARGS },
#endif

#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
		{ "GetAccumulateDamageByVID",			playerGetAccumulateDamageByVID,			METH_VARARGS },
#endif

#ifdef ENABLE_KEYCHANGE_SYSTEM
		// Keyboard Controls
		{ "IsOpenKeySettingWindow", playerIsOpenKeySettingWindow, METH_VARARGS },
		{ "KeySettingClear", playerKeySettingClear, METH_VARARGS },
		{ "KeySetting", playerKeySetting, METH_VARARGS },
		{ "OnKeyDown", playerOnKeyDown, METH_VARARGS },
		{ "OnKeyUp", playerOnKeyUp, METH_VARARGS },
#endif

#ifdef ENABLE_PARTY_MATCH
		{ "GetPartyMatchInfoMap",		playerGetPartyMatchInfoMap,			METH_VARARGS },
		{ "IsPartyMatchLoaded",			playerIsPartyMatchLoaded,			METH_VARARGS },
		{ "IsPartyMatchEnoughItem",		playerIsPartyMatchEnoughItem,		METH_VARARGS },
#endif

#ifdef ENABLE_SECOND_GUILDRENEWAL_SYSTEM
		{ "SetParalysis",		playerSetParalysis,		METH_VARARGS },
#endif

		{ "GetLevel",		playerGetLevel,		METH_VARARGS },

#ifdef ENABLE_MINI_GAME_YUTNORI
		{ "YutnoriShow",					playerYutnoriShow,					METH_VARARGS },
		{ "YutnoriChangeMotion",			playerYutnoriChangeMotion,			METH_VARARGS },
#endif

#ifdef ENABLE_FLOWER_EVENT
		{ "GetFlower", playerGetFlower, METH_VARARGS },
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
		{ "GetPremiumBattlePassID",		playerGetPremiumBattlePassID,		METH_VARARGS },
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
		{ "GetPageEquipment", 		playerGetPageEquipment,			METH_VARARGS },
		{ "GetPageEquipmentTotal", 	playerGetPageEquipmentTotal,	METH_VARARGS },
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
		{ "GetLottoMoney",				playerGetLottoMoney,				METH_VARARGS },
		{ "GetLottoTotalMoney",			playerGetLottoTotalMoney,			METH_VARARGS },
#endif

		{nullptr, nullptr, 0}};

	PyObject* poModule = Py_InitModule("playerm2g2", s_methods);	//@fixme430
	PyModule_AddIntConstant(poModule, "LEVEL",						POINT_LEVEL);						// 1
	PyModule_AddIntConstant(poModule, "VOICE",						POINT_VOICE);						// 2
	PyModule_AddIntConstant(poModule, "EXP",						POINT_EXP);							// 3
	PyModule_AddIntConstant(poModule, "NEXT_EXP",					POINT_NEXT_EXP);					// 4
	PyModule_AddIntConstant(poModule, "HP",							POINT_HP);							// 5
	PyModule_AddIntConstant(poModule, "MAX_HP",						POINT_MAX_HP);						// 6
	PyModule_AddIntConstant(poModule, "SP",							POINT_SP);							// 7
	PyModule_AddIntConstant(poModule, "MAX_SP",						POINT_MAX_SP);						// 8
	PyModule_AddIntConstant(poModule, "STAMINA",					POINT_STAMINA);						// 9
	PyModule_AddIntConstant(poModule, "MAX_STAMINA",				POINT_MAX_STAMINA);					// 10
	PyModule_AddIntConstant(poModule, "ELK",						POINT_GOLD);						// 11
	PyModule_AddIntConstant(poModule, "ST",							POINT_ST);							// 12
	PyModule_AddIntConstant(poModule, "HT",							POINT_HT);							// 13
	PyModule_AddIntConstant(poModule, "DX",							POINT_DX);							// 14
	PyModule_AddIntConstant(poModule, "IQ",							POINT_IQ);							// 15
	PyModule_AddIntConstant(poModule, "ATT_POWER",					POINT_ATT_POWER);					// 16
	PyModule_AddIntConstant(poModule, "ATT_SPEED",					POINT_ATT_SPEED);					// 17
	PyModule_AddIntConstant(poModule, "EVADE_RATE",					POINT_EVADE_RATE);					// 18
	PyModule_AddIntConstant(poModule, "MOVING_SPEED",				POINT_MOV_SPEED);					// 19
	PyModule_AddIntConstant(poModule, "DEF_GRADE",					POINT_DEF_GRADE);					// 20
	PyModule_AddIntConstant(poModule, "CASTING_SPEED",				POINT_CASTING_SPEED);				// 21
	PyModule_AddIntConstant(poModule, "MAG_ATT",					POINT_MAGIC_ATT_GRADE);				// 22
	PyModule_AddIntConstant(poModule, "MAG_DEF",					POINT_MAGIC_DEF_GRADE);				// 23
	PyModule_AddIntConstant(poModule, "EMPIRE_POINT",				POINT_EMPIRE_POINT);				// 24
	PyModule_AddIntConstant(poModule, "STAT",						POINT_STAT);						// 26
	PyModule_AddIntConstant(poModule, "SKILL_PASSIVE",				POINT_SUB_SKILL);					// 27
	PyModule_AddIntConstant(poModule, "SKILL_SUPPORT",				POINT_SUB_SKILL);					// 27
	PyModule_AddIntConstant(poModule, "SKILL_ACTIVE",				POINT_SKILL);						// 28
	PyModule_AddIntConstant(poModule, "ATT_MIN",					POINT_MIN_ATK);						// 29
	PyModule_AddIntConstant(poModule, "ATT_MAX",					POINT_MAX_ATK);						// 30
	PyModule_AddIntConstant(poModule, "PLAYTIME",					POINT_PLAYTIME);					// 31
	PyModule_AddIntConstant(poModule, "BOW_DISTANCE",				POINT_BOW_DISTANCE);				// 34
	PyModule_AddIntConstant(poModule, "HP_RECOVERY",				POINT_HP_RECOVERY);					// 35
	PyModule_AddIntConstant(poModule, "SP_RECOVERY",				POINT_SP_RECOVERY);					// 36
	PyModule_AddIntConstant(poModule, "POINT_CRITICAL_PCT",			POINT_CRITICAL_PCT);				// 40
	PyModule_AddIntConstant(poModule, "POINT_PENETRATE_PCT",		POINT_PENETRATE_PCT);				// 41
	PyModule_AddIntConstant(poModule, "ATTACKER_BONUS",				POINT_PARTY_ATT_GRADE);				// 91
	PyModule_AddIntConstant(poModule, "ATT_BONUS",					POINT_ATT_GRADE_BONUS);				// 95
	PyModule_AddIntConstant(poModule, "DEF_BONUS",					POINT_DEF_GRADE_BONUS);				// 96
	PyModule_AddIntConstant(poModule, "SKILL_HORSE",				POINT_HORSE_SKILL);					// 113
	PyModule_AddIntConstant(poModule, "POINT_MALL_ATTBONUS",		POINT_MALL_ATTBONUS);				// 114
	PyModule_AddIntConstant(poModule, "POINT_MALL_DEFBONUS",		POINT_MALL_DEFBONUS);				// 115
	PyModule_AddIntConstant(poModule, "POINT_MALL_EXPBONUS",		POINT_MALL_EXPBONUS);				// 116
	PyModule_AddIntConstant(poModule, "POINT_MALL_ITEMBONUS",		POINT_MALL_ITEMBONUS);				// 117
	PyModule_AddIntConstant(poModule, "POINT_MALL_GOLDBONUS",		POINT_MALL_GOLDBONUS);				// 118
	PyModule_AddIntConstant(poModule, "POINT_MAX_HP_PCT",			POINT_MAX_HP_PCT);					// 119
	PyModule_AddIntConstant(poModule, "POINT_MAX_SP_PCT",			POINT_MAX_SP_PCT);					// 120
	PyModule_AddIntConstant(poModule, "POINT_SKILL_DAMAGE_BONUS",	POINT_SKILL_DAMAGE_BONUS);			// 121
	PyModule_AddIntConstant(poModule, "POINT_NORMAL_HIT_DAMAGE_BONUS", POINT_NORMAL_HIT_DAMAGE_BONUS);	// 122
	PyModule_AddIntConstant(poModule, "POINT_SKILL_DEFEND_BONUS",	POINT_SKILL_DEFEND_BONUS);			// 123
	PyModule_AddIntConstant(poModule, "POINT_NORMAL_HIT_DEFEND_BONUS", POINT_NORMAL_HIT_DEFEND_BONUS);	// 124
	PyModule_AddIntConstant(poModule, "POINT_PC_BANG_EXP_BONUS",	POINT_PC_BANG_EXP_BONUS);			// 125
	PyModule_AddIntConstant(poModule, "POINT_PC_BANG_DROP_BONUS",	POINT_PC_BANG_DROP_BONUS);			// 126
	PyModule_AddIntConstant(poModule, "ENERGY",						POINT_ENERGY);						// 128
	PyModule_AddIntConstant(poModule, "ENERGY_END_TIME",			POINT_ENERGY_END_TIME);				// 129
	PyModule_AddIntConstant(poModule, "POINT_MELEE_MAGIC_ATT_BONUS_PER", POINT_MELEE_MAGIC_ATT_BONUS_PER);	// 132
#ifdef ENABLE_CHEQUE_SYSTEM
	PyModule_AddIntConstant(poModule, "CHEQUE",						POINT_CHEQUE);
#endif
#ifdef ENABLE_GEM_SYSTEM
	PyModule_AddIntConstant(poModule, "GEM",						POINT_GEM);							// 165
#endif
	PyModule_AddIntConstant(poModule, "MIN_MAGIC_WEP",				POINT_MIN_MAGIC_WEP);				// 202
	PyModule_AddIntConstant(poModule, "MAX_MAGIC_WEP",				POINT_MAX_MAGIC_WEP);				// 203

#ifdef ENABLE_YOHARA_SYSTEM
	PyModule_AddIntConstant(poModule, "SUNGMA_STR",					POINT_SUNGMA_STR);
	PyModule_AddIntConstant(poModule, "SUNGMA_HP",					POINT_SUNGMA_HP);
	PyModule_AddIntConstant(poModule, "SUNGMA_MOVE",				POINT_SUNGMA_MOVE);
	PyModule_AddIntConstant(poModule, "SUNGMA_IMMUNE",				POINT_SUNGMA_IMMUNE);

	PyModule_AddIntConstant(poModule, "SUNGMA_PER_STR",				POINT_SUNGMA_PER_STR);
	PyModule_AddIntConstant(poModule, "SUNGMA_PER_HP",				POINT_SUNGMA_PER_HP);
	PyModule_AddIntConstant(poModule, "SUNGMA_PER_MOVE",			POINT_SUNGMA_PER_MOVE);
	PyModule_AddIntConstant(poModule, "SUNGMA_PER_IMMUNE",			POINT_SUNGMA_PER_IMMUNE);

	PyModule_AddIntConstant(poModule, "CONQUEROR_LEVEL",			POINT_CONQUEROR_LEVEL);
	PyModule_AddIntConstant(poModule, "CONQUEROR_LEVEL_STEP",		POINT_CONQUEROR_LEVEL_STEP);
	PyModule_AddIntConstant(poModule, "CONQUEROR_POINT",			POINT_CONQUEROR_POINT);
	PyModule_AddIntConstant(poModule, "CONQUEROR_EXP",				POINT_CONQUEROR_EXP);
	PyModule_AddIntConstant(poModule, "CONQUEROR_NEXT_EXP",			POINT_CONQUEROR_NEXT_EXP);
#endif

	PyModule_AddIntConstant(poModule, "MAX_NUM",					POINT_MAX_NUM);						// 255

	////
	PyModule_AddIntConstant(poModule, "REFINE_FAIL_GRADE_DOWN",		REFINE_FAIL_GRADE_DOWN);
	PyModule_AddIntConstant(poModule, "REFINE_FAIL_DEL_ITEM",		REFINE_FAIL_DEL_ITEM);
	PyModule_AddIntConstant(poModule, "REFINE_FAIL_KEEP_GRADE",		REFINE_FAIL_KEEP_GRADE);
	PyModule_AddIntConstant(poModule, "REFINE_FAIL_MAX",			REFINE_FAIL_MAX);
	////

	PyModule_AddIntConstant(poModule, "SKILL_GRADE_NORMAL",			CPythonPlayer::SKILL_NORMAL);
	PyModule_AddIntConstant(poModule, "SKILL_GRADE_MASTER",			CPythonPlayer::SKILL_MASTER);
	PyModule_AddIntConstant(poModule, "SKILL_GRADE_GRAND_MASTER",	CPythonPlayer::SKILL_GRAND_MASTER);
	PyModule_AddIntConstant(poModule, "SKILL_GRADE_PERFECT_MASTER",	CPythonPlayer::SKILL_PERFECT_MASTER);

	PyModule_AddIntConstant(poModule, "CATEGORY_ACTIVE",			CPythonPlayer::CATEGORY_ACTIVE);
	PyModule_AddIntConstant(poModule, "CATEGORY_PASSIVE",			CPythonPlayer::CATEGORY_PASSIVE);

	PyModule_AddIntConstant(poModule, "INVENTORY_PAGE_COLUMN",		c_Inventory_Page_Column);
	PyModule_AddIntConstant(poModule, "INVENTORY_PAGE_ROW",			c_Inventory_Page_Row);
	PyModule_AddIntConstant(poModule, "INVENTORY_PAGE_SIZE",		c_Inventory_Page_Size);
	PyModule_AddIntConstant(poModule, "INVENTORY_PAGE_COUNT",		c_Inventory_Page_Count);
	PyModule_AddIntConstant(poModule, "INVENTORY_SLOT_COUNT",		c_Inventory_Slot_Count);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_SLOT_START",		c_Equipment_Start);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_PAGE_COUNT",		c_Equipment_Count);
	//ENABLE_NEW_EQUIPMENT_SYSTEM
	PyModule_AddIntConstant(poModule, "NEW_EQUIPMENT_SLOT_START",	c_New_Equipment_Start);
	PyModule_AddIntConstant(poModule, "NEW_EQUIPMENT_SLOT_COUNT",	c_New_Equipment_Count);
	//END_ENABLE_NEW_EQUIPMENT_SYSTEM

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	PyModule_AddIntConstant(poModule, "ROULETTE_ITEM_MAX", ROULETTE_ITEM_MAX);
	PyModule_AddIntConstant(poModule, "ROULETTE_PACKET_CLOSE", 0);
	PyModule_AddIntConstant(poModule, "ROULETTE_PACKET_TURN", 1);
	PyModule_AddIntConstant(poModule, "ROULETTE_PACKET_GIFT", 2);
	PyModule_AddIntConstant(poModule, "EVENTSOUL", POINT_SOUL);
#endif

	PyModule_AddIntConstant(poModule, "MBF_SKILL",					CPythonPlayer::MBF_SKILL);
	PyModule_AddIntConstant(poModule, "MBF_ATTACK",					CPythonPlayer::MBF_ATTACK);
	PyModule_AddIntConstant(poModule, "MBF_CAMERA",					CPythonPlayer::MBF_CAMERA);
	PyModule_AddIntConstant(poModule, "MBF_SMART",					CPythonPlayer::MBF_SMART);
	PyModule_AddIntConstant(poModule, "MBF_MOVE",					CPythonPlayer::MBF_MOVE);
	PyModule_AddIntConstant(poModule, "MBF_AUTO",					CPythonPlayer::MBF_AUTO);
	PyModule_AddIntConstant(poModule, "MBS_PRESS",					CPythonPlayer::MBS_PRESS);
	PyModule_AddIntConstant(poModule, "MBS_CLICK",					CPythonPlayer::MBS_CLICK);
	PyModule_AddIntConstant(poModule, "MBT_RIGHT",					CPythonPlayer::MBT_RIGHT);
	PyModule_AddIntConstant(poModule, "MBT_LEFT",					CPythonPlayer::MBT_LEFT);

	// ESlotType
	// Public code with server
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_NONE",						SLOT_TYPE_NONE);				// 0
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_INVENTORY",				SLOT_TYPE_INVENTORY);			// 1
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_SKILL",					SLOT_TYPE_SKILL);				// 2
	// Special indecies for client
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_SHOP",						SLOT_TYPE_SHOP);
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_EXCHANGE_OWNER",			SLOT_TYPE_EXCHANGE_OWNER);
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_EXCHANGE_TARGET",			SLOT_TYPE_EXCHANGE_TARGET);
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_QUICK_SLOT",				SLOT_TYPE_QUICK_SLOT);
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_SAFEBOX",					SLOT_TYPE_SAFEBOX);
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_PRIVATE_SHOP",				SLOT_TYPE_PRIVATE_SHOP);
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_MALL",						SLOT_TYPE_MALL);
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_EMOTION",					SLOT_TYPE_EMOTION);
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_DRAGON_SOUL_INVENTORY",	SLOT_TYPE_DRAGON_SOUL_INVENTORY);

	PyModule_AddIntConstant(poModule, "SLOT_TYPE_GUILDBANK",				SLOT_TYPE_GUILDBANK);
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_ACCE",						SLOT_TYPE_ACCE);
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_PET_FEED_WINDOW",			SLOT_TYPE_PET_FEED_WINDOW);
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_EQUIPMENT",				SLOT_TYPE_EQUIPMENT);
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_BELT_INVENTORY",			SLOT_TYPE_BELT_INVENTORY);
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_AUTO",						SLOT_TYPE_AUTO);
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_CHANGE_LOOK",				SLOT_TYPE_CHANGE_LOOK);	//ENABLE_CHANGE_LOOK_SYSTEM
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_FISH_EVENT",				SLOT_TYPE_FISH_EVENT);
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_AURA",						SLOT_TYPE_AURA);
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_PREMIUM_PRIVATE_SHOP",		SLOT_TYPE_PREMIUM_PRIVATE_SHOP);
#ifdef ENABLE_SWITCHBOT
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_SWITCHBOT",				SLOT_TYPE_SWITCHBOT);
#endif

	// EWindows
	PyModule_AddIntConstant(poModule, "RESERVED_WINDOW",		RESERVED_WINDOW);
	PyModule_AddIntConstant(poModule, "INVENTORY",				INVENTORY);
	PyModule_AddIntConstant(poModule, "EQUIPMENT",				EQUIPMENT);
	PyModule_AddIntConstant(poModule, "SAFEBOX",				SAFEBOX);
	PyModule_AddIntConstant(poModule, "MALL",					MALL);
	PyModule_AddIntConstant(poModule, "DRAGON_SOUL_INVENTORY",	DRAGON_SOUL_INVENTORY);
	PyModule_AddIntConstant(poModule, "BELT_INVENTORY",			BELT_INVENTORY);
	PyModule_AddIntConstant(poModule, "GUILDBANK",				GUILDBANK);
	PyModule_AddIntConstant(poModule, "MAIL",					MAIL);
	PyModule_AddIntConstant(poModule, "NPC_STORAGE",			NPC_STORAGE);			//ENABLE_ATTR_6TH_7TH
	PyModule_AddIntConstant(poModule, "PREMIUM_PRIVATE_SHOP",	PREMIUM_PRIVATE_SHOP);
	PyModule_AddIntConstant(poModule, "ACCEREFINE",				ACCEREFINE);
	PyModule_AddIntConstant(poModule, "GROUND",					GROUND);
	PyModule_AddIntConstant(poModule, "PET_FEED",				PET_FEED);
	PyModule_AddIntConstant(poModule, "CHANGELOOK",				CHANGELOOK);
	PyModule_AddIntConstant(poModule, "AURA_REFINE",			AURA_REFINE);
#ifdef ENABLE_SWITCHBOT
	PyModule_AddIntConstant(poModule, "SWITCHBOT",				SWITCHBOT);
#endif
#ifdef ENABLE_SPECIAL_INVENTORY
	PyModule_AddIntConstant(poModule, "INVENTORY_TYPE_INVENTORY", INVENTORY_TYPE_INVENTORY);
	PyModule_AddIntConstant(poModule, "INVENTORY_TYPE_SKILLBOOK", INVENTORY_TYPE_SKILLBOOK);
	PyModule_AddIntConstant(poModule, "INVENTORY_TYPE_STONE", INVENTORY_TYPE_STONE);
	PyModule_AddIntConstant(poModule, "INVENTORY_TYPE_MATERIAL", INVENTORY_TYPE_MATERIAL);
	PyModule_AddIntConstant(poModule, "INVENTORY_TYPE_COUNT", INVENTORY_TYPE_COUNT);

	PyModule_AddIntConstant(poModule, "SPECIAL_INVENTORY_WIDTH", SPECIAL_INVENTORY_WIDTH);
	PyModule_AddIntConstant(poModule, "SPECIAL_INVENTORY_HEIGHT", SPECIAL_INVENTORY_HEIGHT);
	PyModule_AddIntConstant(poModule, "SPECIAL_INVENTORY_PAGE_SIZE", SPECIAL_INVENTORY_PAGE_SIZE);

	PyModule_AddIntConstant(poModule, "SPECIAL_INVENTORY_SLOT_START", c_Special_Inventory_Slot_Start);
	PyModule_AddIntConstant(poModule, "SPECIAL_INVENTORY_SLOT_END", c_Special_Inventory_Slot_End);

	PyModule_AddIntConstant(poModule, "SKILLBOOK_INVENTORY_SLOT_START", c_Special_Inventory_Skillbook_Slot_Start);
	PyModule_AddIntConstant(poModule, "SKILLBOOK_INVENTORY_SLOT_END", c_Special_Inventory_Skillbook_Slot_End);

	PyModule_AddIntConstant(poModule, "STONE_INVENTORY_SLOT_START", c_Special_Inventory_Stone_Slot_Start);
	PyModule_AddIntConstant(poModule, "STONE_INVENTORY_SLOT_END", c_Special_Inventory_Stone_Slot_End);

	PyModule_AddIntConstant(poModule, "MATERIAL_INVENTORY_SLOT_START", c_Special_Inventory_Material_Slot_Start);
	PyModule_AddIntConstant(poModule, "MATERIAL_INVENTORY_SLOT_END", c_Special_Inventory_Material_Slot_End);
#endif

	PyModule_AddIntConstant(poModule, "ITEM_MONEY", -1);
	PyModule_AddIntConstant(poModule, "SKILL_SLOT_COUNT", SKILL_MAX_NUM);
	PyModule_AddIntConstant(poModule, "EFFECT_PICK", CPythonPlayer::EFFECT_PICK);

	PyModule_AddIntConstant(poModule, "METIN_SOCKET_TYPE_NONE", CPythonPlayer::METIN_SOCKET_TYPE_NONE);
	PyModule_AddIntConstant(poModule, "METIN_SOCKET_TYPE_SILVER", CPythonPlayer::METIN_SOCKET_TYPE_SILVER);
	PyModule_AddIntConstant(poModule, "METIN_SOCKET_TYPE_GOLD", CPythonPlayer::METIN_SOCKET_TYPE_GOLD);
	PyModule_AddIntConstant(poModule, "METIN_SOCKET_MAX_NUM", ITEM_SOCKET_SLOT_MAX_NUM);
#ifdef ENABLE_PROTO_RENEWAL
	PyModule_AddIntConstant(poModule, "ITEM_METIN_SOCKET_MAX", METIN_SOCKET_MAX_NUM);
#endif

	// refactored attribute slot begin
	PyModule_AddIntConstant(poModule, "ATTRIBUTE_SLOT_NORM_NUM", ITEM_ATTRIBUTE_SLOT_NORM_NUM);
	PyModule_AddIntConstant(poModule, "ATTRIBUTE_SLOT_RARE_NUM", ITEM_ATTRIBUTE_SLOT_RARE_NUM);
	PyModule_AddIntConstant(poModule, "ATTRIBUTE_SLOT_NORM_START", ITEM_ATTRIBUTE_SLOT_NORM_START);
	PyModule_AddIntConstant(poModule, "ATTRIBUTE_SLOT_NORM_END", ITEM_ATTRIBUTE_SLOT_NORM_END);
	PyModule_AddIntConstant(poModule, "ATTRIBUTE_SLOT_RARE_START", ITEM_ATTRIBUTE_SLOT_RARE_START);
	PyModule_AddIntConstant(poModule, "ATTRIBUTE_SLOT_RARE_END", ITEM_ATTRIBUTE_SLOT_RARE_END);
	PyModule_AddIntConstant(poModule, "ATTRIBUTE_SLOT_MAX_NUM", ITEM_ATTRIBUTE_SLOT_MAX_NUM);
	// refactored attribute slot end
#ifdef ENABLE_YOHARA_SYSTEM
	PyModule_AddIntConstant(poModule, "APPLY_RANDOM_SLOT_MAX_NUM", APPLY_RANDOM_SLOT_MAX_NUM);
	PyModule_AddIntConstant(poModule, "ITEM_RANDOM_VALUES_NUM", ITEM_RANDOM_VALUES_MAX_NUM);
#endif

	// Refine
	PyModule_AddIntConstant(poModule, "REFINE_CANT", REFINE_CANT);
	PyModule_AddIntConstant(poModule, "REFINE_OK", REFINE_OK);
	PyModule_AddIntConstant(poModule, "REFINE_ALREADY_MAX_SOCKET_COUNT", REFINE_ALREADY_MAX_SOCKET_COUNT);
	PyModule_AddIntConstant(poModule, "REFINE_NEED_MORE_GOOD_SCROLL", REFINE_NEED_MORE_GOOD_SCROLL);
#ifdef __UNIMPLEMENTED_EXTEND__
	PyModule_AddIntConstant(poModule, "REFINE_CANT_NO_DD_ITEM", REFINE_CANT_NO_DD_ITEM);
	PyModule_AddIntConstant(poModule, "REFINE_CANT_DD_ITEM", REFINE_CANT_DD_ITEM);
#endif
	PyModule_AddIntConstant(poModule, "REFINE_CANT_MAKE_SOCKET_ITEM", REFINE_CANT_MAKE_SOCKET_ITEM);
	PyModule_AddIntConstant(poModule, "REFINE_NOT_NEXT_GRADE_ITEM", REFINE_NOT_NEXT_GRADE_ITEM);
	PyModule_AddIntConstant(poModule, "REFINE_CANT_REFINE_METIN_TO_EQUIPMENT", REFINE_CANT_REFINE_METIN_TO_EQUIPMENT);
	PyModule_AddIntConstant(poModule, "REFINE_CANT_REFINE_ROD", REFINE_CANT_REFINE_ROD);
	PyModule_AddIntConstant(poModule, "ATTACH_METIN_CANT", ATTACH_METIN_CANT);
	PyModule_AddIntConstant(poModule, "ATTACH_METIN_OK", ATTACH_METIN_OK);
	PyModule_AddIntConstant(poModule, "ATTACH_METIN_NOT_MATCHABLE_ITEM", ATTACH_METIN_NOT_MATCHABLE_ITEM);
	PyModule_AddIntConstant(poModule, "ATTACH_METIN_NO_MATCHABLE_SOCKET", ATTACH_METIN_NO_MATCHABLE_SOCKET);
	PyModule_AddIntConstant(poModule, "ATTACH_METIN_NOT_EXIST_GOLD_SOCKET", ATTACH_METIN_NOT_EXIST_GOLD_SOCKET);
	PyModule_AddIntConstant(poModule, "ATTACH_METIN_CANT_ATTACH_TO_EQUIPMENT", ATTACH_METIN_CANT_ATTACH_TO_EQUIPMENT);
	PyModule_AddIntConstant(poModule, "DETACH_METIN_CANT", DETACH_METIN_CANT);
	PyModule_AddIntConstant(poModule, "DETACH_METIN_OK", DETACH_METIN_OK);

#ifdef WJ_ENABLE_TRADABLE_ICON
	PyModule_AddIntConstant(poModule, "ON_TOP_WND_NONE", ON_TOP_WND_NONE);
	PyModule_AddIntConstant(poModule, "ON_TOP_WND_SHOP", ON_TOP_WND_SHOP);
	PyModule_AddIntConstant(poModule, "ON_TOP_WND_EXCHANGE", ON_TOP_WND_EXCHANGE);
	PyModule_AddIntConstant(poModule, "ON_TOP_WND_SAFEBOX", ON_TOP_WND_SAFEBOX);
	PyModule_AddIntConstant(poModule, "ON_TOP_WND_PRIVATE_SHOP", ON_TOP_WND_PRIVATE_SHOP);
#	ifdef ENABLE_MOVE_COSTUME_ATTR
	PyModule_AddIntConstant(poModule, "ON_TOP_WND_ITEM_COMB", ON_TOP_WND_ITEM_COMB);
#	endif
#	ifdef ENABLE_GROWTH_PET_SYSTEM
	PyModule_AddIntConstant(poModule, "ON_TOP_WND_PET_FEED", ON_TOP_WND_PET_FEED);
#	endif
#	ifdef ENABLE_FISH_EVENT
	PyModule_AddIntConstant(poModule, "ON_TOP_WND_FISH_EVENT", ON_TOP_WND_FISH_EVENT);
#	endif
#	ifdef ENABLE_MAILBOX
	PyModule_AddIntConstant(poModule, "ON_TOP_WND_MAILBOX", ON_TOP_WND_MAILBOX);
#	endif
#	ifdef ENABLE_PET_ATTR_DETERMINE
	PyModule_AddIntConstant(poModule, "ON_TOP_WND_PET_ATTR_CHANGE", ON_TOP_WND_PET_ATTR_CHANGE);
#	endif
#	ifdef ENABLE_LUCKY_EVENT
	PyModule_AddIntConstant(poModule, "ON_TOP_WND_LUCKY_BOX", ON_TOP_WND_LUCKY_BOX);
#	endif
#	ifdef ENABLE_ATTR_6TH_7TH
	PyModule_AddIntConstant(poModule, "ON_TOP_WND_ATTR67", ON_TOP_WND_ATTR67);
#	endif
#	ifdef ENABLE_PET_PRIMIUM_FEEDSTUFF
	PyModule_AddIntConstant(poModule, "ON_TOP_WND_PET_PRIMIUM_FEEDSTUFF", ON_TOP_WND_PET_PRIMIUM_FEEDSTUFF);
#	endif
#	ifdef ENABLE_PASSIVE_ATTR
	PyModule_AddIntConstant(poModule, "ON_TOP_WND_PASSIVE_ATTR", ON_TOP_WND_PASSIVE_ATTR);
#	endif
#endif

	// Party
	PyModule_AddIntConstant(poModule, "PARTY_STATE_NORMAL", CPythonPlayer::PARTY_ROLE_NORMAL);
	PyModule_AddIntConstant(poModule, "PARTY_STATE_LEADER", CPythonPlayer::PARTY_ROLE_LEADER);
	PyModule_AddIntConstant(poModule, "PARTY_STATE_ATTACKER", CPythonPlayer::PARTY_ROLE_ATTACKER);
	PyModule_AddIntConstant(poModule, "PARTY_STATE_TANKER", CPythonPlayer::PARTY_ROLE_TANKER);
	PyModule_AddIntConstant(poModule, "PARTY_STATE_BUFFER", CPythonPlayer::PARTY_ROLE_BUFFER);
	PyModule_AddIntConstant(poModule, "PARTY_STATE_SKILL_MASTER", CPythonPlayer::PARTY_ROLE_SKILL_MASTER);
	PyModule_AddIntConstant(poModule, "PARTY_STATE_BERSERKER", CPythonPlayer::PARTY_ROLE_BERSERKER);
	PyModule_AddIntConstant(poModule, "PARTY_STATE_DEFENDER", CPythonPlayer::PARTY_ROLE_DEFENDER);
	PyModule_AddIntConstant(poModule, "PARTY_STATE_MAX_NUM", CPythonPlayer::PARTY_ROLE_MAX_NUM);

	// Skill Index
	PyModule_AddIntConstant(poModule, "SKILL_INDEX_TONGSOL", c_iSkillIndex_Tongsol);
	PyModule_AddIntConstant(poModule, "SKILL_INDEX_FISHING", c_iSkillIndex_Fishing);
	PyModule_AddIntConstant(poModule, "SKILL_INDEX_MINING", c_iSkillIndex_Mining);
	PyModule_AddIntConstant(poModule, "SKILL_INDEX_MAKING", c_iSkillIndex_Making);
	PyModule_AddIntConstant(poModule, "SKILL_INDEX_COMBO", c_iSkillIndex_Combo);
#ifdef ENABLE_PASSIVE_ATTR
	PyModule_AddIntConstant(poModule, "SKILL_INDEX_AUTOATTACK", c_iSkillIndex_AutoAttack);
#endif
	PyModule_AddIntConstant(poModule, "SKILL_INDEX_LANGUAGE1", c_iSkillIndex_Language1);
	PyModule_AddIntConstant(poModule, "SKILL_INDEX_LANGUAGE2", c_iSkillIndex_Language2);
	PyModule_AddIntConstant(poModule, "SKILL_INDEX_LANGUAGE3", c_iSkillIndex_Language3);
	PyModule_AddIntConstant(poModule, "SKILL_INDEX_POLYMORPH", c_iSkillIndex_Polymorph);
	PyModule_AddIntConstant(poModule, "SKILL_INDEX_RIDING", c_iSkillIndex_Riding);
	PyModule_AddIntConstant(poModule, "SKILL_INDEX_SUMMON", c_iSkillIndex_Summon);

	// PK Mode
	PyModule_AddIntConstant(poModule, "PK_MODE_PEACE", PK_MODE_PEACE);
	PyModule_AddIntConstant(poModule, "PK_MODE_REVENGE", PK_MODE_REVENGE);
	PyModule_AddIntConstant(poModule, "PK_MODE_FREE", PK_MODE_FREE);
	PyModule_AddIntConstant(poModule, "PK_MODE_PROTECT", PK_MODE_PROTECT);
	PyModule_AddIntConstant(poModule, "PK_MODE_GUILD", PK_MODE_GUILD);
	PyModule_AddIntConstant(poModule, "PK_MODE_MAX_NUM", PK_MODE_MAX_NUM);

	// Block Mode
	PyModule_AddIntConstant(poModule, "BLOCK_EXCHANGE", BLOCK_EXCHANGE);
	PyModule_AddIntConstant(poModule, "BLOCK_PARTY", BLOCK_PARTY_INVITE);
	PyModule_AddIntConstant(poModule, "BLOCK_GUILD", BLOCK_GUILD_INVITE);
	PyModule_AddIntConstant(poModule, "BLOCK_WHISPER", BLOCK_WHISPER);
	PyModule_AddIntConstant(poModule, "BLOCK_FRIEND", BLOCK_MESSENGER_INVITE);
	PyModule_AddIntConstant(poModule, "BLOCK_PARTY_REQUEST", BLOCK_PARTY_REQUEST);

	// Party
	PyModule_AddIntConstant(poModule, "PARTY_EXP_NON_DISTRIBUTION", PARTY_EXP_DISTRIBUTION_NON_PARITY);
	PyModule_AddIntConstant(poModule, "PARTY_EXP_DISTRIBUTION_PARITY", PARTY_EXP_DISTRIBUTION_PARITY);
#ifdef ENABLE_PARTY_EXP_DISTRIBUTION_EQUAL
	PyModule_AddIntConstant(poModule, "PARTY_EXP_DISTRIBUTION_EQUAL", PARTY_EXP_DISTRIBUTION_EQUAL);
#endif

	// Emotion
	PyModule_AddIntConstant(poModule, "EMOTION_CLAP", EMOTION_CLAP);
	PyModule_AddIntConstant(poModule, "EMOTION_CHEERS_1", EMOTION_CHEERS_1);
	PyModule_AddIntConstant(poModule, "EMOTION_CHEERS_2", EMOTION_CHEERS_2);
	PyModule_AddIntConstant(poModule, "EMOTION_DANCE_1", EMOTION_DANCE_1);
	PyModule_AddIntConstant(poModule, "EMOTION_DANCE_2", EMOTION_DANCE_2);
	PyModule_AddIntConstant(poModule, "EMOTION_DANCE_3", EMOTION_DANCE_3);
	PyModule_AddIntConstant(poModule, "EMOTION_DANCE_4", EMOTION_DANCE_4);
	PyModule_AddIntConstant(poModule, "EMOTION_DANCE_5", EMOTION_DANCE_5);
	PyModule_AddIntConstant(poModule, "EMOTION_DANCE_6", EMOTION_DANCE_6);
	PyModule_AddIntConstant(poModule, "EMOTION_CONGRATULATION", EMOTION_CONGRATULATION);
	PyModule_AddIntConstant(poModule, "EMOTION_FORGIVE", EMOTION_FORGIVE);
	PyModule_AddIntConstant(poModule, "EMOTION_ANGRY", EMOTION_ANGRY);
	PyModule_AddIntConstant(poModule, "EMOTION_ATTRACTIVE", EMOTION_ATTRACTIVE);
	PyModule_AddIntConstant(poModule, "EMOTION_SAD", EMOTION_SAD);
	PyModule_AddIntConstant(poModule, "EMOTION_SHY", EMOTION_SHY);
	PyModule_AddIntConstant(poModule, "EMOTION_CHEERUP", EMOTION_CHEERUP);
	PyModule_AddIntConstant(poModule, "EMOTION_BANTER", EMOTION_BANTER);
	PyModule_AddIntConstant(poModule, "EMOTION_JOY", EMOTION_JOY);

	PyModule_AddIntConstant(poModule, "EMOTION_KISS", EMOTION_KISS);
	PyModule_AddIntConstant(poModule, "EMOTION_FRENCH_KISS", EMOTION_FRENCH_KISS);
	PyModule_AddIntConstant(poModule, "EMOTION_SLAP", EMOTION_SLAP);

#ifdef ENABLE_EXPRESSING_EMOTION
	PyModule_AddIntConstant(poModule, "EMOTION_PUSHUP",			EMOTION_PUSHUP);
	PyModule_AddIntConstant(poModule, "EMOTION_DANCE_7",		EMOTION_DANCE_7);
	PyModule_AddIntConstant(poModule, "EMOTION_EXERCISE",		EMOTION_EXERCISE);
	PyModule_AddIntConstant(poModule, "EMOTION_DOZE",			EMOTION_DOZE);
	PyModule_AddIntConstant(poModule, "EMOTION_SELFIE",			EMOTION_SELFIE);
#endif

	//// ¿⁄µøπ∞æ‡ ≈∏¿‘
	PyModule_AddIntConstant(poModule, "AUTO_POTION_TYPE_HP", CPythonPlayer::AUTO_POTION_TYPE_HP);
	PyModule_AddIntConstant(poModule, "AUTO_POTION_TYPE_SP", CPythonPlayer::AUTO_POTION_TYPE_SP);
#ifdef ENABLE_ANTI_EXP_RING
	PyModule_AddIntConstant(poModule, "AUTO_POTION_TYPE_EXP", CPythonPlayer::AUTO_POTION_TYPE_EXP);
#endif

	// øÎ»•ºÆ
	PyModule_AddIntConstant(poModule, "DRAGON_SOUL_PAGE_SIZE", c_DragonSoul_Inventory_Box_Size);
	PyModule_AddIntConstant(poModule, "DRAGON_SOUL_PAGE_COUNT", DRAGON_SOUL_GRADE_MAX);
	PyModule_AddIntConstant(poModule, "DRAGON_SOUL_SLOT_COUNT", c_DragonSoul_Inventory_Count);
	PyModule_AddIntConstant(poModule, "DRAGON_SOUL_EQUIPMENT_SLOT_START", c_DragonSoul_Equip_Start);
	PyModule_AddIntConstant(poModule, "DRAGON_SOUL_EQUIPMENT_PAGE_COUNT", DS_DECK_MAX_NUM);
	PyModule_AddIntConstant(poModule, "DRAGON_SOUL_EQUIPMENT_FIRST_SIZE", c_DragonSoul_Equip_Slot_Max);
	PyModule_AddIntConstant(poModule, "DRAGON_SOUL_EQUIPMENT_SLOT_END", c_DragonSoul_Equip_End);

	// øÎ»•ºÆ ∞≥∑Æ√¢
	PyModule_AddIntConstant(poModule, "DRAGON_SOUL_REFINE_CLOSE", DS_SUB_HEADER_CLOSE);
	PyModule_AddIntConstant(poModule, "DS_SUB_HEADER_DO_UPGRADE", DS_SUB_HEADER_DO_UPGRADE);
	PyModule_AddIntConstant(poModule, "DS_SUB_HEADER_DO_IMPROVEMENT", DS_SUB_HEADER_DO_IMPROVEMENT);
	PyModule_AddIntConstant(poModule, "DS_SUB_HEADER_DO_REFINE", DS_SUB_HEADER_DO_REFINE);
#ifdef ENABLE_DS_CHANGE_ATTR
	PyModule_AddIntConstant(poModule, "DS_SUB_HEADER_OPEN", DS_SUB_HEADER_OPEN);
	PyModule_AddIntConstant(poModule, "DS_SUB_HEADER_OPEN_CHANGE_ATTR", DS_SUB_HEADER_OPEN_CHANGE_ATTR);
	PyModule_AddIntConstant(poModule, "DS_SUB_HEADER_DO_CHANGE_ATTR", DS_SUB_HEADER_DO_CHANGE_ATTR);
#endif

#ifdef ENABLE_BATTLE_FIELD
	PyModule_AddIntConstant(poModule, "PK_MODE_BATTLE", PK_MODE_BATTLE);
	PyModule_AddIntConstant(poModule, "BATTLE_POINT", POINT_BATTLE_FIELD);
#endif

#ifdef ENABLE_TARGET_AFFECT_SHOWER
	PyModule_AddIntConstant(poModule, "TARGET_AFFECT_POISON", TARGET_AFFECT_POISON);
	PyModule_AddIntConstant(poModule, "TARGET_AFFECT_FIRE", TARGET_AFFECT_FIRE);
	PyModule_AddIntConstant(poModule, "TARGET_AFFECT_STUN", TARGET_AFFECT_STUN);
	PyModule_AddIntConstant(poModule, "TARGET_AFFECT_SLOW", TARGET_AFFECT_SLOW);
#	if defined(ENABLE_PROTO_RENEWAL_CUSTOM) && defined(ENABLE_WOLFMAN_CHARACTER)
	PyModule_AddIntConstant(poModule, "TARGET_AFFECT_BLEEDING", TARGET_AFFECT_BLEEDING);
#	endif
	PyModule_AddIntConstant(poModule, "TARGET_AFFECT_MAX_NUM", TARGET_AFFECT_MAX_NUM);
#endif

	// Money Max Limit
	PyModule_AddIntConstant(poModule, "GOLD_MAX", GOLD_MAX);
#ifdef ENABLE_CHEQUE_SYSTEM
	PyModule_AddIntConstant(poModule, "CHEQUE_MAX", CHEQUE_MAX);
#endif
#ifdef ENABLE_GEM_SYSTEM
	PyModule_AddIntConstant(poModule, "GAYA_MAX", GAYA_MAX);
#endif

	// New Modules
#ifdef ENABLE_SPECIAL_INVENTORY
	PyModule_AddIntConstant(poModule, "ITEM_SLOT_COUNT", 180);
	PyModule_AddIntConstant(poModule, "SPECIAL_ITEM_SLOT_COUNT", c_Inventory_Slot_Count);
#else
	PyModule_AddIntConstant(poModule, "ITEM_SLOT_COUNT", c_ItemSlot_Count);
#endif

#ifdef ENABLE_ATTR_6TH_7TH
	PyModule_AddIntConstant(poModule, "SKILLBOOK_COMB_SLOT_MAX", c_SkillBook_Comb_Slot_Max);
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
	PyModule_AddIntConstant(poModule, "POINT_MEDAL_OF_HONOR", POINT_MEDAL_OF_HONOR);
#endif

#ifdef ENABLE_REFINE_ELEMENT
	PyModule_AddIntConstant(poModule, "ATT_ELEMENTS", POINT_ATT_ELEMENTS);
	PyModule_AddIntConstant(poModule, "ELEMENT_UPGRADE_CANT_ADD", ELEMENT_UPGRADE_CANT_ADD);
	PyModule_AddIntConstant(poModule, "ELEMENT_UPGRADE_ADD", ELEMENT_UPGRADE_ADD);
	PyModule_AddIntConstant(poModule, "ELEMENT_DOWNGRADE_CANT_ADD", ELEMENT_DOWNGRADE_CANT_ADD);
	PyModule_AddIntConstant(poModule, "ELEMENT_DOWNGRADE_ADD", ELEMENT_DOWNGRADE_ADD);
	PyModule_AddIntConstant(poModule, "ELEMENT_CANT", ELEMENT_CANT);
	PyModule_AddIntConstant(poModule, "ELEMENT_CHANGE_ADD", ELEMENT_CHANGE_ADD);
	PyModule_AddIntConstant(poModule, "ELEMENT_CHANGE_CANT_ADD", ELEMENT_CHANGE_CANT_ADD);
	PyModule_AddIntConstant(poModule, "MAX_ELEMENTS_SPELL", MAX_ELEMENTS_SPELL);
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
	PyModule_AddIntConstant(poModule, "PET_FEED_SLOT_MAX", PET_FEED_SLOT_MAX);
	PyModule_AddIntConstant(poModule, "FEED_LIFE_TIME_EVENT", FEED_LIFE_TIME_EVENT);
	PyModule_AddIntConstant(poModule, "FEED_EVOL_EVENT", FEED_EVOL_EVENT);
	PyModule_AddIntConstant(poModule, "FEED_EXP_EVENT", FEED_EXP_EVENT);
	PyModule_AddIntConstant(poModule, "FEED_BUTTON_MAX", FEED_BUTTON_MAX);
	PyModule_AddIntConstant(poModule, "PET_GROWTH_EVOL_MAX", PET_GROWTH_EVOL_MAX);
	PyModule_AddIntConstant(poModule, "PET_GROWTH_SKILL_OPEN_EVOL_LEVEL", PET_GROWTH_SKILL_OPEN_EVOL_LEVEL);
	PyModule_AddIntConstant(poModule, "PET_GROWTH_SKILL_LEVEL_MAX", PET_GROWTH_SKILL_LEVEL_MAX);
	PyModule_AddIntConstant(poModule, "SPECIAL_EVOL_MIN_AGE", SPECIAL_EVOL_MIN_AGE);
	PyModule_AddIntConstant(poModule, "LIFE_TIME_FLASH_MIN_TIME", LIFE_TIME_FLASH_MIN_TIME);
	PyModule_AddIntConstant(poModule, "PET_SKILL_COUNT_MAX", PET_SKILL_COUNT_MAX);

	PyModule_AddIntConstant(poModule, "QUICK_SLOT_POS_ERROR", QUICK_SLOT_POS_ERROR);
	PyModule_AddIntConstant(poModule, "QUICK_SLOT_ITEM_USE_SUCCESS", QUICK_SLOT_ITEM_USE_SUCCESS);
	PyModule_AddIntConstant(poModule, "QUICK_SLOT_IS_NOT_ITEM", QUICK_SLOT_IS_NOT_ITEM);
	PyModule_AddIntConstant(poModule, "QUICK_SLOT_PET_ITEM_USE_SUCCESS", QUICK_SLOT_PET_ITEM_USE_SUCCESS);
	PyModule_AddIntConstant(poModule, "QUICK_SLOT_PET_ITEM_USE_FAILED", QUICK_SLOT_PET_ITEM_USE_FAILED);
	PyModule_AddIntConstant(poModule, "QUICK_SLOT_CAN_NOT_USE_PET_ITEM", QUICK_SLOT_CAN_NOT_USE_PET_ITEM);

#	ifdef ENABLE_PET_ATTR_DETERMINE
	PyModule_AddIntConstant(poModule, "PET_WND_SLOT_ATTR_CHANGE", PET_WND_SLOT_ATTR_CHANGE); // PET_WND_SLOT_ATTR_CHANGE
	PyModule_AddIntConstant(poModule, "PET_WND_SLOT_ATTR_CHANGE_ITEM", PET_WND_SLOT_ATTR_CHANGE_ITEM); // PET_WND_SLOT_ATTR_CHANGE_ITEM
	PyModule_AddIntConstant(poModule, "PET_WND_SLOT_ATTR_CHANGE_RESULT", PET_WND_SLOT_ATTR_CHANGE_RESULT); // PET_WND_SLOT_ATTR_CHANGE_RESULT
	PyModule_AddIntConstant(poModule, "PET_WND_SLOT_ATTR_CHANGE_MAX", PET_WND_SLOT_ATTR_CHANGE_MAX); // PET_WND_SLOT_ATTR_CHANGE_MAX
#	endif

	PyModule_AddIntConstant(poModule, "PET_WINDOW_INFO", PET_WINDOW_INFO);
	PyModule_AddIntConstant(poModule, "PET_WINDOW_ATTR_CHANGE", PET_WINDOW_ATTR_CHANGE);
	PyModule_AddIntConstant(poModule, "PET_WINDOW_PRIMIUM_FEEDSTUFF", PET_WINDOW_PRIMIUM_FEEDSTUFF);

	PyModule_AddIntConstant(poModule, "PET_REVIVE_MATERIAL_SLOT_MAX", PET_REVIVE_MATERIAL_SLOT_MAX);
#endif

#if defined (ENABLE_EXTEND_INVEN_SYSTEM) || defined(ENABLE_EXTEND_INVEN_ITEM_UPGRADE)
	PyModule_AddIntConstant(poModule, "EX_INVEN_FAIL_FALL_SHORT", 0);
	PyModule_AddIntConstant(poModule, "EX_INVEN_SUCCESS", 1);
	PyModule_AddIntConstant(poModule, "EX_INVEN_FAIL_FOURTH_PAGE_STAGE_MAX", 2);

	PyModule_AddIntConstant(poModule, "EX_INVENTORY_STAGE_MAX", 18);
	PyModule_AddIntConstant(poModule, "EX_INVENTORY_PAGE_COUNT", 2);
	PyModule_AddIntConstant(poModule, "EX_INVENTORY_PAGE_START", 3);

	PyModule_AddIntConstant(poModule, "WEAR_MAX", c_Wear_Max);

# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	PyModule_AddIntConstant(poModule, "EX_SPECIAL_INVENTORY_STAGE_MAX", 27);
	PyModule_AddIntConstant(poModule, "EX_SPECIAL_INVENTORY_PAGE_COUNT", 3);
	PyModule_AddIntConstant(poModule, "EX_SPECIAL_INVENTORY_PAGE_START", 2);
# endif
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "ACCE_SLOT_TYPE_COMBINE", ACCE_SLOT_TYPE_COMBINE);
	PyModule_AddIntConstant(poModule, "ACCE_SLOT_TYPE_ABSORB", ACCE_SLOT_TYPE_ABSORB);

	PyModule_AddIntConstant(poModule, "ACCE_SLOT_LEFT", 0);
	PyModule_AddIntConstant(poModule, "ACCE_SLOT_RIGHT", 1);

	PyModule_AddIntConstant(poModule, "ACCE_SLOT_MAX", 3);

	PyModule_AddIntConstant(poModule, "ACCEREFINE", 10);
#endif

#ifdef ENABLE_MOVE_COSTUME_ATTR
	PyModule_AddIntConstant(poModule, "COMB_WND_SLOT_MEDIUM", COMB_WND_SLOT_MEDIUM);
	PyModule_AddIntConstant(poModule, "COMB_WND_SLOT_BASE", COMB_WND_SLOT_BASE);
	PyModule_AddIntConstant(poModule, "COMB_WND_SLOT_MATERIAL", COMB_WND_SLOT_MATERIAL);
	PyModule_AddIntConstant(poModule, "COMB_WND_SLOT_RESULT", COMB_WND_SLOT_RESULT);
	PyModule_AddIntConstant(poModule, "COMB_WND_SLOT_MAX", COMB_WND_SLOT_MAX);
#endif

#ifdef ENABLE_AURA_SYSTEM
	PyModule_AddIntConstant(poModule, "REFINE_CANT_REFINE_AURA_ITEM", REFINE_CANT_REFINE_AURA_ITEM);
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_AURA", SLOT_TYPE_AURA);
	PyModule_AddIntConstant(poModule, "AURA_MAX_LEVEL", c_AuraMaxLevel);
	PyModule_AddIntConstant(poModule, "AURA_SLOT_MAIN", AURA_SLOT_MAIN);
	PyModule_AddIntConstant(poModule, "AURA_SLOT_SUB", AURA_SLOT_SUB);
	PyModule_AddIntConstant(poModule, "AURA_SLOT_RESULT", AURA_SLOT_RESULT);
	PyModule_AddIntConstant(poModule, "AURA_SLOT_MAX", AURA_SLOT_MAX);
	PyModule_AddIntConstant(poModule, "AURA_WINDOW_TYPE_ABSORB", AURA_WINDOW_TYPE_ABSORB);
	PyModule_AddIntConstant(poModule, "AURA_WINDOW_TYPE_GROWTH", AURA_WINDOW_TYPE_GROWTH);
	PyModule_AddIntConstant(poModule, "AURA_WINDOW_TYPE_EVOLVE", AURA_WINDOW_TYPE_EVOLVE);
	PyModule_AddIntConstant(poModule, "AURA_WINDOW_TYPE_MAX", AURA_WINDOW_TYPE_MAX);
	PyModule_AddIntConstant(poModule, "AURA_REFINE_INFO_SLOT_CURRENT", AURA_REFINE_INFO_SLOT_CURRENT);
	PyModule_AddIntConstant(poModule, "AURA_REFINE_INFO_SLOT_NEXT", AURA_REFINE_INFO_SLOT_NEXT);
	PyModule_AddIntConstant(poModule, "AURA_REFINE_INFO_SLOT_EVOLVED", AURA_REFINE_INFO_SLOT_EVOLVED);
	PyModule_AddIntConstant(poModule, "AURA_REFINE_INFO_STEP", AURA_REFINE_INFO_STEP);
	PyModule_AddIntConstant(poModule, "AURA_REFINE_INFO_LEVEL_MIN", AURA_REFINE_INFO_LEVEL_MIN);
	PyModule_AddIntConstant(poModule, "AURA_REFINE_INFO_LEVEL_MAX", AURA_REFINE_INFO_LEVEL_MAX);
	PyModule_AddIntConstant(poModule, "AURA_REFINE_INFO_NEED_EXP", AURA_REFINE_INFO_NEED_EXP);
	PyModule_AddIntConstant(poModule, "AURA_REFINE_INFO_MATERIAL_VNUM", AURA_REFINE_INFO_MATERIAL_VNUM);
	PyModule_AddIntConstant(poModule, "AURA_REFINE_INFO_MATERIAL_COUNT", AURA_REFINE_INFO_MATERIAL_COUNT);
	PyModule_AddIntConstant(poModule, "AURA_REFINE_INFO_NEED_GOLD", AURA_REFINE_INFO_NEED_GOLD);
	PyModule_AddIntConstant(poModule, "AURA_REFINE_INFO_EVOLVE_PCT", AURA_REFINE_INFO_EVOLVE_PCT);
	PyModule_AddIntConstant(poModule, "ITEM_SOCKET_AURA_DRAIN_ITEM_VNUM", CPythonPlayer::ITEM_SOCKET_AURA_DRAIN_ITEM_VNUM);
	PyModule_AddIntConstant(poModule, "ITEM_SOCKET_AURA_CURRENT_LEVEL", CPythonPlayer::ITEM_SOCKET_AURA_CURRENT_LEVEL);
	PyModule_AddIntConstant(poModule, "ITEM_SOCKET_AURA_BOOST", CPythonPlayer::ITEM_SOCKET_AURA_BOOST);
	PyModule_AddIntConstant(poModule, "ITEM_VALUE_AURA_MATERIAL_EXP", CPythonPlayer::ITEM_VALUE_AURA_MATERIAL_EXP);
	PyModule_AddIntConstant(poModule, "ITEM_VALUE_AURA_BOOST_PERCENT", CPythonPlayer::ITEM_VALUE_AURA_BOOST_PERCENT);
	PyModule_AddIntConstant(poModule, "ITEM_VALUE_AURA_BOOST_TIME", CPythonPlayer::ITEM_VALUE_AURA_BOOST_TIME);
	PyModule_AddIntConstant(poModule, "ITEM_VALUE_AURA_BOOST_UNLIMITED", CPythonPlayer::ITEM_VALUE_AURA_BOOST_UNLIMITED);
#endif

#ifdef ENABLE_CUBE_RENEWAL
	PyModule_AddIntConstant(poModule, "CUBE_ARMOR",					CUBE_ARMOR);
	PyModule_AddIntConstant(poModule, "CUBE_WEAPON",				CUBE_WEAPON);
	PyModule_AddIntConstant(poModule, "CUBE_ACCESSORY",				CUBE_ACCESSORY);
	PyModule_AddIntConstant(poModule, "CUBE_BELT",					CUBE_BELT);
	PyModule_AddIntConstant(poModule, "CUBE_EVENT",					CUBE_EVENT);
	PyModule_AddIntConstant(poModule, "CUBE_ETC",					CUBE_ETC);
	PyModule_AddIntConstant(poModule, "CUBE_JOB",					CUBE_JOB);
	PyModule_AddIntConstant(poModule, "CUBE_SETADD_WEAPON",			CUBE_SETADD_WEAPON);
	PyModule_AddIntConstant(poModule, "CUBE_SETADD_ARMOR_BODY",		CUBE_SETADD_ARMOR_BODY);
	PyModule_AddIntConstant(poModule, "CUBE_SETADD_ARMOR_HELMET",	CUBE_SETADD_ARMOR_HELMET);
	PyModule_AddIntConstant(poModule, "CUBE_PET",					CUBE_PET);
	PyModule_AddIntConstant(poModule, "CUBE_SKILL_BOOK",			CUBE_SKILL_BOOK);
	PyModule_AddIntConstant(poModule, "CUBE_ARMOR_GLOVE",			CUBE_ARMOR_GLOVE);
	PyModule_AddIntConstant(poModule, "CUBE_CATEGORY_MAX",			CUBE_CATEGORY_MAX);
#endif

#ifdef ENABLE_SET_ITEM
	PyModule_AddIntConstant(poModule, "SET_ITEM_SET_VALUE_NONE",	SET_ITEM_SET_VALUE_NONE);
	PyModule_AddIntConstant(poModule, "SET_ITEM_SET_VALUE_1",		SET_ITEM_SET_VALUE_1);
	PyModule_AddIntConstant(poModule, "SET_ITEM_SET_VALUE_2",		SET_ITEM_SET_VALUE_2);
	PyModule_AddIntConstant(poModule, "SET_ITEM_SET_VALUE_3",		SET_ITEM_SET_VALUE_3);
	PyModule_AddIntConstant(poModule, "SET_ITEM_SET_VALUE_4",		SET_ITEM_SET_VALUE_4);
	PyModule_AddIntConstant(poModule, "SET_ITEM_SET_VALUE_5",		SET_ITEM_SET_VALUE_5);
	PyModule_AddIntConstant(poModule, "SET_ITEM_SET_VALUE_MAX",		SET_ITEM_SET_VALUE_MAX);
#endif

#ifdef ENABLE_INGAME_WIKI
	PyModule_AddIntConstant(poModule, "MAIN_RACE_WARRIOR_M",		MAIN_RACE_WARRIOR_M);
	PyModule_AddIntConstant(poModule, "MAIN_RACE_ASSASSIN_W",		MAIN_RACE_ASSASSIN_W);
	PyModule_AddIntConstant(poModule, "MAIN_RACE_SURA_M",			MAIN_RACE_SURA_M);
	PyModule_AddIntConstant(poModule, "MAIN_RACE_SHAMAN_W",			MAIN_RACE_SHAMAN_W);
	PyModule_AddIntConstant(poModule, "MAIN_RACE_WARRIOR_W",		MAIN_RACE_WARRIOR_W);
	PyModule_AddIntConstant(poModule, "MAIN_RACE_ASSASSIN_M",		MAIN_RACE_ASSASSIN_M);
	PyModule_AddIntConstant(poModule, "MAIN_RACE_SURA_W",			MAIN_RACE_SURA_W);
	PyModule_AddIntConstant(poModule, "MAIN_RACE_SHAMAN_M",			MAIN_RACE_SHAMAN_M);
#ifdef ENABLE_WOLFMAN_CHARACTER
	PyModule_AddIntConstant(poModule, "MAIN_RACE_WOLFMAN_M",		MAIN_RACE_WOLFMAN_M);
#endif
	PyModule_AddIntConstant(poModule, "MAIN_RACE_MAX_NUM",			MAIN_RACE_MAX_NUM);
#endif

	PyModule_AddIntConstant(poModule, "JOB_WARRIOR",				0/*JOB_WARRIOR*/);
	PyModule_AddIntConstant(poModule, "JOB_ASSASSIN",				1/*JOB_ASSASSIN*/);
	PyModule_AddIntConstant(poModule, "JOB_SURA",					2/*JOB_SURA*/);
	PyModule_AddIntConstant(poModule, "JOB_SHAMAN",					3/*JOB_SHAMAN*/);
	PyModule_AddIntConstant(poModule, "JOB_WOLFMAN",				4/*JOB_WOLFMAN*/);

#ifdef ENABLE_AUTO_SYSTEM
	PyModule_AddIntConstant(poModule, "AUTO_SKILL_SLOT_MAX", AUTO_SKILL_SLOT_MAX);
	PyModule_AddIntConstant(poModule, "AUTO_POSITINO_SLOT_START", AUTO_SKILL_SLOT_MAX);
	PyModule_AddIntConstant(poModule, "AUTO_POSITINO_SLOT_MAX", AUTO_POSITINO_SLOT_MAX);
#endif

#ifdef ENABLE_PASSIVE_SYSTEM
	PyModule_AddIntConstant(poModule, "PASSIVE_ATTR_SLOT_INDEX_JOB", PASSIVE_ATTR_SLOT_INDEX_JOB);
	PyModule_AddIntConstant(poModule, "PASSIVE_ATTR_SLOT_INDEX_WEAPON", PASSIVE_ATTR_SLOT_INDEX_WEAPON);
	PyModule_AddIntConstant(poModule, "PASSIVE_ATTR_SLOT_INDEX_ELEMENT", PASSIVE_ATTR_SLOT_INDEX_ELEMENT);
	PyModule_AddIntConstant(poModule, "PASSIVE_ATTR_SLOT_INDEX_ARMOR", PASSIVE_ATTR_SLOT_INDEX_ARMOR);
	PyModule_AddIntConstant(poModule, "PASSIVE_ATTR_SLOT_INDEX_ACCE", PASSIVE_ATTR_SLOT_INDEX_ACCE);
	PyModule_AddIntConstant(poModule, "PASSIVE_ATTR_SLOT_INDEX_MAX", PASSIVE_ATTR_SLOT_INDEX_MAX);
#endif

#ifdef ENABLE_KEYCHANGE_SYSTEM
	PyModule_AddIntConstant(poModule, "KEY_MOVE_UP_1", KEY_MOVE_UP_1);
	PyModule_AddIntConstant(poModule, "KEY_MOVE_DOWN_1", KEY_MOVE_DOWN_1);
	PyModule_AddIntConstant(poModule, "KEY_MOVE_LEFT_1", KEY_MOVE_LEFT_1);
	PyModule_AddIntConstant(poModule, "KEY_MOVE_RIGHT_1", KEY_MOVE_RIGHT_1);
	PyModule_AddIntConstant(poModule, "KEY_MOVE_UP_2", KEY_MOVE_UP_2);
	PyModule_AddIntConstant(poModule, "KEY_MOVE_DOWN_2", KEY_MOVE_DOWN_2);
	PyModule_AddIntConstant(poModule, "KEY_MOVE_LEFT_2", KEY_MOVE_LEFT_2);
	PyModule_AddIntConstant(poModule, "KEY_MOVE_RIGHT_2", KEY_MOVE_RIGHT_2);
	PyModule_AddIntConstant(poModule, "KEY_CAMERA_ROTATE_POSITIVE_1", KEY_CAMERA_ROTATE_POSITIVE_1);
	PyModule_AddIntConstant(poModule, "KEY_CAMERA_ROTATE_NEGATIVE_1", KEY_CAMERA_ROTATE_NEGATIVE_1);
	PyModule_AddIntConstant(poModule, "KEY_CAMERA_ZOOM_POSITIVE_1", KEY_CAMERA_ZOOM_POSITIVE_1);
	PyModule_AddIntConstant(poModule, "KEY_CAMERA_ZOOM_NEGATIVE_1", KEY_CAMERA_ZOOM_NEGATIVE_1);
	PyModule_AddIntConstant(poModule, "KEY_CAMERA_PITCH_POSITIVE_1", KEY_CAMERA_PITCH_POSITIVE_1);
	PyModule_AddIntConstant(poModule, "KEY_CAMERA_PITCH_NEGATIVE_1", KEY_CAMERA_PITCH_NEGATIVE_1);
	PyModule_AddIntConstant(poModule, "KEY_CAMERA_ROTATE_POSITIVE_2", KEY_CAMERA_ROTATE_POSITIVE_2);
	PyModule_AddIntConstant(poModule, "KEY_CAMERA_ROTATE_NEGATIVE_2", KEY_CAMERA_ROTATE_NEGATIVE_2);
	PyModule_AddIntConstant(poModule, "KEY_CAMERA_ZOOM_POSITIVE_2", KEY_CAMERA_ZOOM_POSITIVE_2);
	PyModule_AddIntConstant(poModule, "KEY_CAMERA_ZOOM_NEGATIVE_2", KEY_CAMERA_ZOOM_NEGATIVE_2);
	PyModule_AddIntConstant(poModule, "KEY_CAMERA_PITCH_POSITIVE_2", KEY_CAMERA_PITCH_POSITIVE_2);
	PyModule_AddIntConstant(poModule, "KEY_CAMERA_PITCH_NEGATIVE_2", KEY_CAMERA_PITCH_NEGATIVE_2);
	PyModule_AddIntConstant(poModule, "KEY_ROOTING_1", KEY_ROOTING_1);
	PyModule_AddIntConstant(poModule, "KEY_ROOTING_2", KEY_ROOTING_2);
	PyModule_AddIntConstant(poModule, "KEY_ATTACK", KEY_ATTACK);
	PyModule_AddIntConstant(poModule, "KEY_RIDEMYHORS", KEY_RIDEMYHORS);
	PyModule_AddIntConstant(poModule, "KEY_FEEDMYHORS", KEY_FEEDMYHORS);
	PyModule_AddIntConstant(poModule, "KEY_BYEMYHORS", KEY_BYEMYHORS);
	PyModule_AddIntConstant(poModule, "KEY_RIDEHORS", KEY_RIDEHORS);
	PyModule_AddIntConstant(poModule, "KEY_EMOTION1", KEY_EMOTION1);
	PyModule_AddIntConstant(poModule, "KEY_EMOTION2", KEY_EMOTION2);
	PyModule_AddIntConstant(poModule, "KEY_EMOTION3", KEY_EMOTION3);
	PyModule_AddIntConstant(poModule, "KEY_EMOTION4", KEY_EMOTION4);
	PyModule_AddIntConstant(poModule, "KEY_EMOTION5", KEY_EMOTION5);
	PyModule_AddIntConstant(poModule, "KEY_EMOTION6", KEY_EMOTION6);
	PyModule_AddIntConstant(poModule, "KEY_EMOTION7", KEY_EMOTION7);
	PyModule_AddIntConstant(poModule, "KEY_EMOTION8", KEY_EMOTION8);
	PyModule_AddIntConstant(poModule, "KEY_EMOTION9", KEY_EMOTION9);
	PyModule_AddIntConstant(poModule, "KEY_SLOT_1", KEY_SLOT_1);
	PyModule_AddIntConstant(poModule, "KEY_SLOT_2", KEY_SLOT_2);
	PyModule_AddIntConstant(poModule, "KEY_SLOT_3", KEY_SLOT_3);
	PyModule_AddIntConstant(poModule, "KEY_SLOT_4", KEY_SLOT_4);
	PyModule_AddIntConstant(poModule, "KEY_SLOT_5", KEY_SLOT_5);
	PyModule_AddIntConstant(poModule, "KEY_SLOT_6", KEY_SLOT_6);
	PyModule_AddIntConstant(poModule, "KEY_SLOT_7", KEY_SLOT_7);
	PyModule_AddIntConstant(poModule, "KEY_SLOT_8", KEY_SLOT_8);
	PyModule_AddIntConstant(poModule, "KEY_SLOT_CHANGE_1", KEY_SLOT_CHANGE_1);
	PyModule_AddIntConstant(poModule, "KEY_SLOT_CHANGE_2", KEY_SLOT_CHANGE_2);
	PyModule_AddIntConstant(poModule, "KEY_SLOT_CHANGE_3", KEY_SLOT_CHANGE_3);
	PyModule_AddIntConstant(poModule, "KEY_SLOT_CHANGE_4", KEY_SLOT_CHANGE_4);
	PyModule_AddIntConstant(poModule, "KEY_OPEN_STATE", KEY_OPEN_STATE);
	PyModule_AddIntConstant(poModule, "KEY_OPEN_SKILL", KEY_OPEN_SKILL);
	PyModule_AddIntConstant(poModule, "KEY_OPEN_QUEST", KEY_OPEN_QUEST);
	PyModule_AddIntConstant(poModule, "KEY_OPEN_INVENTORY", KEY_OPEN_INVENTORY);
	PyModule_AddIntConstant(poModule, "KEY_OPEN_DDS", KEY_OPEN_DDS);
	PyModule_AddIntConstant(poModule, "KEY_OPEN_MINIMAP", KEY_OPEN_MINIMAP);
	PyModule_AddIntConstant(poModule, "KEY_OPEN_LOGCHAT", KEY_OPEN_LOGCHAT);
	PyModule_AddIntConstant(poModule, "KEY_OPEN_PET", KEY_OPEN_PET);
	PyModule_AddIntConstant(poModule, "KEY_OPEN_GUILD", KEY_OPEN_GUILD);
	PyModule_AddIntConstant(poModule, "KEY_OPEN_MESSENGER", KEY_OPEN_MESSENGER);
	PyModule_AddIntConstant(poModule, "KEY_OPEN_HELP", KEY_OPEN_HELP);
	PyModule_AddIntConstant(poModule, "KEY_OPEN_ACTION", KEY_OPEN_ACTION);
	PyModule_AddIntConstant(poModule, "KEY_SCROLL_ONOFF", KEY_SCROLL_ONOFF);
	PyModule_AddIntConstant(poModule, "KEY_PLUS_MINIMAP", KEY_PLUS_MINIMAP);
	PyModule_AddIntConstant(poModule, "KEY_MIN_MINIMAP", KEY_MIN_MINIMAP);
	PyModule_AddIntConstant(poModule, "KEY_SCREENSHOT", KEY_SCREENSHOT);
	PyModule_AddIntConstant(poModule, "KEY_SHOW_NAME", KEY_SHOW_NAME);
	PyModule_AddIntConstant(poModule, "KEY_OPEN_AUTO", KEY_OPEN_AUTO);
	PyModule_AddIntConstant(poModule, "KEY_AUTO_RUN", KEY_AUTO_RUN);
	PyModule_AddIntConstant(poModule, "KEY_NEXT_TARGET", KEY_NEXT_TARGET);
	PyModule_AddIntConstant(poModule, "KEY_MONSTER_CARD", KEY_MONSTER_CARD);
	PyModule_AddIntConstant(poModule, "KEY_PARTY_MATCH", KEY_PARTY_MATCH);
	PyModule_AddIntConstant(poModule, "KEY_SELECT_DSS_1", KEY_SELECT_DSS_1);
	PyModule_AddIntConstant(poModule, "KEY_SELECT_DSS_2", KEY_SELECT_DSS_2);
	PyModule_AddIntConstant(poModule, "KEY_PASSIVE_ATTR1", KEY_PASSIVE_ATTR1);
	PyModule_AddIntConstant(poModule, "KEY_PASSIVE_ATTR2", KEY_PASSIVE_ATTR2);

	PyModule_AddIntConstant(poModule, "KEY_ADDKEYBUFFERCONTROL", KEY_ADDKEYBUFFERCONTROL);
	PyModule_AddIntConstant(poModule, "KEY_ADDKEYBUFFERALT", KEY_ADDKEYBUFFERALT);
	PyModule_AddIntConstant(poModule, "KEY_ADDKEYBUFFERSHIFT", KEY_ADDKEYBUFFERSHIFT);
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	PyModule_AddIntConstant(poModule, "TRANSMUTATION_TYPE_MOUNT", static_cast<uint8_t>(ETRANSMUTATIONTYPE::TRANSMUTATION_TYPE_MOUNT));
	PyModule_AddIntConstant(poModule, "TRANSMUTATION_TYPE_ITEM", static_cast<uint8_t>(ETRANSMUTATIONTYPE::TRANSMUTATION_TYPE_ITEM));
#endif

#ifdef ENABLE_PARTY_MATCH
	PyModule_AddIntConstant(poModule, "PARTY_MATCH_SEARCH", CPythonPlayer::PARTY_MATCH_SEARCH);
	PyModule_AddIntConstant(poModule, "PARTY_MATCH_CANCEL", CPythonPlayer::PARTY_MATCH_CANCEL);
	PyModule_AddIntConstant(poModule, "PARTY_MATCH_INFO", CPythonPlayer::PARTY_MATCH_INFO);
	//PyModule_AddIntConstant(poModule, "PARTY_MATCH_INFO_MEBMER", CPythonPlayer::PARTY_MATCH_INFO_MEBMER);

	PyModule_AddIntConstant(poModule, "PARTY_MATCH_REQUIRED_ITEM_MAX", CPythonPlayer::PARTY_MATCH_REQUIRED_ITEM_MAX);

	PyModule_AddIntConstant(poModule, "PARTY_MATCH_FAIL", CPythonPlayer::PARTY_MATCH_FAIL);
	PyModule_AddIntConstant(poModule, "PARTY_MATCH_SUCCESS", CPythonPlayer::PARTY_MATCH_SUCCESS);
	PyModule_AddIntConstant(poModule, "PARTY_MATCH_START", CPythonPlayer::PARTY_MATCH_START);
	PyModule_AddIntConstant(poModule, "PARTY_MATCH_CANCEL_SUCCESS", CPythonPlayer::PARTY_MATCH_CANCEL_SUCCESS);
	PyModule_AddIntConstant(poModule, "PARTY_MATCH_FAIL_NO_ITEM", CPythonPlayer::PARTY_MATCH_FAIL_NO_ITEM);
	PyModule_AddIntConstant(poModule, "PARTY_MATCH_FAIL_LEVEL", CPythonPlayer::PARTY_MATCH_FAIL_LEVEL);
	PyModule_AddIntConstant(poModule, "PARTY_MATCH_FAIL_NOT_LEADER", CPythonPlayer::PARTY_MATCH_FAIL_NOT_LEADER);
	PyModule_AddIntConstant(poModule, "PARTY_MATCH_FAIL_MEMBER_NOT_CONDITION", CPythonPlayer::PARTY_MATCH_FAIL_MEMBER_NOT_CONDITION);
	PyModule_AddIntConstant(poModule, "PARTY_MATCH_FAIL_NONE_MAP_INDEX", CPythonPlayer::PARTY_MATCH_FAIL_NONE_MAP_INDEX);
	PyModule_AddIntConstant(poModule, "PARTY_MATCH_FAIL_IMPOSSIBLE_MAP", CPythonPlayer::PARTY_MATCH_FAIL_IMPOSSIBLE_MAP);
	PyModule_AddIntConstant(poModule, "PARTY_MATCH_HOLD", CPythonPlayer::PARTY_MATCH_HOLD);
	PyModule_AddIntConstant(poModule, "PARTY_MATCH_FAIL_FULL_MEMBER", CPythonPlayer::PARTY_MATCH_FAIL_FULL_MEMBER);
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	PyModule_AddIntConstant(poModule, "ADDITIONAL_EQUIPMENT_1", ADDITIONAL_EQUIPMENT_1);
	PyModule_AddIntConstant(poModule, "SLOT_TYPE_ADDITIONAL_EQUIPMENT_1", SLOT_TYPE_ADDITIONAL_EQUIPMENT_1);
	PyModule_AddIntConstant(poModule, "ADDITIONAL_EQUIPMENT_1_START", c_Additional_Equip_Slot_Start);
	PyModule_AddIntConstant(poModule, "ADDITIONAL_EQUIPMENT_MAX_SLOT", ADDITIONAL_EQUIPMENT_MAX_SLOT);
#endif
}
