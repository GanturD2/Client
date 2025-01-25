#include "stdafx.h"
#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "regen.h"
#include "mob_manager.h"
#include "dungeon.h"
#ifdef ENABLE_12ZI
# include "zodiac_temple.h"
#endif

LPREGEN regen_list = nullptr;
LPREGEN_EXCEPTION regen_exception_list = nullptr;

enum ERegenModes
{
	MODE_TYPE = 0,
	MODE_SX,
	MODE_SY,
	MODE_EX,
	MODE_EY,
	MODE_Z_SECTION,
	MODE_DIRECTION,
	MODE_REGEN_TIME,
	MODE_REGEN_PERCENT,
	MODE_MAX_COUNT,
	MODE_VNUM
};

#ifdef ENABLE_12ZI
enum EExtendRegenModes
{
	MODE_MAX_VNUM = 11,
	MODE_MIN_LEVEL,
	MODE_MAX_LEVEL,
	MODE_SPAWN_MIN_GROUP_VNUM,
	MODE_SPAWN_MAX_GROUP_VNUM,
	MODE_SPAWN_MIN_GROUP_LEVEL,
	MODE_SPAWN_MAX_GROUP_LEVEL,
	MODE_SPAWN_BOSS_SPAWN_VNUM,
	MODE_SPAWN_BOSS_SPAWN_LEVEL,
};
#endif

static bool get_word(FILE* fp, char* buf) // received in word units.
{
	int i = 0;
	int c;

	int semicolon_mode = 0;

	while ((c = fgetc(fp)) != EOF)
	{
		if (i == 0)
		{
			if (c == '"')
			{
				semicolon_mode = 1;
				continue;
			}

			if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
				continue;
		}

		if (semicolon_mode)
		{
			if (c == '"')
			{
				buf[i] = '\0';
				return true;
			}

			buf[i++] = c;
		}
		else
		{
			if ((c == ' ' || c == '\t' || c == '\n' || c == '\r'))
			{
				buf[i] = '\0';
				return true;
			}

			buf[i++] = c;
		}

		if (i == 2 && buf[0] == '/' && buf[1] == '/')
		{
			buf[i] = '\0';
			return true;
		}
	}

	buf[i] = '\0';
	return (i != 0);
}

static void next_line(FILE* fp)
{
	int c;

	while ((c = fgetc(fp)) != EOF)
		if (c == '\n')
			return;
}

static bool read_line(FILE* fp, LPREGEN regen)
{
	char szTmp[256];

	int mode = MODE_TYPE;
	int tmpTime;
	uint32_t i;

	while (get_word(fp, szTmp))
	{
		if (!strncmp(szTmp, "//", 2))
		{
			next_line(fp);
			continue;
		}

		switch (mode)
		{
			case MODE_TYPE:
				if (szTmp[0] == 'm')
					regen->type = REGEN_TYPE_MOB;
				else if (szTmp[0] == 'g')
				{
					regen->type = REGEN_TYPE_GROUP;

					if (szTmp[1] == 'a')
						regen->is_aggressive = true;
				}
				else if (szTmp[0] == 'e')
					regen->type = REGEN_TYPE_EXCEPTION;
				else if (szTmp[0] == 'r')
					regen->type = REGEN_TYPE_GROUP_GROUP;
				else if (szTmp[0] == 's')
					regen->type = REGEN_TYPE_ANYWHERE;
				else
				{
					sys_err("read_line: unknown regen type %c", szTmp[0]);
					exit(1);
				}

				++mode;
				break;

			case MODE_SX:
				str_to_number(regen->sx, szTmp);
				++mode;
				break;

			case MODE_SY:
				str_to_number(regen->sy, szTmp);
				++mode;
				break;

			case MODE_EX:
				{
					int iX = 0;
					str_to_number(iX, szTmp);

					regen->sx -= iX;
					regen->ex = regen->sx + iX * 2;

					regen->sx *= 100;
					regen->ex *= 100;

					++mode;
				}
				break;

			case MODE_EY:
				{
					int iY = 0;
					str_to_number(iY, szTmp);

					regen->sy -= iY;
					regen->ey = regen->sy + iY * 2;

					regen->sy *= 100;
					regen->ey *= 100;

					++mode;
				}
				break;

			case MODE_Z_SECTION:
				str_to_number(regen->z_section, szTmp);

				// If it's an exception, let's go.
				if (regen->type == REGEN_TYPE_EXCEPTION)
					return true;

				++mode;
				break;

			case MODE_DIRECTION:
				str_to_number(regen->direction, szTmp);
				++mode;
				break;

			case MODE_REGEN_TIME:
				regen->time = 0;
				tmpTime = 0;

				for (i = 0; i < strlen(szTmp); ++i)
				{
					switch (szTmp[i])
					{
						case 'h':
							regen->time += tmpTime * 3600;
							tmpTime = 0;
							break;

						case 'm':
							regen->time += tmpTime * 60;
							tmpTime = 0;
							break;

						case 's':
							regen->time += tmpTime;
							tmpTime = 0;
							break;

						default:
							if (szTmp[i] >= '0' && szTmp[i] <= '9')
							{
								tmpTime *= 10;
								tmpTime += (szTmp[i] - '0');
							}
					}
				}

				++mode;
				break;

			case MODE_REGEN_PERCENT:
				++mode;
				break;

			case MODE_MAX_COUNT:
				regen->count = 0;
				str_to_number(regen->max_count, szTmp);
				++mode;
				break;

			case MODE_VNUM:
				str_to_number(regen->vnum, szTmp);
				++mode;
				return true;
		}
	}

	return false;
}

#ifdef ENABLE_12ZI
static bool read_line_zodiac(FILE* fp, LPREGEN regen)
{
	char szTmp[256];

	int mode = MODE_TYPE;
	int tmpTime;
	uint32_t i;

	while (get_word(fp, szTmp))
	{
		if (!strncmp(szTmp, "//", 2))
		{
			next_line(fp);
			continue;
		}

		switch (mode)
		{
			case MODE_TYPE:
				if (szTmp[0] == 'm')
					regen->type = REGEN_TYPE_MOB;
				else if (szTmp[0] == 'g')
				{
					regen->type = REGEN_TYPE_GROUP;

					if (szTmp[1] == 'a')
						regen->is_aggressive = true;
				}
				else if (szTmp[0] == 'e')
					regen->type = REGEN_TYPE_EXCEPTION;
				else if (szTmp[0] == 'r')
					regen->type = REGEN_TYPE_GROUP_GROUP;
				else if (szTmp[0] == 's')
					regen->type = REGEN_TYPE_ANYWHERE;
				else
				{
					sys_err("read_line_zodiac: unknown regen type %c", szTmp[0]);
					exit(1);
				}

				++mode;
				break;

			case MODE_SX:
				str_to_number(regen->sx, szTmp);
				++mode;
				break;

			case MODE_SY:
				str_to_number(regen->sy, szTmp);
				++mode;
				break;

			case MODE_EX:
			{
				int iX = 0;
				str_to_number(iX, szTmp);

				regen->sx -= iX;
				regen->ex = regen->sx + iX * 2;

				regen->sx *= 100;
				regen->ex *= 100;

				++mode;
			}
			break;

			case MODE_EY:
			{
				int iY = 0;
				str_to_number(iY, szTmp);

				regen->sy -= iY;
				regen->ey = regen->sy + iY * 2;

				regen->sy *= 100;
				regen->ey *= 100;

				++mode;
			}
			break;

			case MODE_Z_SECTION:
				str_to_number(regen->z_section, szTmp);

				// If it's an exception, let's go.
				if (regen->type == REGEN_TYPE_EXCEPTION)
					return true;

				++mode;
				break;

			case MODE_DIRECTION:
				str_to_number(regen->direction, szTmp);
				++mode;
				break;

			case MODE_REGEN_TIME:
				regen->time = 0;
				tmpTime = 0;

				for (i = 0; i < strlen(szTmp); ++i)
				{
					switch (szTmp[i])
					{
						case 'h':
							regen->time += tmpTime * 3600;
							tmpTime = 0;
							break;

						case 'm':
							regen->time += tmpTime * 60;
							tmpTime = 0;
							break;

						case 's':
							regen->time += tmpTime;
							tmpTime = 0;
							break;

						default:
							if (szTmp[i] >= '0' && szTmp[i] <= '9')
							{
								tmpTime *= 10;
								tmpTime += (szTmp[i] - '0');
							}
					}
				}

				++mode;
				break;

			case MODE_REGEN_PERCENT:
				++mode;
				break;

			case MODE_MAX_COUNT:
				regen->count = 0;
				str_to_number(regen->max_count, szTmp);
				++mode;
				break;

			case MODE_VNUM:
				str_to_number(regen->vnum, szTmp);
				++mode;
				break;

			case MODE_MAX_VNUM:
				str_to_number(regen->max_vnum, szTmp);
				++mode;
				break;

			case MODE_MIN_LEVEL:
				str_to_number(regen->min_level, szTmp);
				++mode;
				break;

			case MODE_MAX_LEVEL:
				str_to_number(regen->max_level, szTmp);
				++mode;
				break;

			case MODE_SPAWN_MIN_GROUP_VNUM:
				str_to_number(regen->min_group_vnum, (szTmp));
				++mode;
				break;
			case MODE_SPAWN_MAX_GROUP_VNUM:
				str_to_number(regen->max_group_vnum, szTmp);
				++mode;
				break;
			case MODE_SPAWN_MIN_GROUP_LEVEL:
				str_to_number(regen->min_group_level, szTmp);
				++mode;
				break;
			case MODE_SPAWN_MAX_GROUP_LEVEL:
				str_to_number(regen->max_group_level, szTmp);
				++mode;
				break;
			case MODE_SPAWN_BOSS_SPAWN_VNUM:
				str_to_number(regen->boss_spawn_vnum, szTmp);
				++mode;
				break;
			case MODE_SPAWN_BOSS_SPAWN_LEVEL:
				str_to_number(regen->boss_spawn_level, szTmp);
				++mode;
				return true;

			default:
				break;
		}
	}

	return false;
}
#endif

bool is_regen_exception(long x, long y)
{
	LPREGEN_EXCEPTION exc;

	for (exc = regen_exception_list; exc; exc = exc->next)
	{
		if (exc->sx <= x && exc->sy <= y)
		{
			if (exc->ex >= x && exc->ey >= y)
				return true;
		}
	}

	return false;
}

static void regen_spawn_dungeon(LPREGEN regen, LPDUNGEON pDungeon, bool bOnce)
{
	uint32_t num;
	uint32_t i;

	num = (regen->max_count - regen->count);

	if (!num)
		return;

	for (i = 0; i < num; ++i)
	{
		LPCHARACTER ch = nullptr;

		if (regen->type == REGEN_TYPE_ANYWHERE)
		{
			ch = CHARACTER_MANAGER::Instance().SpawnMobRandomPosition(regen->vnum, regen->lMapIndex);

			if (ch)
			{
				++regen->count;
				ch->SetDungeon(pDungeon);
			}
		}
		else if (regen->sx == regen->ex && regen->sy == regen->ey)
		{
			ch = CHARACTER_MANAGER::Instance().SpawnMob(regen->vnum,
				regen->lMapIndex,
				regen->sx,
				regen->sy,
				regen->z_section,
				false,
				regen->direction == 0 ? number(0, 7) * 45 : (regen->direction - 1) * 45);

			if (ch)
			{
				++regen->count;
				ch->SetDungeon(pDungeon);
			}
		}
		else
		{
			if (regen->type == REGEN_TYPE_MOB)
			{
				ch = CHARACTER_MANAGER::Instance().SpawnMobRange(regen->vnum, regen->lMapIndex, regen->sx, regen->sy, regen->ex, regen->ey, true);

				if (ch)
				{
					++regen->count;
					ch->SetDungeon(pDungeon);
				}
			}
			else if (regen->type == REGEN_TYPE_GROUP)
			{
				if (CHARACTER_MANAGER::Instance().SpawnGroup(regen->vnum, regen->lMapIndex, regen->sx, regen->sy, regen->ex, regen->ey, bOnce ? nullptr : regen, regen->is_aggressive, pDungeon))
					++regen->count;
			}
			else if (regen->type == REGEN_TYPE_GROUP_GROUP)
			{
				if (CHARACTER_MANAGER::Instance().SpawnGroupGroup(regen->vnum, regen->lMapIndex, regen->sx, regen->sy, regen->ex, regen->ey, bOnce ? nullptr : regen, regen->is_aggressive, pDungeon))
					++regen->count;
			}
		}

		if (ch && !bOnce)
			ch->SetRegen(regen);

#ifdef ENABLE_DEFENSE_WAVE
		if (ch && ch->IsDefenseWaveMastAttackMob(regen->vnum))
		{
			if (pDungeon && pDungeon->GetMast())
				ch->SetVictim(pDungeon->GetMast());
			else
				sys_err("regen_spawn_dungeon Trying to set victim to mast, but mast not exist.");
		}
#endif

#ifdef ENABLE_SUNG_MAHI_TOWER
		if (ch && pDungeon)
		{
			uint8_t bDungeonLevel = pDungeon->GetDungeonDifficulty();
			ch->SetDungeonMultipliers(bDungeonLevel);
		}
#endif
	}
}

static void regen_spawn(LPREGEN regen, bool bOnce)
{
	uint32_t num;
	uint32_t i;

	num = (regen->max_count - regen->count);

	if (!num)
		return;

	for (i = 0; i < num; ++i)
	{
		LPCHARACTER ch = nullptr;

		if (regen->type == REGEN_TYPE_ANYWHERE)
		{
			ch = CHARACTER_MANAGER::Instance().SpawnMobRandomPosition(regen->vnum, regen->lMapIndex);

			if (ch)
				++regen->count;
		}
		else if (regen->sx == regen->ex && regen->sy == regen->ey)
		{
			ch = CHARACTER_MANAGER::Instance().SpawnMob(regen->vnum,
				regen->lMapIndex,
				regen->sx,
				regen->sy,
				regen->z_section,
				false,
				regen->direction == 0 ? number(0, 7) * 45 : (regen->direction - 1) * 45);

			if (ch)
				++regen->count;
		}
		else
		{
			if (regen->type == REGEN_TYPE_MOB)
			{
				ch = CHARACTER_MANAGER::Instance().SpawnMobRange(regen->vnum, regen->lMapIndex, regen->sx, regen->sy, regen->ex, regen->ey, true, regen->is_aggressive, regen->is_aggressive);

				if (ch)
					++regen->count;
			}
			else if (regen->type == REGEN_TYPE_GROUP)
			{
				if (CHARACTER_MANAGER::Instance().SpawnGroup(regen->vnum, regen->lMapIndex, regen->sx, regen->sy, regen->ex, regen->ey, bOnce ? nullptr : regen, regen->is_aggressive))
					++regen->count;
			}
			else if (regen->type == REGEN_TYPE_GROUP_GROUP)
			{
				if (CHARACTER_MANAGER::Instance().SpawnGroupGroup(regen->vnum, regen->lMapIndex, regen->sx, regen->sy, regen->ex, regen->ey, bOnce ? nullptr : regen, regen->is_aggressive))
					++regen->count;
			}
		}

		if (ch && !bOnce)
			ch->SetRegen(regen);
	}
}

EVENTFUNC(dungeon_regen_event)
{
	dungeon_regen_event_info* info = dynamic_cast<dungeon_regen_event_info*>(event->info);

	if (info == nullptr)
	{
		sys_err("dungeon_regen_event> <Factor> Null pointer");
		return 0;
	}

	LPDUNGEON pDungeon = CDungeonManager::Instance().Find(info->dungeon_id);
	if (pDungeon == nullptr)
	{
		return 0;
	}

	LPREGEN regen = info->regen;
	if (regen->time == 0)
	{
		regen->event = nullptr;
	}

	regen_spawn_dungeon(regen, pDungeon, false);
	return PASSES_PER_SEC(regen->time);
}

bool regen_do(const char* filename, long lMapIndex, int base_x, int base_y, LPDUNGEON pDungeon, bool bOnce)
{
	if (g_bNoRegen)
		return true;

	if (lMapIndex >= 114 && lMapIndex <= 117)
		return true;

	LPREGEN regen = nullptr;
	FILE* fp = fopen(filename, "rt");

	if (nullptr == fp)
	{
		sys_err("SYSTEM: regen_do: %s: file not found", filename);
		return false;
	}

	while (true)
	{
		REGEN tmp{};

		if (!read_line(fp, &tmp))
			break;

		if (tmp.type == REGEN_TYPE_MOB ||
			tmp.type == REGEN_TYPE_GROUP ||
			tmp.type == REGEN_TYPE_GROUP_GROUP ||
			tmp.type == REGEN_TYPE_ANYWHERE)
		{
			if (!bOnce)
			{
				regen = M2_NEW REGEN;
				*regen = tmp;
			}
			else
				regen = &tmp;

			if (pDungeon)
				regen->is_aggressive = true;

			regen->lMapIndex = lMapIndex;
			regen->count = 0;

			regen->sx += base_x;
			regen->ex += base_x;

			regen->sy += base_y;
			regen->ey += base_y;

			if (regen->sx > regen->ex)
			{
				regen->sx ^= regen->ex;
				regen->ex ^= regen->sx;
				regen->sx ^= regen->ex;
			}

			if (regen->sy > regen->ey)
			{
				regen->sy ^= regen->ey;
				regen->ey ^= regen->sy;
				regen->sy ^= regen->ey;
			}

			if (regen->type == REGEN_TYPE_MOB)
			{
				const CMob* p = CMobManager::Instance().Get(regen->vnum);

				if (!p)
				{
					sys_err("In %s, No mob data by vnum %u", filename, regen->vnum);
					if (!bOnce) {
						M2_DELETE(regen);
					}
					continue;
				}
			}

			if (!bOnce && pDungeon != nullptr)
			{
				dungeon_regen_event_info* info = AllocEventInfo<dungeon_regen_event_info>();

				info->regen = regen;
				info->dungeon_id = pDungeon->GetId();

				regen->event = event_create(dungeon_regen_event, info, PASSES_PER_SEC(number(0, 16)) + PASSES_PER_SEC(regen->time));

				pDungeon->AddRegen(regen);
				// regen_id should be determined at this point,
				// before the call to CHARACTER::SetRegen()
			}

			// At first, it regenerates unconditionally.
			regen_spawn_dungeon(regen, pDungeon, bOnce);

		}
	}

	fclose(fp);
	return true;
}

#ifdef ENABLE_12ZI
bool exist_regen(const char* filename) noexcept
{
	FILE* fp = fopen(filename, "rt");
	if (!fp)
		return false;

	fclose(fp);
	return true;
}

static void regen_spawn_zodiac(LPREGEN regen, LPZODIAC pZodiac, bool bOnce)
{
	if (!pZodiac)
		return;

	const uint32_t num = (regen->max_count - regen->count);
	if (!num)
		return;

	for (uint32_t i = 0; i < num; ++i)
	{
		LPCHARACTER ch = nullptr;

		const uint32_t vnum = !regen->max_vnum ? regen->vnum : number(regen->vnum, regen->max_vnum);
		const uint8_t level = number(regen->min_level, regen->max_level);

		/*const uint8_t bMission = pZodiac->GetMission();
		if (bMission == KILL_STONE_STATUE)
		{
			const CMob* pkMob = CMobManager::Instance().Get(regen->vnum);
			if (!(regen->vnum >= 20452 && regen->vnum <= 20463) && pkMob->m_table.bType == CHAR_TYPE_STONE)
			{
				if (!pZodiac->CanSpawnStone())
				{
					pZodiac->SetSpawnStone(false);
					continue;
				}
			}
		}*/

		if (regen->type == REGEN_TYPE_MOB)
		{
			ch = CHARACTER_MANAGER::Instance().SpawnMobRange(vnum, regen->lMapIndex, regen->sx, regen->sy, regen->ex, regen->ey, true, false, regen->is_aggressive, level);

			if (ch)
			{
				++regen->count;
				ch->SetZodiac(pZodiac);
				ch->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
			}
		}
		else if (regen->type == REGEN_TYPE_GROUP)
		{
			if (CHARACTER_MANAGER::Instance().SpawnGroupZodiac(vnum, regen->lMapIndex, regen->sx, regen->sy, regen->ex, regen->ey, bOnce ? nullptr : regen, regen->is_aggressive, pZodiac, level))
				++regen->count;
		}
		else if (regen->type == REGEN_TYPE_GROUP_GROUP)
		{
			if (CHARACTER_MANAGER::Instance().SpawnGroupGroupZodiac(vnum, regen->lMapIndex, regen->sx, regen->sy, regen->ex, regen->ey, bOnce ? nullptr : regen, regen->is_aggressive, pZodiac, level))
				++regen->count;
		}

		/*if (bMission == KILL_STONE_STATUE)
		{
			//if (pZodiac->CanSpawnStone())
			pZodiac->SetSpawnStone(false);
		}*/

		if (ch)
		{
			/*ch->zodiac_mob_info.min_group_vnum = regen->min_group_vnum;
			ch->zodiac_mob_info.max_group_vnum = regen->max_group_vnum;
			ch->zodiac_mob_info.min_group_level = regen->min_group_level;
			ch->zodiac_mob_info.max_group_level = regen->max_group_level;
			ch->zodiac_mob_info.boss_spawn_vnum = regen->boss_spawn_vnum;
			ch->zodiac_mob_info.boss_spawn_level = regen->boss_spawn_level;*/

			if (!bOnce)
				ch->SetRegen(regen);
		}
	}
}

EVENTFUNC(zodiac_regen_event)
{
	zodiac_regen_event_info* info = dynamic_cast<zodiac_regen_event_info*>(event->info);

	if (info == nullptr)
	{
		sys_err("zodiac_regen_event> <Factor> Null pointer");
		return 0;
	}

	LPZODIAC pZodiac = CZodiacManager::Instance().Find(info->zodiac_id);
	if (pZodiac == nullptr)
		return 0;

	LPREGEN regen = info->regen;
	if (regen->time == 0)
	{
		regen->event = nullptr;
	}

	regen_spawn_zodiac(regen, pZodiac, false);
	return PASSES_PER_SEC(regen->time);
}

bool regen_zodiac(const char* filename, long lMapIndex, int base_x, int base_y, LPZODIAC pZodiac, bool bOnce)
{
	if (g_bNoRegen)
		return true;

	LPREGEN regen = nullptr;
	FILE* fp = fopen(filename, "rt");

	if (nullptr == fp)
	{
		sys_err("SYSTEM: regen_zodiac: %s: file not found", filename);
		return false;
	}

	while (true)
	{
		REGEN tmp{};

		if (!read_line_zodiac(fp, &tmp))
			break;

		if (tmp.type == REGEN_TYPE_MOB ||
			tmp.type == REGEN_TYPE_GROUP ||
			tmp.type == REGEN_TYPE_GROUP_GROUP ||
			tmp.type == REGEN_TYPE_ANYWHERE)
		{
			if (!bOnce)
			{
				regen = M2_NEW REGEN;
				*regen = tmp;
			}
			else
				regen = &tmp;

			if (pZodiac)
				regen->is_aggressive = true;

			regen->lMapIndex = lMapIndex;
			regen->count = 0;

			regen->sx += base_x;
			regen->ex += base_x;

			regen->sy += base_y;
			regen->ey += base_y;

			if (regen->sx > regen->ex)
			{
				regen->sx ^= regen->ex;
				regen->ex ^= regen->sx;
				regen->sx ^= regen->ex;
			}

			if (regen->sy > regen->ey)
			{
				regen->sy ^= regen->ey;
				regen->ey ^= regen->sy;
				regen->sy ^= regen->ey;
			}

			if (regen->type == REGEN_TYPE_MOB)
			{
				const CMob* p = CMobManager::Instance().Get(regen->vnum);

				if (!p)
				{
					sys_err("In %s, No mob data by vnum %u", filename, regen->vnum);
					if (!bOnce)
						M2_DELETE(regen);

					continue;
				}
			}

			if (!bOnce && pZodiac != nullptr)
			{
				zodiac_regen_event_info* info = AllocEventInfo<zodiac_regen_event_info>();

				info->regen = regen;
				info->zodiac_id = pZodiac->GetId();

				regen->event = regen->time ? event_create(zodiac_regen_event, info, PASSES_PER_SEC(number(0, 16)) + PASSES_PER_SEC(regen->time)) : 0;

				pZodiac->AddRegen(regen);
				// regen_id should be determined at this point,
				// before the call to CHARACTER::SetRegen()
			}

			// At first, it regenerates unconditionally.
			regen_spawn_zodiac(regen, pZodiac, bOnce);
		}
	}

	fclose(fp);
	return true;
}
#endif

bool regen_load_in_file(const char* filename, long lMapIndex, int base_x, int base_y)
{
	if (g_bNoRegen)
		return true;

	LPREGEN regen = nullptr;
	FILE* fp = fopen(filename, "rt");

	if (nullptr == fp)
	{
		sys_err("SYSTEM: regen_do: %s: file not found", filename);
		return false;
	}

	while (true)
	{
		REGEN tmp{};

		if (!read_line(fp, &tmp))
			break;

		if (tmp.type == REGEN_TYPE_MOB ||
			tmp.type == REGEN_TYPE_GROUP ||
			tmp.type == REGEN_TYPE_GROUP_GROUP ||
			tmp.type == REGEN_TYPE_ANYWHERE)
		{
			regen = &tmp;

			regen->is_aggressive = true;

			regen->lMapIndex = lMapIndex;
			regen->count = 0;

			regen->sx += base_x;
			regen->ex += base_x;

			regen->sy += base_y;
			regen->ey += base_y;

			if (regen->sx > regen->ex)
			{
				regen->sx ^= regen->ex;
				regen->ex ^= regen->sx;
				regen->sx ^= regen->ex;
			}

			if (regen->sy > regen->ey)
			{
				regen->sy ^= regen->ey;
				regen->ey ^= regen->sy;
				regen->sy ^= regen->ey;
			}

			if (regen->type == REGEN_TYPE_MOB)
			{
				const CMob* p = CMobManager::Instance().Get(regen->vnum);

				if (!p)
				{
					sys_err("In %s, No mob data by vnum %u", filename, regen->vnum);
					continue;
				}
			}

			// At first, it regenerates unconditionally.
			regen_spawn(regen, true);
		}
	}

	fclose(fp);
	return true;
}

EVENTFUNC(regen_event)
{
	regen_event_info* info = dynamic_cast<regen_event_info*>(event->info);

	if (info == nullptr)
	{
		sys_err("regen_event> <Factor> Null pointer");
		return 0;
	}

	LPREGEN regen = info->regen;

	if (regen->time == 0)
		regen->event = nullptr;

	regen_spawn(regen, false);
	return PASSES_PER_SEC(regen->time);
}

bool regen_load(const char* filename, long lMapIndex, int base_x, int base_y)
{
	if (g_bNoRegen)
		return true;

	LPREGEN regen = nullptr;
	FILE* fp = fopen(filename, "rt");

	if (nullptr == fp)
	{
		sys_log(0, "SYSTEM: regen_load: %s: file not found", filename);
		return false;
	}

	while (true)
	{
		REGEN tmp{};

		if (!read_line(fp, &tmp))
			break;

		if (tmp.type == REGEN_TYPE_MOB ||
			tmp.type == REGEN_TYPE_GROUP ||
			tmp.type == REGEN_TYPE_GROUP_GROUP ||
			tmp.type == REGEN_TYPE_ANYWHERE)
		{
			if (test_server)
				CMobManager::Instance().IncRegenCount(tmp.type, tmp.vnum, tmp.max_count, tmp.time);

			regen = M2_NEW REGEN;
			*regen = tmp;
			INSERT_TO_TW_LIST(regen, regen_list, prev, next);

			memcpy(&regen->szFileName[0], filename, sizeof(regen->szFileName));

			regen->lMapIndex = lMapIndex;
			regen->count = 0;

			regen->sx += base_x;
			regen->ex += base_x;

			regen->sy += base_y;
			regen->ey += base_y;

			if (regen->sx > regen->ex)
			{
				regen->sx ^= regen->ex;
				regen->ex ^= regen->sx;
				regen->sx ^= regen->ex;
			}

			if (regen->sy > regen->ey)
			{
				regen->sy ^= regen->ey;
				regen->ey ^= regen->sy;
				regen->sy ^= regen->ey;
			}

			if (regen->type == REGEN_TYPE_MOB)
			{
				const CMob* p = CMobManager::Instance().Get(regen->vnum);

				if (!p)
				{
					sys_err("In %s, No mob data by vnum %u", filename, regen->vnum);
				}
				else if (p->m_table.bType == CHAR_TYPE_NPC || p->m_table.bType == CHAR_TYPE_WARP || p->m_table.bType == CHAR_TYPE_GOTO)
				{
					SECTREE_MANAGER::Instance().InsertNPCPosition(lMapIndex,
						p->m_table.bType,
						p->m_table.szLocaleName,
						p->m_table.dwVnum,
						(regen->sx + regen->ex) / 2 - base_x,
						(regen->sy + regen->ey) / 2 - base_y);
				}
			}

			//NO_REGEN
			// When setting the regen time to 0 in Desc: regen.txt (other regen related texts)
			// Do not regenerate.
			if (regen->time != 0)
			{
				// At first, it regenerates unconditionally.
				regen_spawn(regen, false);

				regen_event_info* info = AllocEventInfo<regen_event_info>();

				info->regen = regen;
				regen->event = event_create(regen_event, info, PASSES_PER_SEC(number(0, 16)) + PASSES_PER_SEC(regen->time));
			}
			//END_NO_REGEN
		}
		else if (tmp.type == REGEN_TYPE_EXCEPTION)
		{
			LPREGEN_EXCEPTION exc;

			exc = M2_NEW REGEN_EXCEPTION;

			exc->sx = tmp.sx;
			exc->sy = tmp.sy;
			exc->ex = tmp.ex;
			exc->ey = tmp.ey;
			exc->z_section = tmp.z_section;

			INSERT_TO_TW_LIST(exc, regen_exception_list, prev, next);
		}
	}

	fclose(fp);
	return true;
}

void regen_free(void)
{
	LPREGEN regen, next_regen;
	LPREGEN_EXCEPTION exc, next_exc;

	for (regen = regen_list; regen; regen = next_regen)
	{
		next_regen = regen->next;

		event_cancel(&regen->event);
		M2_DELETE(regen);
	}

	regen_list = nullptr;

	for (exc = regen_exception_list; exc; exc = next_exc)
	{
		next_exc = exc->next;

		M2_DELETE(exc);
	}

	regen_exception_list = nullptr;
}

void regen_reset(int x, int y)
{
	LPREGEN regen;

	for (regen = regen_list; regen; regen = regen->next)
	{
		if (!regen->event)
			continue;

		// If there are coordinates, only the regen list within the coordinates is regenerated.
		if (x != 0 || y != 0)
		{
			if (x >= regen->sx && x <= regen->ex)
			{
				if (y >= regen->sy && y <= regen->ey)
					event_reset_time(regen->event, 1);
			}
		}
		// If not, all regen
		else
			event_reset_time(regen->event, 1);
	}
}

void clear_regen(const char* filename)
{
	LPREGEN regen;
	for (regen = regen_list; regen; regen = regen->next)
	{
		if (!regen->event)
			continue;

		if (!*regen->szFileName)
			continue;

		if (strcmp(&regen->szFileName[0], filename) == 0)
		{
			event_cancel(&regen->event);
			//M2_DELETE(regen);
			REMOVE_FROM_TW_LIST(regen, regen_list, prev, next);
		}
	}
}

bool IsValidRegen(const LPREGEN& current_regen, size_t regen_id) noexcept
{
	LPREGEN regen;
	for (regen = regen_list; regen; regen = regen->next)
	{
		if (regen == current_regen && regen->id == regen_id)
			return true;
	}

	return false;
}

bool is_valid_regen(LPREGEN currRegen)
{
	LPREGEN regen;

	for (regen = regen_list; regen; regen = regen->next)
	{
		if (regen == currRegen)
			return true;
	}

	return false;
}

void regen_free_map(long lMapIndex)
{
	LPREGEN regen, prev, next;

	for (regen = regen_list; regen; regen = regen->next)
	{
		if (regen->lMapIndex != lMapIndex)
			continue;

		event_cancel(&regen->event);
		REMOVE_FROM_TW_LIST(regen, regen_list, prev, next);
		M2_DELETE(regen);
	}
}
