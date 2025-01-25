#ifndef __HEADER_VNUM_HELPER__
#define __HEADER_VNUM_HELPER__

#include "CommonDefines.h"

/*
	When the source identifies items, mobs, etc. that already exist or will be added in the future, all
	The readability is very poor because the identifier (number = VNum) is hard-coded.

	In the future, added by Seungcheol's suggestion to let you know which item (or mob) it is by looking at the source.

	* This file is expected to be changed frequently. If you put it in PCH, you have to compile the whole file every time it changes.
	First, I included it in the required cpp file and used it.

	* If implemented in cpp, you need to compile ~ link, so just put the header in common. (To use both game and db project)

	@date 2011.8.29.
*/

class CItemVnumHelper
{
public:
	// Phoenix Summon Ticket for German DVD
	static const bool IsPhoenix(uint32_t vnum) noexcept { return 53001 == vnum; } // NOTE: The Phoenix Summon item is 53001, but the mob-vnum is 34001.

	// Ramadan Event Crescent Ring (Originally, it was a special item for Ramadan events, but it is said to be reused in various directions in the future)
	static const bool IsRamadanMoonRing(uint32_t vnum) noexcept { return 71135 == vnum; }

	// Halloween candy (specs the same as the crescent ring)
	static const bool IsHalloweenCandy(uint32_t vnum) noexcept { return 71136 == vnum; }

	// Christmas Ring of Happiness
	static const bool IsHappinessRing(uint32_t vnum) noexcept { return 71143 == vnum; }

	// Valentine love pendant
	static const bool IsLovePendant(uint32_t vnum) noexcept { return 71145 == vnum; }

	static const bool IsSpeedBoots(uint32_t vnum) noexcept { return 72701 == vnum; }

	static const bool IsHeromedal(uint32_t vnum) noexcept { return 71158 == vnum; }

	// Chocolate pendant
	static const bool IsChocolatePendant(uint32_t vnum) noexcept { return 71199 == vnum; }

	static const bool IsEmotionMask(uint32_t vnum) noexcept { return 71011 == vnum; }

	static const bool IsWillPowerRing(uint32_t vnum) noexcept { return 71148 == vnum; }

	static const bool IsDeadlyPowerRing(uint32_t vnum) noexcept { return 71149 == vnum; }

	static const bool IsEasterCandyEqip(uint32_t vnum) noexcept { return 71188 == vnum; }

	static const bool IsHorseBook(uint32_t vnum) noexcept { return 50051 == vnum || 50052 == vnum || 50053 == vnum; }

	static const bool IsBouquet(uint32_t dwVnum) noexcept
	{
		if (dwVnum >= 50201 && dwVnum <= 50202)
			return true;

		return false;
	}

	static const bool IsSmokinOrWeedingDress(uint32_t dwVnum) noexcept
	{
		if ((dwVnum >= 11901 && dwVnum <= 11904) || (dwVnum >= 11911 && dwVnum <= 11914))
			return true;

		return false;
	}


#ifdef ENABLE_SUNG_MAHI_TOWER
	static const bool IsSungMahiItem(uint32_t dwVnum) noexcept
	{
		if ((dwVnum >= 70390 && dwVnum <= 70395) || (dwVnum == 70405))
			return true;

		return false;
	}
#endif
};

class CMobVnumHelper
{
public:
	// Phoenix mob number for German DVD
	static bool IsPhoenix(uint32_t vnum) noexcept { return 34001 == vnum; }
	static bool IsIcePhoenix(uint32_t vnum) noexcept { return 34003 == vnum; }
	// Is it a pet managed by PetSystem?
	static bool IsPetUsingPetSystem(uint32_t vnum) noexcept { return (IsPhoenix(vnum) || IsReindeerYoung(vnum)) || IsIcePhoenix(vnum); }

	// 2011 Christmas Event Pet (Baby Reindeer)
	static bool IsReindeerYoung(uint32_t vnum) noexcept { return 34002 == vnum; }

	// Ramadan black horse for rewards (20119) .. Ramadan black horse clone for Halloween events (same spec, 20219)
	static bool IsRamadanBlackHorse(uint32_t vnum) noexcept { return 20119 == vnum || 20219 == vnum || 22022 == vnum; }
};

class CVnumHelper
{
};

#endif //__HEADER_VNUM_HELPER__
