#include "StdAfx.h"
#include "GameType.h"

std::string g_strResourcePath = "d:/ymir work/";
std::string g_strImagePath = "d:/ymir work/ui/";

std::string g_strGuildSymbolPathName = "mark/10/";

// DEFAULT_FONT
static std::string gs_strDefaultFontName = "����ü:12.fnt";
static std::string gs_strDefaultItalicFontName = "����ü:12i.fnt";
static CResource * gs_pkDefaultFont = nullptr;
static CResource * gs_pkDefaultItalicFont = nullptr;

static bool gs_isReloadDefaultFont = false;

void DefaultFont_Startup()
{
	gs_pkDefaultFont = nullptr;
}

void DefaultFont_Cleanup()
{
	if (gs_pkDefaultFont)
		gs_pkDefaultFont->Release();
}

void DefaultFont_SetName(const char * c_szFontName)
{
	gs_strDefaultFontName = c_szFontName;
	gs_strDefaultFontName += ".fnt";

	gs_strDefaultItalicFontName = c_szFontName;
	if (strchr(c_szFontName, ':'))
		gs_strDefaultItalicFontName += "i";
	gs_strDefaultItalicFontName += ".fnt";

	gs_isReloadDefaultFont = true;
}

bool ReloadDefaultFonts()
{
	CResourceManager & rkResMgr = CResourceManager::Instance();

	gs_isReloadDefaultFont = false;

	CResource * pkNewFont = rkResMgr.GetResourcePointer(gs_strDefaultFontName.c_str());
	pkNewFont->AddReference();
	if (gs_pkDefaultFont)
		gs_pkDefaultFont->Release();
	gs_pkDefaultFont = pkNewFont;

	CResource * pkNewItalicFont = rkResMgr.GetResourcePointer(gs_strDefaultItalicFontName.c_str());
	pkNewItalicFont->AddReference();
	if (gs_pkDefaultItalicFont)
		gs_pkDefaultItalicFont->Release();
	gs_pkDefaultItalicFont = pkNewItalicFont;

	return true;
}

CResource * DefaultFont_GetResource()
{
	if (!gs_pkDefaultFont || gs_isReloadDefaultFont)
		ReloadDefaultFonts();
	return gs_pkDefaultFont;
}

CResource * DefaultItalicFont_GetResource()
{
	if (!gs_pkDefaultItalicFont || gs_isReloadDefaultFont)
		ReloadDefaultFonts();
	return gs_pkDefaultItalicFont;
}

// END_OF_DEFAULT_FONT

void SetGuildSymbolPath(const char * c_szPathName)
{
	g_strGuildSymbolPathName = "mark/";
	g_strGuildSymbolPathName += c_szPathName;
	g_strGuildSymbolPathName += "/";
}

const char * GetGuildSymbolFileName(uint32_t dwGuildID)
{
	return _getf("%s%03d.jpg", g_strGuildSymbolPathName.c_str(), dwGuildID);
}

//// SlotTypeToInvenType
uint8_t c_aSlotTypeToInvenType[SLOT_TYPE_MAX] =
{
	RESERVED_WINDOW, // SLOT_TYPE_NONE
	INVENTORY, // SLOT_TYPE_INVENTORY
	RESERVED_WINDOW, // SLOT_TYPE_SKILL
	RESERVED_WINDOW, // SLOT_TYPE_EMOTION
	RESERVED_WINDOW, // SLOT_TYPE_SHOP
	RESERVED_WINDOW, // SLOT_TYPE_EXCHANGE_OWNER
	RESERVED_WINDOW, // SLOT_TYPE_EXCHANGE_TARGET
	RESERVED_WINDOW, // SLOT_TYPE_QUICK_SLOT
	RESERVED_WINDOW, // SLOT_TYPE_SAFEBOX <- In case of SAFEBOX and MALL, hard-coded LEGACY code is maintained.
	RESERVED_WINDOW, // SLOT_TYPE_GUILDBANK
	ACCEREFINE, // SLOT_TYPE_ACCE
	RESERVED_WINDOW, // SLOT_TYPE_PRIVATE_SHOP
	RESERVED_WINDOW, // SLOT_TYPE_MALL <- In case of SAFEBOX and MALL, hard-coded LEGACY code is maintained.
	DRAGON_SOUL_INVENTORY, // SLOT_TYPE_DRAGON_SOUL_INVENTORY
	PET_FEED, // SLOT_TYPE_PET_FEED_WINDOW
	EQUIPMENT, // SLOT_TYPE_EQUIPMENT
	BELT_INVENTORY, // SLOT_TYPE_BELT_INVENTORY
	RESERVED_WINDOW, // SLOT_TYPE_AUTO
	CHANGELOOK, // SLOT_TYPE_CHANGE_LOOK
	RESERVED_WINDOW, // SLOT_TYPE_FISH_EVENT
	AURA_REFINE, // SLOT_TYPE_AURA
	RESERVED_WINDOW, // SLOT_TYPE_PREMIUM_PRIVATE_SHOP
#ifdef ENABLE_SWITCHBOT
	SWITCHBOT,
#endif
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	ADDITIONAL_EQUIPMENT_1, // SLOT_TYPE_NEW_EQUIPMENT_PAGE
#endif
};

uint8_t SlotTypeToInvenType(uint8_t bSlotType)
{
	if (bSlotType >= SLOT_TYPE_MAX)
		return RESERVED_WINDOW;
	return c_aSlotTypeToInvenType[bSlotType];
}

//// WindowTypeToSlotType
uint8_t c_aWndTypeToSlotType[WINDOW_TYPE_MAX] =
{
	SLOT_TYPE_NONE,
	SLOT_TYPE_INVENTORY, // INVENTORY
	SLOT_TYPE_EQUIPMENT, // EQUIPMENT
	SLOT_TYPE_SAFEBOX, // SAFEBOX
	SLOT_TYPE_MALL, // MALL
	SLOT_TYPE_DRAGON_SOUL_INVENTORY, // DRAGON_SOUL_INVENTORY
	SLOT_TYPE_BELT_INVENTORY, // BELT_INVENTORY
	SLOT_TYPE_GUILDBANK, // GUILDBANK
	SLOT_TYPE_NONE, // MAIL
	SLOT_TYPE_NONE, // NPC_STORAGE
	SLOT_TYPE_PREMIUM_PRIVATE_SHOP, // PREMIUM_PRIVATE_SHOP
	SLOT_TYPE_ACCE, // ACCEREFINE
	SLOT_TYPE_NONE, // GROUND
	SLOT_TYPE_PET_FEED_WINDOW, // PET_FEED
	SLOT_TYPE_CHANGE_LOOK, // CHANGELOOK
	SLOT_TYPE_AURA, // AURA_REFINE
#ifdef ENABLE_SWITCHBOT
	SLOT_TYPE_SWITCHBOT, // SWITCHBOT
#endif
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	SLOT_TYPE_ADDITIONAL_EQUIPMENT_1, // ADDITIONAL_EQUIPMENT_1
#endif
};

uint8_t WindowTypeToSlotType(uint8_t bWindowType)
{
	if (bWindowType >= WINDOW_TYPE_MAX)
		return RESERVED_WINDOW;

	return c_aWndTypeToSlotType[bWindowType];
}

////

#ifdef ENABLE_DETAILS_UI
#include "Packet.h"
typedef struct SApplyInfo
{
	uint16_t	wPointType;                          // APPLY -> POINT
} TApplyInfo;

const TApplyInfo aApplyInfo[CItemData::MAX_APPLY_NUM] =
{
	{ POINT_NONE,					},	// 0	- APPLY_NONE,	0
	{ POINT_MAX_HP,					},	// 6	- APPLY_MAX_HP,	1
	{ POINT_MAX_SP,					},	// 8	- APPLY_MAX_SP,	2
	{ POINT_HT,						},	// 13	- APPLY_CON,	3
	{ POINT_IQ,						},	// 15	- APPLY_INT,	4
	{ POINT_ST,						},	// 12	- APPLY_STR,	5
	{ POINT_DX,						},	// 14	- APPLY_DEX,	6
	{ POINT_ATT_SPEED,				},	// 17	- APPLY_ATT_SPEED,	7
	{ POINT_MOV_SPEED,				},	// 19	- APPLY_MOV_SPEED,	8
	{ POINT_CASTING_SPEED,			},	// 21	- APPLY_CAST_SPEED,	9
	{ POINT_HP_REGEN,				},	// 32	- APPLY_HP_REGEN,	10
	{ POINT_SP_REGEN,				},	// 33	- APPLY_SP_REGEN,	11
	{ POINT_POISON_PCT,				},	// 37	- APPLY_POISON_PCT,	12
	{ POINT_STUN_PCT,				},	// 38	- APPLY_STUN_PCT,	13
	{ POINT_SLOW_PCT,				},	// 39	- APPLY_SLOW_PCT,	14
	{ POINT_CRITICAL_PCT,			},	// 40	- APPLY_CRITICAL_PCT,	15
	{ POINT_PENETRATE_PCT,			},	// 41	- APPLY_PENETRATE_PCT,	16
	{ POINT_ATTBONUS_HUMAN,			},	// 43	- APPLY_ATTBONUS_HUMAN,	17
	{ POINT_ATTBONUS_ANIMAL,		},	// 44	- APPLY_ATTBONUS_ANIMAL,	18
	{ POINT_ATTBONUS_ORC,			},	// 45	- APPLY_ATTBONUS_ORC,	19
	{ POINT_ATTBONUS_MILGYO,		},	// 46	- APPLY_ATTBONUS_MILGYO,	20
	{ POINT_ATTBONUS_UNDEAD,		},	// 47	- APPLY_ATTBONUS_UNDEAD,	21
	{ POINT_ATTBONUS_DEVIL,			},	// 48	- APPLY_ATTBONUS_DEVIL,	22
	{ POINT_STEAL_HP,				},	// 63	- APPLY_STEAL_HP,	23
	{ POINT_STEAL_SP,				},	// 64	- APPLY_STEAL_SP,	24
	{ POINT_MANA_BURN_PCT,			},	// 65	- APPLY_MANA_BURN_PCT,	25
	{ POINT_DAMAGE_SP_RECOVER,		},	// 66	- APPLY_DAMAGE_SP_RECOVER,	26
	{ POINT_BLOCK,					},	// 67	- APPLY_BLOCK,	27
	{ POINT_DODGE,					},	// 68	- APPLY_DODGE,	28
	{ POINT_RESIST_SWORD,			},	// 69	- APPLY_RESIST_SWORD,	29
	{ POINT_RESIST_TWOHAND,			},	// 70	- APPLY_RESIST_TWOHAND,	30
	{ POINT_RESIST_DAGGER,			},	// 71	- APPLY_RESIST_DAGGER,	31
	{ POINT_RESIST_BELL,			},	// 72	- APPLY_RESIST_BELL,	32
	{ POINT_RESIST_FAN,				},	// 73	- APPLY_RESIST_FAN,	33
	{ POINT_RESIST_BOW,				},	// 74	- APPLY_RESIST_BOW,	34
	{ POINT_RESIST_FIRE,			},	// 75	- APPLY_RESIST_FIRE,	35
	{ POINT_RESIST_ELEC,			},	// 76	- APPLY_RESIST_ELEC,	36
	{ POINT_RESIST_MAGIC,			},	// 77	- APPLY_RESIST_MAGIC,	37
	{ POINT_RESIST_WIND,			},	// 78	- APPLY_RESIST_WIND,	38
	{ POINT_REFLECT_MELEE,			},	// 79	- APPLY_REFLECT_MELEE,	39
	{ POINT_REFLECT_CURSE,			},	// 80	- APPLY_REFLECT_CURSE,	40
	{ POINT_POISON_REDUCE,			},	// 81	- APPLY_POISON_REDUCE,	41
	{ POINT_KILL_SP_RECOVER,		},	// 82	- APPLY_KILL_SP_RECOVER,	42
	{ POINT_EXP_DOUBLE_BONUS,		},	// 83	- APPLY_EXP_DOUBLE_BONUS,	43
	{ POINT_GOLD_DOUBLE_BONUS,		},	// 84	- APPLY_GOLD_DOUBLE_BONUS,	44
	{ POINT_ITEM_DROP_BONUS,		},	// 85	- APPLY_ITEM_DROP_BONUS,	45
	{ POINT_POTION_BONUS,			},	// 86	- APPLY_POTION_BONUS,	46
	{ POINT_KILL_HP_RECOVER,		},	// 87	- APPLY_KILL_HP_RECOVER,	47
	{ POINT_IMMUNE_STUN,			},	// 88	- APPLY_IMMUNE_STUN,	48
	{ POINT_IMMUNE_SLOW,			},	// 89	- APPLY_IMMUNE_SLOW,	49
	{ POINT_IMMUNE_FALL,			},	// 90	- APPLY_IMMUNE_FALL,	50
	{ POINT_NONE,					},	// 0	- APPLY_SKILL,	51
	{ POINT_BOW_DISTANCE,			},	// 34	- APPLY_BOW_DISTANCE,	52
	{ POINT_ATT_GRADE_BONUS,		},	// 95	- APPLY_ATT_GRADE,	53
	{ POINT_DEF_GRADE_BONUS,		},	// 96	- APPLY_DEF_GRADE,	54
	{ POINT_MAGIC_ATT_GRADE_BONUS,	},	// 97	- APPLY_MAGIC_ATT_GRADE,	55
	{ POINT_MAGIC_DEF_GRADE_BONUS,	},	// 98	- APPLY_MAGIC_DEF_GRADE,	56
	{ POINT_CURSE_PCT,				},	// 42	- APPLY_CURSE_PCT,	57
	{ POINT_MAX_STAMINA				},	// 10	- APPLY_MAX_STAMINA,	58
	{ POINT_ATTBONUS_WARRIOR		},	// 54	- APPLY_ATTBONUS_WARRIOR,	59
	{ POINT_ATTBONUS_ASSASSIN		},	// 55	- APPLY_ATTBONUS_ASSASSIN,	60
	{ POINT_ATTBONUS_SURA			},	// 56	- APPLY_ATTBONUS_SURA,	61
	{ POINT_ATTBONUS_SHAMAN			},	// 57	- APPLY_ATTBONUS_SHAMAN,	62
	{ POINT_ATTBONUS_MONSTER		},	// 53	- APPLY_ATTBONUS_MONSTER,	63
	{ POINT_ATT_BONUS				},	// 93	- APPLY_MALL_ATTBONUS	64
	{ POINT_MALL_DEFBONUS			},	// 115	- APPLY_MALL_DEFBONUS,	65
	{ POINT_MALL_EXPBONUS			},	// 116	- APPLY_MALL_EXPBONUS,	66
	{ POINT_MALL_ITEMBONUS			},	// 117	- APPLY_MALL_ITEMBONUS,	67
	{ POINT_MALL_GOLDBONUS			},	// 118	- APPLY_MALL_GOLDBONUS,	68
	{ POINT_MAX_HP_PCT				},	// 119	- APPLY_MAX_HP_PCT,	69
	{ POINT_MAX_SP_PCT				},	// 120	- APPLY_MAX_SP_PCT,	70
	{ POINT_SKILL_DAMAGE_BONUS		},	// 121	- APPLY_SKILL_DAMAGE_BONUS,	71
	{ POINT_NORMAL_HIT_DAMAGE_BONUS	},	// 122	- APPLY_NORMAL_HIT_DAMAGE_BONUS,	72
	{ POINT_SKILL_DEFEND_BONUS		},	// 123	- APPLY_SKILL_DEFEND_BONUS,	73
	{ POINT_NORMAL_HIT_DEFEND_BONUS	},	// 124	- APPLY_NORMAL_HIT_DEFEND_BONUS,	74
	{ POINT_PC_BANG_EXP_BONUS		},	// 125	- APPLY_PC_BANG_EXP_BONUS,	75
	{ POINT_PC_BANG_DROP_BONUS		},	// 126	- APPLY_PC_BANG_DROP_BONUS,	76
	{ POINT_NONE,					},	// 0	- POINT_NONE,	77
	{ POINT_RESIST_WARRIOR,			},	// 59	- APPLY_RESIST_WARRIOR,	78
	{ POINT_RESIST_ASSASSIN,		},	// 60	- APPLY_RESIST_ASSASSIN,	79
	{ POINT_RESIST_SURA,			},	// 61	- APPLY_RESIST_SURA,	80
	{ POINT_RESIST_SHAMAN,			},	// 62	- APPLY_RESIST_SHAMAN,	81
	{ POINT_ENERGY					},	// 128	- APPLY_ENERGY,	82
	{ POINT_DEF_GRADE				},	// 16	- APPLY_DEF_GRADE,	83
	{ POINT_COSTUME_ATTR_BONUS		},	// 130	- APPLY_COSTUME_ATTR_BONUS,	84
	{ POINT_MAGIC_ATT_BONUS_PER		},	// 131	- APPLY_MAGIC_ATTBONUS_PER,	85
	{ POINT_MELEE_MAGIC_ATT_BONUS_PER },// 132	- APPLY_MELEE_MAGIC_ATTBONUS_PER,	86
	{ POINT_RESIST_ICE,				},	// 133	- APPLY_RESIST_ICE,	87
	{ POINT_RESIST_EARTH,			},	// 134	- APPLY_RESIST_EARTH,	88
	{ POINT_RESIST_DARK,			},	// 135	- APPLY_RESIST_DARK,	89
	{ POINT_RESIST_CRITICAL,		},	// 136	- APPLY_ANTI_CRITICAL_PCT,	90
	{ POINT_RESIST_PENETRATE,		},	// 137	- APPLY_ANTI_PENETRATE_PCT,	91
	{ POINT_BLEEDING_REDUCE,		},	// 138	- APPLY_BLEEDING_REDUCE,	92
	{ POINT_BLEEDING_PCT,			},	// 139	- APPLY_BLEEDING_PCT,	93
	{ POINT_ATTBONUS_WOLFMAN,		},	// 140	- APPLY_ATTBONUS_WOLFMAN,	94
	{ POINT_RESIST_WOLFMAN,			},	// 141	- APPLY_RESIST_WOLFMAN,	95
	{ POINT_RESIST_CLAW,			},	// 142	- APPLY_RESIST_CLAW,	96
	{ POINT_ACCEDRAIN_RATE,			},	// 143	- APPLY_ACCEDRAIN_RATE,	97
	{ POINT_RESIST_MAGIC_REDUCTION,	},	// 144	- APPLY_RESIST_MAGIC_REDUCTION,	98
	{ POINT_ENCHANT_ELECT,	},	// 
	{ POINT_ENCHANT_FIRE,	},	// 
	{ POINT_ENCHANT_ICE,	},	// 
	{ POINT_ENCHANT_WIND,	},	// 
	{ POINT_ENCHANT_EARTH,	},	// 
	{ POINT_ENCHANT_DARK,	},	// 
	{ POINT_ATTBONUS_CZ,	},	// 
	{ POINT_ATTBONUS_SWORD,	},	// 
	{ POINT_ATTBONUS_TWOHAND,	},	// 
	{ POINT_ATTBONUS_DAGGER,	},	// 
	{ POINT_ATTBONUS_BELL,	},	// 
	{ POINT_ATTBONUS_FAN,	},	// 
	{ POINT_ATTBONUS_BOW,	},	// 
	{ POINT_ATTBONUS_CLAW,	},	// 
	{ POINT_RESIST_HUMAN,	},	// 
	{ POINT_RESIST_MOUNT_FALL,	},	// 
	{ POINT_RESIST_FIST,	},	// 
	{ POINT_MOUNT, },	// 
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
	{ POINT_SKILL_DAMAGE_SAMYEON, }, // APPLY_SKILL_DAMAGE_SAMYEON 119
	{ POINT_SKILL_DAMAGE_TANHWAN, }, // APPLY_SKILL_DAMAGE_TANHWAN 120
	{ POINT_SKILL_DAMAGE_PALBANG, }, // APPLY_SKILL_DAMAGE_PALBANG 121
	{ POINT_SKILL_DAMAGE_GIGONGCHAM, }, // APPLY_SKILL_DAMAGE_GIGONGCHAM 122
	{ POINT_SKILL_DAMAGE_GYOKSAN, }, // APPLY_SKILL_DAMAGE_GYOKSAN 123
	{ POINT_SKILL_DAMAGE_GEOMPUNG, }, // APPLY_SKILL_DAMAGE_GEOMPUNG 124
	{ POINT_SKILL_DAMAGE_AMSEOP, }, // APPLY_SKILL_DAMAGE_AMSEOP 125
	{ POINT_SKILL_DAMAGE_GUNGSIN, }, // APPLY_SKILL_DAMAGE_GUNGSIN 126
	{ POINT_SKILL_DAMAGE_CHARYUN, }, // APPLY_SKILL_DAMAGE_CHARYUN 127
	{ POINT_SKILL_DAMAGE_SANGONG, }, // APPLY_SKILL_DAMAGE_SANGONG 128
	{ POINT_SKILL_DAMAGE_YEONSA, }, // APPLY_SKILL_DAMAGE_YEONSA 129
	{ POINT_SKILL_DAMAGE_KWANKYEOK, }, // APPLY_SKILL_DAMAGE_KWANKYEOK 130
	{ POINT_SKILL_DAMAGE_GIGUNG, }, // APPLY_SKILL_DAMAGE_GIGUNG 131
	{ POINT_SKILL_DAMAGE_HWAJO, }, // APPLY_SKILL_DAMAGE_HWAJO 132
	{ POINT_SKILL_DAMAGE_SWAERYUNG, }, // APPLY_SKILL_DAMAGE_SWAERYUNG 133
	{ POINT_SKILL_DAMAGE_YONGKWON, }, // APPLY_SKILL_DAMAGE_YONGKWON 134
	{ POINT_SKILL_DAMAGE_PABEOB, }, // APPLY_SKILL_DAMAGE_PABEOB 135
	{ POINT_SKILL_DAMAGE_MARYUNG, }, // APPLY_SKILL_DAMAGE_MARYUNG 136
	{ POINT_SKILL_DAMAGE_HWAYEOMPOK, }, // APPLY_SKILL_DAMAGE_HWAYEOMPOK 137
	{ POINT_SKILL_DAMAGE_MAHWAN, }, // APPLY_SKILL_DAMAGE_MAHWAN 138
	{ POINT_SKILL_DAMAGE_BIPABU, }, // APPLY_SKILL_DAMAGE_BIPABU 139
	{ POINT_SKILL_DAMAGE_YONGBI, }, // APPLY_SKILL_DAMAGE_YONGBI 140
	{ POINT_SKILL_DAMAGE_PAERYONG, }, // APPLY_SKILL_DAMAGE_PAERYONG 141
	{ POINT_SKILL_DAMAGE_NOEJEON, }, // APPLY_SKILL_DAMAGE_NOEJEON 142
	{ POINT_SKILL_DAMAGE_BYEURAK, }, // APPLY_SKILL_DAMAGE_BYEURAK 143
	{ POINT_SKILL_DAMAGE_CHAIN, }, // APPLY_SKILL_DAMAGE_CHAIN 144
	{ POINT_SKILL_DAMAGE_CHAYEOL, }, // APPLY_SKILL_DAMAGE_CHAYEOL 145
	{ POINT_SKILL_DAMAGE_SALPOONG, }, // APPLY_SKILL_DAMAGE_SALPOONG 146
	{ POINT_SKILL_DAMAGE_GONGDAB, }, // APPLY_SKILL_DAMAGE_GONGDAB 147
	{ POINT_SKILL_DAMAGE_PASWAE, }, // APPLY_SKILL_DAMAGE_PASWAE 148
	{ POINT_NORMAL_HIT_DEFEND_BONUS_BOSS_OR_MORE, }, // APPLY_NORMAL_HIT_DEFEND_BONUS_BOSS_OR_MORE 149
	{ POINT_SKILL_DEFEND_BONUS_BOSS_OR_MORE, }, // APPLY_SKILL_DEFEND_BONUS_BOSS_OR_MORE 150
	{ POINT_NORMAL_HIT_DAMAGE_BONUS_BOSS_OR_MORE, }, // APPLY_NORMAL_HIT_DAMAGE_BONUS_BOSS_OR_MORE 151
	{ POINT_SKILL_DAMAGE_BONUS_BOSS_OR_MORE, }, // APPLY_SKILL_DAMAGE_BONUS_BOSS_OR_MORE 152
	{ POINT_HIT_BUFF_ENCHANT_FIRE, }, // APPLY_HIT_BUFF_ENCHANT_FIRE 153
	{ POINT_HIT_BUFF_ENCHANT_ICE, }, // APPLY_HIT_BUFF_ENCHANT_ICE 154
	{ POINT_HIT_BUFF_ENCHANT_ELEC, }, // APPLY_HIT_BUFF_ENCHANT_ELEC 155
	{ POINT_HIT_BUFF_ENCHANT_WIND, }, // APPLY_HIT_BUFF_ENCHANT_WIND 156
	{ POINT_HIT_BUFF_ENCHANT_DARK, }, // APPLY_HIT_BUFF_ENCHANT_DARK 157
	{ POINT_HIT_BUFF_ENCHANT_EARTH, }, // APPLY_HIT_BUFF_ENCHANT_EARTH 158
	{ POINT_HIT_BUFF_RESIST_FIRE, }, // APPLY_HIT_BUFF_RESIST_FIRE 159
	{ POINT_HIT_BUFF_RESIST_ICE, }, // APPLY_HIT_BUFF_RESIST_ICE 160
	{ POINT_HIT_BUFF_RESIST_ELEC, }, // APPLY_HIT_BUFF_RESIST_ELEC 161
	{ POINT_HIT_BUFF_RESIST_WIND, }, // APPLY_HIT_BUFF_RESIST_WIND 162
	{ POINT_HIT_BUFF_RESIST_DARK, }, // APPLY_HIT_BUFF_RESIST_DARK 163
	{ POINT_HIT_BUFF_RESIST_EARTH, }, // APPLY_HIT_BUFF_RESIST_EARTH 164
	{ POINT_USE_SKILL_CHEONGRANG_MOV_SPEED, }, // APPLY_USE_SKILL_CHEONGRANG_MOV_SPEED 165
	{ POINT_USE_SKILL_CHEONGRANG_CASTING_SPEED, }, // APPLY_USE_SKILL_CHEONGRANG_CASTING_SPEED 166
	{ POINT_USE_SKILL_CHAYEOL_CRITICAL_PCT, }, // APPLY_USE_SKILL_CHAYEOL_CRITICAL_PCT 167
	{ POINT_USE_SKILL_SANGONG_ATT_GRADE_BONUS, }, // APPLY_USE_SKILL_SANGONG_ATT_GRADE_BONUS 168
	{ POINT_USE_SKILL_GIGUNG_ATT_GRADE_BONUS, }, // APPLY_USE_SKILL_GIGUNG_ATT_GRADE_BONUS 169
	{ POINT_USE_SKILL_JEOKRANG_DEF_BONUS, }, // APPLY_USE_SKILL_JEOKRANG_DEF_BONUS 170
	{ POINT_USE_SKILL_GWIGEOM_DEF_BONUS, }, // APPLY_USE_SKILL_GWIGEOM_DEF_BONUS 171
	{ POINT_USE_SKILL_TERROR_ATT_GRADE_BONUS, }, // APPLY_USE_SKILL_TERROR_ATT_GRADE_BONUS 172
	{ POINT_USE_SKILL_MUYEONG_ATT_GRADE_BONUS, }, // APPLY_USE_SKILL_MUYEONG_ATT_GRADE_BONUS 173
	{ POINT_USE_SKILL_MANASHILED_CASTING_SPEED, }, // APPLY_USE_SKILL_MANASHILED_CASTING_SPEED 174
	{ POINT_USE_SKILL_HOSIN_DEF_BONUS, }, // APPLY_USE_SKILL_HOSIN_DEF_BONUS 175
	{ POINT_USE_SKILL_GICHEON_ATT_GRADE_BONUS, }, // APPLY_USE_SKILL_GICHEON_ATT_GRADE_BONUS 176
	{ POINT_USE_SKILL_JEONGEOP_ATT_GRADE_BONUS, }, // APPLY_USE_SKILL_JEONGEOP_ATT_GRADE_BONUS 177
	{ POINT_USE_SKILL_JEUNGRYEOK_DEF_BONUS, }, // APPLY_USE_SKILL_JEUNGRYEOK_DEF_BONUS 178
	{ POINT_USE_SKILL_GIHYEOL_ATT_GRADE_BONUS, }, // APPLY_USE_SKILL_GIHYEOL_ATT_GRADE_BONUS 179
	{ POINT_USE_SKILL_CHUNKEON_CASTING_SPEED, }, // APPLY_USE_SKILL_CHUNKEON_CASTING_SPEED 180
	{ POINT_USE_SKILL_NOEGEOM_ATT_GRADE_BONUS, }, // APPLY_USE_SKILL_NOEGEOM_ATT_GRADE_BONUS 181
	{ POINT_SKILL_DURATION_INCREASE_EUNHYUNG, }, // APPLY_SKILL_DURATION_INCREASE_EUNHYUNG 182
	{ POINT_SKILL_DURATION_INCREASE_GYEONGGONG, }, // APPLY_SKILL_DURATION_INCREASE_GYEONGGONG 183
	{ POINT_SKILL_DURATION_INCREASE_GEOMKYUNG, }, // APPLY_SKILL_DURATION_INCREASE_GEOMKYUNG 184
	{ POINT_SKILL_DURATION_INCREASE_JEOKRANG, }, // APPLY_SKILL_DURATION_INCREASE_JEOKRANG 185
	{ POINT_USE_SKILL_PALBANG_HP_ABSORB, }, // APPLY_USE_SKILL_PALBANG_HP_ABSORB 186
	{ POINT_USE_SKILL_AMSEOP_HP_ABSORB, }, // APPLY_USE_SKILL_AMSEOP_HP_ABSORB 187
	{ POINT_USE_SKILL_YEONSA_HP_ABSORB, }, // APPLY_USE_SKILL_YEONSA_HP_ABSORB 188
	{ POINT_USE_SKILL_YONGBI_HP_ABSORB, }, // APPLY_USE_SKILL_YONGBI_HP_ABSORB 189
	{ POINT_USE_SKILL_CHAIN_HP_ABSORB, }, // APPLY_USE_SKILL_CHAIN_HP_ABSORB 190
	{ POINT_USE_SKILL_PASWAE_SP_ABSORB, }, // APPLY_USE_SKILL_PASWAE_SP_ABSORB 191
	{ POINT_USE_SKILL_GIGONGCHAM_STUN, }, // APPLY_USE_SKILL_GIGONGCHAM_STUN 192
	{ POINT_USE_SKILL_CHARYUN_STUN, }, // APPLY_USE_SKILL_CHARYUN_STUN 193
	{ POINT_USE_SKILL_PABEOB_STUN, }, // APPLY_USE_SKILL_PABEOB_STUN 194
	{ POINT_USE_SKILL_MAHWAN_STUN, }, // APPLY_USE_SKILL_MAHWAN_STUN 195
	{ POINT_USE_SKILL_GONGDAB_STUN, }, // APPLY_USE_SKILL_GONGDAB_STUN 196
	{ POINT_USE_SKILL_SAMYEON_STUN, }, // APPLY_USE_SKILL_SAMYEON_STUN 197
	{ POINT_USE_SKILL_GYOKSAN_KNOCKBACK, }, // APPLY_USE_SKILL_GYOKSAN_KNOCKBACK 198
	{ POINT_USE_SKILL_SEOMJEON_KNOCKBACK, }, // APPLY_USE_SKILL_SEOMJEON_KNOCKBACK 199
	{ POINT_USE_SKILL_SWAERYUNG_KNOCKBACK, }, // APPLY_USE_SKILL_SWAERYUNG_KNOCKBACK 200
	{ POINT_USE_SKILL_HWAYEOMPOK_KNOCKBACK, }, // APPLY_USE_SKILL_HWAYEOMPOK_KNOCKBACK 201
	{ POINT_USE_SKILL_GONGDAB_KNOCKBACK, }, // APPLY_USE_SKILL_GONGDAB_KNOCKBACK 202
	{ POINT_USE_SKILL_KWANKYEOK_KNOCKBACK, }, // APPLY_USE_SKILL_KWANKYEOK_KNOCKBACK 203
	{ POINT_USE_SKILL_SAMYEON_NEXT_COOLTIME_DECREASE_10PER, }, // APPLY_USE_SKILL_SAMYEON_NEXT_COOLTIME_DECREASE_10PER 204
	{ POINT_USE_SKILL_GEOMPUNG_NEXT_COOLTIME_DECREASE_10PER, }, // APPLY_USE_SKILL_GEOMPUNG_NEXT_COOLTIME_DECREASE_10PER 205
	{ POINT_USE_SKILL_GUNGSIN_NEXT_COOLTIME_DECREASE_10PER, }, // APPLY_USE_SKILL_GUNGSIN_NEXT_COOLTIME_DECREASE_10PER 206
	{ POINT_USE_SKILL_KWANKYEOK_NEXT_COOLTIME_DECREASE_10PER, }, // APPLY_USE_SKILL_KWANKYEOK_NEXT_COOLTIME_DECREASE_10PER 207
	{ POINT_USE_SKILL_YONGKWON_NEXT_COOLTIME_DECREASE_10PER, }, // APPLY_USE_SKILL_YONGKWON_NEXT_COOLTIME_DECREASE_10PER 208
	{ POINT_USE_SKILL_MARYUNG_NEXT_COOLTIME_DECREASE_10PER, }, // APPLY_USE_SKILL_MARYUNG_NEXT_COOLTIME_DECREASE_10PER 209
	{ POINT_USE_SKILL_BIPABU_NEXT_COOLTIME_DECREASE_10PER, }, // APPLY_USE_SKILL_BIPABU_NEXT_COOLTIME_DECREASE_10PER 210
	{ POINT_USE_SKILL_NOEJEON_NEXT_COOLTIME_DECREASE_10PER, }, // APPLY_USE_SKILL_NOEJEON_NEXT_COOLTIME_DECREASE_10PER 211
	{ POINT_USE_SKILL_SALPOONG_NEXT_COOLTIME_DECREASE_10PER, }, // APPLY_USE_SKILL_SALPOONG_NEXT_COOLTIME_DECREASE_10PER 212
	{ POINT_USE_SKILL_PASWAE_NEXT_COOLTIME_DECREASE_10PER, }, // APPLY_USE_SKILL_PASWAE_NEXT_COOLTIME_DECREASE_10PER 213
	{ POINT_ATTBONUS_STONE, }, // APPLY_ATTBONUS_STONE 214
	{ POINT_DAMAGE_HP_RECOVERY, }, // APPLY_DAMAGE_HP_RECOVERY 215
	{ POINT_DAMAGE_SP_RECOVERY, }, // APPLY_DAMAGE_SP_RECOVERY 216
	{ POINT_ALIGNMENT_DAMAGE_BONUS, }, // APPLY_ALIGNMENT_DAMAGE_BONUS 217
	{ POINT_NORMAL_DAMAGE_GUARD, }, // APPLY_NORMAL_DAMAGE_GUARD 218
	{ POINT_MORE_THEN_HP90_DAMAGE_REDUCE, }, // APPLY_MORE_THEN_HP90_DAMAGE_REDUCE 219
	{ POINT_USE_SKILL_TUSOK_HP_ABSORB, }, // APPLY_USE_SKILL_TUSOK_HP_ABSORB 220
	{ POINT_USE_SKILL_PAERYONG_HP_ABSORB, }, // APPLY_USE_SKILL_PAERYONG_HP_ABSORB 221
	{ POINT_USE_SKILL_BYEURAK_HP_ABSORB, }, // APPLY_USE_SKILL_BYEURAK_HP_ABSORB 222
	{ POINT_FIRST_ATTRIBUTE_BONUS, }, // APPLY_FIRST_ATTRIBUTE_BONUS 223
	{ POINT_SECOND_ATTRIBUTE_BONUS, }, // APPLY_SECOND_ATTRIBUTE_BONUS 224
	{ POINT_THIRD_ATTRIBUTE_BONUS, }, // APPLY_THIRD_ATTRIBUTE_BONUS 225
	{ POINT_FOURTH_ATTRIBUTE_BONUS, }, // APPLY_FOURTH_ATTRIBUTE_BONUS 226
	{ POINT_FIFTH_ATTRIBUTE_BONUS, }, // APPLY_FIFTH_ATTRIBUTE_BONUS 227
	{ POINT_USE_SKILL_SAMYEON_NEXT_COOLTIME_DECREASE_20PER, }, // APPLY_USE_SKILL_SAMYEON_NEXT_COOLTIME_DECREASE_20PER 228
	{ POINT_USE_SKILL_GEOMPUNG_NEXT_COOLTIME_DECREASE_20PER, }, // APPLY_USE_SKILL_GEOMPUNG_NEXT_COOLTIME_DECREASE_20PER 229
	{ POINT_USE_SKILL_GUNGSIN_NEXT_COOLTIME_DECREASE_20PER, }, // APPLY_USE_SKILL_GUNGSIN_NEXT_COOLTIME_DECREASE_20PER 230
	{ POINT_USE_SKILL_KWANKYEOK_NEXT_COOLTIME_DECREASE_20PER, }, // APPLY_USE_SKILL_KWANKYEOK_NEXT_COOLTIME_DECREASE_20PER 231
	{ POINT_USE_SKILL_YONGKWON_NEXT_COOLTIME_DECREASE_20PER, }, // APPLY_USE_SKILL_YONGKWON_NEXT_COOLTIME_DECREASE_20PER 232
	{ POINT_USE_SKILL_MARYUNG_NEXT_COOLTIME_DECREASE_20PER, }, // APPLY_USE_SKILL_MARYUNG_NEXT_COOLTIME_DECREASE_20PER 233
	{ POINT_USE_SKILL_BIPABU_NEXT_COOLTIME_DECREASE_20PER, }, // APPLY_USE_SKILL_BIPABU_NEXT_COOLTIME_DECREASE_20PER 234
	{ POINT_USE_SKILL_NOEJEON_NEXT_COOLTIME_DECREASE_20PER, }, // APPLY_USE_SKILL_NOEJEON_NEXT_COOLTIME_DECREASE_20PER 235
	{ POINT_USE_SKILL_SALPOONG_NEXT_COOLTIME_DECREASE_20PER, }, // APPLY_USE_SKILL_SALPOONG_NEXT_COOLTIME_DECREASE_20PER 236
	{ POINT_USE_SKILL_PASWAE_NEXT_COOLTIME_DECREASE_20PER, }, // APPLY_USE_SKILL_PASWAE_NEXT_COOLTIME_DECREASE_20PER 237
	{ POINT_USE_SKILL_CHAYEOL_HP_ABSORB, }, // APPLY_USE_SKILL_CHAYEOL_HP_ABSORB 238
	{ POINT_SUNGMA_STR, }, //  239
	{ POINT_SUNGMA_HP, }, //  240
	{ POINT_SUNGMA_MOVE, }, //  241
	{ POINT_SUNGMA_IMMUNE, }, //  242
	{ POINT_HIT_PCT, }, //  243
	{ POINT_RANDOM, }, //  244
	{ POINT_ATTBONUS_PER_HUMAN, }, // APPLY_ATTBONUS_PER_HUMAN 245
	{ POINT_ATTBONUS_PER_ANIMAL, }, // APPLY_ATTBONUS_PER_ANIMAL 246
	{ POINT_ATTBONUS_PER_ORC, }, // APPLY_ATTBONUS_PER_ORC 247
	{ POINT_ATTBONUS_PER_MILGYO, }, // APPLY_ATTBONUS_PER_MILGYO 248
	{ POINT_ATTBONUS_PER_UNDEAD, }, // APPLY_ATTBONUS_PER_UNDEAD 249
	{ POINT_ATTBONUS_PER_DEVIL, }, // APPLY_ATTBONUS_PER_DEVIL 250
	{ POINT_ENCHANT_PER_ELECT, }, // APPLY_ENCHANT_PER_ELECT 251
	{ POINT_ENCHANT_PER_FIRE, }, // APPLY_ENCHANT_PER_FIRE 252
	{ POINT_ENCHANT_PER_ICE, }, // APPLY_ENCHANT_PER_ICE 253
	{ POINT_ENCHANT_PER_WIND, }, // APPLY_ENCHANT_PER_WIND 254
	{ POINT_ENCHANT_PER_EARTH, }, // APPLY_ENCHANT_PER_EARTH 255
	{ POINT_ENCHANT_PER_DARK, }, // APPLY_ENCHANT_PER_DARK 256
	{ POINT_ATTBONUS_PER_CZ, }, // APPLY_ATTBONUS_PER_CZ 257
	{ POINT_ATTBONUS_PER_INSECT, }, // APPLY_ATTBONUS_PER_INSECT 258
	{ POINT_ATTBONUS_PER_DESERT, }, // APPLY_ATTBONUS_PER_DESERT 259
	{ POINT_ATTBONUS_PER_STONE, }, // APPLY_ATTBONUS_PER_STONE 260
	{ POINT_ATTBONUS_PER_MONSTER, }, // APPLY_ATTBONUS_PER_MONSTER 261
	{ POINT_RESIST_PER_HUMAN, }, // APPLY_RESIST_PER_HUMAN 262
	{ POINT_RESIST_PER_ICE, }, // APPLY_RESIST_PER_ICE 263
	{ POINT_RESIST_PER_DARK, }, // APPLY_RESIST_PER_DARK 264
	{ POINT_RESIST_PER_EARTH, }, // APPLY_RESIST_PER_EARTH 265
	{ POINT_RESIST_PER_FIRE, }, // APPLY_RESIST_PER_FIRE 266
	{ POINT_RESIST_PER_ELEC, }, // APPLY_RESIST_PER_ELEC 267
	{ POINT_RESIST_PER_MAGIC, }, // APPLY_RESIST_PER_MAGIC 268
	{ POINT_RESIST_PER_WIND, }, // APPLY_RESIST_PER_WIND 269
	{ POINT_RESIST_PER_WIND, }, // 
	{ POINT_HIT_BUFF_SUNGMA_STR, }, // 
	{ POINT_HIT_BUFF_SUNGMA_MOVE, }, // 
	{ POINT_HIT_BUFF_SUNGMA_HP, }, // 
	{ POINT_HIT_BUFF_SUNGMA_IMMUNE, }, // 
	{ POINT_MOUNT_MELEE_MAGIC_ATTBONUS_PER, }, // 
	{ POINT_DISMOUNT_MOVE_SPEED_BONUS_PER, }, // 
	{ POINT_HIT_AUTO_HP_RECOVERY, }, // 
	{ POINT_HIT_AUTO_SP_RECOVERY, }, // 
	{ POINT_USE_SKILL_COOLTIME_DECREASE_ALL, }, // 
	{ POINT_HIT_STONE_ATTBONUS_STONE, }, // 
	{ POINT_HIT_STONE_DEF_GRADE_BONUS, }, // 
	{ POINT_KILL_BOSS_ITEM_BONUS, }, // 
	{ POINT_MOB_HIT_MOB_AGGRESSIVE, }, // 
	{ POINT_NO_DEATH_AND_HP_RECOVERY30, }, // 
	{ POINT_AUTO_PICKUP, }, // 
	{ POINT_MOUNT_NO_KNOCKBACK, }, // 
	{ POINT_SUNGMA_PER_STR, }, // 
	{ POINT_SUNGMA_PER_HP, }, // 
	{ POINT_SUNGMA_PER_MOVE, }, // 
	{ POINT_SUNGMA_PER_IMMUNE, }, // 
	{ POINT_IMMUNE_POISON100, }, // 
	{ POINT_IMMUNE_BLEEDING100, }, // 
#endif
};

uint16_t ApplyTypeToPointType(uint16_t wApplyType)	//@fixme436
{
	if (wApplyType >= CItemData::MAX_APPLY_NUM)
		return POINT_NONE;
	else
		return aApplyInfo[wApplyType].wPointType;
}
#endif

#ifdef ENABLE_AURA_SYSTEM
static int s_aiAuraRefineInfo[CItemData::AURA_GRADE_MAX_NUM][AURA_REFINE_INFO_MAX] = {
	{1,   1,  49,  1000, 30617, 10,  5000000, 100},
	{2,  50,  99,  2000, 31136, 10,  5000000, 100},
	{3, 100, 149,  4000, 31137, 10,  5000000, 100},
	{4, 150, 199,  8000, 31138, 10,  8000000, 100},
	{5, 200, 249, 16000, 31138, 20, 10000000, 100},
	{6, 250, 250,     0,     0,  0,        0,   0},
	{0,   0,   0,     0,     0,  0,        0,   0}
};

int* GetAuraRefineInfo(uint8_t bLevel)
{
	if (bLevel > 250)
		return nullptr;

	for (int i = 0; i < CItemData::AURA_GRADE_MAX_NUM + 1; ++i)
	{
		if (bLevel >= s_aiAuraRefineInfo[i][AURA_REFINE_INFO_LEVEL_MIN] && bLevel <= s_aiAuraRefineInfo[i][AURA_REFINE_INFO_LEVEL_MAX])
			return s_aiAuraRefineInfo[i];
	}

	return nullptr;
}
#endif

#define __UNKNOWN__
#ifdef ENABLE_FLOWER_EVENT
typedef struct SPointInfo
{
	//@fixme436
	uint16_t wApplyType; // POINT -> APPLY
} TPointInfo;

const TPointInfo aPointInfo[POINT_MAX_NUM] =
{
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_MAX_HP,
	CItemData::APPLY_NONE,
	CItemData::APPLY_MAX_SP,
	CItemData::APPLY_NONE,
	CItemData::APPLY_MAX_STAMINA,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_ATT_SPEED,
	CItemData::APPLY_NONE,
	CItemData::APPLY_MOV_SPEED,
	CItemData::APPLY_DEF_GRADE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_MAGIC_ATT_GRADE,
	CItemData::APPLY_MAGIC_DEF_GRADE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_SKILL,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_HP_REGEN,
	CItemData::APPLY_SP_REGEN,
	CItemData::APPLY_BOW_DISTANCE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_POISON_PCT,
	CItemData::APPLY_STUN_PCT,
	CItemData::APPLY_SLOW_PCT,
	CItemData::APPLY_CRITICAL_PCT,
	CItemData::APPLY_PENETRATE_PCT,
	CItemData::APPLY_CURSE_PCT,
	CItemData::APPLY_ATTBONUS_HUMAN,
	CItemData::APPLY_ATTBONUS_ANIMAL,
	CItemData::APPLY_ATTBONUS_ORC,
	CItemData::APPLY_ATTBONUS_MILGYO,
	CItemData::APPLY_ATTBONUS_UNDEAD,
	CItemData::APPLY_ATTBONUS_DEVIL,
	CItemData::APPLY_ATTBONUS_INSECT,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_ATTBONUS_DESERT,
	CItemData::APPLY_ATT_BONUS_TO_MONSTER,
	CItemData::APPLY_ATT_BONUS_TO_WARRIOR,
	CItemData::APPLY_ATT_BONUS_TO_ASSASSIN,
	CItemData::APPLY_ATT_BONUS_TO_SURA,
	CItemData::APPLY_ATT_BONUS_TO_SHAMAN,
	CItemData::APPLY_NONE,
	CItemData::APPLY_RESIST_WARRIOR,
	CItemData::APPLY_RESIST_ASSASSIN,
	CItemData::APPLY_RESIST_SURA,
	CItemData::APPLY_RESIST_SHAMAN,
	CItemData::APPLY_STEAL_HP,
	CItemData::APPLY_STEAL_SP,
	CItemData::APPLY_MANA_BURN_PCT,
	CItemData::APPLY_DAMAGE_SP_RECOVER,
	CItemData::APPLY_BLOCK,
	CItemData::APPLY_DODGE,
	CItemData::APPLY_RESIST_SWORD,
	CItemData::APPLY_RESIST_TWOHAND,
	CItemData::APPLY_RESIST_DAGGER,
	CItemData::APPLY_RESIST_BELL,
	CItemData::APPLY_RESIST_FAN,
	CItemData::APPLY_RESIST_BOW,
	CItemData::APPLY_RESIST_FIRE,
	CItemData::APPLY_RESIST_ELEC,
	CItemData::APPLY_RESIST_MAGIC,
	CItemData::APPLY_RESIST_WIND,
	CItemData::APPLY_REFLECT_MELEE,
	CItemData::APPLY_REFLECT_CURSE,
	CItemData::APPLY_POISON_REDUCE,
	CItemData::APPLY_KILL_SP_RECOVER,
	CItemData::APPLY_EXP_DOUBLE_BONUS,
	CItemData::APPLY_GOLD_DOUBLE_BONUS,
	CItemData::APPLY_ITEM_DROP_BONUS,
	CItemData::APPLY_POTION_BONUS,
	CItemData::APPLY_KILL_HP_RECOVER,
	CItemData::APPLY_IMMUNE_STUN,
	CItemData::APPLY_IMMUNE_SLOW,
	CItemData::APPLY_IMMUNE_FALL,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_MALL_ATTBONUS,
	CItemData::APPLY_MALL_DEFBONUS,
	CItemData::APPLY_ATT_GRADE_BONUS,
	CItemData::APPLY_DEF_GRADE_BONUS,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_MOUNT,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_MALL_ATTBONUS,
	CItemData::APPLY_MALL_DEFBONUS,
	CItemData::APPLY_MALL_EXPBONUS,
	CItemData::APPLY_MALL_ITEMBONUS,
	CItemData::APPLY_MALL_GOLDBONUS,
	CItemData::APPLY_MAX_HP_PCT,
	CItemData::APPLY_MAX_SP_PCT,
	CItemData::APPLY_SKILL_DAMAGE_BONUS,
	CItemData::APPLY_NORMAL_HIT_DAMAGE_BONUS,
	CItemData::APPLY_SKILL_DEFEND_BONUS,
	CItemData::APPLY_NORMAL_HIT_DEFEND_BONUS,
	CItemData::APPLY_PC_BANG_EXP_BONUS,
	CItemData::APPLY_PC_BANG_DROP_BONUS,
	CItemData::APPLY_NONE,
	CItemData::APPLY_ENERGY,
	CItemData::APPLY_NONE,
	CItemData::APPLY_COSTUME_ATTR_BONUS,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_RESIST_ICE,
	CItemData::APPLY_RESIST_EARTH,
	CItemData::APPLY_RESIST_DARK,
	CItemData::APPLY_CRITICAL_PCT,
	CItemData::APPLY_ANTI_PENETRATE_PCT,
#ifdef ENABLE_WOLFMAN_CHARACTER
	CItemData::APPLY_BLEEDING_REDUCE,
	CItemData::APPLY_BLEEDING_PCT,
	CItemData::APPLY_ATT_BONUS_TO_WOLFMAN,
	CItemData::APPLY_RESIST_WOLFMAN,
	CItemData::APPLY_RESIST_CLAW,
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	CItemData::APPLY_ACCEDRAIN_RATE,
#endif
#ifdef ENABLE_MAGIC_REDUCTION_SYSTEM
	CItemData::APPLY_RESIST_MAGIC_REDUCTION,
#endif
#ifdef ENABLE_CHEQUE_SYSTEM
	CItemData::APPLY_NONE,
#endif
#ifdef ENABLE_BATTLE_FIELD
	CItemData::APPLY_NONE,
#endif
#ifdef ENABLE_ELEMENT_ADD
	CItemData::APPLY_RESIST_HUMAN,
	CItemData::APPLY_ENCHANT_ELECT,
	CItemData::APPLY_ENCHANT_FIRE,
	CItemData::APPLY_ENCHANT_ICE,
	CItemData::APPLY_ENCHANT_WIND,
	CItemData::APPLY_ENCHANT_EARTH,
	CItemData::APPLY_ENCHANT_DARK,
	CItemData::APPLY_ATTBONUS_CZ,
#endif
#ifdef ENABLE_12ZI
	CItemData::APPLY_NONE,
#endif
#ifdef ENABLE_GEM_SYSTEM
	CItemData::APPLY_NONE,
#endif
#ifdef ENABLE_PENDANT
	CItemData::APPLY_ATTBONUS_SWORD,
	CItemData::APPLY_ATTBONUS_TWOHAND,
	CItemData::APPLY_ATTBONUS_DAGGER,
	CItemData::APPLY_ATTBONUS_BELL,
	CItemData::APPLY_ATTBONUS_FAN,
	CItemData::APPLY_ATTBONUS_BOW,
#ifdef ENABLE_WOLFMAN_CHARACTER
	CItemData::APPLY_ATTBONUS_CLAW,
#endif
	CItemData::APPLY_RESIST_MOUNT_FALL,
#endif
	CItemData::APPLY_RESIST_FIST,
#ifdef __UNKNOWN__
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
#endif
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
	CItemData::APPLY_SKILL_DAMAGE_SAMYEON,
	CItemData::APPLY_SKILL_DAMAGE_TANHWAN,
	CItemData::APPLY_SKILL_DAMAGE_PALBANG,
	CItemData::APPLY_SKILL_DAMAGE_GIGONGCHAM,
	CItemData::APPLY_SKILL_DAMAGE_GYOKSAN,
	CItemData::APPLY_SKILL_DAMAGE_GEOMPUNG,
	CItemData::APPLY_SKILL_DAMAGE_AMSEOP,
	CItemData::APPLY_SKILL_DAMAGE_GUNGSIN,
	CItemData::APPLY_SKILL_DAMAGE_CHARYUN,
	CItemData::APPLY_SKILL_DAMAGE_SANGONG,
	CItemData::APPLY_SKILL_DAMAGE_YEONSA,
	CItemData::APPLY_SKILL_DAMAGE_KWANKYEOK,
	CItemData::APPLY_SKILL_DAMAGE_GIGUNG,
	CItemData::APPLY_SKILL_DAMAGE_HWAJO,
	CItemData::APPLY_SKILL_DAMAGE_SWAERYUNG,
	CItemData::APPLY_SKILL_DAMAGE_YONGKWON,
	CItemData::APPLY_SKILL_DAMAGE_PABEOB,
	CItemData::APPLY_SKILL_DAMAGE_MARYUNG,
	CItemData::APPLY_SKILL_DAMAGE_HWAYEOMPOK,
	CItemData::APPLY_SKILL_DAMAGE_MAHWAN,
	CItemData::APPLY_SKILL_DAMAGE_BIPABU,
	CItemData::APPLY_SKILL_DAMAGE_YONGBI,
	CItemData::APPLY_SKILL_DAMAGE_PAERYONG,
	CItemData::APPLY_SKILL_DAMAGE_NOEJEON,
	CItemData::APPLY_SKILL_DAMAGE_BYEURAK,
	CItemData::APPLY_SKILL_DAMAGE_CHAIN,
	CItemData::APPLY_SKILL_DAMAGE_CHAYEOL,
	CItemData::APPLY_SKILL_DAMAGE_SALPOONG,
	CItemData::APPLY_SKILL_DAMAGE_GONGDAB,
	CItemData::APPLY_SKILL_DAMAGE_PASWAE,
	CItemData::APPLY_SKILL_DEFEND_BONUS_BOSS_OR_MORE,
	CItemData::APPLY_NORMAL_HIT_DEFEND_BONUS_BOSS_OR_MORE,
	CItemData::APPLY_NORMAL_HIT_DAMAGE_BONUS_BOSS_OR_MORE,
	CItemData::APPLY_SKILL_DAMAGE_BONUS_BOSS_OR_MORE,
	CItemData::APPLY_HIT_BUFF_ENCHANT_FIRE,
	CItemData::APPLY_HIT_BUFF_ENCHANT_ICE,
	CItemData::APPLY_HIT_BUFF_ENCHANT_ELEC,
	CItemData::APPLY_HIT_BUFF_ENCHANT_WIND,
	CItemData::APPLY_HIT_BUFF_ENCHANT_DARK,
	CItemData::APPLY_HIT_BUFF_ENCHANT_EARTH,
	CItemData::APPLY_HIT_BUFF_RESIST_FIRE,
	CItemData::APPLY_HIT_BUFF_RESIST_ICE,
	CItemData::APPLY_HIT_BUFF_RESIST_ELEC,
	CItemData::APPLY_HIT_BUFF_RESIST_WIND,
	CItemData::APPLY_HIT_BUFF_RESIST_DARK,
	CItemData::APPLY_HIT_BUFF_RESIST_EARTH,
	CItemData::APPLY_USE_SKILL_CHEONGRANG_MOV_SPEED,
	CItemData::APPLY_USE_SKILL_CHEONGRANG_CASTING_SPEED,
	CItemData::APPLY_USE_SKILL_CHAYEOL_CRITICAL_PCT,
	CItemData::APPLY_USE_SKILL_SANGONG_ATT_GRADE_BONUS,
	CItemData::APPLY_USE_SKILL_GIGUNG_ATT_GRADE_BONUS,
	CItemData::APPLY_USE_SKILL_JEOKRANG_DEF_BONUS,
	CItemData::APPLY_USE_SKILL_GWIGEOM_DEF_BONUS,
	CItemData::APPLY_USE_SKILL_TERROR_ATT_GRADE_BONUS,
	CItemData::APPLY_USE_SKILL_MUYEONG_ATT_GRADE_BONUS,
	CItemData::APPLY_USE_SKILL_MANASHILED_CASTING_SPEED,
	CItemData::APPLY_USE_SKILL_HOSIN_DEF_BONUS,
	CItemData::APPLY_USE_SKILL_GICHEON_ATT_GRADE_BONUS,
	CItemData::APPLY_USE_SKILL_JEONGEOP_ATT_GRADE_BONUS,
	CItemData::APPLY_USE_SKILL_JEUNGRYEOK_DEF_BONUS,
	CItemData::APPLY_USE_SKILL_GIHYEOL_ATT_GRADE_BONUS,
	CItemData::APPLY_USE_SKILL_CHUNKEON_CASTING_SPEED,
	CItemData::APPLY_USE_SKILL_NOEGEOM_ATT_GRADE_BONUS,
	CItemData::APPLY_SKILL_DURATION_INCREASE_EUNHYUNG,
	CItemData::APPLY_SKILL_DURATION_INCREASE_GYEONGGONG,
	CItemData::APPLY_SKILL_DURATION_INCREASE_GEOMKYUNG,
	CItemData::APPLY_SKILL_DURATION_INCREASE_JEOKRANG,
	CItemData::APPLY_USE_SKILL_PALBANG_HP_ABSORB,
	CItemData::APPLY_USE_SKILL_AMSEOP_HP_ABSORB,
	CItemData::APPLY_USE_SKILL_YEONSA_HP_ABSORB,
	CItemData::APPLY_USE_SKILL_YONGBI_HP_ABSORB,
	CItemData::APPLY_USE_SKILL_CHAIN_HP_ABSORB,
	CItemData::APPLY_USE_SKILL_PASWAE_SP_ABSORB,
	CItemData::APPLY_USE_SKILL_GIGONGCHAM_STUN,
	CItemData::APPLY_USE_SKILL_CHARYUN_STUN,
	CItemData::APPLY_USE_SKILL_PABEOB_STUN,
	CItemData::APPLY_USE_SKILL_MAHWAN_STUN,
	CItemData::APPLY_USE_SKILL_GONGDAB_STUN,
	CItemData::APPLY_USE_SKILL_SAMYEON_STUN,
	CItemData::APPLY_USE_SKILL_GYOKSAN_KNOCKBACK,
	CItemData::APPLY_USE_SKILL_SEOMJEON_KNOCKBACK,
	CItemData::APPLY_USE_SKILL_SWAERYUNG_KNOCKBACK,
	CItemData::APPLY_USE_SKILL_HWAYEOMPOK_KNOCKBACK,
	CItemData::APPLY_USE_SKILL_GONGDAB_KNOCKBACK,
	CItemData::APPLY_USE_SKILL_KWANKYEOK_KNOCKBACK,
	CItemData::APPLY_USE_SKILL_SAMYEON_NEXT_COOLTIME_DECREASE_10PER,
	CItemData::APPLY_USE_SKILL_GEOMPUNG_NEXT_COOLTIME_DECREASE_10PER,
	CItemData::APPLY_USE_SKILL_GUNGSIN_NEXT_COOLTIME_DECREASE_10PER,
	CItemData::APPLY_USE_SKILL_KWANKYEOK_NEXT_COOLTIME_DECREASE_10PER,
	CItemData::APPLY_USE_SKILL_YONGKWON_NEXT_COOLTIME_DECREASE_10PER,
	CItemData::APPLY_USE_SKILL_MARYUNG_NEXT_COOLTIME_DECREASE_10PER,
	CItemData::APPLY_USE_SKILL_BIPABU_NEXT_COOLTIME_DECREASE_10PER,
	CItemData::APPLY_USE_SKILL_NOEJEON_NEXT_COOLTIME_DECREASE_10PER,
	CItemData::APPLY_USE_SKILL_SALPOONG_NEXT_COOLTIME_DECREASE_10PER,
	CItemData::APPLY_USE_SKILL_PASWAE_NEXT_COOLTIME_DECREASE_10PER,
	CItemData::APPLY_ATTBONUS_STONE,
	CItemData::APPLY_DAMAGE_HP_RECOVERY,
	CItemData::APPLY_DAMAGE_SP_RECOVERY,
	CItemData::APPLY_ALIGNMENT_DAMAGE_BONUS,
	CItemData::APPLY_NORMAL_DAMAGE_GUARD,
	CItemData::APPLY_MORE_THEN_HP90_DAMAGE_REDUCE,
	CItemData::APPLY_USE_SKILL_TUSOK_HP_ABSORB,
	CItemData::APPLY_USE_SKILL_PAERYONG_HP_ABSORB,
	CItemData::APPLY_USE_SKILL_BYEURAK_HP_ABSORB,
	CItemData::APPLY_FIRST_ATTRIBUTE_BONUS,
	CItemData::APPLY_SECOND_ATTRIBUTE_BONUS,
	CItemData::APPLY_THIRD_ATTRIBUTE_BONUS,
	CItemData::APPLY_FOURTH_ATTRIBUTE_BONUS,
	CItemData::APPLY_FIFTH_ATTRIBUTE_BONUS,
	CItemData::APPLY_USE_SKILL_SAMYEON_NEXT_COOLTIME_DECREASE_20PER,
	CItemData::APPLY_USE_SKILL_GEOMPUNG_NEXT_COOLTIME_DECREASE_20PER,
	CItemData::APPLY_USE_SKILL_GUNGSIN_NEXT_COOLTIME_DECREASE_20PER,
	CItemData::APPLY_USE_SKILL_KWANKYEOK_NEXT_COOLTIME_DECREASE_20PER,
	CItemData::APPLY_USE_SKILL_YONGKWON_NEXT_COOLTIME_DECREASE_20PER,
#endif
	CItemData::APPLY_NONE,
#ifdef ENABLE_MEDAL_OF_HONOR
	CItemData::APPLY_NONE,
#endif
#ifdef __UNKNOWN__
	CItemData::APPLY_NONE,
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	CItemData::APPLY_SUNGMA_STR,
	CItemData::APPLY_SUNGMA_HP,
	CItemData::APPLY_SUNGMA_MOVE,
	CItemData::APPLY_SUNGMA_IMMUNE,

	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
#endif
//#ifdef ENABLE_PRECISION_SKILL
	CItemData::APPLY_NONE,
//#endif
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
	CItemData::APPLY_ATTBONUS_PER_HUMAN,
	CItemData::APPLY_ATTBONUS_PER_ANIMAL,
	CItemData::APPLY_ATTBONUS_PER_ORC,
	CItemData::APPLY_ATTBONUS_PER_MILGYO,
	CItemData::APPLY_ATTBONUS_PER_UNDEAD,
	CItemData::APPLY_ATTBONUS_PER_DEVIL,
	CItemData::APPLY_ENCHANT_PER_ELECT,
	CItemData::APPLY_ENCHANT_PER_FIRE,
	CItemData::APPLY_ENCHANT_PER_ICE,
	CItemData::APPLY_ENCHANT_PER_WIND,
	CItemData::APPLY_ENCHANT_PER_EARTH,
	CItemData::APPLY_ENCHANT_PER_DARK,
	CItemData::APPLY_ATTBONUS_PER_CZ,
	CItemData::APPLY_ATTBONUS_PER_INSECT,
	CItemData::APPLY_ATTBONUS_PER_DESERT,
	CItemData::APPLY_ATTBONUS_PER_STONE,
	CItemData::APPLY_ATTBONUS_PER_MONSTER,
	CItemData::APPLY_RESIST_PER_HUMAN,
	CItemData::APPLY_RESIST_PER_ICE,
	CItemData::APPLY_RESIST_PER_DARK,
	CItemData::APPLY_RESIST_PER_EARTH,
	CItemData::APPLY_RESIST_PER_FIRE,
	CItemData::APPLY_RESIST_PER_ELEC,
	CItemData::APPLY_RESIST_PER_MAGIC,
	CItemData::APPLY_RESIST_PER_WIND,
#endif
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
	CItemData::APPLY_USE_SKILL_MARYUNG_NEXT_COOLTIME_DECREASE_20PER,
	CItemData::APPLY_USE_SKILL_BIPABU_NEXT_COOLTIME_DECREASE_20PER,
	CItemData::APPLY_USE_SKILL_NOEJEON_NEXT_COOLTIME_DECREASE_20PER,
	CItemData::APPLY_USE_SKILL_SALPOONG_NEXT_COOLTIME_DECREASE_20PER,
	CItemData::APPLY_USE_SKILL_PASWAE_NEXT_COOLTIME_DECREASE_20PER,
	CItemData::APPLY_USE_SKILL_CHAYEOL_HP_ABSORB,
#endif
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
	CItemData::APPLY_MOUNT_MELEE_MAGIC_ATTBONUS_PER,
	CItemData::APPLY_DISMOUNT_MOVE_SPEED_BONUS_PER,
	CItemData::APPLY_HIT_AUTO_HP_RECOVERY,
	CItemData::APPLY_HIT_AUTO_SP_RECOVERY,
	CItemData::APPLY_USE_SKILL_COOLTIME_DECREASE_ALL,
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	CItemData::APPLY_RANDOM,
	CItemData::APPLY_HIT_BUFF_SUNGMA_STR,
	CItemData::APPLY_HIT_BUFF_SUNGMA_MOVE,
	CItemData::APPLY_HIT_BUFF_SUNGMA_HP,
	CItemData::APPLY_HIT_BUFF_SUNGMA_IMMUNE,
	CItemData::APPLY_SUNGMA_PER_STR,
	CItemData::APPLY_SUNGMA_PER_HP,
	CItemData::APPLY_SUNGMA_PER_MOVE,
	CItemData::APPLY_SUNGMA_PER_IMMUNE,
#endif
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
	CItemData::APPLY_HIT_STONE_ATTBONUS_STONE,
	CItemData::APPLY_HIT_STONE_DEF_GRADE_BONUS,
	CItemData::APPLY_MONSTER_DEFEND_BONUS,
#endif
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
	CItemData::APPLY_KILL_BOSS_ITEM_BONUS,
	CItemData::APPLY_MOB_HIT_MOB_AGGRESSIVE,
	CItemData::APPLY_NO_DEATH_AND_HP_RECOVERY30,
	CItemData::APPLY_AUTO_PICKUP,
	CItemData::APPLY_MOUNT_NO_KNOCKBACK,
	CItemData::APPLY_IMMUNE_POISON100,
	CItemData::APPLY_IMMUNE_BLEEDING100,
	CItemData::APPLY_MONSTER_DEFEND_BONUS,
#endif
#ifdef __UNKNOWN__
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
#endif
#ifdef ENABLE_FLOWER_EVENT
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
	CItemData::APPLY_NONE,
#endif
};

uint16_t PointTypeToApplyType(uint16_t wPointType) // @fixme-WL001
{
	if (wPointType >= POINT_MAX_NUM)
		return CItemData::APPLY_NONE;
	else
		return aPointInfo[wPointType].wApplyType;
}
#endif