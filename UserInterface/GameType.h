#pragma once
#include "../GameLib/ItemData.h"

struct SAffects
{
	enum
	{
		AFFECT_MAX_NUM = 32
	};

	SAffects() = default;

	SAffects(const uint32_t& c_rAffects) { __SetAffects(c_rAffects); }
	int operator=(const uint32_t& c_rAffects) { __SetAffects(c_rAffects); }

	BOOL IsAffect(uint8_t byIndex) const { return dwAffects & (1 << byIndex); }

	void __SetAffects(const uint32_t& c_rAffects) { dwAffects = c_rAffects; }

	uint32_t dwAffects{ 0 };
};

extern std::string g_strGuildSymbolPathName;

constexpr uint32_t c_Name_Max_Length = 64;
constexpr uint32_t c_FileName_Max_Length = 128;
constexpr uint32_t c_Short_Name_Max_Length = 32;

constexpr uint32_t c_Inventory_Page_Column = 5;
constexpr uint32_t c_Inventory_Page_Row = 9;
constexpr uint32_t c_Inventory_Page_Size = c_Inventory_Page_Column * c_Inventory_Page_Row; // x*y
#if defined(ENABLE_EXTEND_INVEN_SYSTEM) || defined(ENABLE_EXTEND_INVEN_PAGE_SYSTEM)
constexpr uint32_t c_Inventory_Page_Count = 4;
#else
constexpr uint32_t c_Inventory_Page_Count = 2;
#endif
constexpr uint32_t c_Equipment_Count = 12;
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
const int ROULETTE_ITEM_MAX = 20;
#endif

constexpr uint32_t c_Equipment_Start = 0;
constexpr uint32_t c_Equipment_Body = c_Equipment_Start + CItemData::WEAR_BODY;
constexpr uint32_t c_Equipment_Head = c_Equipment_Start + CItemData::WEAR_HEAD;
constexpr uint32_t c_Equipment_Shoes = c_Equipment_Start + CItemData::WEAR_FOOTS;
constexpr uint32_t c_Equipment_Wrist = c_Equipment_Start + CItemData::WEAR_WRIST;
constexpr uint32_t c_Equipment_Weapon = c_Equipment_Start + CItemData::WEAR_WEAPON;
constexpr uint32_t c_Equipment_Neck = c_Equipment_Start + CItemData::WEAR_NECK;
constexpr uint32_t c_Equipment_Ear = c_Equipment_Start + CItemData::WEAR_EAR;
constexpr uint32_t c_Equipment_Unique1 = c_Equipment_Start + CItemData::WEAR_UNIQUE1;
constexpr uint32_t c_Equipment_Unique2 = c_Equipment_Start + CItemData::WEAR_UNIQUE2;
constexpr uint32_t c_Equipment_Arrow = c_Equipment_Start + CItemData::WEAR_ARROW;
constexpr uint32_t c_Equipment_Shield = c_Equipment_Start + CItemData::WEAR_SHIELD;

// New rings & belts added
// Locations that can be assigned to mounted items are cut off due to existing equipment, quest rewards, and costume systems.
// This is all upset because of the implementation of the buff compensation buff like an equipped item... ¤¶¤²
//
// In summary, the existing equipment windows have an item position of 90 to 102 in the server DB,
// New slots added in early 2013 start from 111. The maximum value that can be used in wearing equipment is up to 121, and from 122 it is used in Dragon Spirit Stone.
//ENABLE_NEW_EQUIPMENT_SYSTEM
constexpr uint32_t c_New_Equipment_Start = c_Equipment_Start + CItemData::WEAR_RING1;
constexpr uint32_t c_Equipment_Ring1 = c_Equipment_Start + CItemData::WEAR_RING1;
constexpr uint32_t c_Equipment_Ring2 = c_Equipment_Start + CItemData::WEAR_RING2;
constexpr uint32_t c_Equipment_Belt = c_Equipment_Start + CItemData::WEAR_BELT;
constexpr uint32_t c_Equipment_Pendant = c_Equipment_Start + CItemData::WEAR_PENDANT;
constexpr uint32_t c_Equipment_Glove = c_Equipment_Start + CItemData::WEAR_GLOVE;
#ifdef ENABLE_PET_SYSTEM
constexpr uint32_t c_Equipment_Standard_Pet = c_Equipment_Start + CItemData::WEAR_PET;
#endif
constexpr uint32_t c_New_Equipment_Count = CItemData::WEAR_MAX_NUM - CItemData::WEAR_RING1;
//END_ENABLE_NEW_EQUIPMENT_SYSTEM

#ifdef ENABLE_ATTR_6TH_7TH
const uint8_t c_SkillBook_Comb_Slot_Max = 10;
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
enum ESkillColorLength
{
#	ifdef ENABLE_NINETH_SKILL
	MAX_SKILL_COUNT = 9,
	MAX_NORMAL_SKILL_COUNT = 6,
#	else
	MAX_SKILL_COUNT = 6,
#	endif
	MAX_EFFECT_COUNT = 5,
#	if defined(ENABLE_WOLFMAN_CHARACTER) && defined(ENABLE_NINETH_SKILL)
	MAX_BUFF_COUNT = 7,
#	elif defined(ENABLE_WOLFMAN_CHARACTER) && !defined(ENABLE_NINETH_SKILL) || !defined(ENABLE_WOLFMAN_CHARACTER) && defined(ENABLE_NINETH_SKILL)
	MAX_BUFF_COUNT = 6,
#	else
	MAX_BUFF_COUNT = 5,
#	endif
	BUFF_BEGIN = MAX_SKILL_COUNT,
};
#endif

enum EDragonSoulDeckType
{
	DS_DECK_1,
	DS_DECK_2,
	DS_DECK_MAX_NUM = 2
};

enum EDragonSoulGradeTypes
{
	DRAGON_SOUL_GRADE_NORMAL,
	DRAGON_SOUL_GRADE_BRILLIANT,
	DRAGON_SOUL_GRADE_RARE,
	DRAGON_SOUL_GRADE_ANCIENT,
	DRAGON_SOUL_GRADE_LEGENDARY,
	DRAGON_SOUL_GRADE_MYTH,	//ENABLE_DS_GRADE_MYTH
	DRAGON_SOUL_GRADE_MAX,
};

enum EDragonSoulStepTypes
{
	DRAGON_SOUL_STEP_LOWEST,
	DRAGON_SOUL_STEP_LOW,
	DRAGON_SOUL_STEP_MID,
	DRAGON_SOUL_STEP_HIGH,
	DRAGON_SOUL_STEP_HIGHEST,
	DRAGON_SOUL_STEP_MAX
};

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE 
enum AdditionalEquipmentConfig
{
	ADDITIONAL_EQUIPMENT_MAX_SLOT = CItemData::WEAR_MAX_NUM,
};
#endif

constexpr uint32_t c_Costume_Slot_Start = c_Equipment_Start + CItemData::WEAR_COSTUME_BODY;
constexpr uint32_t	c_Costume_Slot_Body = c_Costume_Slot_Start + CItemData::COSTUME_BODY;
constexpr uint32_t	c_Costume_Slot_Hair = c_Costume_Slot_Start + CItemData::COSTUME_HAIR;
constexpr uint32_t	c_Costume_Slot_Mount = c_Costume_Slot_Start + CItemData::COSTUME_MOUNT;
constexpr uint32_t	c_Costume_Slot_Acce = c_Costume_Slot_Start + CItemData::COSTUME_ACCE;
constexpr uint32_t	c_Costume_Slot_Weapon = c_Costume_Slot_Start + CItemData::COSTUME_WEAPON;
constexpr uint32_t	c_Costume_Slot_Aura = c_Costume_Slot_Start + CItemData::COSTUME_AURA;	//ENABLE_AURA_SYSTEM
constexpr uint32_t c_Costume_Slot_Count = CItemData::COSTUME_NUM_TYPES;
constexpr uint32_t c_Costume_Slot_End = c_Costume_Slot_Start + c_Costume_Slot_Count;

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE 
constexpr uint32_t c_Additional_Equip_Slot_Start = c_New_Equipment_Start + c_New_Equipment_Count;
constexpr uint32_t c_Additional_Equip_Slot_End = c_Additional_Equip_Slot_Start + ADDITIONAL_EQUIPMENT_MAX_SLOT;
#endif

// [Caution] Beware of hard coding number (32). Dragon Soul Stone slots in the current server start at 32.
// Keep in mind that the EWearPositions enumeration in the server common/length.h file will be extended to 32 (more than 32 will be difficult to extend),
// After that, the dragon spirit stone mounting slot is used.
constexpr uint32_t c_Wear_Max = CItemData::WEAR_MAX_NUM;
constexpr uint32_t c_DragonSoul_Equip_Start = c_Wear_Max;
constexpr uint32_t c_DragonSoul_Equip_Slot_Max = 6;
constexpr uint32_t c_DragonSoul_Equip_End = c_DragonSoul_Equip_Start + c_DragonSoul_Equip_Slot_Max * DS_DECK_MAX_NUM;
constexpr uint32_t c_Equipment_Slot_Count = c_Wear_Max + c_DragonSoul_Equip_Slot_Max * DS_DECK_MAX_NUM;

//ENABLE_NEW_EQUIPMENT_SYSTEM
// Inventory provided by belt items
enum EBeltInventorySize
{
#ifdef ENABLE_BELT_INVENTORY_RENEWAL
	BELT_INVENTORY_SLOT_WIDTH = 6,
	BELT_INVENTORY_SLOT_HEIGHT = 6,
#else
	BELT_INVENTORY_WIDTH = 4,
	BELT_INVENTORY_HEIGHT = 4,
#endif
};

constexpr uint32_t c_Belt_Inventory_Slot_Start = 0;
constexpr uint32_t c_Belt_Inventory_Width = BELT_INVENTORY_SLOT_WIDTH;
constexpr uint32_t c_Belt_Inventory_Height = BELT_INVENTORY_SLOT_HEIGHT;
constexpr uint32_t c_Belt_Inventory_Slot_Count = c_Belt_Inventory_Width * c_Belt_Inventory_Height;
constexpr uint32_t c_Belt_Inventory_Slot_End = c_Belt_Inventory_Slot_Start + c_Belt_Inventory_Slot_Count;
//END_ENABLE_NEW_EQUIPMENT_SYSTEM

#ifdef ENABLE_SPECIAL_INVENTORY
enum ESpecialInventoryTypes
{
	INVENTORY_TYPE_INVENTORY,
	INVENTORY_TYPE_SKILLBOOK,
	INVENTORY_TYPE_STONE,
	INVENTORY_TYPE_MATERIAL,
	INVENTORY_TYPE_COUNT,
};

enum ESpecialInventorySize
{
	SPECIAL_INVENTORY_WIDTH = 5,
	SPECIAL_INVENTORY_HEIGHT = 9,
	SPECIAL_INVENTORY_PAGE_SIZE = SPECIAL_INVENTORY_WIDTH * SPECIAL_INVENTORY_HEIGHT,
};

constexpr uint32_t c_Special_Inventory_Slot_Start = c_Inventory_Page_Size * c_Inventory_Page_Count;
constexpr uint32_t c_Special_Inventory_Skillbook_Slot_Start = c_Special_Inventory_Slot_Start;
constexpr uint32_t c_Special_Inventory_Skillbook_Slot_End = c_Special_Inventory_Skillbook_Slot_Start + SPECIAL_INVENTORY_PAGE_SIZE * c_Inventory_Page_Count;

constexpr uint32_t c_Special_Inventory_Stone_Slot_Start = c_Special_Inventory_Skillbook_Slot_End;
constexpr uint32_t c_Special_Inventory_Stone_Slot_End = c_Special_Inventory_Stone_Slot_Start + SPECIAL_INVENTORY_PAGE_SIZE * c_Inventory_Page_Count;

constexpr uint32_t c_Special_Inventory_Material_Slot_Start = c_Special_Inventory_Stone_Slot_End;
constexpr uint32_t c_Special_Inventory_Material_Slot_End = c_Special_Inventory_Material_Slot_Start + SPECIAL_INVENTORY_PAGE_SIZE * c_Inventory_Page_Count;

constexpr uint32_t c_Special_Inventory_Slot_End = c_Special_Inventory_Material_Slot_End;

constexpr uint32_t c_Inventory_Slot_Count = c_Special_Inventory_Slot_End;
#else
constexpr uint32_t c_Inventory_Slot_Count = c_Inventory_Page_Size * c_Inventory_Page_Count;
#endif

// Dragon Spirit-only inventory
constexpr uint32_t c_DragonSoul_Inventory_Start = 0;
constexpr uint32_t c_DragonSoul_Inventory_Box_Size = 32;
constexpr uint32_t c_DragonSoul_Inventory_Count = CItemData::DS_SLOT_NUM_TYPES * DRAGON_SOUL_GRADE_MAX * c_DragonSoul_Inventory_Box_Size;
constexpr uint32_t c_DragonSoul_Inventory_End = c_DragonSoul_Inventory_Start + c_DragonSoul_Inventory_Count;

enum ESlotType
{
	SLOT_TYPE_NONE,
	SLOT_TYPE_INVENTORY,
	SLOT_TYPE_SKILL,
	SLOT_TYPE_EMOTION,
	SLOT_TYPE_SHOP,
	SLOT_TYPE_EXCHANGE_OWNER,
	SLOT_TYPE_EXCHANGE_TARGET,
	SLOT_TYPE_QUICK_SLOT,
	SLOT_TYPE_SAFEBOX,
	SLOT_TYPE_GUILDBANK,			//ENABLE_GUILDBANK
	SLOT_TYPE_ACCE,					//ENABLE_ACCE_COSTUME_SYSTEM
	SLOT_TYPE_PRIVATE_SHOP,
	SLOT_TYPE_MALL,
	SLOT_TYPE_DRAGON_SOUL_INVENTORY,
	SLOT_TYPE_PET_FEED_WINDOW,		//ENABLE_GROWTH_PET_SYSTEM
	SLOT_TYPE_EQUIPMENT,
	SLOT_TYPE_BELT_INVENTORY,
	SLOT_TYPE_AUTO,					//ENABLE_AUTO_SYSTEM
	SLOT_TYPE_CHANGE_LOOK,			//ENABLE_CHANGE_LOOK_SYSTEM
	SLOT_TYPE_FISH_EVENT,			//ENABLE_FISH_EVENT
	SLOT_TYPE_AURA,					//ENABLE_AURA_SYSTEM
	SLOT_TYPE_PREMIUM_PRIVATE_SHOP,	//ENABLE_PREMIUM_PRIVATE_SHOP
#ifdef ENABLE_SWITCHBOT
	SLOT_TYPE_SWITCHBOT,
#endif
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	SLOT_TYPE_ADDITIONAL_EQUIPMENT_1,
#endif

	SLOT_TYPE_MAX
};

enum EWindows
{
	RESERVED_WINDOW,
	INVENTORY,				//Basic inventory. (2 pages of 45 spaces = 90 spaces)
	EQUIPMENT,
	SAFEBOX,
	MALL,
	DRAGON_SOUL_INVENTORY,
	BELT_INVENTORY,			//NOTE: Belt inventory provided by the new belt slot item in W2.1 version
	GUILDBANK,				//ENABLE_GUILDBANK
	MAIL,					//ENABLE_MAILBOX
	NPC_STORAGE,			//ENABLE_ATTR_6TH_7TH
	PREMIUM_PRIVATE_SHOP,	//ENABLE_PREMIUM_PRIVATE_SHOP
	ACCEREFINE,				//ENABLE_ACCE_COSTUME_SYSTEM
	GROUND,					//NOTE: As of February 5, 2013 unused.. Why is it there???
	PET_FEED,				//ENABLE_GROWTH_PET_SYSTEM
	CHANGELOOK,				//ENABLE_CHANGE_LOOK_SYSTEM
	AURA_REFINE,			//ENABLE_AURA_SYSTEM
#ifdef ENABLE_SWITCHBOT
	SWITCHBOT,
#endif
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	ADDITIONAL_EQUIPMENT_1,
#endif

	WINDOW_TYPE_MAX
};

#ifdef ENABLE_FISH_EVENT
enum EFishEventInfo
{
	FISH_EVENT_SHAPE_NONE,
	FISH_EVENT_SHAPE_1,
	FISH_EVENT_SHAPE_2,
	FISH_EVENT_SHAPE_3,
	FISH_EVENT_SHAPE_4,
	FISH_EVENT_SHAPE_5,
	FISH_EVENT_SHAPE_6,
	FISH_EVENT_SHAPE_7,
	FISH_EVENT_SHAPE_MAX_NUM,
};
#endif

#ifdef ENABLE_BIOLOG_SYSTEM
enum EBiologBonuses
{
	MAX_BONUSES_LENGTH = 5,
};
#endif

enum ECashMaxTypes
{
	GOLD_MAX = 2000000000,
	CHEQUE_MAX = 999,
	GAYA_MAX = 999999,
};

enum EDSInventoryMaxNum
{
	DS_INVENTORY_MAX_NUM = c_DragonSoul_Inventory_Count,
	DS_REFINE_WINDOW_MAX_NUM = 15
};

#ifdef ENABLE_SWITCHBOT
enum ESwitchbotValues
{
	SWITCHBOT_SLOT_COUNT = 7,
	SWITCHBOT_ALTERNATIVE_COUNT = 5,
	MAX_NORM_ATTR_NUM = 5,
};

enum EAttributeSet
{
	ATTRIBUTE_SET_WEAPON,
	ATTRIBUTE_SET_BODY,
	ATTRIBUTE_SET_WRIST,
	ATTRIBUTE_SET_FOOTS,
	ATTRIBUTE_SET_NECK,
	ATTRIBUTE_SET_HEAD,
	ATTRIBUTE_SET_SHIELD,
	ATTRIBUTE_SET_EAR,
#	ifdef ENABLE_PENDANT
	ATTRIBUTE_SET_PENDANT,
#	endif
#	ifdef ENABLE_GLOVE_SYSTEM
	ATTRIBUTE_SET_GLOVE,
#	endif
#	ifdef ENABLE_COSTUME_SYSTEM
	ATTRIBUTE_SET_COSTUME_BODY,
	ATTRIBUTE_SET_COSTUME_HAIR,
#	if defined(ENABLE_WEAPON_COSTUME_SYSTEM)
	ATTRIBUTE_SET_COSTUME_WEAPON,
#	endif
#endif
	ATTRIBUTE_SET_MAX_NUM,
};
#endif

#pragma pack(push, 1)
#define WORD_MAX 0xffff

typedef struct SItemPos
{
	uint8_t window_type;
	uint16_t cell;
	SItemPos()
	{
		window_type = INVENTORY;
		cell = WORD_MAX;
	}
	SItemPos(uint8_t _window_type, uint16_t _cell)
	{
		window_type = _window_type;
		cell = _cell;
	}

	// In the previous cell type, uint8_t is mostly, but oi
	// Some parts are int, some parts are uint16_t,
	// It is received as the largest type, int
	//  int operator=(const int _cell)
	//  {
	//		window_type = INVENTORY;
	//      cell = _cell;
	//      return cell;
	//  }
	bool IsValidCell() const
	{
		switch (window_type)
		{
			case INVENTORY:
				return cell < c_Inventory_Slot_Count;

			case EQUIPMENT:
				return cell < c_Equipment_Slot_Count;

			case BELT_INVENTORY:
				return cell < c_Belt_Inventory_Slot_End;

			case DRAGON_SOUL_INVENTORY:
				return cell < (DS_INVENTORY_MAX_NUM);

#ifdef ENABLE_ATTR_6TH_7TH
			case NPC_STORAGE:
				return cell < 1;
#endif

#ifdef ENABLE_SWITCHBOT
			case SWITCHBOT:
				return cell < (SWITCHBOT_SLOT_COUNT);
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
			case ADDITIONAL_EQUIPMENT_1:
				return cell < (ADDITIONAL_EQUIPMENT_MAX_SLOT);
#endif

			default:
				return false;
		}
	}
	bool IsEquipCell() const
	{
		switch (window_type)
		{
			case EQUIPMENT:
				return (c_Equipment_Start + c_Wear_Max > cell) && (c_Equipment_Start <= cell);

			case INVENTORY:
			case BELT_INVENTORY:
			case DRAGON_SOUL_INVENTORY:
				return false;

			default:
				return false;
		}
	}

	//ENABLE_NEW_EQUIPMENT_SYSTEM
	bool IsBeltInventoryCell() const
	{
		const bool bResult = c_Belt_Inventory_Slot_Start <= cell && c_Belt_Inventory_Slot_End > cell;
		return bResult;
	}
	//END_ENABLE_NEW_EQUIPMENT_SYSTEM

	bool IsNPOS()
	{
		return (window_type == RESERVED_WINDOW && cell == WORD_MAX);
	}

	bool operator==(const struct SItemPos& rhs) const
	{
		return (window_type == rhs.window_type) && (cell == rhs.cell);
	}

	bool operator!=(const struct SItemPos& rhs) const
	{
		return (window_type != rhs.window_type) || (cell != rhs.cell);
	}

	bool operator<(const struct SItemPos& rhs) const
	{
		return (window_type < rhs.window_type) || ((window_type == rhs.window_type) && (cell < rhs.cell));
	}
} TItemPos;
const TItemPos NPOS(RESERVED_WINDOW, WORD_MAX);
#pragma pack(pop)

constexpr uint32_t c_QuickBar_Line_Count = 3;
constexpr uint32_t c_QuickBar_Slot_Count = 12;

constexpr float c_Idle_WaitTime = 5.0f;

constexpr int c_Monster_Race_Start_Number = 6;
constexpr int c_Monster_Model_Start_Number = 20001;

constexpr float c_fAttack_Delay_Time = 0.2f;
constexpr float c_fHit_Delay_Time = 0.1f;
constexpr float c_fCrash_Wave_Time = 0.2f;
constexpr float c_fCrash_Wave_Distance = 3.0f;

constexpr float c_fHeight_Step_Distance = 50.0f;

enum
{
	DISTANCE_TYPE_FOUR_WAY,
	DISTANCE_TYPE_EIGHT_WAY,
	DISTANCE_TYPE_ONE_WAY,
	DISTANCE_TYPE_MAX_NUM
};

constexpr float c_fMagic_Script_Version = 1.0f;
constexpr float c_fSkill_Script_Version = 1.0f;
constexpr float c_fMagicSoundInformation_Version = 1.0f;
constexpr float c_fBattleCommand_Script_Version = 1.0f;
constexpr float c_fEmotionCommand_Script_Version = 1.0f;
constexpr float c_fActive_Script_Version = 1.0f;
constexpr float c_fPassive_Script_Version = 1.0f;

// Used by PushMove
constexpr float c_fWalkDistance = 175.0f;
constexpr float c_fRunDistance = 310.0f;

#define FILE_MAX_LEN 128

enum
{
#ifdef ENABLE_PROTO_RENEWAL
	ITEM_SOCKET_SLOT_MAX_NUM = 6,
	METIN_SOCKET_MAX_NUM = 3,
#else
	ITEM_SOCKET_SLOT_MAX_NUM = 3,
#endif
	// refactored attribute slot begin
	ITEM_ATTRIBUTE_SLOT_NORM_NUM = 5,
	ITEM_ATTRIBUTE_SLOT_RARE_NUM = 2,

	ITEM_ATTRIBUTE_SLOT_NORM_START = 0,
	ITEM_ATTRIBUTE_SLOT_NORM_END = ITEM_ATTRIBUTE_SLOT_NORM_START + ITEM_ATTRIBUTE_SLOT_NORM_NUM,

	ITEM_ATTRIBUTE_SLOT_RARE_START = ITEM_ATTRIBUTE_SLOT_NORM_END,
	ITEM_ATTRIBUTE_SLOT_RARE_END = ITEM_ATTRIBUTE_SLOT_RARE_START + ITEM_ATTRIBUTE_SLOT_RARE_NUM,

	ITEM_ATTRIBUTE_SLOT_MAX_NUM = ITEM_ATTRIBUTE_SLOT_RARE_END, // 7

#ifdef ENABLE_YOHARA_SYSTEM
	APPLY_RANDOM_SLOT_MAX_NUM = 3,
	ITEM_RANDOM_VALUES_MAX_NUM = 4,
#endif

#ifdef ENABLE_REFINE_ELEMENT
	MAX_ELEMENTS_SPELL = 3,
#endif
	// refactored attribute slot end
};

#pragma pack(push)
#pragma pack(1)

typedef struct SQuickSlot
{
	uint8_t Type;
	uint8_t Position;
} TQuickSlot;

typedef struct TPlayerItemAttribute
{
	uint16_t wType;	//@fixme436
	int16_t sValue;
} TPlayerItemAttribute;

#ifdef ENABLE_YOHARA_SYSTEM
typedef struct TPlayerItemApplyRandom
{
	uint16_t wType;	//@fixme532
	int16_t sValue;
	uint8_t row;
} TPlayerItemApplyRandom;
#endif

#ifdef ENABLE_CHEQUE_SYSTEM
typedef struct SItemPriceType
{
	SItemPriceType() { dwPrice = dwCheque = 0; }
	SItemPriceType(uint32_t gold, uint32_t cheque)
	{
		dwPrice = gold;
		dwCheque = cheque;
	}
	uint32_t dwPrice;
	uint32_t dwCheque;
} TItemPriceType;
#endif

typedef struct packet_item
{
	uint32_t vnum;
	uint8_t count;
	uint32_t flags;
	uint32_t anti_flags;
#ifdef ENABLE_SOULBIND_SYSTEM
	long nSealDate;
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	uint32_t dwTransmutationVnum;
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
	bool is_basic;
#endif
#ifdef ENABLE_REFINE_ELEMENT
	uint8_t grade_element;
	uint32_t attack_element[MAX_ELEMENTS_SPELL];
	uint8_t element_type_bonus;
	int16_t elements_value_bonus[MAX_ELEMENTS_SPELL];
#endif
#ifdef ENABLE_SET_ITEM
	uint8_t set_value;
#endif
	long alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#ifdef ENABLE_YOHARA_SYSTEM
	TPlayerItemApplyRandom aApplyRandom[APPLY_RANDOM_SLOT_MAX_NUM];
	uint16_t alRandomValues[ITEM_RANDOM_VALUES_MAX_NUM];
#endif
} TItemData;

typedef struct packet_shop_item
{
	uint32_t vnum;
#ifdef ENABLE_CHEQUE_SYSTEM
	TItemPriceType price;
#else
	uint32_t price;
#endif
	uint8_t count;
#ifdef ENABLE_MEDAL_OF_HONOR
	uint8_t limit_level;
#endif
	uint8_t display_pos;
	long alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#ifdef ENABLE_YOHARA_SYSTEM
	TPlayerItemApplyRandom	aApplyRandom[APPLY_RANDOM_SLOT_MAX_NUM];
	uint16_t alRandomValues[ITEM_RANDOM_VALUES_MAX_NUM];
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	uint32_t dwTransmutationVnum;
#endif
#ifdef ENABLE_12ZI
	uint32_t getLimitedCount;
	uint32_t getLimitedPurchaseCount;
#endif
#ifdef ENABLE_REFINE_ELEMENT
	uint8_t grade_element;
	uint32_t attack_element[MAX_ELEMENTS_SPELL];
	uint8_t element_type_bonus;
	int16_t elements_value_bonus[MAX_ELEMENTS_SPELL];
#endif
#ifdef ENABLE_SET_ITEM
	uint8_t set_value;
#endif
} TShopItemData;

#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
typedef struct SShopPriceLimitCount
{
	uint32_t dwLimitedCount;
	uint32_t dwLimitedPurchaseCount;
} TShopPriceLimitCount;
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
typedef struct SGrowthPetInfo
{
	uint32_t pet_id;
	uint32_t pet_vnum;
	bool is_summoned;
	uint8_t flash_event;
	char pet_nick[25]; //CHARACTER_NAME_MAX_LEN + 1
	uint8_t pet_level;
	uint8_t evol_level;
	uint8_t pet_type;
	float pet_hp;
	float pet_def;
	float pet_sp;
	time_t pet_birthday;
	time_t pet_end_time;
	time_t pet_max_time;
	uint32_t exp_monster;
	uint32_t exp_item;
	uint32_t next_exp_monster;
	uint32_t next_exp_item;
	uint8_t skill_count;
	uint8_t skill_vnum[3];
	uint8_t skill_level[3];
	float skill_spec[3];
	time_t skill_cool[3];
	float skill_formula1[3];
	float next_skill_formula1[3];
	float skill_formula2[3];
	float next_skill_formula2[3];
} TGrowthPetInfo;
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
typedef struct SExtBattlePassRewardItem
{
	uint32_t dwVnum;
	uint8_t bCount;
} TExtBattlePassRewardItem;

typedef struct SExtBattlePassMissionInfo
{
	uint8_t bMissionIndex;
	uint8_t bMissionType;
	uint32_t dwMissionInfo[3];
	TExtBattlePassRewardItem aRewardList[3];
} TExtBattlePassMissionInfo;

typedef struct SExtBattlePassRanking
{
	uint8_t bPos;
	char playerName[24 + 1];
	uint32_t dwFinishTime;
} TExtBattlePassRanking;
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
enum class ETRANSMUTATIONTYPE : uint8_t
{
	TRANSMUTATION_TYPE_MOUNT,
	TRANSMUTATION_TYPE_ITEM
};

enum class ETRANSMUTATIONSLOTTYPE : size_t
{
	TRANSMUTATION_SLOT_LEFT,
	TRANSMUTATION_SLOT_RIGHT,

	TRANSMUTATION_SLOT_MAX
};

enum class ETRANSMUTATIONSETTINGS : uint32_t
{
	TRANSMUTATION_ITEM_PRICE = 50000000, // 50M
	TRANSMUTATION_MOUNT_PRICE = 30000000, // 30M
	TRANSMUTATION_TICKET_1 = 72326,
	TRANSMUTATION_TICKET_2 = 72341,
	TRANSMUTATION_CLEAR_SCROLL = 72325,
};
#endif

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
typedef struct packet_gemshop_item
{
	uint32_t vnum;
	uint32_t price;
	uint8_t count;
	uint8_t display_pos;

	uint8_t status;
	uint8_t unlocked;
} TGemShopItem;
#endif

#pragma pack(pop)

inline float GetSqrtDistance(int ix1, int iy1, int ix2, int iy2) // By sqrt
{
	const float dx = ix1 - ix2;
	const float dy = iy1 - iy2;

	return sqrtf(dx * dx + dy * dy);
}

// DEFAULT_FONT
void DefaultFont_Startup();
void DefaultFont_Cleanup();
void DefaultFont_SetName(const char * c_szFontName);
CResource * DefaultFont_GetResource();
CResource * DefaultItalicFont_GetResource();
// END_OF_DEFAULT_FONT

void SetGuildSymbolPath(const char * c_szPathName);
const char * GetGuildSymbolFileName(uint32_t dwGuildID);
uint8_t SlotTypeToInvenType(uint8_t bSlotType);
uint8_t WindowTypeToSlotType(uint8_t bWindowType);
#ifdef ENABLE_DETAILS_UI
uint16_t ApplyTypeToPointType(uint16_t wApplyType);	//@fixme436
#endif
#ifdef ENABLE_FLOWER_EVENT
uint16_t PointTypeToApplyType(uint16_t wPointType);	//@fixme436
#endif
#ifdef ENABLE_AURA_SYSTEM
int* GetAuraRefineInfo(uint8_t bLevel);
#endif

#ifdef WJ_ENABLE_TRADABLE_ICON
enum ETopWindowTypes
{
	ON_TOP_WND_NONE,
	ON_TOP_WND_SHOP,
	ON_TOP_WND_EXCHANGE,
	ON_TOP_WND_SAFEBOX,
	ON_TOP_WND_PRIVATE_SHOP,
	ON_TOP_WND_ITEM_COMB,				//ENABLE_MOVE_COSTUME_ATTR
	ON_TOP_WND_PET_FEED,				//ENABLE_GROWTH_PET_SYSTEM
	ON_TOP_WND_FISH_EVENT,				//ENABLE_FISH_EVENT
	ON_TOP_WND_MAILBOX,					//ENABLE_MAILBOX
	ON_TOP_WND_PET_ATTR_CHANGE,			//ENABLE_PET_ATTR_DETERMINE
	ON_TOP_WND_LUCKY_BOX,				//ENABLE_LUCKY_EVENT
	ON_TOP_WND_ATTR67,					//ENABLE_ATTR_6TH_7TH
	ON_TOP_WND_PET_PRIMIUM_FEEDSTUFF,	//ENABLE_PET_PRIMIUM_FEEDSTUFF
	ON_TOP_WND_PASSIVE_ATTR,			//ENABLE_PASSIVE_ATTR
	ON_TOP_WND_MAX
};
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
enum EAcceSlots
{
	ACCE_SLOT_LEFT,
	ACCE_SLOT_RIGHT,
	ACCE_SLOT_RESULT,

	ACCE_SLOT_MAX,
};

enum EAcceTypes
{
	ACCE_SLOT_TYPE_COMBINE,
	ACCE_SLOT_TYPE_ABSORB,

	ACCE_SLOT_TYPE_MAX,
};

enum EAcceInfo
{
	ACCE_ABSORPTION_SOCKET = 0,
	ACCE_ABSORBED_SOCKET = 1,
	ACCE_CLEAN_ATTR_VALUE0 = 7,
	ACCE_WINDOW_MAX_MATERIALS = 2,
};
#endif

#ifdef ENABLE_MOVE_COSTUME_ATTR
enum ECostumeAttrTypes
{
	COMB_WND_SLOT_MEDIUM,
	COMB_WND_SLOT_BASE,
	COMB_WND_SLOT_MATERIAL,
	COMB_WND_SLOT_RESULT,

	COMB_WND_SLOT_MAX = 3,
};
#endif

#ifdef ENABLE_AURA_SYSTEM
const uint8_t c_AuraMaxLevel = 250;

enum EAuraRefineInfoSlot
{
	AURA_REFINE_INFO_SLOT_CURRENT,
	AURA_REFINE_INFO_SLOT_NEXT,
	AURA_REFINE_INFO_SLOT_EVOLVED,
	AURA_REFINE_INFO_SLOT_MAX
};

enum EAuraWindowType
{
	AURA_WINDOW_TYPE_ABSORB,
	AURA_WINDOW_TYPE_GROWTH,
	AURA_WINDOW_TYPE_EVOLVE,
	AURA_WINDOW_TYPE_MAX,
};

enum EAuraSlotType
{
	AURA_SLOT_MAIN,
	AURA_SLOT_SUB,
	AURA_SLOT_RESULT,
	AURA_SLOT_MAX
};

enum EAuraRefineInfoType
{
	AURA_REFINE_INFO_STEP,
	AURA_REFINE_INFO_LEVEL_MIN,
	AURA_REFINE_INFO_LEVEL_MAX,
	AURA_REFINE_INFO_NEED_EXP,
	AURA_REFINE_INFO_MATERIAL_VNUM,
	AURA_REFINE_INFO_MATERIAL_COUNT,
	AURA_REFINE_INFO_NEED_GOLD,
	AURA_REFINE_INFO_EVOLVE_PCT,
	AURA_REFINE_INFO_MAX
};
#endif

#ifdef ENABLE_CUBE_RENEWAL
enum ECubeRenewalCategory
{
	CUBE_ARMOR,
	CUBE_WEAPON,
	CUBE_ACCESSORY,
	CUBE_BELT,
	CUBE_EVENT,
	CUBE_ETC,
	CUBE_JOB,
	CUBE_SETADD_WEAPON,
	CUBE_SETADD_ARMOR_BODY,
	CUBE_SETADD_ARMOR_HELMET,
	CUBE_PET,
	CUBE_SKILL_BOOK,
	CUBE_ARMOR_GLOVE,
	CUBE_CATEGORY_MAX
};
#endif

#ifdef ENABLE_SET_ITEM
enum ESetItemType
{
	SET_ITEM_SET_VALUE_NONE,
	SET_ITEM_SET_VALUE_1,
	SET_ITEM_SET_VALUE_2,
	SET_ITEM_SET_VALUE_3,
	SET_ITEM_SET_VALUE_4,
	SET_ITEM_SET_VALUE_5,
	SET_ITEM_SET_VALUE_MAX
};
#endif

#ifdef ENABLE_AUTO_SYSTEM
enum EAutoSlots
{
	AUTO_SKILL_SLOT_MAX = 12,
	AUTO_POSITINO_SLOT_MAX = 12 + 12 + 1,
};

const int auto_red_potions[] = { 27001, 27002, 27003, 27007 };
const int auto_blue_potions[] = { 27004, 27005, 27006, 27008 };

const float AUTO_MAX_FOCUS_DISTANCE = 6000.0f;
const uint8_t AUTO_MAX_KILL_SECOND = 10;

typedef struct SAutoSlot
{
	uint32_t slotPos;
	uint32_t fillingTime;
	long nextUsage;
} TAutoSlot;
#endif

#ifdef ENABLE_PASSIVE_SYSTEM
enum EPassiveAttr
{
	PASSIVE_ATTR_SLOT_INDEX_JOB,
	PASSIVE_ATTR_SLOT_INDEX_WEAPON,
	PASSIVE_ATTR_SLOT_INDEX_ELEMENT,
	PASSIVE_ATTR_SLOT_INDEX_ARMOR,
	PASSIVE_ATTR_SLOT_INDEX_ACCE,
	PASSIVE_ATTR_SLOT_INDEX_MAX,
};
#endif

#ifdef ENABLE_KEYCHANGE_SYSTEM
enum EKeyboardSetting
{
	KEY_NONE,
	KEY_MOVE_UP_1,
	KEY_MOVE_DOWN_1,
	KEY_MOVE_LEFT_1,
	KEY_MOVE_RIGHT_1,
	KEY_MOVE_UP_2,
	KEY_MOVE_DOWN_2,
	KEY_MOVE_LEFT_2,
	KEY_MOVE_RIGHT_2,
	KEY_CAMERA_ROTATE_POSITIVE_1,
	KEY_CAMERA_ROTATE_NEGATIVE_1,
	KEY_CAMERA_ZOOM_POSITIVE_1,
	KEY_CAMERA_ZOOM_NEGATIVE_1,
	KEY_CAMERA_PITCH_POSITIVE_1,
	KEY_CAMERA_PITCH_NEGATIVE_1,
	KEY_CAMERA_ROTATE_POSITIVE_2,
	KEY_CAMERA_ROTATE_NEGATIVE_2,
	KEY_CAMERA_ZOOM_POSITIVE_2,
	KEY_CAMERA_ZOOM_NEGATIVE_2,
	KEY_CAMERA_PITCH_POSITIVE_2,
	KEY_CAMERA_PITCH_NEGATIVE_2,
	KEY_ROOTING_1,
	KEY_ROOTING_2,
	KEY_ATTACK,
	KEY_RIDEMYHORS,
	KEY_FEEDMYHORS,
	KEY_BYEMYHORS,
	KEY_RIDEHORS,
	KEY_EMOTION1,
	KEY_EMOTION2,
	KEY_EMOTION3,
	KEY_EMOTION4,
	KEY_EMOTION5,
	KEY_EMOTION6,
	KEY_EMOTION7,
	KEY_EMOTION8,
	KEY_EMOTION9,
	KEY_SLOT_1,
	KEY_SLOT_2,
	KEY_SLOT_3,
	KEY_SLOT_4,
	KEY_SLOT_5,
	KEY_SLOT_6,
	KEY_SLOT_7,
	KEY_SLOT_8,
	KEY_SLOT_CHANGE_1,
	KEY_SLOT_CHANGE_2,
	KEY_SLOT_CHANGE_3,
	KEY_SLOT_CHANGE_4,
	KEY_OPEN_STATE,
	KEY_OPEN_SKILL,
	KEY_OPEN_QUEST,
	KEY_OPEN_INVENTORY,
	KEY_OPEN_DDS,
	KEY_OPEN_MINIMAP,
	KEY_OPEN_LOGCHAT,
	KEY_OPEN_PET,
	KEY_OPEN_GUILD,
	KEY_OPEN_MESSENGER,
	KEY_OPEN_HELP,
	KEY_OPEN_ACTION,
	KEY_SCROLL_ONOFF,
	KEY_PLUS_MINIMAP,
	KEY_MIN_MINIMAP,
	KEY_SCREENSHOT,
	KEY_SHOW_NAME,
	KEY_OPEN_AUTO,
	KEY_AUTO_RUN,
	KEY_NEXT_TARGET,
	KEY_MONSTER_CARD,
	KEY_PARTY_MATCH,
	KEY_SELECT_DSS_1,
	KEY_SELECT_DSS_2,
	KEY_PASSIVE_ATTR1,
	KEY_PASSIVE_ATTR2,

	KEY_ADDKEYBUFFERCONTROL = 100,
	KEY_ADDKEYBUFFERALT = 300,
	KEY_ADDKEYBUFFERSHIFT = 500,
};
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
enum EPetFeedEventType
{
	FEED_LIFE_TIME_EVENT,
	FEED_EVOL_EVENT,
	FEED_EXP_EVENT,
	FEED_BUTTON_MAX = 3,
	PET_GROWTH_EVOL_MAX = 4,
	PET_GROWTH_SKILL_OPEN_EVOL_LEVEL = 4,
	PET_GROWTH_SKILL_LEVEL_MAX = 20,
	SPECIAL_EVOL_MIN_AGE = 2592000,
	LIFE_TIME_FLASH_MIN_TIME = 3600,
	PET_SKILL_COUNT_MAX = 3,
	PET_FEED_SLOT_MAX = 9,
	PET_REVIVE_MATERIAL_SLOT_MAX = 10,
};

enum EPetQuickSlot
{
	QUICK_SLOT_POS_ERROR,
	QUICK_SLOT_ITEM_USE_SUCCESS,
	QUICK_SLOT_IS_NOT_ITEM,
	QUICK_SLOT_PET_ITEM_USE_SUCCESS,
	QUICK_SLOT_PET_ITEM_USE_FAILED,
	QUICK_SLOT_CAN_NOT_USE_PET_ITEM
};

enum EPetAttrChangeInfo
{
	PET_WND_SLOT_ATTR_CHANGE,
	PET_WND_SLOT_ATTR_CHANGE_ITEM,
	PET_WND_SLOT_ATTR_CHANGE_RESULT,
	PET_WND_SLOT_ATTR_CHANGE_MAX,
};

enum EPetPageType
{
	PET_WINDOW_INFO,
	PET_WINDOW_ATTR_CHANGE,
	PET_WINDOW_PRIMIUM_FEEDSTUFF
};
#endif

#ifdef ENABLE_WEATHER_INFO
enum DailyWeather
{
	MIDNIGHT	= 0,	// Mitternacht
	BEFORE_MID	= 23,	// Mitternacht

	SUNRISE		= 6,	// Sonnenaufgang
	NOON		= 9,	// Mittag
	SUNSET		= 18,	// Sonnenuntergang
	DARKNESS	= 20,	// Nacht
};
#endif

#ifdef ENABLE_DUNGEON_INFO_SYSTEM
enum EDungeonInfo
{
	MAX_DUNGEONS = 100,
	MAX_REQUIRED_ITEMS = 3,
	MAX_BOSS_ITEM_SLOTS = 16,
	MAX_BOSS_DROP_ITEMS = 255,
	MAX_DUNGEON_SCROLL = 10,
};
#endif
