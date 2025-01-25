#pragma once

#ifdef ENABLE_DS_SET
#include "ItemData.h"

enum EDSStepTypes
{
	DS_STEP_LOWEST,
	DS_STEP_LOW,
	DS_STEP_MID,
	DS_STEP_HIGH,
	DS_STEP_HIGHEST,
	DS_STEP_MAX,
};

enum EDSGradeTypes
{
	DS_GRADE_NORMAL,
	DS_GRADE_BRILLIANT,
	DS_GRADE_RARE,
	DS_GRADE_ANCIENT,
	DS_GRADE_LEGENDARY,
	DS_GRADE_MYTH,
	DS_GRADE_MAX,
};

enum EDSStrengthTypes
{
	DS_STRENGTH_MAX = 7,
};

typedef struct SValueName
{
	const char* c_pszName;
	long		lValue;
} TValueName;

long GetApplyTypeByName(const char* c_pszApplyName);
#endif
