#include "stdafx.h"

#include <math.h>
#include "ProtoReader.h"

#include "CsvReader.h"

#include <sstream>

using namespace std;

inline string trim_left(const string& str)
{
	const string::size_type n = str.find_first_not_of(" \t\v\n\r");
	return n == string::npos ? str : str.substr(n, str.length());
}

inline string trim_right(const string& str)
{
	const string::size_type n = str.find_last_not_of(" \t\v\n\r");
	return n == string::npos ? str : str.substr(0, n + 1);
}

string trim(const string& str) { return trim_left(trim_right(str)); }

static string* StringSplit(string strOrigin, string strTok)
{
	uint32_t cutAt; // Cutting position
	int index = 0; // String index
	string* strResult = new string[30]; // Variable to return

	// Repeat until strTok is found
	while ((cutAt = strOrigin.find_first_of(strTok)) != strOrigin.npos)
	{
		if (cutAt > 0) // If the cutting position is greater than 0 (on success)
		{
			strResult[index++] = strOrigin.substr(0, cutAt); // Add to result array
		}
		strOrigin = strOrigin.substr(cutAt + 1); // The original is the rest except the cut
	}

	if (strOrigin.length() > 0) // If the original still remains
	{
		strResult[index++] = strOrigin.substr(0, cutAt); // Add the rest to the result array
	}

	for (int i = 0; i < index; i++)
	{
		strResult[i] = trim(strResult[i]);
	}

	return strResult; // Return result
}

int get_Item_Type_Value(string inputString)
{
	string arType[] =
	{
		"ITEM_NONE",			//0
		"ITEM_WEAPON",			//1
		"ITEM_ARMOR",			//2
		"ITEM_USE",				//3
		"ITEM_AUTOUSE",			//4
		"ITEM_MATERIAL",		//5
		"ITEM_SPECIAL",			//6
		"ITEM_TOOL",			//7
		"ITEM_LOTTERY",			//8
		"ITEM_ELK",				//9
		"ITEM_METIN",			//10
		"ITEM_CONTAINER",		//11
		"ITEM_FISH",			//12
		"ITEM_ROD",				//13
		"ITEM_RESOURCE",		//14
		"ITEM_CAMPFIRE",		//15
		"ITEM_UNIQUE",			//16
		"ITEM_SKILLBOOK",		//17
		"ITEM_QUEST",			//18
		"ITEM_POLYMORPH",		//19
		"ITEM_TREASURE_BOX",	//20
		"ITEM_TREASURE_KEY",	//21
		"ITEM_SKILLFORGET",		//22
		"ITEM_GIFTBOX",			//23
		"ITEM_PICK",			//24
		"ITEM_HAIR",			//25
		"ITEM_TOTEM",			//26
		"ITEM_BLEND",			//27
		"ITEM_COSTUME",			//28
		"ITEM_DS",				//29
		"ITEM_SPECIAL_DS",		//30
		"ITEM_EXTRACT",			//31
		"ITEM_SECONDARY_COIN",	//32
		"ITEM_RING",			//33
		"ITEM_BELT",			//34
		"ITEM_PET",				//35
		"ITEM_MEDIUM",			//36	[ENABLE_MOVE_COSTUME_ATTR}
		"ITEM_GACHA",			//37	[ENABLE_BATTLE_FIELD]
		"ITEM_SOUL",			//38	[ENABLE_SOUL_SYSTEM]
		"ITEM_PASSIVE",			//39	[ENABLE_PASSIVE_SYSTEM]
		"ITEM_MERCENARY"		//40
	};

	int retInt = -1;
	//cout << "Type : " << typeStr << " -> ";
	for (uint32_t j = 0; j < sizeof(arType) / sizeof(arType[0]); j++)
	{
		string tempString = arType[j];
		if (inputString.find(tempString) != string::npos && tempString.find(inputString) != string::npos)
		{
			//cout << j << " ";
			retInt = j;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

int get_Item_SubType_Value(uint32_t type_value, string inputString)
{
	static string arSub1[] = // ITEM_WEAPON
	{
		"WEAPON_SWORD",
		"WEAPON_DAGGER",
		"WEAPON_BOW",
		"WEAPON_TWO_HANDED",
		"WEAPON_BELL",
		"WEAPON_FAN",
		"WEAPON_ARROW",
		"WEAPON_MOUNT_SPEAR",
		"WEAPON_CLAW", // ENABLE_WOLFMAN_CHARACTER
		"WEAPON_QUIVER", // ENABLE_QUIVER_SYSTEM
		"WEAPON_BOUQUET"
	};

	static string arSub2[] = // ITEM_ARMOR
	{
		"ARMOR_BODY",
		"ARMOR_HEAD",
		"ARMOR_SHIELD",
		"ARMOR_WRIST",
		"ARMOR_FOOTS",
		"ARMOR_NECK",
		"ARMOR_EAR",
		"ARMOR_PENDANT", // ENABLE_PENDANT
		"ARMOR_GLOVE", // ENABLE_GLOVE_SYSTEM
		"ARMOR_NUM_TYPES"
	};

	static string arSub3[] = // ITEM_USE
	{
		"USE_POTION",						// 0
		"USE_TALISMAN",						// 1
		"USE_TUNING",						// 2
		"USE_MOVE",							// 3
		"USE_TREASURE_BOX",					// 4
		"USE_MONEYBAG",						// 5
		"USE_BAIT",							// 6
		"USE_ABILITY_UP",					// 7
		"USE_AFFECT",						// 8
		"USE_CREATE_STONE",					// 9
		"USE_SPECIAL",						// 10
		"USE_POTION_NODELAY",				// 11
		"USE_CLEAR",						// 12
		"USE_INVISIBILITY",					// 13
		"USE_DETACHMENT",					// 14
		"USE_BUCKET",						// 15
		"USE_POTION_CONTINUE",				// 16
		"USE_CLEAN_SOCKET",					// 17
		"USE_CHANGE_ATTRIBUTE",				// 18
		"USE_ADD_ATTRIBUTE",				// 19
		"USE_ADD_ACCESSORY_SOCKET",			// 20
		"USE_PUT_INTO_ACCESSORY_SOCKET",	// 21
		"USE_ADD_ATTRIBUTE2",				// 22
		"USE_RECIPE",						// 23
		"USE_CHANGE_ATTRIBUTE2",			// 24
		"USE_BIND",							// 25
		"USE_UNBIND",						// 26
		"USE_TIME_CHARGE_PER",				// 27
		"USE_TIME_CHARGE_FIX",				// 28
		"USE_PUT_INTO_BELT_SOCKET",			// 29
		"USE_PUT_INTO_RING_SOCKET",			// 30
		"USE_CHANGE_COSTUME_ATTR",			// 31
		"USE_RESET_COSTUME_ATTR",			// 32
		"USE_UNK_33",						// 33
		"USE_SELECT_ATTRIBUTE",				// 34
		"USE_FLOWER",						// 35
		"USE_EMOTION_PACK",					// 36
		"USE_ELEMENT_UPGRADE",				// 37 ENABLE_REFINE_ELEMENT
		"USE_ELEMENT_DOWNGRADE",			// 38 ENABLE_REFINE_ELEMENT
		"USE_ELEMENT_CHANGE",				// 39 ENABLE_REFINE_ELEMENT
		"USE_CALL",							// 40
		"USE_POTION_TOWER",					// 41
		"USE_POTION_NODELAY_TOWER",			// 42
		"USE_REMOVE_AFFECT",				// 43
		"USE_EMOTION_TOWER",				// 44
		"USE_SECRET_DUNGEON_SCROLL",		// 45

		// Custom
		"USE_AGGREGATE_MONSTER",
#if defined(ENABLE_AURA_SYSTEM) && defined(ENABLE_AURA_BOOST)
		"USE_PUT_INTO_AURA_SOCKET",	// ENABLE_AURA_SYSTEM
#endif
	};

	static string arSub4[] = // ITEM_AUTOUSE
	{
		"AUTOUSE_POTION",
		"AUTOUSE_ABILITY_UP",
		"AUTOUSE_BOMB",
		"AUTOUSE_GOLD",
		"AUTOUSE_MONEYBAG",
		"AUTOUSE_TREASURE_BOX"
	};

	static string arSub5[] = // ITEM_MATERIAL
	{
		"MATERIAL_LEATHER",
		"MATERIAL_BLOOD",
		"MATERIAL_ROOT",
		"MATERIAL_NEEDLE",
		"MATERIAL_JEWEL",
		"MATERIAL_DS_REFINE_NORMAL",
		"MATERIAL_DS_REFINE_BLESSED",
		"MATERIAL_DS_REFINE_HOLLY",
		"MATERIAL_DS_CHANGE_ATTR",
		"MATERIAL_PASSIVE_WEAPON",
		"MATERIAL_PASSIVE_ARMOR",
		"MATERIAL_PASSIVE_ACCE",
		"MATERIAL_PASSIVE_ELEMENT",
		"UNK5_13",
		"UNK5_14",
		"UNK5_15",
		"UNK5_16",
		"UNK5_17",
		"UNK5_18",
		"UNK5_19",
		"UNK5_20",
		"UNK5_21",
		"UNK5_22",
		"UNK5_23",
		"UNK5_24",
		"UNK5_25",
		"UNK5_26",
		"UNK5_27",
		"UNK5_28",
		"UNK5_29",
		"UNK5_30",
		"UNK5_31",
		"UNK5_32",
		"UNK5_33",
		"UNK5_34",
		"UNK5_35",
		"UNK5_36",
		"UNK5_37",
		"UNK5_38",
		"UNK5_39"
	};

	static string arSub6[] = // ITEM_SPECIAL
	{
		"SPECIAL_MAP",
		"SPECIAL_KEY",
		"SPECIAL_DOC",
		"SPECIAL_SPIRIT"
	};

	static string arSub7[] = // ITEM_TOOL
	{
		"TOOL_FISHING_ROD"
	};

	static string arSub8[] = // ITEM_LOTTERY
	{
		"LOTTERY_TICKET",
		"LOTTERY_INSTANT"
	};

	static string arSub10[] = // ITEM_METIN
	{
		"METIN_NORMAL",
		"METIN_GOLD",
		"METIN_SUNGMA", // ENABLE_YOHARA_SYSTEM
	};

	static string arSub12[] = // ITEM_FISH
	{
		"FISH_ALIVE",
		"FISH_DEAD"
	};

	static string arSub14[] = // ITEM_RESOURCE
	{
		"RESOURCE_FISHBONE",
		"RESOURCE_WATERSTONEPIECE",
		"RESOURCE_WATERSTONE",
		"RESOURCE_BLOOD_PEARL",
		"RESOURCE_BLUE_PEARL",
		"RESOURCE_WHITE_PEARL",
		"RESOURCE_BUCKET",
		"RESOURCE_CRYSTAL",
		"RESOURCE_GEM",
		"RESOURCE_STONE",
		"RESOURCE_METIN",
		"RESOURCE_ORE",
		"RESOURCE_AURA" // ENABLE_AURA_SYSTEM
	};

	static string arSub16[] = // ITEM_UNIQUE
	{
		"UNIQUE_NONE",
		"UNIQUE_BOOK",
		"UNIQUE_SPECIAL_RIDE",
		"UNIQUE_SPECIAL_MOUNT_RIDE",
		"UNIQUE_4",
		"UNIQUE_5",
		"UNIQUE_6",
		"UNIQUE_7",
		"UNIQUE_8",
		"UNIQUE_9",
		"UNIQUE_10" // changed USE_SPECIAL to UNIQUE_10 (USE_SPECIAL used in arSub3, this is arSub16)
	};

	static string arSub18[] = // ITEM_QUEST
	{
		"QUEST_NONE",
		"QUEST_PET_PAY",
		"QUEST_WARP",
		"UNK18_3",
		"UNK18_4",
		"UNK18_5",
		"UNK18_6",
		"UNK18_7",
		"UNK18_8",
		"UNK18_9",
		"UNK18_10"
	};

	static string arSub23[] = //ITEM_GIFTBOX
	{
		"UNK23_1",
		"UNK23_2"
	};

	static string arSub28[] = // ITEM_COSTUME
	{
		"COSTUME_BODY",
		"COSTUME_HAIR",
		"COSTUME_MOUNT",
		"COSTUME_ACCE",
		"COSTUME_WEAPON",
		"COSTUME_AURA" // ENABLE_AURA_SYSTEM
	};

	static string arSub29[] = // ITEM_DS
	{
		"DS_SLOT1",
		"DS_SLOT2",
		"DS_SLOT3",
		"DS_SLOT4",
		"DS_SLOT5",
		"DS_SLOT6"
	};

	static string arSub31[] = // ITEM_EXTRACT
	{
		"EXTRACT_DRAGON_SOUL",
		"EXTRACT_DRAGON_HEART"
	};

	static string arSub35[] = // ITEM_PET
	{
		"PET_EGG",
		"PET_UPBRINGING",
		"PET_BAG",
		"PET_FEEDSTUFF",
		"PET_SKILL",
		"PET_SKILL_DEL_BOOK",
		"PET_NAME_CHANGE",
		"PET_EXPFOOD",
		"PET_SKILL_ALL_DEL_BOOK",
		"PET_EXPFOOD_PER",
		"PET_ITEM_TYPE",
		"PET_ATTR_CHANGE",
		"PET_PAY",
		"PET_PRIMIUM_FEEDSTUFF"
	};

	static string arSub36[] = // ITEM_MEDIUM [ENABLE_MOVE_COSTUME_ATTR]
	{
		"MEDIUM_MOVE_COSTUME_ATTR",
		"MEDIUM_MOVE_ACCE_ATTR"
	};

	static string arSub37[] = // ITEM_GACHA [ENABLE_BATTLE_FIELD]
	{
		"USE_GACHA",
		"GEM_LUCKY_BOX_GACHA",
		"SPECIAL_LUCKY_BOX_GACHA"
	};

	static string arSub38[] = // ITEM_SOUL [ENABLE_SOUL_SYSTEM]
	{
		"SOUL_RED",
		"SOUL_BLUE"
	};

	static string arSub39[] = // ITEM_PASSIVE [ENABLE_PASSIVE_SYSTEM]
	{
		"PASSIVE_JOB",
	};

	static string arSub40[] = //ITEM_MERCENARY
	{
		"MERCENARY_0",
		"MERCENARY_1",
		"MERCENARY_2",
		"MERCENARY_3",
		"MERCENARY_4",
		"MERCENARY_5",
		"MERCENARY_6"
	};

	static string* arSubType[] =
	{
		0,			//0		- ITEM_NONE
		arSub1,		//1		- ITEM_WEAPON
		arSub2,		//2		- ITEM_ARMOR
		arSub3,		//3		- ITEM_USE
		arSub4,		//4		- ITEM_AUTOUSE
		arSub5,		//5		- ITEM_MATERIAL
		arSub6,		//6		- ITEM_SPECIAL
		arSub7,		//7		- ITEM_TOOL
		arSub8,		//8		- ITEM_LOTTERY
		0,			//9		- ITEM_ELK
		arSub10,	//10	- ITEM_METIN
		0,			//11	- ITEM_CONTAINER
		arSub12,	//12	- ITEM_FISH
		0,			//13	- ITEM_ROD
		arSub14,	//14	- ITEM_RESOURCE
		0,			//15	- ITEM_CAMPFIRE
		arSub16,	//16	- ITEM_UNIQUE
		0,			//17	- ITEM_SKILLBOOK
		arSub18, 	//18	- ITEM_QUEST
		0,			//19	- ITEM_POLYMORPH
		0,			//20	- ITEM_TREASURE_BOX
		0,			//21	- ITEM_TREASURE_KEY
		0,			//22	- ITEM_SKILLFORGET
		arSub23,	//23	- ITEM_GIFTBOX
		0,			//24	- ITEM_PICK
		0,			//25	- ITEM_HAIR
		0,			//26	- ITEM_TOTEM
		0,			//27	- ITEM_BLEND
		arSub28,	//28	- ITEM_COSTUME
		arSub29,	//29	- ITEM_DS
		arSub29,	//30	- ITEM_SPECIAL_DS
		arSub31,	//31	- ITEM_EXTRACT
		0,			//32	- ITEM_SECONDARY_COIN
		0,			//33	- ITEM_RING
		0,			//34	- ITEM_BELT
		arSub35,	//35	- ITEM_PET
		arSub36,	//36 	- ITEM_MEDIUM	[ENABLE_MOVE_COSTUME_ATTR]
		arSub37,	//37	- ITEM_GACHA	[ENABLE_BATTLE_FIELD]
		arSub38,	//38	- ITEM_SOUL		[ENABLE_SOUL_SYSTEM]
		arSub39,	//39	- ITEM_PASSIVE	[ENABLE_PASSIVE_SYSTEM]
		arSub40		//40	- ITEM_MERCENARY
	};

	static int arNumberOfSubtype[_countof(arSubType)] =
	{
		0,										//0		- ITEM_NONE
		sizeof(arSub1) / sizeof(arSub1[0]),		//1		- ITEM_WEAPON
		sizeof(arSub2) / sizeof(arSub2[0]),		//2		- ITEM_ARMOR
		sizeof(arSub3) / sizeof(arSub3[0]),		//3		- ITEM_USE
		sizeof(arSub4) / sizeof(arSub4[0]),		//4		- ITEM_AUTOUSE
		sizeof(arSub5) / sizeof(arSub5[0]),		//5		- ITEM_MATERIAL
		sizeof(arSub6) / sizeof(arSub6[0]),		//6		- ITEM_SPECIAL
		sizeof(arSub7) / sizeof(arSub7[0]),		//7		- ITEM_TOOL
		sizeof(arSub8) / sizeof(arSub8[0]),		//8		- ITEM_LOTTERY
		0,										//9		- ITEM_ELK
		sizeof(arSub10) / sizeof(arSub10[0]),	//10	- ITEM_METIN
		0,										//11	- ITEM_CONTAINER
		sizeof(arSub12) / sizeof(arSub12[0]),	//12	- ITEM_FISH
		0,										//13	- ITEM_ROD
		sizeof(arSub14) / sizeof(arSub14[0]),	//14	- ITEM_RESOURCE
		0,										//15	- ITEM_CAMPFIRE
		sizeof(arSub16) / sizeof(arSub16[0]),	//16	- ITEM_UNIQUE
		0,										//17	- ITEM_SKILLBOOK
		sizeof(arSub18) / sizeof(arSub18[0]),	//18	- ITEM_QUEST
		0,										//19	- ITEM_POLYMORPH
		0,										//20	- ITEM_TREASURE_BOX
		0,										//21	- ITEM_TREASURE_KEY
		0,										//22	- ITEM_SKILLFORGET
		sizeof(arSub23) / sizeof(arSub23[0]),	//23	- ITEM_GIFTBOX
		0,										//24	- ITEM_PICK
		0,										//25	- ITEM_HAIR
		0,										//26	- ITEM_TOTEM
		0,										//27	- ITEM_BLEND
		sizeof(arSub28) / sizeof(arSub28[0]),	//28	- ITEM_COSTUME
		sizeof(arSub29) / sizeof(arSub29[0]),	//29	- ITEM_DS
		sizeof(arSub29) / sizeof(arSub29[0]),	//30	- ITEM_SPECIAL_DS
		sizeof(arSub31) / sizeof(arSub31[0]),	//31	- ITEM_EXTRACT
		0,										//32	- ITEM_SECONDARY_COIN
		0,										//33	- ITEM_RING
		0,										//34	- ITEM_BELT
		sizeof(arSub35)/sizeof(arSub35[0]),		//35	- ITEM_PET
		sizeof(arSub36)/sizeof(arSub36[0]),		//36	- ITEM_MEDIUM	[ENABLE_MOVE_COSTUME_ATTR]
		sizeof(arSub37) / sizeof(arSub37[0]),	//37	- ITEM_GACHA	[ENABLE_BATTLE_FIELD]
		sizeof(arSub38) / sizeof(arSub38[0]),	//38	- ITEM_SOUL		[ENABLE_SOUL_SYSTEM]
		sizeof(arSub39) / sizeof(arSub39[0]),	//39	- ITEM_PASSIVE	[ENABLE_PASSIVE_SYSTEM]
		sizeof(arSub40) / sizeof(arSub40[0]),	//40	- ITEM_MERCENARY
	};

	assert(_countof(arSubType) > type_value && "Subtype rule: Out of range!!");

	// assert It doesn't seem to work...
	if (_countof(arSubType) <= type_value)
	{
		sys_err("SubType : Out of range!! (type_value: %d, count of registered subtype: %d", type_value, _countof(arSubType));
		return -1;
	}

	// Check if an item type subtype array exists, otherwise return 0
	if (arSubType[type_value] == 0)
	{
		return 0;
	}
	//

	int retInt = -1;
	//cout << "SubType : " << subTypeStr << " -> ";
	for (int j = 0; j < arNumberOfSubtype[type_value]; j++)
	{
		string tempString = arSubType[type_value][j];
		string tempInputString = trim(inputString);
		if (tempInputString.compare(tempString) == 0)
		{
			//cout << j << " ";
			retInt = j;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

int get_Mask_Type_Value(string inputString)
{
	string arMaskType[] =
	{
		"MASK_ITEM_TYPE_NONE",
		"MASK_ITEM_TYPE_MOUNT_PET",
		"MASK_ITEM_TYPE_EQUIPMENT_WEAPON",
		"MASK_ITEM_TYPE_EQUIPMENT_ARMOR",
		"MASK_ITEM_TYPE_EQUIPMENT_JEWELRY",
		"MASK_ITEM_TYPE_TUNING",
		"MASK_ITEM_TYPE_POTION",
		"MASK_ITEM_TYPE_FISHING_PICK",
		"MASK_ITEM_TYPE_DRAGON_STONE",
		"MASK_ITEM_TYPE_COSTUMES",
		"MASK_ITEM_TYPE_SKILL",
		"MASK_ITEM_TYPE_UNIQUE",
		"MASK_ITEM_TYPE_ETC",
		"MASK_ITEM_TYPE_MAX"
	};

	int retInt = -1;
	for (uint32_t j = 0; j < sizeof(arMaskType) / sizeof(arMaskType[0]); j++)
	{
		string tempString = arMaskType[j];
		if (inputString.find(tempString) != string::npos && tempString.find(inputString) != string::npos)
		{
			retInt = j;
			break;
		}
	}

	return retInt;
}

int get_Mask_SubType_Value(uint32_t type_value, string inputString)
{
	static string arMaskSub1[] =
	{
		"MASK_ITEM_SUBTYPE_MOUNT_PET_MOUNT",
		"MASK_ITEM_SUBTYPE_MOUNT_PET_CHARGED_PET",
		"MASK_ITEM_SUBTYPE_MOUNT_PET_FREE_PET",
		"MASK_ITEM_SUBTYPE_MOUNT_PET_EGG"
	};

	static string arMaskSub2[] =
	{
		"MASK_ITEM_SUBTYPE_WEAPON_WEAPON_SWORD",
		"MASK_ITEM_SUBTYPE_WEAPON_WEAPON_DAGGER",
		"MASK_ITEM_SUBTYPE_WEAPON_WEAPON_BOW",
		"MASK_ITEM_SUBTYPE_WEAPON_WEAPON_TWO_HANDED",
		"MASK_ITEM_SUBTYPE_WEAPON_WEAPON_BELL",
		"MASK_ITEM_SUBTYPE_WEAPON_WEAPON_CLAW",
		"MASK_ITEM_SUBTYPE_WEAPON_WEAPON_FAN",
		"MASK_ITEM_SUBTYPE_WEAPON_WEAPON_MOUNT_SPEAR",
		"MASK_ITEM_SUBTYPE_WEAPON_WEAPON_ARROW",
		"MASK_ITEM_SUBTYPE_WEAPON_WEAPON_QUIVER"
	};

	static string arMaskSub3[] =
	{
		"MASK_ITEM_SUBTYPE_ARMOR_ARMOR_BODY",
		"MASK_ITEM_SUBTYPE_ARMOR_ARMOR_HEAD",
		"MASK_ITEM_SUBTYPE_ARMOR_ARMOR_SHIELD",
		"MASK_ITEM_SUBTYPE_ARMOR_ARMOR_GLOVE" // ENABLE_GLOVE_SYSTEM
	};

	static string arMaskSub4[] =
	{
		"MASK_ITEM_SUBTYPE_JEWELRY_ARMOR_WRIST",
		"MASK_ITEM_SUBTYPE_JEWELRY_ARMOR_FOOTS",
		"MASK_ITEM_SUBTYPE_JEWELRY_ARMOR_NECK",
		"MASK_ITEM_SUBTYPE_JEWELRY_ARMOR_EAR",
		"MASK_ITEM_SUBTYPE_JEWELRY_ITEM_BELT",
		"MASK_ITEM_SUBTYPE_JEWELRY_ARMOR_PENDANT"
	};

	static string arMaskSub5[] =
	{
		"MASK_ITEM_SUBTYPE_TUNING_RESOURCE",
		"MASK_ITEM_SUBTYPE_TUNING_STONE",
		"MASK_ITEM_SUBTYPE_TUNING_ETC"
	};

	static string arMaskSub6[] =
	{
		"MASK_ITEM_SUBTYPE_POTION_ABILITY",
		"MASK_ITEM_SUBTYPE_POTION_HAIRDYE",
		"MASK_ITEM_SUBTYPE_POTION_ETC"
	};

	static string arMaskSub7[] =
	{
		"MASK_ITEM_SUBTYPE_FISHING_PICK_FISHING_POLE",
		"MASK_ITEM_SUBTYPE_FISHING_PICK_EQUIPMENT_PICK",
		"MASK_ITEM_SUBTYPE_FISHING_PICK_FOOD",
		"MASK_ITEM_SUBTYPE_FISHING_PICK_STONE",
		"MASK_ITEM_SUBTYPE_FISHING_PICK_ETC"
	};

	static string arMaskSub8[] =
	{
		"MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_DIAMOND",
		"MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_RUBY",
		"MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_JADE",
		"MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_SAPPHIRE",
		"MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_GARNET",
		"MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_ONYX",
		"MASK_ITEM_SUBTYPE_DRAGON_STONE_ETC"
	};

	static string arMaskSub9[] =
	{
		"MASK_ITEM_SUBTYPE_COSTUMES_COSTUME_WEAPON",
		"MASK_ITEM_SUBTYPE_COSTUMES_COSTUME_BODY",
		"MASK_ITEM_SUBTYPE_COSTUMES_COSTUME_HAIR",
		"MASK_ITEM_SUBTYPE_COSTUMES_SASH",
		"MASK_ITEM_SUBTYPE_COSTUMES_ETC",
		"MASK_ITEM_SUBTYPE_COSTUMES_AURA", // ENABLE_AURA_SYSTEM
	};

	static string arMaskSub10[] =
	{
		"MASK_ITEM_SUBTYPE_SKILL_PAHAE",
		"MASK_ITEM_SUBTYPE_SKILL_SKILL_BOOK",
		"MASK_ITEM_SUBTYPE_SKILL_BOOK_OF_OBLIVION",
		"MASK_ITEM_SUBTYPE_SKILL_ETC",
		"MASK_ITEM_SUBTYPE_SKILL_SKILL_BOOK_HORSE"
	};

	static string arMaskSub11[] =
	{
		"MASK_ITEM_SUBTYPE_UNIQUE_ABILITY",
		"MASK_ITEM_SUBTYPE_UNIQUE_ETC"
	};

	static string arMaskSub12[] =
	{
		"MASK_ITEM_SUBTYPE_ETC_GIFTBOX",
		"MASK_ITEM_SUBTYPE_ETC_MATRIMORY",
		"MASK_ITEM_SUBTYPE_ETC_EVENT",
		"MASK_ITEM_SUBTYPE_ETC_SEAL",
		"MASK_ITEM_SUBTYPE_ETC_PARTI",
		"MASK_ITEM_SUBTYPE_ETC_POLYMORPH",
		"MASK_ITEM_SUBTYPE_ETC_RECIPE",
		"MASK_ITEM_SUBTYPE_ETC_ETC"
	};

	static string* arMaskSubType[] =
	{
		0, //0
		arMaskSub1, //1
		arMaskSub2, //2
		arMaskSub3, //3
		arMaskSub4, //4
		arMaskSub5, //5
		arMaskSub6, //6
		arMaskSub7, //7
		arMaskSub8, //8
		arMaskSub9, //9
		arMaskSub10, //10
		arMaskSub11, //11
		arMaskSub12, //12
		0, //14
	};

	static int arNumberOfMaskSubtype[14] =
	{
		0,
		sizeof(arMaskSub1) / sizeof(arMaskSub1[0]),
		sizeof(arMaskSub2) / sizeof(arMaskSub2[0]),
		sizeof(arMaskSub3) / sizeof(arMaskSub3[0]),
		sizeof(arMaskSub4) / sizeof(arMaskSub4[0]),
		sizeof(arMaskSub5) / sizeof(arMaskSub5[0]),
		sizeof(arMaskSub6) / sizeof(arMaskSub6[0]),
		sizeof(arMaskSub7) / sizeof(arMaskSub7[0]),
		sizeof(arMaskSub8) / sizeof(arMaskSub8[0]),
		sizeof(arMaskSub9) / sizeof(arMaskSub9[0]),
		sizeof(arMaskSub10) / sizeof(arMaskSub10[0]),
		sizeof(arMaskSub11) / sizeof(arMaskSub11[0]),
		sizeof(arMaskSub12) / sizeof(arMaskSub12[0]),
		0,
	};

	if (arMaskSubType[type_value] == 0)
	{
		return 0;
	}

	int retInt = -1;
	for (int j = 0; j < arNumberOfMaskSubtype[type_value]; j++)
	{
		string tempString = arMaskSubType[type_value][j];
		string tempInputString = trim(inputString);
		if (tempInputString.compare(tempString) == 0)
		{
			retInt = j;
			break;
		}
	}

	return retInt;
}

int get_Item_AntiFlag_Value(string inputString)
{
	string arAntiFlag[] =
	{
		"ANTI_FEMALE",			// 0
		"ANTI_MALE",			// 1
		"ANTI_MUSA",			// 2
		"ANTI_ASSASSIN",		// 3
		"ANTI_SURA",			// 4
		"ANTI_MUDANG",			// 5
		"ANTI_GET",				// 6
		"ANTI_DROP",			// 7
		"ANTI_SELL",			// 8
		"ANTI_EMPIRE_A",		// 9
		"ANTI_EMPIRE_B",		// 10
		"ANTI_EMPIRE_C",		// 11
		"ANTI_SAVE",			// 12
		"ANTI_GIVE",			// 13
		"ANTI_PKDROP",			// 14
		"ANTI_STACK",			// 15
		"ANTI_MYSHOP",			// 16
		"ANTI_SAFEBOX",			// 17
		"ANTI_WOLFMAN",			// 18	[ENABLE_WOLFMAN_CHARACTER]
		"ANTI_RT_REMOVE",		// 19
		"ANTI_QUICKSLOT",		// 20	[ENABLE_ANTI_QUICKSLOT]
		"ANTI_CHANGELOOK",		// 21	[ENABLE_CHANGE_LOOK_SYSTEM]
		"ANTI_REINFORCE",		// 22
		"ANTI_ENCHANT",			// 23
		"ANTI_ENERGY",			// 24
		"ANTI_PETFEED",			// 25	[ENABLE_GROWTH_PET_SYSTEM]
		"ANTI_APPLY",			// 26
		"ANTI_ACCE",			// 27	[ENABLE_ACCE_COSTUME_SYSTEM]
		"ANTI_MAIL"				// 28	[ENABLE_MAILBOX]
	};

	int retValue = 0;
	string* arInputString = StringSplit(inputString, "|"); // Arrange the contents of the proto information into words.
	for (uint32_t i = 0; i < sizeof(arAntiFlag) / sizeof(arAntiFlag[0]); i++)
	{
		string tempString = arAntiFlag[i];
		for (uint32_t j = 0; j < 30; j++) // Up to 30 words. (Hard coding)
		{
			string tempString2 = arInputString[j];
			if (tempString2.compare(tempString) == 0) // Make sure it matches.
			{
				retValue = retValue + static_cast<int>(pow(static_cast<float>(2), static_cast<float>(i)));
			}

			if (tempString2.compare("") == 0)
				break;
		}
	}
	delete[]arInputString;
	//cout << "AntiFlag : " << antiFlagStr << " -> " << retValue << endl;

	return retValue;
}

int get_Item_Flag_Value(string inputString)
{
	string arFlag[] =
	{
		"ITEM_TUNABLE",			// 0	ITEM_FLAG_REFINEABLE
		"ITEM_SAVE",			// 1	ITEM_FLAG_SAVE
		"ITEM_STACKABLE",		// 2	ITEM_FLAG_STACKABLE
		"COUNT_PER_1GOLD",		// 3	ITEM_FLAG_COUNT_PER_1GOLD
		"ITEM_SLOW_QUERY",		// 4	ITEM_FLAG_SLOW_QUERY
		"ITEM_UNIQUE",			// 5	ITEM_FLAG_UNIQUE - Unused
		"ITEM_MAKECOUNT",		// 6	ITEM_FLAG_MAKECOUNT
		"ITEM_IRREMOVABLE",		// 7	ITEM_FLAG_IRREMOVABLE
		"CONFIRM_WHEN_USE",		// 8	ITEM_FLAG_CONFIRM_WHEN_USE
		"QUEST_USE",			// 9	ITEM_FLAG_QUEST_USE
		"QUEST_USE_MULTIPLE",	// 10	ITEM_FLAG_QUEST_USE_MULTIPLE
		"QUEST_GIVE",			// 11	ITEM_FLAG_QUEST_GIVE
		"LOG",					// 12	ITEM_FLAG_LOG
		"ITEM_APPLICABLE",		// 13	ITEM_FLAG_APPLICABLE
#ifdef ENABLE_PROTO_RENEWAL_CUSTOM
		"ITEM_GROUP_DMG_WEAPON"	// 14 ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
#endif
	};

	int retValue = 0;
	string* arInputString = StringSplit(inputString, "|"); // Arrange the contents of the proto information into words.
	for (uint32_t i = 0; i < sizeof(arFlag) / sizeof(arFlag[0]); i++)
	{
		string tempString = arFlag[i];
		for (uint32_t j = 0; j < 30; j++) // Up to 30 words. (Hard coding)
		{
			string tempString2 = arInputString[j];
			if (tempString2.compare(tempString) == 0) // Make sure it matches.
			{
				retValue = retValue + static_cast<int>(pow(static_cast<float>(2), static_cast<float>(i)));
			}

			if (tempString2.compare("") == 0)
				break;
		}
	}
	delete[]arInputString;
	//cout << "Flag : " << flagStr << " -> " << retValue << endl;

	return retValue;
}

int get_Item_WearFlag_Value(string inputString)
{
	string arWearrFlag[] =
	{
		"WEAR_BODY",	// 0
		"WEAR_HEAD",	// 1
		"WEAR_FOOTS",	// 2
		"WEAR_WRIST",	// 3
		"WEAR_WEAPON",	// 4
		"WEAR_NECK",	// 5
		"WEAR_EAR",		// 6
		"WEAR_SHIELD",	// 7
		"WEAR_UNIQUE",	// 8
		"WEAR_ARROW",	// 9
		"WEAR_HAIR",	// 10
		"WEAR_ABILITY",	// 11
		"WEAR_PENDANT",	// 12	ENABLE_PENDANT
		"WEAR_GLOVE",	// 13	ENABLE_GLOVE_SYSTEM
#if defined(ENABLE_PROTO_RENEWAL_CUSTOM) && defined(ENABLE_PET_SYSTEM)
		"WEAR_PET"		// 14
#endif
	};

	int retValue = 0;
	string* arInputString = StringSplit(inputString, "|"); // Arrange the contents of the proto information into words.
	for (uint32_t i = 0; i < sizeof(arWearrFlag) / sizeof(arWearrFlag[0]); i++)
	{
		string tempString = arWearrFlag[i];
		for (uint32_t j = 0; j < 30; j++) // Up to 30 words. (Hard coding)
		{
			string tempString2 = arInputString[j];
			if (tempString2.compare(tempString) == 0) // Make sure it matches.
			{
				retValue = retValue + static_cast<int>(pow(static_cast<float>(2), static_cast<float>(i)));
			}

			if (tempString2.compare("") == 0)
				break;
		}
	}
	delete[]arInputString;
	//cout << "WearFlag : " << wearFlagStr << " -> " << retValue << endl;

	return retValue;
}

int get_Item_Immune_Value(string inputString)
{
	string arImmune[] =
	{
		"PARA",
		"CURSE",
		"STUN",
		"SLEEP",
		"SLOW",
		"POISON",
		"TERROR",
		"REFLECT"
	};

	int retValue = 0;
	string* arInputString = StringSplit(inputString, "|"); // Arrange the contents of the proto information into words.
	for (uint32_t i = 0; i < sizeof(arImmune) / sizeof(arImmune[0]); i++)
	{
		string tempString = arImmune[i];
		for (uint32_t j = 0; j < 30; j++) // Up to 30 words. (Hard coding)
		{
			string tempString2 = arInputString[j];
			if (tempString2.compare(tempString) == 0)
			{
				retValue = retValue + static_cast<int>(pow(static_cast<float>(2), static_cast<float>(i)));
			}

			if (tempString2.compare("") == 0)
				break;
		}
	}
	delete[]arInputString;
	//cout << "Immune : " << immuneStr << " -> " << retValue << endl;

	return retValue;
}

int get_Item_LimitType_Value(string inputString)
{
	string arLimitType[] =
	{
		"LIMIT_NONE",
		"LEVEL",
		"STR",
		"DEX",
		"INT",
		"CON",
		"REAL_TIME",
		"REAL_TIME_FIRST_USE",
		"TIMER_BASED_ON_WEAR",
		"NEWWORLD_LEVEL", // ENABLE_YOHARA_SYSTEM
		"DURATION",
	};

	int retInt = -1;
	//cout << "LimitType : " << limitTypeStr << " -> ";
	for (uint32_t j = 0; j < sizeof(arLimitType) / sizeof(arLimitType[0]); j++)
	{
		string tempString = arLimitType[j];
		string tempInputString = trim(inputString);
		if (tempInputString.compare(tempString) == 0)
		{
			//cout << j << " ";
			retInt = j;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

int get_Item_ApplyType_Value(string inputString)
{
	string arApplyType[] =
	{
		// the lenght of string less is necessary for can use long enum in applys if you use proto from db
		"NONE",
		"MAX_HP",
		"MAX_SP",
		"CON",
		"INT",
		"STR",
		"DEX",
		"ATT_SPEED",
		"MOV_SPEED",
		"CAST_SPEED",
		"HP_REGEN", // 10
		"SP_REGEN",
		"POISON_PCT",
		"STUN_PCT",
		"SLOW_PCT",
		"CRITICAL_PCT",
		"PENETRATE_PCT",
		"ATTBONUS_HUMAN",
		"ATTBONUS_ANIMAL",
		"ATTBONUS_ORC",
		"ATTBONUS_MILGYO", // 20
		"ATTBONUS_UNDEAD",
		"ATTBONUS_DEVIL",
		"STEAL_HP",
		"STEAL_SP",
		"MANA_BURN_PCT",
		"DAMAGE_SP_RECOVER",
		"BLOCK",
		"DODGE",
		"RESIST_SWORD",
		"RESIST_TWOHAND", // 30
		"RESIST_DAGGER",
		"RESIST_BELL",
		"RESIST_FAN",
		"RESIST_BOW",
		"RESIST_FIRE",
		"RESIST_ELEC",
		"RESIST_MAGIC",
		"RESIST_WIND",
		"REFLECT_MELEE",
		"REFLECT_CURSE", // 40
		"POISON_REDUCE",
		"KILL_SP_RECOVER",
		"EXP_DOUBLE_BONUS",
		"GOLD_DOUBLE_BONUS",
		"ITEM_DROP_BONUS",
		"POTION_BONUS",
		"KILL_HP_RECOVER",
		"IMMUNE_STUN",
		"IMMUNE_SLOW",
		"IMMUNE_FALL", // 50
		"SKILL",
		"BOW_DISTANCE",
		"ATT_GRADE_BONUS",
		"DEF_GRADE_BONUS",
		"MAGIC_ATT_GRADE_BONUS",
		"MAGIC_DEF_GRADE_BONUS",
		"CURSE_PCT",
		"MAX_STAMINA",
		"ATT_BONUS_TO_WARRIOR",
		"ATT_BONUS_TO_ASSASSIN", // 60
		"ATT_BONUS_TO_SURA",
		"ATT_BONUS_TO_SHAMAN",
		"ATT_BONUS_TO_MONSTER",
		"ATT_BONUS",
		"MALL_DEFBONUS",
		"MALL_EXPBONUS",
		"MALL_ITEMBONUS",
		"MALL_GOLDBONUS",
		"MAX_HP_PCT",
		"MAX_SP_PCT", // 70
		"SKILL_DAMAGE_BONUS",
		"NORMAL_HIT_DAMAGE_BONUS",
		"SKILL_DEFEND_BONUS",
		"NORMAL_HIT_DEFEND_BONUS",
		"EXTRACT_HP_PCT",
		"PC_BANG_EXP_BONUS",
		"PC_BANG_DROP_BONUS",
		"RESIST_WARRIOR",
		"RESIST_ASSASSIN",
		"RESIST_SURA", // 80
		"RESIST_SHAMAN",
		"ENERGY",
		"DEF_GRADE",
		"COSTUME_ATTR_BONUS",
		"MAGIC_ATT_BONUS_PER",
		"MELEE_MAGIC_ATT_BONUS_PER",
		"RESIST_ICE",
		"RESIST_EARTH",
		"RESIST_DARK",
		"RESIST_CRITICAL", // 90
		"RESIST_PENETRATE",
		"BLEEDING_REDUCE",
		"BLEEDING_PCT",
		"ATT_BONUS_TO_WOLFMAN",
		"RESIST_WOLFMAN",
		"RESIST_CLAW",
		"ACCEDRAIN_RATE",
		"RESIST_MAGIC_REDUCTION",
		"ENCHANT_ELECT",
		"ENCHANT_FIRE", // 100
		"ENCHANT_ICE",
		"ENCHANT_WIND",
		"ENCHANT_EARTH",
		"ENCHANT_DARK",
		"ATTBONUS_CZ",
		"ATTBONUS_INSECT",
		"ATTBONUS_DESERT",
		"ATTBONUS_SWORD",
		"ATTBONUS_TWOHAND",
		"ATTBONUS_DAGGER", // 110
		"ATTBONUS_BELL",
		"ATTBONUS_FAN",
		"ATTBONUS_BOW",
		"ATTBONUS_CLAW",
		"RESIST_HUMAN",
		"RESIST_MOUNT_FALL",
		"RESIST_FIST",
		"MOUNT",
		"SK_DMG_SAMYEON",
		"SK_DMG_TANHWAN", // 120
		"SK_DMG_PALBANG",
		"SK_DMG_GIGONGCHAM",
		"SK_DMG_GYOKSAN",
		"SK_DMG_GEOMPUNG",
		"SK_DMG_AMSEOP",
		"SK_DMG_GUNGSIN",
		"SK_DMG_CHARYUN",
		"SK_DMG_SANGONG",
		"SK_DMG_YEONSA",
		"SK_DMG_KWANKYEOK", // 130
		"SK_DMG_GIGUNG",
		"SK_DMG_HWAJO",
		"SK_DMG_SWAERYUNG",
		"SK_DMG_YONGKWON",
		"SK_DMG_PABEOB",
		"SK_DMG_MARYUNG",
		"SK_DMG_HWAYEOMPOK",
		"SK_DMG_MAHWAN",
		"SK_DMG_BIPABU",
		"SK_DMG_YONGBI", // 140
		"SK_DMG_PAERYONG",
		"SK_DMG_NOEJEON",
		"SK_DMG_BYEURAK",
		"SK_DMG_CHAIN",
		"SK_DMG_CHAYEOL",
		"SK_DMG_SALPOONG",
		"SK_DMG_GONGDAB",
		"SK_DMG_PASWAE",
		"NORMAL_HIT_DEFEND_BONUS_BOSS_OR_MORE",
		"SK_DEFEND_BONUS_BOSS_OR_MORE", // 150
		"NORMAL_HIT_DAMAGE_BONUS_BOSS_OR_MORE",
		"SK_DMG_BONUS_BOSS_OR_MORE",
		"HIT_BUFF_ENCHANT_FIRE",
		"HIT_BUFF_ENCHANT_ICE",
		"HIT_BUFF_ENCHANT_ELEC",
		"HIT_BUFF_ENCHANT_WIND",
		"HIT_BUFF_ENCHANT_DARK",
		"HIT_BUFF_ENCHANT_EARTH",
		"HIT_BUFF_RESIST_FIRE",
		"HIT_BUFF_RESIST_ICE", // 160
		"HIT_BUFF_RESIST_ELEC",
		"HIT_BUFF_RESIST_WIND",
		"HIT_BUFF_RESIST_DARK",
		"HIT_BUFF_RESIST_EARTH",
		"USE_SK_CHEONGRANG_MOV_SPEED",
		"USE_SK_CHEONGRANG_CASTING_SPEED",
		"USE_SK_CHAYEOL_CRITICAL_PCT",
		"USE_SK_SANGONG_ATT_GRADE_BONUS",
		"USE_SK_GIGUNG_ATT_GRADE_BONUS",
		"USE_SK_JEOKRANG_DEF_BONUS", // 170
		"USE_SK_GWIGEOM_DEF_BONUS",
		"USE_SK_TERROR_ATT_GRADE_BONUS",
		"USE_SK_MUYEONG_ATT_GRADE_BONUS",
		"USE_SK_MANASHILED_CASTING_SPEED",
		"USE_SK_HOSIN_DEF_BONUS",
		"USE_SK_GICHEON_ATT_GRADE_BONUS",
		"USE_SK_JEONGEOP_ATT_GRADE_BONUS",
		"USE_SK_JEUNGRYEOK_DEF_BONUS",
		"USE_SK_GIHYEOL_ATT_GRADE_BONUS",
		"USE_SK_CHUNKEON_CASTING_SPEED", // 180
		"USE_SK_NOEGEOM_ATT_GRADE_BONUS",
		"SK_DURATION_INCREASE_EUNHYUNG",
		"SK_DURATION_INCREASE_GYEONGGONG",
		"SK_DURATION_INCREASE_GEOMKYUNG",
		"SK_DURATION_INCREASE_JEOKRANG",
		"USE_SK_PALBANG_HP_ABSORB",
		"USE_SK_AMSEOP_HP_ABSORB",
		"USE_SK_YEONSA_HP_ABSORB",
		"USE_SK_YONGBI_HP_ABSORB",
		"USE_SK_CHAIN_HP_ABSORB", // 190
		"USE_SK_PASWAE_SP_ABSORB",
		"USE_SK_GIGONGCHAM_STUN",
		"USE_SK_CHARYUN_STUN",
		"USE_SK_PABEOB_STUN",
		"USE_SK_MAHWAN_STUN",
		"USE_SK_GONGDAB_STUN",
		"USE_SK_SAMYEON_STUN",
		"USE_SK_GYOKSAN_KNOCK",
		"USE_SK_SEOMJEON_KNOCK",
		"USE_SK_SWAERYUNG_KNOCK", // 200
		"USE_SK_HWAYEOMPOK_KNOCK",
		"USE_SK_GONGDAB_KNOCK",
		"USE_SK_KWANKYEOK_KNOCK",
		"USE_SK_SAMYEON_NEXT_COOL_DEC_10PER",
		"USE_SK_GEOMPUNG_NEXT_COOL_DEC_10PER",
		"USE_SK_GUNGSIN_NEXT_COOL_DEC_10PER",
		"USE_SK_KWANKYEOK_NEXT_COOL_DEC_10PER",
		"USE_SK_YONGKWON_NEXT_COOL_DEC_10PER",
		"USE_SK_MARYUNG_NEXT_COOL_DEC_10PER",
		"USE_SK_BIPABU_NEXT_COOL_DEC_10PER", // 210
		"USE_SK_NOEJEON_NEXT_COOL_DEC_10PER",
		"USE_SK_SALPOONG_NEXT_COOL_DEC_10PER",
		"USE_SK_PASWAE_NEXT_COOL_DEC_10PER",
		"ATTBONUS_STONE",
		"DMG_HP_RECOVERY",
		"DMG_SP_RECOVERY",
		"ALIGNMENT_DMG_BONUS",
		"NORMAL_DMG_GUARD",
		"MORE_THEN_HP90_DMG_REDUCE",
		"USE_SK_TUSOK_HP_ABSORB", // 220
		"USE_SK_PAERYONG_HP_ABSORB",
		"USE_SK_BYEURAK_HP_ABSORB",
		"FIRST_ATTR_BONUS",
		"SECOND_ATTR_BONUS",
		"THIRD_ATTR_BONUS",
		"FOURTH_ATTR_BONUS",
		"FIFTH_ATTR_BONUS",
		"USE_SK_SAMYEON_NEXT_COOL_DEC_20PER",
		"USE_SK_GEOMPUNG_NEXT_COOL_DEC_20PER",
		"USE_SK_GUNGSIN_NEXT_COOL_DEC_20PER", // 230
		"USE_SK_KWANKYEOK_NEXT_COOL_DEC_20PER",
		"USE_SK_YONGKWON_NEXT_COOL_DEC_20PER",
		"USE_SK_MARYUNG_NEXT_COOL_DEC_20PER",
		"USE_SK_BIPABU_NEXT_COOL_DEC_20PER",
		"USE_SK_NOEJEON_NEXT_COOL_DEC_20PER",
		"USE_SK_SALPOONG_NEXT_COOL_DEC_20PER",
		"USE_SK_PASWAE_NEXT_COOL_DEC_20PER",
		"USE_SK_CHAYEOL_HP_ABSORB",
		"SUNGMA_STR",
		"SUNGMA_HP", // 240
		"SUNGMA_MOVE",
		"SUNGMA_IMMUNE",
		"HIT_PCT",
		"RANDOM",
		"ATTBONUS_PER_HUMAN",
		"ATTBONUS_PER_ANIMAL",
		"ATTBONUS_PER_ORC",
		"ATTBONUS_PER_MILGYO",
		"ATTBONUS_PER_UNDEAD",
		"ATTBONUS_PER_DEVIL", // 250
		"ENCHANT_PER_ELECT",
		"ENCHANT_PER_FIRE",
		"ENCHANT_PER_ICE",
		"ENCHANT_PER_WIND",
		"ENCHANT_PER_EARTH",
		"ENCHANT_PER_DARK",
		"ATTBONUS_PER_CZ",
		"ATTBONUS_PER_INSECT",
		"ATTBONUS_PER_DESERT",
		"ATTBONUS_PER_STONE", // 260
		"ATTBONUS_PER_MONSTER",
		"RESIST_PER_HUMAN",
		"RESIST_PER_ICE",
		"RESIST_PER_DARK",
		"RESIST_PER_EARTH",
		"RESIST_PER_FIRE",
		"RESIST_PER_ELEC",
		"RESIST_PER_MAGIC",
		"RESIST_PER_WIND",
		"HIT_BUFF_SSTR", // 270
		"HIT_BUFF_SMOVE",
		"HIT_BUFF_SHP",
		"HIT_BUFF_SIMN",
		"MOUNT_MELEE_MAGIC_ATT_PER",
		"DISMOUNT_MOVE_SPEED_BONUS_PER",
		"HIT_AUTO_HP_REC",
		"HIT_AUTO_SP_REC",
		"USE_SK_COOL_DEC_ALL",
		"HIT_STONE_ATT",
		"HIT_STONE_DEF", // 280
		"KILL_BOSS_ITEM_BONUS",
		"MOB_HIT_MOB_AGGR",
		"NO_DEATH_AND_HP_REC30",
		"AUTO_PICKUP",
		"MOUNT_NO_KNOCKBACK",
		"SUNGMA_PER_STR",
		"SUNGMA_PER_HP",
		"SUNGMA_PER_MOVE",
		"SUNGMA_PER_IMN",
		"IMN_POISON100", // 290
		"IMN_BLEED100",
		"MONSTER_DEFEND_BONUS"
	};

	int retInt = -1;
	//cout << "ApplyType : " << applyTypeStr << " -> ";
	for (uint32_t j = 0; j < sizeof(arApplyType) / sizeof(arApplyType[0]); j++)
	{
		string tempString = arApplyType[j];
		string tempInputString = trim(inputString);
		if (tempInputString.compare(tempString) == 0)
		{
			//cout << j << " ";
			retInt = j;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

// Mob Proto also read.
int get_Mob_Rank_Value(string inputString)
{
	string arRank[] =
	{
		"PAWN",
		"S_PAWN",
		"KNIGHT",
		"S_KNIGHT",
		"BOSS",
		"KING"
	};

	int retInt = -1;
	//cout << "Rank : " << rankStr << " -> ";
	for (uint32_t j = 0; j < sizeof(arRank) / sizeof(arRank[0]); j++)
	{
		string tempString = arRank[j];
		string tempInputString = trim(inputString);
		if (tempInputString.compare(tempString) == 0)
		{
			//cout << j << " ";
			retInt = j;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

int get_Mob_Type_Value(string inputString)
{
	string arType[] =
	{
		"MONSTER",
		"NPC",
		"STONE",
		"WARP",
		"DOOR",
		"BUILDING",
		"PC",
		"POLYMORPH_PC",
		"HORSE",
		"GOTO",
		"PET",
		"PET_PAY",
		"SHOP", // ENABLE_PREMIUM_PRIVATE_SHOP
		"OBJECT"
	};

	int retInt = -1;
	//cout << "Type : " << typeStr << " -> ";
	for (uint32_t j = 0; j < sizeof(arType) / sizeof(arType[0]); j++)
	{
		string tempString = arType[j];
		string tempInputString = trim(inputString);
		if (tempInputString.compare(tempString) == 0)
		{
			//cout << j << " ";
			retInt = j;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

int get_Mob_BattleType_Value(string inputString)
{
	string arBattleType[] =
	{
		"MELEE",
		"RANGE",
		"MAGIC",
		"SPECIAL",
		"POWER",
		"TANKER",
		"SUPER_POWER",
		"SUPER_TANKER"
	};

	int retInt = -1;
	//cout << "Battle Type : " << battleTypeStr << " -> ";
	for (uint32_t j = 0; j < sizeof(arBattleType) / sizeof(arBattleType[0]); j++)
	{
		string tempString = arBattleType[j];
		string tempInputString = trim(inputString);
		if (tempInputString.compare(tempString) == 0)
		{
			//cout << j << " ";
			retInt = j;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

int get_Mob_Size_Value(string inputString)
{
	string arSize[] =
	{
		"SMALL", //@fixme201 SAMLL to SMALL
		"MEDIUM",
		"BIG"
	};

	int retInt = 0;
	//cout << "Size : " << sizeStr << " -> ";
	for (uint32_t j = 0; j < sizeof(arSize) / sizeof(arSize[0]); j++)
	{
		string tempString = arSize[j];
		string tempInputString = trim(inputString);
		if (tempInputString.compare(tempString) == 0)
		{
			//cout << j << " ";
			retInt = j + 1;
			break;
		}
	}
	//cout << endl;

	return retInt;
}

int get_Mob_AIFlag_Value(string inputString)
{
	string arAIFlag[] =
	{
		"AGGR",
		"NOMOVE",
		"COWARD",
		"NOATTSHINSU",
		"NOATTCHUNJO",
		"NOATTJINNO",
		"ATTMOB",
		"BERSERK",
		"STONESKIN",
		"GODSPEED",
		"DEATHBLOW",
		"REVIVE",
		"HEALER",	//ENABLE_DAWNMIST_DUNGEON
		"COUNT",
		"NORECOVERY",
		// Zodiac
		"REFLECT",
		"FALL",
		"VIT",
		"RATTSPEED",
		"RCASTSPEED",
		"RHP_REGEN",
		"TIMEVIT",

		// Unknown Types
		"UNK_AI_FLAG22",
		"UNK_AI_FLAG23",
		"UNK_AI_FLAG24",
		"UNK_AI_FLAG25",
		"ELEMENT_BUFF_NONE",
		"ELEMENT_BUFF_FIRE",
		"ELEMENT_BUFF_ICE ",
		"ELEMENT_BUFF_ELECT",
		"ELEMENT_BUFF_WIND",
		"ELEMENT_BUFF_EARTH",
	};

	int retValue = 0;
	string* arInputString = StringSplit(inputString, ","); // Arrange the contents of the proto information into words.
	for (uint32_t i = 0; i < sizeof(arAIFlag) / sizeof(arAIFlag[0]); i++)
	{
		string tempString = arAIFlag[i];
		for (uint32_t j = 0; j < 32; j++) // Up to 30 words. (Hard coding)
		{
			string tempString2 = arInputString[j];
			if (tempString2.compare(tempString) == 0) // Make sure it matches.
			{
				retValue = retValue + static_cast<int>(pow(static_cast<float>(2), static_cast<float>(i)));
			}

			if (tempString2.compare("") == 0)
				break;
		}
	}
	delete[]arInputString;
	//cout << "AIFlag : " << aiFlagStr << " -> " << retValue << endl;

	return retValue;
}

int get_Mob_AIFlagEx_Value(string inputString)
{
	string arAIFlagEx[] =
	{
		"ELEMENT_BUFF_DARK",
	};

	int retValue = 0;
	string* arInputString = StringSplit(inputString, ","); // Arrange the contents of the proto information into words.
	for (uint32_t i = 0; i < sizeof(arAIFlagEx) / sizeof(arAIFlagEx[0]); i++)
	{
		string tempString = arAIFlagEx[i];
		for (uint32_t j = 0; j < 30; j++) // Up to 30 words. (Hard coding)
		{
			string tempString2 = arInputString[j];
			if (tempString2.compare(tempString) == 0) // Make sure it matches.
			{
				retValue = retValue + static_cast<int>(pow(static_cast<float>(2), static_cast<float>(i)));
			}

			if (tempString2.compare("") == 0)
				break;
		}
	}
	delete[]arInputString;
	//cout << "UnkFlag : " << unkFlagStr << " -> " << retValue << endl;

	return retValue;
}

int get_Mob_RaceFlag_Value(string inputString)
{
	string arRaceFlag[] =
	{
		"ANIMAL",
		"UNDEAD",
		"DEVIL",
		"HUMAN",
		"ORC",
		"MILGYO",
		"INSECT",
		"DESERT",
		"TREE",
		"DECO",
		"HIDE",
		"ATT_CZ",	//ENABLE_ELEMENT_ADD | ENABLE_12ZI
		"AWEAKEN",
		"SUNGMAHEE",
		"OUTPOST",
		"UNKNOWN_RACE_FLAG1",
		"UNKNOWN_RACE_FLAG2",
	};

	int retValue = 0;
	string* arInputString = StringSplit(inputString, ","); // Arrange the contents of the proto information into words.
	for (uint32_t i = 0; i < sizeof(arRaceFlag) / sizeof(arRaceFlag[0]); i++)
	{
		string tempString = arRaceFlag[i];
		for (uint32_t j = 0; j < 30; j++) // Up to 30 words. (Hard coding)
		{
			string tempString2 = arInputString[j];
			if (tempString2.compare(tempString) == 0) // Make sure it matches.
			{
				retValue = retValue + static_cast<int>(pow(static_cast<float>(2), static_cast<float>(i)));
			}

			if (tempString2.compare("") == 0)
				break;
		}
	}
	delete[]arInputString;
	//cout << "Race Flag : " << raceFlagStr << " -> " << retValue << endl;

	return retValue;
}

int get_Mob_ImmuneFlag_Value(string inputString)
{
	string arImmuneFlag[] =
	{
		"STUN",
		"SLOW",
		"FALL",
		"CURSE",
		"POISON",
		"TERROR",
		"REFLECT",
		"UNK_IMMUNE_FLAG",
		"UNK_IMMUNE_FLAG1",
		"UNK_IMMUNE_FLAG2",
		"UNK_IMMUNE_FLAG3",
		"UNK_IMMUNE_FLAG4",
		"UNK_IMMUNE_FLAG5",
		"UNK_IMMUNE_FLAG6",
		"UNK_IMMUNE_FLAG7",
		"UNK_IMMUNE_FLAG8"
	};

	int retValue = 0;
	string* arInputString = StringSplit(inputString, ","); // Arrange the contents of the proto information into words.
	for (uint32_t i = 0; i < sizeof(arImmuneFlag) / sizeof(arImmuneFlag[0]); i++)
	{
		string tempString = arImmuneFlag[i];
		for (uint32_t j = 0; j < 30; j++) // Up to 30 words. (Hard coding)
		{
			string tempString2 = arInputString[j];
			if (tempString2.compare(tempString) == 0) // Make sure it matches.
			{
				retValue = retValue + static_cast<int>(pow(static_cast<float>(2), static_cast<float>(i)));
			}

			if (tempString2.compare("") == 0)
				break;
		}
	}
	delete[]arInputString;
	//cout << "Immune Flag : " << immuneFlagStr << " -> " << retValue << endl;

	return retValue;
}

#ifndef __DUMP_PROTO__
// Set the mob table.
bool Set_Proto_Mob_Table(TMobTable* mobTable, const cCsvTable& csvTable, std::map<int, const char*>& nameMap)
{
	int col = 0;
	str_to_number(mobTable->dwVnum, csvTable.AsStringByIndex(col++));
	strlcpy(mobTable->szName, csvTable.AsStringByIndex(col++), sizeof(mobTable->szName));

	//3. Give your region a name.
	map<int, const char*>::iterator it;
	it = nameMap.find(mobTable->dwVnum);
	if (it != nameMap.end())
	{
		const char* localeName = it->second;
		strlcpy(mobTable->szLocaleName, localeName, sizeof(mobTable->szLocaleName));
	}
	else
	{
		strlcpy(mobTable->szLocaleName, mobTable->szName, sizeof(mobTable->szLocaleName));
	}

	//RANK
	const int rankValue = get_Mob_Rank_Value(csvTable.AsStringByIndex(col++));
	mobTable->bRank = rankValue;

	//TYPE
	const int typeValue = get_Mob_Type_Value(csvTable.AsStringByIndex(col++));
	mobTable->bType = typeValue;

	//BATTLE_TYPE
	const int battleTypeValue = get_Mob_BattleType_Value(csvTable.AsStringByIndex(col++));
	mobTable->bBattleType = battleTypeValue;

	//LEVEL
	str_to_number(mobTable->bLevel, csvTable.AsStringByIndex(col++));

#ifdef ENABLE_MOB_SCALE
	// SCALE
	col++;
#endif

	//SIZE
	const int sizeValue = get_Mob_Size_Value(csvTable.AsStringByIndex(col++));
	mobTable->bSize = sizeValue;

	//AI_FLAG
	const int aiFlagValue = get_Mob_AIFlag_Value(csvTable.AsStringByIndex(col++));
	mobTable->dwAIFlag = aiFlagValue;

	const int unkFlagValue = get_Mob_AIFlagEx_Value(csvTable.AsStringByIndex(col++));
	mobTable->dwAIFlagEx = unkFlagValue;

	//mount_capacity;
	col++;

	//RACE_FLAG
	const int raceFlagValue = get_Mob_RaceFlag_Value(csvTable.AsStringByIndex(col++));
	mobTable->dwRaceFlag = raceFlagValue;

	//IMMUNE_FLAG
	const int immuneFlagValue = get_Mob_ImmuneFlag_Value(csvTable.AsStringByIndex(col++));
	mobTable->dwImmuneFlag = immuneFlagValue;

	str_to_number(mobTable->bEmpire, csvTable.AsStringByIndex(col++)); //col = 12

	strlcpy(mobTable->szFolder, csvTable.AsStringByIndex(col++), sizeof(mobTable->szFolder));

	str_to_number(mobTable->bOnClickType, csvTable.AsStringByIndex(col++));

	str_to_number(mobTable->bStr, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bDex, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bCon, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bInt, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bSungMaSt, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bSungMaDx, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bSungMaHt, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bSungMaIq, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwDamageRange[0], csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwDamageRange[1], csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwMaxHP, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bRegenCycle, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bRegenPercent, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwGoldMin, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwGoldMax, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwExp, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwSungMaExp, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->wDef, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->sAttackSpeed, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->sMovingSpeed, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bAggresiveHPPct, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->wAggressiveSight, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->wAttackRange, csvTable.AsStringByIndex(col++));

	str_to_number(mobTable->dwDropItemVnum, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwResurrectionVnum, csvTable.AsStringByIndex(col++));

	for (int i = 0; i < MOB_ENCHANTS_MAX_NUM; ++i)
		str_to_number(mobTable->cEnchants[i], csvTable.AsStringByIndex(col++));

	for (int i = 0; i < MOB_RESISTS_MAX_NUM; ++i)
		str_to_number(mobTable->cResists[i], csvTable.AsStringByIndex(col++));

	for (int i = 0; i < MOB_ELEMENTAL_MAX_NUM; ++i)
		str_to_number(mobTable->cElementalFlags[i], csvTable.AsStringByIndex(col++));

	str_to_number(mobTable->cResistDark, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->cResistIce, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->cResistEarth, csvTable.AsStringByIndex(col++));

	str_to_number(mobTable->fDamMultiply, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwSummonVnum, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->dwDrainSP, csvTable.AsStringByIndex(col++));

	//Mob_Color
	++col;

	str_to_number(mobTable->dwPolymorphItemVnum, csvTable.AsStringByIndex(col++));

	for (int i = 0; i < MOB_SKILL_MAX_NUM; ++i)
	{
		str_to_number(mobTable->Skills[i].bLevel, csvTable.AsStringByIndex(col++));
		str_to_number(mobTable->Skills[i].dwVnum, csvTable.AsStringByIndex(col++));
	}

	str_to_number(mobTable->bBerserkPoint, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bStoneSkinPoint, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bGodSpeedPoint, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bDeathBlowPoint, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bRevivePoint, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bHealPoint, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bRAttSpeedPoint, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bRCastSpeedPoint, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->bRHPRegenPoint, csvTable.AsStringByIndex(col++));
	str_to_number(mobTable->fHitRange, csvTable.AsStringByIndex(col++));

	sys_log(0, "MOB #%-5d %-24s level: %-3u rank: %u empire: %d", mobTable->dwVnum, mobTable->szLocaleName, mobTable->bLevel, mobTable->bRank, mobTable->bEmpire);

	return true;
}

bool Set_Proto_Item_Table(TItemTable* itemTable, const cCsvTable& csvTable, std::map<int, const char*>& nameMap)
{
	// Reading vnum and vnum range.
	{
		std::string s(csvTable.AsStringByIndex(0));
		const size_t pos = s.find("~");
		// If there is no'~' in the vnum field, pass
		if (std::string::npos == pos)
		{
			itemTable->dwVnum = atoi(s.c_str());
			if (0 == itemTable->dwVnum)
			{
				printf("INVALID VNUM %s\n", s.c_str());
				return false;
			}
			itemTable->dwVnumRange = 0;
		}
		else
		{
			std::string s_start_vnum(s.substr(0, pos));
			std::string s_end_vnum(s.substr(pos + 1));

			const int start_vnum = atoi(s_start_vnum.c_str());
			const int end_vnum = atoi(s_end_vnum.c_str());
			if (0 == start_vnum || (0 != end_vnum && end_vnum < start_vnum))
			{
				printf("INVALID VNUM RANGE%s\n", s.c_str());
				return false;
			}
			itemTable->dwVnum = start_vnum;
			itemTable->dwVnumRange = end_vnum - start_vnum;
		}
	}

	int col = 1;

	strncpy(itemTable->szName, csvTable.AsStringByIndex(col++), ITEM_NAME_MAX_LEN);
	// If the name file exists, information is read.
	std::map<int, const char*>::iterator it;
	it = nameMap.find(itemTable->dwVnum);
	if (it != nameMap.end())
	{
		const char* localeName = it->second;
		strncpy(&itemTable->szLocaleName[0], localeName, sizeof(itemTable->szLocaleName));
	}
	else //If the name file does not exist, in Korean...
	{
		strncpy(&itemTable->szLocaleName[0], itemTable->szName, sizeof(itemTable->szLocaleName));
	}

	itemTable->bType = get_Item_Type_Value(csvTable.AsStringByIndex(col++));
	itemTable->bSubType = get_Item_SubType_Value(itemTable->bType, csvTable.AsStringByIndex(col++));
	itemTable->bSize = atoi(csvTable.AsStringByIndex(col++));
	itemTable->dwAntiFlags = get_Item_AntiFlag_Value(csvTable.AsStringByIndex(col++));
	itemTable->dwFlags = get_Item_Flag_Value(csvTable.AsStringByIndex(col++));
	itemTable->dwWearFlags = get_Item_WearFlag_Value(csvTable.AsStringByIndex(col++));
	itemTable->dwImmuneFlag = get_Item_Immune_Value(csvTable.AsStringByIndex(col++));
	itemTable->dwShopBuyPrice = atoi(csvTable.AsStringByIndex(col++));
	itemTable->dwShopSellPrice = atoi(csvTable.AsStringByIndex(col++));
	itemTable->dwRefineElementApplyType = atoi(csvTable.AsStringByIndex(col++));
	itemTable->dwRefineElementGrade = atoi(csvTable.AsStringByIndex(col++));
	itemTable->dwRefineElementValue = atoi(csvTable.AsStringByIndex(col++));
	itemTable->dwRefineElementBonus = atoi(csvTable.AsStringByIndex(col++));
	itemTable->dwRefinedVnum = atoi(csvTable.AsStringByIndex(col++));
	itemTable->wRefineSet = atoi(csvTable.AsStringByIndex(col++));
	itemTable->dwMaterial67 = atoi(csvTable.AsStringByIndex(col++));
	itemTable->bAlterToMagicItemPct = atoi(csvTable.AsStringByIndex(col++));

	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		itemTable->aLimits[i].bType = get_Item_LimitType_Value(csvTable.AsStringByIndex(col++));
		itemTable->aLimits[i].lValue = atoi(csvTable.AsStringByIndex(col++));
	}

	for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
	{
		itemTable->aApplies[i].wType = get_Item_ApplyType_Value(csvTable.AsStringByIndex(col++));
		itemTable->aApplies[i].lValue = atoi(csvTable.AsStringByIndex(col++));
	}

	for (int i = 0; i < ITEM_VALUES_MAX_NUM; ++i)
		itemTable->alValues[i] = atoi(csvTable.AsStringByIndex(col++));

	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		itemTable->alSockets[i] = atoi(csvTable.AsStringByIndex(col++));

	itemTable->bSpecular = atoi(csvTable.AsStringByIndex(col++));
	itemTable->bGainSocketPct = atoi(csvTable.AsStringByIndex(col++));
	itemTable->bMaskType = get_Mask_Type_Value(csvTable.AsStringByIndex(col++));
	itemTable->bMaskSubType = get_Mask_SubType_Value(itemTable->bMaskType, csvTable.AsStringByIndex(col++));
	col++; //AddonType

	itemTable->bWeight = 0;

	sys_log(0, "ITEM #%-5d %-24s", itemTable->dwVnum, itemTable->szLocaleName);

	return true;
}
#endif
