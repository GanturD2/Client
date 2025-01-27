
#ifndef __INC_METIN2_UNIQUE_ITEM_H
#define __INC_METIN2_UNIQUE_ITEM_H

enum
{
	UNIQUE_GROUP_LUCKY_GOLD					= 10024,	// ???
	UNIQUE_GROUP_AUTOLOOT					= 10011,	// ???
	UNIQUE_GROUP_RING_OF_EXP				= 10000,	// ???
	UNIQUE_GROUP_FISH_MIND					= 10009,	// ???
	UNIQUE_GROUP_LARGE_SAFEBOX				= 10021,	// ???
	UNIQUE_GROUP_DOUBLE_ITEM				= 10002,	// ???
	UNIQUE_GROUP_RING_OF_LANGUAGE			= 10025,	// ???
	UNIQUE_GROUP_SPECIAL_RIDE				= 10030,	// ???
	UNIQUE_GROUP_DRAGON_HEART				= 10053,	// ???

	UNIQUE_ITEM_TEARDROP_OF_GODNESS			= 70012,	// Tr�ne der G�ttin
	UNIQUE_ITEM_RING_OF_LANGUAGE			= 70006,	// Sprachenring
	UNIQUE_ITEM_RING_OF_LANGUAGE_SAMPLE		= 70047,	// Sprachenring (Beispiel)
	UNIQUE_ITEM_WHITE_FLAG					= 70008,	// Wei�e Flagge
	UNIQUE_ITEM_TREASURE_BOX				= 70009,	// Schatztruhe
	UNIQUE_ITEM_CAPE_OF_COURAGE				= 70038,	// Tapferkeitsumhang
	UNIQUE_ITEM_HALF_STAMINA				= 70040,	// Orkhartn�ckigkeit
	UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE		= 70048,	// Fl�chtlingsumhang
	UNIQUE_ITEM_SKIP_ITEM_DROP_PENALTY		= 70049,	// Lucys Ring
	UNIQUE_ITEM_FASTER_ALIGNMENT_UP_BY_TIME	= 70050,	// Symbol d. weisen Kaisers
	UNIQUE_ITEM_FASTER_ALIGNMENT_UP_BY_KILL	= 70051,	// Handschuh weiser Kaiser
	UNIQUE_ITEM_NO_BAD_LUCK_EFFECT			= 70052,	// Karma-Zauber 1
	UNIQUE_ITEM_LARBOR_MEDAL				= 70004,	// Flei�medaille
	UNIQUE_ITEM_DOUBLE_EXP					= 70005,	// Erfahrungsring
	UNIQUE_ITEM_FULL_EXP					= 38058,	// Trank der Weisheit (3h)
	UNIQUE_ITEM_DOUBLE_ITEM					= 70043,	// Diebeshandschuh
	UNIQUE_ITEM_PARTY_BONUS_EXP				= 70003,	// Buch des Anf�hrers
	UNIQUE_ITEM_PARTY_BONUS_EXP_MALL		= 71012,	// Buch des Anf�hrers
	UNIQUE_ITEM_PARTY_BONUS_EXP_GIFT		= 76011,	// Buch des Anf�hrers
	ITEM_GIVE_STAT_RESET_COUNT_VNUM			= 70014,	// Bluttablette
	ITEM_SKILLFORGET_VNUM					= 70037,	// Buch des Vergessens
	ITEM_SKILLFORGET2_VNUM					= 70055,	// ???
	UNIQUE_ITEM_FISH_MIND					= 71008,	// Fischereibuch
	UNIQUE_ITEM_SAFEBOX_EXPAND				= 71009,	// Lagertruhe
	UNIQUE_ITEM_AUTOLOOT_GOLD				= 71010,	// Dritte Hand
	UNIQUE_ITEM_EMOTION_MASK				= 71011,	// Emotionsmaske
	UNIQUE_ITEM_EMOTION_MASK2				= 71033,	// Emotionsmaske
	ITEM_NEW_YEAR_GREETING_VNUM				= 50023,	// Geldtasche
	ITEM_WONSO_BEAN_VNUM					= 50020,	// Bohnenkuchen
	ITEM_WONSO_SUGAR_VNUM					= 50021,	// Zuckerkuchen
	ITEM_WONSO_FRUIT_VNUM					= 50022,	// Obstkuchen
	ITEM_VALENTINE_ROSE						= 50024,	// Rose
	ITEM_VALENTINE_CHOCOLATE				= 50025,	// Schokolade
	ITEM_WHITEDAY_CANDY						= 50032,	// Bonbon
	ITEM_WHITEDAY_ROSE						= 50031,	// Rose
	ITEM_HORSE_FOOD_1						= 50054,	// Heu
	ITEM_HORSE_FOOD_2						= 50055,	// Karotte
	ITEM_HORSE_FOOD_3						= 50056,	// Roter Ginseng
	ITEM_REVIVE_HORSE_1						= 50057,	// Kraut der leichten Affen
	ITEM_REVIVE_HORSE_2						= 50058,	// Kraut d. normalen Affen
	ITEM_REVIVE_HORSE_3						= 50059,	// Kraut d. schweren Affen
	ITEM_HORSE_SKILL_TRAIN_BOOK				= 50060,	// Buch der Reitkunst
//	UNIQUE_ITEM_MARRIAGE_FASTER_LOVE_POINT	= 71068,	// Feder der Liebenden
	UNIQUE_ITEM_MARRIAGE_PENETRATE_BONUS	= 71069,	// Ohrringe der Harmonie
	UNIQUE_ITEM_MARRIAGE_EXP_BONUS			= 71070,	// Armband der Liebe
	UNIQUE_ITEM_MARRIAGE_CRITICAL_BONUS		= 71071,	// Ohrringe der Liebe
	UNIQUE_ITEM_MARRIAGE_TRANSFER_DAMAGE	= 71072,	// Armband der Harmonie
	UNIQUE_ITEM_MARRIAGE_ATTACK_BONUS		= 71073,	// Halskette der Liebe
	UNIQUE_ITEM_MARRIAGE_DEFENSE_BONUS		= 71074,	// Halskette der Harmonie
	ITEM_MARRIAGE_RING						= 70302,	// Ehering
	ITEM_MINING_SKILL_TRAIN_BOOK			= 50600,	// Bergbau-Buch
	ITEM_SKILLBOOK_VNUM						= 50300,	// Fertigkeitsbuch
#ifdef ENABLE_REFINE_ABILITY_SKILL
	ITEM_REFINE_ABILITY_SKILL_TRAIN_BOOK	= 51306,	// Buch des Schmiedens
#endif
	ITEM_PRISM								= 71113,	// Glas der Einsicht
	ITEM_AUTO_HP_RECOVERY_S					= 72723,	// Elixier der Sonne (K)
	ITEM_AUTO_HP_RECOVERY_M					= 72724,	// Elixier der Sonne (M)
	ITEM_AUTO_HP_RECOVERY_L					= 72725,	// Elixier der Sonne (G)
	ITEM_AUTO_HP_RECOVERY_X					= 72726,	// Elixier der Sonne (S)
	ITEM_AUTO_SP_RECOVERY_S					= 72727,	// Elixier des Mondes (K)
	ITEM_AUTO_SP_RECOVERY_M					= 72728,	// Elixier des Mondes (M)
	ITEM_AUTO_SP_RECOVERY_L					= 72729,	// Elixier des Mondes (G)
	ITEM_AUTO_SP_RECOVERY_X					= 72730,	// Elixier des Mondes (S)
	ITEM_AUTO_EXP_RING_S					= 96021,	// Elixier der Erfahrung (K)
	ITEM_AUTO_EXP_RING_M					= 96022,	// Elixier der Erfahrung (M)
	ITEM_AUTO_EXP_RING_G					= 96023,	// Elixier der Erfahrung (G)
	ITEM_RAMADAN_CANDY						= 50183,	// S��igkeiten
	UNIQUE_ITEM_RAMADAN_RING				= 71135,	// Halbmondring
	UNIQUE_ITEM_RAMADAN_RING_PLUS			= 71235,	// Halbmondring
	UNIQUE_EASTER_CANDY_EQIP				= 71188,	// Lolli der Magie
	ITEM_NOG_POCKET							= 50216,	// Gl�hwein
	REWARD_UNIQUE_BANDAGE					= 27124,	// Verband
	UNIQUE_CHRISTMAS_LOLLIPOP				= 71136,	// Lolli der St�rke
	REWARD_BOX_ITEM_AUTO_SP_RECOVERY_XS		= 76004,	// Elixier des Mondes (K)
	REWARD_BOX_ITEM_AUTO_SP_RECOVERY_S		= 76005,	// Elixier des Mondes (M)
	REWARD_BOX_ITEM_AUTO_HP_RECOVERY_S		= 76021,	// Elixier der Sonne (K)
	REWARD_BOX_ITEM_AUTO_HP_RECOVERY_XS		= 76022,	// Elixier der Sonne (M)
	REWARD_BOX_UNIQUE_ITEM_CAPE_OF_COURAGE	= 76007,	// Tapferkeitsumhang
	DRAGON_SOUL_EXTRACTOR_GROUP				= 10600,	// ???
	DRAGON_HEART_EXTRACTOR_GROUP			= 10601,	// ???
	DRAGON_HEART_VNUM						= 100000,	// Elixier der Zeit (K)
#ifdef ENABLE_AURA_SYSTEM
	ITEM_AURA_BOOST_ITEM_VNUM_BASE			= 49980,
#endif
#ifdef ENABLE_SET_ITEM
	ITEM_DELETE_SET_EFFECT					= 39115,	// Set-Effekt l�schen
#endif
#ifdef ENABLE_MAILBOX
	MOBILE_MAILBOX							= 72340,
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	TRANSMUTATION_REVERSAL					= 72325,
	TRANSMUTATION_TICKET_1					= 72326,
	TRANSMUTATION_TICKET_2					= 72341,
#endif
#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
	PRIVATE_SHOP_SEARCH_LOOKING_GLASS = 60004,
	PRIVATE_SHOP_SEARCH_TRADING_GLASS = 60005,
#endif
};

#endif /* __INC_METIN2_UNIQUE_ITEM_H */

