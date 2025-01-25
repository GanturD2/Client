#include "StdAfx.h"
#include "PythonGuild.h"
#include "AbstractPlayer.h"
#include "MarkManager.h"

std::map<uint32_t, uint32_t> g_GuildSkillSlotToIndexMap;

void CPythonGuild::EnableGuild()
{
	m_bGuildEnable = TRUE;
}

void CPythonGuild::SetGuildMoney(uint32_t dwMoney)
{
	m_GuildInfo.dwGuildMoney = dwMoney;
}

void CPythonGuild::SetGuildEXP(uint8_t byLevel, uint32_t dwEXP)
{
	m_GuildInfo.dwGuildLevel = byLevel;
	m_GuildInfo.dwCurrentExperience = dwEXP;
}

void CPythonGuild::SetGradeData(uint8_t byGradeNumber, const TGuildGradeData & rGuildGradeData)
{
	m_GradeDataMap[byGradeNumber] = rGuildGradeData;
}

void CPythonGuild::SetGradeName(uint8_t byGradeNumber, const char * c_szName)
{
	if (!__IsGradeData(byGradeNumber))
		return;

	TGuildGradeData & rGradeData = m_GradeDataMap.find(byGradeNumber)->second;
	rGradeData.strName = c_szName;
}

void CPythonGuild::SetGradeAuthority(uint8_t byGradeNumber, uint8_t byAuthority)
{
	if (!__IsGradeData(byGradeNumber))
		return;

	TGuildGradeData & rGradeData = m_GradeDataMap.find(byGradeNumber)->second;
	rGradeData.byAuthorityFlag = byAuthority;
}

void CPythonGuild::ClearComment()
{
	m_GuildBoardCommentVector.clear();
}

void CPythonGuild::RegisterComment(uint32_t dwCommentID, const char * c_szName, const char * c_szComment)
{
	if (0 == strlen(c_szComment))
		return;

	TGuildBoardCommentData CommentData;
	CommentData.dwCommentID = dwCommentID;
	CommentData.strName = c_szName;
	CommentData.strComment = c_szComment;

	m_GuildBoardCommentVector.emplace_back(CommentData);
}

void CPythonGuild::RegisterMember(TGuildMemberData & rGuildMemberData)
{
	TGuildMemberData * pGuildMemberData;
	if (GetMemberDataPtrByPID(rGuildMemberData.dwPID, &pGuildMemberData))
	{
		pGuildMemberData->byGeneralFlag = rGuildMemberData.byGeneralFlag;
		pGuildMemberData->byGrade = rGuildMemberData.byGrade;
		pGuildMemberData->byLevel = rGuildMemberData.byLevel;
		pGuildMemberData->dwOffer = rGuildMemberData.dwOffer;
	}
	else
		m_GuildMemberDataVector.emplace_back(rGuildMemberData);

	__CalculateLevelAverage();
	__SortMember();
}

struct CPythonGuild_FFindGuildMemberByPID
{
	CPythonGuild_FFindGuildMemberByPID(uint32_t dwSearchingPID_) : dwSearchingPID(dwSearchingPID_) {}
	int operator()(CPythonGuild::TGuildMemberData & rGuildMemberData) const { return rGuildMemberData.dwPID == dwSearchingPID; }

	uint32_t dwSearchingPID;
};

struct CPythonGuild_FFindGuildMemberByName
{
	CPythonGuild_FFindGuildMemberByName(const char * c_szSearchingName) : strSearchingName(c_szSearchingName) {}
	int operator()(CPythonGuild::TGuildMemberData & rGuildMemberData) const { return rGuildMemberData.strName == strSearchingName; }

	std::string strSearchingName;
};

void CPythonGuild::ChangeGuildMemberGrade(uint32_t dwPID, uint8_t byGrade)
{
	TGuildMemberData * pGuildMemberData;
	if (!GetMemberDataPtrByPID(dwPID, &pGuildMemberData))
		return;

	pGuildMemberData->byGrade = byGrade;
}

void CPythonGuild::ChangeGuildMemberGeneralFlag(uint32_t dwPID, uint8_t byFlag)
{
	TGuildMemberData * pGuildMemberData;
	if (!GetMemberDataPtrByPID(dwPID, &pGuildMemberData))
		return;

	pGuildMemberData->byGeneralFlag = byFlag;
}

void CPythonGuild::RemoveMember(uint32_t dwPID)
{
	auto itor = std::find_if(m_GuildMemberDataVector.begin(), m_GuildMemberDataVector.end(), CPythonGuild_FFindGuildMemberByPID(dwPID));

	if (m_GuildMemberDataVector.end() == itor)
		return;

	m_GuildMemberDataVector.erase(itor);
}

#ifdef ENABLE_COLORED_GUILD_RANKS
void CPythonGuild::RegisterGuildName(uint32_t dwID, const char * c_szName, int rank)
{
	GuildNameRank gnr;
	gnr.name = std::string(c_szName);
	gnr.rank = rank;
	m_GuildNameMap.emplace(dwID, gnr);
}
#else
void CPythonGuild::RegisterGuildName(uint32_t dwID, const char * c_szName)
{
	m_GuildNameMap.emplace(dwID, c_szName);
}
#endif

BOOL CPythonGuild::IsMainPlayer(uint32_t dwPID)
{
	TGuildMemberData * pGuildMemberData;
	if (!GetMemberDataPtrByPID(dwPID, &pGuildMemberData))
		return FALSE;

	IAbstractPlayer & rPlayer = IAbstractPlayer::GetSingleton();
	return rPlayer.GetName() == pGuildMemberData->strName;
}

BOOL CPythonGuild::IsGuildEnable() const
{
	return m_bGuildEnable;
}

CPythonGuild::TGuildInfo & CPythonGuild::GetGuildInfoRef()
{
	return m_GuildInfo;
}

BOOL CPythonGuild::GetGradeDataPtr(uint32_t dwGradeNumber, TGuildGradeData ** ppData)
{
	auto itor = m_GradeDataMap.find(dwGradeNumber);
	if (m_GradeDataMap.end() == itor)
		return FALSE;

	*ppData = &(itor->second);

	return TRUE;
}

const CPythonGuild::TGuildBoardCommentDataVector & CPythonGuild::GetGuildBoardCommentVector() const
{
	return m_GuildBoardCommentVector;
}

uint32_t CPythonGuild::GetMemberCount() const
{
	return m_GuildMemberDataVector.size();
}

BOOL CPythonGuild::GetMemberDataPtr(uint32_t dwIndex, TGuildMemberData ** ppData)
{
	if (dwIndex >= m_GuildMemberDataVector.size())
		return FALSE;

	*ppData = &m_GuildMemberDataVector[dwIndex];

	return TRUE;
}

BOOL CPythonGuild::GetMemberDataPtrByPID(uint32_t dwPID, TGuildMemberData ** ppData)
{
	auto itor = std::find_if(m_GuildMemberDataVector.begin(), m_GuildMemberDataVector.end(), CPythonGuild_FFindGuildMemberByPID(dwPID));

	if (m_GuildMemberDataVector.end() == itor)
		return FALSE;

	*ppData = &(*itor);
	return TRUE;
}

BOOL CPythonGuild::GetMemberDataPtrByName(const char * c_szName, TGuildMemberData ** ppData)
{
	auto itor = std::find_if(m_GuildMemberDataVector.begin(), m_GuildMemberDataVector.end(), CPythonGuild_FFindGuildMemberByName(c_szName));

	if (m_GuildMemberDataVector.end() == itor)
		return FALSE;

	*ppData = &(*itor);
	return TRUE;
}

uint32_t CPythonGuild::GetGuildMemberLevelSummary() const
{
	return m_dwMemberLevelSummary;
}

uint32_t CPythonGuild::GetGuildMemberLevelAverage() const
{
	return m_dwMemberLevelAverage;
}

uint32_t CPythonGuild::GetGuildExperienceSummary() const
{
	return m_dwMemberExperienceSummary;
}

CPythonGuild::TGuildSkillData & CPythonGuild::GetGuildSkillDataRef()
{
	return m_GuildSkillData;
}

bool CPythonGuild::GetGuildName(uint32_t dwID, std::string * pstrGuildName)
{
	if (m_GuildNameMap.end() == m_GuildNameMap.find(dwID))
		return false;

#ifdef ENABLE_COLORED_GUILD_RANKS
	switch (m_GuildNameMap[dwID].rank)
	{
		case 1:	//
			*pstrGuildName = "|cffFFC125" + m_GuildNameMap[dwID].name + "|r";
			break;

		case 2:	//
			*pstrGuildName = "|cff888888" + m_GuildNameMap[dwID].name + "|r";
			break;

		case 3:	//
			*pstrGuildName = "|cffCD661D" + m_GuildNameMap[dwID].name + "|r";
			break;
		default:
			*pstrGuildName = m_GuildNameMap[dwID].name;
			break;
	}
#else
	*pstrGuildName = m_GuildNameMap[dwID];
#endif

	return true;
}

uint32_t CPythonGuild::GetGuildID() const
{
	return m_GuildInfo.dwGuildID;
}

BOOL CPythonGuild::HasGuildLand() const
{
	return m_GuildInfo.bHasLand;
}

void CPythonGuild::StartGuildWar(uint32_t dwEnemyGuildID)
{
	int i;

	for (i = 0; i < ENEMY_GUILD_SLOT_MAX_COUNT; ++i)
		if (dwEnemyGuildID == m_adwEnemyGuildID[i])
			return;

	for (i = 0; i < ENEMY_GUILD_SLOT_MAX_COUNT; ++i)
		if (0 == m_adwEnemyGuildID[i])
		{
			m_adwEnemyGuildID[i] = dwEnemyGuildID;
			break;
		}
}

void CPythonGuild::EndGuildWar(uint32_t dwEnemyGuildID)
{
	for (auto & i : m_adwEnemyGuildID)
		if (dwEnemyGuildID == i)
			i = 0;
}

uint32_t CPythonGuild::GetEnemyGuildID(uint32_t dwIndex)
{
	if (dwIndex >= ENEMY_GUILD_SLOT_MAX_COUNT)
		return 0;

	return m_adwEnemyGuildID[dwIndex];
}

BOOL CPythonGuild::IsDoingGuildWar()
{
	for (auto & i : m_adwEnemyGuildID)
		if (0 != i)
			return TRUE;

	return FALSE;
}

void CPythonGuild::__CalculateLevelAverage()
{
	m_dwMemberLevelSummary = 0;
	m_dwMemberLevelAverage = 0;
	m_dwMemberExperienceSummary = 0;

	if (m_GuildMemberDataVector.empty())
		return;

	// Sum Level & Experience
	auto itor = m_GuildMemberDataVector.begin();
	for (; itor != m_GuildMemberDataVector.end(); ++itor)
	{
		TGuildMemberData & rGuildMemberData = *itor;
		m_dwMemberLevelSummary += rGuildMemberData.byLevel;
		m_dwMemberExperienceSummary += rGuildMemberData.dwOffer;
	}

	assert(!m_GuildMemberDataVector.empty());
	m_dwMemberLevelAverage = m_dwMemberLevelSummary / m_GuildMemberDataVector.size();
}

struct CPythonGuild_SLessMemberGrade
{
	bool operator()(CPythonGuild::TGuildMemberData & rleft, CPythonGuild::TGuildMemberData & rright) const
	{
		if (rleft.byGrade < rright.byGrade)
			return true;

		return false;
	}
};

void CPythonGuild::__SortMember()
{
	std::sort(m_GuildMemberDataVector.begin(), m_GuildMemberDataVector.end(), CPythonGuild_SLessMemberGrade());
}

BOOL CPythonGuild::__IsGradeData(uint8_t byGradeNumber)
{
	return m_GradeDataMap.end() != m_GradeDataMap.find(byGradeNumber);
}

void CPythonGuild::__Initialize()
{
	ZeroMemory(&m_GuildInfo, sizeof(m_GuildInfo));
	ZeroMemory(&m_GuildSkillData, sizeof(m_GuildSkillData));
	ZeroMemory(&m_adwEnemyGuildID, ENEMY_GUILD_SLOT_MAX_COUNT * sizeof(uint32_t));
	m_GradeDataMap.clear();
	m_GuildMemberDataVector.clear();
	m_dwMemberLevelSummary = 0;
	m_dwMemberLevelAverage = 0;
	m_bGuildEnable = FALSE;
	m_GuildNameMap.clear();
#ifdef ENABLE_GUILDBANK_LOG
	m_GuildInfo.pLogCount = 0;
#endif
}

void CPythonGuild::Destroy()
{
	__Initialize();
}

CPythonGuild::CPythonGuild()
{
	__Initialize();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

PyObject * guildIsGuildEnable(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonGuild::Instance().IsGuildEnable());
}

PyObject * guildGetGuildID(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonGuild::Instance().GetGuildID());
}

PyObject * guildHasGuildLand(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonGuild::Instance().HasGuildLand());
}

PyObject * guildGetGuildName(PyObject * poSelf, PyObject * poArgs)
{
	int iGuildID;
	if (!PyTuple_GetInteger(poArgs, 0, &iGuildID))
	{
		CPythonGuild::TGuildInfo & rGuildInfo = CPythonGuild::Instance().GetGuildInfoRef();
		return Py_BuildValue("s", rGuildInfo.szGuildName);
	}

	std::string strGuildName;
	if (!CPythonGuild::Instance().GetGuildName(iGuildID, &strGuildName))
		return Py_BuildValue("s", "Noname");

	return Py_BuildValue("s", strGuildName.c_str());
}

PyObject * guildGetGuildMasterName(PyObject * poSelf, PyObject * poArgs)
{
	CPythonGuild::TGuildInfo & rGuildInfo = CPythonGuild::Instance().GetGuildInfoRef();

	CPythonGuild::TGuildMemberData * pData;
	if (!CPythonGuild::Instance().GetMemberDataPtrByPID(rGuildInfo.dwMasterPID, &pData))
		return Py_BuildValue("s", "Noname");

	return Py_BuildValue("s", pData->strName.c_str());
}

PyObject * guildGetEnemyGuildName(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildValue("s", "");

	uint32_t dwEnemyGuildID = CPythonGuild::Instance().GetEnemyGuildID(iIndex);

	std::string strEnemyGuildName;
	if (!CPythonGuild::Instance().GetGuildName(dwEnemyGuildID, &strEnemyGuildName))
		return Py_BuildValue("s", "");

	return Py_BuildValue("s", strEnemyGuildName.c_str());
}

PyObject * guildGetGuildMoney(PyObject * poSelf, PyObject * poArgs)
{
	CPythonGuild::TGuildInfo & rGuildInfo = CPythonGuild::Instance().GetGuildInfoRef();
	return Py_BuildValue("i", rGuildInfo.dwGuildMoney);
}

PyObject * guildGetGuildBoardCommentCount(PyObject * poSelf, PyObject * poArgs)
{
	const CPythonGuild::TGuildBoardCommentDataVector & rCommentVector = CPythonGuild::Instance().GetGuildBoardCommentVector();
	return Py_BuildValue("i", rCommentVector.size());
}

PyObject * guildGetGuildBoardCommentData(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const CPythonGuild::TGuildBoardCommentDataVector & c_rCommentVector = CPythonGuild::Instance().GetGuildBoardCommentVector();
	if (uint32_t(iIndex) >= c_rCommentVector.size())
		return Py_BuildValue("iss", 0, "Noname", "Noname");

	const CPythonGuild::TGuildBoardCommentData & c_rData = c_rCommentVector[iIndex];

	return Py_BuildValue("iss", c_rData.dwCommentID, c_rData.strName.c_str(), c_rData.strComment.c_str());
}

PyObject * guildGetGuildLevel(PyObject * poSelf, PyObject * poArgs)
{
	CPythonGuild::TGuildInfo & rGuildInfo = CPythonGuild::Instance().GetGuildInfoRef();
	return Py_BuildValue("i", rGuildInfo.dwGuildLevel);
}

PyObject * guildGetGuildExperience(PyObject * poSelf, PyObject * poArgs)
{
	CPythonGuild::TGuildInfo & rGuildInfo = CPythonGuild::Instance().GetGuildInfoRef();

#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_EXTENDED_GUILD_LEVEL)
	int GULID_MAX_LEVEL = 40;
#else
	int GULID_MAX_LEVEL = 20;
#endif
	if (rGuildInfo.dwGuildLevel >= GULID_MAX_LEVEL)
		return Py_BuildValue("ii", 0, 0);

	unsigned lastExp = LocaleService_GetLastExp(rGuildInfo.dwGuildLevel);

	return Py_BuildValue("ii", rGuildInfo.dwCurrentExperience, lastExp - rGuildInfo.dwCurrentExperience);
}

PyObject * guildGetGuildMemberCount(PyObject * poSelf, PyObject * poArgs)
{
	CPythonGuild::TGuildInfo & rGuildInfo = CPythonGuild::Instance().GetGuildInfoRef();
	return Py_BuildValue("ii", rGuildInfo.dwCurrentMemberCount, rGuildInfo.dwMaxMemberCount);
}

PyObject * guildGetGuildMemberLevelSummary(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonGuild::Instance().GetGuildMemberLevelSummary());
}

PyObject * guildGetGuildMemberLevelAverage(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonGuild::Instance().GetGuildMemberLevelAverage());
}

PyObject * guildGetGuildExperienceSummary(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonGuild::Instance().GetGuildExperienceSummary());
}

PyObject * guildGetGuildSkillPoint(PyObject * poSelf, PyObject * poArgs)
{
	const CPythonGuild::TGuildSkillData & c_rSkillData = CPythonGuild::Instance().GetGuildSkillDataRef();
	return Py_BuildValue("i", c_rSkillData.bySkillPoint);
}

PyObject * guildGetDragonPowerPoint(PyObject * poSelf, PyObject * poArgs)
{
	const CPythonGuild::TGuildSkillData & c_rSkillData = CPythonGuild::Instance().GetGuildSkillDataRef();
	return Py_BuildValue("ii", c_rSkillData.wGuildPoint, c_rSkillData.wMaxGuildPoint);
}

PyObject * guildGetGuildSkillLevel(PyObject * poSelf, PyObject * poArgs)
{
	assert(FALSE && !"guildGetGuildSkillLevel - The function is not used.");

	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BuildException();

	const CPythonGuild::TGuildSkillData & c_rSkillData = CPythonGuild::Instance().GetGuildSkillDataRef();
	return Py_BuildValue("i", c_rSkillData.bySkillLevel[iSkillIndex]);
}

PyObject * guildGetSkillLevel(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	auto itor = g_GuildSkillSlotToIndexMap.find(iSlotIndex);

	if (g_GuildSkillSlotToIndexMap.end() == itor)
		return Py_BuildValue("i", 0);

	uint32_t dwSkillIndex = itor->second;
	assert(dwSkillIndex < CPythonGuild::GUILD_SKILL_MAX_NUM);

	const CPythonGuild::TGuildSkillData & c_rSkillData = CPythonGuild::Instance().GetGuildSkillDataRef();
	return Py_BuildValue("i", c_rSkillData.bySkillLevel[dwSkillIndex]);
}

PyObject * guildGetSkillMaxLevelNew(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	auto itor = g_GuildSkillSlotToIndexMap.find(iSlotIndex);

	if (g_GuildSkillSlotToIndexMap.end() == itor)
		return Py_BuildValue("i", 0);

	uint32_t dwSkillIndex = itor->second;
	assert(dwSkillIndex < CPythonGuild::GUILD_SKILL_MAX_NUM);

	const CPythonGuild::TGuildSkillData & c_rSkillData = CPythonGuild::Instance().GetGuildSkillDataRef();
	return Py_BuildValue("i", c_rSkillData.bySkillLevel[dwSkillIndex]);
}

PyObject * guildSetSkillIndex(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSkillIndex))
		return Py_BuildException();

	g_GuildSkillSlotToIndexMap.emplace(iSlotIndex, iSkillIndex);

	return Py_BuildNone();
}

PyObject * guildGetSkillIndex(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	auto itor = g_GuildSkillSlotToIndexMap.find(iSlotIndex);

	if (g_GuildSkillSlotToIndexMap.end() == itor)
		return Py_BuildValue("i", 0);

	uint32_t dwSkillIndex = itor->second;
	return Py_BuildValue("i", dwSkillIndex);
}

PyObject * guildGetGradeData(PyObject * poSelf, PyObject * poArgs)
{
	int iGradeNumber;
	if (!PyTuple_GetInteger(poArgs, 0, &iGradeNumber))
		return Py_BuildException();

	CPythonGuild::TGuildGradeData * pData;
	if (!CPythonGuild::Instance().GetGradeDataPtr(iGradeNumber, &pData))
		return Py_BuildValue("si", "?", 0);

	return Py_BuildValue("si", pData->strName.c_str(), pData->byAuthorityFlag);
}

PyObject * guildGetGradeName(PyObject * poSelf, PyObject * poArgs)
{
	int iGradeNumber;
	if (!PyTuple_GetInteger(poArgs, 0, &iGradeNumber))
		return Py_BuildException();

	CPythonGuild::TGuildGradeData * pData;
	if (!CPythonGuild::Instance().GetGradeDataPtr(iGradeNumber, &pData))
		return Py_BuildValue("s", "?");

	return Py_BuildValue("s", pData->strName.c_str());
}

PyObject * guildGetMemberCount(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonGuild::Instance().GetMemberCount());
}

PyObject * guildGetMemberData(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	CPythonGuild::TGuildMemberData * pData;
	if (!CPythonGuild::Instance().GetMemberDataPtr(iIndex, &pData))
		return Py_BuildValue("isiiiii", -1, "", 0, 0, 0, 0, 0);

	return Py_BuildValue("isiiiii", pData->dwPID, pData->strName.c_str(), pData->byGrade, pData->byJob, pData->byLevel, pData->dwOffer,
						 pData->byGeneralFlag);
}

PyObject * guildMemberIndexToPID(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	CPythonGuild::TGuildMemberData * pData;
	if (!CPythonGuild::Instance().GetMemberDataPtr(iIndex, &pData))
		return Py_BuildValue("i", -1);

	return Py_BuildValue("i", pData->dwPID);
}

PyObject * guildIsMember(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	CPythonGuild::TGuildMemberData * pData;
	if (CPythonGuild::Instance().GetMemberDataPtr(iIndex, &pData))
		return Py_BuildValue("i", TRUE);

	return Py_BuildValue("i", FALSE);
}

PyObject * guildIsMemberByName(PyObject * poSelf, PyObject * poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	CPythonGuild::TGuildMemberData * pData;
	if (CPythonGuild::Instance().GetMemberDataPtrByName(szName, &pData))
		return Py_BuildValue("i", TRUE);

	return Py_BuildValue("i", FALSE);
}

PyObject * guildMainPlayerHasAuthority(PyObject * poSelf, PyObject * poArgs)
{
	int iAuthority;
	if (!PyTuple_GetInteger(poArgs, 0, &iAuthority))
		return Py_BuildException();

	IAbstractPlayer & rPlayer = IAbstractPlayer::GetSingleton();
	const char * c_szMainPlayerName = rPlayer.GetName();

	CPythonGuild::TGuildMemberData * pMemberData;
	if (!CPythonGuild::Instance().GetMemberDataPtrByName(c_szMainPlayerName, &pMemberData))
		return Py_BuildValue("i", FALSE);

	CPythonGuild::TGuildGradeData * pGradeData;
	if (!CPythonGuild::Instance().GetGradeDataPtr(pMemberData->byGrade, &pGradeData))
		return Py_BuildValue("i", FALSE);

	return Py_BuildValue("i", iAuthority == (pGradeData->byAuthorityFlag & iAuthority));
}

PyObject * guildDestroy(PyObject * poSelf, PyObject * poArgs)
{
	CPythonGuild::Instance().Destroy();
	g_GuildSkillSlotToIndexMap.clear();
	return Py_BuildNone();
}


PyObject * guildGuildIDToMarkID(PyObject * poSelf, PyObject * poArgs)
{
	int guild_id;
	if (!PyTuple_GetInteger(poArgs, 0, &guild_id))
		return Py_BuildException();

	return Py_BuildValue("i", CGuildMarkManager::Instance().GetMarkID(guild_id));
}

PyObject * guildGetMarkImageFilenameByMarkID(PyObject * poSelf, PyObject * poArgs)
{
	int markID;

	if (!PyTuple_GetInteger(poArgs, 0, &markID))
		return Py_BuildException();

	std::string imagePath;
	CGuildMarkManager::Instance().GetMarkImageFilename(markID / CGuildMarkImage::MARK_TOTAL_COUNT, imagePath);
	return Py_BuildValue("s", imagePath.c_str());
}

PyObject * guildGetMarkIndexByMarkID(PyObject * poSelf, PyObject * poArgs)
{
	int markID;

	if (!PyTuple_GetInteger(poArgs, 0, &markID))
		return Py_BuildException();

	return Py_BuildValue("i", markID % CGuildMarkImage::MARK_TOTAL_COUNT);
}

#ifdef ENABLE_GUILDRENEWAL_SYSTEM
PyObject* guildGetWarRecode(PyObject* poSelf, PyObject* poArgs)
{
#ifdef ENABLE_GUILD_WAR_SCORE
	int index;

	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	CPythonGuild::TGuildInfo& rGuildInfo = CPythonGuild::Instance().GetGuildInfoRef();
	int win = rGuildInfo.winTypes[index];
	int lose = rGuildInfo.lossTypes[index];
	int draw = rGuildInfo.drawTypes[index];

	int all = draw + lose + win;

	return Py_BuildValue("iiii", win, lose, draw, all);
#else
	return Py_BuildValue("iiii", 0, 0, 0, 0);
#endif
}

PyObject* guildGetGuildLadderRanking(PyObject* poSelf, PyObject* poArgs)
{
#ifdef ENABLE_GUILD_WAR_SCORE
	CPythonGuild::TGuildInfo& rGuildInfo = CPythonGuild::Instance().GetGuildInfoRef();
	int ladder = rGuildInfo.ladderPoints;
	int ranking = rGuildInfo.rank;

	return Py_BuildValue("ii", ladder, ranking);
#else
	return Py_BuildValue("ii", 0, 0);
#endif
}

PyObject* guildGetBaseInfoBankGold(PyObject* poSelf, PyObject* poArgs)
{
	CPythonGuild::TGuildInfo& rGuildInfo = CPythonGuild::Instance().GetGuildInfoRef();
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDSTORAGE_SYSTEM)
	return Py_BuildValue("iss", rGuildInfo.dwGuildMoney, rGuildInfo.goldCheckout, rGuildInfo.itemCheckout);
#else
	return Py_BuildValue("iss", rGuildInfo.dwGuildMoney, " ", " ");
#endif
}

PyObject* guildGetBaseInfo(PyObject* poSelf, PyObject* poArgs)
{
#ifdef ENABLE_GUILD_LAND_INFO
	CPythonGuild::TGuildInfo& rGuildInfo = CPythonGuild::Instance().GetGuildInfoRef();
	return Py_BuildValue("i", rGuildInfo.mindex);
#else
	return Py_BuildValue("i", 0);
#endif
}

PyObject* guildGetbuildingInfo(PyObject* poSelf, PyObject* poArgs)
{
#ifdef ENABLE_GUILD_LAND_INFO
	CPythonGuild::TGuildInfo& rGuildInfo = CPythonGuild::Instance().GetGuildInfoRef();
	return Py_BuildValue("iiii", rGuildInfo.guildAltar, rGuildInfo.guildAlchemist, rGuildInfo.guildBlacksmith, rGuildInfo.storage_lv);
#else
	return Py_BuildValue("iiii", 0, 0, 0, 0);
#endif
}
#endif

#ifdef ENABLE_GUILDBANK_LOG
PyObject* guildGetGuildBankInfoSize(PyObject* poSelf, PyObject* poArgs)
{
	CPythonGuild::TGuildInfo& rGuildInfo = CPythonGuild::Instance().GetGuildInfoRef();
	return Py_BuildValue("i", rGuildInfo.pLogCount);
}

PyObject* guildGetGuildBankInfoData(PyObject* poSelf, PyObject* poArgs)
{
	int index = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	if (index >= CPythonGuild::Instance().GetLogs().size())
		index = 0;

	return Py_BuildValue("ssiii", CPythonGuild::Instance().GetLogs()[index].szName, CPythonGuild::Instance().GetLogs()[index].szItemName, CPythonGuild::Instance().GetLogs()[index].itemAction, CPythonGuild::Instance().GetLogs()[index].itemCount, CPythonGuild::Instance().GetLogs()[index].datatype);
}
#endif

#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
PyObject* guildGetGuildDonateStats(PyObject* poSelf, PyObject* poArgs)
{
	int normal_donate = 3;
	int normal_exp = 1000;

	int middle_donate = 10;
	int middle_exp = 10000;

	int high_donate = 30;
	int high_exp = 30000;

	return Py_BuildValue("iiiiii", normal_donate, normal_exp, middle_donate, middle_exp, high_donate, high_exp);
}

PyObject* guildGetGuildDonateCount(PyObject* poSelf, PyObject* poArgs)
{
	int donate_max = 3;

	CPythonGuild::TGuildInfo& rGuildInfo = CPythonGuild::Instance().GetGuildInfoRef();
	return Py_BuildValue("ii", rGuildInfo.bDonateCount, donate_max);
}
#endif

void initguild()
{
	static PyMethodDef s_methods[] = {// Enable
									  {"IsGuildEnable", guildIsGuildEnable, METH_VARARGS},
									  {"GuildIDToMarkID", guildGuildIDToMarkID, METH_VARARGS},
									  {"GetMarkImageFilenameByMarkID", guildGetMarkImageFilenameByMarkID, METH_VARARGS},
									  {"GetMarkIndexByMarkID", guildGetMarkIndexByMarkID, METH_VARARGS},

									  // GuildInfo
									  {"GetGuildID", guildGetGuildID, METH_VARARGS},
									  {"HasGuildLand", guildHasGuildLand, METH_VARARGS},
									  {"GetGuildName", guildGetGuildName, METH_VARARGS},
									  {"GetGuildMasterName", guildGetGuildMasterName, METH_VARARGS},
									  {"GetEnemyGuildName", guildGetEnemyGuildName, METH_VARARGS},
									  {"GetGuildMoney", guildGetGuildMoney, METH_VARARGS},

									  // BoardPage
									  {"GetGuildBoardCommentCount", guildGetGuildBoardCommentCount, METH_VARARGS},
									  {"GetGuildBoardCommentData", guildGetGuildBoardCommentData, METH_VARARGS},

									  // MemberPage
									  {"GetGuildLevel", guildGetGuildLevel, METH_VARARGS},
									  {"GetGuildExperience", guildGetGuildExperience, METH_VARARGS},
									  {"GetGuildMemberCount", guildGetGuildMemberCount, METH_VARARGS},
									  {"GetGuildMemberLevelSummary", guildGetGuildMemberLevelSummary, METH_VARARGS},
									  {"GetGuildMemberLevelAverage", guildGetGuildMemberLevelAverage, METH_VARARGS},
									  {"GetGuildExperienceSummary", guildGetGuildExperienceSummary, METH_VARARGS},

									  // SkillPage
									  {"GetGuildSkillPoint", guildGetGuildSkillPoint, METH_VARARGS},
									  {"GetDragonPowerPoint", guildGetDragonPowerPoint, METH_VARARGS},
									  {"GetGuildSkillLevel", guildGetGuildSkillLevel, METH_VARARGS},
									  {"GetSkillLevel", guildGetSkillLevel, METH_VARARGS},
									  {"GetSkillMaxLevelNew", guildGetSkillMaxLevelNew, METH_VARARGS},

									  {"SetSkillIndex", guildSetSkillIndex, METH_VARARGS},
									  {"GetSkillIndex", guildGetSkillIndex, METH_VARARGS},

									  // GradePage
									  {"GetGradeData", guildGetGradeData, METH_VARARGS},
									  {"GetGradeName", guildGetGradeName, METH_VARARGS},

									  // About Member
									  {"GetMemberCount", guildGetMemberCount, METH_VARARGS},
									  {"GetMemberData", guildGetMemberData, METH_VARARGS},
									  {"MemberIndexToPID", guildMemberIndexToPID, METH_VARARGS},
									  {"IsMember", guildIsMember, METH_VARARGS},
									  {"IsMemberByName", guildIsMemberByName, METH_VARARGS},
									  {"MainPlayerHasAuthority", guildMainPlayerHasAuthority, METH_VARARGS},

									  // Guild
									  {"Destroy", guildDestroy, METH_VARARGS},

#ifdef ENABLE_GUILDRENEWAL_SYSTEM
		{ "GetWarRecode",					guildGetWarRecode,					METH_VARARGS },
		{ "GetGuildLadderRanking",			guildGetGuildLadderRanking,			METH_VARARGS },
		{ "GetBaseInfoBankGold",			guildGetBaseInfoBankGold,			METH_VARARGS },
		{ "GetBaseInfo",					guildGetBaseInfo,					METH_VARARGS },
		{ "GetbuildingInfo",				guildGetbuildingInfo,				METH_VARARGS },
#endif
#ifdef ENABLE_GUILDBANK_LOG
		{ "GetGuildBankInfoSize",			guildGetGuildBankInfoSize,			METH_VARARGS },
		{ "GetGuildBankInfoData",			guildGetGuildBankInfoData,			METH_VARARGS },
#endif

#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
		{ "GetGuildDonateStats",			guildGetGuildDonateStats,			METH_VARARGS },
		{ "GetGuildDonateCount",			guildGetGuildDonateCount,			METH_VARARGS },
#endif

									  {nullptr, nullptr, 0}};

	PyObject * poModule = Py_InitModule("guild", s_methods);
	PyModule_AddIntConstant(poModule, "AUTH_ADD_MEMBER", GUILD_AUTH_ADD_MEMBER);
	PyModule_AddIntConstant(poModule, "AUTH_REMOVE_MEMBER", GUILD_AUTH_REMOVE_MEMBER);
	PyModule_AddIntConstant(poModule, "AUTH_NOTICE", GUILD_AUTH_NOTICE);
	PyModule_AddIntConstant(poModule, "AUTH_SKILL", GUILD_AUTH_SKILL);
#ifdef ENABLE_GUILDRENEWAL_SYSTEM
	PyModule_AddIntConstant(poModule, "AUTH_WAR", GUILD_AUTH_WAR);
	PyModule_AddIntConstant(poModule, "AUTH_BANK", GUILD_AUTH_BANK);
#endif

	PyModule_AddIntConstant(poModule, "ENEMY_GUILD_SLOT_MAX_COUNT", CPythonGuild::ENEMY_GUILD_SLOT_MAX_COUNT);
}
