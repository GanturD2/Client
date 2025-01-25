#ifndef __INC_GAME_MOTION_H__
#define __INC_GAME_MOTION_H__

#include "../../common/d3dtype.h"
#include "../../common/CommonDefines.h"

enum EMotionMode
{
	MOTION_MODE_GENERAL,
	MOTION_MODE_ONEHAND_SWORD,
	MOTION_MODE_TWOHAND_SWORD,
	MOTION_MODE_DUALHAND_SWORD,
	MOTION_MODE_BOW,
	MOTION_MODE_BELL,
	MOTION_MODE_FAN,
	MOTION_MODE_HORSE,
#ifdef ENABLE_WOLFMAN_CHARACTER
	MOTION_MODE_CLAW,
#endif
	MOTION_MODE_MAX_NUM
};

enum EPublicMotion
{
	MOTION_NONE,                // 0
	MOTION_WAIT,                // 1
	MOTION_WALK,                // 2
	MOTION_RUN,                 // 3
	MOTION_CHANGE_WEAPON,       // 4
	MOTION_DAMAGE,              // 5
	MOTION_DAMAGE_FLYING,       // 6
	MOTION_STAND_UP,            // 7
	MOTION_DAMAGE_BACK,         // 8
	MOTION_DAMAGE_FLYING_BACK,  // 9
	MOTION_STAND_UP_BACK,       // 10
	MOTION_DEAD,                // 11
	MOTION_DEAD_BACK,           // 12
	MOTION_NORMAL_ATTACK,		// 13
	MOTION_COMBO_ATTACK_1,		// 14
	MOTION_COMBO_ATTACK_2,      // 15
	MOTION_COMBO_ATTACK_3,      // 16
	MOTION_COMBO_ATTACK_4,      // 17
	MOTION_COMBO_ATTACK_5,      // 18
	MOTION_COMBO_ATTACK_6,      // 19
	MOTION_COMBO_ATTACK_7,      // 20
	MOTION_COMBO_ATTACK_8,      // 21
	MOTION_INTRO_WAIT,          // 22
	MOTION_INTRO_SELECTED,      // 23
	MOTION_INTRO_NOT_SELECTED,  // 24
	MOTION_SPAWN,               // 25
	MOTION_FISHING_THROW,       // 26
	MOTION_FISHING_WAIT,        // 27
	MOTION_FISHING_STOP,        // 28
	MOTION_FISHING_REACT,       // 29
	MOTION_FISHING_CATCH,       // 30
	MOTION_FISHING_FAIL,        // 31
	MOTION_STOP,                // 32
	MOTION_SPECIAL_1,           // 33
	MOTION_SPECIAL_2,           // 34
	MOTION_SPECIAL_3,			// 35
	MOTION_SPECIAL_4,			// 36
	MOTION_SPECIAL_5,			// 37
#ifdef ENABLE_DAWNMIST_DUNGEON
	MOTION_SPECIAL_6,
	MOTION_SPECIAL_7,
#	ifdef ENABLE_WOLFMAN_CHARACTER
	MOTION_SPECIAL_8,
#	endif
#endif
	PUBLIC_MOTION_END,
	MOTION_MAX_NUM = PUBLIC_MOTION_END,
};

class CMob;

class CMotion
{
public:
	CMotion();
	~CMotion();

	bool LoadFromFile(const char* c_pszFileName);
	bool LoadMobSkillFromFile(const char* c_pszFileName, CMob* pMob, int iSkillIndex);

	float GetDuration() const;
	const D3DVECTOR& GetAccumVector() const;

	bool IsEmpty();

protected:
	bool m_isEmpty;
	float m_fDuration;
	bool m_isAccumulation;
	D3DVECTOR m_vec3Accumulation;
};

typedef uint32_t MOTION_KEY;

#define MAKE_MOTION_KEY(mode, index) ( ((mode) << 24) | ((index) << 8) | (0) )
#define MAKE_RANDOM_MOTION_KEY(mode, index, type) ( ((mode) << 24) | ((index) << 8) | (type) )

#define GET_MOTION_MODE(key) ((uint8_t) ((key >> 24) & 0xFF))
#define GET_MOTION_INDEX(key) ((uint16_t) ((key >> 8) & 0xFFFF))
#define GET_MOTION_SUB_INDEX(key) ((uint8_t) ((key) & 0xFF))

class CMotionSet
{
public:
	typedef std::map<uint32_t, CMotion*> TContainer;
	typedef TContainer::iterator iterator;
	typedef TContainer::const_iterator const_iterator;

public:
	CMotionSet();
	~CMotionSet();

	void Insert(uint32_t dwKey, CMotion* pkMotion);
	bool Load(const char* szFileName, int mode, int motion);

	const CMotion* GetMotion(uint32_t dwKey) const;

protected:
	// uint32_t = MOTION_KEY
	TContainer m_map_pkMotion;
};

class CMotionManager : public singleton<CMotionManager>
{
public:
	typedef std::map<uint32_t, CMotionSet*> TContainer;
	typedef TContainer::iterator iterator;

	CMotionManager();
	virtual ~CMotionManager();

	bool Build();

	const CMotionSet* GetMotionSet(uint32_t dwVnum);
	const CMotion* GetMotion(uint32_t dwVnum, uint32_t dwKey);
	float GetMotionDuration(uint32_t dwVnum, uint32_t dwKey);

	// POLYMORPH_BUG_FIX
	float GetNormalAttackDuration(uint32_t dwVnum);
	// END_OF_POLYMORPH_BUG_FIX

protected:
	// uint32_t = JOB or MONSTER VNUM
	TContainer m_map_pkMotionSet;

	// POLYMORPH_BUG_FIX
	std::map<uint32_t, float> m_map_normalAttackDuration;
	// END_OF_POLYMORPH_BUG_FIX
};

#endif
