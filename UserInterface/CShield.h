#pragma once

#ifdef ENABLE_CSHIELD

extern bool hackFound;

class CShield : public CSingleton<CShield>
{
public:
	CShield();
	~CShield();

#if defined(ENABLE_CHECK_ATTACKSPEED_HACK) && !defined(_DEBUG)
public:
	bool CheckAttackspeed(float attackSpeed, float range, bool twoHanded, bool twoHandedHorse);
#endif

#if defined(ENABLE_CHECK_MOVESPEED_HACK) && !defined(_DEBUG)
public:
	bool CheckMovespeed(float moveSpeed);
#endif

	inline void Close();
};
#endif
