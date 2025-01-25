#include "StdAfx.h"

#ifdef ENABLE_CSHIELD
#include "CShield.h"

CShield::CShield() {}
CShield::~CShield() {}

#if defined(ENABLE_CHECK_ATTACKSPEED_HACK) && !defined(_DEBUG)
bool CShield::CheckAttackspeed(float attackSpeed, float range, bool twoHanded, bool twoHandedHorse)
{
	if (attackSpeed > 2.0f)
		return false;

	TraceError("CheckAttackspeed: %.2f %.2f %d %d", attackSpeed, range, twoHanded, twoHandedHorse);
	return true;
}
#endif

#if defined(ENABLE_CHECK_MOVESPEED_HACK) && !defined(_DEBUG)
	bool CShield::CheckMovespeed(float moveSpeed)
	{
		if (moveSpeed > 4.0f)
			return false;

		TraceError("CheckMovespeed: %.2f", moveSpeed);
		return true;
}
#endif

#if defined(ENABLE_CHECK_ATTACKSPEED_HACK) || defined(ENABLE_CHECK_MOVESPEED_HACK) && !defined(_DEBUG)
	inline void CShield::Close()
	{
		SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)NULL);
		__asm {
			mov eax, 0x001
			jmp eax
		}
	}
#endif

#endif