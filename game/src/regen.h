#include "dungeon.h"
#ifdef ENABLE_12ZI
# include "zodiac_temple.h"
#endif

enum
{
	REGEN_TYPE_MOB,
	REGEN_TYPE_GROUP,
	REGEN_TYPE_EXCEPTION,
	REGEN_TYPE_GROUP_GROUP,
	REGEN_TYPE_ANYWHERE,
	REGEN_TYPE_MAX_NUM
};

typedef struct regen
{
	LPREGEN prev, next;
	char szFileName[256 + 1];
	long lMapIndex;
	int type;
	int sx, sy, ex, ey;
	uint8_t z_section;

	uint8_t direction;

	uint32_t time;

	int max_count;
	int count;
	int vnum;

	bool is_aggressive;

	LPEVENT event;

	size_t id; // to help dungeon regen identification

#ifdef ENABLE_12ZI
	uint32_t max_vnum;
	uint8_t min_level;
	uint8_t max_level;
	uint32_t min_group_vnum;
	uint32_t max_group_vnum;
	uint8_t min_group_level;
	uint8_t max_group_level;
	uint32_t boss_spawn_vnum;
	uint8_t boss_spawn_level;
#endif

	regen() noexcept :
		prev(nullptr), next(nullptr),
		lMapIndex(0),
		type(0),
		sx(0), sy(0), ex(0), ey(0),
		z_section(0),
		direction(0),
		time(0),
		max_count(0),
		count(0),
		vnum(0),
		is_aggressive(0),
		event(nullptr),
		id(0)
#ifdef ENABLE_12ZI
		, max_vnum(0),
		min_level(0),
		max_level(0),
		min_group_vnum(0),
		max_group_vnum(0),
		min_group_level(0),
		max_group_level(0),
		boss_spawn_vnum(0),
		boss_spawn_level(0)
#endif
	{
		*szFileName = {};
	}
} REGEN;

EVENTINFO(regen_event_info)
{
	LPREGEN regen;

	regen_event_info()
		: regen(0)
	{
	}
};

typedef regen_event_info REGEN_EVENT_INFO;

typedef struct regen_exception
{
	LPREGEN_EXCEPTION prev, next;

	int sx, sy, ex, ey;
	uint8_t z_section;
} REGEN_EXCEPTION;

class CDungeon;

EVENTINFO(dungeon_regen_event_info)
{
	LPREGEN regen;
	CDungeon::IdType dungeon_id;

	dungeon_regen_event_info()
		: regen(0)
		, dungeon_id(0)
	{
	}
};

#ifdef ENABLE_12ZI
class CZodiac;

EVENTINFO(zodiac_regen_event_info)
{
	LPREGEN regen;
	CZodiac::IdType zodiac_id;

	zodiac_regen_event_info()
		: regen(0)
		, zodiac_id(0)
	{
	}
};

extern bool exist_regen(const char* filename) noexcept;
extern bool regen_zodiac(const char* filename, long lMapIndex, int base_x, int base_y, LPZODIAC pZodiac, bool bOnce = true);
#endif

extern bool regen_load(const char* filename, long lMapIndex, int base_x, int base_y);
extern bool regen_do(const char* filename, long lMapIndex, int base_x, int base_y, LPDUNGEON pDungeon, bool bOnce = true);
extern bool regen_load_in_file(const char* filename, long lMapIndex, int base_x, int base_y);
extern void regen_free();

extern bool is_regen_exception(long x, long y);
extern void regen_reset(int x, int y);

extern void clear_regen(const char* filename);
extern bool IsValidRegen(const LPREGEN& regen, size_t regen_id) noexcept;

extern bool is_valid_regen(LPREGEN currRegen);
extern void regen_free_map(long lMapIndex);
