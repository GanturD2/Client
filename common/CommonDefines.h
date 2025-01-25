#ifndef __INC_METIN2_COMMON_DEFINES_H__
#define __INC_METIN2_COMMON_DEFINES_H__

enum eCommonDefines {
	//DEFINES						//VALUE			//DESCRIPTION								// DEFAULT
	EVENT_MOB_RATE_LIMIT			= 1000,			//											// 1000
	MAP_ALLOW_LIMIT					= 45,			//											// 32
	PRIV_EMPIRE_RATE_LIMIT			= 200,			//											// 200
	PRIV_EMPIRE_TIME_LIMIT			= 60*60*24*7,	//											// 60*60*24*7 [1 week in seconds]
	PRIV_GUILD_RATE_LIMIT			= 50,			//											// 50
	PRIV_GUILD_TIME_LIMIT			= 60*60*24*7,	//											// 60*60*24*7 [1 week in seconds]
	PRIV_PLAYER_RATE_LIMIT			= 100,			//											// 100
	GUILD_GRADE_NAME_MAXLEN			= 8,			//											// 8
	GUILD_GRADECOUNT				= 15,			//											// 15
	GUILD_COMMENT_MAXCOUNT			= 12,			//											// 12
	GUILD_COMMENT_MAXLEN			= 50,			//											// 50
	GUILD_BASEPOWER					= 400,			//											// 400
	GUILD_POWER_PER_SKILL_LEVEL_	= 200,			//											// 200
	GUILD_POWER_PER_LEVEL_			= 100,			//											// 100
	GUILD_MINIMUM_LEADERSHIP_		= 40,			//											// 40
	GUILDWAR_MIN_MEMBER_COUNT		= 1,			// Mindestspieleranzahl f¸r ein GK			// 8
	GUILD_LADDERPOINT_PER_LEVEL		= 1000,			//											// 1000
	GUILD_CREATE_ITEMVNUM			= 70101,		//											// 70101
	PARTY_CAN_JOIN_MAX_LEVEL		= 30,			//											// 30
	DISAPPEARANCE_KILLED_MONSTER	= 10,			//											// 10
	REVIVEINVISIBLE_TIME			= 5,			//											// 5
	STONE_FAILURE0					= 80,			// Steinupgrade %-Change					// 80
	STONE_FAILURE1					= 70,			// Steinupgrade %-Change					// 70
	STONE_FAILURE2					= 60,			// Steinupgrade %-Change					// 60
	STONE_FAILURE3					= 50,			// Steinupgrade %-Change					// 50
	STONE_FAILURE4					= 40,			// Steinupgrade %-Change					// 40
	SB_NEED_EXP						= 20000,		// Exp f¸r Fertigkeitsbuch					// 20000
	SB_C_NEED_EXP					= 20000,		// Exp f¸r 9th SKills						// 20000
	OWNERSHIP_EVENT_TIME			= 300,			//											// 300
	MIN_ADDON_FKS					= -30,			// Minimum FKS								// -30
	MAX_ADDON_FKS					= 30,			// Maximum FKS								// 30
	MIN_ADDON_DSS					= 32767,		// Minimum DSS								// 32767
	MAX_ADDON_DSS					= 32767,		// Maximum DSS								// 32767
	SERVER_STATE_CACHE_EXPIRATION	= 5*60,			// Channelstatus Update Time				// 5*60 [5 Minutes in seconds]
	PC_MOVE_SPEED					= 100,			// Spieler Movespeed						// 100
	PC_ATT_SPEED					= 100,			// Spieler Attackspeed						// 100
	PC_CAST_SPEED					= 100,			// Spieler Zauberspeed						// 100
	PC_MAX_ATT_SPEED				= 170,			// Maximum Spieler Att-Speed				// 170
	MOB_MAX_ATT_SPEED				= 250,			// Maximum Mob Att-Speed					// 250
	PC_MAX_MOVE_SPEED				= 200,			// Maximum Spieler Move-Speed				// 200
	MOB_MAX_MOVE_SPEED				= 250,			// Maximum Mob Move-Speed					// 250
	ITEM_FLOOR_TIME					= 30,			// Default Items on Floor Time Destroy		// 30
	ITEM_DEST_TIME_AUTOITEM			= 300,			// Autogive Destroy Time					// 300
	ITEM_DEST_TIME_DROPITEM			= 150,			// DropItem Destroy Time					// 150
	ITEM_DEST_TIME_DROPGOLD			= 300,			// DropGold Destroy Time					// 300

	MAX_RANK_POINTS					= 20000,		// Maximum Rangpunkte						// 20000
	START_LEVEL						= 1,			// Startlevel								// 1
	START_GOLD						= 0,			// Startgold								// 0
	START_WON						= 0,			// Startwon									// 0		[ENABLE_CHEQUE_SYSTEM]
	START_GEM						= 0,			// Startgaya								// 0		[ENABLE_GEM_SYSTEM]
	DROPABLE_GOLD_LIMIT				= 100000,		// Dropbar (1000-1 = 999)					// 1000
	ITEM_COUNT_LIMIT				= 250,			// Max. Stack-Itemanzahl					// 200
	ITEM_BONUS_CHANGE_TIME			= 60,			// 											// 60
	STATUSPOINT_GET_LEVELLIMIT		= 90,			// 											// 90
	STATUSPOINT_SET_MAXVALUE		= 90,			// 											// 90
	SHOUT_LIMIT_LEVEL				= 15,			// 											// 15
	PLAYER_MAX_LEVEL				= 120,			// Max Spieler Level						// 99
	CONQUEROR_MAX_LEVEL				= 30,			// Max. Conqueror Level						// 30		[ENABLE_YOHARA_SYSTEM]
	SKILLBOOK_NEXT_READ_MIN			= 28800,		// Min. benˆtigte Exp f¸r FBs				// 28800
	SKILLBOOK_NEXT_READ_MAX			= 43200,		// Max. benˆtigte Exp f¸r FBs				// 43200
	RANGE_PICK						= 600,			// Range Pickup								// 300
	SUNGMA_ATTR_PROB				= 40,			// Sungma Items Change of Attr				// 40
	PREMIUM_PRIVATE_SHOP_DEFAULT_TIME = 60 * 24,	// Offlineshop Open Time [1 Day]			// 60 * 24

	CL_TRANSMUTATION_PRICE			= 15000000,		// Transmutation Kosten						// 15000000	[ENABLE_CHANGE_LOOK_SYSTEM]
	PET_INCUBATRICE_COST			= 100000,		// Kosten f¸r Ausbr¸ten						// 100000	[ENABLE_GROWTH_PET_SYSTEM]
	IMMUNE_PCT						= 90,			// %-Immunrate (if Def. active)				// 90		[ENABLE_IMMUNE_PERC]
	SHAM_ASSA_ATT_BONUS				= 15,			// +%-Wert f¸r den PvE Boost				// 15		[ENABLE_SHAM_ASSA_ATT_BONUS]
	KK_ATT_BONUS_AGAINST_NH			= 10,			// +%-Wert f¸r den PvP Boost				// 10		[ENABLE_KK_ATT_BONUS_AGAINST_NH]
	ANNOUNCEMENT_REFINE_SUCCES_MIN_LEVEL	= 9,	// Ab welcher + Refine es angezeigt wird	// 9		[ENABLE_ANNOUNCEMENT_REFINE_SUCCES]
	ITEM_LEVEL_LIMIT				= 64,			// Bis welchem Lv. es nicht angezeigt wird	// 64		[ENABLE_ANNOUNCEMENT_REFINE_SUCCES]
	MEMBER_COUNT_BONUS				= 6,			// Anzahl der Member [Bonus 2]				// 6		[ENABLE_GUILDRENEWAL_SYSTEM]
	AFK_TIME_LIMIT					= 3,			// Afk Mode nach [x] Minuten				// 3		[ENABLE_AFK_MODE_SYSTEM]
	SOULUNBIND_TIME					= 72,			// Unbind Time [in hours]					// 72		[ENABLE_SEALBIND_SYSTEM]
	AFTERDEATH_SHIELD_DURATION		= 3,			// Duration of shield protection (in secs)	// 15		[ENABLE_AFTERDEATH_SHIELD]
	OFFLINESHOP_OPEN_TIME			= 10,			// Time of open Offlineshops				// 60*24*3	[ENABLE_PREMIUM_PRIVATE_SHOP]
	MAX_FARMING_ACCOUTS				= 2,			// Max Accounts per HWID					// 2		[ENABLE_MULTI_FARM_BLOCK]
	BATTLE_FIELD_KILL_TIME			= 60,			// Killzeit per Spieler [Sekunden]			// 600		[ENABLE_BATTLE_FIELD]
	DS_ATTR_CHANGE_PRICE			= 500000,		// DS-Attr-Change Preis						// 500000	[ENABLE_DS_CHANGE_ATTR]
	PERCENT_ADD_BONUS_ELEMENTS_SPELL = 25,			// Element Erfolgsrate						// 50		[ENABLE_REFINE_ELEMENT]
	SOUL_COLLECT_PERC				= 10,			// Seelen %-Chance							// 10		[ENABLE_SUMMER_EVENT_ROULETTE]
	FISHING_NEED_CATCH				= 3,			// 

	LOTTO_TICKET_COST				= 5000000,		// Wie viel ein neues Ticket kostet			// 5000000	[ENABLE_WORLD_LOTTERY_SYSTEM]
	MIN_JACKPOT_MONEY				= 250000000,	// Mind. Gewinnsumme						// 250000000 [ENABLE_WORLD_LOTTERY_SYSTEM]

	// Dungeon Settings
	MELEY_MIN_LVL					= 3,			//											// 3		[ENABLE_GUILD_DRAGONLAIR_SYSTEM]
	MELEY_LADDER_POINTS_COST		= 600,			// 											// 600		[ENABLE_GUILD_DRAGONLAIR_SYSTEM]
	MELEY_LADDER_POINTS_RETURN		= 300,			// 											// 300		[ENABLE_GUILD_DRAGONLAIR_SYSTEM]
	MELEY_COOLDOWN_DUNGEON			= 10800,		// 											// 10800	[ENABLE_GUILD_DRAGONLAIR_SYSTEM]
	WEATHER_REFRESH_TIMER			= 60,			// Wetter Reload Zeit [sek]					// 60 * 5	[ENABLE_WEATHER_INFO]
	WEATHER_RAIN_PERC				= 5,			// Wetter Regen Chance						// 1		[ENABLE_WEATHER_INFO]
	RAIN_SNOW_ACTIVE_TIME			= 30,			// Aktiviert [min]							// 30		[ENABLE_WEATHER_INFO]

	CHECK_PICKUP_HACK				= 10,			//
	PICKUP_RANGE					= 150,			// Pickup-range								// 300		[ENABLE_CHECK_PICKUP_HACK]
	PICKUP_RANGE_TOLERANCE			= 100,			// Pickup-tolerance							// 150		[ENABLE_CHECK_PICKUP_HACK]
};


/*
//------Servercommands------>
ENABLE_EVENT_BANNER_FLAG							/banner 1 easter
ENABLE_BATTLE_FIELD									/e battlefield_status 1
*/



//------NOT_FINISHED------>
#define ENABLE_PULSE_MANAGER
#define ENABLE_SKILL_TABLE_RENEWAL





//////////////////////////////////////////////////[ONLY GAMESOURCE]//////////////////////////////////////////////////
#define ENABLE_MEMORY_LEAK_FIX
//#define ENABLE_HAND_SHAKE_LIMIT
#define ENABLE_PROXY_IP
/*BIND_IP: <Private IPv4 IP> (it will be something within a private IP class. Check the IP of the machine using the "ifconfig" command)
PROXY_IP: <Public IPv4 IP> (check out something like https://www.whatismyip.com)*/

//------SECURITY------>
#define ENABLE_PORT_SECURITY						// Blockiert db_port, p2p_port und remote adminpage exploits
#define ENABLE_MAIN_HACK
//#define ENABLE_CSHIELD
	#ifdef ENABLE_CSHIELD
	#	define ENABLE_CHECK_GHOSTMODE						// checks if player is using ghostmode
	#	define ENABLE_CHECK_WALLHACK						// checks player position for wallhack
	#	define ENABLE_CHECK_PICKUP_HACK PICKUP_RANGE+PICKUP_RANGE_TOLERANCE //checks if player using pickup hack //important: check CPythonPlayer::SendClickItemPacket for pickup time. if you dont have that just disable this define
	#	define ENABLE_CHECK_ATTACKSPEED_HACK				// checks if player is attacking too fast
	#	define ENABLE_CHECK_MOVESPEED_HACK					// checks if player is moving too fast
	#	define ENABLE_CHECK_WAIT_HACK						// checks if player is using waithack
	#	define ENABLE_CHECK_VALID_FISH_POSITION				// 
	#	define ENABLE_CHECK_ANTI_SAFEZONE					// 
	#	define ENABLE_CHECK_SKILL_HACK						// 
	#endif
#define FLUSH_AT_SHUTDOWN							// flush cache at /shutdown
//#define ENABLE_SYSLOG_PACKET_SENT					// Send PacketHeader Syslog





//--------SYSTEMS------->





//--------OPTIONS------->
#define ENABLE_REVERSED_FUNCTIONS					// Offical Used Functions
#define ENABLE_SYNCPOSITION_DISCONNECT_FIX			// 
#define ENABLE_EXTENDED_RELOAD						// Extended Reload commands
#define ENABLE_CMS_ITEMSHOP							// Ingame Itemshop
#define ENABLE_GM_BLOCK								// 
#define ENABLE_CHECK_WINDOW_RENEWAL					// 
#define ENABLE_PVP_BALANCE							// Pvp Attr like Offical
#define ENABLE_AFFECT_RENEWAL						// 
#define ENABLE_REFINE_ABILITY_SKILL					// Schmiedeskill
#define ENABLE_ATTRIBUTE_CHANGE_ON_START_ITEM		// 
#define ENABLE_ANNOUNCEMENT_REFINE_SUCCES			// 
#define ENABLE_ANNOUNCEMENT_LEVELUP					// 
#define ENABLE_AGGROMOB_SAFEZONE_SKIP				// xP3NG3Rx - Aggro Mob skips Safezone
#define ENABLE_GUILDLAND_INACTIVITY_DELETE			// 
#define ENABLE_EFFECT_FOR_TRADE						// Effect after completed trading
#define ENABLE_BLOCK_SKILL_OXEVENT					// 
#define ENABLE_BLOCK_BUFF_IN_MAP					// 
#define ENABLE_HORSESKILLS_ON_MOUNTS				// 
#define DISABLE_RIDE_ON_SPECIAL_MAP					// 
#define ENABLE_RITUAL_STONE							// Ritualstein
#define ENABLE_SEAL_OF_GOD							// Siegel der Gˆtter
#define ENABLE_SHAMAN_PARTY_BUFF_SYSTEM				// Shaman 1click group-buff + selfbuff
#define ENABLE_SHAM_ASSA_ATT_BONUS					// Shamis & Ninjas +% im PvE
//#define ENABLE_KK_ATT_BONUS_AGAINST_NH			// PvP-System [KK machen nur gegen NH mehr schaden]
#define ENABLE_DROP_GOLD							// 
#define ENABLE_GOLD_MULTIPLIER						// 
#define ENABLE_GOLD10_PCT							// 
#define ENABLE_BLOCK_CHEST_INVENTORY_FULL			// 
//#define ENABLE_CANT_CREATE_SHOPS_IN_OTHER_EMPIRE	// Shops kˆnnen nicht im fremden Reich geˆffnet werden
#define ENABLE_ARENA_FORBIDDEN_ITEMS_LIST			// Verbotene Liste via Germany Ordner
#define ENABLE_HORSE_APPEARANCE						// Horseskin via Database
//#define ENABLE_MOUNT_ENTITY_REFRESH				// 
#define ENABLE_EXTEND_ITEM_AWARD					// Bonus on Itemshop-Items possible
//#define MOB_DROP_ITEM_RENEWAL						// enable on mob_drop_item -> map_index (like: "map_index    41")
//#define ENABLE_CUBE_COPY_ATTR						// Allow copy bonis in cube
#define ENABLE_PORTAL_NAMES							// Erlaubt Anzeige der Portalnamen
#define ENABLE_MAP_NAME_INFO						// Mapnamen werden im Chat angezeigt
//#define ENABLE_CLEAN_BANNED_PLAYER_DATA			// 
#define ENABLE_SET_STATES							// Status Verteilen mit Eingabe
	#ifdef ENABLE_SET_STATES
	#	define SET_STATES_PRIVILEGES GM_PLAYER				// Privileges: GM_PLAYER, GM_LOW_WIZARD
	#endif
#define ENABLE_DUNGEON_MUSIC_FIX					// 
#define ENABLE_POISON_FIX							// 
//#define DISABLE_STONE_LV20_DROP					// 
//#define DISABLE_BOSS_LV25_DROP					// 





//////////////////////////////////////////////////[GAMESOURCE& BINARY]//////////////////////////////////////////////////
//-----GLOBAL_STUFF---->
//#define ENABLE_SEQUENCE_SYSTEM	//@fixme426
#define ENABLE_HWID_BAN								// HWID-Manager
	#ifdef ENABLE_HWID_BAN
//	#	define ENABLE_HWID_BAN_EXTENDED						// if other hwid tries to login --> not possible, needs extra reset function
	#	define ENABLE_MULTI_FARM_BLOCK						// Multifarmblock
	#	define GM_UNBAN_RIGHT 4								// 
	#	define GM_BAN_RIGHT 4								// 
	#	define GM_HWID_BAN_RIGHT 4							// 
	#	define GM_DELETE_HWID_BAN_RIGHT 4					// 
	#	define GM_TOTAL_HWID_BAN_RIGHT 4					// 
	#endif





//-----PROTO_STRUCT---->
#define ENABLE_PROTO_RENEWAL						// Offical Proto Struct
#define ENABLE_PROTO_RENEWAL_CUSTOM					// Proto Changes to be used by Custom Systems
#define ENABLE_NEW_DISTANCE_CALC					// Distance Calculation Renewal





//------CHARACTERS------>
#define ENABLE_PLAYER_PER_ACCOUNT5					// Erlaubt 5 Character
#define ENABLE_WOLFMAN_CHARACTER					// Wolfman Character
	#ifdef ENABLE_WOLFMAN_CHARACTER
//	#	define USE_MOB_BLEEDING_AS_POISON					// Bluten wird deaktiviert und ist wie Gift
//	#	define USE_MOB_CLAW_AS_DAGGER						// Krallen wird deaktiviert und ist wie Dolche
//	#	define USE_ITEM_BLEEDING_AS_POISON					// Giftbonus kann bluten deffen (bluten wird deaktiviert)
//	#	define USE_ITEM_CLAW_AS_DAGGER						// Wiederstand gegen Dolche bonus kann Krallen deffen (Krallen wird deaktiviert)
	#	define USE_WOLFMAN_STONES							// Wolfmenschsteine kˆnnen bei Metins gedroppt werden
	#	define USE_WOLFMAN_BOOKS							// Wolfmensch Skillb¸cher kˆnnen gedropt werden
	#	define USE_LYCAN_CREATE_POSITION					// if enabled, the lycan will be warped to his own village at character creation
	#endif





//--------SYSTEMS------->
#define ENABLE_MOB_SCALE							// Scale per mob_proto.txt
#define NEW_SELECT_CHARACTER						// Select last played Character
#define WJ_SHOW_MOB_INFO							// Monster Lv/Aggro Anzeige
#define WJ_ENABLE_TRADABLE_ICON						// Nichthandelbare Items anzeigen
#define ENABLE_SWAP_SYSTEM							// Inventory Item Swap System
#define ENABLE_PET_SYSTEM							// 
	#ifdef ENABLE_PET_SYSTEM
	#	define PET_AUTO_PICKUP								// 
	#endif
#define ENABLE_GROWTH_PET_SYSTEM					// 
	#ifdef ENABLE_GROWTH_PET_SYSTEM
	#	define ENABLE_PET_ATTR_DETERMINE					// 
	#	define ENABLE_PET_PRIMIUM_FEEDSTUFF					// 
	#endif
#define ENABLE_ACCE_COSTUME_SYSTEM					// Sash-System
#define ENABLE_AURA_SYSTEM							// Aura
	#ifdef ENABLE_AURA_SYSTEM
	#	define ENABLE_AURA_BOOST							// 
	#endif
#define ENABLE_YOHARA_SYSTEM						// Conqueror Level + Sungma
	#ifdef ENABLE_YOHARA_SYSTEM
	#	define ENABLE_NINETH_SKILL							// 9th Skill
//	#	define ENABLE_PASSIVE_SYSTEM						// Relict Window [unfinished]
	#endif
#define ENABLE_GLOVE_SYSTEM							// Glove Item
	#ifdef ENABLE_GLOVE_SYSTEM
	#	define ENABLE_GLOVE_ITEM_ATTR						// item_attr table
	#endif
#define ENABLE_REFINE_ELEMENT						// Element Set System [Worldard]
#define ENABLE_GUILDSTORAGE_SYSTEM					// Gildenlager
#define ENABLE_SPECIAL_INVENTORY					// Special Inventory [Sanii]
#define ENABLE_DS_GRADE_MYTH						// Myth-grade
#define ENABLE_DS_CHANGE_ATTR						// 
#define ENABLE_DS_REFINE_WINDOW						// 
#define ENABLE_DS_SET								// 
#define ENABLE_MOUNT_COSTUME_SYSTEM					// enable mount costume slot
	#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	#	define ENABLE_MOUNT_PROTO_AFFECT_SYSTEM				// 
	#	define ENABLE_CHANGE_LOOK_MOUNT						// 
	#	define ENABLE_MOUNT_CHECK							// 
	//#	define ENABLE_NO_WAIT_TO_RIDE						// 
	#endif
#define ENABLE_WEAPON_COSTUME_SYSTEM				// enable weapon costume slot
	#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	#	define ENABLE_ITEM_ATTR_COSTUME						// enable costume_hair, costume_body, costume_weapon item_attr/_rare parts
	#endif
#define ENABLE_USE_COSTUME_ATTR						// enables body costume bonus switcher
#define ENABLE_MOVE_COSTUME_ATTR					// Costume bonus transfer
	#ifdef ENABLE_MOVE_COSTUME_ATTR
	#	define ENABLE_MOVE_COSTUME_ACCE_ATTR
	#endif
#define ENABLE_HIDE_COSTUME_SYSTEM					// Disable/Enable Costume Design
#define ENABLE_CHANGED_ATTR							// 
#define ENABLE_MAGIC_REDUCTION_SYSTEM				// enable resist magic reduction bonus
	#ifdef ENABLE_MAGIC_REDUCTION_SYSTEM
	#	define USE_MAGIC_REDUCTION_STONES					// enable resist magic reduction stone drops from metins
	#endif
#define ENABLE_PENDANT								// Pendant-Item-Slot
	#ifdef ENABLE_PENDANT
	#	define ENABLE_ELEMENT_ADD							// 
	#	define ENABLE_PENDANT_ITEM_ATTR						// allows to give Pendant Bonus from item_attr/rare
	#endif
#define ENABLE_MULTI_LANGUAGE_SYSTEM				// Multilang System [Sonitex]
#define ENABLE_SEALBIND_SYSTEM						// Seelenbindung
#define ENABLE_CHEQUE_SYSTEM						// Won/Cheque
	#ifdef ENABLE_CHEQUE_SYSTEM
	#	define ENABLE_CHEQUE_EXCHANGE_WINDOW				// xP3ng3rx won -> yang / yang -> won Exchange Window
	#endif
#define ENABLE_GACHA_SYSTEM							// Gacha-System
#define ENABLE_ACHIEVEMENT_SYSTEM					// 
#define ENABLE_BIOLOG_SYSTEM						// 
#define ENABLE_AUTO_SYSTEM							// AutoHunt
#ifdef ENABLE_AUTO_SYSTEM
	#define ENABLE_AUTO_RESTART_EVENT
#endif
#define ENABLE_RESEARCHER_ELIXIR_FIX				// 
#define ENABLE_678TH_SKILL							// 678 Passive Skills
#define ENABLE_78TH_SKILL							// 7&8 Passive Skills
#define ENABLE_PASSIVE_ATTR							// 
	#ifdef ENABLE_PASSIVE_ATTR
	#	define ENABLE_PRECISION								// 
	#	define ENABLE_CHARISMA								// 
	#	define ENABLE_INSPIRATION							// 
	#endif
#define ENABLE_SWITCHBOT							// Switchbot [Sanii]
	#ifdef ENABLE_SWITCHBOT
//	#	define ENABLE_IGNORE_TYPE_GREEN_ATTR_ITEM			// 
	#	define ENABLE_SWITCHBOT_PM							// Out of switchers in PM
	#endif
#define ENABLE_BATTLE_PASS_SYSTEM					// Battle pass system
	#ifdef ENABLE_BATTLE_PASS_SYSTEM
	#	define RESTRICT_COMMAND_GET_INFO			GM_LOW_WIZARD
	#	define RESTRICT_COMMAND_SET_MISSION			GM_IMPLEMENTOR
	#	define RESTRICT_COMMAND_PREMIUM_ACTIVATE	GM_IMPLEMENTOR	
	#endif
#define ENABLE_CHANGE_LOOK_SYSTEM					// Transmutation
#define ENABLE_ATTR_6TH_7TH							// 67 Bonus Board
	#ifdef ENABLE_ATTR_6TH_7TH
	#	define ENABLE_ATTR_6TH_7TH_EXTEND					// New offical Bonus
	#	define ENABLE_ATTR_6TH_7TH_ATTR_TABLE				// 
	#endif
#define ENABLE_SKILL_COLOR_SYSTEM					// Skill Color Change [Owsap]
#define ENABLE_CUBE_RENEWAL							// Cube Renewal
	#ifdef ENABLE_CUBE_RENEWAL
	#	define ENABLE_CUBE_RENEWAL_COPY						// able to Copy attributes
	#endif
#define ENABLE_VIP_SYSTEM							// VIP System
	#ifdef ENABLE_VIP_SYSTEM
//	#	define ENABLE_SWITCHBOT_FOR_VIP						// only VIP's can use Switchbot
//	#	define ENABLE_SHAM_ASSA_ATT_BONUS_FOR_VIP			// only VIP's get SHAM/ASSA ATT Bonus
//	#	define ENABLE_KK_ATT_BONUS_AGAINST_NH_FOR_VIP		// only VIP's get KK_ATT Bonus against NH
//	#	define ENABLE_YANG_BOMB_FOR_VIP						// only VIP's can use YangBomb-Feature
//	#	define ENABLE_TRANSMUTATION_FOR_VIP					// only VIP's can use Transmutation
//	#	define ENABLE_DROP_INFO_FOR_VIP
	#endif

#define ENABLE_PREMIUM_PRIVATE_SHOP					// 
	#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	#	define ENABLE_OPEN_SHOP_WITHOUT_BAG					// 
	#	define ENABLE_OPEN_SHOP_ONLY_IN_MARKET				// 
	#	define ENABLE_OPEN_SHOP_WITH_PASSWORD				// 
	#	define ENABLE_PREMIUM_PRIVATE_SHOP_TIME				// DO NOT ENABLE IF SHOP_ACTIVITY IS ENABLED
	#	define ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL			// 
	#	define ENABLE_SHOP_NO_SPEND_MIN_IF_ONLINE			// rework in future
//	#	define ENABLE_PRIVATESHOP_SYSTEM_ALWAYS_SILK		// 
//	#	define ENABLE_PRIVATE_SHOP_SEARCH_NEED_ITEM			// 
//	#	define ENABLE_PRIVATSHOP_EXTENSIONS					// 
	#	define ENABLE_PRIVATESHOP_SEARCH_SYSTEM				// 
	#	define ENABLE_PRIVATESHOP_SEARCH_PACKETFLOOD_TEMP_FIX// Only max 800 Entrys will send to Binary [Stop Crash Client] - Packet Flood
	#endif
#define ENABLE_MYSHOP_DECO							// Shop Decoration
	#ifdef ENABLE_MYSHOP_DECO
		#define ENABLE_MYSHOP_DECO_PREMIUM					// 
	//	#define ENABLE_MYSHOP_DECO_EXTENSION				// 
	#endif
#define ENABLE_MAILBOX								// Mailbox
#define ENABLE_TICKET_SYSTEM						// Ingame Ticket
#define ENABLE_DESTROY_SYSTEM						// Drop Items Zerstˆren System
//#define ENABLE_GROUP_DAMAGE_WEAPON					// Group-Dmg-System
#define ENABLE_INGAME_WIKI							// 
#define ENABLE_QUIVER_SYSTEM						// Pfeilkˆcher
#define ENABLE_DUNGEON_INFO_SYSTEM					// Dungeon Information
#define ENABLE_MOVE_CHANNEL							// Channel Switcher
#define ENABLE_QUEST_RENEWAL						// Quest page renewal
#define ENABLE_PARTY_MATCH							// 
#define ENABLE_FISHING_RENEWAL						// 
#define ENABLE_WEATHER_INFO							// Automatic Weather Load





/*-----MiniGame*///----->
#define ENABLE_EVENTS								// 
	#ifdef ENABLE_EVENTS
	#	define ENABLE_EVENT_MANAGER							// Event Info [Board]
	#	define ENABLE_EVENT_BANNER_FLAG						// Event-Banner

	#	define ENABLE_MINI_GAME								// Header of MiniEvents

	//	Offical Events
//	#	define ENABLE_LUCKY_EVENT							// unused
	#	define ENABLE_MINI_GAME_OKEY_NORMAL					// Okey-Card
//	#	define ENABLE_MINI_GAME_YUTNORI						// Halloween
	#	define ENABLE_MONSTER_BACK							// Bossj‰ger
			#ifdef ENABLE_MONSTER_BACK
			#	define ENABLE_ACCUMULATE_DAMAGE_DISPLAY				// 
		//	#	define ENABLE_10TH_EVENT							// 
			#endif
	#	define ENABLE_EASTER_EVENT							// Oster-Event
	#	define ENABLE_SUMMER_EVENT							// unused
	#	define ENABLE_2017_RAMADAN							// [Serverside]
	#	define ENABLE_NEW_HALLOWEEN_EVENT					// [Serverside]
	#	define ENABLE_SOUL_SYSTEM							// Soul System [Owsap]
	#	define ENABLE_2016_VALENTINE						// Valentinstag
	#	define ENABLE_FISH_EVENT							// Fischpuzzle
	#	define ENABLE_FLOWER_EVENT							// Flower Event
	#	define ENABLE_MINI_GAME_CATCH_KING					// Schnapp den Kˆnig
	#	define ENABLE_GEM_SYSTEM							// Gaya
			#ifdef ENABLE_GEM_SYSTEM
			#	define ENABLE_GEM_SHOP								// 
			#endif
	#	define ENABLE_MINI_GAME_FINDM						// 
			#ifdef ENABLE_MINI_GAME_FINDM
			#	define ENABLE_MINI_GAME_FINDM_HINT					// 
			#endif
	#	define ENABLE_SUMMER_EVENT_ROULETTE					// Blutmond-Event
	#	define ENABLE_MINI_GAME_BNW							// Black 'n White
//	#	define ENABLE_BATTLE_ROYALE							// unused
	#	define ENABLE_METINSTONE_RAIN_EVENT					// 
	#	define ENABLE_SOCCER_BALL_EVENT						// Football drop	[]
	#	define ENABLE_WORD_GAME_EVENT						// M-E-T-I-N-2		[]

	#	define ENABLE_WORLD_BOSS
		#ifdef ENABLE_WORLD_BOSS
		#	define WORLD_BOSS_PHASE 6
		#	define BATTLE_PHASE 4
		#	define COOLDOWN_PHASE 2

		#	define ENABLE_WB_RANKING
		#	ifdef ENABLE_WB_RANKING
		#		define WB_MIN_DMG 60000
		#	endif
		#endif

	#	define ENABLE_SUPERSTONE_RAIN_EVENT
	#	define ENABLE_MINING_EVENT
	//	Custom Events
	#endif





/*-----Dungeons*///----->
#define ENABLE_DUNGEON_RENEWAL
#define ENABLE_DUNGEON_DEAD_TIME_RENEWAL
#define ENABLE_ELEMENTAL_WORLD
	#ifdef ENABLE_ELEMENTAL_WORLD
	#	define DROP_PERC_ELEMENT_04 4
	#endif

#define ENABLE_DEVIL_TOWER
#define ENABLE_DEVIL_CATACOMBS
#define ENABLE_SPIDER_DUNGEON
#define ENABLE_FLAME_DUNGEON
#define ENABLE_SNOW_DUNGEON

#define ENABLE_RANKING_SYSTEM						// 

#define ENABLE_DAWNMIST_DUNGEON						// Temple of Ochao

#define ENABLE_GUILD_DRAGONLAIR_SYSTEM				// 
	#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	#	define ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM			// 
	#	define ENABLE_BLUEDRAGON_RENEWAL					// 
	#endif

#define ENABLE_BATTLE_FIELD							// 
#define ENABLE_12ZI									// Zodiak Dungeon
	#ifdef ENABLE_12ZI
	#	define ENABLE_CHAT_MISSION_ALTERNATIVE				// 
	#	define ENABLE_SERVERTIME_PORTAL_SPAWN				// 
//	#	define ENABLE_12ZI_SHOP_LIMIT
//	#	define ENABLE_12ZI_REWARD_TABLE
	#endif
#define ENABLE_DEFENSE_WAVE							// Schiffverteidigung
	#ifdef ENABLE_DEFENSE_WAVE
	#	define ENABLE_NEW_CHAT_DUNGEON						// 
	#	define ENABLE_PARTY_FEATURES						// 
	#endif
#define ENABLE_QUEEN_NETHIS							// Schlangentempel
#define ENABLE_WHITE_DRAGON							// Alastor
#define ENABLE_SUNG_MAHI_TOWER						// Sungmahi Tower




//--------OPTIONS------->
#define ENABLE_EXTEND_INVEN_SYSTEM					// 
	#ifdef ENABLE_EXTEND_INVEN_SYSTEM
	#	define ENABLE_EXTEND_INVEN_ITEM_UPGRADE				// 
	#	define ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV	// Special Inv Unlock Extension
	#	define ENABLE_BELT_INVENTORY						// 
	#else
	#	define ENABLE_EXTEND_INVEN_PAGE_SYSTEM				// 
	#endif
#define ENABLE_BELT_INVENTORY_RENEWAL				// Offical Belt Extension
#define ENABLE_ADDITIONAL_EQUIPMENT_PAGE			// Offical Inventory rework
#define WJ_NEW_USER_CARE							// 
#define ENABLE_AFTERDEATH_SHIELD					// Shield after Dead
	#ifdef ENABLE_AFTERDEATH_SHIELD
	#	define AFTERDEATH_SHIELD_ONLY_WARMAP				// 
	#endif
#define ENABLE_EXPRESSING_EMOTION					// Neue Emotionen
#define ENABLE_CHAT_SETTINGS						// 
#define ENABLE_SHOW_GUILD_LEADER					// Leader, CoLeader Anzeige
#define ENABLE_GUILDRENEWAL_SYSTEM					// Guild-Extension like Offical
	#ifdef ENABLE_GUILDRENEWAL_SYSTEM
	#	define ENABLE_USE_MONEY_FROM_GUILD					// Pay with GuildMoney
	#	define ENABLE_NEW_WAR_OPTIONS						// New GuildWar Options (Round, Points, Time)
	#	define ENABLE_EXTENDED_GUILD_LEVEL					// Extend Guild-Lv up to 30

	#	define ENABLE_MEDAL_OF_HONOR						// New Guild currency
	#	define ENABLE_GUILD_DONATE_ATTENDANCE				// Daily-Donate

	#	define ENABLE_GUILD_WAR_SCORE						// War-Page
	#	define ENABLE_GUILD_LAND_INFO						// LandInfo-Page
	#	define ENABLE_GUILDBANK_LOG							// GuildLogs

	#	define ENABLE_GUILDBANK_EXTENDED_LOGS				// Log.cpp Link
	#	define ENABLE_EXTENDED_RENEWAL_FEATURES				// Change GuildMaster | Delete Land
	#	define ENABLE_COLEADER_WAR_PRIVILEGES				// CoLeader have the same Rights, as Leader, when Leader is offline
	#	define ENABLE_GUILDWAR_BUTTON						// GK-Creating -> Button is activating in Game
//	#	define ENABLE_GUILD_RANKING							// unfinished
	#endif
#define ENABLE_HUNTING_SYSTEM						// Jagdauftr‰ge
	#ifdef ENABLE_HUNTING_SYSTEM
	#	define HUNTING_MISSION_COUNT 90
	#	define HUNTING_MONEY_TABLE_SIZE 9
	#	define HUNTING_EXP_TABLE_SIZE 9
	#endif
#define ENABLE_WORLD_LOTTERY_SYSTEM					// Lotto
	#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
	#	define NEW_LOTTERY_NUMBERS_ACTIVATE	1				// Neue ziehungen finden statt
	#	define LOTTO_PUT_TICKET_COST_PERC_TO_JACKPOT 75		// Wie viel Prozent von den Ticketkosten in den n‰chsten Jackpot gehen

	//	Percent per same number (1 = 0,01% || 10 = 0,1% || 100 = 1% > 5% from 100.000.000 = 5.000.000)
	#	define PERC_REWARD_FOR_1_SAME_NUMBER 3				// Percent reward of Jackpot for 1 same number
	#	define MAX_REWARD_FOR_1_SAME_NUMBER 5000000			// Max reward of Jackpot for 1 same number ( Deactivate when is 0 )
	#	define PERC_REWARD_FOR_2_SAME_NUMBER 10				// Percent reward of Jackpot for 2 same number
	#	define MAX_REWARD_FOR_2_SAME_NUMBER 0				// Max reward of Jackpot for 2 same number ( Deactivate when is 0 )
	#	define PERC_REWARD_FOR_3_SAME_NUMBER 100			// Percent reward of Jackpot for 3 same number
	#	define MAX_REWARD_FOR_3_SAME_NUMBER 0				// Max reward of Jackpot for 3 same number ( Deactivate when is 0 )
	#	define GENERATE_NEW_LOTTO_NUMBERS_PULSE_MIN 2		// In welchen abst‰nden neue Lottonummern generiert werden
	#endif
#define ENABLE_SET_ITEM								// 
#define ENABLE_SAFEBOX_IMPROVING					// 
//#define ENABLE_SAFEBOX_MONEY						// 
#define ENABLE_REFINE_MSG_ADD						// 
#define ENABLE_PORTAL_LEVEL_LIMIT					// 
#define WJ_SHOW_PARTY_ON_MINIMAP					// Show party member on atlas
#define ENABLE_AFK_MODE_SYSTEM						// Afk Mode
#define ENABLE_VIEW_EQUIP_RENEWAL					// 
#define ENABLE_LOADING_TIP							// Loading Tips
#define ENABLE_TARGET_AFFECT_SHOWER					// Sanii
#define ENABLE_VIEW_TARGET_DECIMAL_HP				// HP Anzeige @Monster
#define ENABLE_SEND_TARGET_INFO						// Monster Target Drop Information
	#ifdef ENABLE_SEND_TARGET_INFO
	#	define ENABLE_COMMON_DROP_INFO						// L‰dt common_drop_item.txt
	#endif
#define ENABLE_DELETE_FAILURE_TYPE					// 
#define ENABLE_GIVE_BASIC_ITEM						// Basic Items Give
	#ifdef ENABLE_GIVE_BASIC_ITEM
	#	define GIVE_BASIC_SKILLS							// Horse[11], Language[P]
//	#	define GIVE_BASIC_EXTENDED_ITEMS					// Erweitertes Equipment

	#	define BLOCK_ADDCHANGE_ATTR_ON_BASIC				// active: Items aren't able to change bonus
//	#	define BLOCK_DESTRY_ATTR_ON_BASIC					// active: Items aren't able to destroy
//	#	define BLOCK_REFINE_ON_BASIC						// active: Items aren't able to refine
	#endif
#define ENABLE_DICE_SYSTEM							// enable dice system: if the mob is king or boss and you're in party, the dropped item is randomly rolled
#define ENABLE_PARTY_EXP_DISTRIBUTION_EQUAL			// Gleichm‰ﬂige Gruppenverteilung
#define ENABLE_RENEWAL_SHOPEX						// Erweiterter Shop [Extend: Item, Exp]
#define ENABLE_ANTI_EXP_RING						// Anti-Exp-Ring
#define ENABLE_INVENTORY_SORT						// Inventar Sortier Button
#define ENABLE_CUBE_PERCENT_INFO					// Percent on Cube Window
#define ENABLE_AFFECT_POLYMORPH_REMOVE				// Remove when click on Polymorph on left Top Icon
#define ENABLE_AFFECT_BUFF_REMOVE					// Remove when click on Buff on left Top Icon
#define ENABLE_SKILL_COOLTIME_UPDATE				// Skill Cool Times are reset when the player dies
//#define ENABLE_COLORED_GUILD_RANKS				// Guildranks are colored
//#define ENABLE_GROUPLOOT_ONLY_FOR_ATTACKERS		// only Players which attacks get loot
#define ENABLE_OX_RENEWAL							// 
//#define OX_REWARD_UPDATE							// OX Update [Mali] (better Quest)
//#define ENABLE_UNMOUNT_WHEN_DEAD					// 
#define ENABLE_EXTENDED_ITEMNAME_ON_GROUND			// Itemnames on Ground
#define ENABLE_EMPIRE_FLAG							// Empire Flags on Global Chat
#define ENABLE_MESSENGER_BLOCK 						// Messenger block
#define ENABLE_GM_MESSENGER_LIST					// GameMaster Online/Offline List
#define ENABLE_MONARCH_AFFECT						// Monarch Ingame Effect





//////////////////////////////////////////////////[STANDARD DEFINES]//////////////////////////////////////////////////
/*<-----DB----->*/
#define ENABLE_DEFAULT_PRIV							//	[ClientManager.cpp]			enable default priv loading from common.priv_settings
#define ENABLE_ITEMAWARD_REFRESH					//	[ClientManager.cpp]			enable a select query every 5 seconds into player.item_award
#define ENABLE_PROTO_FROM_DB						//	[ClientManager.h]			read protos from db if "PROTO_FROM_DB = 1" is specified inside conf.txt
													//								mirror protos to db if "MIRROR2DB = 1" is specified inside conf.txt
#define ENABLE_AUTODETECT_VNUMRANGE					//	[ClientManagerBoot.cpp]		if protos are loaded from db, it will automatically detect the vnum range for ds items


/*<-----GAME----->*/
#define NEW_ICEDAMAGE_SYSTEM						//	[char.h]					add new system for nemere dungeon and so on
#define ENABLE_ANTI_CMD_FLOOD						//	[char.h]					limit player's command execution to 10 commands per second, otherwise it'll be disconnected!
#define ENABLE_OPEN_SHOP_WITH_ARMOR					//	[char.h]					if enabled, people can open a personal shop with the armor equipped
#define ENABLE_GOHOME_IF_MAP_NOT_ALLOWED			//	[char.cpp]					you'll go back to your village if you're not allowed to go in that map
#define ENABLE_GM_FLAG_IF_TEST_SERVER				//	[char.cpp]					show the gm flag if it's on test server mode
#define ENABLE_GM_FLAG_FOR_LOW_WIZARD				//	[char.cpp]					GM Zeichen auch als Low-Wizard
// #define ENABLE_POTIONS_LEVELUP					//	[char.cpp]					Pots Loot bei Lvl-Up
#define ENABLE_NEWEXP_CALCULATION					//	[char_battle.cpp]			recalculate exp rate so you won't get random negative exp/marriage points
#define ENABLE_EFFECT_PENETRATE						//	[char_battle.cpp]			enable penetrate effect when performing a penetration
#define ENABLE_KILL_EVENT_FIX					//	[char_battle.cpp]			if you want to fix the 0 exp problem about the when kill lua event (recommended)
// #define ENABLE_FIREWORK_STUN						//	[char_item.cpp]				enable stun affect when using firework items
#define ENABLE_ADDSTONE_FAILURE						//	[char_item.cpp]				enable add stone failure
#define ENABLE_ADDSTONE_FAILURE_CUSTOM				//	[char_item.cpp]				enable add stone failure +0,+1,+2,+3,+4
#define ENABLE_EFFECT_EXTRAPOT						//	[char_item.cpp]				enable extrapot effects when using green/purple potions
#define ENABLE_BOOKS_STACKFIX						//	[char_item.cpp]				enable stackable books instead of remove all the pile
#define ENABLE_STACKABLES_STONES					//	[char_item.cpp]				enable stackable spirit stones instead of remove all the pile
#define ENABLE_FORCE2MASTERSKILL					//	[char_skill.cpp]			skill always pass to m1 when b17 instead of b(number(17-20))
// #define ENABLE_MOUNTSKILL_CHECK					//	[char_skill.cpp]			check whether horse mount vnum should be checked when skilling
// #define ENABLE_NULLIFYAFFECT_LIMIT				//	[char_skill.cpp]			sura skill 66 won't nullify players with level < or > of yours by 9
// #define ENABLE_MASTER_SKILLBOOK_NO_STEPS			//	[char_skill.cpp]			if enabled, you will only need a book to increase a master skill, and not as many as the level-20
#define ENABLE_CMD_PLAYER							//	[config.cpp]				enable PLAYER grade inside CMD
#define ENABLE_EXPTABLE_FROMDB						//	[config.cpp]				read the exp table from the db
#define ENABLE_AUTODETECT_INTERNAL_IP				//	[config.cpp]				autodetect internal ip if the public one is missing
// #define ENABLE_GENERAL_CMD						//	[config.cpp]				if enabled, it reads a general CMD from "locale/%s/conf/GENERAL_CMD", "locale/%s/conf/GENERAL_CMD_CHANNEL_%d", and/or "locale/%s/conf/GENERAL_CMD_CHANNEL_%d_HOSTNAME_%s"
// #define ENABLE_GENERAL_CONFIG					//	[config.cpp]				if enabled, it reads a general CONFIG from "locale/%s/conf/GENERAL_CONFIG", "locale/%s/conf/GENERAL_CONFIG_CHANNEL_%d", and/or "locale/%s/conf/GENERAL_CONFIG_CHANNEL_%d_HOSTNAME_%s"
													//								in the GENERAL_CONFIG, all the options are valid except: HOSTNAME, CHANNEL, PLAYER_SQL, COMMON_SQL, LOG_SQL, PORT, P2P_PORT, MAP_ALLOW, AUTH_SERVER
// #define ENABLE_BLOCK_CMD_SHORTCUT				//	[cmd.cpp]					if enabled, people won't be able to shorten commands
// #define ENABLE_STATPLUS_NOLIMIT					//	[cmd_gm.cpp]				disable only 90 points for con+/int+/str+/dex+ commands
#define ENABLE_SET_STATE_WITH_TARGET				//	[cmd_gm.cpp]				enable set_state target as 3rd arg
#define ENABLE_CMD_IPURGE_EX						//	[cmd_gm.cpp]				/ipurge 2nd arg can remove items from a specific window (inv/equip/ds/belt/all)
// #define ENABLE_SPAMDB_REFRESH					//	[db.cpp]					enable a select query every 10 minutes into common.spam_db
#define ENABLE_FISHINGROD_RENEWAL					//	[fishing.cpp]				if the upgrading of the fishing rod will fail, it won't turn back of 1 grade, but just lose 10% mastering points.
#define ENABLE_NEWGUILDMAKE							//	[guild.h]					enable pc.make_guild0 and disable CInputMain::AnswerMakeGuild
#define ENABLE_INFINITE_HORSE_HEALTH_STAMINA		//	[horse_rider.h]				the horse wonít lose health nor stamina
// #define ENABLE_ACCOUNT_W_SPECIALCHARS			//	[input_auth.cpp]			enable special characters in account names (account.account.login)
#define ENABLE_GOHOME_IF_MAP_NOT_EXIST				//	[input_db.cpp]				you'll go back to your village if the map doesn't exist
#define ENABLE_CHAT_COLOR_SYSTEM					//	[input_main.cpp]			enable chat colors based on IsGm or GetEmpire (+colored empire name)
#define ENABLE_CHAT_SPAMLIMIT						//	[input_main.cpp]			limit chat spam to 4 messages for 5 seconds, if you spam it for 10 times, you'll be disconnected!
#define ENABLE_WHISPER_CHAT_SPAMLIMIT				//	[input_main.cpp]			limit whisper chat to 10 messages per 5 seconds, otherwise you'll be disconnected!
#define ENABLE_CHAT_LOGGING							//	[input_main.cpp]			enable chat logging (which saves all the gm chats)
#define ENABLE_IMMUNE_FIX							//	[item.cpp]					fix immune bug where you need to equip shield at last (or refresh compute e.g. un/riding horse)
#define ENABLE_FIX_READ_ETC_DROP_ITEM_FILE_BY_VNUM	//	[item_manager_read_tables]	ReadEtcDropItemFile: No such an item, because of korean names
#define ENABLE_PICKAXE_RENEWAL						//	[mining.cpp]				if the upgrading of the pickaxe will fail, it won't turn back of 1 grade, but just lose 10% mastering points
#define ENABLE_QUEST_DIE_EVENT						//	[quest.h]					add quest event "die"
#define ENABLE_QUEST_BOOT_EVENT						//	[quest.h]					when boot begin trigger when game starts
#define ENABLE_QUEST_DND_EVENT						//	[quest.h]					when itemvnum.dnd begin trigger to create scrolls via quest
#define ENABLE_TRANSLATE_LUA						//	[questlua.cpp]				enable translate.lua loading
// #define ENABLE_LOCALECHECK_CHANGENAME			//	[questlua_pc.cpp]			enable check that unable change name on Europe Locales
// #define ENABLE_PC_OPENSHOP						//	[questlua_pc.cpp]			enable pc.open_shop0(idshop) but buy/sell not work yet
#define D_JOIN_AS_JUMP_PARTY						//	[questlua_dungeon.cpp]		x
#define ENABLE_QUESTLIB_EXTRA_LUA					//	[questlua.cpp]				x
#define ENABLE_PARTYKILL							//	[questmanager.cpp]			re-enable PartyKill
// #define ENABLE_SHOP_BLACKLIST					//	[shop.cpp]					enable ignore 70024 (Blessing Marble) and 70035 (Magic Copper Ore)
#define ENABLE_NEW_RETARDED_GF_START_POSITION		//	[start_position.cpp]		New Gameforge Startposition
#define ENABLE_D_NJGUILD							//	[general]					enable d.new_jump_all_guild+cpp relative functions (untested)
#define ENABLE_FULL_NOTICE							//	[general]					enable new big notice features
#define ENABLE_NEWSTUFF								//	[general]					enable new stuff (new lua funcs, new CONFIG options, ecc)
#define ENABLE_NEWSTUFF_2017						//
#define ENABLE_NEWSTUFF_2019						//
#define ENABLE_CMD_WARP_IN_DUNGEON					//	[general]					/warp <player> will warp successfully even if the player is inside a dungeon (be sure a .quest login event won't still warp you out)

/*
@#GENERAL MACROS
#define __UNIMPLEMENTED__					//drafts of new things to be implemented
*/

/*
#@general
@warme001: be aware about PLAYER_MAX_LEVEL_CONST (common/length.h) and gPlayerMaxLevel (game/config.h)
@warme002: be aware about ITEM_MAX_COUNT (common/item_length.h) and g_bItemCountLimit (game/config.h)
@warme003: do_click_safebox can be used by PLAYER in every map!
@warme004: `when vnum.kill begin` and `when kill begin` are both triggered
@warme005: different locale stuff
@warme007: on db/src/ClientManager.cpp; commented locale set from common.locale due to its uselessness and bugginess (./close && ./start)
			it processes a nullptr mysql connection (dat ymir threading) if there was a bit of overload before starting the process up again
@warme008: on char_item.cpp; now 27996 (poison bottle) can inflict poison
@warme009: on char_resist.cpp; if bleeding is used as poison, the bleeding enchantment is poison enchantment/50 (so mobs can bleed players)
@warme010: on char_state.cpp; test_server is used as "BOOL g_test_server"
@warme011: on dungeon.cpp; you should never use d.join instead of d.new_jump_party since it causes random crashes due to a wrong implementation of the party hash check
@warme012: trivial errors are now considered as simple logs
@warme013: unneccessary errors are now simply commented


#@common
@fixme301: on tables.h; TPlayerTable hp/mp from int16_t to int (hp/mp >32767 should be fixed)

#@db/src
@fixme201: on ProtoReader.cpp; changed 'SAMLL' into 'SMALL'
@fixme202: on ClientManagerGuild.cpp; fixed the guild remove member time issue if the player was offline
			(withdraw_time -> new_withdraw_time)
@fixme203: on ClientManagerPlayer.cpp; dandling pointer for "command"
@fixme204: on Cache.cpp; myshop_pricelist primary key duplication error if there are many items of the same vnum in the personal shop

#@game/src
@fixme101: on log.cpp; fixed '%s' for invalid_server_log
@fixme102: on cmd_general.cpp; inside ACMD(do_war) fixed the unsigned bug
@fixme103: on config, input_login, input_main.cpp; fixed clientcheckversion (version > date) to (version != date) and delay from 10 to 0
@fixme104: on char.cpp, questlua_pc.cpp; fixed get status point after lv90 changing 90 with gPlayerMaxLevel
@fixme105: on cmd.cpp; disabled every korean command
@fixme106: on input_main.cpp; if a full-speeded player is on a mount (es. lion), he'll be brought back due to the distance range
@fixme107: on char_battle.cpp; if character (player|mob) has negative hp on dead, sura&co will absorb hp/mp losing 'em themselves
@fixme108: on char.cpp; if you change a mount but the previous is not 0, all the entities (npcs&co) in the player client
			(not others) are vanished until another refresh (not exists mounts still bug you after second mount call)
@fixme109: on questmanager.cpp; if you kill a player (war m), `when kill begin` will be triggered twice
@fixme110: on char_affect.cpp; if you attack when semi-transparent (revived or ninja skill or white flag) you'll still be transparent
@fixme111: on test.cpp; ConvertAttribute2 has x and y inverted (before y->x after x->y)
@fixme112: on char_item.cpp; you can change bonuses in equipped items too (until re-login)
			bonus values will not be refreshed by ChangePoint and unequipping it will remove back only the new bonuses set on
			e.g. you had a 2500hp bonus shoes, you changed it to 50mp when equipped and you'll unequipped
			what will it happen? instead of remove 2500hp, you won't receive 50mp and you also lose 50mp when unequipped
@fixme113: on char_item.cpp; same thing of #112
			you can remove stones from equipped items w/o losing bonuses
			e.g. have an item with antiwar+4 equipped:
			1) remove all the stones
			2) unequip it
			3) re-add stone
			4) re-equip it
			5) repeat it thrice
				result? an item with no stones but you'll have 75% of antiwar
@fixme114: on char_item.cpp; gathering of #111, #112 and few others.
@fixme115: on char_item.cpp; you can retrieve all the item on the ground if you're in a party and the owner is not in yours.
@fixme116: on char_skill.cpp; normal horse mount skills cannot inflict damage
@fixme117: on char_item.cpp; you can't swap equipment from inventory if full, and also prevent unmotivated belt swap if its inventory is not empty
@fixme118: on char.cpp; when ComputePoints is called:
			you'll gain as many hp/mp as many you have in your equipment bonuses
			affect hp/mp will be lost when login or updating
@fixme119: on input_main.cpp; you can put items from safebox/mall to belt inventory w/o checking the type (items with size>1 are not placeable anyway)
@fixme120: on input_login.cpp; few packet IDs not checked
@fixme121: on char_item.cpp; the refine scroll item value 1 from the magic stone was generating useless syserrs
@fixme122: on arena.cpp; few other potions were not checked on arena map
@fixme123: on char_item.cpp; USE_CHANGE_ATTRIBUTE2 (24) sub type check bug (the condition could never be true)
@fixme124: on char_item.cpp; no check on 6-7 add/change items about costume stuff
@fixme125: on char.cpp; dungeon regen pointing to a dangling pointer (not required -> removed)
@fixme126: on marriage.cpp; fix lovepoints overflow
@fixme127: on cube.cpp; /cube r_info exploit fix; it can cause a crash due to an unchecked cube npc masters vnums
			1) you open the Baek-Go cube's console
			2) click on an npc/kill a mob without close the cube console
			3) digit /cube r_info 4) crash core
@fixme128: on char.cpp; mining hack fix; you can mine a vein anywhere in the map because there's no check on the character
			which means, you can stay at 0x0y and mining a vein in 666x999y and get the stuff beside him or in the pc's inventory
@fixme129: on PetSystem.cpp; the azrael pets (53005->34004 normal/53006->34009 gold) don't give the buff if not in dungeon at summon up and remove them anyway when unsummoned
@fixme130: on messenger_manager.cpp; and cmd_general.cpp if you do /messenger_auth n XXX, the player with the name XXX will receive a "refused friend invite" print from you
			which means, if you flood this packet, the "victim" will be disconnected or at maximum could get lag
@fixme131: on char.cpp; fix annoying sync packets sendable even on unfightable pc/npc entities
			e.g. wallhack against players' shops inside the village's squares (where the NOPK attr is set) to move them out and kill them
@fixme132: on shop.cpp; if two people buy the same item at the same time from a pc's shop, the slower one will receive a wrong return packet (crash client)
@fixme133: on input_main.cpp; banword check and hyper text feature were processing the final chat string instead of the raw one
@fixme134: on questlua_pc.cpp; the pc.mount_bonus was addable even if the mount wasn't spawn (only /unmount pc.unmount can remove it)
@fixme135: on char.cpp; if the Sync is made before a move packet and the sectree differs of few x/y coordinates, the sectree will be changed without update (crash character) (troublesome -> removed)
@fixme136: on char.cpp; there are no checks about the zero division exception: e.g. if you set a mob's max hp to 0 in the mob proto, you'll get random crashes.
@fixme137: on char_battle.cpp; when a player dies, the HP could have a negative value. Now it's 0 like the official.
@fixme138: on db.cpp, input_auth.cpp; the account's password was shown in the mysql history queries as clear text at every login attempt (mysql full granted user required -> now hashed)
@fixme139: on shop.h; CShop class destructor wasn't virtual. If a derived class like CShopEx was deleted, a memory leak would have been generated.
@fixme140: on input_main.cpp; the belt could be put into the safebox even though the belt inventory isn't empty.
@fixme141: on char_item.cpp; the items in the belt inventory could be used even if their slot were not available
@fixme142: on messenger_manager.cpp; sql injection fix about net.SendMessengerRemovePacket
@fixme143: on guild_manager.cpp; sql injection fix about net.SendAnswerMakeGuildPacket
@fixme144: on sectree_manager.cpp; if map/index doesn't end with a newline, the game will crash
@fixme145: on input_main.cpp; guild_add_member can add any vid as guild's member even if it's a mob or an npc
@fixme147: on char_item.cpp; ramadan candy item can be used even if the relative affect is still up
@fixme148: on item_manager_read_tables.cpp; type quest, special, attr not handled in ConvSpecialDropItemFile
@fixme149: on char.cpp; refine material skip exploit if items are swapped
@fixme150: on exchange.cpp; char_item.cpp; prevent item module swapping if the quest is suspended
@fixme152: on questlua_pc.cpp; pc.get_special_ride_vnum was checking socket2 instead of socket0
@fixme153: on threeway_war.cpp; kills made of people over lvl99 weren't counted
@fixme154: on cmd_gm.cpp; the /all_skill_master command will now set the right amount of points to the sub skills
@fixme156: on char_affect.cpp; prevent doubling the affect values before they are loaded (such as pc.mount_bonus at login; because the quest is loaded before the quests)
@fixme157: on OxEvent.cpp; the attender list wasn't cleared after stopping the ox event
@fixme158: on input_main.cpp; the deviltower refiner won't set the flag to 0 anymore if you have no money, and it will decrease it by 1 for allowing multiple refine attempts
@fixme159: on exchange.cpp; when exchanging, a wrong check in the ds items was not allowing the exchange due to "not enough space in ds inventory" if the first sub ds inventory slot was not empty
@fixme160: on DragonSoul.cpp; when removing a ds stone, if the destination slot wasn't empty, the ds item in there would have been replaced and lost
@fixme168: on questevent.cpp; if the quest info name is already null, the std::string constructor will throw an exception
@fixme169: on char_item.cpp; mythical peach alternative vnum
@fixme170: on item.cpp; Fix of bonus application like Official on special mineral slots.
@fixme171: on dungeon.cpp; previously (!ch->IsPC())
@fixme172: on dungeon.cpp; addet ch->IsPC() before sending
@fixme173: on item.cpp; Range_pick extend 300 to 600
@fixme174: on questmanager.cpp; ch check
@fixme177: on cmd_gm.cpp; import correct msg
@fixme180: on cmd_general.cpp; /costume will cause game core crashes if the relative costume bonus ids aren't present inside fn_string or have no %d
@fixme182: on input_login.cpp;
@fixme183: on input_main.cpp, messenger_manager.cpp; remove friendlist -> removed from companion too
@fixme185: on ClientManagerBoot.cpp; addet material_count if nullptr
@fixme186: on item.cpp; ch check
@fixme188: on char.h; No Reward on dead!
@fixme189: on item_manager.cpp; fix out of range

@fixme400: on item_manager.cpp; Bonus on Rings visable
@fixme401: on char_item.cpp; 2 rings with same vnum->fixxed
@fixme402: on cmd_gm.cpp; Berechnung der Rangpunkte angepasst(ch->tch)
@fixme403: on input_main.cpp; EXP - Verteilungeinstellung nur fur den Gruppenleader
@fixme404: on char_item.cpp; Pots kˆnnen nicht mehrfach verwendet werden
@fixme405; on char.cpp; 50 % nach Max - Level noch an EXP sammeln mˆglich
@fixme406; on char_affect.cpp; Ignore lower buffs
@fixme407; on char.cpp; SHOP bundle never let this get into the shop
@fixme408; on cmd_general.cpp; GM_IMPLEMENTOR kˆnnen sofort town / hier - neustarten
@fixme409; on char_item.cpp; make sure, you can't pickup even you're dead
@fixme410; on char_item.cpp; Schmied Distance Check
@fixme411; on item_length.h; Extended Name Max Len
@fixme412; on length.h; Extended Character Max Len
@fixme413; on length.h; Missing Window Type at ValidItemPosition
@fixme414; on char_item.cpp; stack potts directly in belt inventory
@fixme415; on input_login.cpp; falscher Map_index (2)
@fixme416; on input_main.cpp; equiped items->Safebox blocked
@fixme417;
@fixme418; on char.cpp; Stack items you drop for Other guys in your party
@fixme419; on desc_manager.cpp; Fix Desc find PC
@fixme420; on config.cpp; disabled interpreter_set_privilege[load CMD privileges]
@fixme421; on input_login.cpp; GM & invisible, let's set observer!
@fixme422; on char_item.cpp; missing MonkeyDungeon
@fixme423; on char_battle.cpp; Buffs without weapon disabled[VZK, AURA]
@fixme424; on char_item.cpp; Cant change Attributes on equipped items
@fixme425; on char.cpp; Dungeon Corecrash
@fixme426; on char_battle.cpp; Correct Alignment calculation
@fixme427; on belt_inventory_helper.h; allows blend on belt inventory
@fixme428; on questlua_pc.cpp; Remove Affect when change Skillgroup
@fixme429; on input_main.cpp; Nullvalue->Fixxing Kick on Comboattackhack
@fixme430; on PetSystem.cpp; SetSummonItem Socket Fix
@fixme431; on char.cpp; Ghostwalker Fix
@fixme432; on char.cpp; Mining Bot Fix
@fixme433; on char_affect.cpp; Weaponry Sura's Dispel & Healing Power Shamans Heal Lag-Bug
@fixme434; on char_dragonsoul.cpp; AddAffect->Dragonsoul Active Effect
@fixme435; on char_battle.cpp; Poison won't draw monsters aggro anymore
@fixme436; on char_item.cpp; Gl¸ckstasche missing->addet new protos
@fixme437; on char_item.cpp; Expand Stone Range - Missing: Lykaner, Magiebruch
@fixme438; on char_item.cpp; Summon Pet while porting/relog/...
@fixme439; on char_change_empire.cpp; EmpireChange Fix with Guild in other Empire(UpdatePacket)
@fixme440; on questlua_pc.cpp; VIP - System->GameMaster can't be VIP's
@fixme441; on cmd_general.cpp; Guild war negative core down bug fix
@fixme442; on questlua_pc.cpp; Autodisconnect after Change Empire
@fixme443; on char_battle.cpp; Shop Block @ Attack
@fixme444; on char_item.cpp; Missing IsExchanging
@fixme445; on char_item.cpp; Cant use Change Attr on OldHairs
@fixme446; on char_item.cpp; Guildblacksmith correct calculation + display information(ingame)
@fixme447; on char_horse.cpp; IsRiding @ StartRiding = > cant start, while sill riding
@fixme448; on dungeon.cpp; Fix Dungeon count monsters(function was keeping also NPC, it could make ugly problems)
@fixme449; on dungeon.cpp; Fix Dungeon using more memory than right, with notice on all entities(notice was be triggered for every char entity)
@fixme450; on char_item.cpp; Block marriage items while riding
@fixme451; on questlua_global.cpp; Small prevents
@fixme452; on battle.cpp; Fix: Issue on War!->addet : IsObserverMode
@fixme453; on char_battle.cpp; IsObserverMode addet
@fixme454; on PetSystem.cpp; Pet didn't disappear after Teleport/Relog
@fixme455; on char_battle.cpp; Moblock / bravery cape hack
@fixme456; on db.cpp; SQL - Injection - Fix
@fixme457; on input_main.cpp; Wolfman ComboHack - Kick Bug
@fixme458; on char_skill.cpp; Boss kick into walls
@fixme459; on char_skill.cpp; Aura / Vzk ohne Waffe->debuff
@fixme460; on char_item.cpp; gestackte Steine jweils - 1
@fixme461; on char_skill.cpp; SK no fall (1-m10) == AFF_CHEONGEUN_WITH_FALL -> AFF_CHEONGEUN
@fixme462; on char.cpp; Questflagfix eg.hide a costume
@fixme463; on input_main.cpp; cant create guild bellow 40
@fixme464; on cmd_gm.cpp; purge Pet fix
@fixme465; on char_skill.cpp; Magic Attack correct Bonus giving
@fixme466; on char.cpp; temporary fixxed mount bug(kick after riding) - Maybe.msa or proto not correct!
@fixme467; on char_item.cpp; Item Scroll success prob fix
@fixme468; on item.cpp; real_time_expire_event->after time - Costume Weapon into Inventory
@fixme469; on char_battle.cpp; cant drop anything on arenamap
@fixme470; on char.cpp; check shop name - if allowed
@fixme471; on char_item.cpp; Questtrigger "pick" richtig auslˆsen
@fixme472; on char_item.cpp; Memory Leak Fix
@fixme473; on input_login.cpp; Analyze: login phase does not handle this packet!header 100
@fixme474; on char_battle.cpp; Shooting Dragon fix
@fixme475; on Main.cpp; comment DBManager.Quit();
@fixme476; on char_battle.cpp; player can't save their scroll position (block dd maps)
@fixme477; on party.cpp; Cant teleport dead player
@fixme478; on questpc.cpp; items being "remembered" on other quest states
@fixme479; on char.h; uint16_t -> uint32_t (Acce-Fix)
@fixme480; on char.cpp; SetPosition IsDoor()
@fixme481; on char_item.cpp; check if Mount is already euqipped (UNUSED)
@fixme482; on char_item.cpp; Buffs removed wenn Polymorph()
@fixme483; on dragon_soul_table.cpp; DRAGON_SOUL_GRADE_MAX -> DRAGON_SOUL_STEP_MAX
@fixme484; on DragonSoul.cpp; DRAGON_SOUL_STEP_MAX -> DRAGON_SOUL_GRADE_MAX
@fixme485;
@fixme486; on char_battle.cpp; Prevent reflect bug abuse
@fixme487; on char_battle.cpp; new MANASHIELD calculation in %
@fixme488; on cmd_general.cpp; block Monarch warps
@fixme489; on char_item.cpp; block chest open, because invi is full
@fixme490; on char_item.cpp; Block sortinventory
@fixme491; on char_item.cpp; Item refinement crash with same material
@fixme492; on New_PetSystem.cpp; nullpointer
@fixme493; on char_skill.cpp; SKILL_FLAG_STUN fix (because of Party Buffer -> Skill duration/calculation is normaly shit)	-> actually unused, have to test!
@fixme494; on questlua_pc.cpp; Empirechange => if Party -> leave Party before!
@fixme495;
@fixme496; on questlua_global.cpp; Warp_all_to_village function was keeping also STAFF out.
@fixme497; on questlua_global.cpp; Enable Syserr also in LUA.
@fixme498;
@fixme499; on battle.cpp; Fix att range
@fixme500; on char.cpp; 
@fixme501; on cmd_gm.cpp, cube.cpp; yang-bug; uint32_t -> int
@fixme502; on char_item.cpp; campfire fix
@fixme503; on battle.cpp, pvp.cpp; wait hack
@fixme504; on char_battle.cpp; 
@fixme505; on cmd_emotion.cpp; can't do emotion while attack or moving
@fixme506; on char_skill; missing Skills addet
@fixme507; on char_item.cpp; antiexpring effect
@fixme508; on char.h; POINT_MAX more then 255 (UNUSED)
@fixme509; on questmanager.cpp; reload q fix
@fixme510; on char_battle.cpp; aggregate on cape effect
@fixme511; on char_skill.cpp; GDF fix
@fixme512; on char_skill.cpp; Ignore Aggressiv when Ninja is invisible
@fixme513; on cmd_general.cpp; 
@fixme514; on dungeon.cpp; 
@fixme515; on ; (it++ to ++it || nullptr to nullptr)
@fixme516; on item.h; (virtual void to void || &override)
@fixme517; on char_battle.cpp; DeathBlow - more damage for 3 classes without warrior
@fixme518; on cube.cpp; reload fix
@fixme519; ---; refine uint8_t to uint16_t fix
@fixme520; on char.cpp; pet doublebonus fix
@fixme521; on cube.cpp; core-crash fix
@fixme522; on char_battle.cpp | char_item.cpp; Fix Group Exp/Yang/Drop Share on different Maps
@fixme523; on ClientManager.cpp / safebox.cpp; safebox memory leak fix
@fixme524; on questlua_npc.cpp; Rare coredowner fix
@fixme525; on shop_manager.cpp; Memory Leak
@fixme526; on cmd_gm.cpp; ch -> tch
@fixme527; on item_manager.cpp; prevent core crash
@fixme528; on input_main.cpp; Quickslot fix
@fixme529; on cmd_gm.cpp; Guildbuild items useable
@fixme530; on char.cpp; Change Sex without reload
@fixme531; on char.cpp; Dupli-fix when move channel
@fixme532; on x; Extension 255 Bonus +
@fixme533; on marriage.cpp, questlua_marriage.cpp; wedding fixxes
@fixme534; on char_battle.cpp; Crash Resist event fix
@fixme535; on char_item.cpp; AutoGiveItem fix
@fixme536; on questlua_pc.cpp; Status-Reset -> -1 Statuspunkt ist veraltet!
@fixme537; on length.h; SHOP_TAB_COUNT_MAX set to 2 -> because ShopEx bug with 3!
@fixme538; on mining.cpp; Pickup Max Exp ignore refine
@fixme539; on dungeon.cpp; only Monsters and Stones will be despawned
@fixme540; on libthecore - utils.c;	Random-Fix
@fixme541; ---; Performance rework [Prefix ++/-- operators should be preferred for non-primitive types.]
@fixme542; on item_attribute.cpp; Rare (really) core-downer fix related to rare attribute
@fixme543; on char_item.cpp; Crash Fix - Cant Receive Item while Dead!
@fixme544; on input_main.cpp; Dmg/Range Hack Fix

@custom001; on char_affect.cpp; potions display
@custom002; on polymorph.cpp, char_affect.cpp, char_item.cpp; Polymorph Map Blacklist
@custom003; on char_battle.cpp; Exp-Ring Bonus addet
@custom004;
@custom005; on char_horse.cpp; IS_MOUNTABLE_ZONE
@custom006; on char_item.cpp; Can't use StandardPet on specific maps [IS_BLOCKED_PET_SUMMON_MAP]
@custom007; on char_item.cpp; Block -> Change/Add Attributes [Jungheld fix]
@custom008; on char_item.cpp; MoveItem -> DestCell
@custom009; on char_item.cpp; belt_allow_all_items
@custom010; on char_skill.cpp; PAERYONG SKILL center in Enemy
@custom011; cmd_gm.cpp; user coordinates listed
@custom012; on char_item.cpp; Disabling Mounts in Threeway War
@custom013; on char_manager.cpp; Reset Whisper Counter
@custom014; on char_state.cpp; New Respawn Settings
@custom015; on cmd_emotion.cpp; Emotion with same Gender
@custom016; on cmd_emotion.cpp; Emotion without a Mask
@custom017; on cmd_general.cpp; Horse Checker
@custom018;
@custom019; on desc_client.cpp; Channel Status Update Period
@custom020; on exchange.cpp, shop.cpp; Trade Effect
@custom021; on char_item.cpp, fishing.h; new perls
@custom022; on input_main.cpp; frozen Chat
@custom023; on input_main.cpp; Disallow shops on certain maps	(CanOpenShopHere)
@custom024; on item_attribute.cpp; attribute_double_change
@custom025; on questlua_pc.cpp; Stripped Powers of GM per command [/strip]
@custom026; on char_item.cpp; enables Rare-Bonus
@custom027; on char_item.cpp; RemoveBadAffect @Vnum[27874]
@custom028; on char_item.cpp; Alle "Armor-Items" Switchbar!
@custom029; on char_item.cpp; attr_always_add => 
@custom030; on char_item.cpp; attr_always_5_add => 
@custom031; on char_item.cpp; 4 Bonus percent
@custom032; on char_item.cpp; Can't use water & blend together
@custom033; on input_login; OX-Event No Entry
@custom034;
@custom035;
@custom036; on shop.cpp; Autoclose after SoldOut
@custom037; 
@custom038; on cmd_general.cpp; sort Wait
@custom039; on char_item.cpp; Autopott Effect


@infome000; Something maybe interest
@infome001; on char_item.cpp; Weihnachtstruhe TEST
*/
#endif

