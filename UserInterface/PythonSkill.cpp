#include "StdAfx.h"
#include "PythonSkill.h"

#include "../EterBase/Poly/Poly.h"
#include "../EterPack/EterPackManager.h"
#include "InstanceBase.h"
#include "PythonPlayer.h"
#include "../GameLib/GameLibDefines.h"

std::map<std::string, uint32_t> CPythonSkill::SSkillData::ms_StatusNameMap;
std::map<std::string, uint32_t> CPythonSkill::SSkillData::ms_NewMinStatusNameMap;
std::map<std::string, uint32_t> CPythonSkill::SSkillData::ms_NewMaxStatusNameMap;
uint32_t CPythonSkill::SSkillData::ms_dwTimeIncreaseSkillNumber = 0;

BOOL SKILL_EFFECT_UPGRADE_ENABLE = FALSE;

int SplitLine(const char * c_szText, CTokenVector * pstTokenVector, const char * c_szDelimeter)
{
	pstTokenVector->reserve(10);
	pstTokenVector->clear();

	std::string stToken;
	std::string stLine = c_szText;

	uint32_t basePos = 0;

	do
	{
		int beginPos = stLine.find_first_not_of(c_szDelimeter, basePos);

		if (beginPos < 0)
			return -1;

		int endPos;

		if (stLine[beginPos] == '"')
		{
			++beginPos;
			endPos = stLine.find_first_of('\"', beginPos);

			if (endPos < 0)
				return -2;

			basePos = endPos + 1;
		}
		else
		{
			endPos = stLine.find_first_of(c_szDelimeter, beginPos);
			basePos = endPos;
		}

		pstTokenVector->emplace_back(stLine.substr(beginPos, endPos - beginPos));

		// 추가 코드. 맨뒤에 탭이 있는 경우를 체크한다. - [levites]
		if (int(stLine.find_first_not_of(c_szDelimeter, basePos)) < 0)
			break;
	} while (basePos < stLine.length());

	return 0;
}

// OVERWRITE_SKILLPROTO_POLY
void string_replace_word(const char * base, int base_len, const char * src, int src_len, const char * dst, int dst_len,
						 std::string & result)
{
	result.resize(0);
	if (base_len > 0 && dst_len > src_len)
		result.reserve(base_len + (dst_len - src_len) * (base_len / src_len));
	else
		result.reserve(base_len);

	int prev = 0;
	int cur = 0;
	while (cur < base_len)
	{
		if (memcmp(base + cur, src, src_len) == 0)
		{
			result.append(base + prev, cur - prev);
			result.append(dst, dst_len);
			cur += src_len;
			prev = cur;
		}
		else
			++cur;
	}
	result.append(base + prev, cur - prev);
}
// END_OF_OVERWRITE_SKILLPROTO_POLY

bool CPythonSkill::RegisterSkillTable(const char * c_szFileName)
{
	const VOID * pvData;
	CMappedFile kFile;
	if (!CEterPackManager::Instance().Get(kFile, c_szFileName, &pvData))
		return false;

	CMemoryTextFileLoader textFileLoader;
	textFileLoader.Bind(kFile.Size(), pvData);

	// OVERWRITE_SKILLPROTO_POLY
	std::string src_poly_rand;
	std::string src_poly_atk;
	// END_OF_OVERWRITE_SKILLPROTO_POLY

	CTokenVector TokenVector;
	for (uint32_t i = 0; i < textFileLoader.GetLineCount(); ++i)
	{
		if (!textFileLoader.SplitLineByTab(i, &TokenVector))
			continue;

		std::string strLine = textFileLoader.GetLineString(i);

		if (TABLE_TOKEN_TYPE_MAX_NUM != TokenVector.size())
		{
			Tracef("CPythonSkill::RegisterSkillTable(%s) - Strange Token Count [Line:%d / TokenCount:%d]\n", c_szFileName, i,
				   TokenVector.size());
			continue;
		}

		int iVnum = atoi(TokenVector[TABLE_TOKEN_TYPE_VNUM].c_str());

		auto itor = m_SkillDataMap.find(iVnum);
		if (m_SkillDataMap.end() == itor)
		{
			Tracef("CPythonSkill::RegisterSkillTable(%s) - NOT EXIST SkillDesc [Vnum:%d Line:%d]\n", c_szFileName, iVnum, i);
			continue;
		}

		TSkillData & rSkillData = itor->second;

		const std::string & c_strSPCostPoly = TokenVector[TABLE_TOKEN_TYPE_SP_COST_POLY];
		if (!c_strSPCostPoly.empty())
			rSkillData.strNeedSPFormula = c_strSPCostPoly;

		const std::string & c_strCooldownPoly = TokenVector[TABLE_TOKEN_TYPE_COOLDOWN_POLY];
		if (!c_strCooldownPoly.empty())
			rSkillData.strCoolTimeFormula = c_strCooldownPoly;

		const std::string & c_strDurationSPCostPoly = TokenVector[TABLE_TOKEN_TYPE_DURATION_SP_COST_POLY];
		if (!c_strDurationSPCostPoly.empty())
			rSkillData.strContinuationSPFormula = c_strDurationSPCostPoly;

		const std::string & c_strTargetRange = TokenVector[TABLE_TOKEN_TYPE_TARGET_RANGE];
		if (!c_strTargetRange.empty())
			rSkillData.dwTargetRange = atoi(c_strTargetRange.c_str());

		rSkillData.strDuration = TokenVector[TABLE_TOKEN_TYPE_DURATION_POLY];


		const uint32_t LevelEmpty = 1;

		const std::string & c_strMaxLevel = TokenVector[TABLE_TOKEN_TYPE_MAX_LEVEL];
		if (!c_strMaxLevel.empty())
		{
			int maxLevel = atoi(c_strMaxLevel.c_str());
			if (maxLevel > LevelEmpty)
				rSkillData.byMaxLevel = maxLevel;
		}

		const std::string & c_strLevelLimit = TokenVector[TABLE_TOKEN_TYPE_LEVEL_LIMIT];
		if (!c_strLevelLimit.empty())
		{
			int levelLimit = atoi(c_strLevelLimit.c_str());
			if (rSkillData.byLevelLimit > LevelEmpty)
				rSkillData.byLevelLimit = levelLimit;
		}
		const std::string & c_strPointPoly = TokenVector[TABLE_TOKEN_TYPE_POINT_POLY];

		// OVERWRITE_SKILLPROTO_POLY
		bool USE_SKILL_PROTO = LocaleService_IsCHEONMA() ? false : true;

		switch (iVnum)
		{
			case 34:

			// GUILD_SKILL_DISPLAY_BUG_FIX
			case 151:
			case 152:
			case 153:
			case 154:
			case 155:
			case 156:
			case 157:
				// END_OF_GUILD_SKILL_DISPLAY_BUG_FIX
				USE_SKILL_PROTO = false;
				break;
		}

		if (!rSkillData.AffectDataVector.empty() && USE_SKILL_PROTO)
		{
			TAffectData & affect = rSkillData.AffectDataVector[0];

			if (strstr(c_strPointPoly.c_str(), "atk") != nullptr || strstr(c_strPointPoly.c_str(), "mwep") != nullptr ||
				strstr(c_strPointPoly.c_str(), "number") != nullptr)
			{
				src_poly_rand = "";
				src_poly_atk = "";
				std::string src_poly_mwep;

				// MIN
				string_replace_word(c_strPointPoly.c_str(), c_strPointPoly.length(), "number", 6, "min", 3, src_poly_rand);
				string_replace_word(src_poly_rand.c_str(), src_poly_rand.length(), "atk", 3, "minatk", 6, src_poly_atk);
				string_replace_word(src_poly_atk.c_str(), src_poly_atk.length(), "mwep", 4, "minmwep", 7, affect.strAffectMinFormula);
				// END_OF_MIN

				// MAX
				string_replace_word(c_strPointPoly.c_str(), c_strPointPoly.length(), "number", 6, "max", 3, src_poly_rand);
				string_replace_word(src_poly_rand.c_str(), src_poly_rand.length(), "atk", 3, "maxatk", 6, src_poly_atk);
				string_replace_word(src_poly_atk.c_str(), src_poly_atk.length(), "mwep", 4, "maxmwep", 7, affect.strAffectMaxFormula);
				// END_OF_MAX

				switch (iVnum)
				{
					case 1:
						affect.strAffectMinFormula += "* 3";
						affect.strAffectMaxFormula += "* 3";
						break;
				}
			}
			else
			{
				affect.strAffectMinFormula = c_strPointPoly;
				affect.strAffectMaxFormula = "";
			}
		}
		// END_OF_OVERWRITE_SKILLPROTO_POLY
	}

	return true;
}

void CPythonSkill::__RegisterGradeIconImage(TSkillData & rData, const char * c_szHeader, const char * c_szImageName) const
{
	for (int j = 0; j < SKILL_GRADE_COUNT; ++j)
	{
		TGradeData & rGradeData = rData.GradeData[j];

		char szCount[8 + 1];
		_snprintf(szCount, sizeof(szCount), "_%02d", j + 1);

		std::string strFileName;
		strFileName += c_szHeader;
		strFileName += c_szImageName;
		strFileName += szCount;
		strFileName += ".sub";
		rGradeData.pImage = msl::inherit_cast<CGraphicImage *>(CResourceManager::Instance().GetResourcePointer(strFileName.c_str()));
	}
}

void CPythonSkill::__RegisterNormalIconImage(TSkillData & rData, const char * c_szHeader, const char * c_szImageName) const
{
	std::string strFileName;
	strFileName += c_szHeader;
	strFileName += c_szImageName;
	strFileName += ".sub";
	rData.pImage = msl::inherit_cast<CGraphicImage *>(CResourceManager::Instance().GetResourcePointer(strFileName.c_str()));
	for (int j = 0; j < SKILL_GRADE_COUNT; ++j)
	{
		TGradeData & rGradeData = rData.GradeData[j];
		rGradeData.pImage = rData.pImage;
	}
}
extern const uint32_t c_iSkillIndex_Riding;
bool CPythonSkill::RegisterSkillDesc(const char * c_szFileName)
{
	const VOID * pvData;
	CMappedFile kFile;
	if (!CEterPackManager::Instance().Get(kFile, c_szFileName, &pvData))
		return false;

	CMemoryTextFileLoader textFileLoader;
	textFileLoader.Bind(kFile.Size(), pvData);

	CTokenVector TokenVector;
	for (uint32_t i = 0; i < textFileLoader.GetLineCount(); ++i)
	{
		if (!textFileLoader.SplitLineByTab(i, &TokenVector))
			continue;

		if (DESC_TOKEN_TYPE_VNUM >= TokenVector.size())
		{
			TraceError("SkillDesc.line(%d).NO_VNUM_ERROR\n", i + 1);
			continue;
		}

		uint32_t iSkillIndex = atoi(TokenVector[DESC_TOKEN_TYPE_VNUM].c_str());
		if (iSkillIndex == 0)
		{
			TraceError("SkillDesc.line(%d).NO_INDEX_ERROR\n", i + 1);
			continue;
		}

		if (DESC_TOKEN_TYPE_JOB >= TokenVector.size())
		{
			TraceError("SkillDesc.line(%d).NO_JOB_ERROR\n", i + 1);
			continue;
		}

		m_SkillDataMap.emplace(iSkillIndex, TSkillData());
		TSkillData & rSkillData = m_SkillDataMap[iSkillIndex];

		// Vnum
		rSkillData.dwSkillIndex = iSkillIndex;

		// Type
		{
			std::string strJob = TokenVector[DESC_TOKEN_TYPE_JOB];
			stl_lowers(strJob);

			auto it = m_SkillTypeIndexMap.find(strJob);
			if (m_SkillTypeIndexMap.end() == it)
			{
				TraceError("CPythonSkill::RegisterSkillDesc(dwSkillIndex=%d, strType=%s).STRANGE_SKILL_TYPE", iSkillIndex, strJob.c_str());
				continue;
			}

			rSkillData.byType = uint8_t(it->second);
		}

		// Name
		{
			rSkillData.strName = TokenVector[DESC_TOKEN_TYPE_NAME1];
			rSkillData.GradeData[0].strName = TokenVector[DESC_TOKEN_TYPE_NAME1];
			rSkillData.GradeData[1].strName = TokenVector[DESC_TOKEN_TYPE_NAME2];
			rSkillData.GradeData[2].strName = TokenVector[DESC_TOKEN_TYPE_NAME3];

			if (rSkillData.GradeData[1].strName.empty())
				rSkillData.GradeData[1].strName = TokenVector[DESC_TOKEN_TYPE_NAME1];
			if (rSkillData.GradeData[2].strName.empty())
				rSkillData.GradeData[2].strName = TokenVector[DESC_TOKEN_TYPE_NAME1];
		}

		// Description
		{
			rSkillData.strDescription = TokenVector[DESC_TOKEN_TYPE_DESCRIPTION];
		}

		// Condition
		{
			rSkillData.ConditionDataVector.clear();
			for (int j = 0; j < CONDITION_COLUMN_COUNT; ++j)
			{
				const std::string & c_rstrCondition = TokenVector[DESC_TOKEN_TYPE_CONDITION1 + j];
				if (!c_rstrCondition.empty())
					rSkillData.ConditionDataVector.emplace_back(c_rstrCondition);
			}
		}

		// Attribute
		{
			const std::string & c_rstrAttribute = TokenVector[DESC_TOKEN_TYPE_ATTRIBUTE];
			CTokenVector AttributeTokenVector;
			SplitLine(c_rstrAttribute.c_str(), &AttributeTokenVector, "|");

			for (auto & rstrToken : AttributeTokenVector)
			{
				auto it2 = m_SkillAttributeIndexMap.find(rstrToken);
				if (m_SkillAttributeIndexMap.end() == it2)
				{
					TraceError("CPythonSkill::RegisterSkillDesc(dwSkillIndex=%d) - Strange Skill Attribute(%s)", iSkillIndex,
							   rstrToken.c_str());
					continue;
				}
				rSkillData.dwSkillAttribute |= it2->second;
			}
		}

		// Weapon
		{
			const std::string & c_rstrWeapon = TokenVector[DESC_TOKEN_TYPE_WEAPON];
			CTokenVector WeaponTokenVector;
			SplitLine(c_rstrWeapon.c_str(), &WeaponTokenVector, "|");

			for (auto & rstrToken : WeaponTokenVector)
			{
				auto it2 = m_SkillNeedWeaponIndexMap.find(rstrToken);
				if (m_SkillNeedWeaponIndexMap.end() == it2)
				{
					TraceError("CPythonSkill::RegisterSkillDesc(dwSkillIndex=%d) - Strange Skill Need Weapon(%s)", iSkillIndex,
							   rstrToken.c_str());
					continue;
				}
				rSkillData.dwNeedWeapon |= it2->second;
			}
		}

		// Icon Name
		{
			const std::string & c_rstrJob = TokenVector[DESC_TOKEN_TYPE_JOB];
			const std::string & c_rstrIconName = TokenVector[DESC_TOKEN_TYPE_ICON_NAME];

			// NOTE : Support 스킬임에도 3단계를 지원해야 해서 예외 처리 - [levites]
			if (c_iSkillIndex_Riding == iSkillIndex)
			{
				char szIconFileNameHeader[64 + 1];
				_snprintf(szIconFileNameHeader, sizeof(szIconFileNameHeader), "%sskill/common/support/", g_strImagePath.c_str());

				__RegisterGradeIconImage(rSkillData, szIconFileNameHeader, c_rstrIconName.c_str());
			}
			else if (m_PathNameMap.end() != m_PathNameMap.find(c_rstrJob))
			{
				char szIconFileNameHeader[64 + 1];
				_snprintf(szIconFileNameHeader, sizeof(szIconFileNameHeader), "%sskill/%s/", g_strImagePath.c_str(),
						  m_PathNameMap[c_rstrJob].c_str());

				switch (rSkillData.byType)
				{
					case SKILL_TYPE_ACTIVE:
					{
						__RegisterGradeIconImage(rSkillData, szIconFileNameHeader, c_rstrIconName.c_str());
						break;
					}
					case SKILL_TYPE_SUPPORT:
					case SKILL_TYPE_GUILD:
					case SKILL_TYPE_HORSE:
#ifdef ENABLE_OFFICAL_FEATURES
					case SKILL_TYPE_MOUNT_UPGRADE:
#endif
					{
						__RegisterNormalIconImage(rSkillData, szIconFileNameHeader, c_rstrIconName.c_str());
						break;
					}
				}
			}
		}

		// Motion Index
		if (TokenVector.size() > DESC_TOKEN_TYPE_MOTION_INDEX)
		{
			const std::string & c_rstrMotionIndex = TokenVector[DESC_TOKEN_TYPE_MOTION_INDEX];
			if (c_rstrMotionIndex.empty())
			{
				rSkillData.bNoMotion = TRUE;
				rSkillData.wMotionIndex = 0;
			}
			else
			{
				rSkillData.bNoMotion = FALSE;
				rSkillData.wMotionIndex = static_cast<uint16_t>(atoi(c_rstrMotionIndex.c_str()));
			}
		}
		else
			rSkillData.wMotionIndex = 0;

		if (TokenVector.size() > DESC_TOKEN_TYPE_TARGET_COUNT_FORMULA)
			rSkillData.strTargetCountFormula = TokenVector[DESC_TOKEN_TYPE_TARGET_COUNT_FORMULA];
		if (TokenVector.size() > DESC_TOKEN_TYPE_MOTION_LOOP_COUNT_FORMULA)
			rSkillData.strMotionLoopCountFormula = TokenVector[DESC_TOKEN_TYPE_MOTION_LOOP_COUNT_FORMULA];

		rSkillData.AffectDataNewVector.clear();
		rSkillData.AffectDataNewVector.reserve(3);

		// Affect
		for (int j = 0; j < AFFECT_COLUMN_COUNT; ++j)
		{
			const int iDescriptionSlotIndex = DESC_TOKEN_TYPE_AFFECT_DESCRIPTION_1 + j * AFFECT_STEP_COUNT;
			const int iMinSlotIndex = DESC_TOKEN_TYPE_AFFECT_MIN_1 + j * AFFECT_STEP_COUNT;
			const int iMaxSlotIndex = DESC_TOKEN_TYPE_AFFECT_MAX_1 + j * AFFECT_STEP_COUNT;

			if (TokenVector.size() > iDescriptionSlotIndex)
				if (!TokenVector[iDescriptionSlotIndex].empty())
				{
					rSkillData.AffectDataVector.emplace_back();

					TAffectData & rAffectData = *rSkillData.AffectDataVector.rbegin();
					rAffectData.strAffectDescription = "";
					rAffectData.strAffectMinFormula = "";
					rAffectData.strAffectMaxFormula = "";

					rAffectData.strAffectDescription = TokenVector[iDescriptionSlotIndex];
					if (TokenVector.size() > iMinSlotIndex)
						if (!TokenVector[iMinSlotIndex].empty())
							rAffectData.strAffectMinFormula = TokenVector[iMinSlotIndex];
					if (TokenVector.size() > iMaxSlotIndex)
						if (!TokenVector[iMaxSlotIndex].empty())
							rAffectData.strAffectMaxFormula = TokenVector[iMaxSlotIndex];
				}
		}

		if (TokenVector.size() > DESC_TOKEN_TYPE_MOTION_INDEX_GRADE_NUM)
		{
			int numGrade = atoi(TokenVector[DESC_TOKEN_TYPE_MOTION_INDEX_GRADE_NUM].c_str());
			if (SKILL_EFFECT_COUNT < numGrade)
			{
				TraceError("%s[%s] Skill grade motion [%d] Out Of Range.", rSkillData.strName.c_str(),
						   TokenVector[DESC_TOKEN_TYPE_MOTION_INDEX_GRADE_NUM].c_str(), SKILL_EFFECT_COUNT);
				return false;
			}
			for (int iGrade = 0; iGrade < numGrade; iGrade++)
			{
				rSkillData.GradeData[iGrade].wMotionIndex = rSkillData.wMotionIndex + iGrade * SKILL_GRADEGAP;
			}
			for (int iSpace = numGrade; iSpace < SKILL_EFFECT_COUNT; iSpace++)
				rSkillData.GradeData[iSpace].wMotionIndex = rSkillData.wMotionIndex;
		}

		if (TokenVector.size() > DESC_TOKEN_TYPE_LEVEL_LIMIT)
		{
			const std::string & c_rstrLevelLimit = TokenVector[DESC_TOKEN_TYPE_LEVEL_LIMIT];
			if (c_rstrLevelLimit.empty())
				rSkillData.byLevelLimit = 0;
			else
				rSkillData.byLevelLimit = static_cast<uint16_t>(atoi(c_rstrLevelLimit.c_str()));
		}

		if (TokenVector.size() > DESC_TOKEN_TYPE_MAX_LEVEL)
		{
			const std::string & c_rstrMaxLevel = TokenVector[DESC_TOKEN_TYPE_MAX_LEVEL];
			const uint32_t LevelLimitEmpty = 1;
			if (c_rstrMaxLevel.empty())
			{
				if (rSkillData.byLevelLimit > LevelLimitEmpty)
					rSkillData.byMaxLevel = rSkillData.byLevelLimit;
				else
					rSkillData.byMaxLevel = 20;
			}
			else
				rSkillData.byMaxLevel = static_cast<uint8_t>(atoi(c_rstrMaxLevel.c_str()));
		}
	}

	return true;
}

void CPythonSkill::Destroy()
{
	m_SkillDataMap.clear();
}

bool CPythonSkill::RegisterSkill(uint32_t dwSkillIndex, const char * c_szFileName)
{
	CTextFileLoader TextFileLoader;
	if (!TextFileLoader.Load(c_szFileName))
	{
		TraceError("CPythonSkill::RegisterSkill(dwSkillIndex=%d, c_szFileName=%s) - Failed to find file", dwSkillIndex, c_szFileName);
		return false;
	}

	TextFileLoader.SetTop();

	TSkillData SkillData;
	SkillData.dwSkillIndex = dwSkillIndex;

	std::string strTypeName;
	if (TextFileLoader.GetTokenString("type", &strTypeName))
	{
		stl_lowers(strTypeName);
		auto it = m_SkillTypeIndexMap.find(strTypeName);
		if (m_SkillTypeIndexMap.end() == it)
		{
			TraceError("Strange Skill Type - CPythonSkill::RegisterSkill(dwSkillIndex=%d, c_szFileName=%s)", dwSkillIndex, c_szFileName);
			return false;
		}

		SkillData.byType = (uint8_t) it->second;
	}
	else
	{
		TraceError("CPythonSkill::RegisterSkill(dwSkillIndex=%d, c_szFileName=%s) - Failed to find key [type]", dwSkillIndex, c_szFileName);
		return false;
	}

	CTokenVector * pAttributeTokenVector;
	if (TextFileLoader.GetTokenVector("attribute", &pAttributeTokenVector))
	{
		for (auto & rstrToken : *pAttributeTokenVector)
		{
			auto it2 = m_SkillAttributeIndexMap.find(rstrToken);
			if (m_SkillAttributeIndexMap.end() == it2)
			{
				TraceError("Strange Skill Attribute - CPythonSkill::RegisterSkill(dwSkillIndex=%d, c_szFileName=%s)", dwSkillIndex,
						   c_szFileName);
				continue;
			}
			SkillData.dwSkillAttribute |= it2->second;
		}
	}

	CTokenVector * pNeedWeaponVector;
	if (TextFileLoader.GetTokenVector("needweapon", &pNeedWeaponVector))
	{
		for (auto & rstrToken : *pNeedWeaponVector)
		{
			auto it2 = m_SkillNeedWeaponIndexMap.find(rstrToken);
			if (m_SkillNeedWeaponIndexMap.end() == it2)
			{
				TraceError("Strange Skill Need Weapon - CPythonSkill::RegisterSkill(dwSkillIndex=%d, c_szFileName=%s)", dwSkillIndex,
						   c_szFileName);
				continue;
			}
			SkillData.dwNeedWeapon |= it2->second;
		}
	}

	{
		char szName[256];
		sprintf(szName, "%dname", LocaleService_GetCodePage());
		if (!TextFileLoader.GetTokenString(szName, &SkillData.strName))
			if (!TextFileLoader.GetTokenString("name", &SkillData.strName))
			{
				TraceError("CPythonSkill::RegisterSkill(dwSkillIndex=%d, c_szFileName=%s) - Failed to find [%s]", dwSkillIndex,
						   c_szFileName, szName);
				return false;
			}
	}

	{
		char szName[256];
		sprintf(szName, "%ddescription", LocaleService_GetCodePage());
		if (!TextFileLoader.GetTokenString(szName, &SkillData.strDescription))
			TextFileLoader.GetTokenString("description", &SkillData.strDescription);
	}

	if (!TextFileLoader.GetTokenString("iconfilename", &SkillData.strIconFileName))
	{
		TraceError("CPythonSkill::RegisterSkill(dwSkillIndex=%d, c_szFileName=%s) - Failed to find [iconfilename]", dwSkillIndex,
				   c_szFileName);
		return false;
	}

	{
		CTokenVector * pConditionDataVector;

		char szConditionData[256];
		sprintf(szConditionData, "%dconditiondata", LocaleService_GetCodePage());

		bool isConditionData = true;
		if (!TextFileLoader.GetTokenVector(szConditionData, &pConditionDataVector))
			if (!TextFileLoader.GetTokenVector("conditiondata", &pConditionDataVector))
				isConditionData = false;

		if (isConditionData)
		{
			uint32_t dwSize = pConditionDataVector->size();
			SkillData.ConditionDataVector.clear();
			SkillData.ConditionDataVector.resize(dwSize);
			for (uint32_t i = 0; i < dwSize; ++i)
				SkillData.ConditionDataVector[i] = pConditionDataVector->at(i);
		}
	}

	{
		CTokenVector * pAffectDataVector;

		char szAffectData[256];
		sprintf(szAffectData, "%daffectdata", LocaleService_GetCodePage());

		bool isAffectData = true;
		if (!TextFileLoader.GetTokenVector(szAffectData, &pAffectDataVector))
			if (!TextFileLoader.GetTokenVector("affectdata", &pAffectDataVector))
				isAffectData = false;

		if (isAffectData)
		{
			uint32_t dwSize = pAffectDataVector->size() / 3;
			SkillData.AffectDataVector.clear();
			SkillData.AffectDataVector.resize(dwSize);
			for (uint32_t i = 0; i < dwSize; ++i)
			{
				SkillData.AffectDataVector[i].strAffectDescription = pAffectDataVector->at(i * 3 + 0);
				SkillData.AffectDataVector[i].strAffectMinFormula = pAffectDataVector->at(i * 3 + 1);
				SkillData.AffectDataVector[i].strAffectMaxFormula = pAffectDataVector->at(i * 3 + 2);
			}
		}
	}

	{
		CTokenVector * pGradeDataVector;

		char szGradeData[256];
		sprintf(szGradeData, "%dgradedata", LocaleService_GetCodePage());

		if (TextFileLoader.GetTokenVector(szGradeData, &pGradeDataVector))
		{
			if (SKILL_GRADE_COUNT * 2 != pGradeDataVector->size())
				TraceError("CPythonSkill::RegisterSkill(dwSkillIndex=%d, c_szFileName=%s) - Strange Grade Data Count", dwSkillIndex,
						   c_szFileName);

			for (uint32_t i = 0; i < std::min<size_t>(SKILL_GRADE_COUNT, pGradeDataVector->size() / 2); ++i)
			{
				SkillData.GradeData[i].strName = pGradeDataVector->at(i * 2 + 0);
				std::string strIconFileName = g_strImagePath + pGradeDataVector->at(i * 2 + 1);
				SkillData.GradeData[i].pImage =
					msl::inherit_cast<CGraphicImage *>(CResourceManager::Instance().GetResourcePointer(strIconFileName.c_str()));
			}
		}
	}

	TextFileLoader.GetTokenString("cooltimeformula", &SkillData.strCoolTimeFormula);
	TextFileLoader.GetTokenString("targetcountformula", &SkillData.strTargetCountFormula);
	TextFileLoader.GetTokenString("motionloopcountformula", &SkillData.strMotionLoopCountFormula);
	TextFileLoader.GetTokenString("needspformula", &SkillData.strNeedSPFormula);
	TextFileLoader.GetTokenString("continuationspformula", &SkillData.strContinuationSPFormula);
	TextFileLoader.GetTokenDoubleWord("range", &SkillData.dwTargetRange);
	TextFileLoader.GetTokenByte("maxlevel", &SkillData.byMaxLevel);
	TextFileLoader.GetTokenByte("leveluppoint", &SkillData.byLevelUpPoint);

	uint16_t wMotionIndex;
	if (TextFileLoader.GetTokenWord("motionindex", &wMotionIndex))
		SkillData.wMotionIndex = wMotionIndex;
	else
		SkillData.wMotionIndex = 0;

	uint16_t wMotionIndexForMe;
	if (TextFileLoader.GetTokenWord("motionindexforme", &wMotionIndexForMe))
		SkillData.wMotionIndexForMe = wMotionIndexForMe;
	else
		SkillData.wMotionIndexForMe = 0;

	SkillData.strIconFileName = g_strImagePath + SkillData.strIconFileName;
	SkillData.pImage =
		msl::inherit_cast<CGraphicImage *>(CResourceManager::Instance().GetResourcePointer(SkillData.strIconFileName.c_str()));

	m_SkillDataMap.emplace(dwSkillIndex, SkillData);

	/////

	if (SkillData.IsTimeIncreaseSkill())
		SSkillData::ms_dwTimeIncreaseSkillNumber = SkillData.dwSkillIndex;

	return true;
}

BOOL CPythonSkill::GetSkillData(uint32_t dwSkillIndex, TSkillData ** ppSkillData)
{
	auto it = m_SkillDataMap.find(dwSkillIndex);

	if (m_SkillDataMap.end() == it)
		return FALSE;

	*ppSkillData = &(it->second);
	return TRUE;
}

bool CPythonSkill::GetSkillDataByName(const char * c_szName, TSkillData ** ppSkillData)
{
	auto itor = m_SkillDataMap.begin();
	for (; itor != m_SkillDataMap.end(); ++itor)
	{
		TSkillData * pData = &(itor->second);
		if (c_szName == pData->strName)
		{
			*ppSkillData = &(itor->second);
			return true;
		}
	}
	return false;
}

void CPythonSkill::SetPathName(const char * c_szFileName)
{
	m_strPathName = c_szFileName;
}

const char * CPythonSkill::GetPathName() const
{
	return m_strPathName.c_str();
}

CPythonSkill::CPythonSkill()
{
	m_SkillTypeIndexMap.emplace("none", SKILL_TYPE_NONE);
	m_SkillTypeIndexMap.emplace("active", SKILL_TYPE_ACTIVE);
	m_SkillTypeIndexMap.emplace("support", SKILL_TYPE_SUPPORT);
	m_SkillTypeIndexMap.emplace("guild", SKILL_TYPE_GUILD);
	m_SkillTypeIndexMap.emplace("horse", SKILL_TYPE_HORSE);
#ifdef ENABLE_OFFICAL_FEATURES
	m_SkillTypeIndexMap.emplace("mount_upgrade", SKILL_TYPE_MOUNT_UPGRADE);
#endif
	m_SkillTypeIndexMap.emplace("warrior", SKILL_TYPE_ACTIVE);
	m_SkillTypeIndexMap.emplace("assassin", SKILL_TYPE_ACTIVE);
	m_SkillTypeIndexMap.emplace("sura", SKILL_TYPE_ACTIVE);
	m_SkillTypeIndexMap.emplace("shaman", SKILL_TYPE_ACTIVE);
	m_SkillTypeIndexMap.emplace("passive", SKILL_TYPE_ACTIVE);
#ifdef ENABLE_WOLFMAN_CHARACTER
	m_SkillTypeIndexMap.emplace("78skill", SKILL_TYPE_ACTIVE);
	m_SkillTypeIndexMap.emplace("wolfman", SKILL_TYPE_ACTIVE);
#endif

	m_SkillAttributeIndexMap.emplace("NEED_POISON_BOTTLE", SKILL_ATTRIBUTE_NEED_POISON_BOTTLE);
	m_SkillAttributeIndexMap.emplace("NEED_EMPTY_BOTTLE", SKILL_ATTRIBUTE_NEED_EMPTY_BOTTLE);
	m_SkillAttributeIndexMap.emplace("CAN_USE_IF_NOT_ENOUGH", SKILL_ATTRIBUTE_CAN_USE_IF_NOT_ENOUGH);
	m_SkillAttributeIndexMap.emplace("FAN_RANGE", SKILL_ATTRIBUTE_FAN_RANGE);
	m_SkillAttributeIndexMap.emplace("NEED_CORPSE", SKILL_ATTRIBUTE_NEED_CORPSE);
	m_SkillAttributeIndexMap.emplace("NEED_TARGET", SKILL_ATTRIBUTE_NEED_TARGET);
	m_SkillAttributeIndexMap.emplace("TOGGLE", SKILL_ATTRIBUTE_TOGGLE);
	m_SkillAttributeIndexMap.emplace("WEAPON_LIMITATION", SKILL_ATTRIBUTE_WEAPON_LIMITATION);
	m_SkillAttributeIndexMap.emplace("MELEE_ATTACK", SKILL_ATTRIBUTE_MELEE_ATTACK);
	m_SkillAttributeIndexMap.emplace("CHARGE_ATTACK", SKILL_ATTRIBUTE_CHARGE_ATTACK);
	m_SkillAttributeIndexMap.emplace("USE_HP", SKILL_ATTRIBUTE_USE_HP);
	m_SkillAttributeIndexMap.emplace("CAN_CHANGE_DIRECTION", SKILL_ATTRIBUTE_CAN_CHANGE_DIRECTION);
	m_SkillAttributeIndexMap.emplace("STANDING_SKILL", SKILL_ATTRIBUTE_STANDING_SKILL);
	m_SkillAttributeIndexMap.emplace("ONLY_FOR_ALLIANCE", SKILL_ATTRIBUTE_ONLY_FOR_ALLIANCE);
	m_SkillAttributeIndexMap.emplace("CAN_USE_FOR_ME", SKILL_ATTRIBUTE_CAN_USE_FOR_ME);
	m_SkillAttributeIndexMap.emplace("ATTACK_SKILL", SKILL_ATTRIBUTE_ATTACK_SKILL);
	m_SkillAttributeIndexMap.emplace("MOVING_SKILL", SKILL_ATTRIBUTE_MOVING_SKILL);
	m_SkillAttributeIndexMap.emplace("HORSE_SKILL", SKILL_ATTRIBUTE_HORSE_SKILL);
	m_SkillAttributeIndexMap.emplace("TIME_INCREASE_SKILL", SKILL_ATTRIBUTE_TIME_INCREASE_SKILL);
	m_SkillAttributeIndexMap.emplace("PASSIVE", SKILL_ATTRIBUTE_PASSIVE);
	m_SkillAttributeIndexMap.emplace("CANNOT_LEVEL_UP", SKILL_ATTRIBUTE_CANNOT_LEVEL_UP);
	m_SkillAttributeIndexMap.emplace("ONLY_FOR_GUILD_WAR", SKILL_ATTRIBUTE_ONLY_FOR_GUILD_WAR);
	m_SkillAttributeIndexMap.emplace("CIRCLE_RANGE", SKILL_ATTRIBUTE_CIRCLE_RANGE);
	m_SkillAttributeIndexMap.emplace("SEARCH_TARGET", SKILL_ATTRIBUTE_SEARCH_TARGET);
#ifdef ENABLE_OFFICAL_FEATURES
	m_SkillAttributeIndexMap.emplace("IGNORE_BLOCK", SKILL_ATTRIBUTE_IGNORE_BLOCK);
#endif

	m_SkillNeedWeaponIndexMap.emplace("SWORD", SKILL_NEED_WEAPON_SWORD);
	m_SkillNeedWeaponIndexMap.emplace("DAGGER", SKILL_NEED_WEAPON_DAGGER);
	m_SkillNeedWeaponIndexMap.emplace("BOW", SKILL_NEED_WEAPON_BOW);
	m_SkillNeedWeaponIndexMap.emplace("TWO_HANDED", SKILL_NEED_WEAPON_TWO_HANDED);
	m_SkillNeedWeaponIndexMap.emplace("DOUBLE_SWORD", SKILL_NEED_WEAPON_DOUBLE_SWORD);
	m_SkillNeedWeaponIndexMap.emplace("BELL", SKILL_NEED_WEAPON_BELL);
	m_SkillNeedWeaponIndexMap.emplace("FAN", SKILL_NEED_WEAPON_FAN);
	m_SkillNeedWeaponIndexMap.emplace("ARROW", SKILL_NEED_WEAPON_ARROW);
	m_SkillNeedWeaponIndexMap.emplace("EMPTY_HAND", SKILL_NEED_WEAPON_EMPTY_HAND);
	m_SkillNeedWeaponIndexMap.emplace("MOUNT_SPEAR", SKILL_NEED_WEAPON_MOUNT_SPEAR);
#ifdef ENABLE_WOLFMAN_CHARACTER
	m_SkillNeedWeaponIndexMap.emplace("CLAW", SKILL_NEED_WEAPON_CLAW);
#endif
	m_SkillWeaponTypeIndexMap.emplace("SWORD", CItemData::WEAPON_SWORD);
	m_SkillWeaponTypeIndexMap.emplace("DAGGER", CItemData::WEAPON_DAGGER);
	m_SkillWeaponTypeIndexMap.emplace("BOW", CItemData::WEAPON_BOW);
	m_SkillWeaponTypeIndexMap.emplace("TWO_HANDED", CItemData::WEAPON_TWO_HANDED);
	m_SkillWeaponTypeIndexMap.emplace("DOUBLE_SWORD", CItemData::WEAPON_DAGGER);
	m_SkillWeaponTypeIndexMap.emplace("BELL", CItemData::WEAPON_BELL);
	m_SkillWeaponTypeIndexMap.emplace("FAN", CItemData::WEAPON_FAN);
	m_SkillWeaponTypeIndexMap.emplace("ARROW", CItemData::WEAPON_ARROW);
	m_SkillWeaponTypeIndexMap.emplace("MOUNT_SPEAR", CItemData::WEAPON_MOUNT_SPEAR);
#ifdef ENABLE_WOLFMAN_CHARACTER
	m_SkillWeaponTypeIndexMap.emplace("CLAW", CItemData::WEAPON_CLAW);
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	m_SkillWeaponTypeIndexMap.emplace("QUIVER", CItemData::WEAPON_QUIVER);
#endif
	SSkillData::ms_StatusNameMap.emplace("chain", POINT_NONE);
	SSkillData::ms_StatusNameMap.emplace("HR", POINT_HIT_RATE);
	SSkillData::ms_StatusNameMap.emplace("LV", POINT_LEVEL);
	SSkillData::ms_StatusNameMap.emplace("Level", POINT_LEVEL);
	SSkillData::ms_StatusNameMap.emplace("MaxHP", POINT_MAX_HP);
	SSkillData::ms_StatusNameMap.emplace("MaxSP", POINT_MAX_SP);
	SSkillData::ms_StatusNameMap.emplace("MinMWEP", POINT_MIN_WEP);
	SSkillData::ms_StatusNameMap.emplace("MaxMWEP", POINT_MAX_WEP);
	SSkillData::ms_StatusNameMap.emplace("MinWEP", POINT_MIN_WEP);
	SSkillData::ms_StatusNameMap.emplace("MaxWEP", POINT_MAX_WEP);
	SSkillData::ms_StatusNameMap.emplace("MinATK", POINT_MIN_ATK);
	SSkillData::ms_StatusNameMap.emplace("MaxATK", POINT_MAX_ATK);
	SSkillData::ms_StatusNameMap.emplace("ATKSPD", POINT_ATT_SPEED);
	SSkillData::ms_StatusNameMap.emplace("AttackPower", POINT_MIN_ATK);
	SSkillData::ms_StatusNameMap.emplace("AtkMin", POINT_MIN_ATK);
	SSkillData::ms_StatusNameMap.emplace("AtkMax", POINT_MAX_ATK);
	SSkillData::ms_StatusNameMap.emplace("DefencePower", POINT_DEF_GRADE);
	SSkillData::ms_StatusNameMap.emplace("DEF", POINT_DEF_GRADE);
	SSkillData::ms_StatusNameMap.emplace("MWEP", POINT_MAGIC_ATT_GRADE);

	SSkillData::ms_StatusNameMap.emplace("MagicAttackPower", POINT_MAGIC_ATT_GRADE);
	SSkillData::ms_StatusNameMap.emplace("INT", POINT_IQ);
	SSkillData::ms_StatusNameMap.emplace("STR", POINT_ST);
	SSkillData::ms_StatusNameMap.emplace("DEX", POINT_DX);
	SSkillData::ms_StatusNameMap.emplace("CON", POINT_HT);

	SSkillData::ms_StatusNameMap.emplace("minatk", POINT_MIN_ATK);
	SSkillData::ms_StatusNameMap.emplace("maxatk", POINT_MAX_ATK);
	SSkillData::ms_StatusNameMap.emplace("minmtk", POINT_MIN_WEP);
	SSkillData::ms_StatusNameMap.emplace("maxmtk", POINT_MAX_WEP);

	// GUILD_SKILL_DISPLAY_BUG_FIX
	SSkillData::ms_StatusNameMap.emplace("maxhp", POINT_MAX_HP);
	SSkillData::ms_StatusNameMap.emplace("maxsp", POINT_MAX_SP);
	SSkillData::ms_StatusNameMap.emplace("odef", POINT_DEF_GRADE);
	// END_OF_GUILD_SKILL_DISPLAY_BUG_FIX

	SSkillData::ms_StatusNameMap.emplace("minwep", POINT_MIN_WEP);
	SSkillData::ms_StatusNameMap.emplace("maxwep", POINT_MAX_WEP);
	SSkillData::ms_StatusNameMap.emplace("minmwep", POINT_MIN_MAGIC_WEP);
	SSkillData::ms_StatusNameMap.emplace("maxmwep", POINT_MAX_MAGIC_WEP);
	SSkillData::ms_StatusNameMap.emplace("lv", POINT_LEVEL);
	SSkillData::ms_StatusNameMap.emplace("ar", POINT_HIT_RATE);
	SSkillData::ms_StatusNameMap.emplace("iq", POINT_IQ);
	SSkillData::ms_StatusNameMap.emplace("str", POINT_ST);
	SSkillData::ms_StatusNameMap.emplace("dex", POINT_DX);
	SSkillData::ms_StatusNameMap.emplace("con", POINT_HT);

	/////

	SSkillData::ms_NewMinStatusNameMap.emplace("atk", POINT_MIN_ATK);
	SSkillData::ms_NewMinStatusNameMap.emplace("mtk", POINT_MIN_WEP);
	SSkillData::ms_NewMinStatusNameMap.emplace("wep", POINT_MIN_WEP);
	SSkillData::ms_NewMinStatusNameMap.emplace("lv", POINT_LEVEL);
	SSkillData::ms_NewMinStatusNameMap.emplace("ar", POINT_HIT_RATE);
	SSkillData::ms_NewMinStatusNameMap.emplace("iq", POINT_IQ);
	SSkillData::ms_NewMinStatusNameMap.emplace("str", POINT_ST);
	SSkillData::ms_NewMinStatusNameMap.emplace("dex", POINT_DX);
	SSkillData::ms_NewMinStatusNameMap.emplace("con", POINT_HT);

	SSkillData::ms_NewMaxStatusNameMap.emplace("atk", POINT_MAX_ATK);
	SSkillData::ms_NewMaxStatusNameMap.emplace("mtk", POINT_MAX_WEP);
	SSkillData::ms_NewMinStatusNameMap.emplace("wep", POINT_MAX_WEP);
	SSkillData::ms_NewMaxStatusNameMap.emplace("lv", POINT_LEVEL);
	SSkillData::ms_NewMaxStatusNameMap.emplace("ar", POINT_HIT_RATE);
	SSkillData::ms_NewMaxStatusNameMap.emplace("iq", POINT_IQ);
	SSkillData::ms_NewMaxStatusNameMap.emplace("str", POINT_ST);
	SSkillData::ms_NewMaxStatusNameMap.emplace("dex", POINT_DX);
	SSkillData::ms_NewMaxStatusNameMap.emplace("con", POINT_HT);

	m_PathNameMap.emplace("WARRIOR", "warrior");
	m_PathNameMap.emplace("ASSASSIN", "assassin");
	m_PathNameMap.emplace("SURA", "sura");
	m_PathNameMap.emplace("SHAMAN", "shaman");
	m_PathNameMap.emplace("PASSIVE", "passive");
#ifdef ENABLE_WOLFMAN_CHARACTER
	m_PathNameMap.emplace("78SKILL", "78skill");
	m_PathNameMap.emplace("WOLFMAN", "wolfman");
#endif
	m_PathNameMap.emplace("SUPPORT", "common/support");
	m_PathNameMap.emplace("GUILD", "common/guild");
	m_PathNameMap.emplace("HORSE", "common/horse");
}

CPythonSkill::~CPythonSkill() = default;

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t CPythonSkill::SSkillData::MELEE_SKILL_TARGET_RANGE = 170;

const std::string CPythonSkill::SSkillData::GetName() const
{
	return strName;
}

uint8_t CPythonSkill::SSkillData::GetType() const
{
	return byType;
}

BOOL CPythonSkill::SSkillData::IsMeleeSkill() const
{
	if (dwSkillAttribute & SKILL_ATTRIBUTE_MELEE_ATTACK)
		return TRUE;

	return FALSE;
}

BOOL CPythonSkill::SSkillData::IsChargeSkill() const
{
	if (dwSkillAttribute & SKILL_ATTRIBUTE_CHARGE_ATTACK)
		return TRUE;

	return FALSE;
}

BOOL CPythonSkill::SSkillData::IsOnlyForGuildWar() const
{
	if (dwSkillAttribute & SKILL_ATTRIBUTE_ONLY_FOR_GUILD_WAR)
		return TRUE;

	return FALSE;
}

uint32_t CPythonSkill::SSkillData::GetTargetRange() const
{
	if (dwSkillAttribute & SKILL_ATTRIBUTE_MELEE_ATTACK)
		return MELEE_SKILL_TARGET_RANGE;

	if (dwSkillAttribute & SKILL_ATTRIBUTE_CHARGE_ATTACK)
		return MELEE_SKILL_TARGET_RANGE;

	return dwTargetRange;
}

BOOL CPythonSkill::SSkillData::CanChangeDirection() const
{
	return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_CAN_CHANGE_DIRECTION);
}

BOOL CPythonSkill::SSkillData::IsFanRange() const
{
	return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_FAN_RANGE);
}

BOOL CPythonSkill::SSkillData::IsCircleRange() const
{
	return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_CIRCLE_RANGE);
}

BOOL CPythonSkill::SSkillData::IsAutoSearchTarget() const
{
	return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_SEARCH_TARGET);
}

#ifdef ENABLE_OFFICAL_FEATURES
BOOL CPythonSkill::SSkillData::IsIgnoreBlock() const
{
	return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_IGNORE_BLOCK);
}
#endif

BOOL CPythonSkill::SSkillData::IsNeedCorpse() const
{
	return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_NEED_CORPSE);
}

BOOL CPythonSkill::SSkillData::IsNeedTarget() const
{
	return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_NEED_TARGET);
}

BOOL CPythonSkill::SSkillData::IsToggleSkill() const
{
	return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_TOGGLE);
}

BOOL CPythonSkill::SSkillData::IsUseHPSkill() const
{
	return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_USE_HP);
}

BOOL CPythonSkill::SSkillData::IsStandingSkill() const
{
	return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_STANDING_SKILL);
}

BOOL CPythonSkill::SSkillData::CanUseWeaponType(uint32_t dwWeaponType) const
{
	if (!(dwSkillAttribute & SKILL_ATTRIBUTE_WEAPON_LIMITATION))
		return TRUE;

	return 0 != (dwNeedWeapon & (1 << dwWeaponType));
}

BOOL CPythonSkill::SSkillData::IsOnlyForAlliance() const
{
	return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_ONLY_FOR_ALLIANCE);
}

BOOL CPythonSkill::SSkillData::CanUseForMe() const
{
	return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_CAN_USE_FOR_ME);
}

BOOL CPythonSkill::SSkillData::CanUseIfNotEnough() const
{
	return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_CAN_USE_IF_NOT_ENOUGH);
}

BOOL CPythonSkill::SSkillData::IsNeedEmptyBottle() const
{
	return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_NEED_EMPTY_BOTTLE);
}

BOOL CPythonSkill::SSkillData::IsNeedPoisonBottle() const
{
	return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_NEED_POISON_BOTTLE);
}

BOOL CPythonSkill::SSkillData::IsNeedBow() const
{
	if (!(dwSkillAttribute & SKILL_ATTRIBUTE_WEAPON_LIMITATION))
		return FALSE;

	return 0 != (dwNeedWeapon & SKILL_NEED_WEAPON_BOW);
}

BOOL CPythonSkill::SSkillData::IsHorseSkill() const
{
	return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_HORSE_SKILL);
}

BOOL CPythonSkill::SSkillData::IsMovingSkill() const
{
	return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_MOVING_SKILL);
}

BOOL CPythonSkill::SSkillData::IsAttackSkill() const
{
	return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_ATTACK_SKILL);
}

BOOL CPythonSkill::SSkillData::IsTimeIncreaseSkill() const
{
	return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_TIME_INCREASE_SKILL);
}

bool CPythonSkill::SSkillData::GetState(const char * c_szStateName, int * piState, int iMinMaxType) const
{
	std::map<std::string, uint32_t> * pStatusNameMap = nullptr;

	switch (iMinMaxType)
	{
		case VALUE_TYPE_FREE:
			pStatusNameMap = &ms_StatusNameMap;
			break;
		case VALUE_TYPE_MIN:
			pStatusNameMap = &ms_NewMinStatusNameMap;
			break;
		case VALUE_TYPE_MAX:
			pStatusNameMap = &ms_NewMaxStatusNameMap;
			break;
		default:
			return false;
	}

	auto it = pStatusNameMap->find(c_szStateName);

	if (it != pStatusNameMap->end())
		*piState = CPythonPlayer::Instance().GetStatus(it->second);
	else if (0 == strncmp(c_szStateName, "JeungJi", 7))
	{
		*piState = 0;

		// 증지술 임시 제외
		/*
				if (0 != ms_dwTimeIncreaseSkillNumber)
				{
					uint32_t dwSkillSlotIndex;
					if (!CPythonPlayer::Instance().FindSkillSlotIndexBySkillIndex(ms_dwTimeIncreaseSkillNumber, &dwSkillSlotIndex))
						return false;

					*piState = CPythonPlayer::Instance().GetSkillLevel(dwSkillSlotIndex);
				}
		*/
	}

	return true;
}

float CPythonSkill::SSkillData::ProcessFormula(CPoly * pPoly, float fSkillLevel, int iMinMaxType) const
{
	if (pPoly->Analyze())
	{
		for (uint32_t i = 0; i < pPoly->GetVarCount(); ++i)
		{
			const char * c_szVarName = pPoly->GetVarName(i);
			float fState;
			if (!strcmp("SkillPoint", c_szVarName) || !strcmp("k", c_szVarName))
				fState = fSkillLevel;
			else
			{
				int iState;
				if (!GetState(c_szVarName, &iState, iMinMaxType))
					return 0.0f;

				fState = float(iState);

				if (!strcmp("ar", c_szVarName))
					fState /= 100.0f;
			}
			pPoly->SetVar(c_szVarName, fState);
		}
	}
	else
	{
		TraceError("skillGetAffect - Strange Formula [%s]", strName.c_str());
		return 0.0f;
	}

	return pPoly->Eval();
}

const char * CPythonSkill::SSkillData::GetAffectDescription(uint32_t dwIndex, float fSkillLevel)
{
	if (dwIndex >= AffectDataVector.size())
		return nullptr;

	const std::string & c_rstrAffectDescription = AffectDataVector[dwIndex].strAffectDescription;
	const std::string & c_rstrAffectMinFormula = AffectDataVector[dwIndex].strAffectMinFormula;
	const std::string & c_rstrAffectMaxFormula = AffectDataVector[dwIndex].strAffectMaxFormula;

	CPoly minPoly;
	CPoly maxPoly;
	minPoly.SetStr(c_rstrAffectMinFormula);
	maxPoly.SetStr(c_rstrAffectMaxFormula);

	// OVERWRITE_SKILLPROTO_POLY
	float fMinValue = ProcessFormula(&minPoly, fSkillLevel);
	float fMaxValue = ProcessFormula(&maxPoly, fSkillLevel);

	if (fMinValue < 0.0)
		fMinValue = -fMinValue;
	if (fMaxValue < 0.0)
		fMaxValue = -fMaxValue;

	if (CP_ARABIC == GetDefaultCodePage())
	{
		// #0000870: [M2AE] 한국어 모드일때 특정 아랍어 문장에서 크래쉬 발생
		static std::string strDescription;
		strDescription = c_rstrAffectDescription;
		int first = strDescription.find("%.0f");
		if (first >= 0)
		{
			fMinValue = floorf(fMinValue);

			char szMinValue[256];
			_snprintf(szMinValue, sizeof(szMinValue), "%.0f", fMinValue);
			strDescription.replace(first, 4, szMinValue);

			int second = strDescription.find("%.0f", first);
			if (second >= 0)
			{
				fMaxValue = floorf(fMaxValue);

				char szMaxValue[256];
				_snprintf(szMaxValue, sizeof(szMaxValue), "%.0f", fMaxValue);
				strDescription.replace(second, 4, szMaxValue);
			}
		}
		return strDescription.c_str();
	}
	if (strstr(c_rstrAffectDescription.c_str(), "%.0f"))
	{
		fMinValue = floorf(fMinValue);
		fMaxValue = floorf(fMaxValue);
	}

	static char szDescription[64 + 1];
	_snprintf(szDescription, sizeof(szDescription), c_rstrAffectDescription.c_str(), fMinValue, fMaxValue);

	return szDescription;
}

uint32_t CPythonSkill::SSkillData::GetSkillCoolTime(float fSkillPoint) const
{
	if (strCoolTimeFormula.empty())
		return 0;

	CPoly poly;
	poly.SetStr(strCoolTimeFormula);

	return uint32_t(ProcessFormula(&poly, fSkillPoint));
}


uint32_t CPythonSkill::SSkillData::GetTargetCount(float fSkillPoint) const
{
	if (strTargetCountFormula.empty())
		return 0;

	CPoly poly;
	poly.SetStr(strTargetCountFormula);

	return uint32_t(ProcessFormula(&poly, fSkillPoint));
}

uint32_t CPythonSkill::SSkillData::GetSkillMotionIndex(int iGrade) const
{
	if (-1 != iGrade && SKILL_EFFECT_UPGRADE_ENABLE)
	{
		assert(iGrade >= 0 && iGrade < SKILL_EFFECT_COUNT);
		/*
		if (SKILL_GRADE_COUNT == iGrade)
		{
			return GradeData[SKILL_GRADE_COUNT-1].wMotionIndex;
		}
		else */
		if (iGrade >= 0 && iGrade < SKILL_EFFECT_COUNT)
			return GradeData[iGrade].wMotionIndex;
	}

	return wMotionIndex;
}

uint8_t CPythonSkill::SSkillData::GetMaxLevel() const
{
	return byMaxLevel;
}

uint8_t CPythonSkill::SSkillData::GetLevelUpPoint() const
{
	return byLevelUpPoint;
}

BOOL CPythonSkill::SSkillData::IsNoMotion() const
{
	return bNoMotion;
}

bool CPythonSkill::SSkillData::IsCanUseSkill() const
{
	if (0 != (dwSkillAttribute & SKILL_ATTRIBUTE_PASSIVE))
		return false;

	return true;
}

uint32_t CPythonSkill::SSkillData::GetMotionLoopCount(float fSkillPoint) const
{
	if (strMotionLoopCountFormula.empty())
		return 0;

	CPoly poly;
	poly.SetStr(strMotionLoopCountFormula);

	return uint32_t(ProcessFormula(&poly, fSkillPoint));
}

int CPythonSkill::SSkillData::GetNeedSP(float fSkillPoint) const
{
	if (strNeedSPFormula.empty())
		return 0;

	CPoly poly;
	poly.SetStr(strNeedSPFormula);

	return int(ProcessFormula(&poly, fSkillPoint));
}

uint32_t CPythonSkill::SSkillData::GetContinuationSP(float fSkillPoint) const
{
	if (strContinuationSPFormula.empty())
		return 0;

	CPoly poly;
	poly.SetStr(strContinuationSPFormula);

	return uint32_t(ProcessFormula(&poly, fSkillPoint));
}

uint32_t CPythonSkill::SSkillData::GetDuration(float fSkillPoint) const
{
	if (strDuration.empty())
		return 0;

	CPoly poly;
	poly.SetStr(strDuration);

	return uint32_t(ProcessFormula(&poly, fSkillPoint));
}

CPythonSkill::SSkillData::SSkillData()
{
	byType = SKILL_TYPE_ACTIVE;
	byMaxLevel = 20;
	byLevelUpPoint = 1;
	dwSkillAttribute = 0;
	dwNeedWeapon = 0;
	dwTargetRange = 0;
	strCoolTimeFormula = "";
	strMotionLoopCountFormula = "";
	strNeedSPFormula = "";
	strContinuationSPFormula = "";
	isRequirement = FALSE;
	strRequireSkillName = "";
	byRequireSkillLevel = 0;
	strDuration = "";
	byLevelLimit = 0;
	bNoMotion = FALSE;

	strName = "";
	pImage = nullptr;

	for (int j = 0; j < SKILL_GRADE_COUNT; ++j)
	{
		TGradeData & rGradeData = GradeData[j];
		rGradeData.strName = "";
		rGradeData.pImage = nullptr;
		rGradeData.wMotionIndex = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef ENABLE_GROWTH_PET_SYSTEM
bool CPythonSkillPet::RegisterSkillPet(const char* c_szFileName)
{
	const VOID* pvData;
	CMappedFile kFile;
	if (!CEterPackManager::Instance().Get(kFile, c_szFileName, &pvData))
		return false;

	CMemoryTextFileLoader textFileLoader;
	textFileLoader.Bind(kFile.Size(), pvData);

	CTokenVector TokenVector;
	for (uint32_t i = 0; i < textFileLoader.GetLineCount() - 1; ++i)
	{
		if (!textFileLoader.SplitLine(i, &TokenVector, "\t"))
			return false;

		if (DESCPET_TOKEN_TYPE_MAX_NUM > TokenVector.size())
			return false;

		uint32_t iSkillIndex = atoi(TokenVector[DESCPET_TOKEN_TYPE_VNUM].c_str());
		if (iSkillIndex == 0)
			continue;

		m_SkillDataPetMap.emplace(iSkillIndex, TSkillDataPet());
		TSkillDataPet& rSkillData = m_SkillDataPetMap[iSkillIndex];
		rSkillData.dwSkillIndex = iSkillIndex;
		rSkillData.strName = TokenVector[DESCPET_TOKEN_TYPE_NAME];

		char szIconFileNameHeader[64 + 1];
		_snprintf(szIconFileNameHeader, sizeof(szIconFileNameHeader), "%sskill/pet/%s.sub", g_strImagePath.c_str(), TokenVector[DESCPET_TOKEN_TYPE_ICON_NAME].c_str());

		rSkillData.strIconFileName = szIconFileNameHeader;

		std::map<std::string, uint32_t>::iterator itor = m_SkillPetTypeIndexMap.begin();
		for (; itor != m_SkillPetTypeIndexMap.end(); ++itor)
		{
			if (TokenVector[DESCPET_TOKEN_TYPE].compare(itor->first) == 0)
				rSkillData.byType = itor->second;
		}
		rSkillData.strDescription = TokenVector[DESCPET_TOKEN_TYPE_DESCRIPTION];
		rSkillData.dwskilldelay = atoi(TokenVector[DESCPET_TOKEN_TYPE_DELAY].c_str());
		rSkillData.pImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(szIconFileNameHeader);
	}

	return true;
}

void CPythonSkillPet::__RegisterNormalIconImage(TSkillDataPet& rData, const char* c_szHeader, const char* c_szImageName)
{
	std::string strFileName = "";
	strFileName += c_szHeader;
	strFileName += c_szImageName;
	strFileName += ".sub";
	rData.pImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(strFileName.c_str());
}

void CPythonSkillPet::Destroy()
{
	m_SkillDataPetMap.clear();
}

CPythonSkillPet::SSkillDataPet::SSkillDataPet()
{
	dwSkillIndex = 0;
	strName = "";
	strIconFileName = "";
	byType = 0;
	strDescription = "";
	dwskilldelay = 0;
	pImage = nullptr;
}

CPythonSkillPet::CPythonSkillPet()
{
	petslot[0] = 0;
	petslot[1] = 0;
	petslot[2] = 0;

	m_SkillPetTypeIndexMap.emplace(std::map<std::string, uint32_t>::value_type("EMPTY", SKILLPET_TYPE_NONE));
	m_SkillPetTypeIndexMap.emplace(std::map<std::string, uint32_t>::value_type("PASSIVE", SKILLPET_TYPE_PASSIVE));
	m_SkillPetTypeIndexMap.emplace(std::map<std::string, uint32_t>::value_type("AUTO", SKILLPET_TYPE_AUTO));
}

CPythonSkillPet::~CPythonSkillPet()
{
}

void CPythonSkillPet::SetSkillbySlot(int slot, int skillIndex)
{
	petslot[slot] = skillIndex;
}

BOOL CPythonSkillPet::GetSkillData(uint32_t dwSkillIndex, TSkillDataPet** ppSkillData)
{
	TSkillDataPetMap::iterator it = m_SkillDataPetMap.find(dwSkillIndex);

	if (m_SkillDataPetMap.end() == it)
		return false;

	*ppSkillData = &(it->second);
	return true;
}

BOOL CPythonSkillPet::GetSkillIndex(int slot, int* skillIndex)
{
	*skillIndex = petslot[slot];
	return true;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

PyObject * skillSetPathName(PyObject * poSelf, PyObject * poArgs)
{
	char * szPathName;
	if (!PyTuple_GetString(poArgs, 0, &szPathName))
		return Py_BadArgument();

	CPythonSkill::Instance().SetPathName(szPathName);
	return Py_BuildNone();
}

PyObject * skillRegisterSkill(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();
	char * szFileName;
	if (!PyTuple_GetString(poArgs, 1, &szFileName))
		return Py_BadArgument();

	std::string strFullFileName = CPythonSkill::Instance().GetPathName();
	strFullFileName += szFileName;

	if (!CPythonSkill::Instance().RegisterSkill(iSkillIndex, strFullFileName.c_str()))
		return Py_BuildException("skill.RegisterSkill - Failed to find skill data file : %d, %s", iSkillIndex, strFullFileName.c_str());

	return Py_BuildNone();
}

PyObject * skillLoadSkillData(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildNone();
}

PyObject * skillRegisterSkillDesc(PyObject * poSelf, PyObject * poArgs)
{
	char * szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BadArgument();

	if (!CPythonSkill::Instance().RegisterSkillDesc(szFileName))
		return Py_BuildException("Failed to load Skill Desc");

	return Py_BuildNone();
}

PyObject * skillClearSkillData(PyObject * poSelf, PyObject * poArgs)
{
	CPythonSkill::SSkillData::ms_dwTimeIncreaseSkillNumber = 0;
	return Py_BuildNone();
}

PyObject * skillGetSkillName(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	int iSkillGrade = -1;
	PyTuple_GetInteger(poArgs, 1, &iSkillGrade);

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.GetSkillName - Failed to find skill by %d", iSkillIndex);

	if (-1 != iSkillGrade)
		if (iSkillGrade >= 0 && iSkillGrade < CPythonSkill::SKILL_GRADE_COUNT)
			return Py_BuildValue("s", c_pSkillData->GradeData[iSkillGrade].strName.c_str());

	return Py_BuildValue("s", c_pSkillData->strName.c_str());
}

PyObject * skillGetSkillDescription(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.GetSkillDescription - Failed to find skill by %d", iSkillIndex);

	return Py_BuildValue("s", c_pSkillData->strDescription.c_str());
}

PyObject * skillGetSkillType(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.GetSkillType - Failed to find skill by %d", iSkillIndex);

	return Py_BuildValue("i", c_pSkillData->byType);
}

PyObject * skillGetSkillConditionDescriptionCount(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.GetSkillConditionDescriptionCount - Failed to find skill by %d", iSkillIndex);

	return Py_BuildValue("i", c_pSkillData->ConditionDataVector.size());
}

PyObject * skillGetSkillConditionDescription(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	int iConditionIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iConditionIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.GetSkillConditionDescription() - Failed to find skill by %d", iSkillIndex);

	if (iConditionIndex >= c_pSkillData->ConditionDataVector.size())
		return Py_BuildValue("None");

	return Py_BuildValue("s", c_pSkillData->ConditionDataVector[iConditionIndex].c_str());
}

PyObject * skillGetSkillAffectDescriptionCount(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.GetSkillAffectDescriptionCount - Failed to find skill by %d", iSkillIndex);

	return Py_BuildValue("i", c_pSkillData->AffectDataVector.size());
}

PyObject * skillGetSkillAffectDescription(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	int iAffectIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAffectIndex))
		return Py_BadArgument();

	float fSkillPoint;
	if (!PyTuple_GetFloat(poArgs, 2, &fSkillPoint))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.GetSkillAffectDescription - Failed to find skill by %d", iSkillIndex);

	return Py_BuildValue("s", c_pSkillData->GetAffectDescription(iAffectIndex, fSkillPoint));
}

PyObject * skillGetSkillCoolTime(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	float fSkillPoint;
	if (!PyTuple_GetFloat(poArgs, 1, &fSkillPoint))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.GetSkillCoolTime - Failed to find skill by %d", iSkillIndex);

	//@fixme438
	uint32_t dwSkillCoolTime = c_pSkillData->GetSkillCoolTime(fSkillPoint);
	int iCastingSpeed = CPythonPlayer::Instance().GetStatus(POINT_CASTING_SPEED);

	int iSpd = 100 - iCastingSpeed;
	if (iSpd > 0)
		iSpd = 100 + iSpd;
	else if (iSpd < 0)
		iSpd = 10000 / (100 - iSpd);
	else
		iSpd = 100;

	dwSkillCoolTime = dwSkillCoolTime * iSpd / 100;

	return Py_BuildValue("i", dwSkillCoolTime);
	//return Py_BuildValue("i", c_pSkillData->GetSkillCoolTime(fSkillPoint));	//Original
	//@end_fixme438
}

PyObject * skillGetSkillNeedSP(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	float fSkillPoint;
	if (!PyTuple_GetFloat(poArgs, 1, &fSkillPoint))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.GetSkillNeedSP Failed to find skill by %d", iSkillIndex);

	return Py_BuildValue("i", c_pSkillData->GetNeedSP(fSkillPoint));
}

PyObject * skillGetSkillContinuationSP(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	float fSkillPoint;
	if (!PyTuple_GetFloat(poArgs, 1, &fSkillPoint))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.GetSkillContinuationSP - Failed to find skill by %d", iSkillIndex);

	return Py_BuildValue("i", c_pSkillData->GetContinuationSP(fSkillPoint));
}

PyObject * skillGetSkillMaxLevel(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.GetSkillMaxLevel - Failed to find skill by %d", iSkillIndex);

	return Py_BuildValue("i", c_pSkillData->GetMaxLevel());
}

PyObject * skillGetSkillLevelUpPoint(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.GetSkillLevelUpPoint - Failed to find skill by %d", iSkillIndex);

	return Py_BuildValue("i", c_pSkillData->GetLevelUpPoint());
}

PyObject * skillGetSkillLevelLimit(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.GetSkillLevelLimit - Failed to find skill by %d", iSkillIndex);

	return Py_BuildValue("i", c_pSkillData->byLevelLimit);
}

PyObject * skillIsSkillRequirement(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.IsSkillRequirement - Failed to find skill by %d", iSkillIndex);

	if (c_pSkillData->isRequirement)
	{
		CPythonSkill::SSkillData * pRequireSkillData;
		if (!CPythonSkill::Instance().GetSkillDataByName(c_pSkillData->strRequireSkillName.c_str(), &pRequireSkillData))
		{
			TraceError("skill.IsSkillRequirement - Failed to find skill : [%d/%s] can't find [%s]\n", c_pSkillData->dwSkillIndex,
					   c_pSkillData->strName.c_str(), c_pSkillData->strRequireSkillName.c_str());
			return Py_BuildValue("i", FALSE);
		}

		uint32_t dwRequireSkillSlotIndex;
		if (!CPythonPlayer::Instance().FindSkillSlotIndexBySkillIndex(pRequireSkillData->dwSkillIndex, &dwRequireSkillSlotIndex))
			return Py_BuildValue("i", FALSE);
	}

	return Py_BuildValue("i", c_pSkillData->isRequirement);
}

PyObject * skillGetSkillRequirementData(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.GetSkillRequirementData - Failed to find skill by %d", iSkillIndex);

	CPythonSkill::SSkillData * pRequireSkillData;
	if (!CPythonSkill::Instance().GetSkillDataByName(c_pSkillData->strRequireSkillName.c_str(), &pRequireSkillData))
		return Py_BuildValue("si", 0, "None", 0);

	auto ireqLevel =
		static_cast<int>(ceil(float(c_pSkillData->byRequireSkillLevel) / std::max<float>(1, pRequireSkillData->byLevelUpPoint)));
	return Py_BuildValue("si", c_pSkillData->strRequireSkillName.c_str(), ireqLevel);
}

PyObject * skillGetSkillRequireStatCount(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.GetSkillRequireStatCount - Failed to find skill by %d", iSkillIndex);

	return Py_BuildValue("i", c_pSkillData->RequireStatDataVector.size());
}

PyObject * skillGetSkillRequireStatData(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();
	int iStatIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iStatIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.GetSkillRequireStatData - Failed to find skill by %d", iSkillIndex);

	if (iStatIndex >= c_pSkillData->RequireStatDataVector.size())
		return Py_BuildValue("ii", 0, 0);

	const CPythonSkill::TRequireStatData & c_rRequireStatData = c_pSkillData->RequireStatDataVector[iStatIndex];

	return Py_BuildValue("ii", c_rRequireStatData.byPoint, c_rRequireStatData.byLevel);
}

PyObject * skillCanLevelUpSkill(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();
	int iSkillLevel;
	if (!PyTuple_GetInteger(poArgs, 1, &iSkillLevel))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.CanLevelUpSkill - Failed to find skill by %d", iSkillIndex);

	if (iSkillLevel >= c_pSkillData->GetMaxLevel())
	{
		TraceError("PythonSkill::skillCanLevelUpSkill(PyObject * poSelf, PyObject * poArgs) Skill %d is already at Max Level. Current Level %d (maximum Skillable %d)", (iSkillIndex, iSkillLevel, c_pSkillData->GetMaxLevel()));
		return Py_BuildValue("i", FALSE);
	}
	if (c_pSkillData->isRequirement)
	{
		CPythonSkill::SSkillData * pRequireSkillData;
		if (CPythonSkill::Instance().GetSkillDataByName(c_pSkillData->strRequireSkillName.c_str(), &pRequireSkillData))
		{
			uint32_t dwRequireSkillSlotIndex;
			if (CPythonPlayer::Instance().FindSkillSlotIndexBySkillIndex(pRequireSkillData->dwSkillIndex, &dwRequireSkillSlotIndex))
			{
				int iSkillGrade = CPythonPlayer::Instance().GetSkillGrade(dwRequireSkillSlotIndex);
				int iSkillLevel = CPythonPlayer::Instance().GetSkillLevel(dwRequireSkillSlotIndex);
				if (iSkillGrade <= 0)
				{
					if (iSkillLevel < c_pSkillData->byRequireSkillLevel)
					{
						TraceError("PythonSkill::skillCanLevelUpSkill iSkillLevel is smaller than c_pSkillData->byRequireSkillLevel", ( iSkillLevel, c_pSkillData->byRequireSkillLevel));
						return Py_BuildValue("i", FALSE);
					}
				}
			}
		}
	}

	for (auto & c_rRequireStatData : c_pSkillData->RequireStatDataVector)
	{
		if (CPythonPlayer::Instance().GetStatus(c_rRequireStatData.byPoint) < c_rRequireStatData.byLevel)
			return Py_BuildValue("i", FALSE);
	}

	if (0 != (c_pSkillData->dwSkillAttribute & CPythonSkill::SKILL_ATTRIBUTE_CANNOT_LEVEL_UP))
		return Py_BuildValue("i", FALSE);

	return Py_BuildValue("i", TRUE);
}

PyObject * skillCheckRequirementSueccess(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.CheckRequirementSueccess - Failed to find skill by %d", iSkillIndex);

	if (c_pSkillData->isRequirement)
	{
		CPythonSkill::SSkillData * pRequireSkillData;
		if (CPythonSkill::Instance().GetSkillDataByName(c_pSkillData->strRequireSkillName.c_str(), &pRequireSkillData))
		{
			uint32_t dwRequireSkillSlotIndex;
			if (CPythonPlayer::Instance().FindSkillSlotIndexBySkillIndex(pRequireSkillData->dwSkillIndex, &dwRequireSkillSlotIndex))
			{
				int iSkillGrade = CPythonPlayer::Instance().GetSkillGrade(dwRequireSkillSlotIndex);
				int iSkillLevel = CPythonPlayer::Instance().GetSkillLevel(dwRequireSkillSlotIndex);
				if (iSkillGrade <= 0)
					if (iSkillLevel < c_pSkillData->byRequireSkillLevel)
						return Py_BuildValue("i", FALSE);
			}
		}
	}

	return Py_BuildValue("i", TRUE);
}

PyObject * skillGetNeedCharacterLevel(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.GetNeedCharacterLevel - Failed to find skill by %d", iSkillIndex);

	auto itor = c_pSkillData->RequireStatDataVector.begin();
	for (; itor != c_pSkillData->RequireStatDataVector.end(); ++itor)
	{
		const CPythonSkill::TRequireStatData & c_rRequireStatData = *itor;

		if (POINT_LEVEL == c_rRequireStatData.byPoint)
			return Py_BuildValue("i", c_rRequireStatData.byLevel);
	}

	return Py_BuildValue("i", 0);
}

PyObject * skillIsToggleSkill(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.IsToggleSkill - Failed to find skill by %d", iSkillIndex);

	return Py_BuildValue("i", c_pSkillData->IsToggleSkill());
}

PyObject * skillIsUseHPSkill(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.IsUseHPSkill - Failed to find skill by %d", iSkillIndex);

	return Py_BuildValue("i", c_pSkillData->IsUseHPSkill());
}

PyObject * skillIsStandingSkill(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.IsStandingSkill - Failed to find skill by %d", iSkillIndex);

	return Py_BuildValue("i", c_pSkillData->IsStandingSkill());
}

PyObject * skillCanUseSkill(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.CanUseSkill - Failed to find skill by %d", iSkillIndex);

	return Py_BuildValue("i", c_pSkillData->IsCanUseSkill());
}

PyObject * skillIsLevelUpSkill(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.IsLevelUpSkill - Failed to find skill by %d", iSkillIndex);

	// 모두 레벨업 가능

	return Py_BuildValue("i", TRUE);
}

PyObject * skillGetIconName(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildValue("s", "");

	return Py_BuildValue("s", c_pSkillData->strIconFileName.c_str());
}

PyObject * skillGetIconImage(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildValue("i", 0); // 익셉션을 내는 대신 0을 리턴한다.

	return Py_BuildValue("i", c_pSkillData->pImage);
}


PyObject * skillGetIconInstance(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.GetIconInstance - Failed to find skill by %d", iSkillIndex);

	CGraphicImageInstance * pImageInstance = CGraphicImageInstance::New();
	pImageInstance->SetImagePointer(c_pSkillData->pImage);

	return Py_BuildValue("i", pImageInstance);
}

PyObject * skillGetIconImageNew(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	int iGradeIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iGradeIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildValue("i", 0); // 익셉션을 내는 대신 0을 리턴한다.

	if (iGradeIndex < 0)
		iGradeIndex = 0;

	if (iGradeIndex >= CPythonSkill::SKILL_GRADE_COUNT)
		iGradeIndex = CPythonSkill::SKILL_GRADE_COUNT - 1;

	return Py_BuildValue("i", c_pSkillData->GradeData[iGradeIndex].pImage);
}

PyObject * skillGetIconInstanceNew(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	int iGradeIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iGradeIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.GetIconInstanceNew - Failed to find skill by %d", iSkillIndex);

	if (iGradeIndex < 0)
		iGradeIndex = 0;

	if (iGradeIndex >= CPythonSkill::SKILL_GRADE_COUNT)
		iGradeIndex = CPythonSkill::SKILL_GRADE_COUNT - 1;

	CGraphicImageInstance * pImageInstance = CGraphicImageInstance::New();
	pImageInstance->SetImagePointer(c_pSkillData->GradeData[iGradeIndex].pImage);

	return Py_BuildValue("i", pImageInstance);
}

PyObject * skillDeleteIconInstance(PyObject * poSelf, PyObject * poArgs)
{
	int iHandle;
	if (!PyTuple_GetInteger(poArgs, 0, &iHandle))
		return Py_BadArgument();

	CGraphicImageInstance::Delete((CGraphicImageInstance *) iHandle);
	return Py_BuildNone();
}

PyObject * skillGetGradeData(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	int iGradeIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iGradeIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.GetGradeData - Failed to find skill by %d", iSkillIndex);

	if (iGradeIndex < 0 || iGradeIndex >= CPythonSkill::SKILL_GRADE_COUNT)
		return Py_BuildException("Strange grade index [%d]", iSkillIndex, iGradeIndex);

	return Py_BuildValue("i", c_pSkillData->GradeData[iGradeIndex]);
}

PyObject * skillGetNewAffectDataCount(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.GetNewAffectDataCount - Failed to find skill by %d", iSkillIndex);

	return Py_BuildValue("i", c_pSkillData->AffectDataNewVector.size());
}

PyObject * skillGetNewAffectData(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();
	int iAffectIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAffectIndex))
		return Py_BadArgument();
	float fSkillLevel;
	if (!PyTuple_GetFloat(poArgs, 2, &fSkillLevel))
		return Py_BadArgument();

	CPythonSkill::SSkillData * pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &pSkillData))
		return Py_BuildException("skill.GetNewAffectData - Failed to find skill by %d", iSkillIndex);

	if (iAffectIndex < 0 || iAffectIndex >= pSkillData->AffectDataNewVector.size())
		return Py_BuildException(" skill.GetNewAffectData - Strange AffectIndex %d", iAffectIndex);

	CPythonSkill::TAffectDataNew & rAffectData = pSkillData->AffectDataNewVector[iAffectIndex];

	CPoly minPoly;
	CPoly maxPoly;
	minPoly.SetRandom(CPoly::RANDOM_TYPE_FORCE_MIN);
	maxPoly.SetRandom(CPoly::RANDOM_TYPE_FORCE_MAX);
	minPoly.SetStr(rAffectData.strPointPoly);
	maxPoly.SetStr(rAffectData.strPointPoly);
	float fMinValue = pSkillData->ProcessFormula(&minPoly, fSkillLevel, CPythonSkill::VALUE_TYPE_MIN);
	float fMaxValue = pSkillData->ProcessFormula(&maxPoly, fSkillLevel, CPythonSkill::VALUE_TYPE_MAX);

	return Py_BuildValue("sff", rAffectData.strPointType.c_str(), fMinValue, fMaxValue);
}

PyObject * skillGetDuration(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();
	float fSkillLevel;
	if (!PyTuple_GetFloat(poArgs, 1, &fSkillLevel))
		return Py_BadArgument();

	CPythonSkill::SSkillData * c_pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildException("skill.GetDuration - Failed to find skill by %d", iSkillIndex);

	return Py_BuildValue("i", c_pSkillData->GetDuration(fSkillLevel));
}

PyObject * skillTEST(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildNone();
}

#ifdef ENABLE_GROWTH_PET_SYSTEM
PyObject* skillSetPetSkillSlot(PyObject* poSelf, PyObject* poArgs)
{
	int slot;
	int skillIndex;

	if (!PyTuple_GetInteger(poArgs, 0, &slot))
		return Py_BadArgument();

	if (!PyTuple_GetInteger(poArgs, 1, &skillIndex))
		return Py_BadArgument();

	CPythonSkillPet::Instance().SetSkillbySlot(slot, skillIndex);
	return Py_BuildValue("i", 0);
}

PyObject* skillGetPetSkillIconImage(PyObject* poSelf, PyObject* poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkillPet::SSkillDataPet* c_pSkillData;
	if (!CPythonSkillPet::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", c_pSkillData->pImage);
}

PyObject* skillGetPetSkillbySlot(PyObject* poSelf, PyObject* poArgs)
{
	int slot;
	int skillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &slot))
		return Py_BadArgument();

	if (slot > 2)
		return Py_BadArgument();

	if (!CPythonSkillPet::Instance().GetSkillIndex(slot, &skillIndex))
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", skillIndex);
}

PyObject* skillGetPetEmptySkill(PyObject* poSelf, PyObject* poArgs)
{
	CGraphicImage* noskillbtn;
	char szIconFileNameHeader[64 + 1];
	_snprintf(szIconFileNameHeader, sizeof(szIconFileNameHeader), "d:/ymir work/ui/pet/skill_button/skill_enable_button.sub");
	noskillbtn = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(szIconFileNameHeader);
	return Py_BuildValue("i", noskillbtn);
}

PyObject* skillGetPetSkillIconPath(PyObject* poSelf, PyObject* poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkillPet::SSkillDataPet* c_pSkillData;
	if (!CPythonSkillPet::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildValue("s", "");

	return Py_BuildValue("s", c_pSkillData->strIconFileName.c_str());
}

PyObject* skillGetPetSkillInfo(PyObject* poSelf, PyObject* poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BadArgument();

	CPythonSkillPet::SSkillDataPet* c_pSkillData;
	if (!CPythonSkillPet::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
		return Py_BuildValue("ssii", "", "", 0, 0);

	//pet_skill_name, pet_skill_desc, pet_skill_use_type, pet_skill_cool_time
	return Py_BuildValue("ssii", c_pSkillData->strName.c_str(), c_pSkillData->strDescription.c_str(), c_pSkillData->byType, c_pSkillData->dwskilldelay);
}
#endif

#ifdef ENABLE_OFFICAL_FEATURES
PyObject* skillGetSkillPowerByLevel(PyObject* poSelf, PyObject* poArgs)
{
	int iSkillLevel;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillLevel))
		return Py_BadArgument();

	if (iSkillLevel <= 0 || iSkillLevel > 40)
		return Py_BadArgument();

	return Py_BuildValue("i", LocaleService_GetSkillPower(iSkillLevel) / 100);
}
#endif

void initskill()
{
	static PyMethodDef s_methods[] = {{"SetPathName", skillSetPathName, METH_VARARGS},
									  {"RegisterSkill", skillRegisterSkill, METH_VARARGS},
									  {"LoadSkillData", skillLoadSkillData, METH_VARARGS},
									  {"ClearSkillData", skillClearSkillData, METH_VARARGS},

									  /////

									  {"GetSkillName", skillGetSkillName, METH_VARARGS},
									  {"GetSkillDescription", skillGetSkillDescription, METH_VARARGS},
									  {"GetSkillType", skillGetSkillType, METH_VARARGS},
									  {"GetSkillConditionDescriptionCount", skillGetSkillConditionDescriptionCount, METH_VARARGS},
									  {"GetSkillConditionDescription", skillGetSkillConditionDescription, METH_VARARGS},
									  {"GetSkillAffectDescriptionCount", skillGetSkillAffectDescriptionCount, METH_VARARGS},
									  {"GetSkillAffectDescription", skillGetSkillAffectDescription, METH_VARARGS},
									  {"GetSkillCoolTime", skillGetSkillCoolTime, METH_VARARGS},
									  {"GetSkillNeedSP", skillGetSkillNeedSP, METH_VARARGS},
									  {"GetSkillContinuationSP", skillGetSkillContinuationSP, METH_VARARGS},
									  {"GetSkillMaxLevel", skillGetSkillMaxLevel, METH_VARARGS},
									  {"GetSkillLevelUpPoint", skillGetSkillLevelUpPoint, METH_VARARGS},
									  {"GetSkillLevelLimit", skillGetSkillLevelLimit, METH_VARARGS},
									  {"IsSkillRequirement", skillIsSkillRequirement, METH_VARARGS},
									  {"GetSkillRequirementData", skillGetSkillRequirementData, METH_VARARGS},
									  {"GetSkillRequireStatCount", skillGetSkillRequireStatCount, METH_VARARGS},
									  {"GetSkillRequireStatData", skillGetSkillRequireStatData, METH_VARARGS},
									  {"CanLevelUpSkill", skillCanLevelUpSkill, METH_VARARGS},
									  {"IsLevelUpSkill", skillIsLevelUpSkill, METH_VARARGS},
									  {"CheckRequirementSueccess", skillCheckRequirementSueccess, METH_VARARGS},
									  {"GetNeedCharacterLevel", skillGetNeedCharacterLevel, METH_VARARGS},
									  {"IsToggleSkill", skillIsToggleSkill, METH_VARARGS},
									  {"IsUseHPSkill", skillIsUseHPSkill, METH_VARARGS},
									  {"IsStandingSkill", skillIsStandingSkill, METH_VARARGS},
									  {"CanUseSkill", skillCanUseSkill, METH_VARARGS},
									  {"GetIconName", skillGetIconName, METH_VARARGS},
									  {"GetIconImage", skillGetIconImage, METH_VARARGS},
									  {"GetIconImageNew", skillGetIconImageNew, METH_VARARGS},
									  {"GetIconInstance", skillGetIconInstance, METH_VARARGS},
									  {"GetIconInstanceNew", skillGetIconInstanceNew, METH_VARARGS},
									  {"DeleteIconInstance", skillDeleteIconInstance, METH_VARARGS},
									  {"GetGradeData", skillGetGradeData, METH_VARARGS},

									  {"GetNewAffectDataCount", skillGetNewAffectDataCount, METH_VARARGS},
									  {"GetNewAffectData", skillGetNewAffectData, METH_VARARGS},
									  {"GetDuration", skillGetDuration, METH_VARARGS},

									  {"TEST", skillTEST, METH_VARARGS},

#ifdef ENABLE_GROWTH_PET_SYSTEM
		{ "SetPetSkillSlot",						skillSetPetSkillSlot,						METH_VARARGS },
		{ "GetPetSkillIconImage",					skillGetPetSkillIconImage,					METH_VARARGS },
		{ "GetPetSkillbySlot",						skillGetPetSkillbySlot,						METH_VARARGS },
		{ "GetPetEmptySkill",						skillGetPetEmptySkill,						METH_VARARGS },
		{ "GetPetSkillIconPath",					skillGetPetSkillIconPath,					METH_VARARGS },
		{ "GetPetSkillInfo",						skillGetPetSkillInfo,						METH_VARARGS },
#endif

#ifdef ENABLE_OFFICAL_FEATURES
		{ "GetSkillPowerByLevel",					skillGetSkillPowerByLevel,					METH_VARARGS },
#endif

									  {nullptr, nullptr, 0}};

	PyObject * poModule = Py_InitModule("skill", s_methods);
	PyModule_AddIntConstant(poModule, "SKILL_TYPE_NONE", CPythonSkill::SKILL_TYPE_NONE);
	PyModule_AddIntConstant(poModule, "SKILL_TYPE_ACTIVE", CPythonSkill::SKILL_TYPE_ACTIVE);
	PyModule_AddIntConstant(poModule, "SKILL_TYPE_SUPPORT", CPythonSkill::SKILL_TYPE_SUPPORT);
	PyModule_AddIntConstant(poModule, "SKILL_TYPE_GUILD", CPythonSkill::SKILL_TYPE_GUILD);
	PyModule_AddIntConstant(poModule, "SKILL_TYPE_HORSE", CPythonSkill::SKILL_TYPE_HORSE);
#ifdef ENABLE_OFFICAL_FEATURES
	PyModule_AddIntConstant(poModule, "SKILL_TYPE_MOUNT_UPGRADE", CPythonSkill::SKILL_TYPE_MOUNT_UPGRADE);
#endif
	PyModule_AddIntConstant(poModule, "SKILL_TYPE_MAX_NUM", CPythonSkill::SKILL_TYPE_MAX_NUM);

	PyModule_AddIntConstant(poModule, "SKILL_GRADE_COUNT", CPythonSkill::SKILL_GRADE_COUNT);
	PyModule_AddIntConstant(poModule, "SKILL_GRADE_STEP_COUNT", CPythonSkill::SKILL_GRADE_STEP_COUNT);
	PyModule_AddIntConstant(poModule, "SKILL_GRADEGAP", CPythonSkill::SKILL_GRADEGAP);
	PyModule_AddIntConstant(poModule, "SKILL_EFFECT_COUNT", CPythonSkill::SKILL_EFFECT_COUNT);

#ifdef ENABLE_GROWTH_PET_SYSTEM
	PyModule_AddIntConstant(poModule, "PET_SKILL_USE_TYPE_AUTO", CPythonSkillPet::SKILLPET_TYPE_AUTO);
	PyModule_AddIntConstant(poModule, "PET_SKILL_USE_TYPE_PASSIVE", CPythonSkillPet::SKILLPET_TYPE_PASSIVE);
#endif
}
