#include "StdAfx.h"
#include "PythonCharacterManager.h"
#include "PythonBackground.h"
#include "InstanceBase.h"
#include "../GameLib/RaceManager.h"

//////////////////////////////////////////////////////////////////////////
// RaceData 관련 시작
//////////////////////////////////////////////////////////////////////////

PyObject * chrmgrSetEmpireNameMode(PyObject * poSelf, PyObject * poArgs)
{
	int iEnable;
	if (!PyTuple_GetInteger(poArgs, 0, &iEnable))
		return Py_BadArgument();

	CInstanceBase::SetEmpireNameMode(iEnable ? true : false);
	CPythonCharacterManager::Instance().RefreshAllPCTextTail();

	return Py_BuildNone();
}

PyObject * chrmgrRegisterTitleName(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BadArgument();
	char * szTitleName;
	if (!PyTuple_GetString(poArgs, 1, &szTitleName))
		return Py_BadArgument();

	CInstanceBase::RegisterTitleName(iIndex, szTitleName);
	return Py_BuildNone();
}

PyObject * chrmgrRegisterNameColor(PyObject * poSelf, PyObject * poArgs)
{
	int index;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BadArgument();

	int ir;
	if (!PyTuple_GetInteger(poArgs, 1, &ir))
		return Py_BadArgument();
	int ig;
	if (!PyTuple_GetInteger(poArgs, 2, &ig))
		return Py_BadArgument();
	int ib;
	if (!PyTuple_GetInteger(poArgs, 3, &ib))
		return Py_BadArgument();

	CInstanceBase::RegisterNameColor(index, ir, ig, ib);
	return Py_BuildNone();
}

PyObject * chrmgrRegisterTitleColor(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BadArgument();
	int ir;
	if (!PyTuple_GetInteger(poArgs, 1, &ir))
		return Py_BadArgument();
	int ig;
	if (!PyTuple_GetInteger(poArgs, 2, &ig))
		return Py_BadArgument();
	int ib;
	if (!PyTuple_GetInteger(poArgs, 3, &ib))
		return Py_BadArgument();

	CInstanceBase::RegisterTitleColor(iIndex, ir, ig, ib);
	return Py_BuildNone();
}

PyObject * chrmgrGetPickedVID(PyObject * poSelf, PyObject * poArgs)
{
	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();

	uint32_t dwPickedActorID;
	if (rkChrMgr.OLD_GetPickedInstanceVID(&dwPickedActorID))
		return Py_BuildValue("i", dwPickedActorID);

	return Py_BuildValue("i", -1);
}

PyObject * chrmgrGetVIDInfo(PyObject * poSelf, PyObject * poArgs)
{
	int nVID;
	if (!PyTuple_GetInteger(poArgs, 0, &nVID))
		return Py_BadArgument();

	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();

	char szDetail[256] = "";
	CInstanceBase * pkInstBase = rkChrMgr.GetInstancePtr(nVID);
	if (pkInstBase)
	{
		TPixelPosition kPPosInst;
		pkInstBase->NEW_GetPixelPosition(&kPPosInst);

		LONG xInst = kPPosInst.x;
		LONG yInst = kPPosInst.y;

		CPythonBackground & rkBG = CPythonBackground::Instance();
		rkBG.LocalPositionToGlobalPosition(xInst, yInst);
		sprintf(szDetail, "pos=(%ld, %ld)", xInst, yInst);
	}


	char szInfo[1024];
	sprintf(szInfo, "VID %d (isRegistered=%d, isAlive=%d, isDead=%d) %s", nVID, rkChrMgr.IsRegisteredVID(nVID), rkChrMgr.IsAliveVID(nVID),
			rkChrMgr.IsDeadVID(nVID), szDetail);

	return Py_BuildValue("s", szInfo);
}


PyObject * chrmgrSetPathName(PyObject * poSelf, PyObject * poArgs)
{
	char * szPathName;
	if (!PyTuple_GetString(poArgs, 0, &szPathName))
		return Py_BadArgument();

	CRaceManager::Instance().SetPathName(szPathName);
	return Py_BuildNone();
}

PyObject * chrmgrCreateRace(PyObject * poSelf, PyObject * poArgs)
{
	int iRace;
	if (!PyTuple_GetInteger(poArgs, 0, &iRace))
		return Py_BadArgument();

	CRaceManager::Instance().CreateRace(iRace);
	return Py_BuildNone();
}

PyObject * chrmgrSelectRace(PyObject * poSelf, PyObject * poArgs)
{
	int iRace;
	if (!PyTuple_GetInteger(poArgs, 0, &iRace))
		return Py_BadArgument();

	CRaceManager::Instance().SelectRace(iRace);
	return Py_BuildNone();
}

PyObject * chrmgrRegisterAttachingBoneName(PyObject * poSelf, PyObject * poArgs)
{
	int iPartIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iPartIndex))
		return Py_BadArgument();
	char * szBoneName;
	if (!PyTuple_GetString(poArgs, 1, &szBoneName))
		return Py_BadArgument();

	CRaceData * pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
	if (!pRaceData)
		return Py_BuildException("RaceData has not selected!");

	pRaceData->RegisterAttachingBoneName(iPartIndex, szBoneName);
	return Py_BuildNone();
}

PyObject * chrmgrRegisterMotionMode(PyObject * poSelf, PyObject * poArgs)
{
	int iMotionIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iMotionIndex))
		return Py_BadArgument();

	CRaceData * pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
	if (!pRaceData)
		return Py_BuildException("RaceData has not selected!");

	pRaceData->RegisterMotionMode(iMotionIndex);
	return Py_BuildNone();
}


PyObject * chrmgrSetMotionRandomWeight(PyObject * poSelf, PyObject * poArgs)
{
	int iMode;
	if (!PyTuple_GetInteger(poArgs, 0, &iMode))
		return Py_BadArgument();

	int iMotion;
	if (!PyTuple_GetInteger(poArgs, 1, &iMotion))
		return Py_BadArgument();

	int iSubMotion;
	if (!PyTuple_GetInteger(poArgs, 2, &iSubMotion))
		return Py_BadArgument();

	int iPercentage;
	if (!PyTuple_GetInteger(poArgs, 3, &iPercentage))
		return Py_BadArgument();

	CRaceData * pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
	if (!pRaceData)
		return Py_BuildException("RaceData has not selected!");

	if (!pRaceData->SetMotionRandomWeight(iMode, iMotion, iSubMotion, iPercentage))
		Py_BuildException("Failed to SetMotionRandomWeight");

	return Py_BuildNone();
}

PyObject * chrmgrRegisterNormalAttack(PyObject * poSelf, PyObject * poArgs)
{
	int iMode;
	if (!PyTuple_GetInteger(poArgs, 0, &iMode))
		return Py_BadArgument();

	int iMotion;
	if (!PyTuple_GetInteger(poArgs, 1, &iMotion))
		return Py_BadArgument();

	CRaceData * pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
	if (!pRaceData)
		return Py_BuildException("RaceData has not selected!");

	pRaceData->RegisterNormalAttack(iMode, iMotion);
	return Py_BuildNone();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 없어질 함수들
PyObject * chrmgrReserveComboAttack(PyObject * poSelf, PyObject * poArgs)
{
	int iMode;
	if (!PyTuple_GetInteger(poArgs, 0, &iMode))
		return Py_BadArgument();

	int iCount;
	if (!PyTuple_GetInteger(poArgs, 1, &iCount))
		return Py_BadArgument();

	CRaceData * pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
	if (!pRaceData)
		return Py_BuildException("RaceData has not selected!");

	pRaceData->ReserveComboAttack(iMode, 0, iCount);
	return Py_BuildNone();
}

PyObject * chrmgrRegisterComboAttack(PyObject * poSelf, PyObject * poArgs)
{
	int iMode;
	if (!PyTuple_GetInteger(poArgs, 0, &iMode))
		return Py_BadArgument();

	int iComboIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iComboIndex))
		return Py_BadArgument();

	int iMotionIndex;
	if (!PyTuple_GetInteger(poArgs, 2, &iMotionIndex))
		return Py_BadArgument();

	CRaceData * pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
	if (!pRaceData)
		return Py_BuildException("RaceData has not selected!");

	pRaceData->RegisterComboAttack(iMode, 0, iComboIndex, iMotionIndex);
	return Py_BuildNone();
}
// 없어질 함수들
///////////////////////////////////////////////////////////////////////////////////////////////////

PyObject * chrmgrReserveComboAttackNew(PyObject * poSelf, PyObject * poArgs)
{
	int iMotionMode;
	if (!PyTuple_GetInteger(poArgs, 0, &iMotionMode))
		return Py_BadArgument();

	int iComboType;
	if (!PyTuple_GetInteger(poArgs, 1, &iComboType))
		return Py_BadArgument();

	int iCount;
	if (!PyTuple_GetInteger(poArgs, 2, &iCount))
		return Py_BadArgument();

	CRaceData * pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
	if (!pRaceData)
		return Py_BuildException("RaceData has not selected!");

	pRaceData->ReserveComboAttack(iMotionMode, iComboType, iCount);
	return Py_BuildNone();
}

PyObject * chrmgrRegisterComboAttackNew(PyObject * poSelf, PyObject * poArgs)
{
	int iMotionMode;
	if (!PyTuple_GetInteger(poArgs, 0, &iMotionMode))
		return Py_BadArgument();

	int iComboType;
	if (!PyTuple_GetInteger(poArgs, 1, &iComboType))
		return Py_BadArgument();

	int iComboIndex;
	if (!PyTuple_GetInteger(poArgs, 2, &iComboIndex))
		return Py_BadArgument();

	int iMotionIndex;
	if (!PyTuple_GetInteger(poArgs, 3, &iMotionIndex))
		return Py_BadArgument();

	CRaceData * pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
	if (!pRaceData)
		return Py_BuildException("RaceData has not selected!");

	pRaceData->RegisterComboAttack(iMotionMode, iComboType, iComboIndex, iMotionIndex);
	return Py_BuildNone();
}

PyObject * chrmgrLoadRaceData(PyObject * poSelf, PyObject * poArgs)
{
	char * szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BadArgument();

	CRaceData * pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
	if (!pRaceData)
		return Py_BuildException("RaceData has not selected!");

	const char * c_szFullFileName = CRaceManager::Instance().GetFullPathFileName(szFileName);
	if (!pRaceData->LoadRaceData(c_szFullFileName))
		TraceError("Failed to load race data : %s\n", c_szFullFileName);

	return Py_BuildNone();
}

PyObject * chrmgrLoadLocalRaceData(PyObject * poSelf, PyObject * poArgs)
{
	char * szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BadArgument();

	CRaceData * pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
	if (!pRaceData)
		return Py_BuildException("RaceData has not selected!");

	if (!pRaceData->LoadRaceData(szFileName))
		TraceError("Failed to load race data : %s\n", szFileName);

	return Py_BuildNone();
}

/*
PyObject * chrmgrRegisterMotion(PyObject* poSelf, PyObject* poArgs)
{
	int iMode;
	if (!PyTuple_GetInteger(poArgs, 0, &iMode))
		return Py_BadArgument();

	int iMotion;
	if (!PyTuple_GetInteger(poArgs, 1, &iMotion))
		return Py_BadArgument();

	char* szFileName;
	if (!PyTuple_GetString(poArgs, 2, &szFileName))
		return Py_BadArgument();

	int iWeight = 0;
	PyTuple_GetInteger(poArgs, 3, &iWeight);
	iWeight = MIN(100, iWeight);

	CRaceData * pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
	if (!pRaceData)
		return Py_BuildException("RaceData has not selected!");

	pRaceData->NEW_RegisterMotion(iMode, iMotion, CRaceManager::Instance().GetFullPathFileName(szFileName), iWeight);
	return Py_BuildNone();
}
*/

PyObject * chrmgrRegisterCacheMotionData(PyObject * poSelf, PyObject * poArgs)
{
	int iMode;
	if (!PyTuple_GetInteger(poArgs, 0, &iMode))
		return Py_BadArgument();

	int iMotion;
	if (!PyTuple_GetInteger(poArgs, 1, &iMotion))
		return Py_BadArgument();

	char * szFileName;
	if (!PyTuple_GetString(poArgs, 2, &szFileName))
		return Py_BadArgument();

	int iWeight = 0;
	PyTuple_GetInteger(poArgs, 3, &iWeight);
	iWeight = MIN(100, iWeight);

	CRaceData * pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
	if (!pRaceData)
		return Py_BuildException("RaceData has not selected!");

	const char * c_szFullFileName = CRaceManager::Instance().GetFullPathFileName(szFileName);
	CGraphicThing * pkMotionThing = pRaceData->RegisterMotionData(iMode, iMotion, c_szFullFileName, iWeight);

	if (pkMotionThing)
		CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());

	return Py_BuildNone();
}

PyObject * chrmgrRegisterMotionData(PyObject * poSelf, PyObject * poArgs)
{
	int iMode;
	if (!PyTuple_GetInteger(poArgs, 0, &iMode))
		return Py_BadArgument();

	int iMotion;
	if (!PyTuple_GetInteger(poArgs, 1, &iMotion))
		return Py_BadArgument();

	char * szFileName;
	if (!PyTuple_GetString(poArgs, 2, &szFileName))
		return Py_BadArgument();

	int iWeight = 0;
	PyTuple_GetInteger(poArgs, 3, &iWeight);
	iWeight = MIN(100, iWeight);

	CRaceData * pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
	if (!pRaceData)
		return Py_BuildException("RaceData has not selected!");

	const char * c_szFullFileName = CRaceManager::Instance().GetFullPathFileName(szFileName);
	pRaceData->RegisterMotionData(iMode, iMotion, c_szFullFileName, iWeight);

	return Py_BuildNone();
}

PyObject * chrmgrRegisterRaceSrcName(PyObject * poSelf, PyObject * poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BadArgument();

	char * szSrcName;
	if (!PyTuple_GetString(poArgs, 1, &szSrcName))
		return Py_BadArgument();

	CRaceManager::Instance().RegisterRaceSrcName(szName, szSrcName);

	return Py_BuildNone();
}

PyObject * chrmgrRegisterRaceName(PyObject * poSelf, PyObject * poArgs)
{
	int iRaceIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iRaceIndex))
		return Py_BadArgument();

	char * szName;
	if (!PyTuple_GetString(poArgs, 1, &szName))
		return Py_BadArgument();

	CRaceManager::Instance().RegisterRaceName(iRaceIndex, szName);

	return Py_BuildNone();
}

PyObject * chrmgrSetShapeModel(PyObject * poSelf, PyObject * poArgs)
{
	int eShape;
	if (!PyTuple_GetInteger(poArgs, 0, &eShape))
		return Py_BadArgument();

	char * szFileName;
	if (!PyTuple_GetString(poArgs, 1, &szFileName))
		return Py_BadArgument();


	CRaceData * pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
	if (!pRaceData)
		return Py_BuildException("RaceData has not selected!");

	pRaceData->SetShapeModel(eShape, szFileName);
	return Py_BuildNone();
}

PyObject * chrmgrAppendShapeSkin(PyObject * poSelf, PyObject * poArgs)
{
	int eShape;
	if (!PyTuple_GetInteger(poArgs, 0, &eShape))
		return Py_BadArgument();

	int ePart;
	if (!PyTuple_GetInteger(poArgs, 1, &ePart))
		return Py_BadArgument();

	char * szSrcFileName;
	if (!PyTuple_GetString(poArgs, 2, &szSrcFileName))
		return Py_BadArgument();

	char * szDstFileName;
	if (!PyTuple_GetString(poArgs, 3, &szDstFileName))
		return Py_BadArgument();

	CRaceData * pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
	if (!pRaceData)
		return Py_BuildException("RaceData has not selected!");

	pRaceData->AppendShapeSkin(eShape, ePart, szSrcFileName, szDstFileName);
	return Py_BuildNone();
}
//////////////////////////////////////////////////////////////////////////
// RaceData 관련 끝
//////////////////////////////////////////////////////////////////////////

PyObject * chrmgrSetMovingSpeed(PyObject * poSelf, PyObject * poArgs)
{
#ifndef NDEBUG // @warme601 _DISTRIBUTE -> NDEBUG
	int nMovSpd;
	if (!PyTuple_GetInteger(poArgs, 0, &nMovSpd))
		return Py_BadArgument();

	if (nMovSpd < 0)
		return Py_BuildException("MovingSpeed < 0");

	CInstanceBase * pkInst = CPythonCharacterManager::Instance().GetSelectedInstancePtr();

	if (!pkInst)
		return Py_BuildException("MainCharacter has not selected!");

	pkInst->SetMoveSpeed(nMovSpd);
#endif
	return Py_BuildNone();
}

PyObject * chrmgrRegisterEffect(PyObject * poSelf, PyObject * poArgs)
{
	int eEftType;
	if (!PyTuple_GetInteger(poArgs, 0, &eEftType))
		return Py_BadArgument();

	char * szBoneName;
	if (!PyTuple_GetString(poArgs, 1, &szBoneName))
		return Py_BadArgument();

	char * szPathName;
	if (!PyTuple_GetString(poArgs, 2, &szPathName))
		return Py_BadArgument();

	CInstanceBase::RegisterEffect(eEftType, szBoneName, szPathName, false);
	return Py_BuildNone();
}

PyObject * chrmgrRegisterCacheEffect(PyObject * poSelf, PyObject * poArgs)
{
	int eEftType;
	if (!PyTuple_GetInteger(poArgs, 0, &eEftType))
		return Py_BadArgument();

	char * szBoneName;
	if (!PyTuple_GetString(poArgs, 1, &szBoneName))
		return Py_BadArgument();

	char * szPathName;
	if (!PyTuple_GetString(poArgs, 2, &szPathName))
		return Py_BadArgument();

	CInstanceBase::RegisterEffect(eEftType, szBoneName, szPathName, true);

	return Py_BuildNone();
}

PyObject * chrmgrSetDustGap(PyObject * poSelf, PyObject * poArgs)
{
	int nGap;
	if (!PyTuple_GetInteger(poArgs, 0, &nGap))
		return Py_BadArgument();

	CInstanceBase::SetDustGap(nGap);
	return Py_BuildNone();
}

PyObject * chrmgrSetHorseDustGap(PyObject * poSelf, PyObject * poArgs)
{
	int nGap;
	if (!PyTuple_GetInteger(poArgs, 0, &nGap))
		return Py_BadArgument();

	CInstanceBase::SetHorseDustGap(nGap);
	return Py_BuildNone();
}

PyObject * chrmgrToggleDirectionLine(PyObject * poSelf, PyObject * poArgs)
{
	static bool s_isVisible = true;
	CActorInstance::ShowDirectionLine(s_isVisible);

	s_isVisible = !s_isVisible;
	return Py_BuildNone();
}

PyObject * chrmgrRegisterPointEffect(PyObject * poSelf, PyObject * poArgs)
{
	int iEft;
	if (!PyTuple_GetInteger(poArgs, 0, &iEft))
		return Py_BadArgument();

	char * szFileName;
	if (!PyTuple_GetString(poArgs, 1, &szFileName))
		return Py_BadArgument();

	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.RegisterPointEffect(iEft, szFileName);
	return Py_BuildNone();
}

PyObject * chrmgrShowPointEffect(PyObject * poSelf, PyObject * poArgs)
{
	int nVID;
	if (!PyTuple_GetInteger(poArgs, 0, &nVID))
		return Py_BadArgument();

	int nEft;
	if (!PyTuple_GetInteger(poArgs, 1, &nEft))
		return Py_BadArgument();

	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.ShowPointEffect(nEft, nVID >= 0 ? nVID : 0xffffffff);
	return Py_BuildNone();
}


void CPythonCharacterManager::SCRIPT_SetAffect(uint32_t dwVID, uint32_t eState, BOOL isVisible)
{
	CInstanceBase * pkInstSel = (dwVID == 0xffffffff) ? GetSelectedInstancePtr() : GetInstancePtr(dwVID);
	if (!pkInstSel)
		return;

	pkInstSel->SCRIPT_SetAffect(eState, isVisible ? true : false);
}

PyObject * chrmgrSetAffect(PyObject * poSelf, PyObject * poArgs)
{
	int nVID;
	if (!PyTuple_GetInteger(poArgs, 0, &nVID))
		return Py_BadArgument();

	int nEft;
	if (!PyTuple_GetInteger(poArgs, 1, &nEft))
		return Py_BadArgument();

	int nVisible;
	if (!PyTuple_GetInteger(poArgs, 2, &nVisible))
		return Py_BadArgument();

	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.SCRIPT_SetAffect(nVID >= 0 ? nVID : 0xffffffff, nEft, nVisible);
	return Py_BuildNone();
}

void CPythonCharacterManager::SetEmoticon(uint32_t dwVID, uint32_t eState)
{
	CInstanceBase * pkInstSel = (dwVID == 0xffffffff) ? GetSelectedInstancePtr() : GetInstancePtr(dwVID);
	if (!pkInstSel)
		return;

	pkInstSel->SetEmoticon(eState);
}

bool CPythonCharacterManager::IsPossibleEmoticon(uint32_t dwVID)
{
	CInstanceBase * pkInstSel = (dwVID == 0xffffffff) ? GetSelectedInstancePtr() : GetInstancePtr(dwVID);
	if (!pkInstSel)
		return false;

	return pkInstSel->IsPossibleEmoticon();
}

PyObject * chrmgrSetEmoticon(PyObject * poSelf, PyObject * poArgs)
{
	int nVID;
	if (!PyTuple_GetInteger(poArgs, 0, &nVID))
		return Py_BadArgument();

	int nEft;
	if (!PyTuple_GetInteger(poArgs, 1, &nEft))
		return Py_BadArgument();

	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.SetEmoticon(nVID >= 0 ? nVID : 0xffffffff, nEft);
	return Py_BuildNone();
}

PyObject * chrmgrIsPossibleEmoticon(PyObject * poSelf, PyObject * poArgs)
{
	int nVID;
	if (!PyTuple_GetInteger(poArgs, 0, &nVID))
		return Py_BadArgument();

	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();
	int result = rkChrMgr.IsPossibleEmoticon(nVID >= 0 ? nVID : 0xffffffff);

	return Py_BuildValue("i", result);
}

#ifdef UCAN_ADAM
PyObject * chrmgrUcanAdam(PyObject * poSelf, PyObject * poArgs)
{
	float yukariAssa;
	if (!PyTuple_GetFloat(poArgs, 0, &yukariAssa))
		return Py_BadArgument();

	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	CInstanceBase* pkInsMain = rkChrMgr.GetMainInstancePtr();
	if (pkInsMain)
	{
		pkInsMain->GetGraphicThingInstancePtr()->UcanAdam(yukariAssa);
	}
	return Py_BuildNone();
}
#endif

PyObject * chrmgrSetRaceHeight(PyObject* poSelf, PyObject* poArgs)
{
	int iRaceIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iRaceIndex))
		return Py_BadArgument();
	float fRaceHeight = 0.0f;
	if (!PyTuple_GetFloat(poArgs, 1, &fRaceHeight))
		return Py_BadArgument();

	CRaceManager::Instance().SetRaceHeight(iRaceIndex, fRaceHeight);
	return Py_BuildNone();
}

#ifdef ENABLE_12ZI
PyObject * chrmgrIsDead(PyObject* poSelf, PyObject* poArgs)
{
	int nVID;
	if (!PyTuple_GetInteger(poArgs, 0, &nVID))
		return Py_BadArgument();

	bool result = false;
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	CInstanceBase* pkInstBase = rkChrMgr.GetInstancePtr(nVID);
	if (pkInstBase)
	{
		if (pkInstBase->IsDead())
			result = true;
		else
			result = false;
	}

	return Py_BuildValue("b", result);
}

PyObject * chrmgrIsPc(PyObject* poSelf, PyObject* poArgs)
{
	int nVID;
	if (!PyTuple_GetInteger(poArgs, 0, &nVID))
		return Py_BadArgument();

	bool result = false;
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	CInstanceBase* pkInstBase = rkChrMgr.GetInstancePtr(nVID);
	if (pkInstBase)
	{
		if (pkInstBase->IsPC())
			result = true;
		else
			result = false;
	}

	return Py_BuildValue("b", result);
}
#endif

PyObject* chrmgrHasAffectByVID(PyObject* poSelf, PyObject* poArgs)
{
	int iVID, iAffect;

	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BadArgument();

	if (!PyTuple_GetInteger(poArgs, 1, &iAffect))
		return Py_BadArgument();

	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();

	CInstanceBase* p = rkChrMgr.CPythonCharacterManager::GetInstancePtr(iVID);

	if (p)
		return Py_BuildValue("i", p->IsAffect(iAffect));

	return Py_BuildNone();
}

#ifdef ENABLE_AUTO_SYSTEM
PyObject* chrmgrSetAutoOnOff(PyObject* poSelf, PyObject* poArgs)
{
	bool state;
	if (!PyTuple_GetBoolean(poArgs, 0, &state))
		return Py_BadArgument();

	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.CPythonCharacterManager::SetAutoOnOff(state);
	return Py_BuildNone();
}

PyObject* chrmgrGetAutoOnOff(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonCharacterManager::Instance().GetAutoOnOff());
}
#endif

#ifdef ENABLE_PARTY_MATCH
PyObject* chrmgrSetPartyMatchOff(PyObject* poSelf, PyObject* poArgs)
{
	bool state;
	if (!PyTuple_GetBoolean(poArgs, 0, &state))
		return Py_BadArgument();

	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.CPythonCharacterManager::SetPartyMatchOff(state);
	return Py_BuildNone();
}

PyObject* chrmgrGetPartyMatchOff(PyObject* poSelf, PyObject* poArgs)
{
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	return Py_BuildValue("i", rkChrMgr.CPythonCharacterManager::GetPartyMatchOff());
}
#endif

void initchrmgr()
{
	static PyMethodDef s_methods[] = {// RaceData 관련
									  {"SetEmpireNameMode", chrmgrSetEmpireNameMode, METH_VARARGS},
									  {"GetVIDInfo", chrmgrGetVIDInfo, METH_VARARGS},
									  {"GetPickedVID", chrmgrGetPickedVID, METH_VARARGS},
									  {"SetShapeModel", chrmgrSetShapeModel, METH_VARARGS},
									  {"AppendShapeSkin", chrmgrAppendShapeSkin, METH_VARARGS},
									  {"SetPathName", chrmgrSetPathName, METH_VARARGS},
									  {"LoadRaceData", chrmgrLoadRaceData, METH_VARARGS},
									  {"LoadLocalRaceData", chrmgrLoadLocalRaceData, METH_VARARGS},
									  {"CreateRace", chrmgrCreateRace, METH_VARARGS},
									  {"SelectRace", chrmgrSelectRace, METH_VARARGS},
									  {"RegisterAttachingBoneName", chrmgrRegisterAttachingBoneName, METH_VARARGS},
									  {"RegisterMotionMode", chrmgrRegisterMotionMode, METH_VARARGS},
									  //{ "RegisterMotion",				chrmgrRegisterMotion,					METH_VARARGS },
									  {"SetMotionRandomWeight", chrmgrSetMotionRandomWeight, METH_VARARGS},
									  {"RegisterNormalAttack", chrmgrRegisterNormalAttack, METH_VARARGS},
									  {"ReserveComboAttack", chrmgrReserveComboAttack, METH_VARARGS},
									  {"RegisterComboAttack", chrmgrRegisterComboAttack, METH_VARARGS},
									  {"ReserveComboAttackNew", chrmgrReserveComboAttackNew, METH_VARARGS},
									  {"RegisterComboAttackNew", chrmgrRegisterComboAttackNew, METH_VARARGS},
#ifdef UCAN_ADAM
									  {"UcanAdam", chrmgrUcanAdam, METH_VARARGS},
#endif
									  {"RegisterMotionData", chrmgrRegisterMotionData, METH_VARARGS},
									  {"RegisterRaceName", chrmgrRegisterRaceName, METH_VARARGS},
									  {"RegisterRaceSrcName", chrmgrRegisterRaceSrcName, METH_VARARGS},
									  {"RegisterCacheMotionData", chrmgrRegisterCacheMotionData, METH_VARARGS},

									  // ETC
									  {"SetAffect", chrmgrSetAffect, METH_VARARGS},
									  {"SetEmoticon", chrmgrSetEmoticon, METH_VARARGS},
									  {"IsPossibleEmoticon", chrmgrIsPossibleEmoticon, METH_VARARGS},
									  {"RegisterEffect", chrmgrRegisterEffect, METH_VARARGS},
									  {"RegisterCacheEffect", chrmgrRegisterCacheEffect, METH_VARARGS},
									  {"RegisterPointEffect", chrmgrRegisterPointEffect, METH_VARARGS},
									  {"ShowPointEffect", chrmgrShowPointEffect, METH_VARARGS},
									  {"ToggleDirectionLine", chrmgrToggleDirectionLine, METH_VARARGS},
									  {"SetMovingSpeed", chrmgrSetMovingSpeed, METH_VARARGS},
									  {"SetDustGap", chrmgrSetDustGap, METH_VARARGS},
									  {"SetHorseDustGap", chrmgrSetHorseDustGap, METH_VARARGS},

									  {"RegisterTitleName", chrmgrRegisterTitleName, METH_VARARGS},
									  {"RegisterNameColor", chrmgrRegisterNameColor, METH_VARARGS},
									  {"RegisterTitleColor", chrmgrRegisterTitleColor, METH_VARARGS},
		{ "SetRaceHeight",				chrmgrSetRaceHeight,					METH_VARARGS },
#ifdef ENABLE_12ZI
		{ "IsDead",						chrmgrIsDead,							METH_VARARGS },
		{ "IsPC",						chrmgrIsPc,								METH_VARARGS },
#endif
		{ "HasAffectByVID",				chrmgrHasAffectByVID,					METH_VARARGS },

#ifdef ENABLE_AUTO_SYSTEM
	{ "SetAutoOnOff",					chrmgrSetAutoOnOff,						METH_VARARGS },
	{ "GetAutoOnOff",					chrmgrGetAutoOnOff,						METH_VARARGS },
#endif

#ifdef ENABLE_PARTY_MATCH
	{ "SetPartyMatchOff",				chrmgrSetPartyMatchOff,					METH_VARARGS },
	{ "GetPartyMatchOff",				chrmgrGetPartyMatchOff,					METH_VARARGS },
#endif

									  {nullptr, nullptr, 0}};

	PyObject * poModule = Py_InitModule("chrmgrm2g", s_methods);	//@fixme430
	// NAMECOLOR
	PyModule_AddIntConstant(poModule, "NAMECOLOR_MOB",			CInstanceBase::NAMECOLOR_NORMAL_MOB);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_NPC",			CInstanceBase::NAMECOLOR_NORMAL_NPC);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_PC",			CInstanceBase::NAMECOLOR_NORMAL_PC);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_EMPIRE_MOB",	CInstanceBase::NAMECOLOR_EMPIRE_MOB);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_EMPIRE_NPC",	CInstanceBase::NAMECOLOR_EMPIRE_NPC);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_EMPIRE_PC",	CInstanceBase::NAMECOLOR_EMPIRE_PC);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_FUNC",			CInstanceBase::NAMECOLOR_FUNC);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_PK",			CInstanceBase::NAMECOLOR_PK);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_PVP",			CInstanceBase::NAMECOLOR_PVP);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_PARTY",		CInstanceBase::NAMECOLOR_PARTY);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_WARP",			CInstanceBase::NAMECOLOR_WARP);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_WAYPOINT",		CInstanceBase::NAMECOLOR_WAYPOINT);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_EXTRA",		CInstanceBase::NAMECOLOR_EXTRA);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_METIN",		CInstanceBase::NAMECOLOR_METIN);
	PyModule_AddIntConstant(poModule, "NAMECOLOR_BOSS",			CInstanceBase::NAMECOLOR_BOSS);
#ifdef ENABLE_BATTLE_FIELD
	PyModule_AddIntConstant(poModule, "NAMECOLOR_TARGET_VICTIM",CInstanceBase::NAMECOLOR_TARGET_VICTIM);
#endif
	// END_NAMECOLOR

	// EFFECT
	PyModule_AddIntConstant(poModule, "EFFECT_DUST",					CInstanceBase::EFFECT_DUST);
	PyModule_AddIntConstant(poModule, "EFFECT_STUN",					CInstanceBase::EFFECT_STUN);
	PyModule_AddIntConstant(poModule, "EFFECT_HIT",						CInstanceBase::EFFECT_HIT);
	PyModule_AddIntConstant(poModule, "EFFECT_FLAME_ATTACK",			CInstanceBase::EFFECT_FLAME_ATTACK);
	PyModule_AddIntConstant(poModule, "EFFECT_FLAME_HIT",				CInstanceBase::EFFECT_FLAME_HIT);
	PyModule_AddIntConstant(poModule, "EFFECT_FLAME_ATTACH",			CInstanceBase::EFFECT_FLAME_ATTACH);
	PyModule_AddIntConstant(poModule, "EFFECT_ELECTRIC_ATTACK",			CInstanceBase::EFFECT_ELECTRIC_ATTACK);
	PyModule_AddIntConstant(poModule, "EFFECT_ELECTRIC_HIT",			CInstanceBase::EFFECT_ELECTRIC_HIT);
	PyModule_AddIntConstant(poModule, "EFFECT_ELECTRIC_ATTACH",			CInstanceBase::EFFECT_ELECTRIC_ATTACH);
	PyModule_AddIntConstant(poModule, "EFFECT_SPAWN_APPEAR",			CInstanceBase::EFFECT_SPAWN_APPEAR);
	PyModule_AddIntConstant(poModule, "EFFECT_SPAWN_DISAPPEAR",			CInstanceBase::EFFECT_SPAWN_DISAPPEAR);
	PyModule_AddIntConstant(poModule, "EFFECT_LEVELUP",					CInstanceBase::EFFECT_LEVELUP);
	PyModule_AddIntConstant(poModule, "EFFECT_SKILLUP",					CInstanceBase::EFFECT_SKILLUP);
	PyModule_AddIntConstant(poModule, "EFFECT_HPUP_RED",				CInstanceBase::EFFECT_HPUP_RED);
	PyModule_AddIntConstant(poModule, "EFFECT_SPUP_BLUE",				CInstanceBase::EFFECT_SPUP_BLUE);
	PyModule_AddIntConstant(poModule, "EFFECT_SPEEDUP_GREEN",			CInstanceBase::EFFECT_SPEEDUP_GREEN);
	PyModule_AddIntConstant(poModule, "EFFECT_DXUP_PURPLE",				CInstanceBase::EFFECT_DXUP_PURPLE);
	PyModule_AddIntConstant(poModule, "EFFECT_CRITICAL",				CInstanceBase::EFFECT_CRITICAL);
	PyModule_AddIntConstant(poModule, "EFFECT_PENETRATE",				CInstanceBase::EFFECT_PENETRATE);
	PyModule_AddIntConstant(poModule, "EFFECT_BLOCK",					CInstanceBase::EFFECT_BLOCK);
	PyModule_AddIntConstant(poModule, "EFFECT_DODGE",					CInstanceBase::EFFECT_DODGE);
	PyModule_AddIntConstant(poModule, "EFFECT_FIRECRACKER",				CInstanceBase::EFFECT_FIRECRACKER);
	PyModule_AddIntConstant(poModule, "EFFECT_SPIN_TOP",				CInstanceBase::EFFECT_SPIN_TOP);
	PyModule_AddIntConstant(poModule, "EFFECT_WEAPON",					CInstanceBase::EFFECT_WEAPON);
	PyModule_AddIntConstant(poModule, "EFFECT_AFFECT",					CInstanceBase::EFFECT_AFFECT);
	PyModule_AddIntConstant(poModule, "EFFECT_EMOTICON",				CInstanceBase::EFFECT_EMOTICON);
	PyModule_AddIntConstant(poModule, "EFFECT_SELECT",					CInstanceBase::EFFECT_SELECT);
	PyModule_AddIntConstant(poModule, "EFFECT_TARGET",					CInstanceBase::EFFECT_TARGET);
	PyModule_AddIntConstant(poModule, "EFFECT_EMPIRE",					CInstanceBase::EFFECT_EMPIRE);
	PyModule_AddIntConstant(poModule, "EFFECT_HORSE_DUST",				CInstanceBase::EFFECT_HORSE_DUST);
	PyModule_AddIntConstant(poModule, "EFFECT_REFINED",					CInstanceBase::EFFECT_REFINED);
#ifdef ENABLE_TARGET_SELECT_COLOR
	PyModule_AddIntConstant(poModule, "EFFECT_MONSTER",					CInstanceBase::EFFECT_MONSTER);
	PyModule_AddIntConstant(poModule, "EFFECT_TARGET_MONSTER",			CInstanceBase::EFFECT_TARGET_MONSTER);
	PyModule_AddIntConstant(poModule, "EFFECT_SHINSOO",					CInstanceBase::EFFECT_SHINSOO);
	PyModule_AddIntConstant(poModule, "EFFECT_TARGET_SHINSOO",			CInstanceBase::EFFECT_TARGET_SHINSOO);
	PyModule_AddIntConstant(poModule, "EFFECT_CHUNJO",					CInstanceBase::EFFECT_CHUNJO);
	PyModule_AddIntConstant(poModule, "EFFECT_TARGET_CHUNJO",			CInstanceBase::EFFECT_TARGET_CHUNJO);
	PyModule_AddIntConstant(poModule, "EFFECT_JINNOS",					CInstanceBase::EFFECT_JINNOS);
	PyModule_AddIntConstant(poModule, "EFFECT_TARGET_JINNOS",			CInstanceBase::EFFECT_TARGET_JINNOS);
#endif
	PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_TARGET",			CInstanceBase::EFFECT_DAMAGE_TARGET);
	PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_NOT_TARGET",		CInstanceBase::EFFECT_DAMAGE_NOT_TARGET);
	PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_SELFDAMAGE",		CInstanceBase::EFFECT_DAMAGE_SELFDAMAGE);
	PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_SELFDAMAGE2",		CInstanceBase::EFFECT_DAMAGE_SELFDAMAGE2);
	PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_POISON",			CInstanceBase::EFFECT_DAMAGE_POISON);
	PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_MISS",				CInstanceBase::EFFECT_DAMAGE_MISS);
	PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_TARGETMISS",		CInstanceBase::EFFECT_DAMAGE_TARGETMISS);
	PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_CRITICAL",			CInstanceBase::EFFECT_DAMAGE_CRITICAL);
	PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_BLEEDING",			CInstanceBase::EFFECT_DAMAGE_BLEEDING);
	PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_FIRE",				CInstanceBase::EFFECT_DAMAGE_FIRE);
	PyModule_AddIntConstant(poModule, "EFFECT_SUCCESS",					CInstanceBase::EFFECT_SUCCESS) ;
	PyModule_AddIntConstant(poModule, "EFFECT_FAIL",					CInstanceBase::EFFECT_FAIL) ;
	PyModule_AddIntConstant(poModule, "EFFECT_FR_SUCCESS",				CInstanceBase::EFFECT_FR_SUCCESS) ;
	PyModule_AddIntConstant(poModule, "EFFECT_LEVELUP_ON_14_FOR_GERMANY", CInstanceBase::EFFECT_LEVELUP_ON_14_FOR_GERMANY );
	PyModule_AddIntConstant(poModule, "EFFECT_LEVELUP_UNDER_15_FOR_GERMANY", CInstanceBase::EFFECT_LEVELUP_UNDER_15_FOR_GERMANY );
	PyModule_AddIntConstant(poModule, "EFFECT_PERCENT_DAMAGE1",			CInstanceBase::EFFECT_PERCENT_DAMAGE1);
	PyModule_AddIntConstant(poModule, "EFFECT_PERCENT_DAMAGE2",			CInstanceBase::EFFECT_PERCENT_DAMAGE2);
	PyModule_AddIntConstant(poModule, "EFFECT_PERCENT_DAMAGE3",			CInstanceBase::EFFECT_PERCENT_DAMAGE3);
	PyModule_AddIntConstant(poModule, "EFFECT_AUTO_HPUP",				CInstanceBase::EFFECT_AUTO_HPUP);
	PyModule_AddIntConstant(poModule, "EFFECT_AUTO_SPUP",				CInstanceBase::EFFECT_AUTO_SPUP);
	PyModule_AddIntConstant(poModule, "EFFECT_RAMADAN_RING_EQUIP",		CInstanceBase::EFFECT_RAMADAN_RING_EQUIP);
	PyModule_AddIntConstant(poModule, "EFFECT_HALLOWEEN_CANDY_EQUIP",	CInstanceBase::EFFECT_HALLOWEEN_CANDY_EQUIP);
	PyModule_AddIntConstant(poModule, "EFFECT_HAPPINESS_RING_EQUIP",	CInstanceBase::EFFECT_HAPPINESS_RING_EQUIP);
	PyModule_AddIntConstant(poModule, "EFFECT_LOVE_PENDANT_EQUIP",		CInstanceBase::EFFECT_LOVE_PENDANT_EQUIP);
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "EFFECT_ACCE_SUCESS_ABSORB",		CInstanceBase::EFFECT_ACCE_SUCESS_ABSORB);
	PyModule_AddIntConstant(poModule, "EFFECT_ACCE_EQUIP",				CInstanceBase::EFFECT_ACCE_EQUIP);
	PyModule_AddIntConstant(poModule, "EFFECT_ACCE_BACK",				CInstanceBase::EFFECT_ACCE_BACK);
#endif
#ifdef ENABLE_REFINE_ELEMENT
	PyModule_AddIntConstant(poModule, "EFFECT_ELEMENTS_WEAPON",			CInstanceBase::EFFECT_ELEMENTS_WEAPON);
#endif
	// EFFECT_TEMP
	PyModule_AddIntConstant(poModule, "EFFECT_EQUIP_BOOTS",				CInstanceBase::EFFECT_EQUIP_BOOTS);
	PyModule_AddIntConstant(poModule, "EFFECT_EQUIP_HEROMEDAL",			CInstanceBase::EFFECT_EQUIP_HEROMEDAL);
	PyModule_AddIntConstant(poModule, "EFFECT_EQUIP_CHOCOLATE_AMULET",	CInstanceBase::EFFECT_EQUIP_CHOCOLATE_AMULET);
	PyModule_AddIntConstant(poModule, "EFFECT_EQUIP_EMOTION_MASK",		CInstanceBase::EFFECT_EQUIP_EMOTION_MASK);
	PyModule_AddIntConstant(poModule, "EFFECT_WILLPOWER_RING",			CInstanceBase::EFFECT_WILLPOWER_RING);
	PyModule_AddIntConstant(poModule, "EFFECT_DEADLYPOWER_RING",		CInstanceBase::EFFECT_DEADLYPOWER_RING);
	PyModule_AddIntConstant(poModule, "EFFECT_EASTER_CANDY_EQIP",		CInstanceBase::EFFECT_EASTER_CANDY_EQIP);
#ifdef ENABLE_DAWNMIST_DUNGEON
	PyModule_AddIntConstant(poModule, "EFFECT_HEAL",					CInstanceBase::EFFECT_HEAL);
#endif
#ifdef ENABLE_ANTI_EXP_RING
	PyModule_AddIntConstant(poModule, "EFFECT_AUTO_EXP",				CInstanceBase::EFFECT_AUTO_EXP);
#endif
	PyModule_AddIntConstant(poModule, "EFFECT_CAPE_OF_COURAGE",			CInstanceBase::EFFECT_CAPE_OF_COURAGE);	//@fixme421
	PyModule_AddIntConstant(poModule, "EFFECT_GYEONGGONG_BOOM",			CInstanceBase::EFFECT_GYEONGGONG_BOOM);
#ifdef ENABLE_12ZI
	PyModule_AddIntConstant(poModule, "EFFECT_SKILL_DAMAGE_ZONE",			CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE);
	PyModule_AddIntConstant(poModule, "EFFECT_SKILL_SAFE_ZONE",				CInstanceBase::EFFECT_SKILL_SAFE_ZONE);

	PyModule_AddIntConstant(poModule, "EFFECT_METEOR",						CInstanceBase::EFFECT_METEOR);
	PyModule_AddIntConstant(poModule, "EFFECT_BEAD_RAIN",					CInstanceBase::EFFECT_BEAD_RAIN);
	PyModule_AddIntConstant(poModule, "EFFECT_FALL_ROCK",					CInstanceBase::EFFECT_FALL_ROCK);
	PyModule_AddIntConstant(poModule, "EFFECT_ARROW_RAIN",					CInstanceBase::EFFECT_ARROW_RAIN);
	PyModule_AddIntConstant(poModule, "EFFECT_HORSE_DROP",					CInstanceBase::EFFECT_HORSE_DROP);
	PyModule_AddIntConstant(poModule, "EFFECT_EGG_DROP",					CInstanceBase::EFFECT_EGG_DROP);
	PyModule_AddIntConstant(poModule, "EFFECT_DEAPO_BOOM",					CInstanceBase::EFFECT_DEAPO_BOOM);

	PyModule_AddIntConstant(poModule, "EFFECT_SKILL_DAMAGE_ZONE_BIG",		CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_BIG);
	PyModule_AddIntConstant(poModule, "EFFECT_SKILL_DAMAGE_ZONE_MIDDLE",	CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_MIDDLE);
	PyModule_AddIntConstant(poModule, "EFFECT_SKILL_DAMAGE_ZONE_SMALL",		CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_SMALL);

	PyModule_AddIntConstant(poModule, "EFFECT_SKILL_SAFE_ZONE_BIG",			CInstanceBase::EFFECT_SKILL_SAFE_ZONE_BIG);
	PyModule_AddIntConstant(poModule, "EFFECT_SKILL_SAFE_ZONE_MIDDLE",		CInstanceBase::EFFECT_SKILL_SAFE_ZONE_MIDDLE);
	PyModule_AddIntConstant(poModule, "EFFECT_SKILL_SAFE_ZONE_SMALL",		CInstanceBase::EFFECT_SKILL_SAFE_ZONE_SMALL);
#endif
#ifdef ENABLE_BATTLE_FIELD
	PyModule_AddIntConstant(poModule, "EFFECT_BATTLE_POTION",				CInstanceBase::EFFECT_BATTLE_POTION);
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	PyModule_AddIntConstant(poModule, "EFFECT_DRAGONLAIR_STONE_UNBEATABLE_1", CInstanceBase::EFFECT_DRAGONLAIR_STONE_UNBEATABLE_1);
	PyModule_AddIntConstant(poModule, "EFFECT_DRAGONLAIR_STONE_UNBEATABLE_2", CInstanceBase::EFFECT_DRAGONLAIR_STONE_UNBEATABLE_2);
	PyModule_AddIntConstant(poModule, "EFFECT_DRAGONLAIR_STONE_UNBEATABLE_3", CInstanceBase::EFFECT_DRAGONLAIR_STONE_UNBEATABLE_3);
#endif
	PyModule_AddIntConstant(poModule, "EFFECT_FEATHER_WALK",				CInstanceBase::EFFECT_FEATHER_WALK);
#ifdef ENABLE_FLOWER_EVENT
	PyModule_AddIntConstant(poModule, "EFFECT_FLOWER_EVENT",				CInstanceBase::EFFECT_FLOWER_EVENT);
#endif
#ifdef ENABLE_GEM_SYSTEM
	PyModule_AddIntConstant(poModule, "EFFECT_GEM_PENDANT",					CInstanceBase::EFFECT_GEM_PENDANT);
#endif
#ifdef ENABLE_QUEEN_NETHIS
	PyModule_AddIntConstant(poModule, "EFFECT_SNAKE_REGEN",					CInstanceBase::EFFECT_SNAKE_REGEN);
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
	PyModule_AddIntConstant(poModule, "EFFECT_BP_NORMAL_MISSION_COMPLETED",		CInstanceBase::EFFECT_BP_NORMAL_MISSION_COMPLETED);
	PyModule_AddIntConstant(poModule, "EFFECT_BP_PREMIUM_MISSION_COMPLETED",	CInstanceBase::EFFECT_BP_PREMIUM_MISSION_COMPLETED);
	PyModule_AddIntConstant(poModule, "EFFECT_BP_EVENT_MISSION_COMPLETED",		CInstanceBase::EFFECT_BP_EVENT_MISSION_COMPLETED);
	PyModule_AddIntConstant(poModule, "EFFECT_BP_NORMAL_BATTLEPASS_COMPLETED",	CInstanceBase::EFFECT_BP_NORMAL_BATTLEPASS_COMPLETED);
	PyModule_AddIntConstant(poModule, "EFFECT_BP_PREMIUM_BATTLEPASS_COMPLETED",	CInstanceBase::EFFECT_BP_PREMIUM_BATTLEPASS_COMPLETED);
	PyModule_AddIntConstant(poModule, "EFFECT_BP_EVENT_BATTLEPASS_COMPLETED",	CInstanceBase::EFFECT_BP_EVENT_BATTLEPASS_COMPLETED);
#endif
	// END_EFFECT
}
