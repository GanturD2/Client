#pragma once
#define LOCALE_SERVICE_EUROPE

enum eCommon
{
	//DEFINES						//VALUE			// DESCRIPTION								// DEFAULT
	MAX_HIT_COUNT = 32,			// HitCount pro Schlag						// 16
	QUEST_WAIT_TIME = 0,			// Quest Text Zeit							// 10
	PICKUP_DELAY = 300,			// Pickup Zeit								// 500
};

//------NOT_FINISHED------>
//#define ENABLE_PVP_BALANCE
#define WJ_SHOW_NPC_QUEST_NAME
// #define __ATTACK_SPEED_CHECK__
#define ENABLE_LOOTING_SYSTEM




//////////////////////////////////////////////////[ONLY BINARY]//////////////////////////////////////////////////
//-------SERVICE------->
//#define __USE_CYTHON__								// Root-Cythonization
//#define __USE_EXTRA_CYTHON__						// UiScript-Cythonization
//#define ENABLE_DEBUG_GUI							// Debug-Infos: "Coordinate, MousePosition, FPS, ..."
#define ENABLE_EXTENDED_SYSERR						// 
#define ENABLE_SINGLETON_INFORMATION				// marty 1 - singleton ymir retarded version 0
#define ENABLE_NO_INDEX_FILE						// read pack/Index from source instead of file
//#define USE_EPK_NEW_SIGNATURE 1234					// for hook signature change
//#define ENABLE_PACK_IMPORT_MODULE					// will fully remove pack.Get when enabled.
#define ENABLE_PREVENT_FILE_READ_FROM_DRIVE			// will block loading files from the d: drive.
//#define USE_PACK_TYPE_DIO_ONLY						// disable the old 0-1-2-3-4-5 types by enabling the define
#define ENABLE_PACK_TYPE_DIO						// The encryption is inside a static .lib, and the rest as source code via define





//------SECURITY------>
//#define ENABLE_CSHIELD
#ifdef ENABLE_CSHIELD
#	define ENABLE_CHECK_ATTACKSPEED_HACK
#	define ENABLE_CHECK_MOVESPEED_HACK
#endif




//-------STUFF------->
#define ENABLE_PACK_GET_CHECK						// enable py|pyc|txt check on pack.Get
#define ENABLE_TELEPORT_CRASH_FIX					// 
//#define ENABLE_SYSLOG_PACKET_SENT					// 
#define ENABLE_CRASHRPT								// 
#define ENABLE_IE11_UPDATE							// upgrade IE7->IE11
#define HIDE_GRANNY_SYSERR							// 
//#define DISABLE_PROPERTY_SYSERR					// 
#define ENABLE_BINARY_SERVERINFO					// 
#define ENABLE_VIEW_ITEM_VNUM						// 





//--------SYSTEMS------->
#define WJ_ENABLE_PICKUP_ITEM_EFFECT				// 
#define ENABLE_ENERGY_SYSTEM						// Energie-System
#define ENABLE_REFINE_RENEWAL						// Auto Refine
#define ENABLE_ENVIRONMENT_RAIN						// Raindrops





//--------OPTIONS------->
#define ENABLE_OFFICAL_FEATURES						// 
#define ENABLE_REVERSED_FUNCTIONS					// 
//#define ENABLE_REVERSED_FUNCTIONS_EX				// unfinished
#define ENABLE_IMAGE_CLIP_RECT						// 
//#define ENABLE_SKIP_MOVIE							// 
#define ENABLE_RENDER_TARGET						// Render Target
#ifdef ENABLE_RENDER_TARGET
#	define ENABLE_RENDER_TARGET_EFFECT					// 
#	define ENABLE_RENDER_LOGIN_EFFECTS					// 
//#	define ENABLE_RENDER_YUTNORI
#endif
#define ENABLE_NEW_USER_CARE						// 
#define ENABLE_EXTENDED_DMG_EFFECT					// Extended Crit | Pene | Poison dmg effect
#define ENABLE_NEW_EQUIPMENT_SYSTEM					// Ring1, Ring2, BELT
#define NON_CHECK_DRAGON_SOUL_QUALIFIED				// xXxXxXxXxXxXxXxXxX
#define ENABLE_DS_REFINE_WINDOW						// 
#define ENABLE_SOCKET_STRING3						// Allows 3rd Socket
#define ENABLE_EMOJI_TEXTLINE						// p3ng3r
#define ENABLE_DETAILS_UI							// BonusBoard Webzen
#define ENABLE_KEYCHANGE_SYSTEM						// Tastatursystem
#define ENABLE_TARGET_SELECT_COLOR					// Target-Select Color System
#define ENABLE_LEVEL_IN_TRADE						// Lvl-Anzeige im Handelscenter
#define ENABLE_MESSENGER_GET_FRIEND_NAMES			// @Vegas
// #define LOGIN_COUNT_DOWN_UI_MODIFY				// Login Countdown
#define SIGN_FOR_PLUS_ITEMS							// Items+ (+)
#define ENABLE_PLUS_ICON_ITEM						// Displays Value of Item
#define ENABLE_TEXT_LEVEL_REFRESH					// enable text tail level refresh when levelup
#define ENABLE_SET_NO_CHARACTER_ROTATION			// Charakter Richtungs‰nderung ohne Rotation
#define ENABLE_SET_NO_MOUNT_ROTATION				// Mount Richtungs‰nderung ohne Rotation
#define ENABLE_SLOT_WINDOW_EX						// it will fix the refresh of the activated and cooltimed slots in the skill page (V) (except when switching to the equitation tab and vice versa)
#define ENABLE_TEAM_GAMEMASTER						// GM's haben statt Lv Anzeige ein [Team] vor dem Namen
#define ENABLE_NEUTRAL_RANG							// Neutraler Rang bei wenigen Rufpunkten
#define ENABLE_ENVIRONMENT_EFFECT_OPTION			// 
#define ENABLE_SHADOW_RENDER_QUALITY_OPTION			// 
//#define ENABLE_GRAPHIC_ON_OFF						// 
#define ENABLE_EXTENDED_CONFIGS						// Money, Pet, Mount via Config
#define ENABLE_RUN_MOUSE_WHEEL_EVENT				// 
#define ENABLE_SERVER_SELECT_RENEWAL				// 
#define ENABLE_CHANNEL_LIST							// 
#define ENABLE_GUILD_MARK_RENEWAL					// 
#define ENABLE_EARTHQUAKE_EVENT						// 
#define ENABLE_HELP_RENEWAL							// 
#define ENABLE_BALANCE_IMPROVING					// 
//#define ENABLE_DS_TYPE_CHANGE						// 
#define ENABLE_DS_ACTIVE_EFFECT						// 
#define ENABLE_DSS_KEY_SELECT						// 
#define ENABLE_EXPANDED_MONEY_TASKBAR				// 
#define ENABLE_PICK_ROD_REFINE_RENEWAL				// 
#define ENABLE_STRUCTURE_VIEW_MODE					// 
#define ENABLE_TAB_TARGETING						// 
#define POPUPDIALOG_MODIFY							// 
#define ENABLE_ANTIFLAG_TOOLTIP						// 
#define ENABLE_RACE_HEIGHT							// 
#define ENABLE_FLASH_APLICATION						// 
#define ENABLE_RINGS_IN_COSTUME_WINDOW				// 
#define ENABLE_MULTIPLE_CHEST_OPEN					// 
#define ENABLE_COMPARE_TOOLTIP						// 
#define ENABLE_STONE_OF_BLESS						// 
#define ENABLE_BOSS_BOX								// 
#define ENABLE_HEROIC_EFFECT						// 
#define ENABLE_SHINING_SYSTEM						// Sanii Shining System
#define ENABLE_PICKUP_SOUND_EFFECT					// 
#define ENABLE_MAP_LOCATION_APP_NAME				// Enable map location on the app name
#define DISABLE_LINK_FROM_HYPERLINK					// 
#define CEF_BROWSER									// < CEF Browser
#define ENABLE_RENDERING_ONLY_IN_AREA				// Map Area Heavy Effect Usage Fix
#define ENABLE_RENDERING_ONLY_IN_AREA_V2			// CArea loading optimization
#define ENABLE_DISABLE_SOFTWARE_TILING				// 
#define ENABLE_BLOCK_COMMAND_PLAYER					// 
#define ENABLE_MESSENGER_LOCK_FIX					// 
#define ENABLE_FOG_FIX								// 
// #define ENABLE_WATER_OUTPUT_RENDER				// 
// #define ENABLE_NO_PICKUP_LIMIT					// if enabled, there will be no 0.5s of delay when picking up items with keyboard (\\z)
#define WJ_MULTI_TEXTLINE							// 
#define ENABLE_NEW_SHINING_EFFEKT					// Neues Shining-System
#ifdef ENABLE_NEW_SHINING_EFFEKT
#	define ENABLE_LVL115_ARMOR_EFFECT					// R¸stungsshining
#	define ENABLE_LVL96_WEAPON_EFFECT					// 
#	define ENABLE_LVL96_ARMOR_EFFECT					// 

	//Custom
//	#	define USE_WEAPON_COSTUME_WITH_EFFECT				// enable refine effect for weapon costume
//	#	define USE_BODY_COSTUME_WITH_EFFECT					// enable refine effect for body costume
#endif





//////////////////////////////////////////////////[GAMESOURCE& BINARY]//////////////////////////////////////////////////
//-----GLOBAL_STUFF---->
//#define ENABLE_SEQUENCE_SYSTEM					// 
#define ENABLE_HWID_BAN								// 
#ifdef ENABLE_HWID_BAN
#	define ENABLE_MULTIFARM_BLOCK						// 

// you should enable at least 3 conponents - (cpu, gpu, mainboard_name are recommended)
//	#	define HDD_COMPONENT								// 
#	define CPU_COMPONENT								// 
//	#	define PC_NAME_COMPONENT							// 
//	#	define BIOS_DATE_COMPONENT							// 
#	define MAINBOARD_NAME_COMPONENT						// 
#	define GPU_NAME_COMPONENT							// 
#	define HWID "2A4872A8E1205070CB1C89E8194884B0EB44BE50B9F45F30D1E81FFC23D89900"
#endif





//-----PROTO_STRUCT---->
#define ENABLE_PROTO_RENEWAL						// Offical Proto Struct
#define ENABLE_PROTO_RENEWAL_CUSTOM					// Proto Changes to be used by Custom Systems
#define ENABLE_SKILL_TABLE_RENEWAL					// 
#define ENABLE_NEW_DISTANCE_CALC					// Distance Calculation Renewal





//------CHARACTERS------>
#define ENABLE_PLAYER_PER_ACCOUNT5					// Erlaubt 5 Character
#define ENABLE_WOLFMAN_CHARACTER					// Wolfman Character





//--------SYSTEMS------->
#define ENABLE_SCALE_SYSTEM							// 
#define ENABLE_SCALE_SYSTEM_TEXTAIL					// 
#define ENABLE_COLLISION_RENEWAL					// 
#define ENABLE_IMAGE_SCALE							// 
#define ENABLE_ATLAS_SCALE							// 
#define NEW_SELECT_CHARACTER						// Select last played Character
#define WJ_SHOW_MOB_INFO							// Monster Lv/Aggro Anzeige
#define WJ_ENABLE_TRADABLE_ICON						// Nichthandelbare Items anzeigen
//#define ENABLE_SWAP_SYSTEM							// Inventory Item Swap System
#define ENABLE_PET_SYSTEM							// 
#define ENABLE_GROWTH_PET_SYSTEM					// 
#ifdef ENABLE_GROWTH_PET_SYSTEM
#	define ENABLE_GROWTH_PET_SKILL_DEL					// 
//	#	define ENABLE_GROWTH_PET_HATCHING_MONEY_CHANGE		// Hatching Price from Proto
#	define ENABLE_PET_ATTR_DETERMINE					// 
#	define ENABLE_PET_PRIMIUM_FEEDSTUFF					// 
//	#	define ENABLE_PET_MONSTER_TYPE						// 
#endif
#define ENABLE_ACCE_COSTUME_SYSTEM					// Sash System
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
#	define ENABLE_ACCE_SECOND_COSTUME_SYSTEM			// 
#	define WJ_SHOW_ACCE_ONOFF							// Gameoption active/deactive Visable Sash with Button
#endif
#define ENABLE_AURA_SYSTEM							// Aura
#ifdef ENABLE_AURA_SYSTEM
#	define ENABLE_AURA_BOOST							// 
#endif
#define ENABLE_YOHARA_SYSTEM						// Conqueror Level + Sungma
#ifdef ENABLE_YOHARA_SYSTEM
#	define ENABLE_NINETH_SKILL							// 
//	#	define ENABLE_PASSIVE_SYSTEM						// Relict-Window [unfinished]
//	#	define ENABLE_LOAD_SUNGMA_TABLE						// Sungma Load from Binary [unfinished]
#endif
#define ENABLE_GLOVE_SYSTEM							// Glove Item
#define ENABLE_REFINE_ELEMENT						// Element Set System [Worldard]
#define ENABLE_GUILDSTORAGE_SYSTEM					// Gildenlager
#define ENABLE_SPECIAL_INVENTORY					// Special Inventory [Sanii]
#define ENABLE_DRAGON_SOUL_SYSTEM					// Alchemysystem
#define ENABLE_DS_GRADE_MYTH						// Myth-grade
#define ENABLE_DS_CHANGE_ATTR						// 
#define ENABLE_DS_SET								// 
#define ENABLE_COSTUME_SYSTEM						// Cost¸msystem
#define ENABLE_MOUNT_COSTUME_SYSTEM					// enable mount costume slot
#define ENABLE_WEAPON_COSTUME_SYSTEM				// enable weapon costume slot
#define ENABLE_WEAPON_COSTUME_RENEWAL				// Offical Costume Weapon changes
#define ENABLE_USE_COSTUME_ATTR						// enables body costume bonus switcher
#define ENABLE_MOVE_COSTUME_ATTR					// Costume bonus transfer
#define ENABLE_HIDE_COSTUME_SYSTEM					// Disable/Enable Costume Design
#define ENABLE_CHANGED_ATTR							// 
#define ENABLE_MAGIC_REDUCTION_SYSTEM				// enable resist magic reduction bonus
#define ENABLE_PENDANT								// Pendant-Item-Slot
#ifdef ENABLE_PENDANT
#	define ENABLE_ELEMENT_ADD							// Element-Target Reversed
#endif
#define ENABLE_MULTI_LANGUAGE_SYSTEM				// Multilang System [Sonitex]
#define ENABLE_SOULBIND_SYSTEM						// Seelenbindung
#define ENABLE_CHEQUE_SYSTEM						// Won/Cheque
#ifdef ENABLE_CHEQUE_SYSTEM
#	define ENABLE_CHEQUE_EXCHANGE_WINDOW				// xP3ng3rx won -> yang / yang -> won Exchange Window
#endif
#define ENABLE_GACHA_SYSTEM							// Gacha-System
#define ENABLE_ACHIEVEMENT_SYSTEM					// 
#define ENABLE_BIOLOG_SYSTEM						// 
#define ENABLE_AUTO_SYSTEM							// Autojagd
#ifdef ENABLE_AUTO_SYSTEM
#	define ENABLE_AUTO_RESTART_EVENT
#	define ENABLE_IMPROVED_AUTOMATIC_HUNTING_SYSTEM
#	define ENABLE_AUTO_AFFECT_DISPLAY
#	define EVENT_HANDLER_MASTER
#endif
#define ENABLE_678TH_SKILL							// 
#define ENABLE_78TH_SKILL							// 7&8 Passive Skills
#define ENABLE_PASSIVE_ATTR							// 
#define ENABLE_SWITCHBOT							// Switchbot [Sanii]
#define ENABLE_BATTLE_PASS_SYSTEM					// Battle pass system
#define ENABLE_CHANGE_LOOK_SYSTEM					// Transmutation
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
#	define ENABLE_SLOT_COVER_IMAGE_SYSTEM				// 
#	define ENABLE_CHANGE_LOOK_MOUNT						// 
#	define ENABLE_CHANGE_LOOK_HAIR						// 
#endif
#define ENABLE_ATTR_6TH_7TH							// 67 Bonus Board
#ifdef ENABLE_ATTR_6TH_7TH
#	define ENABLE_ATTR_6TH_7TH_EXTEND					// New offical Bonus
#	define ENABLE_SKILLBOOK_COMBINATION					// 
#endif
#define ENABLE_SKILL_COLOR_SYSTEM					// Skill Color Change [Owsap]
#define ENABLE_CUBE_RENEWAL							// Cube Renewal
#define ENABLE_VIP_SYSTEM							// VIP System
#define ENABLE_PREMIUM_PRIVATE_SHOP					// Offlineshop
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
#	define ENABLE_OPEN_SHOP_WITHOUT_BAG					// 
#	define ENABLE_OPEN_SHOP_ONLY_IN_MARKET				// 
#	define ENABLE_OPEN_SHOP_WITH_PASSWORD				// 
#	define ENABLE_PREMIUM_PRIVATE_SHOP_EXPIRE			// 
#	define ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL			// 
#	define ENABLE_PRIVATESHOP_SEARCH_SYSTEM				// 
#	define ENABLE_NEARBY_SHOPS_DISPLAYED				// 
#	define ENABLE_ONE_PASS_PER_LOGIN					// 
#endif
#define ENABLE_MYSHOP_DECO							// Shop Decoration
#define ENABLE_MAILBOX								// Mailbox
#define ENABLE_TICKET_SYSTEM						// Ingame Ticket
#define ENABLE_DESTROY_SYSTEM						// Drop Items Zerstˆren System
//#define ENABLE_GROUP_DAMAGE_WEAPON_EFFECT			// Group-Dmg-System
#define ENABLE_INGAME_WIKI							// 
#define ENABLE_PATCHNOTE_WINDOW						// PatchNotes List
#define ENABLE_QUIVER_SYSTEM						// Pfeilkˆcher
#define ENABLE_DUNGEON_INFO_SYSTEM					// Dungeon Information
#define ENABLE_MOVE_CHANNEL							// Channel Switcher
#define ENABLE_QUEST_RENEWAL						// Quest page renewal
#define ENABLE_PARTY_MATCH							// 
#define ENABLE_FISHING_RENEWAL						// 
#define ENABLE_WEATHER_INFO							// Automatic Weather Load
#ifdef ENABLE_WEATHER_INFO
#	define ENABLE_WEATHER_INFO_EXTENDED					//Load @ Login TimeBased Environment
#endif





/*-----MiniGame*///----->
#define ENABLE_EVENTS								// 
#ifdef ENABLE_EVENTS
#	define ENABLE_EVENT_MANAGER							// Event Info [Board]
#	define ENABLE_EVENT_BANNER_FLAG						// Event-Banner

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

#	define ENABLE_WORLD_BOSS							// Worldboss-Jagd

//	Custom Events
#endif





/*-----Dungeons*///----->
#define ENABLE_RANKING_SYSTEM						// 
#ifdef ENABLE_RANKING_SYSTEM
#define ENABLE_RANKING_SYSTEM_PARTY					// 
#endif
#define ENABLE_DAWNMIST_DUNGEON						// Temple of Ochao
#define ENABLE_GUILD_DRAGONLAIR_SYSTEM				// Meleys Hort
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
#	define ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM			// 
#endif
#define ENABLE_BATTLE_FIELD							// 
#define ENABLE_12ZI									// Zodiak Dungeon
#ifdef ENABLE_12ZI
//	#	define ENABLE_12ZI_SHOP_LIMIT
#	define ENABLE_CHAT_MISSION_ALTERNATIVE				// 
#endif
#define ENABLE_DEFENSE_WAVE							// Schiffverteidigung
#define ENABLE_QUEEN_NETHIS							// 
#define ENABLE_SUNG_MAHI_TOWER




//--------OPTIONS------->
#define ENABLE_EXTEND_INVEN_SYSTEM					// 
#ifdef ENABLE_EXTEND_INVEN_SYSTEM
#	define ENABLE_EXTEND_INVEN_ITEM_UPGRADE				// 
#	define ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV	// Special Inv Unlock Extension
#	define ENABLE_BELT_INVENTORY						// 
//	#	define ENABLE_EQUIPMENT_INVENTORY					// 
#else
#define ENABLE_EXTEND_INVEN_PAGE_SYSTEM				// 
#endif
#define ENABLE_BELT_INVENTORY_RENEWAL				// Offical Belt Extension
#define ENABLE_ADDITIONAL_EQUIPMENT_PAGE			// Offical Inventory rework
#define WJ_NEW_USER_CARE							// 
#define ENABLE_AFTERDEATH_SHIELD					// Shield after Dead
#define ENABLE_EXPRESSING_EMOTION					// Neue Emotionen
#define ENABLE_CHAT_SETTINGS						// 
#define ENABLE_CHAT_SETTINGS_EXTEND					// 
#define ENABLE_SHOW_GUILD_LEADER					// Leader, CoLeader Anzeige
#define ENABLE_GUILDRENEWAL_SYSTEM					// Guild-Extension like Offical
#ifdef ENABLE_GUILDRENEWAL_SYSTEM
#	define ENABLE_SECOND_GUILDRENEWAL_SYSTEM			// 
#	define ENABLE_NEW_WAR_OPTIONS						// New GuildWar Options (Round, Points, Time)
#	define ENABLE_EXTENDED_GUILD_LEVEL					// Extend Guild-Lv up to 30

#	define ENABLE_MEDAL_OF_HONOR						// New Guild currency
#	define ENABLE_GUILD_DONATE_ATTENDANCE				// Daily-Donate

#	define ENABLE_GUILD_WAR_SCORE						// War-Page
#	define ENABLE_GUILD_LAND_INFO						// LandInfo-Page
#	define ENABLE_GUILDBANK_LOG							// GuildLogs

#	define ENABLE_EXTENDED_RENEWAL_FEATURES				// Change GuildMaster | Delete Land
#	define ENABLE_GUILDWAR_BUTTON						// GK-Creating -> Button is activating in Game
//	#	define ENABLE_GUILD_REQUEST							// unfinished
#	define ENABLE_NEW_WORLD_GUILDRENEWAL				// unfinished
//	#	define ENABLE_GUILD_RANKING							// unfinished
#endif
#define ENABLE_HUNTING_SYSTEM						// Jagdauftr‰ge
#define ENABLE_WORLD_LOTTERY_SYSTEM					// Lotto
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
#define ENABLE_DELETE_FAILURE_TYPE					// 
#define ENABLE_DELETE_FAILURE_TYPE_ADD				// 
#define ENABLE_GIVE_BASIC_ITEM						// Basic Items Give
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
#define ENABLE_OX_RENEWAL							// 
#define ENABLE_EXTENDED_ITEMNAME_ON_GROUND			// Itemnames on Ground
#define ENABLE_EMPIRE_FLAG							// Empire Flags on Global Chat
#define ENABLE_MESSENGER_BLOCK 						// Messenger block
#define ENABLE_GM_MESSENGER_LIST
#define ENABLE_MONARCH_AFFECT						// Monarch Ingame Effect
#define ENABLE_BLACK_COLOR_FIX 						// Fixed black color in the game





//////////////////////////////////////////////////[STANDARD DEFINES]//////////////////////////////////////////////////
/*<-----ETERLIB----->*/
// #define ENABLE_SYSERR_BY_PID						//	[Debug.cpp]								create syserr-{pid}.txt instead of syserr.txt
#define ENABLE_RETRY_CONNECT						//	[NetStream.cpp]							retry to reconnect to server after one second for three times
// #define ENABLE_D3D_DETECTION						//	[GrpDevice.cpp]							enable d3d detection(useless)

/*<-----ETERPACK----->*/
#define ENABLE_CRC32_CHECK							//	[EterPack.cpp]							mine: enable/disable crc32 check for type2


/*<-----GAMELIB----->*/
#define AUTODETECT_LYCAN_RODNPICK_BONE				//	[ActorInstanceAttach.cpp]				adjust fishrod/pickaxe attached bone for lycan to equip_right instead of equip_right_weapon
#define ENABLE_PETS_WITHOUT_COLLISIONS				//	[ActorInstanceCollisionDetection.cpp]	disable collisions for pets
#define ENABLE_SHOPS_WITHOUT_COLLISIONS				//	[ActorInstanceCollisionDetection.cpp]	disable collisions for shops
#define ENABLE_MOUNTS_WITHOUT_COLLISIONS			//	[ActorInstanceCollisionDetection.cpp]	disable collisions for mounts
#define ENABLE_PLAYEROX_WITHOUT_COLLISIONS
#define ENABLE_PARTY_WITHOUT_COLLISIONS
#define ENABLE_INVISIBLE_WITHOUT_COLLISIONS			//	[InstanceBaseBattle.cpp]				Invisible Players or Ninja Stealth Skill

// #define ENABLE_NO_PVP_PULLING_EFFECT				//	[ActorInstanceSync.cpp]					enable the pulling effect in pvp (experimental)
#define ENABLE_LOAD_ALTER_ITEMICON					//	[ItemData.cpp]							load a default item icon if the one inside the item_list.txt is missing
#define ENABLE_SKIN_EXTENDED						//	[RaceDataFile.cpp]						extended source/targetskin[2-9] inside .msm


/*<-----USERINTERFACE----->*/
// #define ENABLE_SIMPLE_REFINED_EFFECT_CHECK		//	[InstanceBase.cpp]						enable simple refine effect check (+7 blue, +8 green, +9 red) for any weapon/armor
// #define ENABLE_NO_MOUNT_CHECK					//	[InstanceBase.cpp]						enable attack and skill from all horses/mounts
#define ENABLE_CANSEEHIDDENTHING_FOR_GM				//	[InstanceBase.cpp]						enable gm to see invisible characters (both normal semi-transparent and gm invisibility)
// #define ENABLE_WINDOW_RESET_AUTOATTACK			//	[PythonApplicationProcedure.cpp]		reset automatic attack when switching/minimizing
#define ENABLE_ATLASINFO_FROM_ROOT					//	[PythonBackground.cpp]					read atlasinfo.txt from root instead of locale
//#define ATTACK_TIME_LOG							//	[PythonPlayerEventHandler.cpp]			Attack time log /- Detect Speedhack
// #define ENABLE_ATLAS_MARK_INFO_LOAD				//	[PythonMiniMap.cpp]						if enabled, locale/<lan>/map/<mapname>_point.txt will be read, otherwise no
#define ENABLE_NEW_ATLAS_MARK_INFO					//	[PythonMiniMap.cpp]						read the new locale/<lan>/map/<mapname>_point.txt structure (files used for offline minimap npc rendering)
#define ENABLE_NO_RECV_GAME_LIMIT					//	[PythonNetworkStreamPhaseGame.cpp]		it won't limit to 4 packets and 8kb of data for each iteration of game packets (experimental)
#ifndef ENABLE_ACHIEVEMENT_SYSTEM
#	define ENABLE_PYLIB_CHECK						//	[UserInterface.cpp]						check python lib files to prevent exploit before load them
#endif
#define ENABLE_MILES_CHECK							//	[UserInterface.cpp]						check miles files to prevent mss32.dll exploit before load them


/*<------DumpProto------>*/
#define ENABLE_ADDONTYPE_AUTODETECT					//	[mobproto.cpp]							it autodetects the addontype field value when unpacking to make an item_proto.txt server-side 100% compatible


/*<----Information---->*/
// Wenn man ein Costume-System deaktiviert - muss man in uiscript die costumewindow.py editieren (CostumeSlot auskommentieren)


/*
@#GENERAL MACROS
#define __OBSOLETE__								//useless and pointless code removed
#define __UNIMPLEMENTED__							//drafts of new things to be implemented
#define __UNIMPLEMENTED_EXTEND__
*/


/*
#@@Globally
@warme601: use release as advanced distribute with syserr.txt and so on
@warme666: those features requires the same feature server-sidely otherwise you'll get random issues. (packets not correctly handled)
@warme667: on ScriptLib/StdAfx.h; AT has been unset before loading python include
@warme668: trivial errors will be treated as mere warnings (sys_err -> sys_log) (TraceError -> Tracenf)

#@/MilesLib
@warme001; MilesLib;		AIL_startup responsible to load *.asi *.flt *.m3d *.mix
@warme002; MilesLib;		comments to be cleaned if necessary


#@@Client
@fixme001; UserInterface;	on Packet.h; for do_view_equip (WEAR_MAX_NUM: server 32, client 11) now equal (32 both sides)
@fixme002; EterLib;			on GrpImageTexture.cpp, GrpImage.cpp; to show the name of the failed mapped .dds load
@fixme003; PRTerrainLib;	on TextureSet.cpp; a new texture was added where the last was put
@fixme004; PRTerrainLib;	on TextureSet.cpp; a new textureset index was -1 instead of 0
@fixme005; EterLib;			on SkyBox.cpp; the bottom pic was not shown
@fixme006: UserInterface;	on PythonNetworkStreamModule.cpp, PythonNetworkStreamPhaseGame.cpp; "SEQUENCE mismatch 0xaf != 0x64 header 254" fix
@fixme007: UserInterface;	on Packet.h, PythonNetworkStream.cpp, PythonNetworkStreamPhaseGame.cpp; added unknown header 213 (2 bytes)
@fixme008; EterLib;			on IME.cpp; Ctrl+V crash when pasting images&co (no checks whether the handle was nullptr or not)
@fixme009: UserInterface;	on PythonPlayerModule.cpp; playerm2g2.GetItemLink wasn't considering 6-7 bonuses enough, they could have been seen as 1-5 bonuses if the item didn't have 1-5 bonuses
@fixme010; UserInterface;	on PythonCharacterManager.cpp; ymir forgot .m_dwVID in the format argument (c_rkCreateData -> c_rkCreateData.m_dwVID)
@fixme011; EterLib;			on IME.cpp; non-printing/control characters were printed in the (chat) input (the ones you get when you press Ctrl+<key> in game)
@fixme012; EterLib;			on TextTag.cpp; on arabic locales, the [HyperText code] (alias Prism code) could be edited pressing <Backspace>
@fixme013; UserInterface;	on PythonPlayerModule.cpp; playerm2g2.IsValuableItem was selecting a wrong item.cell
@fixme014; UserInterface;	on PythonPlayerInput.cpp; if you (mouse) click a monster without having arrows, the automatic attack will go in loop (clicking on ground again will fix, but moving with WASD will be bad)
@fixme015; GameLib;			on MapOutdoorLoad.cpp; regen.txt was loaded from launcher even though it's used only by the WorldEditor
@fixme016; UserInterface;	on PythonShop.cpp; ShopEx data weren't cleared if normal shops were open
@fixme017; EterPack;		on inline.h
@fixme019; UserInterface;	on PythonApplicationProcedure.cpp;
@fixme021; GameLib;			on RaceManager.cpp;new npclist.txt autodetects season shape.msm automatically
@fixme022: UserInterface;	on PythonItem.cpp; GetCloseItem distance calculation failed during float conversion (faraway items got distance 2 making them unpickable)




@fixme400; UserInterface;	on PythonTextTail.cpp; Name zentriert
@fixme401; EterPack;		on EterPackManager.cpp; CANNOT_FIND_PACK_FILE Error on DebugMode
@fixme402; UserInterface;	on PythonPlayerModule.cpp; PartyRemovePacket (P3NG3R)
@fixme403; UserInterface;	on PythonApplication.cpp; Whisper Flashing
@fixme404; EterPythonLib;	on PythonGridSlotWindow.cpp; Check Moving Item minor	[P3NG3R Fix]
@fixme405; GameLib;			on ActorInstanceBattle.cpp; Dojang anti attack push
@fixme406; GameLib;			on GameType.cpp; Hit Limit Counter
@fixme407; GameLib;			on ActorInstancePosition.cpp; Movement Check
@fixme408; scriptLib;		on Resource.cpp; "mse" added
@fixme409; scriptLib;		on Resource.cpp; "jpg" added
@fixme410; UserInterface;	on InstanceBase.cpp; Fix Invasibility & Skill Eunhyeong (invisibility effect bug => Minimap fix)
@fixme411; UserInterface;	on packet.h; Parts uint16_t -> uint32_t
@fixme412; GameLib;			on ActorInstanceFly.cpp; range attacks fix
@fixme413; EterPack;		on EterPack.cpp; Memory leak fix
@fixme414; scriptLib;		on PythonLauncher.cpp; Correct definition (True, False)
@fixme415; mileslib;		on SoundManager.cpp;
@fixme416; scriptLib;		on PythonUtils.cpp;
@fixme417; UserInterface;	on PythonCharacterManager.cpp; increase CPU&RAM usage/fix Blackscreen
@fixme418; MilesLib;		on Type.cpp, SoundManager.cpp; Small RE for mss sound scripts
@fixme419; UserInterface;	on PythonNetworkStream.cpp; Reload q fix
@fixme420; eterPythonLib;	on PythonWindow.cpp;
@fixme421; UserInterface;	on InstanceBase.cpp; aggregate on cape effect
@fixme422; UserInterface;	on PythonPlayerInputMouse.cpp; Click Item When Riding
@fixme423; UserInterface;	on PythonChat.cpp; Remove ugly GM-Font
@fixme424; UserInterface;	on PythonNetworkStreamCommand.cpp; cube reload fix
@fixme425; UserInterface;	on ---; refine packet fix
@fixme426; GameLib;			on RaceManager.cpp; extended PetRaceNums (34028-34099 -> 34001-34999)
@fixme427; GameLib;			on ActorInstanceBattle.cpp; shamy horse attack fix
@fixme428; UserInterface;	on InstanceBase.cpp; Lv Display on Metinstones was missing
@fixme429; UserInterface;	on ItemData.h; Extended Name Max Len
@fixme430; UserInterface;	on PythonPlayerModule.cpp | PythonCharacterManagerModule.cpp | PythonChatModule | PythonNetworkStreamModule; Gameforge Modules
@fixme431; EterLib;			on CRenderTarget.cpp; FALSE -> TRUE	[ENABLE_RENDER_TARGET]
@fixme432;
@fixme433; UserInterface;	on PythonNetworkStream.cpp; wrong packet
@fixme434; UserInterface;	on PythonNetworkStreamPhaseGame.cpp; bug
@fixme435; UserInterface;	on PythonNetworkStreamPhaseLoading.cpp; GF: LoadMap
@fixme436; UserInterface;	on GameType.h, Packet.h; Extension 255 Bonus +
@fixme437; UserInterface;	on AffectFlagContainer.h;
@fixme438; UserInterface;	on PythonSkill.cpp; Skill cooltime tooltip fix
@fixme439; UserInterface;	on ActorInstanceCollisionDetection.cpp; Gleichzeitig Hitbare Monster
@fixme440; UserInterface;	on InstanceBase.cpp; Game freezes when a character is loaded [SpeedUp]
@fixme441; UserInterface;	on PythonEventManager.cpp; EventSet Page Text fix
@fixme442; GameLib;			on RaceMotionData.cpp; Knockback no walk/attk fix

@custom001; UserInterface;	on PythonPlayer.cpp; s_dwNextTCPTime=dwCurTime 500 -> 100 (Pickupdelay)
@custom002; UserInterface;	on PythonApplicationLogo.cpp; Enable GF Animation
@custom003; UserInterface;	on StdAfx.h; extended CHARACTER_NAME_MAX_LEN 24 -> 48
@custom004; UserInterface;	on PythonPlayer.cpp; adding Bonus on Status
@custom005;
@custom006; EterLib;			on LensFlare.cpp; Fix Line on screen when adding brightness to the Environment and LensFlare not showing
@custom007; EterLib;			on IME.cpp; Enable Paste [STRG + V]
@custom008; UserInterface;	on InstanceBase.cpp; Test Motion Mount [footsteps]
@custom009; UserInterface;	on InstanceBase.cpp; New Code
@custom010; UserInterface;	on PythonEventManager.cpp; Quest Wait Time set to 0 (Original: 10)
@custom011; GameLib;			on MapOutdoorWater.cpp; WaterOutputRender
@custom012; UserInterface;	on PythonMiniMap.cpp; Minimap Whitemark
@custom013; UserInterface;	on PythonMiniMap.cpp; Boss Mark
@custom014; UserInterface;	on PythonTextTail.cpp; Outline on Texttail
@custom015;
@custom016; UserInterface;	on x.x.x.h; locale-folder-changes

#@@Tools
@fixme201; DumpProto;		on ItemCSVReader.cpp; race splitted with | instead of ,
*/
