#include "StdAfx.h"
#ifdef ENABLE_DS_SET
#include "ItemUtil.h"

// <!> Check and update this to your applies <!>
TValueName c_aApplyTypeNames[] =
{
    { "STR",						CItemData::APPLY_STR						},
    { "DEX",						CItemData::APPLY_DEX						},
    { "CON",						CItemData::APPLY_CON						},
    { "INT",						CItemData::APPLY_INT						},
    { "MAX_HP",						CItemData::APPLY_MAX_HP					    },
    { "MAX_SP",						CItemData::APPLY_MAX_SP					    },
    { "MAX_STAMINA",				CItemData::APPLY_MAX_STAMINA				},
    { "POISON_REDUCE",				CItemData::APPLY_POISON_REDUCE				},
    { "EXP_DOUBLE_BONUS",			CItemData::APPLY_EXP_DOUBLE_BONUS			},
    { "GOLD_DOUBLE_BONUS",			CItemData::APPLY_GOLD_DOUBLE_BONUS			},
    { "ITEM_DROP_BONUS",			CItemData::APPLY_ITEM_DROP_BONUS			},
    { "HP_REGEN",					CItemData::APPLY_HP_REGEN					},
    { "SP_REGEN",					CItemData::APPLY_SP_REGEN					},
    { "ATTACK_SPEED",				CItemData::APPLY_ATT_SPEED					},
    { "MOVE_SPEED",					CItemData::APPLY_MOV_SPEED					},
    { "CAST_SPEED",					CItemData::APPLY_CAST_SPEED				    },
    { "ATT_BONUS",					CItemData::APPLY_ATT_GRADE_BONUS			},
    { "DEF_BONUS",					CItemData::APPLY_DEF_GRADE_BONUS			},
    { "MAGIC_ATT_GRADE",			CItemData::APPLY_MAGIC_ATT_GRADE			},
    { "MAGIC_DEF_GRADE",			CItemData::APPLY_MAGIC_DEF_GRADE			},
    { "SKILL",						CItemData::APPLY_SKILL						},
    { "ATTBONUS_ANIMAL",			CItemData::APPLY_ATTBONUS_ANIMAL			},
    { "ATTBONUS_UNDEAD",			CItemData::APPLY_ATTBONUS_UNDEAD			},
    { "ATTBONUS_DEVIL",				CItemData::APPLY_ATTBONUS_DEVIL			    },
    { "ATTBONUS_HUMAN",				CItemData::APPLY_ATTBONUS_HUMAN			    },
    { "ADD_BOW_DISTANCE",			CItemData::APPLY_BOW_DISTANCE				},
    { "DODGE",						CItemData::APPLY_DODGE						},
    { "BLOCK",						CItemData::APPLY_BLOCK						},
    { "RESIST_SWORD",				CItemData::APPLY_RESIST_SWORD				},
    { "RESIST_TWOHAND",				CItemData::APPLY_RESIST_TWOHAND			    },
    { "RESIST_DAGGER",				CItemData::APPLY_RESIST_DAGGER				},
    { "RESIST_BELL",				CItemData::APPLY_RESIST_BELL				},
    { "RESIST_FAN",					CItemData::APPLY_RESIST_FAN				    },
    { "RESIST_BOW",					CItemData::APPLY_RESIST_BOW				    },
    { "RESIST_FIRE",				CItemData::APPLY_RESIST_FIRE				},
    { "RESIST_ELEC",				CItemData::APPLY_RESIST_ELEC				},
    { "RESIST_MAGIC",				CItemData::APPLY_RESIST_MAGIC				},
    { "RESIST_WIND",				CItemData::APPLY_RESIST_WIND				},
    { "REFLECT_MELEE",				CItemData::APPLY_REFLECT_MELEE				},
    { "REFLECT_CURSE",				CItemData::APPLY_REFLECT_CURSE				},
    { "RESIST_ICE",					CItemData::APPLY_RESIST_ICE				    },
    { "RESIST_EARTH",				CItemData::APPLY_RESIST_EARTH				},
    { "RESIST_DARK",				CItemData::APPLY_RESIST_DARK				},
    { "RESIST_CRITICAL",			CItemData::APPLY_ANTI_CRITICAL_PCT			},
    { "RESIST_PENETRATE",			CItemData::APPLY_ANTI_PENETRATE_PCT		    },
    { "POISON",						CItemData::APPLY_POISON_PCT				    },
    { "SLOW",						CItemData::APPLY_SLOW_PCT					},
    { "STUN",						CItemData::APPLY_STUN_PCT					},
    { "STEAL_HP",					CItemData::APPLY_STEAL_HP					},
    { "STEAL_SP",					CItemData::APPLY_STEAL_SP					},
    { "MANA_BURN_PCT",				CItemData::APPLY_MANA_BURN_PCT				},
    { "CRITICAL",					CItemData::APPLY_CRITICAL_PCT				},
    { "PENETRATE",					CItemData::APPLY_PENETRATE_PCT				},
    { "KILL_SP_RECOVER",			CItemData::APPLY_KILL_SP_RECOVER			},
    { "KILL_HP_RECOVER",			CItemData::APPLY_KILL_HP_RECOVER			},
    { "PENETRATE_PCT",				CItemData::APPLY_PENETRATE_PCT				},
    { "CRITICAL_PCT",				CItemData::APPLY_CRITICAL_PCT				},
    { "POISON_PCT",					CItemData::APPLY_POISON_PCT				    },
    { "STUN_PCT",					CItemData::APPLY_STUN_PCT					},
    { "ATT_BONUS_TO_WARRIOR",		CItemData::APPLY_ATT_BONUS_TO_WARRIOR		},
    { "ATT_BONUS_TO_ASSASSIN",		CItemData::APPLY_ATT_BONUS_TO_ASSASSIN		},
    { "ATT_BONUS_TO_SURA",			CItemData::APPLY_ATT_BONUS_TO_SURA			},
    { "ATT_BONUS_TO_SHAMAN",		CItemData::APPLY_ATT_BONUS_TO_SHAMAN		},
    { "ATT_BONUS_TO_MONSTER",		CItemData::APPLY_ATT_BONUS_TO_MONSTER		},
    { "ATT_BONUS_TO_MOB",			CItemData::APPLY_ATT_BONUS_TO_MONSTER		},
    { "MALL_ATTBONUS",				CItemData::APPLY_MALL_ATTBONUS				},
    { "MALL_EXPBONUS",				CItemData::APPLY_MALL_EXPBONUS				},
    { "MALL_DEFBONUS",				CItemData::APPLY_MALL_DEFBONUS				},
    { "MALL_ITEMBONUS",				CItemData::APPLY_MALL_ITEMBONUS			    },
    { "MALL_GOLDBONUS",				CItemData::APPLY_MALL_GOLDBONUS			    },
    { "MAX_HP_PCT",					CItemData::APPLY_MAX_HP_PCT				    },
    { "MAX_SP_PCT",					CItemData::APPLY_MAX_SP_PCT				    },
    { "SKILL_DAMAGE_BONUS",			CItemData::APPLY_SKILL_DAMAGE_BONUS		    },
    { "NORMAL_HIT_DAMAGE_BONUS",	CItemData::APPLY_NORMAL_HIT_DAMAGE_BONUS	},
    { "SKILL_DEFEND_BONUS",			CItemData::APPLY_SKILL_DEFEND_BONUS		    },
    { "NORMAL_HIT_DEFEND_BONUS",	CItemData::APPLY_NORMAL_HIT_DEFEND_BONUS	},
    { "EXTRACT_HP_PCT",				CItemData::APPLY_EXTRACT_HP_PCT			    },
    { "RESIST_WARRIOR",				CItemData::APPLY_RESIST_WARRIOR			    },
    { "RESIST_ASSASSIN",			CItemData::APPLY_RESIST_ASSASSIN			},
    { "RESIST_SURA",				CItemData::APPLY_RESIST_SURA				},
    { "RESIST_SHAMAN",				CItemData::APPLY_RESIST_SHAMAN				},
    { "INFINITE_AFFECT_DURATION",	0x1FFFFFFF						            },
    { "ENERGY",						CItemData::APPLY_ENERGY					    },
    { "COSTUME_ATTR_BONUS",			CItemData::APPLY_COSTUME_ATTR_BONUS		    },
    { "MAGIC_ATTBONUS_PER",			CItemData::APPLY_MAGIC_ATTBONUS_PER		    },
    { "MELEE_MAGIC_ATTBONUS_PER",	CItemData::APPLY_MELEE_MAGIC_ATTBONUS_PER	},
    { "BLEEDING_REDUCE",			CItemData::APPLY_BLEEDING_REDUCE			},
    { "BLEEDING_PCT",				CItemData::APPLY_BLEEDING_PCT				},
    { "ATT_BONUS_TO_WOLFMAN",		CItemData::APPLY_ATT_BONUS_TO_WOLFMAN		},
    { "RESIST_WOLFMAN",				CItemData::APPLY_RESIST_WOLFMAN			    },
    { "RESIST_CLAW",				CItemData::APPLY_RESIST_CLAW				},
    { "ACCEDRAIN_RATE",				CItemData::APPLY_ACCEDRAIN_RATE			    },
    { "RESIST_MAGIC_REDUCTION",		CItemData::APPLY_RESIST_MAGIC_REDUCTION	},
    { "ENCHANT_ELECT",				CItemData::APPLY_ENCHANT_ELECT				},
    { "ENCHANT_FIRE",				CItemData::APPLY_ENCHANT_FIRE				},
    { "ENCHANT_ICE",				CItemData::APPLY_ENCHANT_ICE				},
    { "ENCHANT_WIND",				CItemData::APPLY_ENCHANT_WIND				},
    { "ENCHANT_EARTH",				CItemData::APPLY_ENCHANT_EARTH				},
    { "ENCHANT_DARK",				CItemData::APPLY_ENCHANT_DARK				},
    { "ATT_BONUS_TO_CZ",			CItemData::APPLY_ATTBONUS_CZ				},
    { "ATT_BONUS_TO_INSECT",		CItemData::APPLY_ATTBONUS_INSECT			},
    { "ATT_BONUS_TO_DESERT",		CItemData::APPLY_ATTBONUS_DESERT			},
    { "ATT_BONUS_TO_SWORD",			CItemData::APPLY_ATTBONUS_SWORD			    },
    { "ATT_BONUS_TO_TWOHAND",		CItemData::APPLY_ATTBONUS_TWOHAND			},
    { "ATT_BONUS_TO_DAGGER",		CItemData::APPLY_ATTBONUS_DAGGER			},
    { "ATT_BONUS_TO_BELL",			CItemData::APPLY_ATTBONUS_BELL				},
    { "ATT_BONUS_TO_FAN",			CItemData::APPLY_ATTBONUS_FAN				},
    { "ATT_BONUS_TO_BOW",			CItemData::APPLY_ATTBONUS_BOW				},
    { "ATT_BONUS_TO_CLAW",			CItemData::APPLY_ATTBONUS_CLAW				},
    { "RESIST_HUMAN",				CItemData::APPLY_RESIST_HUMAN				},
    { "RESIST_MOUNT_FALL",			CItemData::APPLY_RESIST_MOUNT_FALL			},
    { "RESIST_FIST",				CItemData::APPLY_RESIST_FIST				},
    { "MOUNT",						CItemData::APPLY_MOUNT						},

    { nullptr,							0								            }
};

long GetApplyTypeByName(const char* apply_type_string)
{
    TValueName* value_name;
    for (value_name = c_aApplyTypeNames; value_name->c_pszName; ++value_name)
    {
        if (0 == stricmp(value_name->c_pszName, apply_type_string))
            return value_name->lValue;
    }
    return 0;
}
#endif
