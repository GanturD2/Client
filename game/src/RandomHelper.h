#ifndef __HEADER_RANDOM_HELPER__
#define __HEADER_RANDOM_HELPER__

#include "char.h"
#include "item.h"

class CRandomHelper
{
public:
	static int IsSerpentItem(uint32_t vnum)
	{
		// Weapon
		if (
			((vnum >= 360) && (vnum <= 375)) ||		// Schlangenschwert
			((vnum >= 380) && (vnum <= 395)) ||		// Schlangenklinge
			((vnum >= 1210) && (vnum <= 1225)) ||	// Schlangendolch
			((vnum >= 2230) && (vnum <= 2245)) ||	// Schlangenbogen
			((vnum >= 3250) && (vnum <= 3265)) ||	// Schlangengroßschwert
			((vnum >= 5200) && (vnum <= 5215)) ||	// Schlangenglocke
			((vnum >= 6150) && (vnum <= 6165)) ||	// Schlangenkralle
			((vnum >= 7330) && (vnum <= 7345)) ||	// Schlangenfächer
			((vnum >= 23050) && (vnum <= 23089))	// Schlangenhands.
			)
			return true;

		// Armor
		if ((vnum >= 21310) && (vnum <= 21406))
			return true;

		return false;
	}

	static int RandomApplyType(uint8_t type)
	{
		std::map<int, int> applyTypeMap
		{
			// Value of APPLY_RANDOM
			{ 0, APPLY_NONE					},
			{ 1, APPLY_SUNGMA_STR			},
			{ 2, APPLY_SUNGMA_MOVE			},
			{ 3, APPLY_SUNGMA_HP			},
			{ 4, APPLY_SUNGMA_IMMUNE		},
			{ 5, APPLY_HIT_PCT				},
			{ 6, APPLY_SUNGMA_MOVE			},
			{ 7, APPLY_SUNGMA_IMMUNE		},
			{ 8, APPLY_SUNGMA_HP			},
			{ 9, APPLY_SUNGMA_STR			},
			{ 10, APPLY_SUNGMA_STR			},
			{ 11, APPLY_SUNGMA_MOVE			},
			{ 12, APPLY_SUNGMA_HP			},
			{ 13, APPLY_SUNGMA_IMMUNE		},
			{ 14, APPLY_NONE				},	// UNK_RANDOM_ELEMENT_14
			{ 15, APPLY_NONE				},	// UNK_RANDOM_ELEMENT_15
			{ 16, APPLY_NONE				},	// UNK_RANDOM_ELEMENT_16
			{ 17, APPLY_NONE				},	// UNK_RANDOM_ELEMENT_17
			{ 18, APPLY_NONE				},	// Snake-Weapon
			{ 19, APPLY_NONE				},	// Snake-Weapon
			{ 20, APPLY_NONE				},	// Snake-Weapon
			{ 21, APPLY_NONE				},	// Snake-Armor
			{ 22, APPLY_NONE				},	// Snake-Armor
			{ 23, APPLY_NONE				},	// Snake-Armor
			{ 24, APPLY_SUNGMA_STR			},
			{ 25, APPLY_SUNGMA_MOVE			},
			{ 26, APPLY_SUNGMA_HP			},
			{ 27, APPLY_SUNGMA_IMMUNE		},
			{ 28, APPLY_NONE				},	// UNK_RANDOM_SUNGMA_28
			{ 29, APPLY_NONE				},	// UNK_RANDOM_ELEMENT_29
		};

		const auto it = applyTypeMap.find(type);
		return it->second;
	}

	static int GetSnakeArmorRandomApplyType(uint8_t type)
	{
		std::map<int, int> iApplyTypeArmor
		{
			{ 0, APPLY_MAX_HP					},
			{ 1, APPLY_SKILL_DEFEND_BONUS		},
			{ 2, APPLY_NORMAL_HIT_DEFEND_BONUS	},
			{ 3, APPLY_KILL_HP_RECOVER			},
			{ 4, APPLY_BLEEDING_REDUCE			},
			{ 5, APPLY_POISON_REDUCE			},
			{ 6, APPLY_RESIST_MOUNT_FALL		},

			{ 7, APPLY_SUNGMA_STR				},
			{ 8, APPLY_SUNGMA_MOVE				},
			{ 9, APPLY_SUNGMA_HP				},
			{ 10, APPLY_SUNGMA_IMMUNE			},

			{ 11, APPLY_MONSTER_DEFEND_BONUS	},
			{ 12, APPLY_ATTBONUS_MONSTER		},
			{ 13, APPLY_STEAL_HP				},
			{ 14, APPLY_STEAL_SP				},
			{ 15, APPLY_RESIST_MAGIC			},
			{ 16, APPLY_MOV_SPEED				},
		};

		const auto it = iApplyTypeArmor.find(type);
		return it->second;
	}

	static int GetSnakeWeaponRandomApplyType(uint8_t type)
	{
		std::map<int, int> iApplyTypeWeapon
		{
			{ 0, APPLY_NORMAL_HIT_DAMAGE_BONUS	},
			{ 1, APPLY_ATTBONUS_HUMAN			},
			{ 2, APPLY_ATTBONUS_DEVIL			},
			{ 3, APPLY_ATTBONUS_ANIMAL			},
			{ 4, APPLY_ATTBONUS_UNDEAD			},
			{ 5, APPLY_ATTBONUS_ORC				},
			{ 6, APPLY_ATTBONUS_MILGYO			},

			{ 7, APPLY_SUNGMA_STR				},
			{ 8, APPLY_SUNGMA_MOVE				},
			{ 9, APPLY_SUNGMA_HP				},
			{ 10, APPLY_SUNGMA_IMMUNE			},
			{ 11, APPLY_ATT_SPEED				},
			{ 12, APPLY_ATTBONUS_MONSTER		},
			{ 13, APPLY_ATTBONUS_INSECT			},
			{ 14, APPLY_ATTBONUS_CZ				},

			{ 15, APPLY_SKILL_DAMAGE_BONUS		},
			{ 16, APPLY_ATTBONUS_STONE			},
			{ 17, APPLY_ATTBONUS_DESERT			},
			{ 18, APPLY_CAST_SPEED				},
			{ 19, APPLY_EXP_DOUBLE_BONUS		},
			{ 20, APPLY_MANA_BURN_PCT			},
			{ 21, APPLY_SLOW_PCT				},
			{ 22, APPLY_STUN_PCT				},
			{ 23, APPLY_HIT_PCT					},
		};

		const auto it = iApplyTypeWeapon.find(type);
		return it->second;
	}

	static bool GenerateRandomAttr(LPITEM pItem)
	{
		if (!pItem)
			return false;

		const int refineVnum = pItem->GetRefineLevel();

#ifdef ENABLE_YOHARA_SYSTEM
		if (IsSerpentItem(pItem->GetVnum()))
		{
			if (!pItem->ItemHasRandomDefaultAttr())
			{
				const TItemTable* item_table = pItem->GetProto();

				for (int i = 0; i < ITEM_RANDOM_VALUES_MAX_NUM; i++)
				{
					if (!item_table->alSockets[i])
						continue;

					std::string foo = std::to_string(item_table->alSockets[i]);
					std::vector<std::string> results;
					split_value(foo, "00", results);

					const auto min = atoi(results[0].c_str());
					const auto max = atoi(results[1].c_str());

					if (min > max)
						sys_err("RandomHelper NumberErr Vnum %d Name %s", pItem->GetVnum(), item_table->szName);

					const auto irandomValue = static_cast<uint16_t>(number(atoi(results[0].c_str()), atoi(results[1].c_str())));
					pItem->SetRandomDefaultAttr(i, irandomValue);
				}
			}
		}
#endif

		//APPLY_RANDOM	1, 2, 3, 4		23000-23009 (1)		23010-23019 (1)		23020-23029 (1)		23030-23039 (1)
		if ((pItem->GetApplyValue(0) == 1 || pItem->GetApplyValue(0) == 2 || pItem->GetApplyValue(0) == 3 || pItem->GetApplyValue(0) == 4) && !pItem->GetApplyRandomType(0))
		{
			const int randomValue = number(0, 4);
			const int iGloveRandomValue = aiItemApplyRandomGlove[refineVnum].byRandomValue[randomValue];
			pItem->SetItemApplyRandom(0, RandomApplyType(pItem->GetApplyValue(0)), iGloveRandomValue, randomValue);
		}

		//APPLY_RANDOM	5		14580-14619 (1)		16580-16619 (1)		17570-17609 (1)
		if (pItem->GetApplyValue(0) == 5 && !pItem->GetApplyRandomType(0))
		{
			const int randomValue = number(0, 4);
			const int iJewelryRandomValue = aiItemApplyRandomJewelry[refineVnum].byRandomValue[randomValue];
			pItem->SetItemApplyRandom(0, RandomApplyType(pItem->GetApplyValue(0)), iJewelryRandomValue, randomValue);
		}

		//APPLY_RANDOM	6, 7, 8, 9		14580-14589 (2)		16580-16589 (2)		17570-17579 (2)		14590-14599 (2)		16590-16599 (2)		17580-17589 (2)		14600-14609 (2)		16600-16609 (2)		17590-17599 (2)		14610-14619 (2)		16610-16619 (2)		17600-17609 (2)
		if ((pItem->GetApplyValue(1) == 6 || pItem->GetApplyValue(1) == 7 || pItem->GetApplyValue(1) == 8 || pItem->GetApplyValue(1) == 9) && !pItem->GetApplyRandomType(1))
		{
			const int randomValue = number(0, 4);
			const int iJewelryRandomValue = aiItemApplyRandomJewelry[(refineVnum + 10)].byRandomValue[randomValue];
			pItem->SetItemApplyRandom(1, RandomApplyType(pItem->GetApplyValue(1)), iJewelryRandomValue, randomValue);
		}

		//APPLY_RANDOM	10, 11, 12, 13		81000-81009 (1)		81250-81259 (1)		81300-81309 (1)		81550-81559 (1)
		if ((pItem->GetApplyValue(0) == 10 || pItem->GetApplyValue(0) == 11 || pItem->GetApplyValue(0) == 12 || pItem->GetApplyValue(0) == 13) && !pItem->GetApplyRandomType(0))	//Talisman
		{
			const int prob = number(1, 100);
			if (prob <= SUNGMA_ATTR_PROB)
			{
				const int randomValue = number(0, 3);
				const int iTalismanRandomValue = aiItemApplyRandomTalisman[refineVnum].byRandomValue[randomValue];
				pItem->SetItemApplyRandom(0, RandomApplyType(pItem->GetApplyValue(0)), iTalismanRandomValue, randomValue);
			}
		}

		//APPLY_RANDOM	14, 15, 16, 17		81000-81009 (2)		81250-81259 (2)		81300-81300 (2)		81550-81559 (2)
		if ((pItem->GetApplyValue(1) == 14 || pItem->GetApplyValue(1) == 15 || pItem->GetApplyValue(1) == 16 || pItem->GetApplyValue(1) == 17) && !pItem->GetApplyRandomType(1))	//Talisman
		{
			const int prob = number(1, 100);
			if (prob <= SUNGMA_ATTR_PROB)
			{
				const int randomValue = number(0, 3);
				const int iApplyType = number(APPLY_ENCHANT_ELECT, APPLY_ENCHANT_DARK);
				const int iTalismanRandomValue = aiItemApplyRandomTalisman[(refineVnum + 10)].byRandomValue[randomValue];
				pItem->SetItemApplyRandom(1, iApplyType, iTalismanRandomValue, randomValue);
			}
		}

		//APPLY_RANDOM	18		360-375 (1)			380-395 (1)			1210-1225 (1)		2230-2245 (1)		3250-3265 (1)		5200-5215 (1)		6150-6165 (1)		7330-7345 (1)
		if (pItem->GetApplyValue(0) == 18 && !pItem->GetApplyRandomType(0))
		{
			const int prob = number(1, 100);
			if (prob <= SUNGMA_ATTR_PROB)
			{
				const int iAttrPos = number(0, 6);
				const int iAttr = GetSnakeWeaponRandomApplyType(iAttrPos);
				const int randomValue = number(0, 2);
				pItem->SetItemApplyRandom(0, iAttr, aiItemApplyRandomSnakeWeapon[iAttrPos].iRefineValues[refineVnum][randomValue], randomValue);
			}
		}

		//APPLY_RANDOM	19		360-375 (2)			380-395 (2)			1210-1225 (2)		2230-2245 (2)		3250-3265 (2)		5200-5215 (2)		6150-6165 (2)		7330-7345 (2)
		if (pItem->GetApplyValue(1) == 19 && !pItem->GetApplyRandomType(1))
		{
			const int prob = number(1, 100);
			if (prob <= SUNGMA_ATTR_PROB)
			{
				const int iAttrPos = number(7, 14);
				const int iAttr = GetSnakeWeaponRandomApplyType(iAttrPos);
				const int randomValue = number(0, 4);
				pItem->SetItemApplyRandom(1, iAttr, aiItemApplyRandomSnakeWeapon[iAttrPos].iRefineValues[refineVnum][randomValue], randomValue);
			}
		}

		//APPLY_RANDOM	20		360-375 (3)			380-395 (3)			1210-1225 (3)		2230-2245 (3)		3250-3265 (3)		5200-5215 (3)		6150-6165 (3)		7330-7345 (3)
		if (pItem->GetApplyValue(2) == 20 && !pItem->GetApplyRandomType(2))
		{
			const int prob = number(1, 100);
			if (prob <= SUNGMA_ATTR_PROB)
			{
				const int iAttrPos = number(15, 23);
				const int iAttr = GetSnakeWeaponRandomApplyType(iAttrPos);
				const int randomValue = number(0, 2);
				pItem->SetItemApplyRandom(2, iAttr, aiItemApplyRandomSnakeWeapon[iAttrPos].iRefineValues[refineVnum][randomValue], randomValue);
			}
		}

		//APPLY_RANDOM	21		21310-21325 (1)		21330-21345 (1)		21350-21365 (1)		21370-21385 (1)		21390-21405 (1)
		if (pItem->GetApplyValue(0) == 21 && !pItem->GetApplyRandomType(0))
		{
			const int prob = number(1, 100);
			if (prob <= SUNGMA_ATTR_PROB)
			{
				const int iAttrPos = number(0, 6);
				const int iAttr = GetSnakeArmorRandomApplyType(iAttrPos);
				const int randomValue = number(0, 4);
				pItem->SetItemApplyRandom(0, iAttr, aiItemApplyRandomSnakeArmor[iAttrPos].iRefineValues[refineVnum][randomValue], randomValue);
			}
		}

		//APPLY_RANDOM	22		21310-21325 (2)		21330-21345 (2)		21350-21365 (2)		21370-21385 (2)		21390-21405 (2)
		if (pItem->GetApplyValue(1) == 22 && !pItem->GetApplyRandomType(1))
		{
			const int prob = number(1, 100);
			if (prob <= SUNGMA_ATTR_PROB)
			{
				const int iAttrPos = number(7, 10);
				const int iAttr = GetSnakeArmorRandomApplyType(iAttrPos);
				const int randomValue = number(0, 4);
				pItem->SetItemApplyRandom(1, iAttr, aiItemApplyRandomSnakeArmor[iAttrPos].iRefineValues[refineVnum][randomValue], randomValue);
			}
		}

		//APPLY_RANDOM	23		21310-21325 (3)		21330-21345 (3)		21350-21365 (3)		21370-21385 (3)		21390-21405 (3)
		if (pItem->GetApplyValue(2) == 23 && !pItem->GetApplyRandomType(2))
		{
			const int prob = number(1, 100);
			if (prob <= SUNGMA_ATTR_PROB)
			{
				const int iAttrPos = number(11, 16);
				const int iAttr = GetSnakeArmorRandomApplyType(iAttrPos);
				const int randomValue = number(0, 4);
				pItem->SetItemApplyRandom(2, iAttr, aiItemApplyRandomSnakeArmor[iAttrPos].iRefineValues[refineVnum][randomValue], randomValue);
			}
		}

		//APPLY_RANDOM	24, 25, 26, 27		23050-23059 (1)		23060-23069 (1)		23070-23079 (1)		23080-23089 (1)
		if ((pItem->GetApplyValue(0) == 24 || pItem->GetApplyValue(0) == 25 || pItem->GetApplyValue(0) == 26 || pItem->GetApplyValue(0) == 27) && !pItem->GetApplyRandomType(0))
		{
			const int randomValue = number(0, 3);
			const int iGloveRandomValue = aiItemApplyRandomSnakeGlove[refineVnum].byRandomValue[randomValue];
			pItem->SetItemApplyRandom(0, RandomApplyType(pItem->GetApplyValue(0)), iGloveRandomValue, randomValue);
		}

		//APPLY_RANDOM	28		81560-81569 (1)
		if (pItem->GetApplyValue(0) == 28 && !pItem->GetApplyRandomType(0))
		{
			const int prob = number(1, 100);
			if (prob <= SUNGMA_ATTR_PROB)
			{
				const int iApplyType = number(APPLY_SUNGMA_STR, APPLY_SUNGMA_IMMUNE);
				const int randomValue = number(0, 3);
				const int iTalismanRandomValue = aiItemApplyRandomTalisman[refineVnum].byRandomValue[randomValue];
				pItem->SetItemApplyRandom(0, iApplyType, iTalismanRandomValue, randomValue);
			}
		}

		//APPLY_RANDOM	29		81560-81569 (2)
		if (pItem->GetApplyValue(1) == 29 && !pItem->GetApplyRandomType(1))
		{
			const int prob = number(1, 100);
			if (prob <= SUNGMA_ATTR_PROB)
			{
				const int randomValue = number(0, 29);
				const int iApplyType = number(APPLY_ENCHANT_ELECT, APPLY_ENCHANT_DARK);
				const int iTalismanRandomValue = aiItemApplyRandomMysterious[(refineVnum)].byRandomValue[randomValue];
				pItem->SetItemApplyRandom(1, iApplyType, iTalismanRandomValue, randomValue);
			}
		}

		return true;
	}

	static bool RefineRandomAttr(LPITEM pItem, LPITEM newItem, bool refinePos)
	{
		if (!pItem || !newItem)
			return false;

		const int currentLevel = pItem->GetRefineLevel();
		const int refineState = refinePos ? (currentLevel + 1) : ((currentLevel - 1) < 0 ? currentLevel : (currentLevel - 1));

		//APPLY_RANDOM	1, 2, 3, 4		23000-23009 (1)		23010-23019 (1)		23020-23029 (1)		23030-23039 (1)
		if ((pItem->GetApplyValue(0) == 1 || pItem->GetApplyValue(0) == 2 || pItem->GetApplyValue(0) == 3 || pItem->GetApplyValue(0) == 4) && pItem->GetApplyRandomType(0))
		{
			const int16_t sNewValue = aiItemApplyRandomGlove[refineState].byRandomValue[pItem->GetApplyRandomRow(0)];
			newItem->SetItemApplyRandom(0, pItem->GetApplyRandomType(0), sNewValue, pItem->GetApplyRandomRow(0));
		}

		//APPLY_RANDOM	5		14580-14619 (1)		16580-16619 (1)		17570-17609 (1)
		if (pItem->GetApplyValue(0) == 5 && pItem->GetApplyRandomType(0))
		{
			const int16_t sNewValue = aiItemApplyRandomJewelry[refineState].byRandomValue[pItem->GetApplyRandomRow(0)];
			newItem->SetItemApplyRandom(0, pItem->GetApplyRandomType(0), sNewValue, pItem->GetApplyRandomRow(0));
		}

		//APPLY_RANDOM	6, 7, 8, 9		14580-14589 (2)		16580-16589 (2)		17570-17579 (2)		14590-14599 (2)		16590-16599 (2)		17580-17589 (2)		14600-14609 (2)		16600-16609 (2)		17590-17599 (2)		14610-14619 (2)		16610-16619 (2)		17600-17609 (2)
		if ((pItem->GetApplyValue(1) == 6 || pItem->GetApplyValue(1) == 7 || pItem->GetApplyValue(1) == 8 || pItem->GetApplyValue(1) == 9) && pItem->GetApplyRandomType(1))
		{
			const int16_t sNewValue = aiItemApplyRandomJewelry[(refineState + 10)].byRandomValue[pItem->GetApplyRandomRow(1)];
			newItem->SetItemApplyRandom(1, pItem->GetApplyRandomType(1), sNewValue, pItem->GetApplyRandomRow(1));
		}

		//APPLY_RANDOM	10, 11, 12, 13		81000-81009 (1)		81250-81259 (1)		81300-81309 (1)		81550-81559 (1)
		if ((pItem->GetApplyValue(0) == 10 || pItem->GetApplyValue(0) == 11 || pItem->GetApplyValue(0) == 12 || pItem->GetApplyValue(0) == 13) /* && pItem->GetApplyRandomType(0)*/)	//Talisman
		{
			const uint16_t applyRandomType = pItem->GetApplyRandomType(0);
			if (!applyRandomType)
			{
				const int randomValue = number(0, 3);
				newItem->SetItemApplyRandom(0, RandomApplyType(pItem->GetApplyValue(0)), aiItemApplyRandomTalisman[refineState].byRandomValue[randomValue], randomValue);
			}
			else
			{
				const int16_t sNewValue = aiItemApplyRandomTalisman[refineState].byRandomValue[pItem->GetApplyRandomRow(0)];
				newItem->SetItemApplyRandom(0, applyRandomType, sNewValue, pItem->GetApplyRandomRow(0));
			}
		}

		//APPLY_RANDOM	14, 15, 16, 17		81000-81009 (2)		81250-81259 (2)		81300-81300 (2)		81550-81559 (2)
		if ((pItem->GetApplyValue(1) == 14 || pItem->GetApplyValue(1) == 15 || pItem->GetApplyValue(1) == 16 || pItem->GetApplyValue(1) == 17) /* && pItem->GetApplyRandomType(1)*/)	//Talisman
		{
			const uint16_t applyRandomType = pItem->GetApplyRandomType(1);
			if (!applyRandomType)
			{
				const int iApplyType = number(APPLY_ENCHANT_ELECT, APPLY_ENCHANT_DARK);
				const int randomValue = number(0, 3);
				newItem->SetItemApplyRandom(1, iApplyType, aiItemApplyRandomTalisman[(refineState + 10)].byRandomValue[randomValue], randomValue);
			}
			else
			{
				const int16_t sNewValue = aiItemApplyRandomTalisman[(refineState + 10)].byRandomValue[pItem->GetApplyRandomRow(1)];
				newItem->SetItemApplyRandom(1, applyRandomType, sNewValue, pItem->GetApplyRandomRow(1));
			}
		}

		int iApplyTypeWeapon[] =
		{
			APPLY_NORMAL_HIT_DAMAGE_BONUS, APPLY_ATTBONUS_HUMAN, APPLY_ATTBONUS_DEVIL, APPLY_ATTBONUS_ANIMAL, APPLY_ATTBONUS_UNDEAD, APPLY_ATTBONUS_ORC, APPLY_ATTBONUS_MILGYO,
			APPLY_SUNGMA_STR, APPLY_SUNGMA_MOVE, APPLY_SUNGMA_HP, APPLY_SUNGMA_IMMUNE, APPLY_ATT_SPEED, APPLY_ATTBONUS_MONSTER, APPLY_ATTBONUS_INSECT, APPLY_ATTBONUS_CZ,
			APPLY_SKILL_DAMAGE_BONUS, APPLY_ATTBONUS_STONE, APPLY_ATTBONUS_DESERT, APPLY_CAST_SPEED, APPLY_EXP_DOUBLE_BONUS, APPLY_MANA_BURN_PCT, APPLY_SLOW_PCT, APPLY_STUN_PCT, APPLY_HIT_PCT
		};

		//APPLY_RANDOM	18		360-375 (1)			380-395 (1)			1210-1225 (1)		2230-2245 (1)		3250-3265 (1)		5200-5215 (1)		6150-6165 (1)		7330-7345 (1)
		if (pItem->GetApplyValue(0) == 18 /* && pItem->GetApplyRandomType(0)*/)
		{
			const uint16_t applyRandomType = pItem->GetApplyRandomType(0);
			if (!applyRandomType)
			{
				const int iAttrPos = number(0, 6);
				const int iAttr = GetSnakeWeaponRandomApplyType(iAttrPos);
				const SRandomSnakeWeaponInfo& info = aiItemApplyRandomSnakeWeapon[iAttrPos];
				const int randomValue = number(0, 2);
				newItem->SetItemApplyRandom(0, iAttr, info.iRefineValues[refineState][randomValue], randomValue);
			}
			else
			{
				const int pos = std::distance(iApplyTypeWeapon, std::find(iApplyTypeWeapon, iApplyTypeWeapon + sizeof(iApplyTypeWeapon) / sizeof(iApplyTypeWeapon[0]), applyRandomType));
				const int16_t sNewValue = aiItemApplyRandomSnakeWeapon[pos].iRefineValues[refineState][pItem->GetApplyRandomRow(0)];
				newItem->SetItemApplyRandom(0, applyRandomType, sNewValue, pItem->GetApplyRandomRow(0));
			}
		}

		//APPLY_RANDOM	19		360-375 (2)			380-395 (2)			1210-1225 (2)		2230-2245 (2)		3250-3265 (2)		5200-5215 (2)		6150-6165 (2)		7330-7345 (2)
		if (pItem->GetApplyValue(1) == 19 /* && pItem->GetApplyRandomType(1)*/)
		{
			const uint16_t applyRandomType = pItem->GetApplyRandomType(1);
			if (!applyRandomType)
			{
				const int iAttrPos = number(7, 14);
				const int iAttr = GetSnakeWeaponRandomApplyType(iAttrPos);
				const SRandomSnakeWeaponInfo& info = aiItemApplyRandomSnakeWeapon[iAttrPos];
				const int randomValue = number(0, 2);
				newItem->SetItemApplyRandom(1, iAttr, info.iRefineValues[refineState][randomValue], randomValue);
			}
			else
			{
				const int pos = std::distance(iApplyTypeWeapon, std::find(iApplyTypeWeapon, iApplyTypeWeapon + sizeof(iApplyTypeWeapon) / sizeof(iApplyTypeWeapon[0]), applyRandomType));
				const int16_t sNewValue = aiItemApplyRandomSnakeWeapon[pos].iRefineValues[refineState][pItem->GetApplyRandomRow(1)];
				newItem->SetItemApplyRandom(1, applyRandomType, sNewValue, pItem->GetApplyRandomRow(1));
			}
		}

		//APPLY_RANDOM	20		360-375 (3)			380-395 (3)			1210-1225 (3)		2230-2245 (3)		3250-3265 (3)		5200-5215 (3)		6150-6165 (3)		7330-7345 (3)
		if (pItem->GetApplyValue(2) == 20 /* && pItem->GetApplyRandomType(2)*/)
		{
			const uint16_t applyRandomType = pItem->GetApplyRandomType(2);
			if (!applyRandomType)
			{
				const int iAttrPos = number(15, 23);
				const int iAttr = GetSnakeWeaponRandomApplyType(iAttrPos);
				const SRandomSnakeWeaponInfo& info = aiItemApplyRandomSnakeWeapon[iAttrPos];
				const int randomValue = number(0, 2);
				newItem->SetItemApplyRandom(2, iAttr, info.iRefineValues[refineState][randomValue], randomValue);
			}
			else
			{
				const int pos = std::distance(iApplyTypeWeapon, std::find(iApplyTypeWeapon, iApplyTypeWeapon + sizeof(iApplyTypeWeapon) / sizeof(iApplyTypeWeapon[0]), applyRandomType));
				const int16_t sNewValue = aiItemApplyRandomSnakeWeapon[pos].iRefineValues[refineState][pItem->GetApplyRandomRow(2)];
				newItem->SetItemApplyRandom(2, applyRandomType, sNewValue, pItem->GetApplyRandomRow(2));
			}
		}

		int iApplyTypeArmor[] =
		{
			APPLY_MAX_HP, APPLY_SKILL_DEFEND_BONUS, APPLY_NORMAL_HIT_DEFEND_BONUS, APPLY_KILL_HP_RECOVER, APPLY_BLEEDING_REDUCE, APPLY_POISON_REDUCE, APPLY_RESIST_MOUNT_FALL,
			APPLY_SUNGMA_STR, APPLY_SUNGMA_MOVE, APPLY_SUNGMA_HP, APPLY_SUNGMA_IMMUNE,
			APPLY_MONSTER_DEFEND_BONUS, APPLY_ATTBONUS_MONSTER, APPLY_STEAL_HP, APPLY_STEAL_SP, APPLY_RESIST_MAGIC, APPLY_MOV_SPEED
		};

		//APPLY_RANDOM	21		21310-21325 (1)		21330-21345 (1)		21350-21365 (1)		21370-21385 (1)		21390-21405 (1)
		if (pItem->GetApplyValue(0) == 21 /* && pItem->GetApplyRandomType(0)*/)
		{
			const uint16_t applyRandomType = pItem->GetApplyRandomType(0);
			if (!applyRandomType)
			{
				const int iAttrPos = number(0, 6);
				const int iAttr = GetSnakeArmorRandomApplyType(iAttrPos);
				const int randomValue = number(0, 4);
				newItem->SetItemApplyRandom(0, iAttr, aiItemApplyRandomSnakeArmor[iAttrPos].iRefineValues[refineState][randomValue], randomValue);
			}
			else
			{
				const int pos = std::distance(iApplyTypeArmor, std::find(iApplyTypeArmor, iApplyTypeArmor + sizeof(iApplyTypeArmor) / sizeof(iApplyTypeArmor[0]), applyRandomType));
				const int16_t sNewValue = aiItemApplyRandomSnakeArmor[pos].iRefineValues[refineState][pItem->GetApplyRandomRow(0)];
				newItem->SetItemApplyRandom(0, applyRandomType, sNewValue, pItem->GetApplyRandomRow(0));
			}
		}

		//APPLY_RANDOM	22		21310-21325 (2)		21330-21345 (2)		21350-21365 (2)		21370-21385 (2)		21390-21405 (2)
		if (pItem->GetApplyValue(1) == 22 /* && pItem->GetApplyRandomType(1)*/)
		{
			const uint16_t applyRandomType = pItem->GetApplyRandomType(1);
			if (!applyRandomType)
			{
				const int iAttrPos = number(7, 10);
				const int iAttr = GetSnakeArmorRandomApplyType(iAttrPos);
				const int randomValue = number(0, 4);
				newItem->SetItemApplyRandom(1, iAttr, aiItemApplyRandomSnakeArmor[iAttrPos].iRefineValues[refineState][randomValue], randomValue);
			}
			else
			{
				const int pos = std::distance(iApplyTypeArmor, std::find(iApplyTypeArmor, iApplyTypeArmor + sizeof(iApplyTypeArmor) / sizeof(iApplyTypeArmor[0]), applyRandomType));
				const int16_t sNewValue = aiItemApplyRandomSnakeArmor[pos].iRefineValues[refineState][pItem->GetApplyRandomRow(1)];
				newItem->SetItemApplyRandom(1, applyRandomType, sNewValue, pItem->GetApplyRandomRow(1));
			}
		}

		//APPLY_RANDOM	23		21310-21325 (3)		21330-21345 (3)		21350-21365 (3)		21370-21385 (3)		21390-21405 (3)
		if (pItem->GetApplyValue(2) == 23 /* && pItem->GetApplyRandomType(2)*/)
		{
			const uint16_t applyRandomType = pItem->GetApplyRandomType(2);
			if (!applyRandomType)
			{
				const int iAttrPos = number(11, 16);
				const int iAttr = GetSnakeArmorRandomApplyType(iAttrPos);
				const int randomValue = number(0, 4);
				newItem->SetItemApplyRandom(2, iAttr, aiItemApplyRandomSnakeArmor[iAttrPos].iRefineValues[refineState][randomValue], randomValue);
			}
			else
			{
				const int pos = std::distance(iApplyTypeArmor, std::find(iApplyTypeArmor, iApplyTypeArmor + sizeof(iApplyTypeArmor) / sizeof(iApplyTypeArmor[0]), applyRandomType));
				const int16_t sNewValue = aiItemApplyRandomSnakeArmor[pos].iRefineValues[refineState][pItem->GetApplyRandomRow(2)];
				newItem->SetItemApplyRandom(2, applyRandomType, sNewValue, pItem->GetApplyRandomRow(2));
			}
		}

		//APPLY_RANDOM	24, 25, 26, 27		23050-23059 (1)		23060-23069 (1)		23070-23079 (1)		23080-23089 (1)
		if ((pItem->GetApplyValue(0) == 24 || pItem->GetApplyValue(0) == 25 || pItem->GetApplyValue(0) == 26 || pItem->GetApplyValue(0) == 27) && pItem->GetApplyRandomType(0))
		{
			const int16_t sNewValue = aiItemApplyRandomSnakeGlove[refineState].byRandomValue[pItem->GetApplyRandomRow(0)];
			newItem->SetItemApplyRandom(0, pItem->GetApplyRandomType(0), sNewValue, pItem->GetApplyRandomRow(0));
		}


		//APPLY_RANDOM	28		81560-81569 (1)
		if (pItem->GetApplyValue(0) == 28 /* && pItem->GetApplyRandomType(0)*/)
		{
			const uint16_t applyRandomType = pItem->GetApplyRandomType(0);
			if (!applyRandomType)
			{
				const int iApplyType = number(APPLY_SUNGMA_STR, APPLY_SUNGMA_IMMUNE);
				const int randomValue = number(0, 3);
				newItem->SetItemApplyRandom(0, iApplyType, aiItemApplyRandomTalisman[refineState].byRandomValue[randomValue], randomValue);
			}
			else
			{
				const int16_t sNewValue = aiItemApplyRandomTalisman[refineState].byRandomValue[pItem->GetApplyRandomRow(0)];
				newItem->SetItemApplyRandom(0, applyRandomType, sNewValue, pItem->GetApplyRandomRow(0));
			}
		}

		//APPLY_RANDOM	29		81560-81569 (2)
		if (pItem->GetApplyValue(1) == 29 /* && pItem->GetApplyRandomType(1)*/)
		{
			const uint16_t applyRandomType = pItem->GetApplyRandomType(1);
			if (!applyRandomType)
			{
				const int iApplyType = number(APPLY_ENCHANT_ELECT, APPLY_ENCHANT_DARK);
				const int randomValue = number(0, 29);
				newItem->SetItemApplyRandom(1, iApplyType, aiItemApplyRandomMysterious[(refineState)].byRandomValue[randomValue], randomValue);
			}
			else
			{
				const int16_t sNewValue = aiItemApplyRandomMysterious[(refineState)].byRandomValue[pItem->GetApplyRandomRow(1)];
				newItem->SetItemApplyRandom(1, applyRandomType, sNewValue, pItem->GetApplyRandomRow(1));
			}
		}

		return true;
	}
};
#endif //__HEADER_BELT_INVENTORY_HELPER__
