#include "stdafx.h"

#include "../../common/VnumHelper.h"

#include "char.h"

#include "config.h"
#include "utils.h"
#include "crc32.h"
#include "char_manager.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "buffer_manager.h"
#include "item_manager.h"
#include "motion.h"
#include "vector.h"
#include "packet.h"
#include "cmd.h"
#include "fishing.h"
#include "exchange.h"
#include "battle.h"
#include "affect.h"
#include "shop.h"
#include "shop_manager.h"
#include "safebox.h"
#include "regen.h"
#include "pvp.h"
#include "party.h"
#include "start_position.h"
#include "questmanager.h"
#include "log.h"
#include "p2p.h"
#include "guild.h"
#include "guild_manager.h"
#include "dungeon.h"
#include "messenger_manager.h"
#include "unique_item.h"
#include "priv_manager.h"
#include "war_map.h"
#include "xmas_event.h"
#include "banword.h"
#include "target.h"
#include "wedding.h"
#include "mob_manager.h"
#include "mining.h"
#include "monarch.h"
#include "castle.h"
#include "arena.h"
#include "dev_log.h"
#include "horsename_manager.h"
#include "gm.h"
#include "map_location.h"
#include "BlueDragon_Binder.h"
#include "skill_power.h"
#include "buff_on_attributes.h"
#include "../../common/CommonDefines.h"
#ifdef __PET_SYSTEM__
#include "PetSystem.h"
#endif
#include "DragonSoul.h"
#ifdef ENABLE_SWITCHBOT
#include "switchbot.h"
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
#include "MeleyLair.h"
#endif
#ifdef ENABLE_12ZI
#include "zodiac_temple.h"
#endif
#ifdef ENABLE_SEND_TARGET_INFO
#include <algorithm>
#include <iterator>
using namespace std;
#endif
#ifdef ENABLE_BATTLE_FIELD
#	include "battle_field.h"
#endif
#ifdef ENABLE_PARTY_MATCH
#	include "GroupMatchManager.h"
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
#	include "battle_pass.h"
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
#	include "GrowthPetSystem.h"
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
#	include "AchievementSystem.h"
#endif
#ifdef ENABLE_QUEEN_NETHIS
#	include "SnakeLair.h"
#endif
#ifdef ENABLE_WHITE_DRAGON
#	include "WhiteDragon.h"
#endif
#ifdef ENABLE_EVENT_MANAGER
#	include "event_manager.h"
#endif

extern const uint16_t g_aBuffOnAttrPoints;	//@fixme532
extern bool RaceToJob(unsigned race, unsigned *ret_job);

extern bool IS_SUMMONABLE_ZONE(int map_index); // char_item.cpp
bool CAN_ENTER_ZONE(const LPCHARACTER& ch, int map_index) noexcept;

bool CAN_ENTER_ZONE(const LPCHARACTER& ch, int map_index) noexcept
{
	switch (map_index)
	{
		case MAP_CAPEDRAGONHEAD:
		case MAP_DAWNMISTWOOD:
		case MAP_BAYBLACKSAND:
		case MAP_MT_THUNDER:
			if (ch->GetLevel() < 90)
				return false;

		default:
			return true;
	}

	return true;
}

bool IS_BLOCKED_PET_SUMMON_MAP(int map_index) noexcept	//@custom006
{
	if (map_index == MAP_OXEVENT)
		return true;

	return false;
}

#ifdef DISABLE_RIDE_ON_SPECIAL_MAP
bool IS_MOUNTABLE_ZONE(int map_index)	//@custom005
{
	// (Private map instances, i.e dungeons, have high indexes)
	if (map_index >= 10000)
	{
		map_index /= 10000;
		switch (map_index)
		{
#ifdef ENABLE_SNOW_DUNGEON
			case MapTypes::MAP_N_SNOW_DUNGEON_01:
#endif
#ifdef ENABLE_DAWNMIST_DUNGEON
			case MapTypes::MAP_DAWNMISTWOOD_DUNGEON:
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
			case MapTypes::MAP_N_FLAME_DRAGON:
#endif
#ifdef ENABLE_DEFENSE_WAVE
			case MapTypes::MAP_DEFENSEWAVE:
#endif
#ifdef ENABLE_SUNG_MAHI_TOWER
			case MapTypes::MAP_SMG_DUNGEON_01:
			case MapTypes::MAP_SMG_DUNGEON_02:
#endif
				return false;

			default:
				return true;
		}
	}

	if (CWarMapManager::Instance().IsWarMap(map_index))
		return false;

	if (CArenaManager::Instance().IsArenaMap(map_index))
		return false;

#ifdef ENABLE_BATTLE_FIELD
	if (CBattleField::Instance().IsBattleZoneMapIndex(map_index))
		return false;
#endif

	switch (map_index)
	{
		case GUILD_VILLAGE_02:
		case MAP_T3:
		case MAP_T4:
		case MAP_OXEVENT:
		case MAP_SUNGZI:
		case MAP_SUNGZI_SNOW:
		case MAP_SUNGZI_DESERT_01:
			return false;

		default:
			return true;
	}

	return true;
}
#endif

#ifdef NEW_ICEDAMAGE_SYSTEM
const uint32_t CHARACTER::GetNoDamageRaceFlag() noexcept
{
	return m_dwNDRFlag;
}

void CHARACTER::SetNoDamageRaceFlag(uint32_t dwRaceFlag) noexcept
{
	if (dwRaceFlag >= MAIN_RACE_MAX_NUM)
		return;
	if (IS_SET(m_dwNDRFlag, 1 << dwRaceFlag))
		return;

	SET_BIT(m_dwNDRFlag, 1 << dwRaceFlag);
}

void CHARACTER::UnsetNoDamageRaceFlag(uint32_t dwRaceFlag) noexcept
{
	if (dwRaceFlag >= MAIN_RACE_MAX_NUM)
		return;
	if (!IS_SET(m_dwNDRFlag, 1 << dwRaceFlag))
		return;

	REMOVE_BIT(m_dwNDRFlag, 1 << dwRaceFlag);
}

void CHARACTER::ResetNoDamageRaceFlag() noexcept
{
	m_dwNDRFlag = 0;
}

const std::set<uint32_t>& CHARACTER::GetNoDamageAffectFlag() noexcept
{
	return m_setNDAFlag;
}

void CHARACTER::SetNoDamageAffectFlag(uint32_t dwAffectFlag)
{
	m_setNDAFlag.insert(dwAffectFlag);
}

void CHARACTER::UnsetNoDamageAffectFlag(uint32_t dwAffectFlag)
{
	m_setNDAFlag.erase(dwAffectFlag);
}

void CHARACTER::ResetNoDamageAffectFlag() noexcept
{
	m_setNDAFlag.clear();
}
#endif

// <Factor> DynamicCharacterPtr member function definitions
LPCHARACTER DynamicCharacterPtr::Get() const
{
	LPCHARACTER p = nullptr;
	if (is_pc)
		p = CHARACTER_MANAGER::Instance().FindByPID(id);
	else
		p = CHARACTER_MANAGER::Instance().Find(id);

	return p;
}

DynamicCharacterPtr& DynamicCharacterPtr::operator=(LPCHARACTER character)
{
	if (character == nullptr)
	{
		Reset();
		return *this;
	}

	if (character->IsPC())
	{
		is_pc = true;
		id = character->GetPlayerID();
	}
	else
	{
		is_pc = false;
		id = character->GetVID();
	}

	return *this;
}

CHARACTER::CHARACTER()
{
	m_stateIdle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateIdle, &CHARACTER::EndStateEmpty);
	m_stateMove.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateMove, &CHARACTER::EndStateEmpty);
	m_stateBattle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateBattle, &CHARACTER::EndStateEmpty);

	Initialize();
}

CHARACTER::~CHARACTER()
{
	Destroy();
}

void CHARACTER::Initialize()
{
	CEntity::Initialize(ENTITY_CHARACTER);

	m_bNoOpenedShop = true;

	m_bOpeningSafebox = false;
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	m_bOpeningGuildstorage = false;
#endif

	m_fSyncTime = get_float_time() - 3;
	m_dwPlayerID = 0;
	m_dwKillerPID = 0;
#ifdef ENABLE_SEND_TARGET_INFO
	dwLastTargetInfoPulse = 0;
#endif
	m_iMoveCount = 0;

	m_pkRegen = nullptr;
	regen_id_ = 0;
	m_posRegen.x = m_posRegen.y = m_posRegen.z = 0;
	m_posStart.x = m_posStart.y = 0;
	m_posDest.x = m_posDest.y = 0;

#ifdef ENABLE_CHECK_PICKUP_HACK
	m_dwLastPickupTime = 0;
#endif
#ifdef ENABLE_CHECK_WALLHACK
	m_dwCountWallhackPoint = 0;
#endif
#ifdef ENABLE_CHECK_GHOSTMODE
	m_dwCountGhostmodePoint = 0;
#endif

	m_fRegenAngle = 0.0f;

	m_pkMobData = nullptr;
	m_pkMobInst = nullptr;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	m_pkViewingShop = nullptr;
	m_shopOfflineEvent = nullptr;
	m_shopOfflineEventRunning = false;
	m_pkChrShopOwner = nullptr;
#	ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
	m_dwLastSyncPrivateShopInfo = 0;
#	endif
#else
	m_pkShop = nullptr;
	m_pkChrShopOwner = nullptr;
	m_pkMyShop = nullptr;
#endif

	m_pkExchange = nullptr;
	m_pkParty = nullptr;
	m_pkPartyRequestEvent = nullptr;

	m_pGuild = nullptr;

	m_pkChrTarget = nullptr;
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	m_pkTransmutation = nullptr;
#endif

	m_pkMuyeongEvent = nullptr;
#ifdef ENABLE_PVP_BALANCE
	m_pkGyeongGongEvent = nullptr;
#endif
#ifdef ENABLE_NINETH_SKILL
	m_pkCheonunEvent = nullptr;
#endif
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
	m_pkHitBuffElementEvent = nullptr;
#endif
#ifdef ENABLE_QUEEN_NETHIS
	m_pkSnakeSkillEvent = nullptr;
#endif
#ifdef ENABLE_ELEMENTAL_WORLD
	m_pkElementalWorldEvent = nullptr;
#endif
	m_pkWarpNPCEvent = nullptr;
	m_pkDeadEvent = nullptr;
	m_pkStunEvent = nullptr;
	m_pkSaveEvent = nullptr;
	m_pkRecoveryEvent = nullptr;
	m_pkTimedEvent = nullptr;
	m_pkFishingEvent = nullptr;
	m_pkWarpEvent = nullptr;

	// MINING
	m_pkMiningEvent = nullptr;
	// END_OF_MINING

	m_pkPoisonEvent = nullptr;
#ifdef ENABLE_WOLFMAN_CHARACTER
	m_pkBleedingEvent = nullptr;
#endif
	m_pkFireEvent = nullptr;
	m_pkCheckSpeedHackEvent = nullptr;
	m_speed_hack_count = 0;

	m_pkAffectEvent = nullptr;
	m_afAffectFlag = TAffectFlag(0, 0);

	m_pkDestroyWhenIdleEvent = nullptr;

	m_pkChrSyncOwner = nullptr;

	memset(&m_points, 0, sizeof(m_points));
	memset(&m_pointsInstant, 0, sizeof(m_pointsInstant));
	//m_pointsInstant = {}; //@fixme199
	//m_PlayerSlots = {}; //@fixme199
	memset(&m_quickslot, 0, sizeof(m_quickslot));

	m_bCharType = CHAR_TYPE_MONSTER;

	SetPosition(POS_STANDING);

	m_dwPlayStartTime = m_dwLastMoveTime = get_dword_time();

	GotoState(m_stateIdle);
	m_dwStateDuration = 1;

	m_dwLastAttackTime = get_dword_time() - 20000;

	m_bAddChrState = 0;

	m_pkChrStone = nullptr;

	m_pkSafebox = nullptr;
	m_iSafeboxSize = -1;
	m_iSafeboxLoadTime = 0;

	m_pkMall = nullptr;
	m_iMallLoadTime = 0;

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	m_pkGuildstorage = nullptr;
	m_iGuildstorageLoadTime = 0;
#endif
#ifdef ENABLE_CUBE_RENEWAL
	m_iCubeTime = 0;
#endif

	m_posWarp.x = m_posWarp.y = m_posWarp.z = 0;
	m_lWarpMapIndex = 0;

	m_posExit.x = m_posExit.y = m_posExit.z = 0;
	m_lExitMapIndex = 0;

	m_pSkillLevels = nullptr;

	m_dwMoveStartTime = 0;
	m_dwMoveDuration = 0;

	m_dwFlyTargetID = 0;

	m_dwNextStatePulse = 0;

	m_dwLastDeadTime = get_dword_time() - 180000;

	m_bSkipSave = false;

	m_bItemLoaded = false;
#ifdef ENABLE_AFK_MODE_SYSTEM
	m_pkUpdateCharacter = nullptr;
	m_isAway = false;
#endif
	m_bHasPoisoned = false;
#ifdef ENABLE_WOLFMAN_CHARACTER
	m_bHasBled = false;
#endif
	m_pkDungeon = nullptr;
	m_iEventAttr = 0;

	m_kAttackLog.dwVID = 0;
	m_kAttackLog.dwTime = 0;

	m_bNowWalking = m_bWalking = false;
	ResetChangeAttackPositionTime();

	m_bDetailLog = false;
	m_bMonsterLog = false;

	m_bDisableCooltime = false;

	m_iAlignment = 0;
	m_iRealAlignment = 0;
	m_iKillerModePulse = 0;
	m_bPKMode = PK_MODE_PEACE;

	m_dwQuestNPCVID = 0;
	m_dwQuestByVnum = 0;
	m_pQuestItem = nullptr;

	m_szMobileAuth[0] = '\0';

	m_dwUnderGuildWarInfoMessageTime = get_dword_time() - 60000;

	m_bUnderRefine = false;

	// REFINE_NPC
	m_dwRefineNPCVID = 0;
	// END_OF_REFINE_NPC

	m_dwPolymorphRace = 0;

	m_bStaminaConsume = false;

	ResetChainLightningIndex();

	m_dwMountVnum = 0;
	m_chHorse = nullptr;
	m_chRider = nullptr;

	m_pWarMap = nullptr;
	m_pWeddingMap = nullptr;
	m_bChatCounter = 0;

	ResetStopTime();

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	time_change_equipment_page = 0;
#endif

	m_dwLastVictimSetTime = get_dword_time() - 3000;
	m_iMaxAggro = -100;

	m_bSendHorseLevel = 0;
	m_bSendHorseHealthGrade = 0;
	m_bSendHorseStaminaGrade = 0;

	m_dwLoginPlayTime = 0;

	m_pkChrMarried = nullptr;

	m_posSafeboxOpen.x = -1000;
	m_posSafeboxOpen.y = -1000;

	// EQUIP_LAST_SKILL_DELAY
	m_dwLastSkillTime = get_dword_time();
	// END_OF_EQUIP_LAST_SKILL_DELAY

	// MOB_SKILL_COOLTIME
	memset(m_adwMobSkillCooltime, 0, sizeof(m_adwMobSkillCooltime));
	// END_OF_MOB_SKILL_COOLTIME

	// ARENA
	m_pArena = nullptr;
	m_nPotionLimit = quest::CQuestManager::Instance().GetEventFlag("arena_potion_limit_count");
	// END_ARENA

	//PREVENT_TRADE_WINDOW
	m_isOpenSafebox = 0;
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	m_isOpenGuildstorage = 0;
#endif
	//END_PREVENT_TRADE_WINDOW

	//PREVENT_REFINE_HACK
	m_iRefineTime = 0;
	//END_PREVENT_REFINE_HACK

	//RESTRICT_USE_SEED_OR_MOONBOTTLE
	m_iSeedTime = 0;
	//END_RESTRICT_USE_SEED_OR_MOONBOTTLE
	//PREVENT_PORTAL_AFTER_EXCHANGE
	m_iExchangeTime = 0;
	//END_PREVENT_PORTAL_AFTER_EXCHANGE
	//
	m_iSafeboxLoadTime = 0;

	m_iMyShopTime = 0;

#if defined(ENABLE_GROWTH_PET_SYSTEM) && defined(ENABLE_PET_ATTR_DETERMINE)
	m_iGrowthPetDetermineLoadTime = 0;
#endif

	InitMC();

	m_deposit_pulse = 0;

	SET_OVER_TIME(this, OT_NONE);

	m_strNewName = "";

	m_known_guild.clear();

	m_dwLogOffInterval = 0;
#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	m_gemItems = nullptr;
	m_dwGemNextRefresh = 0;
	m_bGemShopAllSlotUnlocked = false;
#endif

	m_bComboSequence = 0;
	m_dwLastComboTime = 0;
	m_bComboIndex = 0;
	m_iComboHackCount = 0;
	m_dwSkipComboAttackByTime = 0;

	m_dwMountTime = 0;

	m_dwLastGoldDropTime = 0;
#ifdef ENABLE_NEWSTUFF
	m_dwLastItemDropTime = 0;
	m_dwLastBoxUseTime = 0;
	m_dwLastBuySellTime = 0;
#endif

	m_bIsLoadedAffect = false;
	cannot_dead = false;

	// NPC GROUP RESPAWN SETTINGS
	//@custom014
	m_newSummonInterval = 0;
	m_lastSummonTime = 0;
	//@end_custom014
	// END NPC GROUP RESPAWN SETTINGS

#ifdef __PET_SYSTEM__
	m_petSystem = 0;
	m_bIsPet = false;
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
	m_GrowthPetSystem = 0;
	m_bIsGrowthPet = false;
	m_GrowthPetEggVID = 0;
	m_GrowthPetEvolution = 0;
	m_GrowthPetHachWindowIsOpen = false;
	m_GrowthPetWindowType = 0;
#endif

#ifdef NEW_ICEDAMAGE_SYSTEM
	m_dwNDRFlag = 0;
	m_setNDAFlag.clear();
#endif

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	bInventoryStage = 0;
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	bSpecialInventoryStage[0] = 0;
	bSpecialInventoryStage[1] = 0;
	bSpecialInventoryStage[2] = 0;
# endif
#endif

	m_fAttMul = 1.0f;
	m_fDamMul = 1.0f;

	m_powersStripped = false;	//@custom025

	m_pointsInstant.iDragonSoulActiveDeck = -1;
#ifdef ENABLE_DAWNMIST_DUNGEON
	m_dwBattleStartTime = 0;
	m_dwLastAttackedTime = 0;
	m_bySpecialSkillIndex = -1;

	m_pkTempleGuardianMoveEvent = nullptr;
	m_pkHealerEvent = nullptr;
#endif
#ifdef ENABLE_SUNG_MAHI_TOWER
	m_bIsUniqueMaster = false;
#endif
#ifdef ENABLE_ANTI_CMD_FLOOD
	m_dwCmdAntiFloodCount = 0;
	m_dwCmdAntiFloodPulse = 0;
#endif
	memset(&m_tvLastSyncTime, 0, sizeof(m_tvLastSyncTime));
#ifdef ENABLE_CHANGED_ATTR
	memset(&m_ItemSelectAttr, 0, sizeof(m_ItemSelectAttr));
#endif
	m_iSyncHackCount = 0;

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	m_pkMeleyLairDungeon = nullptr;
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
	memset(&m_dwSkillColor, 0, sizeof(m_dwSkillColor));
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	//Private shops
	m_shopItems.clear();
	m_pkMyShop = nullptr;
	m_stShopSign.clear();
	m_dwShopStash = 0;
#	ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	m_dwShopChequeStash = 0;
#	endif
	m_pkEditingShop = false;
#ifdef ENABLE_OPEN_SHOP_WITH_PASSWORD
	m_iPremiumShopLoadTime = 0;
#endif

#ifdef ENABLE_MYSHOP_DECO
	m_bMyPrivShopState = 0;

	m_bMyPrivShopIsCashItem = false;
	m_bMyPrivShopTabCount = 1;

	m_bMyPrivShopType = 0;
	m_bMyPrivShopPolyVnum = 30000;
#endif
#endif

	LastCampFireUse = 0;	//@fixme502
	waitHackCounter = 0;	//@fixme503

#ifdef ENABLE_BATTLE_FIELD
	m_pkBattleZoneEvent = nullptr;
	m_bBattleDeadLimit = 0;

	SetBattleFieldPoint(0);
	m_BattleFieldKillMap.clear();
#endif
#ifdef ENABLE_12ZI
	m_pkZodiac = nullptr;
	m_dwZodiacCzLastTime = 0;
	m_dwLastZodiacAttackTime = 0;
	m_dwDeadCount = 0;
#endif
#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_REWARD_TABLE)
	m_ziRewardTable = nullptr;
#endif
#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
	m_ShopLimitedPurchaseInfo.clear();
#endif
#ifdef ENABLE_ATTR_6TH_7TH
	m_isOpenSkillBookComb = false;
	m_isSkillBookCombType = 0;
	m_AttrItemAdded = nullptr;
#endif
#ifdef ENABLE_REFINE_ELEMENT
	m_OpenElementsSpell = false;
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	pSoulRoulette = nullptr;
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	m_bAcceCombination = false;
	m_bAcceAbsorption = false;
#endif
#ifdef ENABLE_AURA_SYSTEM
	m_bAuraRefineWindowType = AURA_WINDOW_TYPE_MAX;
	m_bAuraRefineWindowOpen = false;
	for (uint8_t i = AURA_SLOT_MAIN; i < AURA_SLOT_MAX; i++)
		m_pAuraRefineWindowItemSlot[i] = NPOS;

	memset(&m_bAuraRefineInfo, 0, AURA_REFINE_INFO_SLOT_MAX * sizeof(TAuraRefineInfo));
#endif
#ifdef ENABLE_MULTI_FARM_BLOCK
	m_bmultiFarmStatus = false;
#endif
#ifdef ENABLE_FLOWER_EVENT
	m_FlowerEvent = nullptr;
#endif
#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
	m_hitCount.clear();
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
	m_vecCatchKingFieldCards.clear();
	bCatchKingHandCard = 0;
	bCatchKingHandCardLeft = 0;
	bCatchKingBetSetNumber = 0;
	dwCatchKingTotalScore = 0;
	dwCatchKingGameStatus = false;
#endif
#ifdef ENABLE_FISH_EVENT
	m_fishSlots = nullptr;
	m_dwFishUseCount = 0;
	m_bFishAttachedShape = 0;
#endif
#ifdef ENABLE_MAILBOX
	m_pkMailBox = nullptr;
	bMailBoxLoading = false;
	m_iMyMailBoxTime = 0;
#endif
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
	m_bDungeonInfoOpen = false;
	m_pkDungeonInfoReloadEvent = nullptr;
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
	m_listExtBattlePass.clear();
	m_bIsLoadedExtBattlePass = false;
	m_dwLastReciveExtBattlePassInfoTime = 0;
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	m_GrowthPetInfo.clear();
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
	m_pkBiologManager = nullptr;
	s_pkReminderEvent = nullptr;
#endif
#ifdef ENABLE_CHANGE_LOOK_MOUNT
	m_dwHorseChangeLookVnum = 0;
#endif
#ifdef ENABLE_FISHING_RENEWAL
	m_pkFishingNewEvent = nullptr;
	m_bFishCatch = 0;
	m_dwLastCatch = 0;
	m_dwCatchFailed = 0;
#endif
#ifdef ENABLE_HIDE_COSTUME_SYSTEM
	m_HideCostumePulse = 0;
	m_bHideBodyCostume = false;
	m_bHideHairCostume = false;
	m_bHideAcceCostume = false;
	m_bHideWeaponCostume = false;
	m_bHideAuraCostume = false;
#endif
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	m_dwLastOfferNewExpTime = 0;
#endif
#ifdef ENABLE_CHECK_WINDOW_RENEWAL
	m_dwOpenedWindow = 0;
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	_achievement.reset();
	m_dwAchievementPoints = m_dwSelectedTitle = 0;
#endif

#ifdef ENABLE_AUTO_RESTART_EVENT
	autohunt_restart = false;
#endif

#ifdef ENABLE_MINI_GAME_BNW
	m_bBNWGameStatus = false;
	m_vecBNWOpponentCards.clear();
	m_vecBNWPlayerCards.clear();
	m_bPlayerScore = 0;
	m_bOpponentScore = 0;
#endif
#ifdef ENABLE_MINI_GAME_FINDM
	m_vecFindMFieldCards.clear();
	wFindMTryCount = 0;
	bFindMGameStatus = false;
	dwFindMStartTime = 0;
	dwFindMNextReveal = 0;
	dwFindMRewardVnum = 0;
	dwFindMRewardCost = 0;
	iRevealedIndex[0] = -1;
	iRevealedIndex[1] = -1;
	m_revealEvent = nullptr;
#ifdef ENABLE_MINI_GAME_FINDM_HINT
	bFindMHintCount = 0;
#endif
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
	bYutNoriGameStatus = false;
	bYutNoriIsPcTurn = false;
	bYutNoriIsStartThrow = false;

	memset(&cYutNoriThrowYut[0], -1, sizeof(cYutNoriThrowYut));
	memset(&cYutNoriUnitPos[0][0], YUT_NORI_START_POS, sizeof(cYutNoriUnitPos));
	memset(&cYutNoriUnitLastPos[0][0], YUT_NORI_START_POS, sizeof(cYutNoriUnitLastPos));

	cYutNoriWhoIsNextCom = 0;
	wYutNoriScore = 0;
	bYutNoriRemainCount = 0;
	bYutNoriProbIndex = YUT_NORI_YUTSEM1;

	bYutNoriIsReThrow = false;
	bYutNoriRewardStatus = false;
#endif
#ifdef ENABLE_CSHIELD
	m_cshield = nullptr;
#endif
#ifdef ENABLE_WORLD_BOSS
	m_pTier = 0;
	m_pGotRewards = false;
#endif
}

void CHARACTER::Create(const char* c_pszName, uint32_t vid, bool isPC)
{
	static int s_crc = 172814;

	char crc_string[128 + 1];
	snprintf(crc_string, sizeof(crc_string), "%s%p%d", c_pszName, this, ++s_crc);
	m_vid = VID(vid, GetCRC32(crc_string, strlen(crc_string)));

	if (isPC)
	{
		m_stName = c_pszName;
#ifdef ENABLE_CSHIELD
		m_cshield = std::make_shared<CShield>();
#endif
	}
}

void CHARACTER::Destroy()
{
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (IsShop())
	{
		CancelShopOfflineEvent();
		CloseShop();
	}
#else
	CloseMyShop();
#endif

	if (m_pkRegen)
	{
		if (m_pkDungeon)
		{
			// Dungeon regen may not be valid at this point
			if (m_pkDungeon->IsValidRegen(m_pkRegen, regen_id_))
				--m_pkRegen->count;
		}
#ifdef ENABLE_12ZI
		else if (m_pkZodiac)
		{
			if (m_pkZodiac->IsValidRegen(m_pkRegen, regen_id_))
				--m_pkRegen->count;
		}
#endif
		else
		{
			if (IsValidRegen(m_pkRegen, regen_id_))
				--m_pkRegen->count;
		}
		m_pkRegen = nullptr;
	}

	if (m_pkDungeon)
		SetDungeon(nullptr);

#ifdef ENABLE_12ZI
	if (m_pkZodiac)
		SetZodiac(nullptr);
#endif

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	if (m_pkMeleyLairDungeon)
		SetMeleyLair(nullptr);
#endif

#ifdef __PET_SYSTEM__
	if (m_petSystem)
	{
		m_petSystem->Destroy();
		delete m_petSystem;

		m_petSystem = 0;
	}
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
	if (m_GrowthPetSystem)
	{
		m_GrowthPetSystem->Destroy();
		delete m_GrowthPetSystem;

		m_GrowthPetSystem = 0;
	}
#endif

	HorseSummon(false);

#ifdef ENABLE_MAILBOX
	SetMailBox(nullptr);
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	SetTransmutation(nullptr);
#endif

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	if (pSoulRoulette) {
		delete pSoulRoulette;
		pSoulRoulette = nullptr;
	}
#endif

	if (GetRider())
		GetRider()->ClearHorseInfo();

	if (GetDesc())
	{
		GetDesc()->BindCharacter(nullptr);
		// BindDesc(nullptr);
	}

	if (m_pkExchange)
		m_pkExchange->Cancel();

	SetVictim(nullptr);

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (GetViewingShop())
	{
		GetViewingShop()->RemoveGuest(this);
		SetViewingShop(nullptr);
	}
#else
	if (GetShop())
	{
		GetShop()->RemoveGuest(this);
		SetShop(nullptr);
	}
#endif

	ClearStone();
	ClearSync();
	ClearTarget();

	if (nullptr == m_pkMobData)
	{
		DragonSoul_CleanUp();
		ClearItem();
	}
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	else if (IsShop())
	{
		ClearShopItem();
	}
#endif

	// <Factor> m_pkParty becomes nullptr after CParty destructor call!
	LPPARTY party = m_pkParty;
	if (party)
	{
		if (party->GetLeaderPID() == GetVID() && !IsPC())
		{
			M2_DELETE(party);
		}
		else
		{
			party->Unlink(this);

			if (!IsPC())
				party->Quit(GetVID());
		}

		SetParty(nullptr); // You don't have to, but it's safe.
	}

	if (m_pkMobInst)
	{
		M2_DELETE(m_pkMobInst);
		m_pkMobInst = nullptr;
	}

	m_pkMobData = nullptr;

	if (m_pkSafebox)
	{
		M2_DELETE(m_pkSafebox);
		m_pkSafebox = nullptr;
	}

	if (m_pkMall)
	{
		M2_DELETE(m_pkMall);
		m_pkMall = nullptr;
	}

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	if (m_pkGuildstorage)
	{
		M2_DELETE(m_pkGuildstorage);
		m_pkGuildstorage = nullptr;
	}
#endif

	for (TMapBuffOnAttrs::iterator it = m_map_buff_on_attrs.begin(); it != m_map_buff_on_attrs.end(); ++it)	//@fixme541
	{
		if (nullptr != it->second)
		{
			M2_DELETE(it->second);
		}
	}
	m_map_buff_on_attrs.clear();

	m_set_pkChrSpawnedBy.clear();

	StopMuyeongEvent();
#ifdef ENABLE_PVP_BALANCE
	StopGyeongGongEvent();
#endif
#ifdef ENABLE_NINETH_SKILL
	StopCheonunEvent();
#endif
#ifdef ENABLE_ELEMENTAL_WORLD
	StopElementalWorldEvent();
#endif

	event_cancel(&m_pkWarpNPCEvent);
	event_cancel(&m_pkRecoveryEvent);
	event_cancel(&m_pkDeadEvent);
	event_cancel(&m_pkSaveEvent);
	event_cancel(&m_pkTimedEvent);
	event_cancel(&m_pkStunEvent);
	event_cancel(&m_pkFishingEvent);
	event_cancel(&m_pkPoisonEvent);
#ifdef ENABLE_WOLFMAN_CHARACTER
	event_cancel(&m_pkBleedingEvent);
#endif
#ifdef ENABLE_BATTLE_FIELD
	event_cancel(&m_pkBattleZoneEvent);
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
	event_cancel(&s_pkReminderEvent);
#endif
#ifdef ENABLE_FISHING_RENEWAL
	event_cancel(&m_pkFishingNewEvent);
#endif
	event_cancel(&m_pkFireEvent);
	event_cancel(&m_pkPartyRequestEvent);
	//DELAYED_WARP
	event_cancel(&m_pkWarpEvent);
	event_cancel(&m_pkCheckSpeedHackEvent);
	//END_DELAYED_WARP

	// RECALL_DELAY
	//event_cancel(&m_pkRecallEvent);
	// END_OF_RECALL_DELAY

	// MINING
	event_cancel(&m_pkMiningEvent);
	// END_OF_MINING

#ifdef ENABLE_MINI_GAME_FINDM
	event_cancel(&m_revealEvent);
#endif

#ifdef ENABLE_DUNGEON_INFO_SYSTEM
	StopDungeonInfoReloadEvent();
#endif

	for (auto& it : m_mapMobSkillEvent)
	{
		LPEVENT pkEvent = it.second;
		event_cancel(&pkEvent);
	}
	m_mapMobSkillEvent.clear();

	//event_cancel(&m_pkAffectEvent);
	ClearAffect();

	event_cancel(&m_pkDestroyWhenIdleEvent);

	if (m_pSkillLevels)
	{
		M2_DELETE_ARRAY(m_pSkillLevels);
		m_pSkillLevels = nullptr;
	}

#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_REWARD_TABLE)
	if (m_ziRewardTable)
	{
		M2_DELETE_ARRAY(m_ziRewardTable);
		m_ziRewardTable = nullptr;
	}
#endif

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	if (m_gemItems)
	{
		M2_DELETE_ARRAY(m_gemItems);
		m_gemItems = nullptr;
	}
#endif

#ifdef ENABLE_FISH_EVENT
	if (m_fishSlots)
	{
		M2_DELETE_ARRAY(m_fishSlots);
		m_fishSlots = nullptr;
	}
#endif

#ifdef ENABLE_FLOWER_EVENT
	if (m_FlowerEvent)
	{
		M2_DELETE_ARRAY(m_FlowerEvent);
		m_FlowerEvent = nullptr;
	}
#endif

	CEntity::Destroy();

	if (GetSectree())
		GetSectree()->RemoveEntity(this);

	if (m_bMonsterLog)
		CHARACTER_MANAGER::Instance().UnregisterForMonsterLog(this);

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	if (IsPC())
		_achievement.reset();
#endif

#ifdef __UNIMPLEMENTED__
	//Clear affects if stored
	if (affectInfo)
	{
		M2_DELETE_ARRAY(affectInfo->data);
		M2_DELETE(affectInfo);
	}
#endif
}

const char* CHARACTER::GetName() const
{
	if (!this)
	{
		sys_err("char.cpp GetName() -> Possible crash core here!");
		return "";
	}

	return m_stName.empty() ? (m_pkMobData ? m_pkMobData->m_table.szLocaleName : "") : m_stName.c_str();
}

std::string CHARACTER::GetStringName() const
{
	return m_stName.empty() ? (m_pkMobData ? std::string(m_pkMobData->m_table.szLocaleName) : "") : m_stName;
}

const char * CHARACTER::GetProtoName() const
{
	return m_pkMobData ? m_pkMobData->m_table.szName : "";
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
void CHARACTER::OpenMyShop(const char* c_pszSign, TShopItemTable* pTable, uint8_t bItemCount
#	ifdef ENABLE_MYSHOP_DECO_PREMIUM
	, uint32_t dwPolyVnum, uint8_t bTitleType
#	endif
)
{
	quest::PC* pPC = quest::CQuestManager::Instance().GetPCForce(GetPlayerID());
	if (pPC->IsRunning())
		return;

	if (bItemCount == 0)
		return;

#ifdef ENABLE_BLOCK_ACTION_WITH_PID
	if (IsBlockedAction(this))
		return;
#endif

	// Disallow shops on certain maps
	if (!CShop::CanOpenShopHere(GetMapIndex()))
		return;

	// Check if the items + our current shop gold stash are exceeding the max yang amount.
	long long lTotalMoney = static_cast<uint32_t>(GetShopGoldStash());
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	long long lTotalCheque = static_cast<uint32_t>(GetShopChequeStash());
#endif

	for (int n = 0; n < bItemCount; ++n)
	{
		lTotalMoney += static_cast<uint32_t>((pTable + n)->price);
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
		lTotalCheque += static_cast<uint32_t>((pTable + n)->cheque);
#endif
	}

	if (lTotalMoney < 0)
		return;

#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	if (lTotalCheque < 0)
		return;
#endif

	if (lTotalMoney >= GOLD_MAX)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_HAVE_%d_IN_GOLD_STASH_AND_THE_ITEMS_YOU_SELL_ARE_WORTH_%lld"), GetShopGoldStash(), lTotalMoney);
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_CANNOT_OPEN_THE_SHOP_BECAUSE_IT_WOULD_SET_YOU_OVER_%d"), (GOLD_MAX - 1));
		return;
	}

#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	if (lTotalCheque >= CHEQUE_MAX)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_HAVE_%d_IN_CHEQUE_STASH_AND_THE_ITEMS_YOU_SELL_ARE_WORTH_%lld"), GetShopChequeStash(), lTotalCheque);
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_CANNOT_OPEN_THE_SHOP_BECAUSE_IT_WOULD_SET_YOU_OVER_%d"), (CHEQUE_MAX - 1));
		return;
	}
#endif

	// Don't create the shop if the title has invalid words in it (or is empty)
	std::string shopSign = c_pszSign;

	shopSign.erase(std::remove_if(shopSign.begin(), shopSign.end(), [](char c) {
		return !isalnum(c) && c != ' ' && c != '+'; //< remove if this applies
		}), shopSign.end());

	if (shopSign.length() < 3)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SHOP_SIGN_INVALID_OR_TOO_SMALL"));
		return;
	}

	if (CBanwordManager::Instance().CheckString(shopSign.c_str(), shopSign.length() + 1)) // Check for banned words
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;812]");
		return;
	}

	// MYSHOP_PRICE_LIST
#ifdef ENABLE_CHEQUE_SYSTEM
	std::map<uint32_t, TItemPriceType> itemkind;
#else
	std::map<uint32_t, uint32_t> itemkind; // Price by item type, first: vnum, second: price in single quantity
#endif
	// END_OF_MYSHOP_PRICE_LIST

	std::set<TItemPos> cont;
	bool emptyShop = true;
	for (uint8_t i = 0; i < bItemCount; ++i)
	{
		if (cont.find((pTable + i)->pos) != cont.end())
		{
			sys_err("MYSHOP: duplicate shop item detected! (name: %s)", GetName());
			return;
		}

		// ANTI_GIVE, ANTI_MYSHOP check
		LPITEM pkItem = GetItem((pTable + i)->pos);

		if (pkItem)
		{
			if (emptyShop)
				emptyShop = false;

			const TItemTable* item_table = pkItem->GetProto();

			if (item_table && (IS_SET(item_table->dwAntiFlags, ITEM_ANTIFLAG_GIVE | ITEM_ANTIFLAG_MYSHOP)))
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;813]");
				return;
			}

			if (pkItem->IsEquipped())
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;1060]");
				return;
			}

#ifdef ENABLE_SEALBIND_SYSTEM
			if (pkItem->IsSealed())
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;1113]");
				return;
			}
#endif

			if (pkItem->isLocked())
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;1133]");
				return;
			}

			if (pkItem->GetVnum() == 50200 || pkItem->GetVnum() == 71049) //SHOP bundle never let this get into the shop!
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEMS_BEING_IN_USE_CAN_NOT_BE_SOLD_IN_SHOP"));
				return;
			}

			if (pkItem->GetWindow() != INVENTORY && pkItem->GetWindow() != DRAGON_SOUL_INVENTORY)
			{
				sys_err("Wrong window (%d) when setting item %lu on shop", pkItem->GetWindow(), pkItem->GetID());
				return;
			}

			// MYSHOP_PRICE_LIST
#ifdef ENABLE_CHEQUE_SYSTEM
			itemkind[pkItem->GetVnum()] = TItemPriceType((pTable + i)->price / pkItem->GetCount(), (pTable + i)->cheque);
#else
			itemkind[pkItem->GetVnum()] = (pTable + i)->price / pkItem->GetCount();
#endif
			// END_OF_MYSHOP_PRICE_LIST
		}

		cont.insert((pTable + i)->pos);
	}

	//No shop to build
	if (emptyShop) {
		return;
	}

	// MYSHOP_PRICE_LIST
	// Reduce the number of bags.
	bool removeBundle = false;
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
	if (dwPolyVnum == 0) // Shop without decoration
#endif
	{
		if (CountSpecifyItem(71049)) // The price information is stored without removing the silk bundle.
		{
			//
			// To store item price information, an item price information packet is created and sent to the DB cache.
			//
			TPacketMyshopPricelistHeader header{};
			TItemPriceInfo info;

			header.dwOwnerID = GetPlayerID();
			header.byCount = static_cast<uint8_t>(itemkind.size());

			TEMP_BUFFER buf;
			buf.write(&header, sizeof(header));

			for (auto& it : itemkind)
			{
				info.dwVnum = it.first;
#ifdef ENABLE_CHEQUE_SYSTEM //OFF_CHEQUE
				info.price.dwPrice = it.second.dwPrice;
				info.price.dwCheque = it.second.dwCheque;
#else
				info.dwPrice = it->second;
#endif

				buf.write(&info, sizeof(info));
			}

			db_clientdesc->DBPacket(HEADER_GD_MYSHOP_PRICELIST_UPDATE, 0, buf.read_peek(), buf.size());
		}
		else if (CountSpecifyItem(50200))
		{
			removeBundle = true; //Remove later, only if the shop successfully spawned
		}
		else
			return; // no bundle => error out
	}
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
	else // Shop with decoration
	{
		if (CountSpecifyItem(71221) <= 0)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You do not have the necessary material to open a private shop with decoration"));
			return;
		}
	}
#endif

	if (m_pkExchange)
		m_pkExchange->Cancel();

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TIME
	LPCHARACTER shopChar = CHARACTER_MANAGER::Instance().SpawnShop(this, shopSign, pTable, bItemCount
#	ifdef ENABLE_MYSHOP_DECO_PREMIUM
		, 0, dwPolyVnum, bTitleType, dwPolyVnum != 0
#	endif
	);
	if (shopChar && shopChar->GetMyShop())
	{
#	ifndef ENABLE_OPEN_SHOP_WITHOUT_BAG
		// Remove shop item bundle
		if (removeBundle)
			RemoveSpecifyItem(50200, 1);
#	endif

#	ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
		if (GetConquerorLevel())
			shopChar->SetConquerorLevel(GetConquerorLevel());
		else
			shopChar->SetLevel(GetLevel());
		shopChar->SetAlignment(GetAlignment());
		shopChar->SetEmpire(GetEmpire());

		if (GetGuild())
			shopChar->SetGuild(GetGuild());
#	endif

		// Set offline time
		shopChar->GetMyShop()->SetOfflineMinutes(GetPremiumPrivateShopRemainSeconds());

		// Save offline time
		shopChar->GetMyShop()->SaveOffline();

		// Sync offline time
		if (GetDesc())
		{
			TPacketGCShopOffTimeSync pack{};
			pack.bHeader = HEADER_GC_SYNC_SHOP_OFFTIME;
			pack.value = GetPremiumPrivateShopRemainSeconds();
			GetDesc()->Packet(&pack, sizeof(TPacketGCShopOffTimeSync));
		}
	}
#else
#	ifndef ENABLE_OPEN_SHOP_WITHOUT_BAG
	if (CHARACTER_MANAGER::Instance().SpawnShop(this, shopSign, pTable, bItemCount))
	{
		if (removeBundle)
			RemoveSpecifyItem(50200, 1);
	}
#	endif
#endif

#if defined(ENABLE_PREMIUM_PRIVATE_SHOP) && defined(ENABLE_SHOP_NO_SPEND_MIN_IF_ONLINE)
	uint32_t pid = GetPlayerID();
	db_clientdesc->DBPacket(HEADER_GD_PLAYER_SHOP_TIMER, 0, &pid, sizeof(uint32_t));
#endif
}

void CHARACTER::CloseShop()
{
	// Only shop characters can be closed.
	if (!IsShop())
		return;

	ClearShopSign();
	CShopManager::Instance().DestroyPCShop(this);

	TPacketGCShopSign p{};

	p.bHeader = HEADER_GC_SHOP_SIGN;
	p.dwVID = GetVID();
	p.szSign[0] = '\0';
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
	p.bType = 0;
#endif

	PacketAround(&p, sizeof(p));
}
#else
void CHARACTER::OpenMyShop(const char * c_pszSign, TShopItemTable * pTable, uint8_t bItemCount)
{
	if (!CanHandleItem()) // @fixme149
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;841]");
		return;
	}

#ifndef ENABLE_OPEN_SHOP_WITH_ARMOR
	if (GetPart(PART_MAIN) > 2)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1025]");
		return;
	}
#endif

	if (!CShop::CanOpenShopHere(GetMapIndex()))	//@custom023
		return;

	//@fixme470
	//Don't create the shop if the title has invalid words in it (or is empty)
	std::string shopSign = c_pszSign;

	shopSign.erase(std::remove_if(shopSign.begin(), shopSign.end(), [](char c) {
		return !isalnum(c) && c != ' ' && c != '+'; //< remove if this applies
	}), shopSign.end());

	if (shopSign.length() < 3) {
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SHOP_SIGN_INVALID_OR_TOO_SMALL"));
		return;
	}

	if (CBanwordManager::Instance().CheckString(shopSign.c_str(), shopSign.length() + 1)) { // Check for banned words
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SHOP_NAME_NOT_ALLOWED"));
		return;
	}
	//@end_fixme470

	if (GetMyShop()) // If the shop is already open, close it.
	{
		CloseMyShop();
		return;
	}

	if (IsRestricted()) {	//@fixme500
		ChatPacket(CHAT_TYPE_INFO, "Trebuie sa astepti %d secunde pentru a face asta.", (GetQuestFlag("restrict.player") - get_global_time()) % 180);
		return;
	}

	// If there is a quest in progress, the shop cannot be opened.
	quest::PC* pPC = quest::CQuestManager::Instance().GetPCForce(GetPlayerID());

	// GetPCForce cannot be NULL, so it is not checked separately
	if (pPC->IsRunning())
		return;

	if (bItemCount == 0)
		return;

	int64_t nTotalMoney = 0;
#ifdef ENABLE_CHEQUE_SYSTEM
	int64_t nTotalCheque = 0;
#endif

	for (int n = 0; n < bItemCount; ++n)
	{
		nTotalMoney += static_cast<int64_t>((pTable + n)->price);
#ifdef ENABLE_CHEQUE_SYSTEM
		nTotalCheque += static_cast<int64_t>((pTable + n)->cheque);
#endif
	}

	nTotalMoney += static_cast<int64_t>(GetGold());
#ifdef ENABLE_CHEQUE_SYSTEM
	nTotalCheque += static_cast<int64_t>(GetCheque());
#endif

	if (GOLD_MAX <= nTotalMoney)
	{
		sys_err("[OVERFLOW_GOLD] Overflow (GOLD_MAX) id %u name %s", GetPlayerID(), GetName());
		ChatPacket(CHAT_TYPE_INFO, "[LS;811]");
		return;
	}

#ifdef ENABLE_CHEQUE_SYSTEM
	if (CHEQUE_MAX <= nTotalCheque)
	{
		sys_err("[OVERFLOW_CHEQUE] Overflow (CHEQUE_MAX) id %u name %s", GetPlayerID(), GetName());
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't create a shop with more than %d won."), (CHEQUE_MAX - 1));
		return;
	}
#endif

	char szSign[SHOP_SIGN_MAX_LEN + 1];
	strlcpy(szSign, c_pszSign, sizeof(szSign));

	m_stShopSign = szSign;

	if (m_stShopSign.length() == 0)
		return;

	if (CBanwordManager::Instance().CheckString(m_stShopSign.c_str(), m_stShopSign.length()))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;812]");
		return;
	}

	// MYSHOP_PRICE_LIST
#ifdef ENABLE_CHEQUE_SYSTEM
	std::map<uint32_t, TItemPriceType> itemkind;
#else
	std::map<uint32_t, uint32_t> itemkind; // Price by item type, first: vnum, second: price in single quantity
#endif
	// END_OF_MYSHOP_PRICE_LIST

	std::set<TItemPos> cont;
	for (uint8_t i = 0; i < bItemCount; ++i)
	{
		if (cont.find((pTable + i)->pos) != cont.end())
		{
			sys_err("MYSHOP: duplicate shop item detected! (name: %s)", GetName());
			return;
		}

		// ANTI_GIVE, ANTI_MYSHOP check
		LPITEM pkItem = GetItem((pTable + i)->pos);

		if (pkItem)
		{
			const TItemTable* item_table = pkItem->GetProto();

			if (item_table && (IS_SET(item_table->dwAntiFlags, ITEM_ANTIFLAG_GIVE | ITEM_ANTIFLAG_MYSHOP)))
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;813]");
				return;
			}

			if (pkItem->IsEquipped())
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;1060]");
				return;
			}

#ifdef ENABLE_GIVE_BASIC_ITEM
			if (pkItem->IsBasicItem())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
				return;
			}
#endif

			if (pkItem->isLocked())
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;1133]");
				return;
			}

#ifdef ENABLE_SEALBIND_SYSTEM
			if (pkItem->IsSealed())
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;1100]");
				return;
			}
#endif

			if (pkItem->GetVnum() == 50200 || pkItem->GetVnum() == 71049) { //@fixme407; SHOP bundle never let this get into the shop!
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEMS_BEING_IN_USE_CAN_NOT_BE_SOLD_IN_SHOP"));
				return;
			}

#ifdef __UNIMPLEMENTED__
			if (pkItem->GetWindow() != INVENTORY && pkItem->GetWindow() != DRAGON_SOUL_INVENTORY) {
				sys_err("Wrong window (%d) when setting item %lu on shop", pkItem->GetWindow(), pkItem->GetID());
				return;
			}
#endif

			// MYSHOP_PRICE_LIST
#ifdef ENABLE_CHEQUE_SYSTEM
			itemkind[pkItem->GetVnum()] = TItemPriceType((pTable + i)->price / pkItem->GetCount(), (pTable + i)->cheque);
#else
			itemkind[pkItem->GetVnum()] = (pTable + i)->price / pkItem->GetCount();
#endif
			// END_OF_MYSHOP_PRICE_LIST
		}

		cont.insert((pTable + i)->pos);
	}

	// MYSHOP_PRICE_LIST
	// Reduce the number of bags.
#ifdef ENABLE_MYSHOP_DECO
	if (m_bMyPrivShopIsCashItem)
#else
	if (CountSpecifyItem(71049)) // The price information is stored without removing the silk bundle.
#endif
	{
		//
		// To store item price information, an item price information packet is created and sent to the DB cache.
		//
		// @fixme403 BEGIN
		TItemPriceListTable header;
		memset(&header, 0, sizeof(TItemPriceListTable));

		header.dwOwnerID = GetPlayerID();
		header.byCount = itemkind.size();

		size_t idx = 0;
		for (auto& it : itemkind)
		{
			header.aPriceInfo[idx].dwVnum = it.first;
#ifdef ENABLE_CHEQUE_SYSTEM
			header.aPriceInfo[idx].price.dwPrice = it.second.dwPrice;
			header.aPriceInfo[idx].price.dwCheque = it.second.dwCheque;
#else
			header.aPriceInfo[idx].dwPrice = it.second;
#endif
			idx++;
		}

		db_clientdesc->DBPacket(HEADER_GD_MYSHOP_PRICELIST_UPDATE, GetDesc()->GetHandle(), &header, sizeof(TItemPriceListTable));
		// @fixme403 END
	}
	// END_OF_MYSHOP_PRICE_LIST
	else if (CountSpecifyItem(50200))
		RemoveSpecifyItem(50200, 1);
	else
		return; // Stop if there is no package.

	if (m_pkExchange)
		m_pkExchange->Cancel();

	TPacketGCShopSign p{};

	p.bHeader = HEADER_GC_SHOP_SIGN;
	p.dwVID = GetVID();
	strlcpy(p.szSign, c_pszSign, sizeof(p.szSign));
#ifdef ENABLE_MYSHOP_DECO
	p.bType = m_bMyPrivShopType;
#endif

	PacketAround(&p, sizeof(TPacketGCShopSign));

	m_pkMyShop = CShopManager::Instance().CreatePCShop(this, pTable, bItemCount);

	if (IsPolymorphed() == true)
	{
		RemoveAffect(AFFECT_POLYMORPH);
	}

	if (GetHorse())
	{
		HorseSummon(false, true);
	}
	// Automatically unmount when opening a personal shop while using new mount
	// It would be nice to handle the new mount with StopRiding, but I don't know why it wasn't done that way.
	else if (GetMountVnum())
	{
		RemoveAffect(AFFECT_MOUNT);
		RemoveAffect(AFFECT_MOUNT_BONUS);
	}

#ifdef ENABLE_MYSHOP_DECO
	SetPolymorph(m_bMyPrivShopPolyVnum, true);
#else
	SetPolymorph(30000, true);
#endif

}

void CHARACTER::CloseMyShop()
{
	if (GetMyShop())
	{
		m_stShopSign.clear();
		CShopManager::Instance().DestroyPCShop(this);
		m_pkMyShop = nullptr;

		TPacketGCShopSign p{};

		p.bHeader = HEADER_GC_SHOP_SIGN;
		p.dwVID = GetVID();
		p.szSign[0] = '\0';
#ifdef ENABLE_MYSHOP_DECO
		p.bType = 0;
#endif

		PacketAround(&p, sizeof(p));

#ifdef ENABLE_WOLFMAN_CHARACTER
		SetPolymorph(m_points.job, true);
		// SetPolymorph(0, true);
#else
		SetPolymorph(GetJob(), true);
#endif
#ifdef ENABLE_MYSHOP_DECO
		SetMyPrivShopState(0, 30000);
#endif
	}
}
#endif

void EncodeMovePacket(TPacketGCMove& pack, uint32_t dwVID, uint8_t bFunc, uint8_t bArg, uint32_t x, uint32_t y, uint32_t dwDuration, uint32_t dwTime, uint8_t bRot) noexcept
{
	pack.bHeader = HEADER_GC_MOVE;
	pack.bFunc = bFunc;
	pack.bArg = bArg;
	pack.dwVID = dwVID;
	pack.dwTime = dwTime ? dwTime : get_dword_time();
	pack.bRot = bRot;
	pack.lX = x;
	pack.lY = y;
	pack.dwDuration = dwDuration;
}

void CHARACTER::RestartAtSamePos()
{
	if (m_bIsObserver)
		return;

	EncodeRemovePacket(this);
	EncodeInsertPacket(this);

	ENTITY_MAP::iterator it = m_map_view.begin();

	while (it != m_map_view.end())
	{
		LPENTITY entity = (it++)->first;
		if (!entity)
			return;

		EncodeRemovePacket(entity);
		if (!m_bIsObserver)
			EncodeInsertPacket(entity);

		if (entity->IsType(ENTITY_CHARACTER))
		{
			const LPCHARACTER& lpChar = dynamic_cast<LPCHARACTER>(entity);
			if (!lpChar)
				return;

			if (lpChar->IsPC() || lpChar->IsNPC() || lpChar->IsMonster())
			{
				if (!entity->IsObserverMode())
					entity->EncodeInsertPacket(this);
			}
		}
		else
		{
			if (!entity->IsObserverMode())
			{
				entity->EncodeInsertPacket(this);
			}
		}
	}
}

// Sends a packet to the entity stating that I have appeared.
void CHARACTER::EncodeInsertPacket(LPENTITY entity)
{
	LPDESC d;
	if (!(d = entity->GetDesc()))
		return;

	// Guild name bug fix code
	LPCHARACTER ch = dynamic_cast<LPCHARACTER>(entity);
	ch->SendGuildName(GetGuild());
	// Guild name bug fix code

	TPacketGCCharacterAdd pack{};
	pack.header = HEADER_GC_CHARACTER_ADD;
	pack.dwVID = m_vid;
#ifdef WJ_SHOW_MOB_INFO
	if (IsMonster() || IsStone())
	{
		pack.dwLevel = GetLevel();
		pack.dwAIFlag = IsMonster() ? GetAIFlag() : 0;
	}
	else
	{
		pack.dwLevel = 0;
		pack.dwAIFlag = 0;
	}
#endif

	pack.bType = GetCharType();
	pack.angle = GetRotation();
	pack.x = GetX();
	pack.y = GetY();
	pack.z = GetZ();
	pack.wRaceNum = GetRaceNum();

#ifdef ENABLE_PORTAL_LEVEL_LIMIT
	pack.iPortalMinLevelLimit = 0;
	pack.iPortalMaxLevelLimit = 0;

	if (IsWarp())
	{
		const std::pair<int, int>* pPortalLevelLimit = CHARACTER_MANAGER::Instance().GetPortalLevelLimit(GetRaceNum());
		if (pPortalLevelLimit != nullptr)
		{
			pack.iPortalMinLevelLimit = pPortalLevelLimit->first;
			pack.iPortalMaxLevelLimit = pPortalLevelLimit->second;
		}
	}
#endif

	if (IsPet()
#ifdef ENABLE_GROWTH_PET_SYSTEM
		|| IsGrowthPet()
#endif
		)
	{
		pack.bMovingSpeed = 150;
	}
	else
	{
#ifdef ENABLE_YOHARA_SYSTEM
		if (IsPC() && IsSungmaMap() && (GetPoint(POINT_SUNGMA_MOVE) < GetSungmaMapAttribute(POINT_SUNGMA_MOVE)))
			pack.bMovingSpeed = static_cast<uint8_t>(GetLimitPoint(POINT_MOV_SPEED)) / 2;
		else
			pack.bMovingSpeed = static_cast<uint8_t>(GetLimitPoint(POINT_MOV_SPEED));
#else
		pack.bMovingSpeed = static_cast<uint8_t>(GetLimitPoint(POINT_MOV_SPEED));
#endif
	}
	pack.bAttackSpeed = static_cast<uint8_t>(GetLimitPoint(POINT_ATT_SPEED));
	pack.dwAffectFlag[0] = m_afAffectFlag.bits[0];
	pack.dwAffectFlag[1] = m_afAffectFlag.bits[1];

	pack.bStateFlag = m_bAddChrState;

	int iDur = 0;

	if (m_posDest.x != pack.x || m_posDest.y != pack.y)
	{
		iDur = (m_dwMoveStartTime + m_dwMoveDuration) - get_dword_time();

		if (iDur <= 0)
		{
			pack.x = m_posDest.x;
			pack.y = m_posDest.y;
		}
	}

	d->Packet(&pack, sizeof(pack));

	if (IsPC() || m_bCharType == CHAR_TYPE_NPC
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
		|| m_bCharType == CHAR_TYPE_HORSE
		|| IsHorse()
#endif
#ifdef ENABLE_PROTO_RENEWAL
		|| m_bCharType == CHAR_TYPE_PET_PAY
		|| IsPet()
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
		|| m_bCharType == CHAR_TYPE_PET
		|| IsGrowthPet()
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		|| IsShop()
#endif
		)
	{
		TPacketGCCharacterAdditionalInfo addPacket;
		memset(&addPacket, 0, sizeof(TPacketGCCharacterAdditionalInfo));

		addPacket.header = HEADER_GC_CHAR_ADDITIONAL_INFO;
		addPacket.dwVID = m_vid;

		addPacket.adwPart[CHR_EQUIPPART_ARMOR] = GetPart(PART_MAIN);
		addPacket.adwPart[CHR_EQUIPPART_WEAPON] = GetPart(PART_WEAPON);
		addPacket.adwPart[CHR_EQUIPPART_HEAD] = GetPart(PART_HEAD);
		addPacket.adwPart[CHR_EQUIPPART_HAIR] = GetPart(PART_HAIR);
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		addPacket.adwPart[CHR_EQUIPPART_ACCE] = GetPart(PART_ACCE);
#endif
#ifdef ENABLE_AURA_SYSTEM
		addPacket.adwPart[CHR_EQUIPPART_AURA] = GetPart(PART_AURA);
#endif
#ifdef ENABLE_PENDANT
		addPacket.adwPart[CHR_EQUIPPART_PENDANT] = GetPart(PART_PENDANT);
#endif

#ifdef ENABLE_REFINE_ELEMENT
		addPacket.dwElementsEffect = GetElementsEffect();
#endif

		addPacket.bPKMode = m_bPKMode;
		addPacket.dwMountVnum = GetMountVnum();
#ifdef ENABLE_QUIVER_SYSTEM
		addPacket.dwArrow = (IsPC() && GetWear(WEAR_ARROW) != nullptr) ? GetWear(WEAR_ARROW)->GetOriginalVnum() : 0;
#endif
		addPacket.bEmpire = m_bEmpire;
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		addPacket.bLanguage = IsPC() ? GetDesc()->GetLanguage() : LANGUAGE_NONE;
#endif

		if (IsPC()
#ifdef ENABLE_GROWTH_PET_SYSTEM
			|| IsGrowthPet()
#endif
#if defined(ENABLE_PREMIUM_PRIVATE_SHOP) && defined(ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL)
			|| IsShop()
#endif
			)
		{
			addPacket.dwLevel = GetLevel();
#ifdef ENABLE_YOHARA_SYSTEM
			addPacket.dwcLevel = GetConquerorLevel();
#endif
		}
		else
		{
			addPacket.dwLevel = 0;
#ifdef ENABLE_YOHARA_SYSTEM
			addPacket.dwcLevel = 0;
#endif
		}

#ifdef ENABLE_GROUP_DAMAGE_WEAPON
		LPITEM weapon = GetWear(WEAR_WEAPON);
		addPacket.lGroupWeapon = weapon ? weapon->GetAttributeValue(5) : 0;
#endif

		if (false)
		{
			const LPCHARACTER& pCh = dynamic_cast<LPCHARACTER>(entity);
			if (GetEmpire() == pCh->GetEmpire() || pCh->GetGMLevel() > GM_PLAYER || m_bCharType == CHAR_TYPE_NPC)
			{
				goto show_all_info;
			}
			else
			{
				memset(addPacket.name, 0, CHARACTER_NAME_MAX_LEN);
				addPacket.dwGuildID = 0;
				addPacket.sAlignment = 0;
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
				addPacket.dwTitle = 0;
#endif
			}
		}
		else
		{
		show_all_info:
#ifdef ENABLE_SKILL_COLOR_SYSTEM
			memcpy(addPacket.dwSkillColor, GetSkillColor(), sizeof(addPacket.dwSkillColor));
#endif
			strlcpy(addPacket.name, GetName(), sizeof(addPacket.name));

#ifdef ENABLE_SHOW_GUILD_LEADER
			if (GetGuild() != nullptr)
			{
				addPacket.dwGuildID = GetGuild()->GetID();
				CGuild* pGuild = this->GetGuild();
				if (pGuild->GetMasterPID() == GetPlayerID())
					addPacket.dwNewIsGuildName = 3;
				else if (pGuild->IsCoLeader(GetPlayerID()))
					addPacket.dwNewIsGuildName = 2;
				else
					addPacket.dwNewIsGuildName = 1;
			}
			else
			{
				addPacket.dwGuildID = 0;
				addPacket.dwNewIsGuildName = 0;
			}
#else
			if (GetGuild() != nullptr)
				addPacket.dwGuildID = GetGuild()->GetID();
			else
				addPacket.dwGuildID = 0;
#endif

			addPacket.sAlignment = m_iAlignment / 10;
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
			addPacket.dwTitle = GetAchievementTitle();
#endif
		}

#ifdef ENABLE_BATTLE_FIELD
		if (CBattleField::Instance().IsBattleZoneMapIndex(GetMapIndex()))
		{
			if (GetName() != ch->GetName())
			{
				memset(addPacket.name, 0, CHARACTER_NAME_MAX_LEN);
				addPacket.dwGuildID = 0;
				addPacket.sAlignment = 0;
				addPacket.dwLevel = 0;
#	ifdef ENABLE_YOHARA_SYSTEM
				addPacket.dwcLevel = 0;
#	endif
			}
		}
#endif

		d->Packet(&addPacket, sizeof(TPacketGCCharacterAdditionalInfo));
	}

	if (iDur)
	{
		TPacketGCMove packMove;
		EncodeMovePacket(packMove, GetVID(), FUNC_MOVE, 0, m_posDest.x, m_posDest.y, iDur, 0, static_cast<uint8_t>(GetRotation() / 5));
		d->Packet(&packMove, sizeof(packMove));

		TPacketGCWalkMode p{};
		p.vid = GetVID();
		p.header = HEADER_GC_WALK_MODE;
		p.mode = m_bNowWalking ? static_cast<uint8_t>(WalkMode::WALKMODE_WALK) : static_cast<uint8_t>(WalkMode::WALKMODE_RUN);

		d->Packet(&p, sizeof(p));
	}

	if (entity->IsType(ENTITY_CHARACTER) && GetDesc())
	{
		const LPCHARACTER& pChr = dynamic_cast<LPCHARACTER>(entity);
		if (pChr->IsWalking())
		{
			TPacketGCWalkMode p{};
			p.vid = pChr->GetVID();
			p.header = HEADER_GC_WALK_MODE;
			p.mode = pChr->m_bNowWalking ? static_cast<uint8_t>(WalkMode::WALKMODE_WALK) : static_cast<uint8_t>(WalkMode::WALKMODE_RUN);
			GetDesc()->Packet(&p, sizeof(p));
		}
	}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (IsShop())
	{
		TPacketGCShopSign p{};

		p.bHeader = HEADER_GC_SHOP_SIGN;
		p.dwVID = GetVID();
		strlcpy(p.szSign, GetShopSign().c_str(), sizeof(p.szSign));
#	ifdef ENABLE_MYSHOP_DECO_PREMIUM
		p.bType = GetMyShop() ? GetMyShop()->GetTitleType() : 0;
#	endif
		d->Packet(&p, sizeof(TPacketGCShopSign));
	}
#else
	if (GetMyShop())
	{
		TPacketGCShopSign p{};

		p.bHeader = HEADER_GC_SHOP_SIGN;
		p.dwVID = GetVID();
		strlcpy(p.szSign, m_stShopSign.c_str(), sizeof(p.szSign));
#ifdef ENABLE_MYSHOP_DECO
		p.bType = m_bMyPrivShopType;
#endif

		d->Packet(&p, sizeof(TPacketGCShopSign));
	}
#endif

	if (entity->IsType(ENTITY_CHARACTER))
	{
		sys_log(3, "EntityInsert %s (RaceNum %d) (%d %d) TO %s",
			GetName(), GetRaceNum(), GetX() / SECTREE_SIZE, GetY() / SECTREE_SIZE, (dynamic_cast<LPCHARACTER>(entity)->GetName()));
	}
}

void CHARACTER::EncodeRemovePacket(LPENTITY entity)
{
	if (!entity)
		return;

	if (entity->GetType() != ENTITY_CHARACTER)
		return;

	LPDESC d;

	if (!(d = entity->GetDesc()))
		return;

	TPacketGCCharacterDelete pack{};

	pack.header = HEADER_GC_CHARACTER_DEL;
	pack.id = m_vid;

	d->Packet(&pack, sizeof(TPacketGCCharacterDelete));

	if (entity->IsType(ENTITY_CHARACTER))
		sys_log(3, "EntityRemove %s(%d) FROM %s", GetName(), (uint32_t)m_vid, dynamic_cast<LPCHARACTER>(entity)->GetName());
}

void CHARACTER::UpdatePacket()
{
	if (GetSectree() == nullptr)
		return;

	if (IsPC() && (!GetDesc() || !GetDesc()->GetCharacter()))
		return;

	TPacketGCCharacterUpdate pack{};
	TPacketGCCharacterUpdate pack2{};

	pack.header = HEADER_GC_CHARACTER_UPDATE;
	pack.dwVID = m_vid;

	pack.adwPart[CHR_EQUIPPART_ARMOR] = GetPart(PART_MAIN);
	pack.adwPart[CHR_EQUIPPART_WEAPON] = GetPart(PART_WEAPON);
	pack.adwPart[CHR_EQUIPPART_HEAD] = GetPart(PART_HEAD);
	pack.adwPart[CHR_EQUIPPART_HAIR] = GetPart(PART_HAIR);
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	pack.adwPart[CHR_EQUIPPART_ACCE] = GetPart(PART_ACCE);
#endif
#ifdef ENABLE_AURA_SYSTEM
	pack.adwPart[CHR_EQUIPPART_AURA] = GetPart(PART_AURA);
#endif
#ifdef ENABLE_PENDANT
	pack.adwPart[CHR_EQUIPPART_PENDANT] = GetPart(PART_PENDANT);
#endif

#ifdef ENABLE_REFINE_ELEMENT
	pack.dwElementsEffect = GetElementsEffect();
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
	memcpy(pack.dwSkillColor, GetSkillColor(), sizeof(pack.dwSkillColor));
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	if (IsPC() && IsSungmaMap() && (GetPoint(POINT_SUNGMA_MOVE) < GetSungmaMapAttribute(POINT_SUNGMA_MOVE)))
		pack.bMovingSpeed = static_cast<uint8_t>(GetLimitPoint(POINT_MOV_SPEED)) / 2;
	else
		pack.bMovingSpeed = static_cast<uint8_t>(GetLimitPoint(POINT_MOV_SPEED));
#else
	pack.bMovingSpeed = static_cast<uint8_t>(GetLimitPoint(POINT_MOV_SPEED));
#endif
	pack.bAttackSpeed = static_cast<uint8_t>(GetLimitPoint(POINT_ATT_SPEED));
	pack.bStateFlag = m_bAddChrState;
	pack.dwAffectFlag[0] = m_afAffectFlag.bits[0];
	pack.dwAffectFlag[1] = m_afAffectFlag.bits[1];
	pack.dwGuildID = 0;
	pack.sAlignment = m_iAlignment / 10;
#ifdef WJ_SHOW_MOB_INFO
	pack.dwLevel = GetLevel();
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	pack.dwcLevel = GetConquerorLevel();
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	pack.bLanguage = IsPC() ? GetDesc()->GetLanguage() : LANGUAGE_NONE;
#endif
	pack.bPKMode = m_bPKMode;

	if (GetGuild())
		pack.dwGuildID = GetGuild()->GetID();
#ifdef ENABLE_SHOW_GUILD_LEADER
	CGuild* pGuild = this->GetGuild();
	if (pGuild)
	{
		if (pGuild->GetMasterPID() == GetPlayerID())
			pack.dwNewIsGuildName = 3;
		else if (pGuild->IsCoLeader(GetPlayerID()))
			pack.dwNewIsGuildName = 2;
		else
			pack.dwNewIsGuildName = 1;
	}
	else
		pack.dwNewIsGuildName = 0;
#endif

	pack.dwMountVnum = GetMountVnum();
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	pack.dwTitle = GetAchievementTitle();
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	pack.dwArrow = GetWear(WEAR_ARROW) != nullptr ? GetWear(WEAR_ARROW)->GetOriginalVnum() : 0;
#endif
#ifdef ENABLE_GROUP_DAMAGE_WEAPON
	LPITEM weapon = GetWear(WEAR_WEAPON);
	pack.lGroupWeapon = weapon ? weapon->GetAttributeValue(5) : 0;
#endif

	pack2 = pack;
	pack2.dwGuildID = 0;
	pack2.sAlignment = 0;
#ifdef WJ_SHOW_MOB_INFO
	pack2.dwLevel = 0;
#endif

	if (false)
	{
		if (m_bIsObserver != true)
		{
			for (ENTITY_MAP::iterator iter = m_map_view.begin(); iter != m_map_view.end(); iter++)
			{
				const LPENTITY& pEntity = iter->first;
				if (pEntity != nullptr)
				{
					if (pEntity->IsType(ENTITY_CHARACTER) == true)
					{
						if (pEntity->GetDesc() != nullptr)
						{
							const LPCHARACTER& pChar = dynamic_cast<LPCHARACTER>(pEntity);
							if (!pChar)
								return;

							if (GetEmpire() == pChar->GetEmpire() || pChar->GetGMLevel() > GM_PLAYER)
								pEntity->GetDesc()->Packet(&pack, sizeof(pack));
							else
								pEntity->GetDesc()->Packet(&pack2, sizeof(pack2));
						}
					}
					else
					{
						if (pEntity->GetDesc() != nullptr)
							pEntity->GetDesc()->Packet(&pack, sizeof(pack));
					}
				}
			}
		}

		if (GetDesc() != nullptr)
		{
			GetDesc()->Packet(&pack, sizeof(pack));
		}
	}
	else
	{
#ifdef ENABLE_BATTLE_FIELD
		if (CBattleField::Instance().IsBattleZoneMapIndex(GetMapIndex()))
		{
			if (GetDesc() != nullptr)
			{
				GetDesc()->Packet(&pack, sizeof(pack));
				PacketAround(&pack2, sizeof(pack2), this);
			}
			return;
		}
#endif

		PacketAround(&pack, sizeof(pack));
	}
}

LPCHARACTER CHARACTER::FindCharacterInView(const char* c_pszName, bool bFindPCOnly)
{
	ENTITY_MAP::iterator it = m_map_view.begin();

	for (; it != m_map_view.end(); ++it)
	{
		if (!it->first->IsType(ENTITY_CHARACTER))
			continue;

		LPCHARACTER tch = dynamic_cast<LPCHARACTER>(it->first);
		if (tch && bFindPCOnly && tch->IsNPC())
			continue;

		if (tch && !strcasecmp(tch->GetName(), c_pszName))
			return (tch);
	}

	return nullptr;
}

void CHARACTER::SetPosition(int pos)
{
	if (pos == POS_STANDING)
	{
		REMOVE_BIT(m_bAddChrState, ADD_CHARACTER_STATE_DEAD);
		REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_STUN);

		event_cancel(&m_pkDeadEvent);
		event_cancel(&m_pkStunEvent);
	}
	else if (pos == POS_DEAD)
		SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_DEAD);

	if (!IsStone() && !IsDoor())	//@fixme480
	{
		switch (pos)
		{
			case POS_FIGHTING:
				if (!IsState(m_stateBattle))
					MonsterLog("[BATTLE] fighting state");

				GotoState(m_stateBattle);
				break;

			default:
				if (!IsState(m_stateIdle))
					MonsterLog("[IDLE] resting state");

				GotoState(m_stateIdle);
				break;
		}
	}

	m_pointsInstant.position = pos;
}

void CHARACTER::Save()
{
	if (!m_bSkipSave)
		CHARACTER_MANAGER::Instance().DelayedSave(this);
}

void CHARACTER::CreatePlayerProto(TPlayerTable& tab)
{
	memset(&tab, 0, sizeof(TPlayerTable));

	if (GetNewName().empty())
	{
		strlcpy(tab.name, GetName(), sizeof(tab.name));
	}
	else
	{
		strlcpy(tab.name, GetNewName().c_str(), sizeof(tab.name));
	}

	strlcpy(tab.ip, GetDesc()->GetHostName(), sizeof(tab.ip));

	tab.id = m_dwPlayerID;
	tab.voice = GetPoint(POINT_VOICE);
	tab.level = GetLevel();
	tab.level_step = GetPoint(POINT_LEVEL_STEP);
	tab.exp = GetExp();
	tab.gold = GetGold();
	tab.job = m_points.job;
	tab.part_base = m_pointsInstant.bBasePart;
	tab.skill_group = m_points.skill_group;
#ifdef ENABLE_CHEQUE_SYSTEM
	tab.cheque = GetCheque();
#endif
#ifdef ENABLE_BATTLE_FIELD
	tab.battle_point = GetBattlePoint();
	tab.aiShopExUsablePoint[SHOP_COIN_TYPE_BATTLE_POINT] = GetShopExUsablePoint(SHOP_COIN_TYPE_BATTLE_POINT);
	tab.aiShopExDailyUse[SHOP_COIN_TYPE_BATTLE_POINT] = GetShopExDailyTimePoint(SHOP_COIN_TYPE_BATTLE_POINT);
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
	tab.medal_honor = GetMedalHonor();
	tab.aiShopExUsablePoint[SHOP_COIN_TYPE_MEDAL_OF_HONOR] = GetShopExUsablePoint(SHOP_COIN_TYPE_MEDAL_OF_HONOR);
	tab.aiShopExDailyUse[SHOP_COIN_TYPE_MEDAL_OF_HONOR] = GetShopExDailyTimePoint(SHOP_COIN_TYPE_MEDAL_OF_HONOR);
#endif
#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	tab.inventory_stage = GetExtendInvenStage();
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	for (int i = 0; i < 3; ++i) {
		tab.special_stage[i] = GetExtendSpecialInvenStage(i);
	}
# endif
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	tab.soul = GetSoulPoint();
	tab.soulre = GetSoulRePoint();
#endif
#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_REWARD_TABLE)
	thecore_memcpy(tab.ziRewardTable, m_ziRewardTable, sizeof(TZiRewardTable) * 2);
#endif

#ifdef ENABLE_GEM_SYSTEM
	tab.gem_point = GetGemPoint();
#endif
#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	tab.gem_next_refresh = GetGemNextRefresh();
	thecore_memcpy(tab.gemItems, m_gemItems, sizeof(TPlayerGemItems) * GEM_SHOP_HOST_ITEM_MAX_NUM);
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	tab.conqueror_level = GetConquerorLevel();
	tab.conqueror_level_step = GetPoint(POINT_CONQUEROR_LEVEL_STEP);
	tab.conqueror_exp = GetConquerorExp();
#endif

#ifdef ENABLE_FLOWER_EVENT
	thecore_memcpy(&tab.flowerEvent[0], m_FlowerEvent, sizeof(TFlowerEvent) * PART_FLOWER_MAX_NUM);
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	tab.page_equipment = GetPageEquipment();
	tab.unlock_page_equipment = GetPageTotalEquipment();
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
	tab.lotto_moneypool = GetLottoMoney();
	tab.lotto_totalmoneywin = GetLottoTotalMoney();
#endif

	const uint32_t dwPlayedTime = (get_dword_time() - m_dwPlayStartTime);

	if (dwPlayedTime > 60000)
	{
		if (GetSectree() && !GetSectree()->IsAttr(GetX(), GetY(), ATTR_BANPK))
		{
			if (GetRealAlignment() < 0)
			{
				if (IsEquipUniqueItem(UNIQUE_ITEM_FASTER_ALIGNMENT_UP_BY_TIME))
					UpdateAlignment(120 * (dwPlayedTime / 60000));
				else
					UpdateAlignment(60 * (dwPlayedTime / 60000));
			}
			else
				UpdateAlignment(5 * (dwPlayedTime / 60000));
		}

		SetRealPoint(POINT_PLAYTIME, GetRealPoint(POINT_PLAYTIME) + dwPlayedTime / 60000);
		ResetPlayTime(dwPlayedTime % 60000);
	}

	tab.playtime = GetRealPoint(POINT_PLAYTIME);
#ifdef NEW_SELECT_CHARACTER
	tab.last_playtime = get_global_time();
#endif
	tab.lAlignment = m_iRealAlignment;
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	CAchievementSystem::Instance().OnCharacterUpdate(this);
#endif

	if (m_posWarp.x != 0 || m_posWarp.y != 0)
	{
		tab.x = m_posWarp.x;
		tab.y = m_posWarp.y;
		tab.z = 0;
		tab.lMapIndex = m_lWarpMapIndex;
	}
	else
	{
		tab.x = GetX();
		tab.y = GetY();
		tab.z = GetZ();
		tab.lMapIndex = GetMapIndex();
	}

	if (m_lExitMapIndex == 0)
	{
		tab.lExitMapIndex = tab.lMapIndex;
		tab.lExitX = tab.x;
		tab.lExitY = tab.y;
	}
	else
	{
		tab.lExitMapIndex = m_lExitMapIndex;
		tab.lExitX = m_posExit.x;
		tab.lExitY = m_posExit.y;
	}

	sys_log(0, "SAVE: %s %dx%d", GetName(), tab.x, tab.y);

	tab.st = GetRealPoint(POINT_ST);
	tab.ht = GetRealPoint(POINT_HT);
	tab.dx = GetRealPoint(POINT_DX);
	tab.iq = GetRealPoint(POINT_IQ);

	tab.stat_point = GetPoint(POINT_STAT);
	tab.skill_point = GetPoint(POINT_SKILL);
	tab.sub_skill_point = GetPoint(POINT_SUB_SKILL);
	tab.horse_skill_point = GetPoint(POINT_HORSE_SKILL);

	tab.stat_reset_count = GetPoint(POINT_STAT_RESET_COUNT);
#ifdef ENABLE_YOHARA_SYSTEM
	tab.conqueror_st = GetRealPoint(POINT_SUNGMA_STR);
	tab.conqueror_ht = GetRealPoint(POINT_SUNGMA_HP);
	tab.conqueror_mov = GetRealPoint(POINT_SUNGMA_MOVE);
	tab.conqueror_imu = GetRealPoint(POINT_SUNGMA_IMMUNE);

	tab.conqueror_point = GetPoint(POINT_CONQUEROR_POINT);
#endif

	tab.hp = GetHP();
	tab.sp = GetSP();

	tab.stamina = GetStamina();

	tab.sRandomHP = m_points.iRandomHP;
	tab.sRandomSP = m_points.iRandomSP;

#ifdef ENABLE_HORSE_APPEARANCE
	tab.sHorse_appearance = m_points.horse_appearance;
#endif

	for (int i = 0; i < QUICKSLOT_MAX_NUM; ++i)
		tab.quickslot[i] = m_quickslot[i];

	if (m_stMobile.length() && !*m_szMobileAuth)
		strlcpy(tab.szMobile, m_stMobile.c_str(), sizeof(tab.szMobile));

	thecore_memcpy(tab.parts, m_pointsInstant.parts, sizeof(tab.parts));

	// REMOVE_REAL_SKILL_LEVLES
	thecore_memcpy(tab.skills, m_pSkillLevels, sizeof(TPlayerSkill) * SKILL_MAX_NUM);
	// END_OF_REMOVE_REAL_SKILL_LEVLES

#ifdef ENABLE_FISH_EVENT
	thecore_memcpy(tab.fishSlots, m_fishSlots, sizeof(TPlayerFishEventSlot) * FISH_EVENT_SLOTS_NUM);
	tab.fishEventUseCount = GetFishEventUseCount();
#endif

#ifdef ENABLE_BIOLOG_SYSTEM
	tab.m_BiologActualMission = GetBiologMissions();
	tab.m_BiologCollectedItems = GetBiologCollectedItems();
	tab.m_BiologCooldownReminder = GetBiologCooldownReminder();
	tab.m_BiologCooldown = GetBiologCooldown();
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
	tab.battle_pass_premium_id = GetExtBattlePassPremiumID();
#endif

	tab.horse = GetHorseData();
}

void CHARACTER::SaveReal()
{
	if (m_bSkipSave)
		return;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (IsShop())
	{
		sys_err("SaveReal should not be happening on a shop char (PID %u)", GetPlayerID());
		return;
	}
#endif

	if (!GetDesc())
	{
		sys_err("Character::Save : no descriptor when saving (name: %s)", GetName());
		return;
	}

	//Save player
	TPlayerTable table;
	CreatePlayerProto(table);

	db_clientdesc->DBPacket(HEADER_GD_PLAYER_SAVE, GetDesc()->GetHandle(), &table, sizeof(TPlayerTable));

	//Save quest
	quest::PC* pkQuestPC = quest::CQuestManager::Instance().GetPCForce(GetPlayerID());

	if (!pkQuestPC)
		sys_err("CHARACTER::Save : null quest::PC pointer! (name %s)", GetName());
	else
	{
		pkQuestPC->Save();
	}

	//Save marriage (if existing)
	marriage::TMarriage* pMarriage = marriage::CManager::Instance().Get(GetPlayerID());
	if (pMarriage)
		pMarriage->Save();
}

void CHARACTER::FlushDelayedSaveItem()
{
	// Save all unsaved items.
	LPITEM item = nullptr;

	for (int i = 0; i < INVENTORY_SLOT_COUNT; ++i)
	{
		if ((item = GetInventoryItem(i)))
			ITEM_MANAGER::Instance().FlushDelayedSave(item);
	}

	for (int i = 0; i < EQUIPMENT_SLOT_COUNT; ++i)
	{
		if ((item = GetEquipmentItem(i)))
			ITEM_MANAGER::Instance().FlushDelayedSave(item);
	}

	for (int i = 0; i < BELT_INVENTORY_SLOT_COUNT; ++i)
	{
		if ((item = GetBeltInventoryItem(i)))
			ITEM_MANAGER::Instance().FlushDelayedSave(item);
	}

	for (int i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
	{
		if ((item = GetDSInventoryItem(i)))
			ITEM_MANAGER::Instance().FlushDelayedSave(item);
	}
}

#ifdef ENABLE_VIP_SYSTEM
void CHARACTER::RefreshVip()
{
	m_afAffectFlag.Set(AFF_VIP);
}
#endif

void CHARACTER::Disconnect(const char* c_pszReason)
{
	assert(GetDesc() != nullptr);

	sys_log(0, "DISCONNECT: %s (%s)", GetName(), c_pszReason ? c_pszReason : "unset");

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	if (GetGuild() && IsOpenGuildstorage())
	{
		GetGuild()->SetStorageState(false, 0);
	}
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (GetViewingShop())
	{
		GetViewingShop()->RemoveGuest(this);
		SetViewingShop(nullptr);
	}
#else
	if (GetShop())
	{
		GetShop()->RemoveGuest(this);
		SetShop(nullptr);
	}
#endif

	if (GetArena() != nullptr)
	{
		GetArena()->OnDisconnect(GetPlayerID());
	}

	if (GetParty() != nullptr)
	{
		GetParty()->UpdateOfflineState(GetPlayerID());
	}

#ifdef NEW_SELECT_CHARACTER
	if (GetDesc())
	{
		TAccountTable& acc_table = GetDesc()->GetAccountTable();
		for (int i = 0; i < PLAYER_PER_ACCOUNT; ++i)
		{
			if (!strncmp(acc_table.players[i].szName, GetName(), CHARACTER_NAME_MAX_LEN))
			{
				acc_table.players[i].wMainPart = GetPart(PART_MAIN);
				acc_table.players[i].wHairPart = GetPart(PART_HAIR);
#	ifdef ENABLE_ACCE_COSTUME_SYSTEM
				acc_table.players[i].wAccePart = GetPart(PART_ACCE);
#	endif
				acc_table.players[i].dwLastPlayTime = get_global_time();
			}
		}
		SPacketCharacterInformation pack{};
		pack.bHeader = HEADER_GC_CHARACTER_INFORMATION;
		thecore_memcpy(pack.players, acc_table.players, sizeof(acc_table.players));
		GetDesc()->Packet(&pack, sizeof(SPacketCharacterInformation));
	}
#endif

	marriage::CManager::Instance().Logout(this);

#ifdef ENABLE_AUTO_SYSTEM
	if (IsAffectFlag(AFF_AUTO_USE))
		RemoveAffect(AFFECT_AUTO);
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	CAchievementSystem::Instance().OnLogout(this);
#endif

#ifdef ENABLE_PARTY_MATCH
	CGroupMatchManager::Instance().StopSearching(this, CGroupMatchManager::EPartyMatchMsg::PARTY_MATCH_FAIL, 0);
#endif

	// P2P Logout
	TPacketGGLogout p{};
	p.bHeader = HEADER_GG_LOGOUT;
	strlcpy(p.szName, GetName(), sizeof(p.szName));
	P2P_MANAGER::Instance().Send(&p, sizeof(TPacketGGLogout));
	LogManager::Instance().CharLog(this, 0, "LOGOUT", "");

	if (m_pWarMap)
		SetWarMap(nullptr);

	if (m_pWeddingMap)
	{
		SetWeddingMap(nullptr);
	}

	if (GetGuild())
		GetGuild()->LogoutMember(this);

	quest::CQuestManager::Instance().LogoutPC(this);

	if (GetParty())
		GetParty()->Unlink(this);

	// If you disconnect when you die, you will lose experience.
	if (IsStun() || IsDead())
	{
		DeathPenalty(0);
		PointChange(POINT_HP, 50 - GetHP());
	}


	if (!CHARACTER_MANAGER::Instance().FlushDelayedSave(this))
	{
		SaveReal();
	}

#ifdef ENABLE_MULTI_FARM_BLOCK
	CHARACTER_MANAGER::Instance().CheckMultiFarmAccount(GetDesc() ? GetDesc()->GetHostName() : "", GetPlayerID(), GetName(), false);
#endif

	FlushDelayedSaveItem();

	SaveAffect();
	m_bIsLoadedAffect = false;

#ifdef ENABLE_BIOLOG_SYSTEM
	if (m_pkBiologManager)
	{
		delete m_pkBiologManager;
		m_pkBiologManager = nullptr;
	}
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
	ListExtBattlePassMap::iterator itext = m_listExtBattlePass.begin();
	while (itext != m_listExtBattlePass.end())
	{
		TPlayerExtBattlePassMission* pkMission = *itext++;

		if (!pkMission->bIsUpdated)
			continue;

		db_clientdesc->DBPacket(HEADER_GD_SAVE_EXT_BATTLE_PASS, 0, pkMission, sizeof(TPlayerExtBattlePassMission));
	}
	m_bIsLoadedExtBattlePass = false;
#endif

	m_bSkipSave = true; // After this, you must not save any more.

	quest::CQuestManager::Instance().DisconnectPC(this);

	CloseSafebox();
	CloseMall();
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	CloseGuildstorage();
#endif

#ifdef ENABLE_AURA_SYSTEM
	if (IsAuraRefineWindowOpen())
		AuraRefineWindowClose();
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	/*if (IsChangeLookWindowOpen())
		ChangeLookWindowClose();*/
#endif

	CPVPManager::Instance().Disconnect(this);

	CTargetManager::Instance().Logout(GetPlayerID());

	MessengerManager::Instance().Logout(GetName());

	if (GetDesc())
	{
		GetDesc()->BindCharacter(nullptr);
		// BindDesc(nullptr);
	}

#if defined(ENABLE_PREMIUM_PRIVATE_SHOP) && !defined(ENABLE_SHOP_NO_SPEND_MIN_IF_ONLINE)
	const uint32_t pid = GetPlayerID();
	db_clientdesc->DBPacket(HEADER_GD_PLAYER_LOGOUT, 0, &pid, sizeof(uint32_t));
#endif

	M2_DESTROY_CHARACTER(this);
}

bool CHARACTER::Show(long lMapIndex, long x, long y, long z, bool bShowSpawnMotion/* = false */)
{
	LPSECTREE sectree = SECTREE_MANAGER::Instance().Get(lMapIndex, x, y);

	if (!sectree)
	{
		sys_log(0, "CHARACTER::Show: cannot find sectree by %dx%d mapindex %d", x, y, lMapIndex);
		return false;
	}

	SetMapIndex(lMapIndex);

#ifdef ENABLE_LOADING_TIP
	ChatPacket(CHAT_TYPE_COMMAND, "target_map %ld", lMapIndex);
#endif

	bool bChangeTree = false;

	if (!GetSectree() || GetSectree() != sectree)
		bChangeTree = true;

	if (bChangeTree)
	{
		if (GetSectree())
			GetSectree()->RemoveEntity(this);

		ViewCleanup();
	}

	if (!IsNPC())
	{
		sys_log(0, "SHOW: %s %dx%dx%d", GetName(), x, y, z);

#ifdef ENABLE_CHECK_MOVESPEED_HACK
		GetCShield()->ResetMoveSpeedhack(get_dword_time());
#endif

		if (GetStamina() < GetMaxStamina())
			StartAffectEvent();
	}
	else if (m_pkMobData)
	{
		m_pkMobInst->m_posLastAttacked.x = x;
		m_pkMobInst->m_posLastAttacked.y = y;
		m_pkMobInst->m_posLastAttacked.z = z;
	}

	if (bShowSpawnMotion)
	{
		SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_SPAWN);
		m_afAffectFlag.Set(AFF_SPAWN);
	}

	SetXYZ(x, y, z);

	m_posDest.x = x;
	m_posDest.y = y;
	m_posDest.z = z;

	m_posStart.x = x;
	m_posStart.y = y;
	m_posStart.z = z;

	if (bChangeTree)
	{
		EncodeInsertPacket(this);
		sectree->InsertEntity(this);

		UpdateSectree();
	}
	else
	{
		ViewReencode();
		sys_log(0, " in same sectree");
	}

	REMOVE_BIT(m_bAddChrState, ADD_CHARACTER_STATE_SPAWN);

	SetValidComboInterval(0);
	return true;
}

// BGM_INFO
struct BGMInfo
{
	std::string name;
	float vol;
};

typedef std::map<unsigned, BGMInfo> BGMInfoMap;

static BGMInfoMap gs_bgmInfoMap;
static bool gs_bgmVolEnable = false;

void CHARACTER_SetBGMVolumeEnable() noexcept
{
	gs_bgmVolEnable = true;
	sys_log(0, "bgm_info.set_bgm_volume_enable");
}

void CHARACTER_AddBGMInfo(unsigned mapIndex, const char* name, float vol)
{
	BGMInfo newInfo;
	newInfo.name = name;
	newInfo.vol = vol;

	gs_bgmInfoMap[mapIndex] = newInfo;

	sys_log(0, "bgm_info.add_info(%d, '%s', %f)", mapIndex, name, vol);
}

const BGMInfo& CHARACTER_GetBGMInfo(unsigned mapIndex)
{
	const BGMInfoMap::iterator f = gs_bgmInfoMap.find(mapIndex);
	if (gs_bgmInfoMap.end() == f)
	{
		static BGMInfo s_empty = { "", 0.0f };
		return s_empty;
	}
	return f->second;
}

bool CHARACTER_IsBGMVolumeEnable() noexcept
{
	return gs_bgmVolEnable;
}
// END_OF_BGM_INFO

void CHARACTER::MainCharacterPacket()
{
#ifdef ENABLE_DUNGEON_MUSIC_FIX
	const unsigned mapIndex = GetMapIndex() < 10000 ? GetMapIndex() : GetMapIndex() / 10000;
#else
	const unsigned mapIndex = GetMapIndex();
#endif
	const BGMInfo& bgmInfo = CHARACTER_GetBGMInfo(mapIndex);

	// SUPPORT_BGM
	if (!bgmInfo.name.empty())
	{
		if (CHARACTER_IsBGMVolumeEnable())
		{
			sys_log(1, "bgm_info.play_bgm_vol(%d, name='%s', vol=%f)", mapIndex, bgmInfo.name.c_str(), bgmInfo.vol);
			TPacketGCMainCharacter4_BGM_VOL mainChrPacket{};
			mainChrPacket.header = HEADER_GC_MAIN_CHARACTER4_BGM_VOL;
			mainChrPacket.dwVID = m_vid;
			mainChrPacket.wRaceNum = GetRaceNum();
			mainChrPacket.lx = GetX();
			mainChrPacket.ly = GetY();
			mainChrPacket.lz = GetZ();
			mainChrPacket.empire = GetDesc()->GetEmpire();
			mainChrPacket.skill_group = GetSkillGroup();
			strlcpy(mainChrPacket.szChrName, GetName(), sizeof(mainChrPacket.szChrName));

			mainChrPacket.fBGMVol = bgmInfo.vol;
			strlcpy(mainChrPacket.szBGMName, bgmInfo.name.c_str(), sizeof(mainChrPacket.szBGMName));
			GetDesc()->Packet(&mainChrPacket, sizeof(TPacketGCMainCharacter4_BGM_VOL));
		}
		else
		{
			sys_log(1, "bgm_info.play(%d, '%s')", mapIndex, bgmInfo.name.c_str());
			TPacketGCMainCharacter3_BGM mainChrPacket{};
			mainChrPacket.header = HEADER_GC_MAIN_CHARACTER3_BGM;
			mainChrPacket.dwVID = m_vid;
			mainChrPacket.wRaceNum = GetRaceNum();
			mainChrPacket.lx = GetX();
			mainChrPacket.ly = GetY();
			mainChrPacket.lz = GetZ();
			mainChrPacket.empire = GetDesc()->GetEmpire();
			mainChrPacket.skill_group = GetSkillGroup();
			strlcpy(mainChrPacket.szChrName, GetName(), sizeof(mainChrPacket.szChrName));
			strlcpy(mainChrPacket.szBGMName, bgmInfo.name.c_str(), sizeof(mainChrPacket.szBGMName));
			GetDesc()->Packet(&mainChrPacket, sizeof(TPacketGCMainCharacter3_BGM));
		}
	}
	// END_OF_SUPPORT_BGM
	else
	{
		sys_log(0, "bgm_info.play(%d, DEFAULT_BGM_NAME)", mapIndex);

		TPacketGCMainCharacter pack{};
		pack.header = HEADER_GC_MAIN_CHARACTER;
		pack.dwVID = m_vid;
		pack.wRaceNum = GetRaceNum();
		pack.lx = GetX();
		pack.ly = GetY();
		pack.lz = GetZ();
		pack.empire = GetDesc()->GetEmpire();
		pack.skill_group = GetSkillGroup();
		strlcpy(pack.szName, GetName(), sizeof(pack.szName));
		GetDesc()->Packet(&pack, sizeof(TPacketGCMainCharacter));

		if (m_stMobile.length())
			ChatPacket(CHAT_TYPE_COMMAND, "sms");
	}
}

void CHARACTER::PointsPacket()
{
	if (!GetDesc())
		return;

	TPacketGCPoints pack{};

	pack.header = HEADER_GC_CHARACTER_POINTS;

	pack.points[POINT_LEVEL] = GetLevel();
	pack.points[POINT_EXP] = GetExp();
	pack.points[POINT_NEXT_EXP] = GetNextExp();
	pack.points[POINT_HP] = GetHP();
	pack.points[POINT_MAX_HP] = GetMaxHP();
	pack.points[POINT_SP] = GetSP();
	pack.points[POINT_MAX_SP] = GetMaxSP();
	pack.points[POINT_GOLD] = GetGold();
	pack.points[POINT_STAMINA] = GetStamina();
	pack.points[POINT_MAX_STAMINA] = GetMaxStamina();

	for (int i = POINT_ST; i < POINT_MAX_NUM; ++i)
		pack.points[i] = GetPoint(i);

#ifdef ENABLE_CHEQUE_SYSTEM
	pack.points[POINT_CHEQUE] = GetCheque();
#endif
#ifdef ENABLE_BATTLE_FIELD
	pack.points[POINT_BATTLE_FIELD] = GetBattlePoint();
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
	pack.points[POINT_MEDAL_OF_HONOR] = GetMedalHonor();
#endif
#ifdef ENABLE_GEM_SYSTEM
	pack.points[POINT_GEM] = GetGemPoint();
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	pack.points[POINT_CONQUEROR_LEVEL] = GetConquerorLevel();
	pack.points[POINT_CONQUEROR_EXP] = GetConquerorExp();
	pack.points[POINT_CONQUEROR_NEXT_EXP] = GetConquerorNextExp();
	pack.points[POINT_MOV_SPEED] = GetMove();
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	pack.points[POINT_SOUL] = GetSoulPoint();
	pack.points[POINT_SOUL_RE] = GetSoulRePoint();
#endif

#ifdef ENABLE_FLOWER_EVENT
	for (int i = POINT_FLOWER_TYPE_1; i < POINT_FLOWER_TYPE_6 + 1; ++i)
	{
		pack.points[i] = GetFlowerEventValue(i - POINT_FLOWER_TYPE_1);
	}
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
	pack.points[POINT_BATTLE_PASS_PREMIUM_ID] = GetExtBattlePassPremiumID();
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	pack.points[POINT_NEW_EQUIPMENT_ACTUAL] = GetPageEquipment();
	pack.points[POINT_NEW_EQUIPMENT_TOTAL] = GetPageTotalEquipment();
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
	pack.points[POINT_LOTTO_MONEY] = GetLottoMoney();
	pack.points[POINT_LOTTO_TOTAL_MONEY] = GetLottoTotalMoney();
#endif

	GetDesc()->Packet(&pack, sizeof(TPacketGCPoints));
}

bool CHARACTER::ChangeSex()
{
	const auto src_race = GetRaceNum();

	switch (src_race)
	{
		case MAIN_RACE_WARRIOR_M:
			m_points.job = MAIN_RACE_WARRIOR_W;
			break;

		case MAIN_RACE_WARRIOR_W:
			m_points.job = MAIN_RACE_WARRIOR_M;
			break;

		case MAIN_RACE_ASSASSIN_M:
			m_points.job = MAIN_RACE_ASSASSIN_W;
			break;

		case MAIN_RACE_ASSASSIN_W:
			m_points.job = MAIN_RACE_ASSASSIN_M;
			break;

		case MAIN_RACE_SURA_M:
			m_points.job = MAIN_RACE_SURA_W;
			break;

		case MAIN_RACE_SURA_W:
			m_points.job = MAIN_RACE_SURA_M;
			break;

		case MAIN_RACE_SHAMAN_M:
			m_points.job = MAIN_RACE_SHAMAN_W;
			break;

		case MAIN_RACE_SHAMAN_W:
			m_points.job = MAIN_RACE_SHAMAN_M;
			break;
#ifdef ENABLE_WOLFMAN_CHARACTER
		case MAIN_RACE_WOLFMAN_M:
			m_points.job = MAIN_RACE_WOLFMAN_M;
			break;
#endif
		default:
			sys_err("CHANGE_SEX: %s unknown race %d", GetName(), src_race);
			return false;
	}

	sys_log(0, "CHANGE_SEX: %s (%d -> %d)", GetName(), src_race, m_points.job);
	return true;
}

uint16_t CHARACTER::GetRaceNum() const noexcept
{
	if (m_dwPolymorphRace)
		return m_dwPolymorphRace;

	if (m_pkMobData)
		return m_pkMobData->m_table.dwVnum;

	return m_points.job;
}

uint16_t CHARACTER::GetPlayerRace() const
{
	return m_points.job;
}

void CHARACTER::SetRace(uint8_t race)
{
	if (race >= MAIN_RACE_MAX_NUM)
	{
		sys_err("CHARACTER::SetRace(name=%s, race=%d).OUT_OF_RACE_RANGE", GetName(), race);
		return;
	}

	m_points.job = race;
}

uint8_t CHARACTER::GetJob() const
{
	const unsigned race = m_points.job;
	unsigned job;

	if (RaceToJob(race, &job))
		return job;

	sys_err("CHARACTER::GetJob(name=%s, race=%d).OUT_OF_RACE_RANGE", GetName(), race);
	return JOB_WARRIOR;
}

void CHARACTER::SetLevel(uint8_t level)
{
	m_points.level = level;

	if (IsPC())
	{
		if (level < PK_PROTECT_LEVEL)
			SetPKMode(PK_MODE_PROTECT);
		else if (GetGMLevel() != GM_PLAYER)
			SetPKMode(PK_MODE_PROTECT);
#ifdef ENABLE_VIP_SYSTEM
		else if (GetGMLevel() != GM_VIP)
			SetPKMode(PK_MODE_PROTECT);
#endif
		else if (m_bPKMode == PK_MODE_PROTECT)
			SetPKMode(PK_MODE_PEACE);

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		CAchievementSystem::Instance().OnCharacterUpdate(this);
#endif
	}
}

void CHARACTER::SetEmpire(uint8_t bEmpire) noexcept
{
	if (bEmpire < 0 || bEmpire > 3)
	{
		sys_err("invalid empire %d for race %d", bEmpire, GetRaceNum());
		return;
	}

	m_bEmpire = bEmpire;
}

#if defined(ENABLE_PREMIUM_PRIVATE_SHOP) && defined(ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL)
void CHARACTER::SyncPrivateShopTailInfo()
{
	// just for prevent
	if (!IsPC())
	{
		sys_err("invalid pc sync");
		return;
	}

	if (!CShop::CanOpenShopHere(GetMapIndex()))
		return;

	if (m_dwLastSyncPrivateShopInfo > get_global_time())
		return;

	LPCHARACTER myShopChar = CHARACTER_MANAGER::Instance().FindPCShopCharacterByPID(GetPlayerID());
	if (myShopChar)
	{
		bool bUpdatePacket = false;

		if (GetLevel() != myShopChar->GetLevel())
		{
			myShopChar->SetLevel(GetLevel());
			bUpdatePacket = true;
		}

		if (GetConquerorLevel() != myShopChar->GetConquerorLevel())
		{
			myShopChar->SetConquerorLevel(GetConquerorLevel());
			bUpdatePacket = true;
		}

		if (GetAlignment() != myShopChar->GetAlignment())
		{
			myShopChar->SetAlignment(GetAlignment());
			bUpdatePacket = true;
		}

		if (GetEmpire() != myShopChar->GetEmpire())
		{
			myShopChar->SetEmpire(GetEmpire());
			bUpdatePacket = true;
		}

		if (bUpdatePacket)
			myShopChar->UpdatePacket();

		m_dwLastSyncPrivateShopInfo = get_global_time() + (60 * 3);
	}
}
#endif

void CHARACTER::SetPlayerProto(const TPlayerTable* t)
{
	if (!GetDesc() || !*GetDesc()->GetHostName())
		sys_err("cannot get desc or hostname");
	else
		SetGMLevel();

	//m_PlayerSlots = std::make_unique<PlayerSlotT>(); //@fixme199
	m_bCharType = CHAR_TYPE_PC;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	SetPlayerID(t->id);
#else
	m_dwPlayerID = t->id;
#endif

	m_iAlignment = t->lAlignment;
	m_iRealAlignment = t->lAlignment;

	m_points.voice = t->voice;

	m_points.skill_group = t->skill_group;

	m_pointsInstant.bBasePart = t->part_base;
	SetPart(PART_HAIR, t->parts[PART_HAIR]);
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	SetPart(PART_ACCE, GetOriginalPart(PART_ACCE));
#endif
#ifdef ENABLE_AURA_SYSTEM
	SetPart(PART_AURA, t->parts[PART_AURA]);
#endif

	m_points.iRandomHP = t->sRandomHP;
	m_points.iRandomSP = t->sRandomSP;

#ifdef ENABLE_HORSE_APPEARANCE
	m_points.horse_appearance = t->sHorse_appearance;
#endif

	// REMOVE_REAL_SKILL_LEVLES
	if (m_pSkillLevels)
		M2_DELETE_ARRAY(m_pSkillLevels);

	m_pSkillLevels = M2_NEW TPlayerSkill[SKILL_MAX_NUM];
	thecore_memcpy(m_pSkillLevels, t->skills, sizeof(TPlayerSkill) * SKILL_MAX_NUM);
	// END_OF_REMOVE_REAL_SKILL_LEVLES

#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_REWARD_TABLE)
	if (m_ziRewardTable)
		M2_DELETE_ARRAY(m_ziRewardTable);

	m_ziRewardTable = M2_NEW TZiRewardTable[2];
	thecore_memcpy(m_ziRewardTable, t->ziRewardTable, sizeof(TZiRewardTable) * 2);
#endif

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	if (m_gemItems)
		M2_DELETE_ARRAY(m_gemItems);

	m_gemItems = M2_NEW TPlayerGemItems[GEM_SHOP_HOST_ITEM_MAX_NUM];
	thecore_memcpy(m_gemItems, t->gemItems, sizeof(TPlayerGemItems) * GEM_SHOP_HOST_ITEM_MAX_NUM);
#endif

#ifdef ENABLE_FISH_EVENT
	if (m_fishSlots)
		M2_DELETE_ARRAY(m_fishSlots);

	m_fishSlots = M2_NEW TPlayerFishEventSlot[FISH_EVENT_SLOTS_NUM];
	thecore_memcpy(m_fishSlots, t->fishSlots, sizeof(TPlayerFishEventSlot) * FISH_EVENT_SLOTS_NUM);

	m_dwFishUseCount = t->fishEventUseCount;
#endif

#ifdef ENABLE_FLOWER_EVENT
	if (m_FlowerEvent)
		M2_DELETE_ARRAY(m_FlowerEvent);

	m_FlowerEvent = M2_NEW TFlowerEvent[PART_FLOWER_MAX_NUM];
	thecore_memcpy(&m_FlowerEvent[0], t->flowerEvent, sizeof(TFlowerEvent) * PART_FLOWER_MAX_NUM);
#endif

	if (t->lMapIndex >= 10000)
	{
		bool bExitPosition = true;
#ifdef ENABLE_12ZI
		if (CZodiacManager::Instance().IsZiStageMapIndex(t->lMapIndex))
			bExitPosition = false;
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
		if (CMeleyLairManager::Instance().IsMeleyMap(t->lMapIndex))
			bExitPosition = false;
#endif
		if (bExitPosition)
		{
			m_posWarp.x = t->lExitX;
			m_posWarp.y = t->lExitY;
			m_lWarpMapIndex = t->lExitMapIndex;
		}
	}

	SetRealPoint(POINT_PLAYTIME, t->playtime);
	m_dwLoginPlayTime = t->playtime;
	SetRealPoint(POINT_ST, t->st);
	SetRealPoint(POINT_HT, t->ht);
	SetRealPoint(POINT_DX, t->dx);
	SetRealPoint(POINT_IQ, t->iq);

	SetPoint(POINT_ST, t->st);
	SetPoint(POINT_HT, t->ht);
	SetPoint(POINT_DX, t->dx);
	SetPoint(POINT_IQ, t->iq);

	SetPoint(POINT_STAT, t->stat_point);
	SetPoint(POINT_SKILL, t->skill_point);
	SetPoint(POINT_SUB_SKILL, t->sub_skill_point);
	SetPoint(POINT_HORSE_SKILL, t->horse_skill_point);

	SetPoint(POINT_STAT_RESET_COUNT, t->stat_reset_count);

	SetPoint(POINT_LEVEL_STEP, t->level_step);
	SetRealPoint(POINT_LEVEL_STEP, t->level_step);

	SetRace(t->job);

	SetLevel(t->level);
	SetExp(t->exp);
	SetGold(t->gold);
#ifdef ENABLE_CHEQUE_SYSTEM
	SetCheque(t->cheque);
#endif

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	SetExtendInvenStage(t->inventory_stage);
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	for (int i = 0; i < 3; ++i) {
		SetExtendSpecialInvenStage(t->special_stage[i], i);
	}
# endif
#endif
#ifdef ENABLE_BATTLE_FIELD
	SetBattlePoint(t->battle_point);
	SetShopExUsablePoint(SHOP_COIN_TYPE_BATTLE_POINT, t->aiShopExUsablePoint[SHOP_COIN_TYPE_BATTLE_POINT]);
	SetShopExDailyTimePoint(SHOP_COIN_TYPE_BATTLE_POINT, t->aiShopExDailyUse[SHOP_COIN_TYPE_BATTLE_POINT]);
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
	SetMedalHonor(t->medal_honor);
	SetShopExUsablePoint(SHOP_COIN_TYPE_MEDAL_OF_HONOR, t->aiShopExUsablePoint[SHOP_COIN_TYPE_MEDAL_OF_HONOR]);
	SetShopExDailyTimePoint(SHOP_COIN_TYPE_MEDAL_OF_HONOR, t->aiShopExDailyUse[SHOP_COIN_TYPE_MEDAL_OF_HONOR]);
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	SetSoulPoint(t->soul);
	SetSoulRePoint(t->soulre);
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
	SetExtBattlePassPremiumID(t->battle_pass_premium_id);
#endif
#ifdef ENABLE_GEM_SYSTEM
	SetGemPoint(t->gem_point);
#endif
#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	SetGemNextRefresh(t->gem_next_refresh);
#endif

#ifdef ENABLE_YOHARA_SYSTEM
	SetRealPoint(POINT_SUNGMA_STR, t->conqueror_st);
	SetRealPoint(POINT_SUNGMA_HP, t->conqueror_ht);
	SetRealPoint(POINT_SUNGMA_MOVE, t->conqueror_mov);
	SetRealPoint(POINT_SUNGMA_IMMUNE, t->conqueror_imu);
	SetPoint(POINT_SUNGMA_STR, t->conqueror_st);
	SetPoint(POINT_SUNGMA_HP, t->conqueror_ht);
	SetPoint(POINT_SUNGMA_MOVE, t->conqueror_mov);
	SetPoint(POINT_SUNGMA_IMMUNE, t->conqueror_imu);
	SetPoint(POINT_CONQUEROR_POINT, t->conqueror_point);
	SetPoint(POINT_CONQUEROR_LEVEL_STEP, t->conqueror_level_step);
	SetRealPoint(POINT_CONQUEROR_LEVEL_STEP, t->conqueror_level_step);
	SetConquerorLevel(t->conqueror_level);
	SetConquerorExp(t->conqueror_exp);
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	SetPageEquipment(t->page_equipment);
	SetPageTotalEquipment(t->unlock_page_equipment);
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
	SetLottoMoney(t->lotto_moneypool);
	SetLottoTotalMoney(t->lotto_totalmoneywin);
#endif

	SetMapIndex(t->lMapIndex);
	SetXYZ(t->x, t->y, t->z);

	ComputePoints();

	SetHP(t->hp);
	SetSP(t->sp);
	SetStamina(t->stamina);

	// Protected mode in GM
#ifndef ENABLE_GM_FLAG_IF_TEST_SERVER
	if (!test_server)
#endif
	{
#ifdef ENABLE_GM_FLAG_FOR_LOW_WIZARD
#ifdef ENABLE_VIP_SYSTEM
		if (GetGMLevel() > GM_VIP)
#else
		if (GetGMLevel() > GM_PLAYER)
#endif
#else
		if (GetGMLevel() > GM_LOW_WIZARD)
#endif
		{
			m_afAffectFlag.Set(AFF_YMIR);
			m_bPKMode = PK_MODE_PROTECT;
		}
#ifdef ENABLE_MONARCH_AFFECT
		else if (IsMonarch())
		{
			m_afAffectFlag.Set(AFF_KING);
		}
#endif
#ifdef ENABLE_VIP_SYSTEM
		else if (GetGMLevel() == GM_VIP)
		{
			m_afAffectFlag.Set(AFF_VIP);
		}
#endif
	}

	if (GetLevel() < PK_PROTECT_LEVEL)
		m_bPKMode = PK_MODE_PROTECT;

#ifdef ENABLE_VIP_SYSTEM
	else if (GetGMLevel() == GM_VIP)
	{
		m_afAffectFlag.Set(AFF_VIP);
	}
#endif

	m_stMobile = t->szMobile;

	SetHorseData(t->horse);

	if (GetHorseLevel() > 0)
		UpdateHorseDataByLogoff(t->logoff_interval);

	thecore_memcpy(m_aiPremiumTimes, t->aiPremiumTimes, sizeof(t->aiPremiumTimes));

	m_dwLogOffInterval = t->logoff_interval;

	sys_log(0, "PLAYER_LOAD: %s PREMIUM %d %d, LOGGOFF_INTERVAL %u PTR: %p", t->name, m_aiPremiumTimes[0], m_aiPremiumTimes[1], t->logoff_interval, this);

	if (GetGMLevel() != GM_PLAYER)
	{
		LogManager::Instance().CharLog(this, GetGMLevel(), "GM_LOGIN", "");
		sys_log(0, "GM_LOGIN(gmlevel=%d, name=%s(%d), pos=(%d, %d)", GetGMLevel(), GetName(), GetPlayerID(), GetX(), GetY());
	}

#ifdef __PET_SYSTEM__
	// NOTE: Once the character is a PC, only have PetSystem. Because of the memory usage rate per European machine, it's a little bit...
	if (m_petSystem)
	{
		m_petSystem->Destroy();
		delete m_petSystem;
	}

	m_petSystem = M2_NEW CPetSystem(this);
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	if (m_GrowthPetSystem)
	{
		m_GrowthPetSystem->Destroy();
		delete m_GrowthPetSystem;
	}

	m_GrowthPetSystem = M2_NEW CGrowthPetSystem(this);
#endif

#ifdef ENABLE_BIOLOG_SYSTEM
	if (m_pkBiologManager)
	{
		sys_err("Biolog manager already exist for owner %u %s", GetPlayerID(), GetName());
		delete m_pkBiologManager;
	}

	m_pkBiologManager = M2_NEW CBiologSystem(this);

	SetBiologMissions(t->m_BiologActualMission);
	SetBiologCollectedItems(t->m_BiologCollectedItems);
	SetBiologCooldownReminder(t->m_BiologCooldownReminder);
	SetBiologCooldown(t->m_BiologCooldown);
#endif
}

EVENTFUNC(kill_ore_load_event)
{
	const char_event_info* info = dynamic_cast<char_event_info*>(event->info);
	if (info == nullptr)
	{
		sys_err("kill_ore_load_even> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;
	if (ch == nullptr) // <Factor>
		return 0;

	ch->m_pkMiningEvent = nullptr;
	M2_DESTROY_CHARACTER(ch);
	return 0;
}

void CHARACTER::SetProto(const CMob* pkMob)
{
	if (m_pkMobInst)
		M2_DELETE(m_pkMobInst);

	m_pkMobData = pkMob;
	m_pkMobInst = M2_NEW CMobInstance;

	m_bPKMode = PK_MODE_FREE;

	const TMobTable* t = &m_pkMobData->m_table;

	m_bCharType = t->bType;

	SetLevel(t->bLevel);
	SetEmpire(t->bEmpire);

	SetExp(t->dwExp);
#ifdef ENABLE_YOHARA_SYSTEM
	SetConquerorExp(t->dwSungMaExp);
#endif
	SetRealPoint(POINT_ST, t->bStr);
	SetRealPoint(POINT_DX, t->bDex);
	SetRealPoint(POINT_HT, t->bCon);
	SetRealPoint(POINT_IQ, t->bInt);
#ifdef ENABLE_PROTO_RENEWAL
	SetRealPoint(POINT_SUNGMA_HP, t->bSungMaHt);
	SetRealPoint(POINT_SUNGMA_STR, t->bSungMaSt);
	SetRealPoint(POINT_SUNGMA_MOVE, t->bSungMaDx);
	SetRealPoint(POINT_SUNGMA_IMMUNE, t->bSungMaIq);
#endif

	ComputePoints();

	SetHP(GetMaxHP());
	SetSP(GetMaxSP());

	////////////////////
	m_pointsInstant.dwAIFlag = t->dwAIFlag;
	SetImmuneFlag(t->dwImmuneFlag);

	AssignTriggers(t);

	ApplyMobAttribute(t);

	if (IsStone())
	{
#ifdef ENABLE_YOHARA_SYSTEM
		if ((GetRaceNum() >= 8062 && GetRaceNum() <= 8067) || GetRaceNum() == 8076)
		{
			DetermineDropSungmaStone();
		}
		else
#endif
		{
			DetermineDropMetinStone();
		}
	}

	if (IsWarp() || IsGoto()
#ifdef ENABLE_DEFENSE_WAVE
		|| IsDefenseWavePortal()
#endif
		)
	{
		StartWarpNPCEvent();
	}

	CHARACTER_MANAGER::Instance().RegisterRaceNumMap(this);

	// XXX X-mas santa hardcoding
	if (GetRaceNum() == xmas::MOB_SANTA_VNUM)
	{
		SetPoint(POINT_ATT_GRADE_BONUS, 10);
		SetPoint(POINT_DEF_GRADE_BONUS, 6);

		// for santa
		// m_dwPlayStartTime = get_dword_time() + 10 * 60 * 1000;
		// freshman anger
		m_dwPlayStartTime = get_dword_time() + 30 * 1000;
		if (test_server)
			m_dwPlayStartTime = get_dword_time() + 30 * 1000;
	}

	// XXX CTF GuildWar hardcoding
	if (warmap::IsWarFlag(GetRaceNum()))
	{
		m_stateIdle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlag, &CHARACTER::EndStateEmpty);
		m_stateMove.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlag, &CHARACTER::EndStateEmpty);
		m_stateBattle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlag, &CHARACTER::EndStateEmpty);
	}

	if (warmap::IsWarFlagBase(GetRaceNum()))
	{
		m_stateIdle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlagBase, &CHARACTER::EndStateEmpty);
		m_stateMove.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlagBase, &CHARACTER::EndStateEmpty);
		m_stateBattle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlagBase, &CHARACTER::EndStateEmpty);
	}

#ifdef ENABLE_PROTO_RENEWAL
	if (m_bCharType == CHAR_TYPE_HORSE)
#else
	if (m_bCharType == CHAR_TYPE_HORSE ||
		GetRaceNum() == 20101 ||
		GetRaceNum() == 20102 ||
		GetRaceNum() == 20103 ||
		GetRaceNum() == 20104 ||
		GetRaceNum() == 20105 ||
		GetRaceNum() == 20106 ||
		GetRaceNum() == 20107 ||
		GetRaceNum() == 20108 ||
		GetRaceNum() == 20109)
#endif
	{
		m_stateIdle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateHorse, &CHARACTER::EndStateEmpty);
		m_stateMove.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateMove, &CHARACTER::EndStateEmpty);
		m_stateBattle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateHorse, &CHARACTER::EndStateEmpty);
	}

	// MINING
	if (mining::IsVeinOfOre(GetRaceNum()))
	{
		char_event_info* info = AllocEventInfo<char_event_info>();
		info->ch = this;
		m_pkMiningEvent = event_create(kill_ore_load_event, info, PASSES_PER_SEC(15 * 60));
	}
	// END_OF_MINING

#ifdef ENABLE_PORTAL_NAMES
	if (IsGoto() || IsWarp())
		SetName(GetProtoName());
#endif
}

const TMobTable& CHARACTER::GetMobTable() const noexcept
{
	return m_pkMobData->m_table;
}

bool CHARACTER::IsRaceFlag(uint32_t dwBit) const noexcept
{
	return m_pkMobData ? IS_SET(m_pkMobData->m_table.dwRaceFlag, dwBit) : 0;
}

uint32_t CHARACTER::GetMobDamageMin() const noexcept
{
	return m_pkMobData->m_table.dwDamageRange[0];
}

uint32_t CHARACTER::GetMobDamageMax() const noexcept
{
	return m_pkMobData->m_table.dwDamageRange[1];
}

float CHARACTER::GetMobDamageMultiply() const noexcept
{
	float fDamMultiply = GetMobTable().fDamMultiply;
	if (IsBerserk())
		fDamMultiply = fDamMultiply * 2.0f; // BALANCE: Doubles for Berserk

	return fDamMultiply;
}

uint32_t CHARACTER::GetMobDropItemVnum() const noexcept
{
	if (m_pkMobData == nullptr)
		return 0;

	return m_pkMobData->m_table.dwDropItemVnum;
}

uint8_t CHARACTER::GetMobLevel() const
{
	if (!m_pkMobData)
		return 0;

	return m_pkMobData->m_table.bLevel;
}

#ifdef ENABLE_MOB_SCALE
uint8_t CHARACTER::GetMobScale() const
{
	if (!m_pkMobData)
		return 100;

	return m_pkMobData->m_table.bScale;
}
#endif

#if defined(ENABLE_PROTO_RENEWAL) && defined(ENABLE_ELEMENT_ADD)
uint8_t CHARACTER::GetMobAttElement(uint8_t bElement) const noexcept
{
	if (m_pkMobData == nullptr)
		return 0;

	if ((bElement < MOB_ELEMENTAL_ELEC) || (bElement >= MOB_ELEMENTAL_MAX_NUM))
		return 0;

	switch (bElement)
	{
		case MOB_ELEMENTAL_ELEC:
			return m_pkMobData->m_table.cElementalFlags[MOB_ELEMENTAL_ELEC];
		case MOB_ELEMENTAL_FIRE:
			return m_pkMobData->m_table.cElementalFlags[MOB_ELEMENTAL_FIRE];
		case MOB_ELEMENTAL_ICE:
			return m_pkMobData->m_table.cElementalFlags[MOB_ELEMENTAL_ICE];
		case MOB_ELEMENTAL_WIND:
			return m_pkMobData->m_table.cElementalFlags[MOB_ELEMENTAL_WIND];
		case MOB_ELEMENTAL_EARTH:
			return m_pkMobData->m_table.cElementalFlags[MOB_ELEMENTAL_EARTH];
		case MOB_ELEMENTAL_DARK:
			return m_pkMobData->m_table.cElementalFlags[MOB_ELEMENTAL_DARK];
		default:
			break;
	}

	return 0;
}
#endif

bool CHARACTER::IsSummonMonster() const noexcept
{
	return GetSummonVnum() != 0;
}

//@custom014
bool CHARACTER::CanSummonMonster() const
{
	if(!IsSummonMonster())
		return false;

	return (thecore_pulse() > m_lastSummonTime + PASSES_PER_SEC(m_newSummonInterval));
}

void CHARACTER::MarkSummonedMonster()
{
	m_lastSummonTime = thecore_pulse();
	m_newSummonInterval = number(g_npcGroupRespawnRange[0], g_npcGroupRespawnRange[1]);
}
//@end_custom014

uint32_t CHARACTER::GetSummonVnum() const noexcept
{
	return m_pkMobData ? m_pkMobData->m_table.dwSummonVnum : 0;
}

uint32_t CHARACTER::GetPolymorphItemVnum() const noexcept
{
	return m_pkMobData ? m_pkMobData->m_table.dwPolymorphItemVnum : 0;
}

uint32_t CHARACTER::GetMonsterDrainSPPoint() const noexcept
{
	return m_pkMobData ? m_pkMobData->m_table.dwDrainSP : 0;
}

uint8_t CHARACTER::GetMobRank() const noexcept
{
	if (!m_pkMobData)
		return MOB_RANK_KNIGHT; // For PC, KNIGHT level

	return m_pkMobData->m_table.bRank;
}

#ifdef ENABLE_NEWSTUFF
uint8_t CHARACTER::GetMobType() const noexcept
{
	if (!m_pkMobData)
		return CHAR_TYPE_MONSTER;

	return m_pkMobData->m_table.bType;
}
#endif

uint8_t CHARACTER::GetMobSize() const noexcept
{
	if (!m_pkMobData)
		return MOBSIZE_MEDIUM;

	return m_pkMobData->m_table.bSize;
}

uint16_t CHARACTER::GetMobAttackRange() const
{
	auto fHitRange = 0;
#ifdef ENABLE_NEW_DISTANCE_CALC
	fHitRange = m_pkMobData ? m_pkMobData->m_table.fHitRange : 0;
#endif

	switch (GetMobBattleType())
	{
		case BATTLE_TYPE_RANGE:
		case BATTLE_TYPE_MAGIC:
		{
#ifdef ENABLE_DEFENSE_WAVE
			if (IsHydraBoss())
			{
				return m_pkMobData->m_table.wAttackRange + GetPoint(POINT_BOW_DISTANCE) + fHitRange + 4000;
			}
			else
#endif
			{
				return m_pkMobData->m_table.wAttackRange + GetPoint(POINT_BOW_DISTANCE) + fHitRange;
			}
		}
		break;

		default:
#ifdef ENABLE_DEFENSE_WAVE
			if (IsDefenseWaveMobs())
				return m_pkMobData->m_table.wAttackRange + fHitRange + 300;
			else
#endif
				return m_pkMobData->m_table.wAttackRange + fHitRange;
	}
}

#if defined(ENABLE_PROTO_RENEWAL) && defined(ENABLE_NEW_DISTANCE_CALC)
float CHARACTER::GetMonsterHitRange() const
{
	if (!m_pkMobData)
		return 70.0f;

	if (m_pkMobData->m_table.fHitRange)
		return m_pkMobData->m_table.fHitRange;

	return 100.0f;
}
#endif

uint8_t CHARACTER::GetMobBattleType() const noexcept
{
	if (!m_pkMobData)
		return BATTLE_TYPE_MELEE;

	return (m_pkMobData->m_table.bBattleType);
}

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
void CHARACTER::ComputeBattlePoints(bool equip_new)
#else
void CHARACTER::ComputeBattlePoints()
#endif
{
	if (IsPolymorphed())
	{
		const auto dwMobVnum = GetPolymorphVnum();
		const CMob* pMob = CMobManager::Instance().Get(dwMobVnum);
		int iAtt = 0;
		int iDef = 0;

		if (pMob)
		{
			iAtt = GetLevel() * 2 + GetPolymorphPoint(POINT_ST) * 2;
			// lev + con
			iDef = GetLevel() + GetPolymorphPoint(POINT_HT) + pMob->m_table.wDef;
		}

		SetPoint(POINT_ATT_GRADE, iAtt);
		SetPoint(POINT_DEF_GRADE, iDef);
		SetPoint(POINT_MAGIC_ATT_GRADE, GetPoint(POINT_ATT_GRADE));
		SetPoint(POINT_MAGIC_DEF_GRADE, GetPoint(POINT_DEF_GRADE));
	}
	else if (IsPC())
	{
		SetPoint(POINT_ATT_GRADE, 0);
		SetPoint(POINT_DEF_GRADE, 0);
		SetPoint(POINT_CLIENT_DEF_GRADE, 0);
		SetPoint(POINT_MAGIC_ATT_GRADE, GetPoint(POINT_ATT_GRADE));
		SetPoint(POINT_MAGIC_DEF_GRADE, GetPoint(POINT_DEF_GRADE));

		//
		// Basic ATK = 2lev + 2str, 2str can be changed per class
		//
		int iAtk = GetLevel() * 2;
		int iStatAtk = 0;

		switch (GetJob())
		{
			case JOB_WARRIOR:
			case JOB_SURA:
				iStatAtk = (2 * GetPoint(POINT_ST));
				break;

			case JOB_ASSASSIN:
				iStatAtk = (4 * GetPoint(POINT_ST) + 2 * GetPoint(POINT_DX)) / 3;
				break;

			case JOB_SHAMAN:
				iStatAtk = (4 * GetPoint(POINT_ST) + 2 * GetPoint(POINT_IQ)) / 3;
				break;
#ifdef ENABLE_WOLFMAN_CHARACTER
			case JOB_WOLFMAN:
				// TODO: Request from the official planner of the attack on the beasts
				iStatAtk = (2 * GetPoint(POINT_ST));
				break;
#endif
			default:
				sys_err("invalid job %d", GetJob());
				iStatAtk = (2 * GetPoint(POINT_ST));
				break;
		}

		// If you are riding a horse and the attack power due to stats is lower than ST*2, set it to ST*2.
		// In order not to lower the attack power of the person who took the wrong stat.
		if (GetMountVnum() && iStatAtk < 2 * GetPoint(POINT_ST))
			iStatAtk = (2 * GetPoint(POINT_ST));

		iAtk += iStatAtk;

		// Horseback Riding (Horse): Reduce damage due to swordsmanship
		if (GetMountVnum())
		{
			if (GetJob() == JOB_SURA && GetSkillGroup() == 1)
			{
				iAtk += (iAtk * GetHorseLevel()) / 60;
			}
			else
			{
				iAtk += (iAtk * GetHorseLevel()) / 30;
			}
		}

		//
		// ATK Setting
		//
		iAtk += GetPoint(POINT_ATT_GRADE_BONUS);

		PointChange(POINT_ATT_GRADE, iAtk);

		// DEF = LEV + CON + ARMOR
		const auto iShowDef = GetLevel() + GetPoint(POINT_HT); // For Ymir(celestial horse)
		const auto iDef = GetLevel() + static_cast<int>(GetPoint(POINT_HT) / 1.25); // For Other
		int iArmor = 0;

		LPITEM pkItem = nullptr;

		for (int i = 0; i < WEAR_MAX_NUM; ++i)
		{
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
			if ((pkItem = GetWear(i)) && pkItem->GetType() == ITEM_ARMOR && equip_new)
#else
			if ((pkItem = GetWear(i)) && pkItem->GetType() == ITEM_ARMOR)
#endif
			{
				if (pkItem->GetSubType() == ARMOR_BODY || pkItem->GetSubType() == ARMOR_HEAD || pkItem->GetSubType() == ARMOR_FOOTS || pkItem->GetSubType() == ARMOR_SHIELD)
				{
					int iArmorVal = 0;
					iArmorVal = pkItem->GetValue(1);
#ifdef ENABLE_YOHARA_SYSTEM
					if (pkItem->ItemHasRandomDefaultAttr())
						iArmorVal = pkItem->GetRandomDefaultAttr(0);
#endif
					iArmor += iArmorVal;
					iArmor += (2 * pkItem->GetValue(5));
				}
			}
#ifdef ENABLE_AURA_SYSTEM
			else if (pkItem && pkItem->GetType() == ITEM_COSTUME && pkItem->GetSubType() == COSTUME_AURA)
			{
				const auto c_lLevelSocket = pkItem->GetSocket(ITEM_SOCKET_AURA_CURRENT_LEVEL);
				const auto c_lDrainSocket = pkItem->GetSocket(ITEM_SOCKET_AURA_DRAIN_ITEM_VNUM);
#	ifdef ENABLE_AURA_BOOST
				const auto c_lBoostSocket = pkItem->GetSocket(ITEM_SOCKET_AURA_BOOST);
#	endif

				const auto bCurLevel = (c_lLevelSocket / 100000) - 1000;
#	ifdef ENABLE_AURA_BOOST
				const auto bBoostIndex = c_lBoostSocket / 100000000;
#	endif

#	ifdef ENABLE_AURA_BOOST
				const TItemTable* pBoosterProto = ITEM_MANAGER::Instance().GetTable(ITEM_AURA_BOOST_ITEM_VNUM_BASE + bBoostIndex);
#	endif
				float fAuraDrainPer = (1.0f * bCurLevel / 10.0f) / 100.0f;
#	ifdef ENABLE_AURA_BOOST
				if (pBoosterProto)
					fAuraDrainPer += 1.0f * pBoosterProto->alValues[ITEM_AURA_BOOST_PERCENT_VALUE] / 100.0f;
#	endif

				TItemTable* pDrainedItem = nullptr;
				if (c_lDrainSocket != 0)
					pDrainedItem = ITEM_MANAGER::Instance().GetTable(c_lDrainSocket);
				if (pDrainedItem != nullptr && pDrainedItem->bType == ITEM_ARMOR && pDrainedItem->bSubType == ARMOR_SHIELD)
				{
					const float fValue = (pDrainedItem->alValues[1] + (2 * pDrainedItem->alValues[5])) * fAuraDrainPer;
					iArmor += static_cast<int>((fValue < 1.0f) ? ceilf(fValue) : truncf(fValue));
				}
			}
#endif
		}

		// If the DEF is lower than the standard DEF of the horse while riding a horse, it is set as the standard DEF.
		if (IsHorseRiding())
		{
			if (iArmor < GetHorseArmor())
				iArmor = GetHorseArmor();

			const char* pHorseName = CHorseNameManager::Instance().GetHorseName(GetPlayerID());

			if (pHorseName != nullptr && strlen(pHorseName))
			{
				iArmor += 20;
			}
		}

		iArmor += GetPoint(POINT_DEF_GRADE_BONUS);
		iArmor += GetPoint(POINT_PARTY_DEFENDER_BONUS);

		// INTERNATIONAL_VERSION
		PointChange(POINT_DEF_GRADE, iDef + iArmor);
		PointChange(POINT_CLIENT_DEF_GRADE, (iShowDef + iArmor) - GetPoint(POINT_DEF_GRADE));
		// END_OF_INTERNATIONAL_VERSION

		PointChange(POINT_MAGIC_ATT_GRADE, GetLevel() * 2 + GetPoint(POINT_IQ) * 2 + GetPoint(POINT_MAGIC_ATT_GRADE_BONUS));
		PointChange(POINT_MAGIC_DEF_GRADE, GetLevel() + (GetPoint(POINT_IQ) * 3 + GetPoint(POINT_HT)) / 3 + iArmor / 2 + GetPoint(POINT_MAGIC_DEF_GRADE_BONUS));
	}
	else
	{
		// 2lev + str * 2
		const auto iAtt = GetLevel() * 2 + GetPoint(POINT_ST) * 2;
		// lev + con
		const auto iDef = GetLevel() + GetPoint(POINT_HT) + GetMobTable().wDef;

		SetPoint(POINT_ATT_GRADE, iAtt);
		SetPoint(POINT_DEF_GRADE, iDef);
		SetPoint(POINT_MAGIC_ATT_GRADE, GetPoint(POINT_ATT_GRADE));
		SetPoint(POINT_MAGIC_DEF_GRADE, GetPoint(POINT_DEF_GRADE));
	}
}

#ifdef ENABLE_YOHARA_SYSTEM
bool CHARACTER::IsSungmaMap()
{
	if (!IsPC())
		return false;

#ifdef ENABLE_QUEEN_NETHIS
	if (SnakeLair::CSnk::instance().IsSnakeMap(GetMapIndex()))
		return true;
#endif
#ifdef ENABLE_WHITE_DRAGON
	if (WhiteDragon::CWhDr::instance().IsWhiteMap(GetMapIndex()))
		return true;
#endif
#ifdef ENABLE_SUNG_MAHI_TOWER
	if (IsSungMahiDungeon(GetMapIndex()))
		return true;
#endif

	const TSungmaTable& r = g_map_SungmaTable[GetMapIndex()];
	if (r.lMapIdx == GetMapIndex())
		return true;

	return false;
}

uint8_t CHARACTER::GetSungmaMapAttribute(uint16_t wPoint)
{
	uint8_t sungma_points = 0;

	if (!IsPC() || !wPoint)
		return sungma_points;

	const TSungmaTable& r = g_map_SungmaTable[GetMapIndex()];
#ifdef ENABLE_QUEEN_NETHIS
	if (!SnakeLair::CSnk::instance().IsSnakeMap(GetMapIndex()))
#endif
#ifdef ENABLE_WHITE_DRAGON
	if (!WhiteDragon::CWhDr::instance().IsWhiteMap(GetMapIndex()))
#endif
#ifdef ENABLE_SUNG_MAHI_TOWER
	if (!IsSungMahiDungeon(GetMapIndex()))
#endif
	{
		if (r.lMapIdx == 0)
			return sungma_points;
	}

	switch (wPoint)
	{
		case POINT_SUNGMA_STR:
		{
			sungma_points = r.bSungmaStr;
#ifdef ENABLE_QUEEN_NETHIS
			if (SnakeLair::CSnk::instance().IsSnakeMap(GetMapIndex()))
				sungma_points = SnakeLair::CSnk::instance().GetSungmaQueenDungeonValue(this, POINT_SUNGMA_STR);
#endif
#ifdef ENABLE_WHITE_DRAGON
			if (WhiteDragon::CWhDr::instance().IsWhiteMap(GetMapIndex()))
				sungma_points = WhiteDragon::CWhDr::instance().GetSungmaWhiteDungeonValue(this, POINT_SUNGMA_STR);
#endif
#ifdef ENABLE_SUNG_MAHI_TOWER
			if (IsSungMahiDungeon(GetMapIndex()))
				sungma_points = GetSungMahiTowerDungeonValue(0/*POINT_SUNGMA_STR*/);
#endif
		}
		break;

		case POINT_SUNGMA_HP:
		{
			sungma_points = r.bSungmaHp;
#ifdef ENABLE_QUEEN_NETHIS
			if (SnakeLair::CSnk::instance().IsSnakeMap(GetMapIndex()))
				sungma_points = SnakeLair::CSnk::instance().GetSungmaQueenDungeonValue(this, POINT_SUNGMA_HP);
#endif
#ifdef ENABLE_WHITE_DRAGON
			if (WhiteDragon::CWhDr::instance().IsWhiteMap(GetMapIndex()))
				sungma_points = WhiteDragon::CWhDr::instance().GetSungmaWhiteDungeonValue(this, POINT_SUNGMA_HP);
#endif
#ifdef ENABLE_SUNG_MAHI_TOWER
			if (IsSungMahiDungeon(GetMapIndex()))
				sungma_points = GetSungMahiTowerDungeonValue(1/*POINT_SUNGMA_HP*/);
#endif
		}
		break;

		case POINT_SUNGMA_MOVE:
		{
			sungma_points = r.bSungmaMove;
#ifdef ENABLE_QUEEN_NETHIS
			if (SnakeLair::CSnk::instance().IsSnakeMap(GetMapIndex()))
				sungma_points = SnakeLair::CSnk::instance().GetSungmaQueenDungeonValue(this, POINT_SUNGMA_MOVE);
#endif
#ifdef ENABLE_WHITE_DRAGON
			if (WhiteDragon::CWhDr::instance().IsWhiteMap(GetMapIndex()))
				sungma_points = WhiteDragon::CWhDr::instance().GetSungmaWhiteDungeonValue(this, POINT_SUNGMA_MOVE);
#endif
#ifdef ENABLE_SUNG_MAHI_TOWER
			if (IsSungMahiDungeon(GetMapIndex()))
				sungma_points = GetSungMahiTowerDungeonValue(2/*POINT_SUNGMA_MOVE*/);
#endif
		}
		break;

		case POINT_SUNGMA_IMMUNE:
		{
			sungma_points = r.bSungmaImmune;
#ifdef ENABLE_QUEEN_NETHIS
			if (SnakeLair::CSnk::instance().IsSnakeMap(GetMapIndex()))
				sungma_points = SnakeLair::CSnk::instance().GetSungmaQueenDungeonValue(this, POINT_SUNGMA_IMMUNE);
#endif
#ifdef ENABLE_WHITE_DRAGON
			if (WhiteDragon::CWhDr::instance().IsWhiteMap(GetMapIndex()))
				sungma_points = WhiteDragon::CWhDr::instance().GetSungmaWhiteDungeonValue(this, POINT_SUNGMA_IMMUNE);
#endif
#ifdef ENABLE_SUNG_MAHI_TOWER
			if (IsSungMahiDungeon(GetMapIndex()))
				sungma_points = GetSungMahiTowerDungeonValue(3/*POINT_SUNGMA_IMMUNE*/);
#endif
		}
		break;

		case POINT_HIT_PCT:
		{
			sungma_points = r.bHitPct;
#ifdef ENABLE_QUEEN_NETHIS
			if (SnakeLair::CSnk::instance().IsSnakeMap(GetMapIndex()))
				sungma_points = SnakeLair::CSnk::instance().GetSungmaQueenDungeonValue(this, POINT_HIT_PCT);
#endif
#ifdef ENABLE_WHITE_DRAGON
			if (WhiteDragon::CWhDr::instance().IsWhiteMap(GetMapIndex()))
				sungma_points = WhiteDragon::CWhDr::instance().GetSungmaWhiteDungeonValue(this, POINT_HIT_PCT);
#endif
#ifdef ENABLE_SUNG_MAHI_TOWER
			if (IsSungMahiDungeon(GetMapIndex()))
				sungma_points = 0;
#endif
		}
		break;

		default:
			return sungma_points;
			break;
	}

	return sungma_points;
}
#endif

void CHARACTER::ComputePoints()
{
	const auto lStat = GetPoint(POINT_STAT);
	const auto lStatResetCount = GetPoint(POINT_STAT_RESET_COUNT);
	const auto lSkillActive = GetPoint(POINT_SKILL);
	const auto lSkillSub = GetPoint(POINT_SUB_SKILL);
	const auto lSkillHorse = GetPoint(POINT_HORSE_SKILL);
	const auto lLevelStep = GetPoint(POINT_LEVEL_STEP);

	const auto lAttackerBonus = GetPoint(POINT_PARTY_ATTACKER_BONUS);
	const auto lTankerBonus = GetPoint(POINT_PARTY_TANKER_BONUS);
	const auto lBufferBonus = GetPoint(POINT_PARTY_BUFFER_BONUS);
	const auto lSkillMasterBonus = GetPoint(POINT_PARTY_SKILL_MASTER_BONUS);
	const auto lHasteBonus = GetPoint(POINT_PARTY_HASTE_BONUS);
	const auto lDefenderBonus = GetPoint(POINT_PARTY_DEFENDER_BONUS);

	const auto lHPRecovery = GetPoint(POINT_HP_RECOVERY);
	const auto lSPRecovery = GetPoint(POINT_SP_RECOVERY);

#ifdef ENABLE_YOHARA_SYSTEM
	const auto lConquerorPoint = GetPoint(POINT_CONQUEROR_POINT);
	const auto lConquerorLevelStep = GetPoint(POINT_CONQUEROR_LEVEL_STEP);
#endif

#ifdef ENABLE_CHEQUE_SYSTEM
	const auto iCheque = GetPoint(POINT_CHEQUE);
#endif

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	const auto lSoul = GetSoulPoint();
	const auto lSoulRe = GetSoulRePoint();
#endif

	memset(m_pointsInstant.points, 0, sizeof(m_pointsInstant.points));
	BuffOnAttr_ClearAll();
	m_SkillDamageBonus.clear();

	SetPoint(POINT_STAT, lStat);
	SetPoint(POINT_SKILL, lSkillActive);
	SetPoint(POINT_SUB_SKILL, lSkillSub);
	SetPoint(POINT_HORSE_SKILL, lSkillHorse);
	SetPoint(POINT_LEVEL_STEP, lLevelStep);
	SetPoint(POINT_STAT_RESET_COUNT, lStatResetCount);

	SetPoint(POINT_ST, GetRealPoint(POINT_ST));
	SetPoint(POINT_HT, GetRealPoint(POINT_HT));
	SetPoint(POINT_DX, GetRealPoint(POINT_DX));
	SetPoint(POINT_IQ, GetRealPoint(POINT_IQ));

	SetPart(PART_MAIN, GetOriginalPart(PART_MAIN));
	SetPart(PART_WEAPON, GetOriginalPart(PART_WEAPON));
	SetPart(PART_HEAD, GetOriginalPart(PART_HEAD));
	SetPart(PART_HAIR, GetOriginalPart(PART_HAIR));
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	SetPart(PART_ACCE, GetOriginalPart(PART_ACCE));
#endif
#ifdef ENABLE_AURA_SYSTEM
	SetPart(PART_AURA, GetOriginalPart(PART_AURA));
#endif

	SetPoint(POINT_PARTY_ATTACKER_BONUS, lAttackerBonus);
	SetPoint(POINT_PARTY_TANKER_BONUS, lTankerBonus);
	SetPoint(POINT_PARTY_BUFFER_BONUS, lBufferBonus);
	SetPoint(POINT_PARTY_SKILL_MASTER_BONUS, lSkillMasterBonus);
	SetPoint(POINT_PARTY_HASTE_BONUS, lHasteBonus);
	SetPoint(POINT_PARTY_DEFENDER_BONUS, lDefenderBonus);

	SetPoint(POINT_HP_RECOVERY, lHPRecovery);
	SetPoint(POINT_SP_RECOVERY, lSPRecovery);

#ifdef ENABLE_YOHARA_SYSTEM
	SetPoint(POINT_CONQUEROR_POINT, lConquerorPoint);
	SetPoint(POINT_CONQUEROR_LEVEL_STEP, lConquerorLevelStep);

	SetPoint(POINT_SUNGMA_STR, GetRealPoint(POINT_SUNGMA_STR));
	SetPoint(POINT_SUNGMA_HP, GetRealPoint(POINT_SUNGMA_HP));
	SetPoint(POINT_SUNGMA_MOVE, GetRealPoint(POINT_SUNGMA_MOVE));
	SetPoint(POINT_SUNGMA_IMMUNE, GetRealPoint(POINT_SUNGMA_IMMUNE));
#endif

	// PC_BANG_ITEM_ADD
	SetPoint(POINT_PC_BANG_EXP_BONUS, 0);
	SetPoint(POINT_PC_BANG_DROP_BONUS, 0);
	// END_PC_BANG_ITEM_ADD

#ifdef ENABLE_CHEQUE_SYSTEM
	SetPoint(POINT_CHEQUE, iCheque);
#endif

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	SetPoint(POINT_SOUL, lSoul);
	SetPoint(POINT_SOUL_RE, lSoulRe);
#endif

	int iMaxHP = 0, iMaxSP = 0;
	int iMaxStamina = 0;

	if (IsPC())
	{
		// Max Life/Spirit
		iMaxHP = JobInitialPoints[GetJob()].max_hp + m_points.iRandomHP + GetPoint(POINT_HT) * JobInitialPoints[GetJob()].hp_per_ht;
		iMaxSP = JobInitialPoints[GetJob()].max_sp + m_points.iRandomSP + GetPoint(POINT_IQ) * JobInitialPoints[GetJob()].sp_per_iq;
		iMaxStamina = JobInitialPoints[GetJob()].max_stamina + GetPoint(POINT_HT) * JobInitialPoints[GetJob()].stamina_per_con;

		{
			CSkillProto* pkSk = CSkillManager::Instance().Get(SKILL_ADD_HP);

			if (nullptr != pkSk)
			{
				pkSk->SetPointVar("k", 1.0f * GetSkillPower(SKILL_ADD_HP) / 100.0f);

				iMaxHP += static_cast<int>(pkSk->kPointPoly.Eval());
			}
		}

		// default values
		SetPoint(POINT_MOV_SPEED, PC_MOVE_SPEED);
		SetPoint(POINT_ATT_SPEED, PC_ATT_SPEED);
		PointChange(POINT_ATT_SPEED, GetPoint(POINT_PARTY_HASTE_BONUS));
		SetPoint(POINT_CASTING_SPEED,	PC_CAST_SPEED);
	}
	else
	{
		iMaxHP = m_pkMobData->m_table.dwMaxHP;
		iMaxSP = 0;
		iMaxStamina = 0;

		SetPoint(POINT_ATT_SPEED, m_pkMobData->m_table.sAttackSpeed);
		SetPoint(POINT_MOV_SPEED, m_pkMobData->m_table.sMovingSpeed);
		SetPoint(POINT_CASTING_SPEED, m_pkMobData->m_table.sAttackSpeed);
	}

	if (IsPC())
	{
		// When riding a horse, if the basic stat is lower than the standard stat of the horse, make it higher.
		// Therefore, since the standard stat of the horse is the standard of the warrior, the sum of all stats for Sura/Shaman is
		// It will generally go higher.
		if (GetMountVnum() >= 20101 && GetMountVnum() <= 20109)	//Give Attr just normal horses
		{
			if (GetHorseST() > GetPoint(POINT_ST))
				PointChange(POINT_ST, GetHorseST() - GetPoint(POINT_ST));

			if (GetHorseDX() > GetPoint(POINT_DX))
				PointChange(POINT_DX, GetHorseDX() - GetPoint(POINT_DX));

			if (GetHorseHT() > GetPoint(POINT_HT))
				PointChange(POINT_HT, GetHorseHT() - GetPoint(POINT_HT));

			if (GetHorseIQ() > GetPoint(POINT_IQ))
				PointChange(POINT_IQ, GetHorseIQ() - GetPoint(POINT_IQ));
		}
	}

	ComputeBattlePoints();

	// Default HP/SP Settings
	if (iMaxHP != GetMaxHP())
	{
		SetRealPoint(POINT_MAX_HP, iMaxHP); // Save the default HP in RealPoint.
	}

	PointChange(POINT_MAX_HP, 0);

	if (iMaxSP != GetMaxSP())
	{
		SetRealPoint(POINT_MAX_SP, iMaxSP); // Save the basic SP in RealPoint.
	}

	PointChange(POINT_MAX_SP, 0);

	SetMaxStamina(iMaxStamina);
	// @fixme118 part1
	const auto iCurHP = this->GetHP();
	const auto iCurSP = this->GetSP();

	m_pointsInstant.dwImmuneFlag = 0;

	for (int i = 0; i < WEAR_MAX_NUM; i++)
	{
		LPITEM pItem = GetWear(i);
		if (pItem)
		{
			pItem->ModifyPoints(true);
			SET_BIT(m_pointsInstant.dwImmuneFlag, GetWear(i)->GetImmuneFlag());
		}
	}

	// Dragon Soul Stone System
	// In ComputePoints, initialize all property values of the character,
	// Because all property values related to items, buffs, etc. are recalculated,
	// The Dragon Soul Stone system must also re-apply the attribute values of all Dragon Soul Stones in ActiveDeck.
	if (DragonSoul_IsDeckActivated())
	{
		for (int i = WEAR_MAX_NUM + DS_SLOT_MAX * DragonSoul_GetActiveDeck();
			i < WEAR_MAX_NUM + DS_SLOT_MAX * (DragonSoul_GetActiveDeck() + 1); i++)
		{
			LPITEM pItem = GetWear(i);
			if (pItem)
			{
				if (DSManager::Instance().IsTimeLeftDragonSoul(pItem))
					pItem->ModifyPoints(true);
			}
		}
	}

	if (GetHP() > GetMaxHP())
		PointChange(POINT_HP, GetMaxHP() - GetHP());

	if (GetSP() > GetMaxSP())
		PointChange(POINT_SP, GetMaxSP() - GetSP());

	ComputeSkillPoints();

	RefreshAffect();

#ifndef ENABLE_PET_SYSTEM	//@fixme520
	CPetSystem* pPetSystem = GetPetSystem();
	if (nullptr != pPetSystem)
		pPetSystem->RefreshBuff();
#endif

	// @fixme118 part2 (after petsystem stuff)
	if (IsPC())
	{
		if (this->GetHP() != iCurHP)
			this->PointChange(POINT_HP, iCurHP - this->GetHP());
		if (this->GetSP() != iCurSP)
			this->PointChange(POINT_SP, iCurSP - this->GetSP());
	}

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	if (IsPC())
		CAchievementSystem::Instance().ComputePoints(this);
#endif

	UpdatePacket();
}

// The unit of m_dwPlayStartTime is milisecond. Record in minutes in database
// Because of this, when calculating the play time / divided by 60000, the remainder is left over.
// When , you have to put it as dwTimeRemain here so that it can be calculated properly.
void CHARACTER::ResetPlayTime(uint32_t dwTimeRemain) noexcept
{
	m_dwPlayStartTime = get_dword_time() - dwTimeRemain;
}

const int aiRecoveryPercents[10] = { 1, 5, 5, 5, 5, 5, 5, 5, 5, 5 };

EVENTFUNC(recovery_event)
{
	const char_event_info* info = dynamic_cast<char_event_info*>(event->info);
	if (info == nullptr)
	{
		sys_err("recovery_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == nullptr) // <Factor>
		return 0;

	if (!ch->IsPC())
	{
		//
		// Monster Recovery
		//
		if (ch->IsAffectFlag(AFF_POISON))
			return PASSES_PER_SEC(MAX(1, ch->GetMobTable().bRegenCycle));
#ifdef ENABLE_WOLFMAN_CHARACTER
		if (ch->IsAffectFlag(AFF_BLEEDING))
			return PASSES_PER_SEC(MAX(1, ch->GetMobTable().bRegenCycle));
#endif
		if (2493 == ch->GetMobTable().dwVnum)	//Aqua Dragon
		{
			int regenPct = BlueDragon_GetRangeFactor("hp_regen", ch->GetHPPct());
			regenPct += ch->GetMobTable().bRegenPercent;

			for (int i = 1; i <= 4; ++i)
			{
				if (REGEN_PECT_BONUS == BlueDragon_GetIndexFactor("DragonStone", i, "effect_type"))
				{
					const auto dwDragonStoneID = BlueDragon_GetIndexFactor("DragonStone", i, "vnum");
					const auto val = BlueDragon_GetIndexFactor("DragonStone", i, "val");
					const auto cnt = SECTREE_MANAGER::Instance().GetMonsterCountInMap(ch->GetMapIndex(), dwDragonStoneID);

					regenPct += (val * cnt);

					break;
				}
			}

			ch->PointChange(POINT_HP, MAX(1, (ch->GetMaxHP() * regenPct) / 100));
		}
		else if (!ch->IsDoor())
		{
			ch->MonsterLog("HP_REGEN +%d", MAX(1, (ch->GetMaxHP() * ch->GetMobTable().bRegenPercent) / 100));
			ch->PointChange(POINT_HP, MAX(1, (ch->GetMaxHP() * ch->GetMobTable().bRegenPercent) / 100));
		}

		if (ch->GetHP() >= ch->GetMaxHP())
		{
			ch->m_pkRecoveryEvent = nullptr;
			return 0;
		}

		if (2493 == ch->GetMobTable().dwVnum)
		{
			for (int i = 1; i <= 4; ++i)
			{
				if (REGEN_TIME_BONUS == BlueDragon_GetIndexFactor("DragonStone", i, "effect_type"))
				{
					const auto dwDragonStoneID = BlueDragon_GetIndexFactor("DragonStone", i, "vnum");
					const auto val = BlueDragon_GetIndexFactor("DragonStone", i, "val");
					const auto cnt = SECTREE_MANAGER::Instance().GetMonsterCountInMap(ch->GetMapIndex(), dwDragonStoneID);

					return PASSES_PER_SEC(MAX(1, (ch->GetMobTable().bRegenCycle - (val * cnt))));
				}
			}
		}

		return PASSES_PER_SEC(MAX(1, ch->GetMobTable().bRegenCycle));
	}
	else
	{
		//
		// PC Recovery
		//
		ch->CheckTarget();
		//ch->UpdateSectree();
		ch->UpdateKillerMode();

		if (ch->IsAffectFlag(AFF_POISON) == true)
		{
			// Prohibit automatic recovery in case of poisoning
			// Disable auto-recovery in case of divination
			return 3;
		}
#ifdef ENABLE_WOLFMAN_CHARACTER
		if (ch->IsAffectFlag(AFF_BLEEDING))
			return 3;
#endif

		const auto iSec = (get_dword_time() - ch->GetLastMoveTime()) / 3000;

		// SP recovery routine.
		// Why did you leave this out as a function?!
		ch->DistributeSP(ch);

		if (ch->GetMaxHP() <= ch->GetHP())
			return PASSES_PER_SEC(3);

		int iPercent = 0;
		int iAmount = 0;

		{
			iPercent = aiRecoveryPercents[MIN(9, iSec)];
			iAmount = 15 + (ch->GetMaxHP() * iPercent) / 100;
		}

		iAmount += (iAmount * ch->GetPoint(POINT_HP_REGEN)) / 100;

		sys_log(1, "RECOVERY_EVENT: %s %d HP_REGEN %d HP +%d", ch->GetName(), iPercent, ch->GetPoint(POINT_HP_REGEN), iAmount);

		ch->PointChange(POINT_HP, iAmount, false);
		return PASSES_PER_SEC(3);
	}
}

void CHARACTER::StartRecoveryEvent()
{
	if (m_pkRecoveryEvent)
		return;

	if (IsDead() || IsStun())
		return;

	if (IsNPC() && GetHP() >= GetMaxHP()) // Monsters do not start when their health is full.
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();

	info->ch = this;

	const auto iSec = IsPC() ? 3 : (MAX(1, GetMobTable().bRegenCycle));
	m_pkRecoveryEvent = event_create(recovery_event, info, PASSES_PER_SEC(iSec));
}

void CHARACTER::Standup()
{
	TPacketGCPosition pack_position{};

	if (!IsPosition(POS_SITTING))
		return;

	SetPosition(POS_STANDING);

	sys_log(1, "STANDUP: %s", GetName());

	pack_position.header = HEADER_GC_CHARACTER_POSITION;
	pack_position.vid = GetVID();
	pack_position.position = POSITION_GENERAL;

	PacketAround(&pack_position, sizeof(TPacketGCPosition));
}

void CHARACTER::Sitdown(int is_ground)
{
	TPacketGCPosition pack_position{};

	if (IsPosition(POS_SITTING))
		return;

	SetPosition(POS_SITTING);
	sys_log(1, "SITDOWN: %s", GetName());

	pack_position.header = HEADER_GC_CHARACTER_POSITION;
	pack_position.vid = GetVID();
	pack_position.position = POSITION_SITTING_GROUND;
	PacketAround(&pack_position, sizeof(TPacketGCPosition));
}

void CHARACTER::SetRotation(float fRot) noexcept
{
	m_pointsInstant.fRot = fRot;
}

// Look in the x and y directions and stand.
void CHARACTER::SetRotationToXY(long x, long y)
{
	SetRotation(GetDegreeFromPositionXY(GetX(), GetY(), x, y));
}

bool CHARACTER::CannotMoveByAffect() const
{
	return (IsAffectFlag(AFF_STUN));
}

bool CHARACTER::CanMove() const
{
	if (IsDead()) // prevent dead move hack @fixme431
		return false;

	if (CannotMoveByAffect())
		return false;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (IsShop())
		return false;
#else
	if (GetMyShop()) // Can't move while the shop is open
		return false;
#endif

#ifdef ENABLE_SUNG_MAHI_TOWER
	if (IsNoattack())
		return false;
#endif

	// If it is 0.2 seconds ago, it cannot move.
	/*
	if (get_float_time() - m_fSyncTime < 0.2f)
		return false;
	*/
	return true;
}

// Unconditionally move to x, y position.
bool CHARACTER::Sync(long x, long y)
{
	if (!GetSectree())
		return false;

	if (IsPC() && IsDead()) // prevent dead move hack
		return false;

	LPSECTREE new_tree = SECTREE_MANAGER::Instance().Get(GetMapIndex(), x, y);

	if (!new_tree)
	{
		if (GetDesc())
		{
			sys_err("cannot find tree at %d %d (name: %s)", x, y, GetName());
#ifdef ENABLE_SYNCPOSITION_DISCONNECT_FIX //@fixme466
			new_tree = GetSectree();
			x = GetX();
			y = GetY();
#else
			GetDesc()->SetPhase(PHASE_CLOSE);
#endif
		}
		else
		{
			sys_err("no tree: %s %d %d %d", GetName(), x, y, GetMapIndex());
			SetNoRewardFlag();
			Dead();
		}

		return false;
	}

#ifdef ENABLE_CHECK_MOVESPEED_HACK
	if (IsPC())
	{
		if (GetCShield()->CheckMoveSpeedhack(x, y, get_dword_time(), IsRiding(), GetMoveSpeed()))
		{
			LogManager::instance().HackLog("CSHIELD SPEED_HACK", this);
			Show(GetMapIndex(), GetX(), GetY(), 0);
			Stop();
		}
	}
#endif

	SetRotationToXY(x, y);
	SetXYZ(x, y, 0);

	if (GetDungeon())
	{
		// Dungeon King Event Attribute Change
		const auto iLastEventAttr = m_iEventAttr;
		m_iEventAttr = new_tree->GetEventAttribute(x, y);

		if (m_iEventAttr != iLastEventAttr)
		{
			if (GetParty())
			{
				quest::CQuestManager::Instance().AttrOut(GetParty()->GetLeaderPID(), this, iLastEventAttr);
				quest::CQuestManager::Instance().AttrIn(GetParty()->GetLeaderPID(), this, m_iEventAttr);
			}
			else
			{
				quest::CQuestManager::Instance().AttrOut(GetPlayerID(), this, iLastEventAttr);
				quest::CQuestManager::Instance().AttrIn(GetPlayerID(), this, m_iEventAttr);
			}
		}
	}

	if (GetSectree() != new_tree)
	{
		if (!IsNPC())
		{
			const SECTREEID id = new_tree->GetID();
			const SECTREEID old_id = GetSectree()->GetID();

			const float fDist = DISTANCE_SQRT(id.coord.x - old_id.coord.x, id.coord.y - old_id.coord.y);
			sys_log(0, "SECTREE DIFFER: %s %dx%d was %dx%d dist %.1fm",
				GetName(),
				id.coord.x,
				id.coord.y,
				old_id.coord.x,
				old_id.coord.y,
				fDist);
		}

		new_tree->InsertEntity(this);
	}

	return true;
}

void CHARACTER::Stop()
{
	if (!IsState(m_stateIdle))
		MonsterLog("[IDLE] stop");

	GotoState(m_stateIdle);

	m_posDest.x = m_posStart.x = GetX();
	m_posDest.y = m_posStart.y = GetY();
}

bool CHARACTER::Goto(long x, long y)
{
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	// NOTE: Prevent Dragon Queen Meley from changing position
	// when warping out of the dungeon.
	if (GetRaceNum() == static_cast<uint32_t>(eDefault::MELEY_BOSS_VNUM))
		return false;
#endif

	// TODO distance check required
	// No need to move if same location (auto success)
	if (GetX() == x && GetY() == y)
		return false;

	if (m_posDest.x == x && m_posDest.y == y)
	{
		if (!IsState(m_stateMove))
		{
			m_dwStateDuration = 4;
			GotoState(m_stateMove);
		}
		return false;
	}

	m_posDest.x = x;
	m_posDest.y = y;

	CalculateMoveDuration();

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	CAchievementSystem::Instance().OnCharacterUpdate(this);
#endif

	m_dwStateDuration = 4;

	if (!IsState(m_stateMove))
	{
		MonsterLog("[MOVE] %s", GetVictim() ? "target tracking" : "just move");

		if (GetVictim())
		{
			//MonsterChat(MONSTER_CHAT_CHASE);
			MonsterChat(MONSTER_CHAT_ATTACK);
		}
	}

	GotoState(m_stateMove);

	return true;
}


uint32_t CHARACTER::GetMotionMode() const
{
	uint32_t dwMode = MOTION_MODE_GENERAL;

	if (IsPolymorphed())
		return dwMode;

	LPITEM pkItem;

	if ((pkItem = GetWear(WEAR_WEAPON)))
	{
		switch (pkItem->GetProto()->bSubType)
		{
			case WEAPON_SWORD:
				dwMode = MOTION_MODE_ONEHAND_SWORD;
				break;

			case WEAPON_TWO_HANDED:
				dwMode = MOTION_MODE_TWOHAND_SWORD;
				break;

			case WEAPON_DAGGER:
				dwMode = MOTION_MODE_DUALHAND_SWORD;
				break;

			case WEAPON_BOW:
				dwMode = MOTION_MODE_BOW;
				break;

			case WEAPON_BELL:
				dwMode = MOTION_MODE_BELL;
				break;

			case WEAPON_FAN:
				dwMode = MOTION_MODE_FAN;
				break;
#ifdef ENABLE_WOLFMAN_CHARACTER
			case WEAPON_CLAW:
				dwMode = MOTION_MODE_CLAW;
				break;
#endif

			default:
				dwMode = MOTION_MODE_GENERAL;
				break;
		}
	}
	return dwMode;
}

float CHARACTER::GetMoveMotionSpeed() const
{
	const auto dwMode = GetMotionMode();

	const CMotion* pkMotion = nullptr;

	if (!GetMountVnum())
		pkMotion = CMotionManager::Instance().GetMotion(GetRaceNum(), MAKE_MOTION_KEY(dwMode, (IsWalking() && IsPC()) ? MOTION_WALK : MOTION_RUN));
	else
	{
		pkMotion = CMotionManager::Instance().GetMotion(GetMountVnum(), MAKE_MOTION_KEY(MOTION_MODE_GENERAL, (IsWalking() && IsPC()) ? MOTION_WALK : MOTION_RUN));

		if (!pkMotion)
			pkMotion = CMotionManager::Instance().GetMotion(GetRaceNum(), MAKE_MOTION_KEY(MOTION_MODE_HORSE, (IsWalking() && IsPC()) ? MOTION_WALK : MOTION_RUN));
	}

	if (pkMotion)
		return -pkMotion->GetAccumVector().y / pkMotion->GetDuration();
	else
	{
		sys_err("cannot find motion (name %s race %d mode %d)", GetName(), GetRaceNum(), dwMode);
		return 300.0f;
	}
}

float CHARACTER::GetMoveSpeed() const
{
	return GetMoveMotionSpeed() * 10000 / CalculateDuration(GetLimitPoint(POINT_MOV_SPEED), 10000);
}

void CHARACTER::CalculateMoveDuration()
{
	m_posStart.x = GetX();
	m_posStart.y = GetY();

	const auto fDist = DISTANCE_SQRT(m_posStart.x - m_posDest.x, m_posStart.y - m_posDest.y);
	const auto motionSpeed = GetMoveMotionSpeed();

	m_dwMoveDuration = CalculateDuration(GetLimitPoint(POINT_MOV_SPEED),
		(int)((fDist / motionSpeed) * 1000.0f));

	if (IsNPC())
		sys_log(1, "%s: GOTO: distance %f, spd %u, duration %u, motion speed %f pos %d %d -> %d %d",
			GetName(), fDist, GetLimitPoint(POINT_MOV_SPEED), m_dwMoveDuration, motionSpeed,
			m_posStart.x, m_posStart.y, m_posDest.x, m_posDest.y);

	m_dwMoveStartTime = get_dword_time();
}

// Move to x y position. (Check if there is no moveable and move to the Sync method)
// The server immediately changes the x and y values of the char, but
// In Clara, interpolation is performed up to x and y changed from the previous position.
// walking or running depends on m_bNowWalking of char
// If warp is intended, use Show.
bool CHARACTER::Move(long x, long y)
{
	if (IsPC() && IsDead())	//@fixme431
		return false;

	// No need to move if same location (auto success)
	if (GetX() == x && GetY() == y)
		return true;

	if (test_server)
	{
		if (m_bDetailLog)
			sys_log(0, "%s position %u %u", GetName(), x, y);
	}

	OnMove();
	return Sync(x, y);
}

void CHARACTER::SendMovePacket(uint8_t bFunc, uint8_t bArg, uint32_t x, uint32_t y, uint32_t dwDuration, uint32_t dwTime, int iRot)
{
	TPacketGCMove pack{};

	if (bFunc == FUNC_WAIT)
	{
		x = m_posDest.x;
		y = m_posDest.y;
		dwDuration = m_dwMoveDuration;
	}

	EncodeMovePacket(pack, GetVID(), bFunc, bArg, x, y, dwDuration, dwTime, iRot == -1 ? (int)GetRotation() / 5 : iRot);
	PacketView(&pack, sizeof(TPacketGCMove), this);
}

int CHARACTER::GetRealPoint(uint16_t type) const noexcept	//@fixme532
{
	return m_points.points[type];
}

void CHARACTER::SetRealPoint(uint16_t type, int val) noexcept	//@fixme532
{
	m_points.points[type] = val;
}

int CHARACTER::GetPolymorphPoint(uint16_t type) const	//@fixme532
{
	if (IsPolymorphed() && !IsPolyMaintainStat())
	{
		const auto dwMobVnum = GetPolymorphVnum();
		const CMob* pMob = CMobManager::Instance().Get(dwMobVnum);
		const auto iPower = GetPolymorphPower();

		if (pMob)
		{
			switch (type)
			{
				case POINT_ST:
					if ((GetJob() == JOB_SHAMAN) || ((GetJob() == JOB_SURA) && (GetSkillGroup() == 2)))
						return pMob->m_table.bStr * iPower / 100 + GetPoint(POINT_IQ);
					return pMob->m_table.bStr * iPower / 100 + GetPoint(POINT_ST);

				case POINT_HT:
					return pMob->m_table.bCon * iPower / 100 + GetPoint(POINT_HT);

				case POINT_IQ:
					return pMob->m_table.bInt * iPower / 100 + GetPoint(POINT_IQ);

				case POINT_DX:
					return pMob->m_table.bDex * iPower / 100 + GetPoint(POINT_DX);

				default:
					return 0;
			}
		}
	}

	return GetPoint(type);
}

int CHARACTER::GetPoint(uint16_t type) const // @fixme532
{
	if (type >= POINT_MAX_NUM)
	{
		sys_err("Point type overflow (type %u)", type);
		return 0;
	}

	int val = m_pointsInstant.points[type];
	int max_val = INT_MAX;

	switch (type)
	{
		case POINT_STEAL_HP:
		case POINT_STEAL_SP:
		{
			max_val = 50;
#ifdef ENABLE_ITEM_ATTR_COSTUME
			max_val += 20;
#endif
#ifdef ENABLE_GLOVE_SYSTEM
			max_val += 10;
#endif
		}
		break;
		case POINT_MOV_SPEED:
		{
			// Nyx: Fix compute move speed when you catch a flag in GuildWar
			if (IsAffectFlag(AFF_WAR_FLAG1) || IsAffectFlag(AFF_WAR_FLAG2) || IsAffectFlag(AFF_WAR_FLAG3))
				val = 50;

#ifdef ENABLE_BATTLE_FIELD
			if (IsAffectFlag(AFF_TARGET_VICTIM))
				val = 130;
#endif
		}
		break;

		default:
			break;
	}

	if (val > max_val)
		sys_err("POINT_ERROR: %s type %d val %d (max: %d)", GetName(), type, val, max_val);

	return (val);
}

int CHARACTER::GetLimitPoint(uint16_t type) const // @fixme532
{
	if (type >= POINT_MAX_NUM)
	{
		sys_err("Point type overflow (type %u)", type);
		return 0;
	}

	int val = m_pointsInstant.points[type];
	int max_val = INT_MAX;
	int limit = INT_MAX;
	int min_limit = -INT_MAX;

	switch (type)
	{
		case POINT_ATT_SPEED:
		{
			min_limit = 0;

			if (IsPC())
				limit = 170;
			else
				limit = 250;
		}
		break;

		case POINT_MOV_SPEED:
		{
			min_limit = 0;

			// Nyx: Fix compute move speed when you catch a flag in GuildWar
			if (IsAffectFlag(AFF_WAR_FLAG1) || IsAffectFlag(AFF_WAR_FLAG2) || IsAffectFlag(AFF_WAR_FLAG3))
				val = 50;

#ifdef ENABLE_BATTLE_FIELD
			if (IsAffectFlag(AFF_TARGET_VICTIM))
				val = 130;
#endif

			if (IsPC())
				limit = 200;
			else
				limit = 250;
		}
		break;

		case POINT_STEAL_HP:
		case POINT_STEAL_SP:
		{
			limit = 50;
			max_val = 50;
		}
		break;

		case POINT_MALL_ATTBONUS:
		case POINT_MALL_DEFBONUS:
		{
			limit = 20;
			max_val = 50;
		}
		break;

		default:
			break;
	}

	if (val > max_val)
		sys_err("POINT_ERROR: %s type %d val %d (max: %d)", GetName(), type, val, max_val);

	if (val > limit)
		val = limit;

	if (val < min_limit)
		val = min_limit;

	return (val);
}

void CHARACTER::SetPoint(uint16_t type, int val)	//@fixme532
{
	if (type >= POINT_MAX_NUM)
	{
		sys_err("Point type overflow (type %u)", type);
		return;
	}

	m_pointsInstant.points[type] = val;

	// If the movement has not been completed yet, the movement time calculation must be re-calculated.
	if (type == POINT_MOV_SPEED && get_dword_time() < m_dwMoveStartTime + m_dwMoveDuration)
	{
		CalculateMoveDuration();
	}
}

int CHARACTER::GetAllowedGold() const noexcept
{
	if (GetLevel() <= 10)
		return 100000;
	else if (GetLevel() <= 20)
		return 500000;
	else
		return 50000000;
}

void CHARACTER::CheckMaximumPoints()
{
	if (GetMaxHP() < GetHP())
		PointChange(POINT_HP, GetMaxHP() - GetHP());

	if (GetMaxSP() < GetSP())
		PointChange(POINT_SP, GetMaxSP() - GetSP());
}

void CHARACTER::PointChange(uint16_t type, int amount, bool bAmount, bool bBroadcast)	//@fixme532
{
	int val = 0;

	//sys_log(0, "PointChange %d %d | %d -> %d cHP %d mHP %d", type, amount, GetPoint(type), GetPoint(type)+amount, GetHP(), GetMaxHP());

	switch (type)
	{
		case POINT_NONE:
			return;

		case POINT_LEVEL:
			if ((GetLevel() + amount) > gPlayerMaxLevel)
				return;

			SetLevel(GetLevel() + amount);
			val = GetLevel();

			sys_log(0, "LEVELUP: %s %d NEXT EXP %d", GetName(), GetLevel(), GetNextExp());
#ifdef ENABLE_WOLFMAN_CHARACTER
			if (GetJob() == JOB_WOLFMAN)
			{
				if ((5 <= val) && (GetSkillGroup() != 1))
				{
					ClearSkill();
					// set skill group
					SetSkillGroup(1);
					// set skill points
					SetRealPoint(POINT_SKILL, GetLevel() - 1);
					SetPoint(POINT_SKILL, GetRealPoint(POINT_SKILL));
					PointChange(POINT_SKILL, 0);
					// update points (not required)
					// ComputePoints();
					// PointsPacket();
				}
			}
#endif

			PointChange(POINT_NEXT_EXP,	GetNextExp(), false);

#ifdef ENABLE_ANNOUNCEMENT_LEVELUP
			if (!IsGM())
			{
				switch (val)
				{
					case 50:
					case 75:
					case 90:
					case 95:
					case 105:
					case 115:
					case 120:
						char szNoticeLevelUp[QUERY_MAX_LEN];
						snprintf(szNoticeLevelUp, sizeof(szNoticeLevelUp), LC_TEXT("[CH%d]: [%s] reached level [%u]. Congratulations!"), g_bChannel, GetName(), GetLevel());
						BroadcastNotice(szNoticeLevelUp, true);
						break;
					default:
						break;
				}
			}
#endif

			if (amount)
			{
				quest::CQuestManager::Instance().LevelUp(GetPlayerID());

#ifdef ENABLE_HUNTING_SYSTEM
				CheckHunting();
#endif

				LogManager::Instance().LevelLog(this, val, GetRealPoint(POINT_PLAYTIME) + (get_dword_time() - m_dwPlayStartTime) / 60000);

				if (GetGuild())
					GetGuild()->LevelChange(GetPlayerID(), GetLevel());

				if (GetParty())
					GetParty()->RequestSetMemberLevel(GetPlayerID(), GetLevel());

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
				CAchievementSystem::Instance().OnCharacterUpdate(this);
#endif

#if defined(ENABLE_PREMIUM_PRIVATE_SHOP) && defined(ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL)
				SyncPrivateShopTailInfo();
#endif
			}
			break;

		case POINT_NEXT_EXP:
			val = GetNextExp();
			bAmount = false; // Unconditionally, bAmount must be false.
			break;

		case POINT_EXP:
			{
#ifdef ENABLE_ANTI_EXP
				if (FindAffect(AFFECT_EXP_CURSE) && amount > 0)
				{
					return;
				}
#endif
				uint32_t exp = GetExp();
				const uint32_t next_exp = GetNextExp();

				// Youth protection
				if (g_bChinaIntoxicationCheck)
				{
					if (IsOverTime(OT_NONE))
					{
						dev_log(LOG_DEB0, "<EXP_LOG> %s = NONE", GetName());
					}
					else if (IsOverTime(OT_3HOUR))
					{
						amount = (amount / 2);
						dev_log(LOG_DEB0, "<EXP_LOG> %s = 3HOUR", GetName());
					}
					else if (IsOverTime(OT_5HOUR))
					{
						amount = 0;
						dev_log(LOG_DEB0, "<EXP_LOG> %s = 5HOUR", GetName());
					}
				}

				// Don't let exp go below 0
				if ((amount < 0) && (exp < static_cast<uint32_t>(-amount)))
				{
					sys_log(1, "%s AMOUNT < 0 %d, CUR EXP: %d", GetName(), -amount, exp);
					amount = -exp;

					SetExp(exp + amount);
					val = GetExp();
				}
				else
				{
					/*if (gPlayerMaxLevel <= GetLevel())
						return;*/

					if (gPlayerMaxLevel == GetLevel() && (next_exp / 4) <= exp)	//@fixme405
						return;

					if (test_server)
						ChatPacket(CHAT_TYPE_EXP_INFO, "Du hast %d Erfahrungspunkte erhalten", amount);

					uint32_t iExpBalance = 0;

					if (gPlayerMaxLevel == GetLevel())	//@fixme405
					{
						if ((amount + exp) > (next_exp / 4))
							amount = ((next_exp / 4) - exp);
					}

					// Level up!
					if (exp + amount >= next_exp)
					{
						iExpBalance = (exp + amount) - next_exp;
						amount = next_exp - exp;

						SetExp(0);
						exp = next_exp;
					}
					else
					{
						SetExp(exp + amount);
						exp = GetExp();
					}

					const uint32_t q = static_cast<uint32_t>(next_exp / 4.0f);
					int iLevStep = GetRealPoint(POINT_LEVEL_STEP);

					// If iLevStep is 4 or higher, the level must have risen, so it cannot be reached here.
					if (iLevStep >= 4)
					{
						sys_err("%s LEVEL_STEP bigger than 4! (%d)", GetName(), iLevStep);
						iLevStep = 4;
					}

					if (exp >= next_exp && iLevStep < 4)
					{
						for (int i = 0; i < 4 - iLevStep; ++i)
							PointChange(POINT_LEVEL_STEP, 1, false, true);
					}
					else if (exp >= q * 3 && iLevStep < 3)
					{
						for (int i = 0; i < 3 - iLevStep; ++i)
							PointChange(POINT_LEVEL_STEP, 1, false, true);
					}
					else if (exp >= q * 2 && iLevStep < 2)
					{
						for (int i = 0; i < 2 - iLevStep; ++i)
							PointChange(POINT_LEVEL_STEP, 1, false, true);
					}
					else if (exp >= q && iLevStep < 1)
						PointChange(POINT_LEVEL_STEP, 1);

					if (iExpBalance)
					{
						PointChange(POINT_EXP, iExpBalance);
					}

					val = GetExp();
				}
			}
			break;

		case POINT_LEVEL_STEP:
			if (amount > 0)
			{
				val = GetPoint(POINT_LEVEL_STEP) + amount;

				switch (val)
				{
					case 1:
					case 2:
					case 3:
						if ((GetLevel() <= g_iStatusPointGetLevelLimit) &&
							(GetLevel() <= gPlayerMaxLevel) ) // @fixme104
							PointChange(POINT_STAT, 1);
						break;

					case 4:
						{
							const auto iHP = number(JobInitialPoints[GetJob()].hp_per_lv_begin, JobInitialPoints[GetJob()].hp_per_lv_end);
							const auto iSP = number(JobInitialPoints[GetJob()].sp_per_lv_begin, JobInitialPoints[GetJob()].sp_per_lv_end);

							m_points.iRandomHP += iHP;
							m_points.iRandomSP += iSP;

							if (GetSkillGroup())
							{
								if (GetLevel() >= 5)
									PointChange(POINT_SKILL, 1);

								if (GetLevel() >= 9)
									PointChange(POINT_SUB_SKILL, 1);
							}

							PointChange(POINT_MAX_HP, iHP);
							PointChange(POINT_MAX_SP, iSP);
							PointChange(POINT_LEVEL, 1, false, true);

							val = 0;
						}
						break;

					default:
						break;
				}

#ifdef ENABLE_POTIONS_LEVELUP
				if (GetLevel() <= 10)
					AutoGiveItem(27001, 2);
				else if (GetLevel() <= 30)
					AutoGiveItem(27002, 2);
				else
				{
					AutoGiveItem(27003, 2);
				}
#endif

				PointChange(POINT_HP, GetMaxHP() - GetHP());
				PointChange(POINT_SP, GetMaxSP() - GetSP());
				PointChange(POINT_STAMINA, GetMaxStamina() - GetStamina());

				SetPoint(POINT_LEVEL_STEP, val);
				SetRealPoint(POINT_LEVEL_STEP, val);

				Save();
			}
			else
				val = GetPoint(POINT_LEVEL_STEP);

			break;

#ifdef ENABLE_YOHARA_SYSTEM
		case POINT_CONQUEROR_LEVEL:
			if ((GetConquerorLevel() + amount) > gConquerorMaxLevel)
				return;

			SetConquerorLevel(GetConquerorLevel() + amount);
			val = GetConquerorLevel();

			sys_log(0, "CLEVELUP: %s %d CNEXT EXP %d", GetName(), GetConquerorLevel(), GetConquerorNextExp());
			PointChange(POINT_CONQUEROR_NEXT_EXP, GetConquerorNextExp(), false);
			break;

		case POINT_CONQUEROR_NEXT_EXP:
			val = GetConquerorNextExp();
			bAmount = false;
			break;

		case POINT_CONQUEROR_EXP:
			{
#ifdef ENABLE_ANTI_EXP
				if (FindAffect(AFFECT_EXP_CURSE) && amount > 0)
				{
					return;
				}
#endif

				auto conqueror_exp = GetConquerorExp();
				const auto conqueror_next_exp = GetConquerorNextExp();

				// Reduktion: Setze auf null, wenn unter null gefallen
				if ((amount < 0) && (conqueror_exp < static_cast<uint32_t>(-amount)))
				{
					sys_log(1, "%s AMOUNT < 0 %d, CUR EXP: %d", GetName(), -amount, conqueror_exp);
					amount = -conqueror_exp;

					SetConquerorExp(conqueror_exp + amount);
					val = GetConquerorExp();
				}
				else
				{
#ifdef ENABLE_CONQUEROR_MAX_LEVEL_EXP
					if (gConquerorMaxLevel <= GetConquerorLevel())
					{
						// Spieler hat das Maximallevel erreicht
						auto max_exp = conqueror_next_exp - 1;

						if (conqueror_exp + amount > max_exp)
						{
							// Verhindere Overflow und begrenze Erfahrung auf das Maximum
							SetConquerorExp(max_exp);
							val = GetConquerorExp();
							return;
						}
						else
						{
							// Erfahrung normal hinzufügen, wenn sie unter der Grenze bleibt
							SetConquerorExp(conqueror_exp + amount);
							val = GetConquerorExp();
							return;
						}
					}
#else
					if (gConquerorMaxLevel <= GetConquerorLevel())
						return;
#endif // ENABLE_CONQUEROR_MAX_LEVEL_EXP

					if (test_server)
						ChatPacket(CHAT_TYPE_EXP_INFO, "Du hast %d Erfahrungspunkte erhalten", amount);

					uint32_t iExpBalance = 0;

					if (conqueror_exp + amount >= conqueror_next_exp)
					{
						iExpBalance = (conqueror_exp + amount) - conqueror_next_exp;
						amount = conqueror_next_exp - conqueror_exp;

						SetConquerorExp(0);
						conqueror_exp = conqueror_next_exp;
					}
					else
					{
						SetConquerorExp(conqueror_exp + amount);
						conqueror_exp = GetConquerorExp();
					}

					const auto q = static_cast<uint32_t>(conqueror_next_exp / 4.0f);
					int ciLevStep = GetRealPoint(POINT_CONQUEROR_LEVEL_STEP);

					if (ciLevStep >= 4)
					{
						sys_err("%s LEVEL_STEP bigger than 4! (%d)", GetName(), ciLevStep);
						ciLevStep = 4;
					}

					if (conqueror_exp >= conqueror_next_exp && ciLevStep < 4)
					{
						for (int i = 0; i < 4 - ciLevStep; ++i)
							PointChange(POINT_CONQUEROR_LEVEL_STEP, 1, false, true);
					}
					else if (conqueror_exp >= q * 3 && ciLevStep < 3)
					{
						for (int i = 0; i < 3 - ciLevStep; ++i)
							PointChange(POINT_CONQUEROR_LEVEL_STEP, 1, false, true);
					}
					else if (conqueror_exp >= q * 2 && ciLevStep < 2)
					{
						for (int i = 0; i < 2 - ciLevStep; ++i)
							PointChange(POINT_CONQUEROR_LEVEL_STEP, 1, false, true);
					}
					else if (conqueror_exp >= q && ciLevStep < 1)
						PointChange(POINT_CONQUEROR_LEVEL_STEP, 1);

					if (iExpBalance)
					{
						PointChange(POINT_CONQUEROR_EXP, iExpBalance);
					}

					val = GetConquerorExp();
				}
			}
			break;

		case POINT_CONQUEROR_LEVEL_STEP:
			if (amount > 0)
			{
				val = GetPoint(POINT_CONQUEROR_LEVEL_STEP) + amount;

				switch (val)
				{
					case 1:
					case 2:
					case 3:
						if ((GetConquerorLevel() <= g_iConquerorPointGetLevelLimit) &&
							(GetConquerorLevel() <= gConquerorMaxLevel) ) // @fixme104
							PointChange(POINT_CONQUEROR_POINT, 1);
						break;

					case 4:
						{
							const auto iHP = number(JobInitialPoints[GetJob()].hp_per_lv_begin, JobInitialPoints[GetJob()].hp_per_lv_end);
							const auto iSP = number(JobInitialPoints[GetJob()].sp_per_lv_begin, JobInitialPoints[GetJob()].sp_per_lv_end);

							m_points.iRandomHP += iHP;
							m_points.iRandomSP += iSP;

							PointChange(POINT_MAX_HP, iHP);
							PointChange(POINT_MAX_SP, iSP);
							PointChange(POINT_CONQUEROR_LEVEL, 1, false, true);
							val = 0;
						}
						break;

					default:
						break;
				}

				SetPoint(POINT_CONQUEROR_LEVEL_STEP, val);
				SetRealPoint(POINT_CONQUEROR_LEVEL_STEP, val);

				Save();
			}
			else
				val = GetPoint(POINT_CONQUEROR_LEVEL_STEP);

			break;
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
		case POINT_BATTLE_PASS_PREMIUM_ID:
		{
			SetExtBattlePassPremiumID(amount);
			val = GetExtBattlePassPremiumID();
		}
		break;
#endif

		case POINT_HP:
			{
				if (IsDead() || IsStun())
					return;

				const auto prev_hp = GetHP();

				amount = MIN(GetMaxHP() - GetHP(), amount);
				SetHP(GetHP() + amount);
				val = GetHP();

				BroadcastTargetPacket();

				if (GetParty() && IsPC() && val != prev_hp)
					GetParty()->SendPartyInfoOneToAll(this);
			}
			break;

		case POINT_SP:
			{
				if (IsDead() || IsStun())
					return;

				amount = MIN(GetMaxSP() - GetSP(), amount);
				SetSP(GetSP() + amount);
				val = GetSP();
			}
			break;

		case POINT_STAMINA:
			{
				if (IsDead() || IsStun())
					return;

				const auto prev_val = GetStamina();
				amount = MIN(GetMaxStamina() - GetStamina(), amount);
				SetStamina(GetStamina() + amount);
				val = GetStamina();

				if (val == 0)
				{
					// Without Stamina, let's walk!
					SetNowWalking(true);
				}
				else if (prev_val == 0)
				{
					// Return to the previous mode because there was no stamina
					ResetWalking();
				}

				if (amount < 0 && val != 0) // No reduction is sent.
					return;
			}
			break;

		case POINT_MAX_HP:
			{
				SetPoint(type, GetPoint(type) + amount);

				//SetMaxHP(GetMaxHP() + amount);
				// Max Life = (Base Max Life + Additional) * Max Life%
				const int hp = GetRealPoint(POINT_MAX_HP);
				int add_hp = MIN(3500, hp * GetPoint(POINT_MAX_HP_PCT) / 100);
				add_hp += GetPoint(POINT_MAX_HP);
				add_hp += GetPoint(POINT_PARTY_TANKER_BONUS);

#ifdef ENABLE_YOHARA_SYSTEM
				int max_hp_val = hp + add_hp;

				if (IsPC() && IsSungmaMap() && (GetPoint(POINT_SUNGMA_HP) < GetSungmaMapAttribute(POINT_SUNGMA_HP)))
					max_hp_val /= 2;

				SetMaxHP(max_hp_val);
#else
				SetMaxHP(hp + add_hp);
#endif
				val = GetMaxHP();
			}
			break;

		case POINT_MAX_SP:
			{
				SetPoint(type, GetPoint(type) + amount);

				//SetMaxSP(GetMaxSP() + amount);
				// Max Spirit = (Base Max Spirit + Additional) * Max Spirit%
				const auto sp = GetRealPoint(POINT_MAX_SP);
				auto add_sp = MIN(800, sp * GetPoint(POINT_MAX_SP_PCT) / 100);
				add_sp += GetPoint(POINT_MAX_SP);
				add_sp += GetPoint(POINT_PARTY_SKILL_MASTER_BONUS);

				SetMaxSP(sp + add_sp);

				val = GetMaxSP();
			}
			break;

		case POINT_MAX_HP_PCT:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);

			PointChange(POINT_MAX_HP, 0);
			break;

		case POINT_MAX_SP_PCT:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);

			PointChange(POINT_MAX_SP, 0);
			break;

		case POINT_MAX_STAMINA:
			SetMaxStamina(GetMaxStamina() + amount);
			val = GetMaxStamina();
			break;

		case POINT_GOLD:
			{
				const int64_t nTotalMoney = static_cast<int64_t>(GetGold()) + static_cast<int64_t>(amount);

				if (GOLD_MAX <= nTotalMoney)
				{
#ifdef AUTO_CHANGE_YANG_TO_WON
					if (GetGold() >= AUTO_WON_AMOUNT - 1) {
						if (GetCheque() >= CHEQUE_MAX - 1) {
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Won limit reached."));
							sys_err("[OVERFLOW_CHEQUE] OriCheque %d AddedCheque %d id %u Name %s ", GetCheque(), amount, GetPlayerID(), GetName());
							LogManager::Instance().CharLog(this, GetCheque() + amount, "OVERFLOW_CHEQUE", "");
							return; // Abbruch der weiteren Verarbeitung
						}

						SetGold(GetGold() - AUTO_YANG_TO_WON_AMOUNT);
						PointChange(POINT_GOLD, 0, true); // Gold-Update an den Client senden
						PointChange(POINT_CHEQUE, 1, true);
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Yang limit reached! Automatic conversion to Won."));
					}
#endif
					sys_err("[OVERFLOW_GOLD] OriGold %d AddedGold %d id %u Name %s ", GetGold(), amount, GetPlayerID(), GetName());
					LogManager::Instance().CharLog(this, GetGold() + amount, "OVERFLOW_GOLD", "");
					return;
				}

				if (g_bChinaIntoxicationCheck && amount > 0) {
					if (IsOverTime(OT_NONE)) {
						dev_log(LOG_DEB0, "<GOLD_LOG> %s = NONE", GetName());
					}
					else if (IsOverTime(OT_3HOUR)) {
						amount = (amount / 2);
						dev_log(LOG_DEB0, "<GOLD_LOG> %s = 3HOUR", GetName());
					}
					else if (IsOverTime(OT_5HOUR)) {
						amount = 0;
						dev_log(LOG_DEB0, "<GOLD_LOG> %s = 5HOUR", GetName());
					}
				}

				SetGold(GetGold() + amount);
				val = GetGold();
			}
			break;

#ifdef ENABLE_CHEQUE_SYSTEM
		case POINT_CHEQUE:
			{
				const int64_t nTotalCheque = static_cast<int64_t>(GetCheque()) + static_cast<int64_t>(amount);
				if (CHEQUE_MAX <= nTotalCheque)
				{
					sys_err("[OVERFLOW_CHEQUE] OriCheque %d AddedCheque %d id %u Name %s ", GetCheque(), amount, GetPlayerID(), GetName());
					LogManager::Instance().CharLog(this, GetCheque() + amount, "OVERFLOW_CHEQUE", "");
					return;
				}

				SetCheque(GetCheque() + amount);
				val = GetCheque();
			}
			break;
#endif

#ifdef ENABLE_GEM_SYSTEM
		case POINT_GEM:
			{
				const int64_t nTotalGem = static_cast<int64_t>(GetGemPoint()) + static_cast<int64_t>(amount);

				if (GEM_MAX <= nTotalGem)
				{
					sys_err("[OVERFLOW_GEM] OriGem %d AddedGem %d id %u Name %s ", GetGemPoint(), amount, GetPlayerID(), GetName());
					LogManager::Instance().CharLog(this, GetGemPoint() + amount, "OVERFLOW_GEM", "");
					return;
				}

				SetGemPoint(GetGemPoint() + amount);
				val = GetGemPoint();
			}
			break;
#endif

#ifdef ENABLE_BATTLE_FIELD
		case POINT_BATTLE_FIELD:
		{
			const int64_t nTotalBattlePoints = static_cast<int64_t>(GetBattlePoint()) + static_cast<int64_t>(amount);

			if ((BATTLE_POINT_MAX <= nTotalBattlePoints) || (nTotalBattlePoints < 0))
				return;

			SetBattlePoint(GetBattlePoint() + amount);
			val = GetBattlePoint();
		}
		break;
#endif

#ifdef ENABLE_MEDAL_OF_HONOR
		case POINT_MEDAL_OF_HONOR:
			{
				const int64_t nTotalMedalHonor = static_cast<int64_t>(GetMedalHonor()) + static_cast<int64_t>(amount);

				if ((MEDAL_OF_HONOR_MAX <= nTotalMedalHonor) || (nTotalMedalHonor < 0))
					return;

				SetMedalHonor(GetMedalHonor() + amount);
				val = GetMedalHonor();
			}
			break;
#endif

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
		case POINT_SOUL:
		{
			const int64_t nTotalSoul = static_cast<int64_t>(GetSoulPoint()) + static_cast<int64_t>(amount);

			if (SOUL_MAX <= nTotalSoul)
			{
				sys_err("[OVERFLOW_SOULPOINT] OriSoul %d AddedSoul %d id %u Name %s ", GetSoulPoint(), amount, GetPlayerID(), GetName());
				LogManager::Instance().CharLog(this, GetSoulPoint() + amount, "OVERFLOW_SOULPOINT", "");
				return;
			}

			SetSoulPoint(GetSoulPoint() + amount);
			val = GetSoulPoint();
		}
		break;
		case POINT_SOUL_RE:
		{
			const int64_t nTotalSoulRe = static_cast<int64_t>(GetSoulRePoint()) + static_cast<int64_t>(amount);

			if (SOUL_RE_MAX <= nTotalSoulRe)
			{
				sys_err("[OVERFLOW_SOULPOINTRE] OriSoul %d AddedSoulRe %d id %u Name %s ", GetSoulRePoint(), amount, GetPlayerID(), GetName());
				LogManager::Instance().CharLog(this, GetSoulRePoint() + amount, "OVERFLOW_SOULPOINTRE", "");
				return;
			}

			SetSoulRePoint(GetSoulRePoint() + amount);
			val = GetSoulRePoint();
		}
		break;
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
		case POINT_NEW_EQUIPMENT_ACTUAL:
		{
			SetPageEquipment(amount);
			val = GetPageEquipment();
		}
		break;

		case POINT_NEW_EQUIPMENT_TOTAL:
		{
			if (GetPageTotalEquipment() + amount >= NEW_EQUIPMENT_MAX_PAGE)
				return;

			SetPageTotalEquipment(GetPageTotalEquipment() + amount);
			val = GetPageTotalEquipment();
		}
		break;
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
		case POINT_LOTTO_MONEY:
		{
			SetLottoMoney(GetLottoMoney() + amount);
			val = GetLottoMoney();
		}
		break;

		case POINT_LOTTO_TOTAL_MONEY:
		{
			SetLottoTotalMoney(GetLottoTotalMoney() + amount);
			val = GetLottoTotalMoney();
		}
		break;
#endif

		case POINT_SKILL:
		case POINT_STAT:
#ifdef ENABLE_YOHARA_SYSTEM
		case POINT_CONQUEROR_POINT:
#endif
		case POINT_SUB_SKILL:
		case POINT_STAT_RESET_COUNT:
		case POINT_HORSE_SKILL:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);

			SetRealPoint(type, val);
			break;

		case POINT_DEF_GRADE:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);

			PointChange(POINT_CLIENT_DEF_GRADE, amount);
			break;

		case POINT_CLIENT_DEF_GRADE:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			break;

#ifdef ENABLE_YOHARA_SYSTEM
		case POINT_MOV_SPEED:
			{
				SetPoint(type, GetPoint(type) + amount);

				const int move = GetRealPoint(POINT_MOV_SPEED);
				const int add_move = GetPoint(POINT_MOV_SPEED);
				int move_speed = move + add_move;

				if (IsPC() && IsSungmaMap() && (GetPoint(POINT_SUNGMA_MOVE) < GetSungmaMapAttribute(POINT_SUNGMA_MOVE)))
				{
					move_speed /= 2;

					if (move_speed >= 200)
						move_speed = 200;
				}

				SetMove(move_speed + amount);
				val = GetMove();
			}
			break;
#endif

		case POINT_ST:
		case POINT_HT:
		case POINT_DX:
		case POINT_IQ:
		case POINT_HP_REGEN:
		case POINT_SP_REGEN:
		case POINT_ATT_SPEED:
		case POINT_ATT_GRADE:
#ifndef ENABLE_YOHARA_SYSTEM
		case POINT_MOV_SPEED:
#endif
		case POINT_CASTING_SPEED:
		case POINT_MAGIC_ATT_GRADE:
		case POINT_MAGIC_DEF_GRADE:
		case POINT_BOW_DISTANCE:
		case POINT_HP_RECOVERY:
		case POINT_SP_RECOVERY:

		case POINT_ATTBONUS_HUMAN:
		case POINT_ATTBONUS_ANIMAL:
		case POINT_ATTBONUS_ORC:
		case POINT_ATTBONUS_MILGYO:
		case POINT_ATTBONUS_UNDEAD:
		case POINT_ATTBONUS_DEVIL:
		case POINT_ATTBONUS_MONSTER:
		case POINT_ATTBONUS_SURA:
		case POINT_ATTBONUS_ASSASSIN:
		case POINT_ATTBONUS_WARRIOR:
		case POINT_ATTBONUS_SHAMAN:
		case POINT_ATTBONUS_WOLFMAN:
		case POINT_POISON_PCT:
		case POINT_BLEEDING_PCT:
		case POINT_STUN_PCT:
		case POINT_SLOW_PCT:

		case POINT_BLOCK:
		case POINT_DODGE:

		case POINT_CRITICAL_PCT:
		case POINT_RESIST_CRITICAL:
		case POINT_PENETRATE_PCT:
		case POINT_RESIST_PENETRATE:
		case POINT_CURSE_PCT:

		case POINT_STEAL_HP:
		case POINT_STEAL_SP:

		case POINT_MANA_BURN_PCT:
		case POINT_DAMAGE_SP_RECOVER:
		case POINT_RESIST_NORMAL_DAMAGE:
		case POINT_RESIST_SWORD:
		case POINT_RESIST_TWOHAND:
		case POINT_RESIST_DAGGER:
		case POINT_RESIST_BELL:
		case POINT_RESIST_FAN:
		case POINT_RESIST_BOW:
		case POINT_RESIST_CLAW:
		case POINT_RESIST_FIRE:
		case POINT_RESIST_ELEC:
		case POINT_RESIST_MAGIC:
		case POINT_ACCEDRAIN_RATE:
		case POINT_RESIST_MAGIC_REDUCTION:
		case POINT_RESIST_WIND:
		case POINT_RESIST_ICE:
		case POINT_RESIST_EARTH:
		case POINT_RESIST_DARK:
		case POINT_REFLECT_MELEE:
		case POINT_REFLECT_CURSE:
		case POINT_POISON_REDUCE:
		case POINT_BLEEDING_REDUCE:
		case POINT_KILL_SP_RECOVER:
		case POINT_KILL_HP_RECOVERY:
		case POINT_HIT_HP_RECOVERY:
		case POINT_HIT_SP_RECOVERY:
		case POINT_MANASHIELD:
		case POINT_ATT_BONUS:
		case POINT_DEF_BONUS:
		case POINT_SKILL_DAMAGE_BONUS:
		case POINT_NORMAL_HIT_DAMAGE_BONUS:

			// DEPEND_BONUS_ATTRIBUTES
		case POINT_SKILL_DEFEND_BONUS:
		case POINT_NORMAL_HIT_DEFEND_BONUS:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			break;
			// END_OF_DEPEND_BONUS_ATTRIBUTES

		case POINT_PARTY_ATTACKER_BONUS:
		case POINT_PARTY_TANKER_BONUS:
		case POINT_PARTY_BUFFER_BONUS:
		case POINT_PARTY_SKILL_MASTER_BONUS:
		case POINT_PARTY_HASTE_BONUS:
		case POINT_PARTY_DEFENDER_BONUS:

		case POINT_RESIST_WARRIOR:
		case POINT_RESIST_ASSASSIN:
		case POINT_RESIST_SURA:
		case POINT_RESIST_SHAMAN:
		case POINT_RESIST_WOLFMAN:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			break;

		case POINT_MALL_ATTBONUS:
		case POINT_MALL_DEFBONUS:
		case POINT_MALL_EXPBONUS:
		case POINT_MALL_ITEMBONUS:
		case POINT_MALL_GOLDBONUS:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			break;
		case POINT_MELEE_MAGIC_ATT_BONUS_PER:
		case POINT_MAGIC_ATT_BONUS_PER:
			if (GetPoint(type) + amount > 100)
			{
				sys_err("POINT_MELEE_MAGIC_ATT_BONUS_PER exceeded over 100!! point type: %d name: %s amount %d", type, GetName(), amount);
				amount = 100 - GetPoint(type);
			}

			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			break;

			// PC_BANG_ITEM_ADD
		case POINT_PC_BANG_EXP_BONUS:
		case POINT_PC_BANG_DROP_BONUS:
		case POINT_RAMADAN_CANDY_BONUS_EXP:
			SetPoint(type, amount);
			val = GetPoint(type);
			break;
			// END_PC_BANG_ITEM_ADD

		case POINT_EXP_DOUBLE_BONUS:
		case POINT_GOLD_DOUBLE_BONUS:
		case POINT_ITEM_DROP_BONUS:
		case POINT_POTION_BONUS:
			//if (GetPoint(type) + amount > 100)
			//{
			//	sys_err("BONUS exceeded over 100!! point type: %d name: %s amount %d", type, GetName(), amount);
			//	amount = 100 - GetPoint(type);
			//}

			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			break;

		case POINT_IMMUNE_STUN:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			if (val)
			{
				// ChatPacket(CHAT_TYPE_INFO, "IMMUNE_STUN SET_BIT type(%u) amount(%d)", type, amount);
				SET_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_STUN);
			}
			else
			{
				// ChatPacket(CHAT_TYPE_INFO, "IMMUNE_STUN REMOVE_BIT type(%u) amount(%d)", type, amount);
				REMOVE_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_STUN);
			}
			break;

		case POINT_IMMUNE_SLOW:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			if (val)
			{
				SET_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_SLOW);
			}
			else
			{
				REMOVE_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_SLOW);
			}
			break;

		case POINT_IMMUNE_FALL:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			if (val)
			{
				SET_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_FALL);
			}
			else
			{
				REMOVE_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_FALL);
			}
			break;

		case POINT_ATT_GRADE_BONUS:
			SetPoint(type, GetPoint(type) + amount);
			PointChange(POINT_ATT_GRADE, amount);
			val = GetPoint(type);
			break;

		case POINT_DEF_GRADE_BONUS:
			SetPoint(type, GetPoint(type) + amount);
			PointChange(POINT_DEF_GRADE, amount);
			val = GetPoint(type);
			break;

		case POINT_MAGIC_ATT_GRADE_BONUS:
			SetPoint(type, GetPoint(type) + amount);
			PointChange(POINT_MAGIC_ATT_GRADE, amount);
			val = GetPoint(type);
			break;

		case POINT_MAGIC_DEF_GRADE_BONUS:
			SetPoint(type, GetPoint(type) + amount);
			PointChange(POINT_MAGIC_DEF_GRADE, amount);
			val = GetPoint(type);
			break;

		case POINT_VOICE:
		case POINT_EMPIRE_POINT:
			//sys_err("CHARACTER::PointChange: %s: point cannot be changed. use SetPoint instead (type: %d)", GetName(), type);
			val = GetRealPoint(type);
			break;

		case POINT_POLYMORPH:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			SetPolymorph(val);
			break;

		case POINT_MOUNT:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			//MountVnum(val);
			break;

		case POINT_ENERGY:
		case POINT_COSTUME_ATTR_BONUS:
			{
				const auto old_val = GetPoint(type);
				SetPoint(type, old_val + amount);
				val = GetPoint(type);
				BuffOnAttr_ValueChange(type, old_val, val);
			}
			break;

		case POINT_ENCHANT_ELECT:
		case POINT_ENCHANT_FIRE:
		case POINT_ENCHANT_ICE:
		case POINT_ENCHANT_WIND:
		case POINT_ENCHANT_EARTH:
		case POINT_ENCHANT_DARK:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			BroadcastTargetPacket();
			break;

		case POINT_ATTBONUS_CZ:
		case POINT_ATTBONUS_INSECT:
		case POINT_ATTBONUS_DESERT:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			break;

		case POINT_ATTBONUS_SWORD:
		case POINT_ATTBONUS_TWOHAND:
		case POINT_ATTBONUS_DAGGER:
		case POINT_ATTBONUS_BELL:
		case POINT_ATTBONUS_FAN:
		case POINT_ATTBONUS_BOW:
		case POINT_ATTBONUS_CLAW:
		case POINT_RESIST_HUMAN:
		case POINT_RESIST_MOUNT_FALL:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			break;

#ifdef ENABLE_REFINE_ELEMENT
		case POINT_ATT_ELEMENTS:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			break;
#endif

#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
		case POINT_RESIST_FIST:
		case POINT_SKILL_DAMAGE_SAMYEON:
		case POINT_SKILL_DAMAGE_TANHWAN:
		case POINT_SKILL_DAMAGE_PALBANG:
		case POINT_SKILL_DAMAGE_GIGONGCHAM:
		case POINT_SKILL_DAMAGE_GYOKSAN:
		case POINT_SKILL_DAMAGE_GEOMPUNG:
		case POINT_SKILL_DAMAGE_AMSEOP:
		case POINT_SKILL_DAMAGE_GUNGSIN:
		case POINT_SKILL_DAMAGE_CHARYUN:
		case POINT_SKILL_DAMAGE_SANGONG:
		case POINT_SKILL_DAMAGE_YEONSA:
		case POINT_SKILL_DAMAGE_KWANKYEOK:
		case POINT_SKILL_DAMAGE_GIGUNG:
		case POINT_SKILL_DAMAGE_HWAJO:
		case POINT_SKILL_DAMAGE_SWAERYUNG:
		case POINT_SKILL_DAMAGE_YONGKWON:
		case POINT_SKILL_DAMAGE_PABEOB:
		case POINT_SKILL_DAMAGE_MARYUNG:
		case POINT_SKILL_DAMAGE_HWAYEOMPOK:
		case POINT_SKILL_DAMAGE_MAHWAN:
		case POINT_SKILL_DAMAGE_BIPABU:
		case POINT_SKILL_DAMAGE_YONGBI:
		case POINT_SKILL_DAMAGE_PAERYONG:
		case POINT_SKILL_DAMAGE_NOEJEON:
		case POINT_SKILL_DAMAGE_BYEURAK:
		case POINT_SKILL_DAMAGE_CHAIN:
		case POINT_SKILL_DAMAGE_CHAYEOL:
		case POINT_SKILL_DAMAGE_SALPOONG:
		case POINT_SKILL_DAMAGE_GONGDAB:
		case POINT_SKILL_DAMAGE_PASWAE:
		case POINT_NORMAL_HIT_DEFEND_BONUS_BOSS_OR_MORE:
		case POINT_SKILL_DEFEND_BONUS_BOSS_OR_MORE:
		case POINT_NORMAL_HIT_DAMAGE_BONUS_BOSS_OR_MORE:
		case POINT_SKILL_DAMAGE_BONUS_BOSS_OR_MORE:
		case POINT_HIT_BUFF_ENCHANT_FIRE:
		case POINT_HIT_BUFF_ENCHANT_ICE:
		case POINT_HIT_BUFF_ENCHANT_ELEC:
		case POINT_HIT_BUFF_ENCHANT_WIND:
		case POINT_HIT_BUFF_ENCHANT_DARK:
		case POINT_HIT_BUFF_ENCHANT_EARTH:
		case POINT_HIT_BUFF_RESIST_FIRE:
		case POINT_HIT_BUFF_RESIST_ICE:
		case POINT_HIT_BUFF_RESIST_ELEC:
		case POINT_HIT_BUFF_RESIST_WIND:
		case POINT_HIT_BUFF_RESIST_DARK:
		case POINT_HIT_BUFF_RESIST_EARTH:
		case POINT_USE_SKILL_CHEONGRANG_MOV_SPEED:
		case POINT_USE_SKILL_CHEONGRANG_CASTING_SPEED:
		case POINT_USE_SKILL_CHAYEOL_CRITICAL_PCT:
		case POINT_USE_SKILL_SANGONG_ATT_GRADE_BONUS:
		case POINT_USE_SKILL_GIGUNG_ATT_GRADE_BONUS:
		case POINT_USE_SKILL_JEOKRANG_DEF_BONUS:
		case POINT_USE_SKILL_GWIGEOM_DEF_BONUS:
		case POINT_USE_SKILL_TERROR_ATT_GRADE_BONUS:
		case POINT_USE_SKILL_MUYEONG_ATT_GRADE_BONUS:
		case POINT_USE_SKILL_MANASHILED_CASTING_SPEED:
		case POINT_USE_SKILL_HOSIN_DEF_BONUS:
		case POINT_USE_SKILL_GICHEON_ATT_GRADE_BONUS:
		case POINT_USE_SKILL_JEONGEOP_ATT_GRADE_BONUS:
		case POINT_USE_SKILL_JEUNGRYEOK_DEF_BONUS:
		case POINT_USE_SKILL_GIHYEOL_ATT_GRADE_BONUS:
		case POINT_USE_SKILL_CHUNKEON_CASTING_SPEED:
		case POINT_USE_SKILL_NOEGEOM_ATT_GRADE_BONUS:
		case POINT_SKILL_DURATION_INCREASE_EUNHYUNG:
		case POINT_SKILL_DURATION_INCREASE_GYEONGGONG:
		case POINT_SKILL_DURATION_INCREASE_GEOMKYUNG:
		case POINT_SKILL_DURATION_INCREASE_JEOKRANG:
		case POINT_USE_SKILL_PALBANG_HP_ABSORB:
		case POINT_USE_SKILL_AMSEOP_HP_ABSORB:
		case POINT_USE_SKILL_YEONSA_HP_ABSORB:
		case POINT_USE_SKILL_YONGBI_HP_ABSORB:
		case POINT_USE_SKILL_CHAIN_HP_ABSORB:
		case POINT_USE_SKILL_PASWAE_SP_ABSORB:
		case POINT_USE_SKILL_GIGONGCHAM_STUN:
		case POINT_USE_SKILL_CHARYUN_STUN:
		case POINT_USE_SKILL_PABEOB_STUN:
		case POINT_USE_SKILL_MAHWAN_STUN:
		case POINT_USE_SKILL_GONGDAB_STUN:
		case POINT_USE_SKILL_SAMYEON_STUN:
		case POINT_USE_SKILL_GYOKSAN_KNOCKBACK:
		case POINT_USE_SKILL_SEOMJEON_KNOCKBACK:
		case POINT_USE_SKILL_SWAERYUNG_KNOCKBACK:
		case POINT_USE_SKILL_HWAYEOMPOK_KNOCKBACK:
		case POINT_USE_SKILL_GONGDAB_KNOCKBACK:
		case POINT_USE_SKILL_KWANKYEOK_KNOCKBACK:
		case POINT_USE_SKILL_SAMYEON_NEXT_COOLTIME_DECREASE_10PER:
		case POINT_USE_SKILL_GEOMPUNG_NEXT_COOLTIME_DECREASE_10PER:
		case POINT_USE_SKILL_GUNGSIN_NEXT_COOLTIME_DECREASE_10PER:
		case POINT_USE_SKILL_KWANKYEOK_NEXT_COOLTIME_DECREASE_10PER:
		case POINT_USE_SKILL_YONGKWON_NEXT_COOLTIME_DECREASE_10PER:
		case POINT_USE_SKILL_MARYUNG_NEXT_COOLTIME_DECREASE_10PER:
		case POINT_USE_SKILL_BIPABU_NEXT_COOLTIME_DECREASE_10PER:
		case POINT_USE_SKILL_NOEJEON_NEXT_COOLTIME_DECREASE_10PER:
		case POINT_USE_SKILL_SALPOONG_NEXT_COOLTIME_DECREASE_10PER:
		case POINT_USE_SKILL_PASWAE_NEXT_COOLTIME_DECREASE_10PER:
		case POINT_ATTBONUS_STONE:
		case POINT_DAMAGE_HP_RECOVERY:
		case POINT_DAMAGE_SP_RECOVERY:
		case POINT_ALIGNMENT_DAMAGE_BONUS:
		case POINT_NORMAL_DAMAGE_GUARD:
		case POINT_MORE_THEN_HP90_DAMAGE_REDUCE:
		case POINT_USE_SKILL_TUSOK_HP_ABSORB:
		case POINT_USE_SKILL_PAERYONG_HP_ABSORB:
		case POINT_USE_SKILL_BYEURAK_HP_ABSORB:
		case POINT_FIRST_ATTRIBUTE_BONUS:
		case POINT_SECOND_ATTRIBUTE_BONUS:
		case POINT_THIRD_ATTRIBUTE_BONUS:
		case POINT_FOURTH_ATTRIBUTE_BONUS:
		case POINT_FIFTH_ATTRIBUTE_BONUS:
		case POINT_USE_SKILL_SAMYEON_NEXT_COOLTIME_DECREASE_20PER:
		case POINT_USE_SKILL_GEOMPUNG_NEXT_COOLTIME_DECREASE_20PER:
		case POINT_USE_SKILL_GUNGSIN_NEXT_COOLTIME_DECREASE_20PER:
		case POINT_USE_SKILL_KWANKYEOK_NEXT_COOLTIME_DECREASE_20PER:
		case POINT_USE_SKILL_YONGKWON_NEXT_COOLTIME_DECREASE_20PER:
		case POINT_USE_SKILL_MARYUNG_NEXT_COOLTIME_DECREASE_20PER:
		case POINT_USE_SKILL_BIPABU_NEXT_COOLTIME_DECREASE_20PER:
		case POINT_USE_SKILL_NOEJEON_NEXT_COOLTIME_DECREASE_20PER:
		case POINT_USE_SKILL_SALPOONG_NEXT_COOLTIME_DECREASE_20PER:
		case POINT_USE_SKILL_PASWAE_NEXT_COOLTIME_DECREASE_20PER:
		case POINT_USE_SKILL_CHAYEOL_HP_ABSORB:
		case POINT_HIT_PCT:
		case POINT_RANDOM:
		case POINT_ATTBONUS_PER_HUMAN:
		case POINT_ATTBONUS_PER_ANIMAL:
		case POINT_ATTBONUS_PER_ORC:
		case POINT_ATTBONUS_PER_MILGYO:
		case POINT_ATTBONUS_PER_UNDEAD:
		case POINT_ATTBONUS_PER_DEVIL:
		case POINT_ENCHANT_PER_ELECT:
		case POINT_ENCHANT_PER_FIRE:
		case POINT_ENCHANT_PER_ICE:
		case POINT_ENCHANT_PER_WIND:
		case POINT_ENCHANT_PER_EARTH:
		case POINT_ENCHANT_PER_DARK:
		case POINT_ATTBONUS_PER_CZ:
		case POINT_ATTBONUS_PER_INSECT:
		case POINT_ATTBONUS_PER_DESERT:
		case POINT_ATTBONUS_PER_STONE:
		case POINT_ATTBONUS_PER_MONSTER:
		case POINT_RESIST_PER_HUMAN:
		case POINT_RESIST_PER_ICE:
		case POINT_RESIST_PER_DARK:
		case POINT_RESIST_PER_EARTH:
		case POINT_RESIST_PER_FIRE:
		case POINT_RESIST_PER_ELEC:
		case POINT_RESIST_PER_MAGIC:
		case POINT_RESIST_PER_WIND:
		case POINT_HIT_BUFF_SUNGMA_STR:
		case POINT_HIT_BUFF_SUNGMA_MOVE:
		case POINT_HIT_BUFF_SUNGMA_HP:
		case POINT_HIT_BUFF_SUNGMA_IMMUNE:
		case POINT_MOUNT_MELEE_MAGIC_ATTBONUS_PER:
		case POINT_DISMOUNT_MOVE_SPEED_BONUS_PER:
		case POINT_HIT_AUTO_HP_RECOVERY:
		case POINT_HIT_AUTO_SP_RECOVERY:
		case POINT_USE_SKILL_COOLTIME_DECREASE_ALL:
		case POINT_HIT_STONE_ATTBONUS_STONE:
		case POINT_HIT_STONE_DEF_GRADE_BONUS:
		case POINT_KILL_BOSS_ITEM_BONUS:
		case POINT_MOB_HIT_MOB_AGGRESSIVE:
		case POINT_NO_DEATH_AND_HP_RECOVERY30:
		case POINT_AUTO_PICKUP:
		case POINT_MOUNT_NO_KNOCKBACK:
		case POINT_IMMUNE_POISON100:
		case POINT_IMMUNE_BLEEDING100:
		case POINT_MONSTER_DEFEND_BONUS:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			break;
#endif

#ifdef ENABLE_YOHARA_SYSTEM
		case POINT_SUNGMA_STR:
			amount *= 1 + (GetPoint(POINT_SUNGMA_PER_STR) / 100);
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);

			PointChange(POINT_ST, 0);
			break;

		case POINT_SUNGMA_HP:
			amount *= 1 + (GetPoint(POINT_SUNGMA_PER_HP) / 100);
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);

			PointChange(POINT_MAX_HP, 0);
			break;

		case POINT_SUNGMA_MOVE:
			amount *= 1 + (GetPoint(POINT_SUNGMA_PER_MOVE) / 100);
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);

			PointChange(POINT_MOV_SPEED, 0);
			break;

		case POINT_SUNGMA_IMMUNE:
			amount *= 1 + (GetPoint(POINT_SUNGMA_PER_IMMUNE) / 100);
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			break;

		case POINT_SUNGMA_PER_STR:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);

			PointChange(POINT_ST, 0);
			break;

		case POINT_SUNGMA_PER_HP:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);

			PointChange(POINT_MAX_HP, 0);
			break;

		case POINT_SUNGMA_PER_MOVE:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);

			PointChange(POINT_MOV_SPEED, 0);
			break;

		case POINT_SUNGMA_PER_IMMUNE:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			break;
#endif

#ifdef ENABLE_FLOWER_EVENT
		case POINT_FLOWER_TYPE_1:
		case POINT_FLOWER_TYPE_2:
		case POINT_FLOWER_TYPE_3:
		case POINT_FLOWER_TYPE_4:
		case POINT_FLOWER_TYPE_5:
		case POINT_FLOWER_TYPE_6:
		{
			uint8_t id_list = type - POINT_FLOWER_TYPE_1;
			const uint32_t old_val = GetFlowerEventValue(id_list);
			m_FlowerEvent[id_list].value = old_val + amount;
			val = GetFlowerEventValue(id_list);
		}
		break;
#endif

		default:
			sys_err("CHARACTER::PointChange: %s: unknown point change type %d", GetName(), type);
			return;
	}

	switch (type)
	{
		case POINT_LEVEL:
		case POINT_ST:
		case POINT_DX:
		case POINT_IQ:
		case POINT_HT:
#ifdef ENABLE_YOHARA_SYSTEM
		case POINT_CONQUEROR_LEVEL:
		case POINT_SUNGMA_STR:
		case POINT_SUNGMA_HP:
		case POINT_SUNGMA_MOVE:
		case POINT_SUNGMA_IMMUNE:
#endif
			ComputeBattlePoints();
			break;
		case POINT_MAX_HP:
		case POINT_MAX_SP:
		case POINT_MAX_STAMINA:
			break;

		default:
			break;
	}

	if (type == POINT_HP && amount == 0)
		return;

	if (GetDesc())
	{
		TPacketGCPointChange pack{};
		pack.header = HEADER_GC_CHARACTER_POINT_CHANGE;
		pack.dwVID = m_vid;
		pack.type = type;
		pack.value = val;
		pack.amount = bAmount ? amount : 0;

		if (!bBroadcast)
			GetDesc()->Packet(&pack, sizeof(TPacketGCPointChange));
		else
			PacketAround(&pack, sizeof(pack));
	}
}

void CHARACTER::ApplyPoint(uint16_t wApplyType, int iVal)	//@fixme532
{
	switch (wApplyType)
	{
		case APPLY_NONE:
			break;

		case APPLY_CON:
			PointChange(POINT_HT, iVal);
			PointChange(POINT_MAX_HP, (iVal * JobInitialPoints[GetJob()].hp_per_ht));
			PointChange(POINT_MAX_STAMINA, (iVal * JobInitialPoints[GetJob()].stamina_per_con));
			break;

		case APPLY_INT:
			PointChange(POINT_IQ, iVal);
			PointChange(POINT_MAX_SP, (iVal * JobInitialPoints[GetJob()].sp_per_iq));
			break;

			case APPLY_SKILL: // SKILL_DAMAGE_BONUS
			{
				// 8-bit vnum, 9-bit add, 15-bit change based on the most significant bit
				// 00000000 00000000 00000000 00000000
				// ^^^^^^^^ ^^^^^^^^^^^^^^^^^^^^^^^^^
				// vnum ^ add change
				auto bSkillVnum = static_cast<uint8_t>(static_cast<uint32_t>(iVal) >> 24);
				const auto iAdd = iVal & 0x00800000;
				auto iChange = iVal & 0x007fffff;

				sys_log(1, "APPLY_SKILL skill %d add? %d change %d", bSkillVnum, iAdd ? 1 : 0, iChange);

				if (0 == iAdd)
					iChange = -iChange;

				const std::unordered_map<uint8_t, int>::iterator iter = m_SkillDamageBonus.find(bSkillVnum);

				if (iter == m_SkillDamageBonus.end())
					m_SkillDamageBonus.insert(std::make_pair(bSkillVnum, iChange));
				else
					iter->second += iChange;
			}
			// END_OF_SKILL_DAMAGE_BONUS
			break;

		/* NOTE: Since the max HP bonus and the quest reward bonus by items use the same method,
		* If only MAX_HP is calculated, there is a problem in the case of quest rewards. Actually, this is also reasonable...
		* The changed formula calculates the ratio of the current maximum hp to the possessed hp, and then corrects the hp based on the changed maximum hp.
		* Originally, it would be better to do PointChange, but it is difficult due to design problems, so skip..
		* Calculate the SP in the same way.
		* Mantis : 101460 ~ ity ~
		*/
		case APPLY_MAX_HP:
		case APPLY_MAX_HP_PCT:
			{
				const auto i = GetMaxHP(); if (i == 0) break;
				PointChange(aApplyInfo[wApplyType].wPointType, iVal);
				const auto fRatio = static_cast<float>(GetMaxHP()) / static_cast<float>(i);
				PointChange(POINT_HP, GetHP() * fRatio - GetHP());
			}
			break;

		case APPLY_MAX_SP:
		case APPLY_MAX_SP_PCT:
			{
				const auto i = GetMaxSP(); if (i == 0) break;
				PointChange(aApplyInfo[wApplyType].wPointType, iVal);
				const auto fRatio = static_cast<float>(GetMaxSP()) / static_cast<float>(i);
				PointChange(POINT_SP, GetSP() * fRatio - GetSP());
			}
			break;

		case APPLY_STR:
		case APPLY_DEX:
		case APPLY_ATT_SPEED:
		case APPLY_MOV_SPEED:
		case APPLY_CAST_SPEED:
		case APPLY_HP_REGEN:
		case APPLY_SP_REGEN:
		case APPLY_POISON_PCT:
		case APPLY_BLEEDING_PCT:
		case APPLY_STUN_PCT:
		case APPLY_SLOW_PCT:
		case APPLY_CRITICAL_PCT:
		case APPLY_PENETRATE_PCT:
		case APPLY_ATTBONUS_HUMAN:
		case APPLY_ATTBONUS_ANIMAL:
		case APPLY_ATTBONUS_ORC:
		case APPLY_ATTBONUS_MILGYO:
		case APPLY_ATTBONUS_UNDEAD:
		case APPLY_ATTBONUS_DEVIL:
		case APPLY_ATTBONUS_WARRIOR:
		case APPLY_ATTBONUS_ASSASSIN:
		case APPLY_ATTBONUS_SURA:
		case APPLY_ATTBONUS_SHAMAN:
		case APPLY_ATTBONUS_WOLFMAN:
		case APPLY_ATTBONUS_MONSTER:
		case APPLY_STEAL_HP:
		case APPLY_STEAL_SP:
		case APPLY_MANA_BURN_PCT:
		case APPLY_DAMAGE_SP_RECOVER:
		case APPLY_BLOCK:
		case APPLY_DODGE:
		case APPLY_RESIST_SWORD:
		case APPLY_RESIST_TWOHAND:
		case APPLY_RESIST_DAGGER:
		case APPLY_RESIST_BELL:
		case APPLY_RESIST_FAN:
		case APPLY_RESIST_BOW:
		case APPLY_RESIST_CLAW:
		case APPLY_RESIST_FIRE:
		case APPLY_RESIST_ELEC:
		case APPLY_RESIST_MAGIC:
		case APPLY_RESIST_WIND:
		case APPLY_RESIST_ICE:
		case APPLY_RESIST_EARTH:
		case APPLY_RESIST_DARK:
		case APPLY_REFLECT_MELEE:
		case APPLY_REFLECT_CURSE:
		case APPLY_ANTI_CRITICAL_PCT:
		case APPLY_ANTI_PENETRATE_PCT:
		case APPLY_POISON_REDUCE:
		case APPLY_BLEEDING_REDUCE:
		case APPLY_KILL_SP_RECOVER:
		case APPLY_EXP_DOUBLE_BONUS:
		case APPLY_GOLD_DOUBLE_BONUS:
		case APPLY_ITEM_DROP_BONUS:
		case APPLY_POTION_BONUS:
		case APPLY_KILL_HP_RECOVER:
		case APPLY_IMMUNE_STUN:
		case APPLY_IMMUNE_SLOW:
		case APPLY_IMMUNE_FALL:
		case APPLY_BOW_DISTANCE:
		case APPLY_ATT_GRADE_BONUS:
		case APPLY_DEF_GRADE_BONUS:
		case APPLY_MAGIC_ATT_GRADE:
		case APPLY_MAGIC_DEF_GRADE:
		case APPLY_CURSE_PCT:
		case APPLY_MAX_STAMINA:
		case APPLY_MALL_ATTBONUS:
		case APPLY_MALL_DEFBONUS:
		case APPLY_MALL_EXPBONUS:
		case APPLY_MALL_ITEMBONUS:
		case APPLY_MALL_GOLDBONUS:
		case APPLY_SKILL_DAMAGE_BONUS:
		case APPLY_NORMAL_HIT_DAMAGE_BONUS:

			// DEPEND_BONUS_ATTRIBUTES
		case APPLY_SKILL_DEFEND_BONUS:
		case APPLY_NORMAL_HIT_DEFEND_BONUS:
			// END_OF_DEPEND_BONUS_ATTRIBUTES

		case APPLY_PC_BANG_EXP_BONUS:
		case APPLY_PC_BANG_DROP_BONUS:

		case APPLY_RESIST_WARRIOR:
		case APPLY_RESIST_ASSASSIN:
		case APPLY_RESIST_SURA:
		case APPLY_RESIST_SHAMAN:
		case APPLY_RESIST_WOLFMAN:
		case APPLY_ENERGY:
		case APPLY_DEF_GRADE:
		case APPLY_COSTUME_ATTR_BONUS:
		case APPLY_MAGIC_ATTBONUS_PER:
		case APPLY_MELEE_MAGIC_ATTBONUS_PER:
		case APPLY_ACCEDRAIN_RATE:
		case APPLY_RESIST_MAGIC_REDUCTION:
		case APPLY_ENCHANT_ELECT:
		case APPLY_ENCHANT_FIRE:
		case APPLY_ENCHANT_ICE:
		case APPLY_ENCHANT_WIND:
		case APPLY_ENCHANT_EARTH:
		case APPLY_ENCHANT_DARK:
		case APPLY_ATTBONUS_CZ:
		case APPLY_ATTBONUS_INSECT:
		case APPLY_ATTBONUS_DESERT:
		case APPLY_ATTBONUS_SWORD:
		case APPLY_ATTBONUS_TWOHAND:
		case APPLY_ATTBONUS_DAGGER:
		case APPLY_ATTBONUS_BELL:
		case APPLY_ATTBONUS_FAN:
		case APPLY_ATTBONUS_BOW:
		case APPLY_ATTBONUS_CLAW:
		case APPLY_RESIST_HUMAN:
		case APPLY_RESIST_MOUNT_FALL:
		case APPLY_RESIST_FIST:
		case APPLY_MOUNT:
			PointChange(aApplyInfo[wApplyType].wPointType, iVal);
			break;

#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
		case APPLY_SKILL_DAMAGE_SAMYEON:
		case APPLY_SKILL_DAMAGE_TANHWAN:
		case APPLY_SKILL_DAMAGE_PALBANG:
		case APPLY_SKILL_DAMAGE_GIGONGCHAM:
		case APPLY_SKILL_DAMAGE_GYOKSAN:
		case APPLY_SKILL_DAMAGE_GEOMPUNG:
		case APPLY_SKILL_DAMAGE_AMSEOP:
		case APPLY_SKILL_DAMAGE_GUNGSIN:
		case APPLY_SKILL_DAMAGE_CHARYUN:
		case APPLY_SKILL_DAMAGE_SANGONG:
		case APPLY_SKILL_DAMAGE_YEONSA:
		case APPLY_SKILL_DAMAGE_KWANKYEOK:
		case APPLY_SKILL_DAMAGE_GIGUNG:
		case APPLY_SKILL_DAMAGE_HWAJO:
		case APPLY_SKILL_DAMAGE_SWAERYUNG:
		case APPLY_SKILL_DAMAGE_YONGKWON:
		case APPLY_SKILL_DAMAGE_PABEOB:
		case APPLY_SKILL_DAMAGE_MARYUNG:
		case APPLY_SKILL_DAMAGE_HWAYEOMPOK:
		case APPLY_SKILL_DAMAGE_MAHWAN:
		case APPLY_SKILL_DAMAGE_BIPABU:
		case APPLY_SKILL_DAMAGE_YONGBI:
		case APPLY_SKILL_DAMAGE_PAERYONG:
		case APPLY_SKILL_DAMAGE_NOEJEON:
		case APPLY_SKILL_DAMAGE_BYEURAK:
		case APPLY_SKILL_DAMAGE_CHAIN:
		case APPLY_SKILL_DAMAGE_CHAYEOL:
		case APPLY_SKILL_DAMAGE_SALPOONG:
		case APPLY_SKILL_DAMAGE_GONGDAB:
		case APPLY_SKILL_DAMAGE_PASWAE:
		case APPLY_NORMAL_HIT_DEFEND_BONUS_BOSS_OR_MORE:
		case APPLY_SKILL_DEFEND_BONUS_BOSS_OR_MORE:
		case APPLY_NORMAL_HIT_DAMAGE_BONUS_BOSS_OR_MORE:
		case APPLY_SKILL_DAMAGE_BONUS_BOSS_OR_MORE:
		case APPLY_HIT_BUFF_ENCHANT_FIRE:
		case APPLY_HIT_BUFF_ENCHANT_ICE:
		case APPLY_HIT_BUFF_ENCHANT_ELEC:
		case APPLY_HIT_BUFF_ENCHANT_WIND:
		case APPLY_HIT_BUFF_ENCHANT_DARK:
		case APPLY_HIT_BUFF_ENCHANT_EARTH:
		case APPLY_HIT_BUFF_RESIST_FIRE:
		case APPLY_HIT_BUFF_RESIST_ICE:
		case APPLY_HIT_BUFF_RESIST_ELEC:
		case APPLY_HIT_BUFF_RESIST_WIND:
		case APPLY_HIT_BUFF_RESIST_DARK:
		case APPLY_HIT_BUFF_RESIST_EARTH:
		case APPLY_USE_SKILL_CHEONGRANG_MOV_SPEED:
		case APPLY_USE_SKILL_CHEONGRANG_CASTING_SPEED:
		case APPLY_USE_SKILL_CHAYEOL_CRITICAL_PCT:
		case APPLY_USE_SKILL_SANGONG_ATT_GRADE_BONUS:
		case APPLY_USE_SKILL_GIGUNG_ATT_GRADE_BONUS:
		case APPLY_USE_SKILL_JEOKRANG_DEF_BONUS:
		case APPLY_USE_SKILL_GWIGEOM_DEF_BONUS:
		case APPLY_USE_SKILL_TERROR_ATT_GRADE_BONUS:
		case APPLY_USE_SKILL_MUYEONG_ATT_GRADE_BONUS:
		case APPLY_USE_SKILL_MANASHILED_CASTING_SPEED:
		case APPLY_USE_SKILL_HOSIN_DEF_BONUS:
		case APPLY_USE_SKILL_GICHEON_ATT_GRADE_BONUS:
		case APPLY_USE_SKILL_JEONGEOP_ATT_GRADE_BONUS:
		case APPLY_USE_SKILL_JEUNGRYEOK_DEF_BONUS:
		case APPLY_USE_SKILL_GIHYEOL_ATT_GRADE_BONUS:
		case APPLY_USE_SKILL_CHUNKEON_CASTING_SPEED:
		case APPLY_USE_SKILL_NOEGEOM_ATT_GRADE_BONUS:
		case APPLY_SKILL_DURATION_INCREASE_EUNHYUNG:
		case APPLY_SKILL_DURATION_INCREASE_GYEONGGONG:
		case APPLY_SKILL_DURATION_INCREASE_GEOMKYUNG:
		case APPLY_SKILL_DURATION_INCREASE_JEOKRANG:
		case APPLY_USE_SKILL_PALBANG_HP_ABSORB:
		case APPLY_USE_SKILL_AMSEOP_HP_ABSORB:
		case APPLY_USE_SKILL_YEONSA_HP_ABSORB:
		case APPLY_USE_SKILL_YONGBI_HP_ABSORB:
		case APPLY_USE_SKILL_CHAIN_HP_ABSORB:
		case APPLY_USE_SKILL_PASWAE_SP_ABSORB:
		case APPLY_USE_SKILL_GIGONGCHAM_STUN:
		case APPLY_USE_SKILL_CHARYUN_STUN:
		case APPLY_USE_SKILL_PABEOB_STUN:
		case APPLY_USE_SKILL_MAHWAN_STUN:
		case APPLY_USE_SKILL_GONGDAB_STUN:
		case APPLY_USE_SKILL_SAMYEON_STUN:
		case APPLY_USE_SKILL_GYOKSAN_KNOCKBACK:
		case APPLY_USE_SKILL_SEOMJEON_KNOCKBACK:
		case APPLY_USE_SKILL_SWAERYUNG_KNOCKBACK:
		case APPLY_USE_SKILL_HWAYEOMPOK_KNOCKBACK:
		case APPLY_USE_SKILL_GONGDAB_KNOCKBACK:
		case APPLY_USE_SKILL_KWANKYEOK_KNOCKBACK:
		case APPLY_USE_SKILL_SAMYEON_NEXT_COOLTIME_DECREASE_10PER:
		case APPLY_USE_SKILL_GEOMPUNG_NEXT_COOLTIME_DECREASE_10PER:
		case APPLY_USE_SKILL_GUNGSIN_NEXT_COOLTIME_DECREASE_10PER:
		case APPLY_USE_SKILL_KWANKYEOK_NEXT_COOLTIME_DECREASE_10PER:
		case APPLY_USE_SKILL_YONGKWON_NEXT_COOLTIME_DECREASE_10PER:
		case APPLY_USE_SKILL_MARYUNG_NEXT_COOLTIME_DECREASE_10PER:
		case APPLY_USE_SKILL_BIPABU_NEXT_COOLTIME_DECREASE_10PER:
		case APPLY_USE_SKILL_NOEJEON_NEXT_COOLTIME_DECREASE_10PER:
		case APPLY_USE_SKILL_SALPOONG_NEXT_COOLTIME_DECREASE_10PER:
		case APPLY_USE_SKILL_PASWAE_NEXT_COOLTIME_DECREASE_10PER:
		case APPLY_ATTBONUS_STONE:
		case APPLY_DAMAGE_HP_RECOVERY:
		case APPLY_DAMAGE_SP_RECOVERY:
		case APPLY_ALIGNMENT_DAMAGE_BONUS:
		case APPLY_NORMAL_DAMAGE_GUARD:
		case APPLY_MORE_THEN_HP90_DAMAGE_REDUCE:
		case APPLY_USE_SKILL_TUSOK_HP_ABSORB:
		case APPLY_USE_SKILL_PAERYONG_HP_ABSORB:
		case APPLY_USE_SKILL_BYEURAK_HP_ABSORB:
		case APPLY_FIRST_ATTRIBUTE_BONUS:
		case APPLY_SECOND_ATTRIBUTE_BONUS:
		case APPLY_THIRD_ATTRIBUTE_BONUS:
		case APPLY_FOURTH_ATTRIBUTE_BONUS:
		case APPLY_FIFTH_ATTRIBUTE_BONUS:
		case APPLY_USE_SKILL_SAMYEON_NEXT_COOLTIME_DECREASE_20PER:
		case APPLY_USE_SKILL_GEOMPUNG_NEXT_COOLTIME_DECREASE_20PER:
		case APPLY_USE_SKILL_GUNGSIN_NEXT_COOLTIME_DECREASE_20PER:
		case APPLY_USE_SKILL_KWANKYEOK_NEXT_COOLTIME_DECREASE_20PER:
		case APPLY_USE_SKILL_YONGKWON_NEXT_COOLTIME_DECREASE_20PER:
		case APPLY_USE_SKILL_MARYUNG_NEXT_COOLTIME_DECREASE_20PER:
		case APPLY_USE_SKILL_BIPABU_NEXT_COOLTIME_DECREASE_20PER:
		case APPLY_USE_SKILL_NOEJEON_NEXT_COOLTIME_DECREASE_20PER:
		case APPLY_USE_SKILL_SALPOONG_NEXT_COOLTIME_DECREASE_20PER:
		case APPLY_USE_SKILL_PASWAE_NEXT_COOLTIME_DECREASE_20PER:
		case APPLY_USE_SKILL_CHAYEOL_HP_ABSORB:
		case APPLY_SUNGMA_STR:
		case APPLY_SUNGMA_HP:
		case APPLY_SUNGMA_MOVE:
		case APPLY_SUNGMA_IMMUNE:
		case APPLY_HIT_PCT:
		case APPLY_RANDOM:
		case APPLY_ATTBONUS_PER_HUMAN:
		case APPLY_ATTBONUS_PER_ANIMAL:
		case APPLY_ATTBONUS_PER_ORC:
		case APPLY_ATTBONUS_PER_MILGYO:
		case APPLY_ATTBONUS_PER_UNDEAD:
		case APPLY_ATTBONUS_PER_DEVIL:
		case APPLY_ENCHANT_PER_ELECT:
		case APPLY_ENCHANT_PER_FIRE:
		case APPLY_ENCHANT_PER_ICE:
		case APPLY_ENCHANT_PER_WIND:
		case APPLY_ENCHANT_PER_EARTH:
		case APPLY_ENCHANT_PER_DARK:
		case APPLY_ATTBONUS_PER_CZ:
		case APPLY_ATTBONUS_PER_INSECT:
		case APPLY_ATTBONUS_PER_DESERT:
		case APPLY_ATTBONUS_PER_STONE:
		case APPLY_ATTBONUS_PER_MONSTER:
		case APPLY_RESIST_PER_HUMAN:
		case APPLY_RESIST_PER_ICE:
		case APPLY_RESIST_PER_DARK:
		case APPLY_RESIST_PER_EARTH:
		case APPLY_RESIST_PER_FIRE:
		case APPLY_RESIST_PER_ELEC:
		case APPLY_RESIST_PER_MAGIC:
		case APPLY_RESIST_PER_WIND:
		case APPLY_HIT_BUFF_SUNGMA_STR:
		case APPLY_HIT_BUFF_SUNGMA_MOVE:
		case APPLY_HIT_BUFF_SUNGMA_HP:
		case APPLY_HIT_BUFF_SUNGMA_IMMUNE:
		case APPLY_MOUNT_MELEE_MAGIC_ATTBONUS_PER:
		case APPLY_DISMOUNT_MOVE_SPEED_BONUS_PER:
		case APPLY_HIT_AUTO_HP_RECOVERY:
		case APPLY_HIT_AUTO_SP_RECOVERY:
		case APPLY_USE_SKILL_COOLTIME_DECREASE_ALL:
		case APPLY_HIT_STONE_ATTBONUS_STONE:
		case APPLY_HIT_STONE_DEF_GRADE_BONUS:
		case APPLY_KILL_BOSS_ITEM_BONUS:
		case APPLY_MOB_HIT_MOB_AGGRESSIVE:
		case APPLY_NO_DEATH_AND_HP_RECOVERY30:
		case APPLY_AUTO_PICKUP:
		case APPLY_MOUNT_NO_KNOCKBACK:
		case APPLY_SUNGMA_PER_STR:
		case APPLY_SUNGMA_PER_HP:
		case APPLY_SUNGMA_PER_MOVE:
		case APPLY_SUNGMA_PER_IMMUNE:
		case APPLY_IMMUNE_POISON100:
		case APPLY_IMMUNE_BLEEDING100:
		case APPLY_MONSTER_DEFEND_BONUS:
			PointChange(aApplyInfo[wApplyType].wPointType, iVal);
			break;
#endif

		default:
			sys_err("Unknown apply type %d name %s", wApplyType, GetName());
			break;
	}
}

void CHARACTER::MotionPacketEncode(uint8_t motion, const LPCHARACTER& victim, TPacketGCMotion* packet)
{
	packet->header = HEADER_GC_MOTION;
	packet->vid = m_vid;
	packet->motion = motion;

	if (victim)
		packet->victim_vid = victim->GetVID();
	else
		packet->victim_vid = 0;
}

void CHARACTER::Motion(uint8_t motion, const LPCHARACTER& victim)
{
	TPacketGCMotion pack_motion;
	MotionPacketEncode(motion, victim, &pack_motion);
	PacketAround(&pack_motion, sizeof(TPacketGCMotion));
}

#ifdef ENABLE_AFK_MODE_SYSTEM
EVENTFUNC(update_character_event)
{
	char_event_info* info = dynamic_cast<char_event_info*>( event->info );
	if ( info == nullptr ) {
		sys_err( "update_character_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER	ch = info->ch;

	if (nullptr == ch || ch->IsNPC())
		return 0;

	int gLastMoveTime = (get_dword_time() - ch->GetLastMoveTime() > 60000 * AFK_TIME_LIMIT);

	if (gLastMoveTime)
	{	
		if (!ch->IsAway())
			ch->SetAway(true);
		
		CAffect* pAffect = ch->FindAffect(AFFECT_AFK);
		//bool pAffectFlag = ch->IsAffectFlag(AFF_AFK);

		if (!pAffect/* || !pAffectFlag*/) {
			ch->AddAffect(AFFECT_AFK, POINT_NONE, 0, AFF_AFK, INFINITE_AFFECT_DURATION, 0, true);
		}
	}

	return PASSES_PER_SEC(15);	//every 15sec will this check recalled
}

void CHARACTER::StartUpdateCharacterEvent()
{
	if (m_pkUpdateCharacter)
		return;
	
	char_event_info* info = AllocEventInfo<char_event_info>();
	info->ch = this;
	m_pkUpdateCharacter = event_create(update_character_event, info, PASSES_PER_SEC(15));
}
#endif

EVENTFUNC(save_event)
{
	const char_event_info* info = dynamic_cast<char_event_info*>(event->info);
	if (info == nullptr)
	{
		sys_err("save_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == nullptr) // <Factor>
		return 0;

	sys_log(1, "SAVE_EVENT: %s", ch->GetName());
	ch->Save();
	ch->FlushDelayedSaveItem();
	return (save_event_second_cycle);
}

void CHARACTER::StartSaveEvent()
{
	if (m_pkSaveEvent)
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();

	info->ch = this;
	m_pkSaveEvent = event_create(save_event, info, save_event_second_cycle);
}

void CHARACTER::MonsterLog(const char* format, ...)
{
	if (!test_server)
		return;

	if (IsPC())
		return;

	char chatbuf[CHAT_MAX_LEN + 1];
	int len = snprintf(chatbuf, sizeof(chatbuf), "%u)", (uint32_t)GetVID());

	if (len < 0 || len >= (int)sizeof(chatbuf))
		len = sizeof(chatbuf) - 1;

	va_list args;

	va_start(args, format);

	const int len2 = vsnprintf(chatbuf + len, sizeof(chatbuf) - len, format, args);

	if (len2 < 0 || len2 >= (int)sizeof(chatbuf) - len)
		len += (sizeof(chatbuf) - len) - 1;
	else
		len += len2;

	// Includes \0 character
	++len;

	va_end(args);

	TPacketGCChat pack_chat{};
	pack_chat.header = HEADER_GC_CHAT;
	pack_chat.size = sizeof(TPacketGCChat) + len;
	pack_chat.type = CHAT_TYPE_TALKING;
	pack_chat.id = (uint32_t)GetVID();
	pack_chat.bEmpire = 0;

	TEMP_BUFFER buf;
	buf.write(&pack_chat, sizeof(TPacketGCChat));
	buf.write(chatbuf, len);

	CHARACTER_MANAGER::Instance().PacketMonsterLog(this, buf.read_peek(), buf.size());
}

void CHARACTER::ChatPacket(uint8_t type, const char* format, ...)
{
	LPDESC d = GetDesc();

	if (!d || !format)
		return;

	char chatbuf[CHAT_MAX_LEN + 1]{};
	va_list args;

	va_start(args, format);
	int len = vsnprintf(chatbuf, sizeof(chatbuf), format, args);
	va_end(args);

	struct packet_chat pack_chat{};
	pack_chat.header = HEADER_GC_CHAT;
	pack_chat.size = sizeof(struct packet_chat) + len;
	pack_chat.type = type;
	pack_chat.id = 0;
	pack_chat.bEmpire = d->GetEmpire();
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	pack_chat.bLanguage = LANGUAGE_NONE;
#endif
#ifdef ENABLE_EMPIRE_FLAG
	pack_chat.bFlag = false;
#endif

	TEMP_BUFFER buf;
	buf.write(&pack_chat, sizeof(struct packet_chat));
	buf.write(chatbuf, len);

	d->Packet(buf.read_peek(), buf.size());

	if (type == CHAT_TYPE_COMMAND && test_server)
		sys_log(0, "SEND_COMMAND %s %s", GetName(), chatbuf);
}

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
#ifdef ENABLE_EMPIRE_FLAG
void CHARACTER::ChatPacket(uint8_t bType, uint8_t bEmpire, uint8_t bLanguage, bool bFlag, const char* c_szFormat, ...)
#else
void CHARACTER::ChatPacket(uint8_t bType, uint8_t bEmpire, uint8_t bLanguage, const char* c_szFormat, ...)
#endif
{
	LPDESC d = GetDesc();

	if (!d || !c_szFormat)
		return;

	char chatbuf[CHAT_MAX_LEN + 1];
	va_list args;

	va_start(args, c_szFormat);
	int len = vsnprintf(chatbuf, sizeof(chatbuf), c_szFormat, args);
	va_end(args);

	struct packet_chat pack_chat;
	pack_chat.header = HEADER_GC_CHAT;
	pack_chat.size = sizeof(struct packet_chat) + len;
	pack_chat.type = bType;
	pack_chat.id = 0;
	pack_chat.bEmpire = bEmpire;
	pack_chat.bLanguage = bLanguage;
#ifdef ENABLE_EMPIRE_FLAG
	pack_chat.bFlag = bFlag;
#endif

	TEMP_BUFFER buf;
	buf.write(&pack_chat, sizeof(struct packet_chat));
	buf.write(chatbuf, len);

	d->Packet(buf.read_peek(), buf.size());
}
#endif

// MINING
void CHARACTER::mining_take()
{
	m_pkMiningEvent = nullptr;
}

void CHARACTER::mining_cancel()
{
	if (m_pkMiningEvent)
	{
		sys_log(0, "XXX MINING CANCEL");
		event_cancel(&m_pkMiningEvent);
		ChatPacket(CHAT_TYPE_INFO, "[LS;815]");
	}
}

void CHARACTER::mining(LPCHARACTER chLoad)
{
	if (m_pkMiningEvent)
	{
		mining_cancel();
		return;
	}

	if (!chLoad)
		return;

	// @fixme128
	if (GetMapIndex() != chLoad->GetMapIndex() || DISTANCE_APPROX(GetX() - chLoad->GetX(), GetY() - chLoad->GetY()) > 1000)
		return;

	if (mining::GetRawOreFromLoad(chLoad->GetRaceNum()) == 0)
		return;

	const CItem* pick = GetWear(WEAR_WEAPON);

	if (!pick || pick->GetType() != ITEM_PICK)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;816]");
		return;
	}

	//@fixme432
	//A special mining bot can mine from everywhere with a faked OnClick packet. 
	if (pick->GetSubType() != 0) {
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_CANT_USE_THIS_PICKAXE_ON_THIS_VEIN"));
		return;
	}

	if (DISTANCE_APPROX(GetX() - chLoad->GetX(), GetY() - chLoad->GetY()) > 2500) {
		LogManager::Instance().HackLog("MINING_LOCATION", this);
		return;
	}

	// const auto count = number(5, 15); // Number of moves, 2 seconds per move
	const auto count = number(4, 7); // Number of moves, 2 seconds per move

	// show mining behavior
	TPacketGCDigMotion p{};
	p.header = HEADER_GC_DIG_MOTION;
	p.vid = GetVID();
	p.target_vid = chLoad->GetVID();
	p.count = count;

	PacketAround(&p, sizeof(p));

	m_pkMiningEvent = mining::CreateMiningEvent(this, chLoad, count);
}
// END_OF_MINING

#ifdef ENABLE_CHECK_VALID_FISH_POSITION
bool CHARACTER::IsNearWater() const
{
	if (!GetSectree())
		return false;

	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			if (IS_SET(GetSectree()->GetAttribute(GetX() + x * 100, GetY() + y * 100), ATTR_WATER))
				return true;
		}
	}

	return false;
}
#endif

void CHARACTER::fishing()
{
#ifdef ENABLE_CHECK_VALID_FISH_POSITION
	if (!IsNearWater())
		return;
#endif

#ifdef ENABLE_MULTI_FARM_BLOCK
	if (!GetMultiStatus())
		return;
#endif

	if (m_pkFishingEvent)
	{
		fishing_take();
		return;
	}

	// Attempting to fish in the impotent property?
	{
		LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::Instance().GetMap(GetMapIndex());
		if (!pkSectreeMap)
			return;

		const int x = GetX();
		const int y = GetY();

		LPSECTREE tree = pkSectreeMap->Find(x, y);
		if (!tree)
			return;

		const uint32_t dwAttr = tree->GetAttribute(x, y);

		if (IS_SET(dwAttr, ATTR_BLOCK))
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;1051]");
			return;
		}
	}

	LPITEM rod = GetWear(WEAR_WEAPON);

	// Fishing rod mounting
	if (!rod || rod->GetType() != ITEM_ROD)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;817]");
		return;
	}

	if (0 == rod->GetSocket(2))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;818]");
		return;
	}

	float fx, fy;
	GetDeltaByDegree(GetRotation(), 400.0f, &fx, &fy);

	m_pkFishingEvent = fishing::CreateFishingEvent(this);
}

void CHARACTER::fishing_take()
{
	const CItem* rod = GetWear(WEAR_WEAPON);
	if (rod && rod->GetType() == ITEM_ROD)
	{
		using fishing::fishing_event_info;
		if (m_pkFishingEvent)
		{
			struct fishing_event_info* info = dynamic_cast<struct fishing_event_info*>(m_pkFishingEvent->info);
			if (info)
				fishing::Take(info, this);
		}
	}
	else
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;819]");
	}

	event_cancel(&m_pkFishingEvent);
}

bool CHARACTER::StartStateMachine(int iNextPulse)
{
	if (CHARACTER_MANAGER::Instance().AddToStateList(this))
	{
		m_dwNextStatePulse = thecore_heart->pulse + iNextPulse;
		return true;
	}

	return false;
}

void CHARACTER::StopStateMachine()
{
	CHARACTER_MANAGER::Instance().RemoveFromStateList(this);
}

void CHARACTER::UpdateStateMachine(uint32_t dwPulse)
{
	if (dwPulse < m_dwNextStatePulse)
		return;

	if (IsDead())
		return;

	Update();
	m_dwNextStatePulse = dwPulse + m_dwStateDuration;
}

void CHARACTER::SetNextStatePulse(int iNextPulse)
{
	CHARACTER_MANAGER::Instance().AddToStateList(this);
	m_dwNextStatePulse = iNextPulse;

	if (iNextPulse < 10)
		MonsterLog("Let's go to the next state");
}


// Character instance update function.
void CHARACTER::UpdateCharacter(uint32_t dwPulse)
{
	CFSM::Update();
}

#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
void CHARACTER::BroadcastUpdateLimitedPurchase(uint32_t dwVnum, uint32_t dwLimitedCount, uint32_t dwLimitedPurchaseCount)
{
	TPacketGCShop pack{};
	pack.header = HEADER_GC_SHOP;
	pack.subheader = SHOP_SUBHEADER_GC_UNK_21;
	pack.size = sizeof(TPacketGCShop) + sizeof(TPacketGCShopUpdateLimitedPurchase);

	TPacketGCShopUpdateLimitedPurchase pack2{};
	pack2.vnum = dwVnum;
	pack2.limited_count = dwLimitedCount;
	pack2.limited_purchase_count = dwLimitedPurchaseCount;

	pack.size = sizeof(TPacketGCShop) + sizeof(TPacketGCShopUpdateLimitedPurchase);

	if (GetDesc())
	{
		GetDesc()->BufferedPacket(&pack, sizeof(TPacketGCShop));
		GetDesc()->Packet(&pack2, sizeof(TPacketGCShopUpdateLimitedPurchase));
	}
}

void CHARACTER::SetPurchaseItemLimit(LPCHARACTER ch, uint32_t dwVnum, uint32_t dwLimitedCount, uint32_t dwLimitedPurchaseCount)
{
	if (!ch)
		return;

	const auto dwPlayerID = ch->GetPlayerID();
	const auto it = m_ShopLimitedPurchaseInfo.find(dwPlayerID);
	if (it == m_ShopLimitedPurchaseInfo.end())
	{
		TShopPriceLimitCount::TPurchaseData purchaseData;
		purchaseData.dwLimitedCount = dwLimitedCount;
		purchaseData.dwLimitedPurchaseCount = dwLimitedPurchaseCount;

		TShopPriceLimitCount purchaseInfo;
		purchaseInfo.data.emplace(dwVnum, purchaseData);

		m_ShopLimitedPurchaseInfo.emplace(dwPlayerID, purchaseInfo);
		ch->BroadcastUpdateLimitedPurchase(dwVnum, dwLimitedCount, dwLimitedPurchaseCount);
	}
	else
	{
		const auto it2 = it->second.data.find(dwVnum);
		if (it2 == it->second.data.end())
		{
			TShopPriceLimitCount::TPurchaseData purchaseData;
			purchaseData.dwLimitedCount = dwLimitedCount;
			purchaseData.dwLimitedPurchaseCount = dwLimitedPurchaseCount;

			it->second.data.emplace(dwVnum, purchaseData);

			ch->BroadcastUpdateLimitedPurchase(dwVnum, dwLimitedCount, dwLimitedPurchaseCount);
			return;
		}

		ch->BroadcastUpdateLimitedPurchase(dwVnum, it2->second.dwLimitedCount, it2->second.dwLimitedPurchaseCount);
	}
}

void CHARACTER::ShopPurchaseLimitItem(LPCHARACTER ch, uint32_t dwVnum)
{
	if (!ch)
		return;

	const auto dwPlayerID = ch->GetPlayerID();
	const auto it = m_ShopLimitedPurchaseInfo.find(dwPlayerID);
	if (it != m_ShopLimitedPurchaseInfo.end())
	{
		const auto it2 = it->second.data.find(dwVnum);
		if (it2 != it->second.data.end())
		{
			it2->second.dwLimitedPurchaseCount++;
			ch->BroadcastUpdateLimitedPurchase(dwVnum, it2->second.dwLimitedCount, it2->second.dwLimitedPurchaseCount);
		}
	}
}

bool CHARACTER::CanShopPurchaseLimitedItem(LPCHARACTER ch, uint32_t dwVnum)
{
	if (!ch)
		return false;

	const auto dwPlayerID = ch->GetPlayerID();
	const auto it = m_ShopLimitedPurchaseInfo.find(dwPlayerID);
	if (it != m_ShopLimitedPurchaseInfo.end())
	{
		const auto it2 = it->second.data.find(dwVnum);
		if (it2 != it->second.data.end())
		{
			if (it2->second.dwLimitedPurchaseCount >= it2->second.dwLimitedCount)
				return false;
		}
	}

	return true;
}

uint32_t CHARACTER::GetShopLimitedCount(uint32_t dwPlayerID, uint32_t dwVnum)
{
	const auto it = m_ShopLimitedPurchaseInfo.find(dwPlayerID);
	if (it != m_ShopLimitedPurchaseInfo.end())
	{
		const auto it2 = it->second.data.find(dwVnum);
		if (it2 != it->second.data.end())
			return it2->second.dwLimitedCount;
	}

	return 0;
}

uint32_t CHARACTER::GetShopLimitedPurchaseCount(uint32_t dwPlayerID, uint32_t dwVnum)
{
	const auto it = m_ShopLimitedPurchaseInfo.find(dwPlayerID);
	if (it != m_ShopLimitedPurchaseInfo.end())
	{
		const auto it2 = it->second.data.find(dwVnum);
		if (it2 != it->second.data.end())
			return it2->second.dwLimitedPurchaseCount;
	}

	return 0;
}
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
void CHARACTER::SetViewingShop(LPSHOP pkShop) noexcept
{
	if ((m_pkViewingShop = pkShop))
		SET_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_SHOP);
	else
	{
		REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_SHOP);
		SetViewingShopOwner(nullptr);
	}
}
#else
void CHARACTER::SetShop(LPSHOP pkShop) noexcept
{
	if ((m_pkShop = pkShop))
		SET_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_SHOP);
	else
	{
		REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_SHOP);
		SetShopOwner(nullptr);
	}
}
#endif

void CHARACTER::SetExchange(CExchange* pkExchange) noexcept
{
	m_pkExchange = pkExchange;
#ifdef ENABLE_CHECK_WINDOW_RENEWAL
	SetOpenedWindow(W_EXCHANGE, pkExchange ? true : false);
#endif
}

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
void CHARACTER::SetSoulRoulette(CSoulRoulette* pt)
{
	if (pSoulRoulette)
		delete pSoulRoulette;
	pSoulRoulette = pt;
# ifdef ENABLE_CHECK_WINDOW_RENEWAL
	SetOpenedWindow(W_ROULETTE, pSoulRoulette ? true : false);
# endif
}
#endif

void CHARACTER::SetPart(uint8_t bPartPos, uint32_t dwVal) noexcept	//@fixme479 - uint16_t -> uint32_t
{
	assert(bPartPos < PART_MAX_NUM);
	m_pointsInstant.parts[bPartPos] = dwVal;
}

uint32_t CHARACTER::GetPart(uint8_t bPartPos) const	//@fixme479 - uint16_t -> uint32_t
{
	assert(bPartPos < PART_MAX_NUM);

#ifdef ENABLE_HIDE_COSTUME_SYSTEM
	if (bPartPos == PART_MAIN && GetWear(WEAR_COSTUME_BODY) && IsBodyCostumeHidden())
	{
		if (const LPITEM pArmor = GetWear(WEAR_BODY))
# ifdef ENABLE_CHANGE_LOOK_SYSTEM
			return pArmor->GetChangeLookVnum() != 0 ? pArmor->GetChangeLookVnum() : pArmor->GetVnum();
# else
			return pArmor->GetVnum();
# endif
		else
			return 0;
	}
	else if (bPartPos == PART_HAIR && GetWear(WEAR_COSTUME_HAIR) && IsHairCostumeHidden())
		return 0;
# ifdef ENABLE_ACCE_COSTUME_SYSTEM
	else if (bPartPos == PART_ACCE && GetWear(WEAR_COSTUME_ACCE) && IsAcceCostumeHidden())
		return 0;
# endif
# ifdef ENABLE_AURA_SYSTEM
	else if (bPartPos == PART_AURA && GetWear(WEAR_COSTUME_AURA) && IsAuraCostumeHidden())
		return 0;
# endif
# ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	else if (bPartPos == PART_WEAPON && GetWear(WEAR_COSTUME_WEAPON) && IsWeaponCostumeHidden())
	{
		if (const LPITEM pWeapon = GetWear(WEAR_WEAPON))
# ifdef ENABLE_CHANGE_LOOK_SYSTEM
			return pWeapon->GetChangeLookVnum() != 0 ? pWeapon->GetChangeLookVnum() : pWeapon->GetVnum();
# else
			return pWeapon->GetVnum();
# endif
		else
			return 0;
	}
# endif
#endif

	return m_pointsInstant.parts[bPartPos];
}

uint32_t CHARACTER::GetOriginalPart(uint8_t bPartPos) const	//@fixme479 - uint16_t -> uint32_t
{
	switch (bPartPos)
	{
		case PART_MAIN:
		{
#ifdef ENABLE_HIDE_COSTUME_SYSTEM
			if (GetWear(WEAR_COSTUME_BODY) && IsBodyCostumeHidden())
				if (const LPITEM pArmor = GetWear(WEAR_BODY))
					return pArmor->GetVnum();
#endif

			if (!IsPC())
				return GetPart(PART_MAIN);
			else
				return m_pointsInstant.bBasePart;
		}

		case PART_HAIR:
#ifdef ENABLE_HIDE_COSTUME_SYSTEM
			if (GetWear(WEAR_COSTUME_HAIR) && IsHairCostumeHidden())
				return 0;
#endif
			return GetPart(PART_HAIR);

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		case PART_ACCE:
# ifdef ENABLE_HIDE_COSTUME_SYSTEM
			if (GetWear(WEAR_COSTUME_ACCE) && IsAcceCostumeHidden())
				return 0;
# endif
			return GetPart(PART_ACCE);
#endif

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
		case PART_WEAPON:
# ifdef ENABLE_HIDE_COSTUME_SYSTEM
			if (GetWear(WEAR_COSTUME_WEAPON) && IsWeaponCostumeHidden())
				if (const LPITEM pWeapon = GetWear(WEAR_WEAPON))
					return pWeapon->GetVnum();
# endif
			return GetPart(PART_WEAPON);
#endif

#ifdef ENABLE_PENDANT
		case PART_PENDANT:
			return GetPart(PART_PENDANT);
#endif

#ifdef ENABLE_AURA_SYSTEM
		case PART_AURA:
# ifdef ENABLE_HIDE_COSTUME_SYSTEM
			if (GetWear(WEAR_COSTUME_AURA) && IsAuraCostumeHidden())
				return 0;
# endif
			return GetPart(PART_AURA);
#endif

		default:
			return 0;
	}
}

uint8_t CHARACTER::GetCharType() const noexcept
{
	return m_bCharType;
}

bool CHARACTER::SetSyncOwner(LPCHARACTER ch, bool bRemoveFromList)
{
	// TRENT_MONSTER
	if (IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOMOVE))
		return false;
	// END_OF_TRENT_MONSTER

	if (ch) // @fixme131
	{
		if (!battle_is_attackable(ch, this))
		{
			SendDamagePacket(ch, 0, DAMAGE_BLOCK);
			return false;
		}
	}

	if (ch == this)
	{
		sys_err("SetSyncOwner owner == this (%p)", this);
		return false;
	}

	if (!ch)
	{
		if (bRemoveFromList && m_pkChrSyncOwner)
		{
			m_pkChrSyncOwner->m_kLst_pkChrSyncOwned.remove(this);
		}

		if (m_pkChrSyncOwner)
			sys_log(1, "SyncRelease %s %p from %s", GetName(), this, m_pkChrSyncOwner->GetName());

		// The pointer must be set to NULL even if it is not removed from the list.
		m_pkChrSyncOwner = nullptr;
	}
	else
	{
		if (!IsSyncOwner(ch))
			return false;

		// If the distance is greater than 200, it cannot be a SyncOwner.
		if (DISTANCE_APPROX(GetX() - ch->GetX(), GetY() - ch->GetY()) > 250)
		{
			sys_log(1, "SetSyncOwner distance over than 250 %s %s", GetName(), ch->GetName());

			// In case of SyncOwner, it is displayed as Owner.
			if (m_pkChrSyncOwner == ch)
				return true;

			return false;
		}

		if (m_pkChrSyncOwner != ch)
		{
			if (m_pkChrSyncOwner)
			{
				sys_log(1, "SyncRelease %s %p from %s", GetName(), this, m_pkChrSyncOwner->GetName());
				m_pkChrSyncOwner->m_kLst_pkChrSyncOwned.remove(this);
			}

			m_pkChrSyncOwner = ch;
			m_pkChrSyncOwner->m_kLst_pkChrSyncOwned.emplace_back(this);

			// When SyncOwner changes, LastSyncTime is initialized.
			static const timeval zero_tv = { 0, 0 };
			SetLastSyncTime(zero_tv);

			sys_log(1, "SetSyncOwner set %s %p to %s", GetName(), this, ch->GetName());
		}

		m_fSyncTime = get_float_time();
	}

	/* TODO:
	* Even if the Sync Owner is the same, it continues to send packets,
	* When the synchronized time has elapsed more than 3 seconds, release the packet
	* If you use the sending method, you can reduce the number of packets.
	*/
	TPacketGCOwnership pack{};
	pack.bHeader = HEADER_GC_OWNERSHIP;
	pack.dwOwnerVID = ch ? ch->GetVID() : 0;
	pack.dwVictimVID = GetVID();

	PacketAround(&pack, sizeof(TPacketGCOwnership));
	return true;
}

struct FuncClearSync
{
	void operator() (LPCHARACTER ch)
	{
		if (ch)
			ch->SetSyncOwner(nullptr, false); // If you set the flag false, for_each will work properly.
	}
};

void CHARACTER::ClearSync()
{
	SetSyncOwner(nullptr);

	// In the below for_each, the pointers of those who have me as m_pkChrSyncOwner are set to NULL.
	std::for_each(m_kLst_pkChrSyncOwned.begin(), m_kLst_pkChrSyncOwned.end(), FuncClearSync());
	m_kLst_pkChrSyncOwned.clear();
}

bool CHARACTER::IsSyncOwner(const LPCHARACTER& ch) const noexcept
{
	if (m_pkChrSyncOwner == ch)
		return true;

	// If the last sync time has passed more than 3 seconds, no one has ownership
	// none. So anyone is a SyncOwner, so it returns true
	if (get_float_time() - m_fSyncTime >= 3.0f)
		return true;

	return false;
}

void CHARACTER::SetParty(LPPARTY pkParty)
{
	if (pkParty == m_pkParty)
		return;

	if (pkParty && m_pkParty)
		sys_err("%s is trying to reassigning party (current %p, new party %p)", GetName(), get_pointer(m_pkParty), get_pointer(pkParty));

	sys_log(1, "PARTY set to %p", get_pointer(pkParty));

	if (m_pkDungeon && IsPC() && !pkParty)	//@fixme425
		SetDungeon(nullptr);

	//if (m_pkDungeon && IsPC())
	//	SetDungeon(nullptr);

#ifdef ENABLE_12ZI
	if (m_pkZodiac && IsPC() && !pkParty)
		SetZodiac(nullptr);
#endif

	m_pkParty = pkParty;

	if (IsPC())
	{
		if (m_pkParty)
			SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_PARTY);
		else
			REMOVE_BIT(m_bAddChrState, ADD_CHARACTER_STATE_PARTY);

		UpdatePacket();
	}
}

// PARTY_JOIN_BUG_FIX
/// Party registration event information
EVENTINFO(TPartyJoinEventInfo)
{
	uint32_t dwGuestPID; ///< PID of the character to join the party
	uint32_t dwLeaderPID; ///< PID of the party leader

	TPartyJoinEventInfo() noexcept
		: dwGuestPID(0)
		, dwLeaderPID(0)
	{
	}
};

EVENTFUNC(party_request_event)
{
	const TPartyJoinEventInfo* info = dynamic_cast<TPartyJoinEventInfo*>(event->info);

	if (info == nullptr)
	{
		sys_err("party_request_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(info->dwGuestPID);
	if (ch)
	{
		sys_log(0, "PartyRequestEvent %s", ch->GetName());
		ch->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequestDenied");
		ch->SetPartyRequestEvent(nullptr);
	}

	return 0;
}

bool CHARACTER::RequestToParty(LPCHARACTER leader)
{
	if (leader && leader->GetParty())
		leader = leader->GetParty()->GetLeaderCharacter();

	if (!leader)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;820]");
		return false;
	}

	if (m_pkPartyRequestEvent)
		return false;

	if (!IsPC() || !leader->IsPC())
		return false;

	if (leader->IsBlockMode(BLOCK_PARTY_REQUEST))
		return false;

	const PartyJoinErrCode errcode = IsPartyJoinableCondition(leader, this);

	switch (errcode)
	{
		case PERR_NONE:
			break;

		case PERR_SERVER:
			ChatPacket(CHAT_TYPE_INFO, "[LS;530]");
			return false;

		case PERR_DIFFEMPIRE:
			ChatPacket(CHAT_TYPE_INFO, "[LS;821]");
			return false;

		case PERR_DUNGEON:
			ChatPacket(CHAT_TYPE_INFO, "[LS;822]");
			return false;

		case PERR_OBSERVER:
			ChatPacket(CHAT_TYPE_INFO, "[LS;823]");
			return false;

		case PERR_LVBOUNDARY:
			ChatPacket(CHAT_TYPE_INFO, "[LS;824]");
			return false;

		case PERR_LOWLEVEL:
			ChatPacket(CHAT_TYPE_INFO, "[LS;826]");
			return false;

		case PERR_HILEVEL:
			ChatPacket(CHAT_TYPE_INFO, "[LS;827]");
			return false;

		case PERR_ALREADYJOIN:
			return false;

		case PERR_PARTYISFULL:
			ChatPacket(CHAT_TYPE_INFO, "[LS;828]");
			return false;

		default:
			sys_err("Do not process party join error(%d)", errcode);
			return false;
	}

	TPartyJoinEventInfo* info = AllocEventInfo<TPartyJoinEventInfo>();

	info->dwGuestPID = GetPlayerID();
	info->dwLeaderPID = leader->GetPlayerID();

	SetPartyRequestEvent(event_create(party_request_event, info, PASSES_PER_SEC(10)));

#ifdef WJ_NEW_USER_CARE
	leader->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequest %u %s", (uint32_t)GetVID(), GetName());
#else
	leader->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequest %u", (uint32_t)GetVID());
#endif
	ChatPacket(CHAT_TYPE_INFO, "[LS;829;%s]", leader->GetName());
	return true;
}

void CHARACTER::DenyToParty(LPCHARACTER member)
{
	if (!member)
		return;

	sys_log(1, "DenyToParty %s member %s %p", GetName(), member->GetName(), get_pointer(member->m_pkPartyRequestEvent));

	if (!member->m_pkPartyRequestEvent)
		return;

	const TPartyJoinEventInfo* info = dynamic_cast<TPartyJoinEventInfo*>(member->m_pkPartyRequestEvent->info);

	if (!info)
	{
		sys_err("CHARACTER::DenyToParty> <Factor> Null pointer");
		return;
	}

	if (info->dwGuestPID != member->GetPlayerID())
		return;

	if (info->dwLeaderPID != GetPlayerID())
		return;

	event_cancel(&member->m_pkPartyRequestEvent);

	member->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequestDenied");
}

void CHARACTER::AcceptToParty(LPCHARACTER member)
{
	if (!member)
		return;

	sys_log(1, "AcceptToParty %s member %s %p", GetName(), member->GetName(), get_pointer(member->m_pkPartyRequestEvent));

	if (!member->m_pkPartyRequestEvent)
		return;

	const TPartyJoinEventInfo* info = dynamic_cast<TPartyJoinEventInfo*>(member->m_pkPartyRequestEvent->info);

	if (!info)
	{
		sys_err("CHARACTER::AcceptToParty> <Factor> Null pointer");
		return;
	}

	if (info->dwGuestPID != member->GetPlayerID())
		return;

	if (info->dwLeaderPID != GetPlayerID())
		return;

	event_cancel(&member->m_pkPartyRequestEvent);

	if (!GetParty())
		member->ChatPacket(CHAT_TYPE_INFO, "[LS;830]");
	else
	{
		if (GetPlayerID() != GetParty()->GetLeaderPID())
			return;

		PartyJoinErrCode errcode = IsPartyJoinableCondition(this, member);
		switch (errcode)
		{
			case PERR_NONE:
				member->PartyJoin(this);
				return;

			case PERR_SERVER:
				member->ChatPacket(CHAT_TYPE_INFO, "[LS;530]");
				break;

			case PERR_DUNGEON:
				member->ChatPacket(CHAT_TYPE_INFO, "[LS;822]");
				break;

			case PERR_OBSERVER:
				member->ChatPacket(CHAT_TYPE_INFO, "[LS;823]");
				break;

			case PERR_LVBOUNDARY:
				member->ChatPacket(CHAT_TYPE_INFO, "[LS;824]");
				break;

			case PERR_LOWLEVEL:
				member->ChatPacket(CHAT_TYPE_INFO, "[LS;826]");
				break;

			case PERR_HILEVEL:
				member->ChatPacket(CHAT_TYPE_INFO, "[LS;827]");
				break;

			case PERR_ALREADYJOIN:
				break;

			case PERR_PARTYISFULL:
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;828]");
				member->ChatPacket(CHAT_TYPE_INFO, "[LS;831]");
				break;
			}

			default:
				sys_err("Do not process party join error(%d)", errcode);
		}
	}

	member->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequestDenied");
}

/**
* Party invitation event callback function.
* When an event is triggered, it is treated as an invitation rejection.
*/
EVENTFUNC(party_invite_event)
{
	const TPartyJoinEventInfo* pInfo = dynamic_cast<TPartyJoinEventInfo*>(event->info);

	if (pInfo == nullptr)
	{
		sys_err("party_invite_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER pchInviter = CHARACTER_MANAGER::Instance().FindByPID(pInfo->dwLeaderPID);
	if (pchInviter)
	{
		sys_log(1, "PartyInviteEvent %s", pchInviter->GetName());
		pchInviter->PartyInviteDeny(pInfo->dwGuestPID);
	}

	return 0;
}

void CHARACTER::PartyInvite(LPCHARACTER pchInvitee)
{
	if (!pchInvitee)
		return;

	if (GetParty() && GetParty()->GetLeaderPID() != GetPlayerID())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;832]");
		return;
	}
	else if (pchInvitee->IsBlockMode(BLOCK_PARTY_INVITE))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;833;%s]", pchInvitee->GetName());
		return;
	}

	const PartyJoinErrCode errcode = IsPartyJoinableCondition(this, pchInvitee);

	switch (errcode)
	{
		case PERR_NONE:
			break;

		case PERR_SERVER:
			ChatPacket(CHAT_TYPE_INFO, "[LS;530]");
			return;

		case PERR_DIFFEMPIRE:
			ChatPacket(CHAT_TYPE_INFO, "[LS;821]");
			return;

		case PERR_DUNGEON:
			ChatPacket(CHAT_TYPE_INFO, "[LS;822]");
			return;

		case PERR_OBSERVER:
			ChatPacket(CHAT_TYPE_INFO, "[LS;823]");
			return;

		case PERR_LVBOUNDARY:
			ChatPacket(CHAT_TYPE_INFO, "[LS;824]");
			return;

		case PERR_LOWLEVEL:
			ChatPacket(CHAT_TYPE_INFO, "[LS;826]");
			return;

		case PERR_HILEVEL:
			ChatPacket(CHAT_TYPE_INFO, "[LS;827]");
			return;

		case PERR_ALREADYJOIN:
			ChatPacket(CHAT_TYPE_INFO, "[LS;834;%s]", pchInvitee->GetName());
			return;

		case PERR_PARTYISFULL:
			ChatPacket(CHAT_TYPE_INFO, "[LS;828]");
			return;

		default:
			sys_err("Do not process party join error(%d)", errcode);
			return;
	}

	if (m_PartyInviteEventMap.end() != m_PartyInviteEventMap.find(pchInvitee->GetPlayerID()))
		return;

	//
	// Add event to EventMap
	//
	TPartyJoinEventInfo* info = AllocEventInfo<TPartyJoinEventInfo>();

	info->dwGuestPID = pchInvitee->GetPlayerID();
	info->dwLeaderPID = GetPlayerID();

	m_PartyInviteEventMap.insert(EventMap::value_type(pchInvitee->GetPlayerID(), event_create(party_invite_event, info, PASSES_PER_SEC(10))));

	//
	// Send invitation packet to invited character
	//

	TPacketGCPartyInvite p{};
	p.header = HEADER_GC_PARTY_INVITE;
	p.leader_vid = GetVID();
	pchInvitee->GetDesc()->Packet(&p, sizeof(p));
}

#ifdef ENABLE_GROUP_DAMAGE_WEAPON
struct FPartyCHCollector
{
	std::vector <LPCHARACTER> vecCHPtr; FPartyCHCollector() { }

	void operator () (LPCHARACTER ch)
	{
		vecCHPtr.emplace_back(ch);
	}
};
#endif

void CHARACTER::PartyInviteAccept(LPCHARACTER pchInvitee)
{
	if (!pchInvitee)
		return;

	const EventMap::iterator itFind = m_PartyInviteEventMap.find(pchInvitee->GetPlayerID());

	if (itFind == m_PartyInviteEventMap.end())
	{
		sys_log(1, "PartyInviteAccept from not invited character(%s)", pchInvitee->GetName());
		return;
	}

	event_cancel(&itFind->second);
	m_PartyInviteEventMap.erase(itFind);

	if (GetParty() && GetParty()->GetLeaderPID() != GetPlayerID())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;832]");
		return;
	}

	const PartyJoinErrCode errcode = IsPartyJoinableMutableCondition(this, pchInvitee);

	switch (errcode)
	{
		case PERR_NONE:
			break;

		case PERR_SERVER:
			pchInvitee->ChatPacket(CHAT_TYPE_INFO, "[LS;530]");
			return;

		case PERR_DUNGEON:
			pchInvitee->ChatPacket(CHAT_TYPE_INFO, "[LS;835]");
			return;

		case PERR_OBSERVER:
			pchInvitee->ChatPacket(CHAT_TYPE_INFO, "[LS;823]");
			return;

		case PERR_LVBOUNDARY:
			pchInvitee->ChatPacket(CHAT_TYPE_INFO, "[LS;824]");
			return;

		case PERR_LOWLEVEL:
			pchInvitee->ChatPacket(CHAT_TYPE_INFO, "[LS;826]");
			return;

		case PERR_HILEVEL:
			pchInvitee->ChatPacket(CHAT_TYPE_INFO, "[LS;827]");
			return;

		case PERR_ALREADYJOIN:
			pchInvitee->ChatPacket(CHAT_TYPE_INFO, "[LS;837]");
			return;

		case PERR_PARTYISFULL:
			ChatPacket(CHAT_TYPE_INFO, "[LS;828]");
			pchInvitee->ChatPacket(CHAT_TYPE_INFO, "[LS;831]");
			return;

		default:
			sys_err("ignore party join error(%d)", errcode);
			return;
	}

	//
	// party join processing
	//

	if (GetParty())
		pchInvitee->PartyJoin(this);
	else
	{
		LPPARTY pParty = CPartyManager::Instance().CreateParty(this);
		if (pParty)
		{
			pParty->Join(pchInvitee->GetPlayerID());
			pParty->Link(pchInvitee);
			pParty->SendPartyInfoAllToOne(this);
		}
	}
}

void CHARACTER::PartyInviteDeny(uint32_t dwPID)
{
	const EventMap::iterator itFind = m_PartyInviteEventMap.find(dwPID);

	if (itFind == m_PartyInviteEventMap.end())
	{
		sys_log(1, "PartyInviteDeny to not exist event(inviter PID: %d, invitee PID: %d)", GetPlayerID(), dwPID);
		return;
	}

	event_cancel(&itFind->second);
	m_PartyInviteEventMap.erase(itFind);

	const LPCHARACTER& pchInvitee = CHARACTER_MANAGER::Instance().FindByPID(dwPID);
	if (pchInvitee)
		ChatPacket(CHAT_TYPE_INFO, "[LS;838;%s]", pchInvitee->GetName());
}

void CHARACTER::PartyJoin(LPCHARACTER pLeader)
{
	if (!pLeader)
		return;

	pLeader->ChatPacket(CHAT_TYPE_INFO, "[LS;839;%s]", GetName());
	ChatPacket(CHAT_TYPE_INFO, "[LS;839;%s]", pLeader->GetName());

	pLeader->GetParty()->Join(GetPlayerID());
	pLeader->GetParty()->Link(this);

#ifdef ENABLE_GROUP_DAMAGE_WEAPON
	FPartyCHCollector f;

	pLeader->GetParty()->ForEachOnMapMember(f, pLeader->GetParty()->GetLeaderCharacter()->GetMapIndex());
	int cnt = 0;
	for (std::vector <LPCHARACTER>::iterator it = f.vecCHPtr.begin(); it != f.vecCHPtr.end(); ++it) {	//@fixme541
		LPCHARACTER partyMember = *it;
		if (!partyMember)
			continue;
		LPITEM eqpdweapon = partyMember->GetWear(WEAR_WEAPON);
		cnt += (eqpdweapon && IS_SET(eqpdweapon->GetFlag(), ITEM_FLAG_GROUP_DMG_WEAPON) ? 1 : 0);
	//	cnt += (eqpdweapon && eqpdweapon->GetVnum() == 149 ? 1 : 0);
		/*pLeader->ChatPacket(7, "[Party Join] ITEM_FLAG_GROUP_WEAPON WORKS!");*/
	}
	
	for (std::vector <LPCHARACTER>::iterator it = f.vecCHPtr.begin(); it != f.vecCHPtr.end(); ++it) {	//@fixme541
		LPCHARACTER partyMember = *it;
		if (!partyMember)
			continue;
		LPITEM eqpdweapon = partyMember->GetWear(WEAR_WEAPON);
		if (eqpdweapon) {
			eqpdweapon->SetForceAttribute(5, 0, (cnt == 3 ? 3 : 0));
			/*partyMember->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Party Join] You earned %d Group-Points"), (cnt == 3 ? 3 : 0));
			ChatPacket(7, "GrpWpnPoints: %d", (cnt == 3 ? 3 : 0));*/
		}
	}
#endif
}

CHARACTER::PartyJoinErrCode CHARACTER::IsPartyJoinableCondition(const LPCHARACTER pchLeader, const LPCHARACTER pchGuest)
{
	if (pchLeader && pchGuest)
	{
		if (pchLeader->GetEmpire() != pchGuest->GetEmpire())
			return PERR_DIFFEMPIRE;

		return IsPartyJoinableMutableCondition(pchLeader, pchGuest);
	}

	return PERR_NONE;
}

static bool __party_can_join_by_level(LPCHARACTER leader, LPCHARACTER quest)
{
	if (leader && quest)
	{
		uint8_t level_limit = PARTY_CAN_JOIN_MAX_LEVEL;
		return (abs(leader->GetLevel() - quest->GetLevel()) <= level_limit);
	}

	return false;
}

CHARACTER::PartyJoinErrCode CHARACTER::IsPartyJoinableMutableCondition(const LPCHARACTER pchLeader, const LPCHARACTER pchGuest)
{
	if (pchLeader && pchGuest)
	{
		if (!CPartyManager::Instance().IsEnablePCParty())
			return PERR_SERVER;
		else if (pchLeader->GetDungeon()
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
			|| (pchLeader->GetMeleyLair() || CMeleyLairManager::Instance().IsMeleyMap(pchLeader->GetMapIndex()))
#endif
			)
			return PERR_DUNGEON;
		else if (pchGuest->IsObserverMode())
			return PERR_OBSERVER;
		else if (false == __party_can_join_by_level(pchLeader, pchGuest))
			return PERR_LVBOUNDARY;
		else if (pchGuest->GetParty())
			return PERR_ALREADYJOIN;
		else if (pchLeader->GetParty())
		{
			if (pchLeader->GetParty()->GetMemberCount() == PARTY_MAX_MEMBER)
				return PERR_PARTYISFULL;
		}
	}

	return PERR_NONE;
}
// END_OF_PARTY_JOIN_BUG_FIX

void CHARACTER::SetDungeon(LPDUNGEON pkDungeon)
{
	if (pkDungeon && m_pkDungeon)
		sys_err("%s is trying to reassigning dungeon (current %p, new party %p)", GetName(), get_pointer(m_pkDungeon), get_pointer(pkDungeon));

	if (m_pkDungeon == pkDungeon)
		return;

	if (m_pkDungeon)
	{
		if (IsPC())
		{
			if (GetParty())
				m_pkDungeon->DecPartyMember(GetParty(), this);
			else
				m_pkDungeon->DecMember(this);
		}
		else if (IsMonster() || IsStone())
		{
			m_pkDungeon->DecMonster();
		}
	}

	m_pkDungeon = pkDungeon;

	if (pkDungeon)
	{
		sys_log(0, "%s DUNGEON set to %p, PARTY is %p", GetName(), get_pointer(pkDungeon), get_pointer(m_pkParty));

		if (IsPC())
		{
			if (GetParty())
				m_pkDungeon->IncPartyMember(GetParty(), this);
			else
				m_pkDungeon->IncMember(this);
		}
		else if (IsMonster() || IsStone())
		{
			m_pkDungeon->IncMonster();
		}
	}
}

#ifdef ENABLE_12ZI
void CHARACTER::SetZodiac(LPZODIAC pkZodiac)
{
	if (pkZodiac && m_pkZodiac)
		sys_err("%s is trying to reassigning zodiac (current %p, new party %p)", GetName(), get_pointer(m_pkZodiac), get_pointer(pkZodiac));

	if (m_pkZodiac == pkZodiac)
	{
		return;
	}

	if (m_pkZodiac)
	{
		if (IsPC())
		{
			if (GetParty())
				m_pkZodiac->DecPartyMember(GetParty(), this);
			else
				m_pkZodiac->DecMember(this);
		}
		else if (IsMonster() || IsStone())
		{
			m_pkZodiac->DecMonster();
		}
	}

	m_pkZodiac = pkZodiac;

	if (pkZodiac)
	{
		sys_log(0, "%s ZODIAC set to %p, PARTY is %p", GetName(), get_pointer(pkZodiac), get_pointer(m_pkParty));

		if (IsPC())
		{
			if (GetParty())
				m_pkZodiac->IncPartyMember(GetParty(), this);
			else
				m_pkZodiac->IncMember(this);
		}
		else if (IsMonster() || IsStone())
		{
			m_pkZodiac->IncMonster();
		}
	}
}
#endif

void CHARACTER::SetWarMap(CWarMap* pWarMap)
{
	if (m_pWarMap)
		m_pWarMap->DecMember(this);

	m_pWarMap = pWarMap;

	if (m_pWarMap)
		m_pWarMap->IncMember(this);
}

void CHARACTER::SetWeddingMap(marriage::WeddingMap* pMap)
{
	if (m_pWeddingMap)
		m_pWeddingMap->DecMember(this);

	m_pWeddingMap = pMap;

	if (m_pWeddingMap)
		m_pWeddingMap->IncMember(this);
}

void CHARACTER::SetRegen(LPREGEN pkRegen)
{
	m_pkRegen = pkRegen;
	if (pkRegen != nullptr)
		regen_id_ = pkRegen->id;

	m_fRegenAngle = GetRotation();
	m_posRegen = GetXYZ();
}

bool CHARACTER::OnIdle() noexcept
{
	return false;
}

void CHARACTER::OnMove(bool bIsAttack)
{
	m_dwLastMoveTime = get_dword_time();

	if (bIsAttack)
	{
		m_dwLastAttackTime = m_dwLastMoveTime;

		if (IsAffectFlag(AFF_REVIVE_INVISIBLE))
			RemoveAffect(AFFECT_REVIVE_INVISIBLE);

		if (IsAffectFlag(AFF_EUNHYUNG))
		{
			RemoveAffect(SKILL_EUNHYUNG);
			SetAffectedEunhyung();
		}
		else
		{
			ClearAffectedEunhyung();
		}

		/*if (IsAffectFlag(AFF_JEONSIN))
			RemoveAffect(SKILL_JEONSINBANGEO);*/
	}

	/*if (IsAffectFlag(AFF_GUNGON))
		RemoveAffect(SKILL_GUNGON);*/

	// MINING
	mining_cancel();
	// END_OF_MINING
}

void CHARACTER::OnClick(LPCHARACTER pkChrCauser)
{
	if (!pkChrCauser)
	{
		sys_err("OnClick %s by nullptr", GetName());
		return;
	}

	const uint32_t vid = GetVID();
	sys_log(0, "OnClick %s[vnum %d ServerUniqueID %d, pid %d] by %s", GetName(), GetRaceNum(), vid, GetPlayerID(), pkChrCauser->GetName());

#ifdef ENABLE_12ZI
	if (pkChrCauser != this && IsPC() && pkChrCauser->IsPC() && IsPolymorphed() && (CZodiacManager::Instance().IsZiStageMapIndex(GetMapIndex())) && (CZodiacManager::Instance().IsZiStageMapIndex(pkChrCauser->GetMapIndex())))
	{
		if (GetPolymorphVnum() >= 20452 && GetPolymorphVnum() <= 20463)
		{
			SetPolymorph(0);
			return;
		}
	}
#endif

#ifdef ENABLE_QUEEN_NETHIS
	if ((IsNPC()) && (GetRaceNum() == (uint16_t)(SnakeLair::PORTAL_VNUM)) && (SnakeLair::CSnk::instance().IsSnakeMap(pkChrCauser->GetMapIndex())))
	{
		SnakeLair::CSnk::instance().Start(pkChrCauser);
		return;
	}
#endif

#ifndef ENABLE_PREMIUM_PRIVATE_SHOP
	// You cannot proceed with the quest with the shop open.
	{
		// However, you can click your own store.
		if (pkChrCauser->GetMyShop() && pkChrCauser != this)
		{
			sys_err("OnClick Fail (%s->%s) - pc has shop", pkChrCauser->GetName(), GetName());
			return;
		}
	}
#endif

	// You cannot proceed with the quest while in exchange.
	{
		if (pkChrCauser->GetExchange())
		{
			sys_err("OnClick Fail (%s->%s) - pc is exchanging", pkChrCauser->GetName(), GetName());
			return;
		}
	}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	//If what the player clicked is a shop...
	if (IsShop())
	{
		if (IsEditingShop())
		{
			pkChrCauser->ChatPacket(CHAT_TYPE_INFO, "[LS;1714]");
			return;
		}

		//Dead players can't trade
		if (pkChrCauser->IsDead() == true)
			return;

		//If clicker has safebox open, a npc shop open, or cube open, that probably breaks things,
		//so... you can't open a shop.
		if (pkChrCauser->GetOpenedWindow(W_SAFEBOX | W_SHOP_OWNER | W_CUBE | W_EXCHANGE
#ifdef ENABLE_SKILLBOOK_COMBINATION
			| W_SKILLBOOK_COMB
#endif
#ifdef ENABLE_ATTR_6TH_7TH
			| W_ATTR_6TH_7TH
#endif
#ifdef ENABLE_AURA_SYSTEM
			| W_AURA
#endif
#ifdef ENABLE_SWITCHBOT
			| W_SWITCHBOT
#endif
		))
		{
			pkChrCauser->ChatPacket(CHAT_TYPE_INFO, "[LS;1757]");
			return;
		}

#	ifndef ENABLE_OPEN_SHOP_WITH_PASSWORD
		//Hey, it's our own shop! Say no more
		if (GetPlayerID() == pkChrCauser->GetPlayerID())
		{
			TPacketGCShop pack{};

			pack.header = HEADER_GC_SHOP;
			pack.subheader = SHOP_SUBHEADER_GC_OPEN_SHOP_EDITOR;
			pack.size = sizeof(TPacketGCShop);

			pkChrCauser->GetDesc()->Packet(&pack, sizeof(pack));
			return;
		}
#	endif

		if (pkChrCauser->GetViewingShop())
		{
			pkChrCauser->GetViewingShop()->RemoveGuest(pkChrCauser);
			pkChrCauser->SetViewingShop(nullptr);
		}

		GetMyShop()->AddGuest(pkChrCauser, GetVID(), false);
		pkChrCauser->SetViewingShopOwner(this);
		return;
	}
#endif

	if (IsPC())
	{
		// If it is set as a target, clicks by PC are also processed as quests.
		if (!CTargetManager::Instance().GetTargetInfo(pkChrCauser->GetPlayerID(), TARGET_TYPE_VID, GetVID()))
		{
#ifndef ENABLE_PREMIUM_PRIVATE_SHOP
			// 2005.03.17.myevan. If it is not the target, the personal shop processing function is activated.
			if (GetMyShop())
			{
				if (pkChrCauser->IsDead()) return;

				//PREVENT_TRADE_WINDOW
				if (pkChrCauser == this) // self is possible
				{
					if (GetOpenedWindow(W_EXCHANGE | W_SAFEBOX | W_SHOP_OWNER | W_CUBE))
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, "[LS;841]");
						return;
					}
				}
				else // when someone clicks
				{
					// Impossible if the person who clicked is using exchange/warehouse/personal store/shop
					if (pkChrCauser->GetOpenedWindow(W_EXCHANGE | W_SAFEBOX | W_MYSHOP | W_SHOP_OWNER | W_CUBE))
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, "[LS;841]");
						return;
					}

					// Not possible if the clicked target is using exchange/warehouse/shop
					//if (GetOpenedWindow(W_EXCHANGE | W_SAFEBOX | W_SHOP_OWNER))
					if (GetOpenedWindow(W_EXCHANGE | W_SAFEBOX | W_CUBE))
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, "[LS;842]");
						return;
					}
				}
				//END_PREVENT_TRADE_WINDOW

				if (pkChrCauser->GetShop())
				{
					pkChrCauser->GetShop()->RemoveGuest(pkChrCauser);
					pkChrCauser->SetShop(nullptr);
				}

				GetMyShop()->AddGuest(pkChrCauser, GetVID(), false);
				pkChrCauser->SetShopOwner(this);
				return;
			}
#endif

			if (test_server)
				sys_err("%s.OnClickFailure(%s) - target is PC", pkChrCauser->GetName(), GetName());

			return;
		}
	}

	// Youth can't do quests
	if (g_bChinaIntoxicationCheck)
	{
		if (pkChrCauser->IsOverTime(OT_3HOUR))
		{
			sys_log(0, "Teen OverTime : name = %s, hour = %d)", pkChrCauser->GetName(), 3);
			return;
		}
		else if (pkChrCauser->IsOverTime(OT_5HOUR))
		{
			sys_log(0, "Teen OverTime : name = %s, hour = %d)", pkChrCauser->GetName(), 5);
			return;
		}
	}

#ifdef ENABLE_EVENT_MANAGER
	if (CEventManager::Instance().GetEventState(CEventManager::EVENT_TYPE_HIDE_AND_SEEK))
	{
		int iTargetVID = quest::CQuestManager::Instance().GetEventFlag("hide_seek_vid");
		if (iTargetVID == vid)
		{
			// Continue with next round only if the reward has been received
			if (LPITEM item = pkChrCauser->AutoGiveItem(CEventManager::HIDE_AND_SEEK_REWARD_VNUM, CEventManager::HIDE_AND_SEEK_REWARD_COUNT))
			{
				int iPosition = number(0, CEventManager::HIDE_AND_SEEK_POSITIONS - 1);
				int iRound = CEventManager::Instance().GetHideAndSeekRound();

				/*
					If the npc is not located on this core, send a packet to others.
					NPC will be destroyed in the HideAndSeekNPC.
				*/

				if (!CEventManager::Instance().HideAndSeekNPC(iPosition, iRound))
				{
					TPacketGGEventHideAndSeek packet;
					packet.bHeader = HEADER_GG_EVENT_HIDE_AND_SEEK;
					packet.iPosition = iPosition;
					packet.iRound = iRound;

					P2P_MANAGER::Instance().Send(&packet, sizeof(packet));
				}

				return;
			}
		}
	}
#endif

	pkChrCauser->SetQuestNPCID(GetVID());

	if (quest::CQuestManager::Instance().Click(pkChrCauser->GetPlayerID(), this))
	{
		return;
	}


	// Perform NPC-only functions: open a shop, etc.
	if (!IsPC())
	{
		if (!m_triggerOnClick.pFunc)
		{
			// View NPC trigger system log
			/*
			sys_err("%s.OnClickFailure(%s) : triggerOnClick.pFunc is EMPTY(pid=%d)",
				pkChrCauser->GetName(),
				GetName(),
				pkChrCauser->GetPlayerID());
			*/
			return;
		}

		m_triggerOnClick.pFunc(this, pkChrCauser);
	}

}

uint8_t CHARACTER::GetGMLevel() const noexcept
{
	if (test_server)
		return GM_IMPLEMENTOR;

	return m_pointsInstant.gm_level;
}

void CHARACTER::SetGMLevel()
{
	if (GetDesc())
	{
		m_pointsInstant.gm_level = gm_get_level(GetName(), GetDesc()->GetHostName(), GetDesc()->GetAccountTable().login);
	}
	else
	{
		m_pointsInstant.gm_level = GM_PLAYER;
	}
}

bool CHARACTER::IsGM() const noexcept
{
	if (IsStrippedPowers())	//@custom025
		return false;

	if (m_pointsInstant.gm_level != GM_PLAYER)
		return true;

	if (test_server)
		return true;

	return false;
}

#ifdef ENABLE_VIP_SYSTEM
BOOL CHARACTER::IsVIP() const
{
	if (m_pointsInstant.gm_level == GM_VIP)
		return true;

	return false;
}
#endif

#ifdef ENABLE_GM_BLOCK
bool CHARACTER::GetStaffPermissions() const noexcept
{
	auto staff_permissions = static_cast<int>(quest::CQuestManager::Instance().GetEventFlag("staff_permissions"));
	if (m_pointsInstant.gm_level >= GM_IMPLEMENTOR || staff_permissions == 2101)
		return true;
	return false;
}
#endif

void CHARACTER::SetStone(LPCHARACTER pkChrStone)
{
	m_pkChrStone = pkChrStone;

	if (m_pkChrStone)
	{
		if (pkChrStone->m_set_pkChrSpawnedBy.find(this) == pkChrStone->m_set_pkChrSpawnedBy.end())
			pkChrStone->m_set_pkChrSpawnedBy.insert(this);
	}
}

struct FuncDeadSpawnedByStone
{
	void operator () (LPCHARACTER ch)
	{
		if (ch)
		{
			ch->SetStone(nullptr);
			ch->Dead(nullptr);
		}
	}
};

void CHARACTER::ClearStone()
{
	if (!m_set_pkChrSpawnedBy.empty())
	{
		// Kill all monsters I spawned.
		FuncDeadSpawnedByStone f;
		std::for_each(m_set_pkChrSpawnedBy.begin(), m_set_pkChrSpawnedBy.end(), f);
		m_set_pkChrSpawnedBy.clear();
	}

	if (!m_pkChrStone)
		return;

	m_pkChrStone->m_set_pkChrSpawnedBy.erase(this);
	m_pkChrStone = nullptr;
}

void CHARACTER::ClearTarget()
{
	if (m_pkChrTarget)
	{
		m_pkChrTarget->m_set_pkChrTargetedBy.erase(this);
		m_pkChrTarget = nullptr;
	}

	TPacketGCTarget p{};

	p.header = HEADER_GC_TARGET;
	p.dwVID = 0;
#ifdef ENABLE_VIEW_TARGET_DECIMAL_HP
	p.icurHP = 0;
	p.iMaxHP = 0;
#else
	p.bHPPercent = 0;
#endif
#ifdef ENABLE_TARGET_AFFECT_SHOWER
	memset(p.affects, 0, sizeof(p.affects));
#endif
#if defined(ENABLE_PENDANT) || defined(ENABLE_ELEMENT_ADD)
	memset(&p.cElementalFlags[0], 0, sizeof(p.cElementalFlags));
#endif

	CHARACTER_SET::iterator it = m_set_pkChrTargetedBy.begin();

	while (it != m_set_pkChrTargetedBy.end())
	{
		LPCHARACTER pkChr = *(it++);
		if (pkChr)
		{
			pkChr->m_pkChrTarget = nullptr;

			if (!pkChr->GetDesc())
			{
				sys_err("%s %p does not have desc", pkChr->GetName(), get_pointer(pkChr));
				abort();
			}

			pkChr->GetDesc()->Packet(&p, sizeof(TPacketGCTarget));
		}
	}

	m_set_pkChrTargetedBy.clear();
}

void CHARACTER::SetTarget(LPCHARACTER pkChrTarget)
{
	if (m_pkChrTarget == pkChrTarget)
		return;

	// CASTLE
	if (IS_CASTLE_MAP(GetMapIndex()) && !IsGM())
		return;
	// CASTLE

	if (m_pkChrTarget)
		m_pkChrTarget->m_set_pkChrTargetedBy.erase(this);

	m_pkChrTarget = pkChrTarget;

	TPacketGCTarget p{};
	p.header = HEADER_GC_TARGET;

	if (m_pkChrTarget)
	{
		m_pkChrTarget->m_set_pkChrTargetedBy.insert(this);

		p.dwVID = m_pkChrTarget->GetVID();

#if defined(ENABLE_PENDANT) || defined(ENABLE_ELEMENT_ADD)
		if (m_pkChrTarget->IsPC())
		{
			if (m_pkChrTarget->GetPoint(POINT_ENCHANT_ELECT))
				p.cElementalFlags[MOB_ELEMENTAL_ELEC] = m_pkChrTarget->GetPoint(POINT_ENCHANT_ELECT);
			if (m_pkChrTarget->GetPoint(POINT_ENCHANT_FIRE))
				p.cElementalFlags[MOB_ELEMENTAL_FIRE] = m_pkChrTarget->GetPoint(POINT_ENCHANT_FIRE);
			if (m_pkChrTarget->GetPoint(POINT_ENCHANT_ICE))
				p.cElementalFlags[MOB_ELEMENTAL_ICE] = m_pkChrTarget->GetPoint(POINT_ENCHANT_ICE);
			if (m_pkChrTarget->GetPoint(POINT_ENCHANT_WIND))
				p.cElementalFlags[MOB_ELEMENTAL_WIND] = m_pkChrTarget->GetPoint(POINT_ENCHANT_WIND);
			if (m_pkChrTarget->GetPoint(POINT_ENCHANT_EARTH))
				p.cElementalFlags[MOB_ELEMENTAL_EARTH] = m_pkChrTarget->GetPoint(POINT_ENCHANT_EARTH);
			if (m_pkChrTarget->GetPoint(POINT_ENCHANT_DARK))
				p.cElementalFlags[MOB_ELEMENTAL_DARK] = m_pkChrTarget->GetPoint(POINT_ENCHANT_DARK);
		}
#endif

#ifdef ENABLE_VIEW_TARGET_DECIMAL_HP
		if ((m_pkChrTarget->GetMaxHP() <= 0))
		{
			p.icurHP = 0;
			p.iMaxHP = 0;
		}
		else if (m_pkChrTarget->IsPC() && !m_pkChrTarget->IsPolymorphed())
		{
			p.icurHP = m_pkChrTarget->GetHP();
			p.iMaxHP = m_pkChrTarget->GetMaxHP();
		}
#else
		if ((m_pkChrTarget->IsPC() && !m_pkChrTarget->IsPolymorphed()) || (m_pkChrTarget->GetMaxHP() <= 0))
		{
			p.bHPPercent = 0;
		}
#endif
		else
		{
#ifdef ENABLE_PROTO_RENEWAL
			if (m_bCharType == CHAR_TYPE_HORSE)
#else
			if (m_pkChrTarget->GetRaceNum() == 20101 ||
				m_pkChrTarget->GetRaceNum() == 20102 ||
				m_pkChrTarget->GetRaceNum() == 20103 ||
				m_pkChrTarget->GetRaceNum() == 20104 ||
				m_pkChrTarget->GetRaceNum() == 20105 ||
				m_pkChrTarget->GetRaceNum() == 20106 ||
				m_pkChrTarget->GetRaceNum() == 20107 ||
				m_pkChrTarget->GetRaceNum() == 20108 ||
				m_pkChrTarget->GetRaceNum() == 20109)
#endif
			{
				const LPCHARACTER& owner = m_pkChrTarget->GetVictim();

				if (owner)
				{
#ifndef ENABLE_VIEW_TARGET_DECIMAL_HP
					const auto iHorseHealth = owner->GetHorseHealth();
#endif
					const auto iHorseMaxHealth = owner->GetHorseMaxHealth();

					if (iHorseMaxHealth)
					{
#ifdef ENABLE_VIEW_TARGET_DECIMAL_HP
						p.icurHP = MINMAX(0, m_pkChrTarget->GetHP(), m_pkChrTarget->GetMaxHP());
						p.iMaxHP = m_pkChrTarget->GetMaxHP();
#else
						p.bHPPercent = MINMAX(0, iHorseHealth * 100 / iHorseMaxHealth, 100);
#endif
					}
					else
					{
#ifdef ENABLE_VIEW_TARGET_DECIMAL_HP
						p.icurHP = MINMAX(0, m_pkChrTarget->GetHP(), m_pkChrTarget->GetMaxHP());
						p.iMaxHP = m_pkChrTarget->GetMaxHP();
#else
						p.bHPPercent = 100;
#endif
					}
				}
				else
				{
#ifdef ENABLE_VIEW_TARGET_DECIMAL_HP
					p.icurHP = MINMAX(0, m_pkChrTarget->GetHP(), m_pkChrTarget->GetMaxHP());
					p.iMaxHP = m_pkChrTarget->GetMaxHP();
#else
					p.bHPPercent = 100;
#endif
				}
			}
			else
			{
				if (m_pkChrTarget->GetMaxHP() <= 0) // @fixme136
				{
#ifdef ENABLE_VIEW_TARGET_DECIMAL_HP
					p.icurHP = 0;
					p.iMaxHP = 0;
#else
					p.bHPPercent = 0;
#endif
				}
				else
				{
#ifdef ENABLE_VIEW_TARGET_DECIMAL_HP
					p.icurHP = MINMAX(0, m_pkChrTarget->GetHP(), m_pkChrTarget->GetMaxHP());
					p.iMaxHP = m_pkChrTarget->GetMaxHP();
#else
					p.bHPPercent = MINMAX(0, (m_pkChrTarget->GetHP() * 100) / m_pkChrTarget->GetMaxHP(), 100);
#endif
				}
			}
		}
	}
	else
	{
		p.dwVID = 0;
#ifdef ENABLE_VIEW_TARGET_DECIMAL_HP
		p.icurHP = 0;
		p.iMaxHP = 0;
#else
		p.bHPPercent = 0;
#endif
	}

#ifdef ENABLE_TARGET_AFFECT_SHOWER
	if (m_pkChrTarget)
	{
		if (IsMonster() || IsStone() || IsPC())
		{
			memset(p.affects, 0, sizeof(p.affects));

			for (uint8_t i = 0; i < TARGET_AFFECT_MAX_NUM; ++i)
			{
				const auto affectNum = g_arTargetAffects[i][0];
				const CAffect* pkAffect = m_pkChrTarget->FindAffect(affectNum);

				if (pkAffect)
				{
					TargetAffectInfo info{};

					info.dwAffectNum = g_arTargetAffects[i][1] - 1;
					info.lDuration = pkAffect->lDuration;

					p.affects[i] = info;
				}
			}
		}
	}
#endif

	GetDesc()->Packet(&p, sizeof(TPacketGCTarget));
}

void CHARACTER::BroadcastTargetPacket()
{
	if (m_set_pkChrTargetedBy.empty())
		return;

	TPacketGCTarget p{};
	p.header = HEADER_GC_TARGET;
	p.dwVID = GetVID();
#ifdef ENABLE_TARGET_AFFECT_SHOWER
	memset(p.affects, 0, sizeof(p.affects));
#endif

	if (IsPC())
	{
#ifdef ENABLE_VIEW_TARGET_DECIMAL_HP
		p.icurHP = MINMAX(0, GetHP(), GetMaxHP());
		p.iMaxHP = GetMaxHP();
#else
		p.bHPPercent = 0;
#endif
	}
	else if (GetMaxHP() <= 0)	// @fixme136
	{
#ifdef ENABLE_VIEW_TARGET_DECIMAL_HP
		p.icurHP = 0;
		p.iMaxHP = 0;
#else
		p.bHPPercent = 0;
#endif
	}
	else
	{
#ifdef ENABLE_VIEW_TARGET_DECIMAL_HP
		p.icurHP = MINMAX(0, GetHP(), GetMaxHP());
		p.iMaxHP = GetMaxHP();
#else
		p.bHPPercent = MINMAX(0, (GetHP() * 100) / GetMaxHP(), 100);
#endif
	}

#if defined(ENABLE_PENDANT) || defined(ENABLE_ELEMENT)
	if (IsPC())
	{
		if (GetPoint(POINT_ENCHANT_ELECT))
			p.cElementalFlags[MOB_ELEMENTAL_ELEC] = GetPoint(POINT_ENCHANT_ELECT);
		if (GetPoint(POINT_ENCHANT_FIRE))
			p.cElementalFlags[MOB_ELEMENTAL_FIRE] = GetPoint(POINT_ENCHANT_FIRE);
		if (GetPoint(POINT_ENCHANT_ICE))
			p.cElementalFlags[MOB_ELEMENTAL_ICE] = GetPoint(POINT_ENCHANT_ICE);
		if (GetPoint(POINT_ENCHANT_WIND))
			p.cElementalFlags[MOB_ELEMENTAL_WIND] = GetPoint(POINT_ENCHANT_WIND);
		if (GetPoint(POINT_ENCHANT_EARTH))
			p.cElementalFlags[MOB_ELEMENTAL_EARTH] = GetPoint(POINT_ENCHANT_EARTH);
		if (GetPoint(POINT_ENCHANT_DARK))
			p.cElementalFlags[MOB_ELEMENTAL_DARK] = GetPoint(POINT_ENCHANT_DARK);
	}
#endif

#ifdef ENABLE_TARGET_AFFECT_SHOWER
	if (IsMonster() || IsStone() || IsPC())
	{
		memset(p.affects, 0, sizeof(p.affects));

		for (int i = 0; i < TARGET_AFFECT_MAX_NUM; ++i)
		{
			uint16_t affectNum = g_arTargetAffects[i][0];
			CAffect* pkAffect = FindAffect(affectNum);

			if (pkAffect)
			{
				TargetAffectInfo info{};

				info.dwAffectNum = g_arTargetAffects[i][1] - 1;
				info.lDuration = pkAffect->lDuration;

				p.affects[i] = info;
			}
		}
	}
#endif

	CHARACTER_SET::iterator it = m_set_pkChrTargetedBy.begin();

	while (it != m_set_pkChrTargetedBy.end())
	{
		LPCHARACTER pkChr = *it++;
		if (pkChr)
		{
			if (!pkChr->GetDesc())
			{
				sys_err("%s %p does not have desc", pkChr->GetName(), get_pointer(pkChr));
				abort();
			}

			pkChr->GetDesc()->Packet(&p, sizeof(TPacketGCTarget));
		}
	}
}

void CHARACTER::CheckTarget()
{
	if (!m_pkChrTarget)
		return;

	if (DISTANCE_APPROX(GetX() - m_pkChrTarget->GetX(), GetY() - m_pkChrTarget->GetY()) >= 4800)
		SetTarget(nullptr);
}

void CHARACTER::SetWarpLocation(long lMapIndex, long x, long y) noexcept
{
	m_posWarp.x = x * 100;
	m_posWarp.y = y * 100;
	m_lWarpMapIndex = lMapIndex;
}

void CHARACTER::SaveExitLocation()
{
	m_posExit = GetXYZ();
	m_lExitMapIndex = GetMapIndex();
}

void CHARACTER::ExitToSavedLocation()
{
	sys_log(0, "ExitToSavedLocation");
	WarpSet(m_posWarp.x, m_posWarp.y, m_lWarpMapIndex);

	m_posExit.x = m_posExit.y = m_posExit.z = 0;
	m_lExitMapIndex = 0;
}

// fixme
// So far, we have externally checked if privateMapIndex is equal to the current map index,
// Otherwise, warpset is called
// Let's put this into the warpset.
#ifdef ENABLE_MOVE_CHANNEL
bool CHARACTER::WarpSet(long x, long y, long lPrivateMapIndex, long lCustomAddr, uint16_t wCustomPort)
#else
bool CHARACTER::WarpSet(long x, long y, long lPrivateMapIndex)
#endif
{
	if (!IsPC())
		return false;

	long lAddr;
	long lMapIndex;
	uint16_t wPort;

	if (!CMapLocation::Instance().Get(x, y, lMapIndex, lAddr, wPort))
	{
		sys_err("cannot find map location index %d x %d y %d name %s", lMapIndex, x, y, GetName());
		return false;
	}

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	if (GetGuild() && IsOpenGuildstorage())
		CloseGuildstorage();
#endif

	// Send Supplementary Data Block if new map requires security packages in loading this map
	{
		long lCurAddr = 0;
		long lCurMapIndex = 0;
		uint16_t wCurPort;

		CMapLocation::Instance().Get(GetX(), GetY(), lCurMapIndex, lCurAddr, wCurPort);

		// do not send SDB files if char is in the same map
		if (lCurMapIndex != lMapIndex)
		{
			const TMapRegion* rMapRgn = SECTREE_MANAGER::Instance().GetMapRegion(lMapIndex);
			{
				DESC_MANAGER::Instance().SendClientPackageSDBToLoadMap(GetDesc(), rMapRgn->strMapName.c_str());
			}
		}
	}

	if (lPrivateMapIndex >= 10000)
	{
		if (lPrivateMapIndex / 10000 != lMapIndex)
		{
			sys_err("Invalid map index %d, must be child of %d", lPrivateMapIndex, lMapIndex);
			return false;
		}

		lMapIndex = lPrivateMapIndex;
	}

	Stop();
	Save();

	if (GetSectree())
	{
		GetSectree()->RemoveEntity(this);
		ViewCleanup();

		EncodeRemovePacket(this);
	}

	m_lWarpMapIndex = lMapIndex;
	m_posWarp.x = x;
	m_posWarp.y = y;

	sys_log(0, "WarpSet %s %d %d current map %d target map %d", GetName(), x, y, GetMapIndex(), lMapIndex);
#ifdef ENABLE_LOADING_TIP
	ChatPacket(CHAT_TYPE_COMMAND, "target_map %ld", lMapIndex);
#endif

	TPacketGCWarp p{};
	p.bHeader = HEADER_GC_WARP;
	p.lX = x;
	p.lY = y;
#ifdef ENABLE_MOVE_CHANNEL
	p.lAddr = lCustomAddr ? lCustomAddr : lAddr;
#	ifdef ENABLE_PROXY_IP
	if (!g_stProxyIP.empty())
		p.lAddr = inet_addr(g_stProxyIP.c_str());
#	endif
	p.wPort = wCustomPort ? wCustomPort : wPort;
#else
	p.lAddr = lAddr;
#	ifdef ENABLE_PROXY_IP
	if (!g_stProxyIP.empty())
		p.lAddr = inet_addr(g_stProxyIP.c_str());
#	endif
	p.wPort = wPort;
#endif

#ifdef ENABLE_SWITCHBOT
	CSwitchbotManager::Instance().SetIsWarping(GetPlayerID(), true);

	if (p.wPort != mother_port)
	{
		CSwitchbotManager::Instance().P2PSendSwitchbot(GetPlayerID(), p.wPort);
	}
#endif

	GetDesc()->Packet(&p, sizeof(TPacketGCWarp));

	char buf[256];
	snprintf(buf, sizeof(buf), "%s MapIdx %ld DestMapIdx%ld DestX%ld DestY%ld Empire%d", GetName(), GetMapIndex(), lPrivateMapIndex, x, y, GetEmpire());
	LogManager::Instance().CharLog(this, 0, "WARP", buf);

	return true;
}

void CHARACTER::WarpEnd()
{
	if (test_server)
		sys_log(0, "WarpEnd %s", GetName());

	if (m_posWarp.x == 0 && m_posWarp.y == 0)
		return;

	int index = m_lWarpMapIndex;

	if (index > 10000)
		index /= 10000;

	if (!map_allow_find(index))
	{
		// You can't warp to this place, so let's go back to the coordinates before warping.
		sys_err("location %d %d not allowed to login this server", m_posWarp.x, m_posWarp.y);
#ifdef ENABLE_GOHOME_IF_MAP_NOT_ALLOWED
		GoHome();
#else
		GetDesc()->SetPhase(PHASE_CLOSE);
#endif
		return;
	}

	sys_log(0, "WarpEnd %s %d %u %u", GetName(), m_lWarpMapIndex, m_posWarp.x, m_posWarp.y);

	Show(m_lWarpMapIndex, m_posWarp.x, m_posWarp.y, 0);
	Stop();

	m_lWarpMapIndex = 0;
	m_posWarp.x = m_posWarp.y = m_posWarp.z = 0;

	{
		// P2P Login
		TPacketGGLogin p{};
		p.bHeader = HEADER_GG_LOGIN;
		strlcpy(p.szName, GetName(), sizeof(p.szName));
		p.dwPID = GetPlayerID();
		p.bEmpire = GetEmpire();
		p.lMapIndex = SECTREE_MANAGER::Instance().GetMapIndex(GetX(), GetY());
		p.bChannel = g_bChannel;
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		p.bLanguage = GetDesc()->GetLanguage();
#endif

		P2P_MANAGER::Instance().Send(&p, sizeof(TPacketGGLogin));
	}
}

bool CHARACTER::Return()
{
	if (!IsNPC())
		return false;

	int x, y;
	/*
	float fDist = DISTANCE_SQRT(m_pkMobData->m_posLastAttacked.x - GetX(), m_pkMobData->m_posLastAttacked.y - GetY());
	float fx, fy;
	GetDeltaByDegree(GetRotation(), fDist, &fx, &fy);
	x = GetX() + (int) fx;
	y = GetY() + (int) fy;
	*/
	SetVictim(nullptr);

	x = m_pkMobInst->m_posLastAttacked.x;
	y = m_pkMobInst->m_posLastAttacked.y;

	SetRotationToXY(x, y);

	if (!Goto(x, y))
		return false;

	SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

	if (test_server)
		sys_log(0, "%s %p Let's give up and go back! %d %d", GetName(), this, x, y);

	if (GetParty())
		GetParty()->SendMessage(this, PM_RETURN, x, y);

	return true;
}

bool CHARACTER::Follow(LPCHARACTER pkChr, float fMinDistance)
{
	if (IsPC())
	{
		sys_err("CHARACTER::Follow : PC cannot use this method", GetName());
		return false;
	}

	// TRENT_MONSTER
	if (IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOMOVE))
	{
		if (pkChr->IsPC()) // When the person you are following is a PC
		{
			// If i'm in a party. I must obey party leader's AI.
			if (!GetParty() || !GetParty()->GetLeader() || GetParty()->GetLeader() == this)
			{
				if (get_dword_time() - m_pkMobInst->m_dwLastAttackedTime >= 15000) // 15 seconds have passed since the last attack
				{
					// If it is more than 50 meters from the last hit, give up and go back.
					if (m_pkMobData->m_table.wAttackRange < DISTANCE_APPROX(pkChr->GetX() - GetX(), pkChr->GetY() - GetY()))
					{
						if (Return())
							return true;
					}
				}
			}
		}
		return false;
	}
	// END_OF_TRENT_MONSTER

	long x = pkChr->GetX();
	long y = pkChr->GetY();

	if (pkChr->IsPC()) // When the person you are following is a PC
	{
		// If i'm in a party. I must obey party leader's AI.
		if (!GetParty() || !GetParty()->GetLeader() || GetParty()->GetLeader() == this)
		{
			if (get_dword_time() - m_pkMobInst->m_dwLastAttackedTime >= 15000) // 15 seconds have passed since the last attack
			{
				// If it is more than 50 meters from the last hit, give up and go back.
				if (5000 < DISTANCE_APPROX(m_pkMobInst->m_posLastAttacked.x - GetX(), m_pkMobInst->m_posLastAttacked.y - GetY()))
					if (Return())
						return true;
			}
		}
	}

	if (IsGuardNPC())
	{
		if (5000 < DISTANCE_APPROX(m_pkMobInst->m_posLastAttacked.x - GetX(), m_pkMobInst->m_posLastAttacked.y - GetY()))
		{
			if (Return())
				return true;
		}
	}

#ifdef ENABLE_AGGROMOB_SAFEZONE_SKIP
	SECTREE *tree = pkChr->GetSectree();
	if (tree && tree->IsAttr(pkChr->GetX(), pkChr->GetY(), ATTR_BANPK)) {
		if (Return())
			return true;
	}
#endif

	if (pkChr->IsState(pkChr->m_stateMove) &&
		GetMobBattleType() != BATTLE_TYPE_RANGE &&
		GetMobBattleType() != BATTLE_TYPE_MAGIC &&
		false == IsPet()
#ifdef ENABLE_GROWTH_PET_SYSTEM
		&& false == IsGrowthPet()
		&& false == IsPetPay()
#endif
		)
	{
		// If the target is moving, predictive movement
		// After predicting the time to meet from the speed difference and distance between me and the other party
		// Assume that the opponent moves in a straight line up to that time and moves there.
		const float rot = pkChr->GetRotation();
		const float rot_delta = GetDegreeDelta(rot, GetDegreeFromPositionXY(GetX(), GetY(), pkChr->GetX(), pkChr->GetY()));

		const float yourSpeed = pkChr->GetMoveSpeed();
		const float mySpeed = GetMoveSpeed();

		const float fDist = DISTANCE_SQRT(x - GetX(), y - GetY());
		const float fFollowSpeed = mySpeed - yourSpeed * cos(rot_delta * M_PI / 180);

		if (fFollowSpeed >= 0.1f)
		{
			const float fMeetTime = fDist / fFollowSpeed;
			float fYourMoveEstimateX = 0.0f, fYourMoveEstimateY = 0.0f;

			if (fMeetTime * yourSpeed <= 100000.0f)
			{
				GetDeltaByDegree(pkChr->GetRotation(), fMeetTime * yourSpeed, &fYourMoveEstimateX, &fYourMoveEstimateY);

				x += static_cast<long>(fYourMoveEstimateX);
				y += static_cast<long>(fYourMoveEstimateY);

				const float fDistNew = sqrt((static_cast<double>(x) - GetX()) * (x - GetX()) + (static_cast<double>(y) - GetY()) * (y - GetY()));
				if (fDist < fDistNew)
				{
					x = static_cast<long>(GetX() + (x - GetX()) * fDist / fDistNew);
					y = static_cast<long>(GetY() + (y - GetY()) * fDist / fDistNew);
				}
			}
		}
	}

	// You have to look at where you want to go.
	SetRotationToXY(x, y);

	const float fDist = DISTANCE_SQRT(x - GetX(), y - GetY());

	if (fDist <= fMinDistance)
		return false;

	float fx = 0.0f, fy = 0.0f;

	if (IsChangeAttackPosition(pkChr) && GetMobRank() < MOB_RANK_BOSS)
	{
		// Move to a random place around the opponent
		SetChangeAttackPositionTime();

		int retry = 16;
		int dx = 0, dy = 0;
		const int rot = static_cast<int>(GetDegreeFromPositionXY(x, y, GetX(), GetY()));

		while (--retry)
		{
			if (fDist < 500.0f)
				GetDeltaByDegree((rot + number(-90, 90) + number(-90, 90)) % 360, fMinDistance, &fx, &fy);
			else
				GetDeltaByDegree(number(0, 359), fMinDistance, &fx, &fy);

			dx = x + static_cast<int>(fx);
			dy = y + static_cast<int>(fy);

			LPSECTREE tree = SECTREE_MANAGER::Instance().Get(GetMapIndex(), dx, dy);
			if (nullptr == tree)
				break;

			if (0 == (tree->GetAttribute(dx, dy) & (ATTR_BLOCK | ATTR_OBJECT)))
				break;
		}

		//sys_log(0, "Go somewhere nearby %s retry %d", GetName(), retry);
		if (!Goto(dx, dy))
			return false;
	}
	else
	{
		// Follow a straight line
		const float fDistToGo = fDist - fMinDistance;
		GetDeltaByDegree(GetRotation(), fDistToGo, &fx, &fy);

		//sys_log(0, "Go straight %s", GetName());
		if (!Goto(GetX() + static_cast<int>(fx), GetY() + static_cast<int>(fy)))
			return false;
	}

	SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);
	//MonsterLog("Follow; %s", pkChr->GetName());
	return true;
}

float CHARACTER::GetDistanceFromSafeboxOpen() const
{
	return DISTANCE_APPROX(GetX() - m_posSafeboxOpen.x, GetY() - m_posSafeboxOpen.y);
}

void CHARACTER::SetSafeboxOpenPosition()
{
	m_posSafeboxOpen = GetXYZ();
}

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
float CHARACTER::GetDistanceFromGuildstorageOpen() const
{
	return DISTANCE_APPROX(GetX() - m_posGuildstorageOpen.x, GetY() - m_posGuildstorageOpen.y);
}

void CHARACTER::SetGuildstorageOpenPosition()
{
	m_posGuildstorageOpen = GetXYZ();
}
#endif

CSafebox* CHARACTER::GetSafebox() const noexcept
{
	return m_pkSafebox;
}

void CHARACTER::ReqSafeboxLoad(const char* pszPassword)
{
	if (!*pszPassword)
		return;

	if (!*pszPassword || strlen(pszPassword) > SAFEBOX_PASSWORD_MAX_LEN)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;526]");
		return;
	}
	else if (m_pkSafebox)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;527]");
		return;
	}

#if defined(ENABLE_GROWTH_PET_SYSTEM) && defined(ENABLE_PET_ATTR_DETERMINE)
	if (IsGrowthPetDetermineWindow())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1344]");
		return;
	}
#endif

#ifdef ENABLE_PULSE_MANAGER
	if (!PulseManager::Instance().IncreaseClock(GetPlayerID(), ePulse::SafeboxLoad, std::chrono::milliseconds(1000)))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;528]");
		return;
	}
#endif

	else if (GetDistanceFromSafeboxOpen() > 1000)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;529]");
		return;
	}
	else if (m_bOpeningSafebox)
	{
		sys_log(0, "Overlapped safebox load request from %s", GetName());
		return;
	}

	SetSafeboxLoadTime();
	m_bOpeningSafebox = true;

	TSafeboxLoadPacket p{};
	p.dwID = GetDesc()->GetAccountTable().id;
	strlcpy(p.szLogin, GetDesc()->GetAccountTable().login, sizeof(p.szLogin));
	strlcpy(p.szPassword, pszPassword, sizeof(p.szPassword));

	db_clientdesc->DBPacket(HEADER_GD_SAFEBOX_LOAD, GetDesc()->GetHandle(), &p, sizeof(p));
}

#ifdef ENABLE_SAFEBOX_MONEY
void CHARACTER::LoadSafebox(int iSize, uint32_t dwGold, int iItemCount, TPlayerItem* pItems)
#else
void CHARACTER::LoadSafebox(int iSize, int iItemCount, TPlayerItem* pItems)
#endif
{
	bool bLoaded = false;

	//PREVENT_TRADE_WINDOW
	SetOpenSafebox(true);
	//END_PREVENT_TRADE_WINDOW

	if (m_pkSafebox)
		bLoaded = true;

	if (!m_pkSafebox)
#ifdef ENABLE_SAFEBOX_MONEY
		m_pkSafebox = M2_NEW CSafebox(this, iSize, dwGold);
#else
		m_pkSafebox = M2_NEW CSafebox(this, iSize);
#endif
	else
		m_pkSafebox->ChangeSize(iSize);

	m_iSafeboxSize = iSize;

	TPacketGCSafeboxSize p{};
	p.bHeader = HEADER_GC_SAFEBOX_SIZE;
	p.bSize = iSize;

	GetDesc()->Packet(&p, sizeof(TPacketGCSafeboxSize));

	if (!bLoaded)
	{
		for (int i = 0; i < iItemCount; ++i, ++pItems)
		{
			if (!m_pkSafebox->IsValidPosition(pItems->pos))
				continue;

			LPITEM item = ITEM_MANAGER::Instance().CreateItem(pItems->vnum, pItems->count, pItems->id);

			if (!item)
			{
				sys_err("cannot create item vnum %d id %u (name: %s)", pItems->vnum, pItems->id, GetName());
				continue;
			}

			// Following same order than player.item table
			item->SetSkipSave(true);
			item->SetSockets(pItems->alSockets);
			item->SetAttributes(pItems->aAttr);
#ifdef ENABLE_YOHARA_SYSTEM
			item->SetRandomAttrs(pItems->aApplyRandom);
			item->SetRandomDefaultAttrs(pItems->alRandomValues);
#endif
#ifdef ENABLE_SEALBIND_SYSTEM
			item->SetSealDate(pItems->nSealDate);
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			item->SetChangeLookVnum(pItems->dwTransmutationVnum);
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
			item->SetBasic(pItems->is_basic);
#endif
#ifdef ENABLE_REFINE_ELEMENT
			item->SetElement(pItems->grade_element, pItems->attack_element, pItems->element_type_bonus, pItems->elements_value_bonus);
#endif
#ifdef ENABLE_SET_ITEM
			item->SetItemSetValue(pItems->set_value);
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
			item->SetGrowthPetItemInfo(pItems->aPetInfo);

			if ((item->GetType() == ITEM_PET) && ((item->GetSubType() == PET_UPBRINGING) || (item->GetSubType() == PET_BAG)))
			{
				const long lPetDuration = (item->GetSocket(0) - get_global_time()) / 60;
				if ((lPetDuration <= 0) && (item->GetSubType() == PET_UPBRINGING))
					ChatPacket(CHAT_TYPE_INFO, "[LS;1149;[IN;%d]]", item->GetVnum());

				SetGrowthPetInfo(pItems->aPetInfo);
				if (test_server)
					sys_err("%d %d %d %d", pItems->owner, pItems->aPetInfo.pet_id, pItems->aPetInfo.exp_monster, pItems->aPetInfo.exp_item);
			}
#endif

			if (!m_pkSafebox->Add(pItems->pos, item))
				M2_DESTROY_ITEM(item);
			else
				item->SetSkipSave(false);
		}

#ifdef ENABLE_GROWTH_PET_SYSTEM
		SendGrowthPetInfoPacket();
#endif
	}
}

void CHARACTER::ChangeSafeboxSize(uint8_t bSize)
{
	//if (!m_pkSafebox)
	//	return;

	TPacketGCSafeboxSize p{};

	p.bHeader = HEADER_GC_SAFEBOX_SIZE;
	p.bSize = bSize;

	GetDesc()->Packet(&p, sizeof(TPacketGCSafeboxSize));

	if (m_pkSafebox)
		m_pkSafebox->ChangeSize(bSize);

	m_iSafeboxSize = bSize;
}

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
void CHARACTER::ReqGuildstorageLoad()
{
	CGuild* pGuild = GetGuild();
	if (!pGuild) {
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("GUILDSTORAGE_NO_GUILD!"));
		return;// no guild ptr fix
	}

	if (pGuild->GetGuildstorage() == 0) {
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("GUILDSTORAGE_HAVE_TO_BUY"));
		return;
	}

	if (GetExchange() || GetMyShop() || 
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		GetViewingShopOwner() || 
#else
		GetShopOwner() || 
#endif
#ifdef ENABLE_MAILBOX
		GetMailBox() ||
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		IsChangeLookWindowOpen() ||
#endif
		IsOpenSafebox() || IsCubeOpen()) {
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANT_OPEN_GUILDSTORAGE_BECAUSE_OF_WINDOWS"));
		return;
	}

#	ifdef ENABLE_GUILDRENEWAL_SYSTEM
	const TGuildMember* m = pGuild->GetMember(GetPlayerID());
	if (!pGuild->HasGradeAuth(m->grade, GUILD_AUTH_BANK)) {
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("GUILD_BANK_PERMIT"));
		return;
	}
#	endif

	if (pGuild->IsStorageOpen())
	{
		LPCHARACTER pChar = CHARACTER_MANAGER::Instance().FindByPID(pGuild->GetStoragePid());
		if (pChar) {
			pChar->ChatPacket(7, "player %s want to open storage.", GetName());
			ChatPacket(CHAT_TYPE_INFO, "guild is already open by %s", pChar->GetName());
		}
		else
			ChatPacket(CHAT_TYPE_INFO, "guild is already open");
		return;
	}

#if defined(ENABLE_GROWTH_PET_SYSTEM) && defined(ENABLE_PET_ATTR_DETERMINE)
	if (IsGrowthPetDetermineWindow())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1344]");
		return;
	}
#endif

	if (GetGuildstorage())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;527]");
		return;
	}

	if (!PulseManager::Instance().IncreaseClock(GetPlayerID(), ePulse::SafeboxLoad, std::chrono::milliseconds(1000)))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;528]");
		return;
	}
	else if (GetDistanceFromGuildstorageOpen() > 1000)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;529]");
		return;
	}
	else if (m_bOpeningGuildstorage)
	{
		sys_log(0, "Overlapped guildstorage load request from %s", GetName());
		return;
	}

	SetGuildstorageLoadTime();
	m_bOpeningGuildstorage = true;

	TSafeboxLoadPacket p;
	p.dwID = GetGuild() ? GetGuild()->GetID() : 0;
	strlcpy(p.szLogin, GetDesc()->GetAccountTable().login, sizeof(p.szLogin));
	strlcpy(p.szPassword, "000000", sizeof(p.szPassword));

	if (!p.dwID)
		return;

	db_clientdesc->DBPacket(HEADER_GD_GUILDSTORAGE_LOAD, GetDesc()->GetHandle(), &p, sizeof(p));
	pGuild->SetStorageState(true, GetPlayerID());
}

void CHARACTER::ChangeGuildstorageSize(uint8_t bSize)
{
	TPacketGCGuildstorageOpen p{};

	p.bHeader = HEADER_GC_GUILDSTORAGE_OPEN;
	p.bSize = bSize;

	GetDesc()->Packet(&p, sizeof(TPacketGCGuildstorageOpen));

	if (m_pkGuildstorage)
		m_pkGuildstorage->ChangeSize(bSize);

	m_iSafeboxSize = bSize;
}
#endif

void CHARACTER::CloseSafebox()
{
	if (!m_pkSafebox)
		return;

	//PREVENT_TRADE_WINDOW
	SetOpenSafebox(false);
	//END_PREVENT_TRADE_WINDOW

#ifdef ENABLE_SAFEBOX_MONEY
	m_pkSafebox->Save();
#endif

	M2_DELETE(m_pkSafebox);
	m_pkSafebox = nullptr;

	ChatPacket(CHAT_TYPE_COMMAND, "CloseSafebox");

	SetSafeboxLoadTime();
	m_bOpeningSafebox = false;

	Save();
}

CSafebox* CHARACTER::GetMall() const noexcept
{
	return m_pkMall;
}

void CHARACTER::LoadMall(int iItemCount, TPlayerItem* pItems)
{
	bool bLoaded = false;

	if (m_pkMall)
		bLoaded = true;

	if (!m_pkMall)
#ifdef ENABLE_SAFEBOX_MONEY
		m_pkMall = M2_NEW CSafebox(this, 3 * SAFEBOX_PAGE_SIZE, 0);
#else
		m_pkMall = M2_NEW CSafebox(this, 3 * SAFEBOX_PAGE_SIZE);
#endif
	else
		m_pkMall->ChangeSize(3 * SAFEBOX_PAGE_SIZE);

	m_pkMall->SetWindowMode(MALL);

	TPacketGCMallOpen p{};

	p.bHeader = HEADER_GC_MALL_OPEN;
	p.bSize = 3 * SAFEBOX_PAGE_SIZE;

	GetDesc()->Packet(&p, sizeof(TPacketGCMallOpen));

	if (!bLoaded)
	{
		for (int i = 0; i < iItemCount; ++i, ++pItems)
		{
			if (!m_pkMall->IsValidPosition(pItems->pos))
				continue;

			LPITEM item = ITEM_MANAGER::Instance().CreateItem(pItems->vnum, pItems->count, pItems->id);

			if (!item)
			{
				sys_err("cannot create item vnum %d id %u (name: %s)", pItems->vnum, pItems->id, GetName());
				continue;
			}

			item->SetSkipSave(true);
			item->SetSockets(pItems->alSockets);
			item->SetAttributes(pItems->aAttr);
#ifdef ENABLE_YOHARA_SYSTEM
			item->SetRandomAttrs(pItems->aApplyRandom);
			item->SetRandomDefaultAttrs(pItems->alRandomValues);
#endif
#ifdef ENABLE_SEALBIND_SYSTEM
			item->SetSealDate(pItems->nSealDate);
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			item->SetChangeLookVnum(pItems->dwTransmutationVnum);
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
			item->SetBasic(pItems->is_basic);
#endif
#ifdef ENABLE_REFINE_ELEMENT
			item->SetElement(pItems->grade_element, pItems->attack_element, pItems->element_type_bonus, pItems->elements_value_bonus);
#endif
#ifdef ENABLE_SET_ITEM
			item->SetItemSetValue(pItems->set_value);
#endif

			if (!m_pkMall->Add(pItems->pos, item))
				M2_DESTROY_ITEM(item);
			else
				item->SetSkipSave(false);
		}
	}
}

void CHARACTER::CloseMall()
{
	if (!m_pkMall)
		return;

#ifdef ENABLE_SAFEBOX_MONEY
	m_pkMall->Save();
#endif

	M2_DELETE(m_pkMall);
	m_pkMall = nullptr;

	ChatPacket(CHAT_TYPE_COMMAND, "CloseMall");
}

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
CSafebox* CHARACTER::GetGuildstorage() const
{
	return m_pkGuildstorage;
}

void CHARACTER::LoadGuildstorage(int iSize, int iItemCount, TPlayerItem* pItems)
{
	bool bLoaded = false;

	//PREVENT_TRADE_WINDOW
	SetOpenGuildstorage(true);
	//END_PREVENT_TRADE_WINDOW

	if (m_pkGuildstorage)
		bLoaded = true;

	if (!m_pkGuildstorage)
#ifdef ENABLE_SAFEBOX_MONEY
		m_pkGuildstorage = M2_NEW CSafebox(this, iSize, 0);
#else
		m_pkGuildstorage = M2_NEW CSafebox(this, iSize);
#endif
	else
		m_pkGuildstorage->ChangeSize(iSize);

	m_iGuildstorageSize = iSize;

	m_pkGuildstorage->SetWindowMode(GUILDBANK);

	TPacketGCGuildstorageOpen p{};

	p.bHeader = HEADER_GC_GUILDSTORAGE_OPEN;
	p.bSize = iSize;

	GetDesc()->Packet(&p, sizeof(TPacketGCGuildstorageOpen));

	if (!bLoaded)
	{
		for (int i = 0; i < iItemCount; ++i, ++pItems)
		{
			if (!m_pkGuildstorage->IsValidPosition(pItems->pos))
				continue;

			LPITEM item = ITEM_MANAGER::Instance().CreateItem(pItems->vnum, pItems->count, pItems->id);

			if (!item)
			{
				sys_err("cannot create item vnum %d id %u (name: %s)", pItems->vnum, pItems->id, GetName());
				continue;
			}

			item->SetSkipSave(true);
			item->SetSockets(pItems->alSockets);
			item->SetAttributes(pItems->aAttr);
#ifdef ENABLE_YOHARA_SYSTEM
			item->SetRandomAttrs(pItems->aApplyRandom);
			item->SetRandomDefaultAttrs(pItems->alRandomValues);
#endif
#ifdef ENABLE_SEALBIND_SYSTEM
			item->SetSealDate(pItems->nSealDate);
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			item->SetChangeLookVnum(pItems->dwTransmutationVnum);
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
			item->SetBasic(pItems->is_basic);
#endif
#ifdef ENABLE_REFINE_ELEMENT
			item->SetElement(pItems->grade_element, pItems->attack_element, pItems->element_type_bonus, pItems->elements_value_bonus);
#endif
#ifdef ENABLE_SET_ITEM
			item->SetItemSetValue(pItems->set_value);
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
			item->SetGrowthPetItemInfo(pItems->aPetInfo);

			if ((item->GetType() == ITEM_PET) && ((item->GetSubType() == PET_UPBRINGING) || (item->GetSubType() == PET_BAG)))
			{
				SetGrowthPetInfo(pItems->aPetInfo);
				//sys_err("GuildBank: %d %d %d %d", pItems->owner, pItems->aPetInfo.pet_id, pItems->aPetInfo.exp_monster, pItems->aPetInfo.exp_item);
			}
#endif

			if (!m_pkGuildstorage->Add(pItems->pos, item))
				M2_DESTROY_ITEM(item);
			else
				item->SetSkipSave(false);
		}

#ifdef ENABLE_GROWTH_PET_SYSTEM
		SendGrowthPetInfoPacket();
#endif
	}
}

void CHARACTER::CloseGuildstorage()
{
	if (!m_pkGuildstorage)
		return;

	//PREVENT_TRADE_WINDOW
	SetOpenGuildstorage(false);
	GetGuild()->SetStorageState(false, 0);
	//END_PREVENT_TRADE_WINDOW

#ifdef ENABLE_SAFEBOX_MONEY
	m_pkGuildstorage->Save();
#endif

	M2_DELETE(m_pkGuildstorage);
	m_pkGuildstorage = nullptr;

	ChatPacket(CHAT_TYPE_COMMAND, "CloseGuildstorage");

	SetGuildstorageLoadTime();
	m_bOpeningGuildstorage = false;
}
#endif

bool CHARACTER::BuildUpdatePartyPacket(TPacketGCPartyUpdate & out)
{
	if (!GetParty())
		return false;

	memset(&out, 0, sizeof(out));

	out.header = HEADER_GC_PARTY_UPDATE;
	out.pid = GetPlayerID();
	if (GetMaxHP() <= 0) // @fixme136
		out.percent_hp = 0;
	else
		out.percent_hp = MINMAX(0, GetHP() * 100 / GetMaxHP(), 100);
	out.role = GetParty()->GetRole(GetPlayerID());

	sys_log(1, "PARTY %s role is %d", GetName(), out.role);

	const LPCHARACTER& l = GetParty()->GetLeaderCharacter();

	if (l && DISTANCE_APPROX(GetX() - l->GetX(), GetY() - l->GetY()) < PARTY_DEFAULT_RANGE)
	{
		out.affects[0] = GetParty()->GetPartyBonusExpPercent();
		out.affects[1] = GetPoint(POINT_PARTY_ATTACKER_BONUS);
		out.affects[2] = GetPoint(POINT_PARTY_TANKER_BONUS);
		out.affects[3] = GetPoint(POINT_PARTY_BUFFER_BONUS);
		out.affects[4] = GetPoint(POINT_PARTY_SKILL_MASTER_BONUS);
		out.affects[5] = GetPoint(POINT_PARTY_HASTE_BONUS);
		out.affects[6] = GetPoint(POINT_PARTY_DEFENDER_BONUS);
	}

	return true;
}

int CHARACTER::GetLeadershipSkillLevel() const
{
	return GetSkillLevel(SKILL_LEADERSHIP);
}

void CHARACTER::QuerySafeboxSize()
{
	if (m_iSafeboxSize == -1)
	{
		DBManager::Instance().ReturnQuery(QID_SAFEBOX_SIZE,
			GetPlayerID(),
			nullptr,
			"SELECT size FROM safebox%s WHERE account_id = %u",
			get_table_postfix(),
			GetDesc()->GetAccountTable().id);
	}
}

void CHARACTER::SetSafeboxSize(int iSize)
{
	sys_log(1, "SetSafeboxSize: %s %d", GetName(), iSize);
	m_iSafeboxSize = iSize;
	DBManager::Instance().Query("UPDATE safebox%s SET size = %d WHERE account_id = %u", get_table_postfix(), iSize / SAFEBOX_PAGE_SIZE, GetDesc()->GetAccountTable().id);
}

int CHARACTER::GetSafeboxSize() const noexcept
{
	return m_iSafeboxSize;
}

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
void CHARACTER::QueryGuildstorageSize()
{
	if (m_iGuildstorageSize == -1)
	{
		DBManager::Instance().ReturnQuery(QID_GUILDSTORAGE_SIZE,
			GetPlayerID(),
			nullptr,
			"SELECT guildstorage FROM guild%s WHERE id = %u",
			get_table_postfix(),
			GetGuild()->GetID());
	}
}

void CHARACTER::SetGuildstorageSize(int iSize)
{
	sys_log(1, "SetGuildstorageSize: %s %d", GetName(), iSize);
	m_iGuildstorageSize = iSize;
	GetGuild()->SetGuildstorage(iSize / SAFEBOX_PAGE_SIZE);
	//DBManager::Instance().Query("UPDATE guild%s SET guildstorage = %d WHERE id = %u", get_table_postfix(), iSize / SAFEBOX_PAGE_SIZE, GetGuild()->GetID());
}

int CHARACTER::GetGuildstorageSize() const
{
	return m_iGuildstorageSize;
}
#endif

void CHARACTER::SetNowWalking(bool bWalkFlag)
{
	//if (m_bNowWalking != bWalkFlag || IsNPC())
	if (m_bNowWalking != bWalkFlag)
	{
		if (bWalkFlag)
		{
			m_bNowWalking = true;
			m_dwWalkStartTime = get_dword_time();
		}
		else
		{
			m_bNowWalking = false;
		}

		//if (m_bNowWalking)
		{
			TPacketGCWalkMode p{};
			p.vid = GetVID();
			p.header = HEADER_GC_WALK_MODE;
			p.mode = m_bNowWalking ? static_cast<uint8_t>(WalkMode::WALKMODE_WALK) : static_cast<uint8_t>(WalkMode::WALKMODE_RUN);

			PacketView(&p, sizeof(p));
		}

		if (IsNPC())
		{
			if (m_bNowWalking)
				MonsterLog("walk");
			else
				MonsterLog("run");
		}

		//sys_log(0, "%s is now %s", GetName(), m_bNowWalking?"walking.":"running.");
	}
}

void CHARACTER::StartStaminaConsume()
{
	if (m_bStaminaConsume)
		return;
	PointChange(POINT_STAMINA, 0);
	m_bStaminaConsume = true;
	//ChatPacket(CHAT_TYPE_COMMAND, "StartStaminaConsume %d %d", STAMINA_PER_STEP * passes_per_sec, GetStamina());
	if (IsStaminaHalfConsume())
		ChatPacket(CHAT_TYPE_COMMAND, "StartStaminaConsume %d %d", STAMINA_PER_STEP * passes_per_sec / 2, GetStamina());
	else
		ChatPacket(CHAT_TYPE_COMMAND, "StartStaminaConsume %d %d", STAMINA_PER_STEP * passes_per_sec, GetStamina());
}

void CHARACTER::StopStaminaConsume()
{
	if (!m_bStaminaConsume)
		return;
	PointChange(POINT_STAMINA, 0);
	m_bStaminaConsume = false;
	ChatPacket(CHAT_TYPE_COMMAND, "StopStaminaConsume %d", GetStamina());
}

bool CHARACTER::IsStaminaConsume() const noexcept
{
	return m_bStaminaConsume;
}

bool CHARACTER::IsStaminaHalfConsume() const
{
	return IsEquipUniqueItem(UNIQUE_ITEM_HALF_STAMINA);
}

void CHARACTER::ResetStopTime() noexcept
{
	m_dwStopTime = get_dword_time();
}

uint32_t CHARACTER::GetStopTime() const noexcept
{
	return m_dwStopTime;
}

void CHARACTER::ResetPoint(int iLv)
{
	const auto bJob = GetJob();

	PointChange(POINT_LEVEL, iLv - GetLevel());

	SetRealPoint(POINT_ST, JobInitialPoints[bJob].st);
	SetPoint(POINT_ST, GetRealPoint(POINT_ST));

	SetRealPoint(POINT_HT, JobInitialPoints[bJob].ht);
	SetPoint(POINT_HT, GetRealPoint(POINT_HT));

	SetRealPoint(POINT_DX, JobInitialPoints[bJob].dx);
	SetPoint(POINT_DX, GetRealPoint(POINT_DX));

	SetRealPoint(POINT_IQ, JobInitialPoints[bJob].iq);
	SetPoint(POINT_IQ, GetRealPoint(POINT_IQ));

	SetRandomHP((iLv - 1) * number(JobInitialPoints[GetJob()].hp_per_lv_begin, JobInitialPoints[GetJob()].hp_per_lv_end));
	SetRandomSP((iLv - 1) * number(JobInitialPoints[GetJob()].sp_per_lv_begin, JobInitialPoints[GetJob()].sp_per_lv_end));

	// @fixme104
	if (iLv > 1)
		PointChange(POINT_STAT, (MINMAX(1, iLv - 1, g_iStatusPointGetLevelLimit) * 3) + GetPoint(POINT_LEVEL_STEP) - GetPoint(POINT_STAT));
	else
		PointChange(POINT_STAT, -GetPoint(POINT_STAT));

	ComputePoints();

	// recovery
	PointChange(POINT_HP, GetMaxHP() - GetHP());
	PointChange(POINT_SP, GetMaxSP() - GetSP());

	PointsPacket();

	PointChange(POINT_STAT, 0);

	LogManager::Instance().CharLog(this, 0, "RESET_POINT", "");
}

#ifdef ENABLE_YOHARA_SYSTEM
void CHARACTER::ResetConquerorPoint(int iLv)
{
	PointChange(POINT_CONQUEROR_LEVEL, iLv - GetConquerorLevel());

	SetRealPoint(POINT_SUNGMA_STR, 0);
	SetPoint(POINT_SUNGMA_STR, GetRealPoint(POINT_SUNGMA_STR));

	SetRealPoint(POINT_SUNGMA_HP, 0);
	SetPoint(POINT_SUNGMA_HP, GetRealPoint(POINT_SUNGMA_HP));

	SetRealPoint(POINT_SUNGMA_MOVE, 0);
	SetPoint(POINT_SUNGMA_MOVE, GetRealPoint(POINT_SUNGMA_MOVE));

	SetRealPoint(POINT_SUNGMA_IMMUNE, 0);
	SetPoint(POINT_SUNGMA_IMMUNE, GetRealPoint(POINT_SUNGMA_IMMUNE));

	// @fixme104
	if (iLv > 1)
		PointChange(POINT_CONQUEROR_POINT, (MINMAX(0, iLv - 1, g_iConquerorPointGetLevelLimit) * 3) + GetPoint(POINT_CONQUEROR_LEVEL_STEP) - GetPoint(POINT_CONQUEROR_POINT));
	else
		PointChange(POINT_CONQUEROR_POINT, -GetPoint(POINT_CONQUEROR_POINT));

	ComputePoints();
	PointsPacket();

	LogManager::Instance().CharLog(this, 0, "RESET_CONQUEROR_POINT", "");
}
#endif

bool CHARACTER::IsChangeAttackPosition(const LPCHARACTER& target) const
{
	if (!target)
		return false;

	if (!IsNPC())
		return true;

	uint32_t dwChangeTime = AI_CHANGE_ATTACK_POISITION_TIME_NEAR;

	if (DISTANCE_APPROX(GetX() - target->GetX(), GetY() - target->GetY()) >
		AI_CHANGE_ATTACK_POISITION_DISTANCE + GetMobAttackRange())
		dwChangeTime = AI_CHANGE_ATTACK_POISITION_TIME_FAR;

	return get_dword_time() - m_dwLastChangeAttackPositionTime > dwChangeTime;
}

void CHARACTER::GiveRandomSkillBook()
{
	LPITEM item = AutoGiveItem(50300);

	if (nullptr != item)
	{
		extern const uint32_t GetRandomSkillVnum(uint8_t bJob = JOB_MAX_NUM);
		uint32_t dwSkillVnum = 0;
		// 50% of getting random books or getting one of the same player's race
		if (!number(0, 1))
			dwSkillVnum = GetRandomSkillVnum(GetJob());
		else
			dwSkillVnum = GetRandomSkillVnum();
		item->SetSocket(0, dwSkillVnum);
	}
}

void CHARACTER::ReviveInvisible(int iDur)
{
	AddAffect(AFFECT_REVIVE_INVISIBLE, POINT_NONE, 0, AFF_REVIVE_INVISIBLE, iDur, 0, true);
}

void CHARACTER::ToggleMonsterLog()
{
	m_bMonsterLog = !m_bMonsterLog;

	if (m_bMonsterLog)
	{
		CHARACTER_MANAGER::Instance().RegisterForMonsterLog(this);
	}
	else
	{
		CHARACTER_MANAGER::Instance().UnregisterForMonsterLog(this);
	}
}

void CHARACTER::SetGuild(CGuild* pGuild)
{
	if (m_pGuild != pGuild)
	{
		m_pGuild = pGuild;
		UpdatePacket();
	}
}

void CHARACTER::SendGreetMessage()
{
	typeof(DBManager::Instance().GetGreetMessage()) v = DBManager::Instance().GetGreetMessage();

	for (auto& it : v)
	{
		ChatPacket(CHAT_TYPE_NOTICE, it.c_str());
	}
}

void CHARACTER::BeginStateEmpty()
{
	MonsterLog("!");
}

void CHARACTER::EffectPacket(int enumEffectType)
{
	TPacketGCSpecialEffect p{};

	p.header = HEADER_GC_SEPCIAL_EFFECT;
	p.type = enumEffectType;
	p.vid = GetVID();

	PacketAround(&p, sizeof(TPacketGCSpecialEffect));
}

void CHARACTER::SpecificEffectPacket(const char filename[MAX_EFFECT_FILE_NAME])
{
	TPacketGCSpecificEffect p{};
	p.header = HEADER_GC_SPECIFIC_EFFECT;
	p.vid = GetVID();
	memcpy(p.effect_file, filename, MAX_EFFECT_FILE_NAME);

	PacketAround(&p, sizeof(TPacketGCSpecificEffect));
}

void CHARACTER::MonsterChat(uint8_t bMonsterChatType)
{
	if (IsPC())
		return;

	char sbuf[256 + 1] = { 0 };
	if (IsMonster())
	{
		if (number(0, 60))
			return;

		snprintf(sbuf, sizeof(sbuf),
			"(locale.monster_chat[%i] and locale.monster_chat[%i][%d] or '')",
			GetRaceNum(), GetRaceNum(), bMonsterChatType * 3 + number(1, 3));
	}
	else
	{
		if (bMonsterChatType != MONSTER_CHAT_WAIT)
			return;

		if (IsGuardNPC())
		{
			if (number(0, 6))
				return;
		}
		else
		{
			if (number(0, 30))
				return;
		}

		snprintf(sbuf, sizeof(sbuf), "(locale.monster_chat[%i] and locale.monster_chat[%i][number(1, table.getn(locale.monster_chat[%i]))] or '')", GetRaceNum(), GetRaceNum(), GetRaceNum());
	}

	std::string text = quest::ScriptToString(sbuf);

	if (text.empty())
		return;

	TPacketGCChat pack_chat{};
	pack_chat.header = HEADER_GC_CHAT;
	pack_chat.size = static_cast<uint16_t>(sizeof(TPacketGCChat) + text.size() + 1);
	pack_chat.type = CHAT_TYPE_TALKING;
	pack_chat.id = GetVID();
	pack_chat.bEmpire = 0;

	TEMP_BUFFER buf;
	buf.write(&pack_chat, sizeof(TPacketGCChat));
	buf.write(text.c_str(), text.size() + 1);

	PacketAround(buf.read_peek(), buf.size());
}

void CHARACTER::SetQuestNPCID(uint32_t vid) noexcept
{
	m_dwQuestNPCVID = vid;
}

LPCHARACTER CHARACTER::GetQuestNPC() const
{
	return CHARACTER_MANAGER::Instance().Find(m_dwQuestNPCVID);
}

void CHARACTER::SetQuestItemPtr(LPITEM item) noexcept
{
	m_pQuestItem = item;
}

void CHARACTER::ClearQuestItemPtr() noexcept
{
	m_pQuestItem = nullptr;
}

LPITEM CHARACTER::GetQuestItemPtr() const noexcept
{
	return m_pQuestItem;
}

#ifdef ENABLE_QUEST_DND_EVENT
void CHARACTER::SetQuestDNDItemPtr(LPITEM item)
{
	m_pQuestDNDItem = item;
}

void CHARACTER::ClearQuestDNDItemPtr()
{
	m_pQuestDNDItem = NULL;
}

LPITEM CHARACTER::GetQuestDNDItemPtr() const
{
	return m_pQuestDNDItem;
}
#endif

LPDUNGEON CHARACTER::GetDungeonForce() const
{
	if (m_lWarpMapIndex > 10000)
		return CDungeonManager::Instance().FindByMapIndex(m_lWarpMapIndex);

	return m_pkDungeon;
}

#ifdef ENABLE_12ZI
LPZODIAC CHARACTER::GetZodiacForce() const
{
	if (m_lWarpMapIndex > 10000)
		return CZodiacManager::Instance().FindByMapIndex(m_lWarpMapIndex);

	return m_pkZodiac;
}
#endif

void CHARACTER::SetBlockMode(uint8_t bFlag)
{
	m_pointsInstant.bBlockMode = bFlag;

	ChatPacket(CHAT_TYPE_COMMAND, "setblockmode %d", m_pointsInstant.bBlockMode);

	SetQuestFlag("game_option.block_exchange", bFlag & BLOCK_EXCHANGE ? 1 : 0);
	SetQuestFlag("game_option.block_party_invite", bFlag & BLOCK_PARTY_INVITE ? 1 : 0);
	SetQuestFlag("game_option.block_guild_invite", bFlag & BLOCK_GUILD_INVITE ? 1 : 0);
	SetQuestFlag("game_option.block_whisper", bFlag & BLOCK_WHISPER ? 1 : 0);
	SetQuestFlag("game_option.block_messenger_invite", bFlag & BLOCK_MESSENGER_INVITE ? 1 : 0);
	SetQuestFlag("game_option.block_party_request", bFlag & BLOCK_PARTY_REQUEST ? 1 : 0);
}

void CHARACTER::SetBlockModeForce(uint8_t bFlag)
{
	m_pointsInstant.bBlockMode = bFlag;
	ChatPacket(CHAT_TYPE_COMMAND, "setblockmode %d", m_pointsInstant.bBlockMode);
}

bool CHARACTER::IsGuardNPC() const noexcept
{
	return IsNPC() && (GetRaceNum() == 11000 || GetRaceNum() == 11002 || GetRaceNum() == 11004);
}

int CHARACTER::GetPolymorphPower() const
{
	if (test_server)
	{
		int value = quest::CQuestManager::Instance().GetEventFlag("poly");
		if (value)
			return value;
	}
	return aiPolymorphPowerByLevel[MINMAX(0, GetSkillLevel(SKILL_POLYMORPH), 40)];
}

void CHARACTER::SetPolymorph(uint32_t dwRaceNum, bool bMaintainStat)
{
#ifdef ENABLE_WOLFMAN_CHARACTER
	if (dwRaceNum < MAIN_RACE_MAX_NUM)
#else
	if (dwRaceNum < JOB_MAX_NUM)
#endif
	{
		dwRaceNum = 0;
		bMaintainStat = false;
	}

	if (m_dwPolymorphRace == dwRaceNum)
		return;

	m_bPolyMaintainStat = bMaintainStat;
	m_dwPolymorphRace = dwRaceNum;

	sys_log(0, "POLYMORPH: %s race %u ", GetName(), dwRaceNum);

#ifdef ENABLE_MOUNT_PROTO_AFFECT_SYSTEM
	if (dwRaceNum != 0)
	{
		MountVnum(0);
		if (IsRiding())
			StopRiding();
	}
	else
	{
		MountVnum(GetPoint(POINT_MOUNT));
	}
#else
	if (dwRaceNum != 0)
		StopRiding();
#endif

	SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_SPAWN);
	m_afAffectFlag.Set(AFF_SPAWN);

	ViewReencode();

	REMOVE_BIT(m_bAddChrState, ADD_CHARACTER_STATE_SPAWN);

	if (!bMaintainStat)
	{
		PointChange(POINT_ST, 0);
		PointChange(POINT_DX, 0);
		PointChange(POINT_IQ, 0);
		PointChange(POINT_HT, 0);
	}

	/* If you die while in polymorph, the polymorph will be released.
	* Because the valid combo interval is different before and after the polymorph
	* Sometimes it is recognized as a Combo Hack or Hacker.
	* So when unpacking or polymorphing,
	* Reset the valid combo interval.
	*/
	SetValidComboInterval(0);
	SetComboSequence(0);

	ComputeBattlePoints();
}

int CHARACTER::GetQuestFlag(const std::string& flag) const
{
	if (!IsPC())
	{
		sys_err("Trying to get qf %s from non player character", flag.c_str());
		return 0;
	}

	quest::CQuestManager& q = quest::CQuestManager::Instance();
	quest::PC* pPC = q.GetPC(GetPlayerID());

	if (!pPC) {	//@fixme462
		//sys_err("Nullpointer when trying to access questflag %s for player with pid %lu", flag.c_str(), pid);
		return 0;
	}
	return pPC->GetFlag(flag);
}

void CHARACTER::SetQuestFlag(const std::string& flag, int value)
{
	quest::CQuestManager& q = quest::CQuestManager::Instance();
	quest::PC* pPC = q.GetPC(GetPlayerID());

	if (!pPC) {	//@fixme462
		//sys_err("Nullpointer when trying to set questflag %s for player with pid %lu", flag.c_str(), pid);
		return;
	}
	pPC->SetFlag(flag, value);
}

void CHARACTER::DetermineDropMetinStone()
{
#ifdef ENABLE_NEWSTUFF
	if (g_NoDropMetinStone)
	{
		m_dwDropMetinStone = 0;
		return;
	}
#endif

	static const uint32_t c_adwMetin[] =
	{
#if defined(ENABLE_WOLFMAN_CHARACTER) && defined(USE_WOLFMAN_STONES)
		28012,
#endif
		28030,
		28031,
		28032,
		28033,
		28034,
		28035,
		28036,
		28037,
		28038,
		28039,
		28040,
		28041,
		28042,
		28043,
#if defined(ENABLE_MAGIC_REDUCTION_SYSTEM) && defined(USE_MAGIC_REDUCTION_STONES)
		28044,
		28045,
#endif
	};

	const uint32_t stone_num = GetRaceNum();
	const int idx = std::lower_bound(aStoneDrop, aStoneDrop + STONE_INFO_MAX_NUM, stone_num) - aStoneDrop;
	if (idx >= STONE_INFO_MAX_NUM || aStoneDrop[idx].dwMobVnum != stone_num)
	{
		m_dwDropMetinStone = 0;
	}
	else
	{
		const SStoneDropInfo& info = aStoneDrop[idx];
		m_bDropMetinStonePct = info.iDropPct;
		{
			m_dwDropMetinStone = c_adwMetin[number(0, sizeof(c_adwMetin) / sizeof(uint32_t) - 1)];
			int iGradePct = number(1, 100);
			for (int iStoneLevel = 0; iStoneLevel < STONE_LEVEL_MAX_NUM; iStoneLevel++)
			{
				const int iLevelGradePortion = info.iLevelPct[iStoneLevel];
				if (iGradePct <= iLevelGradePortion)
				{
					break;
				}
				else
				{
					iGradePct -= iLevelGradePortion;
					m_dwDropMetinStone += 100; // Each stone +a -> +(a+1) increases by 100
				}
			}
		}
	}
}

#ifdef ENABLE_YOHARA_SYSTEM
void CHARACTER::DetermineDropSungmaStone()
{
	static const uint32_t c_adwMetin[] =
	{
		28046,
		//28047, Red Stones cannot be dropped, only get by crafting
	};

	const uint32_t dwSungmaStone = GetRaceNum();
	const int idx = std::lower_bound(aSungmaStoneDrop, aSungmaStoneDrop + SUNGMA_STONE_INFO_MAX_NUM, dwSungmaStone) - aSungmaStoneDrop;
	if (idx >= SUNGMA_STONE_INFO_MAX_NUM || aSungmaStoneDrop[idx].dwMobVnum != dwSungmaStone)
	{
		m_dwDropMetinStone = 0;
	}
	else
	{
		const SStoneDropInfo& info = aSungmaStoneDrop[idx];
		m_bDropMetinStonePct = info.iDropPct;
		{
			m_dwDropMetinStone = c_adwMetin[number(0, sizeof(c_adwMetin) / sizeof(uint32_t) - 1)];
			int iGradePct = number(1, 100);
			for (int iStoneLevel = 0; iStoneLevel < STONE_LEVEL_MAX_NUM; iStoneLevel++)
			{
				const int iLevelGradePortion = info.iLevelPct[iStoneLevel];
				if (iGradePct <= iLevelGradePortion)
					break;
				else
				{
					iGradePct -= iLevelGradePortion;
					m_dwDropMetinStone += 100;
				}
			}
		}
	}
}
#endif

void CHARACTER::SendEquipment(const LPCHARACTER& ch)
{
	if (!ch)
		return;

	TPacketViewEquip p{};
	p.header = HEADER_GC_VIEW_EQUIP;
	p.vid = GetVID();

	for (int i = 0; i < WEAR_MAX_NUM; i++)	//ENABLE_VIEW_EQUIP_RENEWAL
	{
		LPITEM item = GetWear(i);
		if (item)
		{
			p.equips[i].vnum = item->GetVnum();
			p.equips[i].count = item->GetCount();
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			p.equips[i].dwTransmutationVnum = item->GetChangeLookVnum();
#endif

			thecore_memcpy(p.equips[i].alSockets, item->GetSockets(), sizeof(p.equips[i].alSockets));
			thecore_memcpy(p.equips[i].aAttr, item->GetAttributes(), sizeof(p.equips[i].aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
			thecore_memcpy(p.equips[i].aApplyRandom, item->GetApplysRandom(), sizeof(p.equips[i].aApplyRandom));
			thecore_memcpy(p.equips[i].alRandomValues, item->GetRandomDefaultAttrs(), sizeof(p.equips[i].alRandomValues));
#endif
		}
		else
		{
			p.equips[i].vnum = 0;
		}
	}

	ch->GetDesc()->Packet(&p, sizeof(p));
}

bool CHARACTER::CanSummon(int iLeaderShip) noexcept
{
	return ((iLeaderShip >= 20) || ((iLeaderShip >= 12) && ((m_dwLastDeadTime + 180) > get_dword_time())));
}

void CHARACTER::MountVnum(uint32_t vnum)
{
#ifdef DISABLE_RIDE_ON_SPECIAL_MAP
	if (vnum && !IS_MOUNTABLE_ZONE(GetMapIndex()))	//@custom005
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("RIDING_IS_BLOCKED_HERE"));
		return;
	}
#endif

	if (IsPolymorphed())
		return;

	if (m_dwMountVnum == vnum)
		return;

	if ((m_dwMountVnum != 0) && (vnum != 0)) //@fixme108 set recursively to 0 for eventuality
		MountVnum(0);

	m_dwMountVnum = vnum;
	m_dwMountTime = get_dword_time();

	if (m_bIsObserver)
		return;

	//NOTE: Mounting does not delete the object on the client side.
	// And when riding on the server side, the position does not move. Because you can do Collision Adjust on the client side.
	// If the object is destroyed and moved to the server location, collision check is not performed at this time, so there is a problem of getting stuck in the background or breaking through.
	m_posDest.x = m_posStart.x = GetX();
	m_posDest.y = m_posStart.y = GetY();
#ifdef ENABLE_MOUNT_ENTITY_REFRESH
	// EncodeRemovePacket(this); // commented, otherwise it may warp you back
#endif
	EncodeInsertPacket(this);

	ENTITY_MAP::iterator it = m_map_view.begin();

	while (it != m_map_view.end())
	{
		LPENTITY entity = (it++)->first;

#ifdef ENABLE_MOUNT_ENTITY_REFRESH
		if (entity->IsType(ENTITY_CHARACTER))
		{
			EncodeRemovePacket(entity);
			if (!m_bIsObserver)
				EncodeInsertPacket(entity);

			if (!entity->IsObserverMode())
					entity->EncodeInsertPacket(this);
		}
		else
			EncodeInsertPacket(entity);
#else
		EncodeInsertPacket(entity);
#endif
	}

	SetValidComboInterval(0);
	SetComboSequence(0);

	//ComputePoints();
	ComputeBattlePoints(); // fixme
}

namespace {
	class FuncCheckWarp
	{
		public:
			FuncCheckWarp(LPCHARACTER pkWarp)
			{
				if (!pkWarp)
					return;

				m_lTargetY = 0;
				m_lTargetX = 0;

				m_lX = pkWarp->GetX();
				m_lY = pkWarp->GetY();

				m_bInvalid = false;
				m_bEmpire = pkWarp->GetEmpire();

				char szTmp[64];

				if ((3 != sscanf(pkWarp->GetName(), " %s %ld %ld ", szTmp, &m_lTargetX, &m_lTargetY))
#ifdef ENABLE_DEFENSE_WAVE
					&& (!pkWarp->IsDefenseWavePortal())
#endif
					)
				{
					if (number(1, 100) < 5)
						sys_err("Warp NPC name wrong : vnum(%d) name(%s)", pkWarp->GetRaceNum(), pkWarp->GetName());

					m_bInvalid = true;

					return;
				}

				m_lTargetX *= 100;
				m_lTargetY *= 100;

#ifdef ENABLE_PORTAL_LEVEL_LIMIT
				const std::pair<int, int>* pPortalLevelLimit = CHARACTER_MANAGER::Instance().GetPortalLevelLimit(pkWarp->GetRaceNum());
				m_iMinLevelLimit = pPortalLevelLimit ? pPortalLevelLimit->first : 1;
				m_iMaxLevelLimit = pPortalLevelLimit ? pPortalLevelLimit->second : gPlayerMaxLevel;
#endif

				m_bUseWarp = true;

				if (pkWarp->IsGoto())
				{
					const SECTREE_MAP* pkSectreeMap = SECTREE_MANAGER::Instance().GetMap(pkWarp->GetMapIndex());
					m_lTargetX += pkSectreeMap->m_setting.iBaseX;
					m_lTargetY += pkSectreeMap->m_setting.iBaseY;
					m_bUseWarp = false;
				}

#ifdef ENABLE_DEFENSE_WAVE
				pkWarps = pkWarp;
#endif
			}

			bool Valid() noexcept
			{
				return !m_bInvalid;
			}

			void operator() (LPENTITY ent)
			{
				if (!ent)
					return;

				if (!Valid())
					return;

				if (!ent->IsType(ENTITY_CHARACTER))
					return;

				LPCHARACTER pkChr = dynamic_cast<LPCHARACTER>(ent);

				if (!pkChr || !pkChr->IsPC())
					return;

				const int iDist = DISTANCE_APPROX(pkChr->GetX() - m_lX, pkChr->GetY() - m_lY);

				if (iDist > 300)
					return;

				if (m_bEmpire && pkChr->GetEmpire() && m_bEmpire != pkChr->GetEmpire())
					return;

				if (pkChr->IsHack())
					return;

				if (!pkChr->CanHandleItem(false, true))
					return;

#ifdef ENABLE_PORTAL_LEVEL_LIMIT
				const int iPlayerLevel = pkChr->GetLevel();
				if (iPlayerLevel >= m_iMinLevelLimit && iPlayerLevel <= m_iMaxLevelLimit)
#endif
				{
					if (m_bUseWarp)
					{
						auto coordX = m_lTargetX;
						auto coordY = m_lTargetY;
#ifdef ENABLE_DEFENSE_WAVE
						if (pkWarps && pkWarps->IsDefenseWavePortal())
						{
							coordX = DEFENSE_WAVE_NEW_CONTINENT_X * 100;
							coordY = DEFENSE_WAVE_NEW_CONTINENT_Y * 100;
						}
#endif

						pkChr->WarpSet(coordX, coordY);
					}
					else
					{
						pkChr->Show(pkChr->GetMapIndex(), m_lTargetX, m_lTargetY);
						pkChr->Stop();
					}
				}
#ifdef ENABLE_PORTAL_LEVEL_LIMIT
				/*else if (number(1, 100) < 10)
					pkChr->ChatPacket(CHAT_TYPE_INFO, "Your level is not enough for this portal.");*/
#endif
			}

			bool m_bInvalid = false;
			bool m_bUseWarp = false;

			long m_lX = 0;
			long m_lY = 0;
			long m_lTargetX = 0;
			long m_lTargetY = 0;

#ifdef ENABLE_PORTAL_LEVEL_LIMIT
			int m_iMinLevelLimit;
			int m_iMaxLevelLimit;
#endif

			uint8_t m_bEmpire = 0;
#ifdef ENABLE_DEFENSE_WAVE
			LPCHARACTER pkWarps;
#endif
	};
}

EVENTFUNC(warp_npc_event)
{
	const char_event_info* info = dynamic_cast<char_event_info*>(event->info);
	if (info == nullptr)
	{
		sys_err("warp_npc_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == nullptr) // <Factor>
		return 0;

	if (!ch->GetSectree())
	{
		ch->m_pkWarpNPCEvent = nullptr;
		return 0;
	}

	FuncCheckWarp f(ch);
	if (f.Valid())
		ch->GetSectree()->ForEachAround(f);

	return passes_per_sec / 2;
}

void CHARACTER::StartWarpNPCEvent()
{
	if (m_pkWarpNPCEvent)
		return;

	if (!IsWarp() && !IsGoto()
#ifdef ENABLE_DEFENSE_WAVE
		&& !IsDefenseWavePortal()
#endif
		)
	{
		return;
	}

	char_event_info* info = AllocEventInfo<char_event_info>();

	info->ch = this;

	m_pkWarpNPCEvent = event_create(warp_npc_event, info, passes_per_sec / 2);
}

void CHARACTER::SyncPacket()
{
	TEMP_BUFFER buf;

	TPacketGCSyncPositionElement elem{};

	elem.dwVID = GetVID();
	elem.lX = GetX();
	elem.lY = GetY();

	TPacketGCSyncPosition pack{};

	pack.bHeader = HEADER_GC_SYNC_POSITION;
	pack.wSize = sizeof(TPacketGCSyncPosition) + sizeof(TPacketGCSyncPositionElement);

	buf.write(&pack, sizeof(pack));
	buf.write(&elem, sizeof(elem));

	PacketAround(buf.read_peek(), buf.size());
}

LPCHARACTER CHARACTER::GetMarryPartner() const noexcept
{
	return m_pkChrMarried;
}

void CHARACTER::SetMarryPartner(LPCHARACTER ch) noexcept
{
	m_pkChrMarried = ch;
}

int CHARACTER::GetMarriageBonus(uint32_t dwItemVnum, bool bSum)
{
	if (IsNPC())
		return 0;

	marriage::TMarriage* pMarriage = marriage::CManager::Instance().Get(GetPlayerID());

	if (!pMarriage)
		return 0;

	return pMarriage->GetBonus(dwItemVnum, bSum, this);
}

void CHARACTER::ConfirmWithMsg(const char* szMsg, int iTimeout, uint32_t dwRequestPID)
{
	if (!IsPC())
		return;

	TPacketGCQuestConfirm p{};
	p.header = HEADER_GC_QUEST_CONFIRM;
	p.requestPID = dwRequestPID;
	p.timeout = iTimeout;
	strlcpy(p.msg, szMsg, sizeof(p.msg));

	GetDesc()->Packet(&p, sizeof(p));
}

int CHARACTER::GetPremiumRemainSeconds(uint8_t bType) const
{
	if (bType >= PREMIUM_MAX_NUM)
		return 0;

	return m_aiPremiumTimes[bType] - get_global_time();
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
int CHARACTER::GetPremiumPrivateShopRemainSeconds() const
{
	long lShopRemainSeconds = 0;

	const CAffect* pkAffect = FindAffect(AFFECT_PREMIUM_PRIVATE_SHOP);
	if (pkAffect)
		lShopRemainSeconds = pkAffect->lDuration / 60; // affect is in seconds, so we need to divide between 60 for shops

	if (lShopRemainSeconds <= PREMIUM_PRIVATE_SHOP_DEFAULT_TIME)
		lShopRemainSeconds = PREMIUM_PRIVATE_SHOP_DEFAULT_TIME;

	return lShopRemainSeconds;
}
#endif

bool CHARACTER::WarpToPID(uint32_t dwPID)
{
	LPCHARACTER victim;
	if ((victim = (CHARACTER_MANAGER::Instance().FindByPID(dwPID))))
	{
		const auto mapIdx = victim->GetMapIndex();
		if (IS_SUMMONABLE_ZONE(mapIdx))
		{
			if (CAN_ENTER_ZONE(this, mapIdx))
			{
				WarpSet(victim->GetX(), victim->GetY());
			}
			else
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;846]");
				return false;
			}
		}
		else
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;846]");
			return false;
		}
	}
	else
	{
		// Someone is logged in to another server -> Send a message to get coordinates
		// 1. Sprinkle A.pid, B.pid
		// 2. The server with B.pid sends A.pid, coordinates to the rooted server
		// 3. Warp
		CCI* pcci = P2P_MANAGER::Instance().FindByPID(dwPID);

		if (!pcci)
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;848]");
			return false;
		}

		if (pcci->bChannel != g_bChannel)
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;849;%d;%d]", pcci->bChannel, g_bChannel);
			return false;
		}
		else if (false == IS_SUMMONABLE_ZONE(pcci->lMapIndex))
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;846]");
			return false;
		}
		else
		{
			if (!CAN_ENTER_ZONE(this, pcci->lMapIndex))
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;846]");
				return false;
			}

			TPacketGGFindPosition p{};
			p.header = HEADER_GG_FIND_POSITION;
			p.dwFromPID = GetPlayerID();
			p.dwTargetPID = dwPID;
			pcci->pkDesc->Packet(&p, sizeof(TPacketGGFindPosition));

			if (test_server)
				ChatPacket(CHAT_TYPE_PARTY, "sent find position packet for teleport");
		}
	}
	return true;
}

// ADD_REFINE_BUILDING
CGuild* CHARACTER::GetRefineGuild() const
{
	const LPCHARACTER& chRefineNPC = CHARACTER_MANAGER::Instance().Find(m_dwRefineNPCVID);

	return (chRefineNPC ? chRefineNPC->GetGuild() : nullptr);
}

LPCHARACTER CHARACTER::GetRefineNPC() const
{
	if (!m_dwRefineNPCVID)
		return nullptr;

	return CHARACTER_MANAGER::Instance().Find(m_dwRefineNPCVID);
}

bool CHARACTER::IsRefineThroughGuild() const
{
	return GetRefineGuild() != nullptr;
}

int CHARACTER::ComputeRefineFee(int iCost, int iMultiply) const
{
	const CGuild* pGuild = GetRefineGuild();
	if (pGuild)
	{
		if (pGuild == GetGuild())
			/*if (pGuild->GetLevel() >= 20) {	// Preisnachlass: Gildenschmied [Bonus 4]
				return iCost * iMultiply * 4 / 10;	// Doppelter Preis vom normalen Schmied
			}
			else {
				return iCost * iMultiply * 9 / 10;
			}*/
			return iCost * iMultiply * 9 / 10;

		// An additional 3x more if another Empire player tries
		const LPCHARACTER& chRefineNPC = CHARACTER_MANAGER::Instance().Find(m_dwRefineNPCVID);
		if (chRefineNPC && chRefineNPC->GetEmpire() != GetEmpire())
			return iCost * iMultiply * 3;

		return iCost * iMultiply;
	}
	else
		return iCost;
}

void CHARACTER::PayRefineFee(int iTotalMoney)
{
	const int iFee = iTotalMoney / 10;
	CGuild* pGuild = GetRefineGuild();

	int iRemain = iTotalMoney;

	if (pGuild)
	{
		// If it is your guild, iTotalMoney has already excluded 10%
		if (pGuild != GetGuild())
		{
			pGuild->RequestDepositMoney(this, iFee);
			iRemain -= iFee;
		}
	}

	PointChange(POINT_GOLD, -iRemain);

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	CAchievementSystem::Instance().OnGoldChange(this, achievements::ETaskTypes::TYPE_SPEND_UPGRADE, iRemain);
#endif
}
// END_OF_ADD_REFINE_BUILDING

// Check to prevent hacking.
bool CHARACTER::IsHack(bool bSendMsg, bool bCheckShopOwner, int limittime)
{
	const int iPulse = thecore_pulse();

	if (test_server)
		bSendMsg = true;

	// Check after warehouse opening
	if (iPulse - GetSafeboxLoadTime() < PASSES_PER_SEC(limittime))
	{
		if (bSendMsg)
			ChatPacket(CHAT_TYPE_INFO, "[LS;850;%d]", limittime);

		if (test_server)
			ChatPacket(CHAT_TYPE_INFO, "Pulse %d LoadTime %d PASS %d", iPulse, GetSafeboxLoadTime(), PASSES_PER_SEC(limittime));
		return true;
	}

	// Check transaction related window
	if (bCheckShopOwner)
	{
		if (GetOpenedWindow(W_EXCHANGE | W_SAFEBOX | W_CUBE | W_SHOP_OWNER
#ifndef ENABLE_PREMIUM_PRIVATE_SHOP
			| W_MYSHOP
#endif
#ifdef ENABLE_SKILLBOOK_COMBINATION
			| W_SKILLBOOK_COMB
#endif
#ifdef ENABLE_ATTR_6TH_7TH
			| W_ATTR_6TH_7TH
#endif
#ifdef ENABLE_AURA_SYSTEM
			| W_AURA
#endif
#ifdef ENABLE_SWITCHBOT
			| W_SWITCHBOT
#endif
		))
		{
			if (bSendMsg)
				ChatPacket(CHAT_TYPE_INFO, "[LS;851]");

			return true;
		}
	}
	else
	{
		if (GetOpenedWindow(W_EXCHANGE | W_SAFEBOX | W_CUBE | W_MYSHOP
#ifdef ENABLE_SKILLBOOK_COMBINATION
			| W_SKILLBOOK_COMB
#endif
#ifdef ENABLE_ATTR_6TH_7TH
			| W_ATTR_6TH_7TH
#endif
#ifdef ENABLE_AURA_SYSTEM
			| W_AURA
#endif
#ifdef ENABLE_SWITCHBOT
			| W_SWITCHBOT
#endif
		))
		{
			if (bSendMsg)
				ChatPacket(CHAT_TYPE_INFO, "[LS;851]");

			return true;
		}
	}

	//PREVENT_PORTAL_AFTER_EXCHANGE
	//Check the time after exchange
	if (iPulse - GetExchangeTime() < PASSES_PER_SEC(limittime))
	{
		if (bSendMsg)
			ChatPacket(CHAT_TYPE_INFO, "[LS;852;%d]", limittime);
		return true;
	}
	//END_PREVENT_PORTAL_AFTER_EXCHANGE

	//PREVENT_ITEM_COPY
	if (iPulse - GetMyShopTime() < PASSES_PER_SEC(limittime))
	{
		if (bSendMsg)
			ChatPacket(CHAT_TYPE_INFO, "[LS;852;%d]", limittime);
		return true;
	}

	if (iPulse - GetRefineTime() < PASSES_PER_SEC(limittime))
	{
		if (bSendMsg)
			ChatPacket(CHAT_TYPE_INFO, "[LS;437;%d]", limittime);
		return true;
	}
	//END_PREVENT_ITEM_COPY

#ifdef ENABLE_MAILBOX
	if (iPulse - GetMyMailBoxTime() < PASSES_PER_SEC(limittime))
	{
		if (bSendMsg)
			ChatPacket(CHAT_TYPE_INFO, "You cannot go elsewhere for %d seconds after mailbox.", limittime);
		return true;
	}
#endif

	return false;
}

bool CHARACTER::IsMonarch() const
{
	//MONARCH_LIMIT
	if (CMonarch::Instance().IsMonarch(GetPlayerID(), GetEmpire()))
		return true;

	return false;

	//END_MONARCH_LIMIT
}
void CHARACTER::Say(const std::string& s)
{
	TPacketGCScript packet_script{};
	packet_script.header = HEADER_GC_SCRIPT;
	packet_script.skin = 1;
	packet_script.src_size = static_cast<uint16_t>(s.size());
	packet_script.size = packet_script.src_size + sizeof(TPacketGCScript);

	TEMP_BUFFER buf;

	buf.write(&packet_script, sizeof(TPacketGCScript));
	buf.write(&s[0], s.size());

	if (IsPC())
	{
		GetDesc()->Packet(buf.read_peek(), buf.size());
	}
}

//
// Monarch
//
void CHARACTER::InitMC() noexcept
{
	for (int n = 0; n < MI_MAX; ++n)
	{
		m_dwMonarchCooltime[n] = thecore_pulse();
	}

	m_dwMonarchCooltimelimit[MI_HEAL] = PASSES_PER_SEC(MC_HEAL);
	m_dwMonarchCooltimelimit[MI_WARP] = PASSES_PER_SEC(MC_WARP);
	m_dwMonarchCooltimelimit[MI_TRANSFER] = PASSES_PER_SEC(MC_TRANSFER);
	m_dwMonarchCooltimelimit[MI_TAX] = PASSES_PER_SEC(MC_TAX);
	m_dwMonarchCooltimelimit[MI_SUMMON] = PASSES_PER_SEC(MC_SUMMON);

	m_dwMonarchCooltime[MI_HEAL] -= PASSES_PER_SEC(GetMCL(MI_HEAL));
	m_dwMonarchCooltime[MI_WARP] -= PASSES_PER_SEC(GetMCL(MI_WARP));
	m_dwMonarchCooltime[MI_TRANSFER] -= PASSES_PER_SEC(GetMCL(MI_TRANSFER));
	m_dwMonarchCooltime[MI_TAX] -= PASSES_PER_SEC(GetMCL(MI_TAX));
	m_dwMonarchCooltime[MI_SUMMON] -= PASSES_PER_SEC(GetMCL(MI_SUMMON));
}

uint32_t CHARACTER::GetMC(enum MONARCH_INDEX e) const noexcept
{
	return m_dwMonarchCooltime[e];
}

void CHARACTER::SetMC(enum MONARCH_INDEX e) noexcept
{
	m_dwMonarchCooltime[e] = thecore_pulse();
}

bool CHARACTER::IsMCOK(enum MONARCH_INDEX e) const
{
	const int iPulse = thecore_pulse();
	if ((iPulse - GetMC(e)) < GetMCL(e))
	{
		if (test_server)
			sys_log(0, " Pulse %d cooltime %d, limit %d", iPulse, GetMC(e), GetMCL(e));

		return false;
	}

	if (test_server)
		sys_log(0, " Pulse %d cooltime %d, limit %d", iPulse, GetMC(e), GetMCL(e));

	return true;
}

uint32_t CHARACTER::GetMCL(enum MONARCH_INDEX e) const noexcept
{
	return m_dwMonarchCooltimelimit[e];
}

uint32_t CHARACTER::GetMCLTime(enum MONARCH_INDEX e) const
{
	const int iPulse = thecore_pulse();
	if (test_server)
		sys_log(0, " Pulse %d cooltime %d, limit %d", iPulse, GetMC(e), GetMCL(e));

	return (GetMCL(e)) / passes_per_sec - (iPulse - GetMC(e)) / passes_per_sec;
}

bool CHARACTER::IsSiegeNPC() const noexcept
{
	return IsNPC() && (GetRaceNum() == 11000 || GetRaceNum() == 11002 || GetRaceNum() == 11004);
}

//------------------------------------------------
void CHARACTER::UpdateDepositPulse() noexcept
{
#ifdef ENABLE_GUILDRENEWAL_SYSTEM
	m_deposit_pulse = thecore_pulse() + PASSES_PER_SEC(10);	//10sec
#else
	m_deposit_pulse = thecore_pulse() + PASSES_PER_SEC(60 * 5);	//5min
#endif
}

bool CHARACTER::CanDeposit() const noexcept
{
	return (m_deposit_pulse == 0 || (m_deposit_pulse < thecore_pulse()));
}
//------------------------------------------------

ESex GET_SEX(const LPCHARACTER& ch) noexcept
{
	if (!ch)
		return ESex::SEX_MALE;

	switch (ch->GetRaceNum())
	{
		case MAIN_RACE_WARRIOR_M:
		case MAIN_RACE_SURA_M:
		case MAIN_RACE_ASSASSIN_M:
		case MAIN_RACE_SHAMAN_M:
#ifdef ENABLE_WOLFMAN_CHARACTER
		case MAIN_RACE_WOLFMAN_M:
#endif
			return ESex::SEX_MALE;

		case MAIN_RACE_ASSASSIN_W:
		case MAIN_RACE_SHAMAN_W:
		case MAIN_RACE_WARRIOR_W:
		case MAIN_RACE_SURA_W:
			return ESex::SEX_FEMALE;

		default:
			break;
	}

	/* default sex = male */
	return ESex::SEX_MALE;
}

int CHARACTER::GetHPPct() const noexcept
{
	if (GetMaxHP() <= 0) // @fixme136
		return 0;
	return (GetHP() * 100) / GetMaxHP();
}

bool CHARACTER::IsBerserk() const noexcept
{
	if (m_pkMobInst != nullptr)
		return m_pkMobInst->m_IsBerserk;
	else
		return false;
}

void CHARACTER::SetBerserk(bool mode) noexcept
{
	if (m_pkMobInst != nullptr)
		m_pkMobInst->m_IsBerserk = mode;
}

bool CHARACTER::IsGodSpeed() const noexcept
{
	if (m_pkMobInst != nullptr)
		return m_pkMobInst->m_IsGodSpeed;
	else
		return false;
}

void CHARACTER::SetGodSpeed(bool mode)
{
	if (m_pkMobInst != nullptr)
	{
		m_pkMobInst->m_IsGodSpeed = mode;

		if (mode == true)
			SetPoint(POINT_ATT_SPEED, 250);
		else
			SetPoint(POINT_ATT_SPEED, m_pkMobData->m_table.sAttackSpeed);
	}
}

bool CHARACTER::IsDeathBlow() const noexcept
{
	if (number(1, 100) <= m_pkMobData->m_table.bDeathBlowPoint)
		return true;
	else
		return false;
}

struct FFindReviver
{
	FFindReviver() noexcept
	{
		pChar = nullptr;
		HasReviver = false;
	}

	void operator() (LPCHARACTER ch)
	{
		if (ch)
		{
			if (ch->IsMonster() != true)
				return;

			if (ch->IsReviver() == true && pChar != ch && ch->IsDead() != true)
			{
				if (number(1, 100) <= ch->GetMobTable().bRevivePoint)
				{
					HasReviver = true;
					pChar = ch;
				}
			}
		}
	}

	LPCHARACTER pChar;
	bool HasReviver;
};

bool CHARACTER::HasReviverInParty() const
{
	LPPARTY party = GetParty();

	if (party != nullptr)
	{
		if (party->GetMemberCount() == 1) return false;

		FFindReviver f;
		party->ForEachMemberPtr(f);
		return f.HasReviver;
	}

	return false;
}

bool CHARACTER::IsRevive() const noexcept
{
	if (m_pkMobInst != nullptr)
		return m_pkMobInst->m_IsRevive;

	return false;
}

void CHARACTER::SetRevive(bool mode) noexcept
{
	if (m_pkMobInst != nullptr)
		m_pkMobInst->m_IsRevive = mode;
}

#define IS_SPEED_HACK_PLAYER(ch) (ch->m_speed_hack_count > SPEEDHACK_LIMIT_COUNT)

EVENTFUNC(check_speedhack_event)
{
	const char_event_info* info = dynamic_cast<char_event_info*>(event->info);
	if (info == nullptr)
	{
		sys_err("check_speedhack_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (nullptr == ch || ch->IsNPC())
		return 0;

	if (IS_SPEED_HACK_PLAYER(ch))
	{
		// write hack log
		LogManager::Instance().SpeedHackLog(ch->GetPlayerID(), ch->GetX(), ch->GetY(), ch->m_speed_hack_count);

		if (g_bEnableSpeedHackCrash)
		{
			// close connection
			LPDESC desc = ch->GetDesc();

			if (desc)
			{
				DESC_MANAGER::Instance().DestroyDesc(desc);
				return 0;
			}
		}
	}

	ch->m_speed_hack_count = 0;

	ch->ResetComboHackCount();
	return PASSES_PER_SEC(60);
}

void CHARACTER::StartCheckSpeedHackEvent()
{
	if (m_pkCheckSpeedHackEvent)
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();

	info->ch = this;

	m_pkCheckSpeedHackEvent = event_create(check_speedhack_event, info, PASSES_PER_SEC(60));
}

void CHARACTER::GoHome()
{
	WarpSet(EMPIRE_START_X(GetEmpire()), EMPIRE_START_Y(GetEmpire()));
}

void CHARACTER::SendGuildName(const CGuild* pGuild)
{
	if (nullptr == pGuild)
		return;

	DESC* desc = GetDesc();
	if (nullptr == desc)
		return;

	if (m_known_guild.find(pGuild->GetID()) != m_known_guild.end())
		return;

	m_known_guild.insert(pGuild->GetID());

	TPacketGCGuildName pack;
	memset(&pack, 0, sizeof(pack));

	pack.header = HEADER_GC_GUILD;
	pack.subheader = GUILD_SUBHEADER_GC_GUILD_NAME;
	pack.size = sizeof(TPacketGCGuildName);
	pack.guildID = pGuild->GetID();
	memcpy(pack.guildName, pGuild->GetName(), GUILD_NAME_MAX_LEN);
#ifdef ENABLE_COLORED_GUILD_RANKS
	pack.guildRank = CGuildManager::Instance().GetRank(pGuild);
#endif
	desc->Packet(&pack, sizeof(pack));
}

void CHARACTER::SendGuildName(uint32_t dwGuildID)
{
	SendGuildName(CGuildManager::Instance().FindGuild(dwGuildID));
}

EVENTFUNC(destroy_when_idle_event)
{
	const char_event_info* info = dynamic_cast<char_event_info*>(event->info);
	if (info == nullptr)
	{
		sys_err("destroy_when_idle_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;
	if (ch == nullptr) // <Factor>
		return 0;

	if (ch->GetVictim())
		return PASSES_PER_SEC(300);

	sys_log(1, "DESTROY_WHEN_IDLE: %s", ch->GetName());

	ch->m_pkDestroyWhenIdleEvent = nullptr;
	M2_DESTROY_CHARACTER(ch);
	return 0;
}

void CHARACTER::StartDestroyWhenIdleEvent()
{
	if (m_pkDestroyWhenIdleEvent)
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();

	info->ch = this;

	m_pkDestroyWhenIdleEvent = event_create(destroy_when_idle_event, info, PASSES_PER_SEC(300));
}

void CHARACTER::SetComboSequence(uint8_t seq) noexcept
{
	m_bComboSequence = seq;
}

uint8_t CHARACTER::GetComboSequence() const noexcept
{
	return m_bComboSequence;
}

void CHARACTER::SetLastComboTime(uint32_t time) noexcept
{
	m_dwLastComboTime = time;
}

uint32_t CHARACTER::GetLastComboTime() const noexcept
{
	return m_dwLastComboTime;
}

void CHARACTER::SetValidComboInterval(int interval) noexcept
{
	m_iValidComboInterval = interval;
}

int CHARACTER::GetValidComboInterval() const noexcept
{
	return m_iValidComboInterval;
}

uint8_t CHARACTER::GetComboIndex() const noexcept
{
	return m_bComboIndex;
}

void CHARACTER::IncreaseComboHackCount(int k)
{
	m_iComboHackCount += k;
	if (m_iComboHackCount >= 10)
	{
		if (GetDesc())
		{
			if (GetDesc()->DelayedDisconnect(number(2, 7)))
			{
				sys_log(0, "COMBO_HACK_DISCONNECT: %s count: %d", GetName(), m_iComboHackCount);
				LogManager::Instance().HackLog("Combo", this);
			}
		}
	}
}

void CHARACTER::ResetComboHackCount() noexcept
{
	m_iComboHackCount = 0;
}

void CHARACTER::SkipComboAttackByTime(int interval) noexcept
{
	m_dwSkipComboAttackByTime = get_dword_time() + interval;
}

uint32_t CHARACTER::GetSkipComboAttackByTime() const noexcept
{
	return m_dwSkipComboAttackByTime;
}

void CHARACTER::ResetChatCounter() noexcept
{
	m_bChatCounter = 0;
}

uint8_t CHARACTER::IncreaseChatCounter() noexcept
{
	return ++m_bChatCounter;
}

uint8_t CHARACTER::GetChatCounter() const noexcept
{
	return m_bChatCounter;
}

// Are you riding a horse or something?
bool CHARACTER::IsRiding() const
{
	return IsHorseRiding() || GetMountVnum();
}

bool CHARACTER::CanWarp() const noexcept
{
	const int iPulse = thecore_pulse();
	const int limit_time = PASSES_PER_SEC(g_nPortalLimitTime);

	if ((iPulse - GetSafeboxLoadTime()) < limit_time)
		return false;

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	if ((iPulse - GetGuildstorageLoadTime()) < limit_time)
		return false;
#endif

	if ((iPulse - GetExchangeTime()) < limit_time)
		return false;

	if ((iPulse - GetMyShopTime()) < limit_time)
		return false;

#ifdef ENABLE_MAILBOX
	if ((iPulse - GetMyMailBoxTime()) < limit_time)
		return false;
#endif

	if ((iPulse - GetRefineTime()) < limit_time)
		return false;

	if (GetOpenedWindow(W_EXCHANGE | W_SAFEBOX | W_CUBE | W_SHOP_OWNER
#ifndef ENABLE_PREMIUM_PRIVATE_SHOP
		| W_MYSHOP
#endif
#ifdef ENABLE_SKILLBOOK_COMBINATION
		| W_SKILLBOOK_COMB
#endif
#ifdef ENABLE_ATTR_6TH_7TH
		| W_ATTR_6TH_7TH
#endif
#ifdef ENABLE_AURA_SYSTEM
		| W_AURA
#endif
#ifdef ENABLE_SWITCHBOT
		| W_SWITCHBOT
#endif
	))
		return false;

	return true;
}

uint32_t CHARACTER::GetNextExp() const noexcept
{
	if (PLAYER_MAX_LEVEL_CONST < GetLevel())
		return 2500000000u;
	else
		return exp_table[GetLevel()];
}

#ifdef ENABLE_YOHARA_SYSTEM
uint32_t CHARACTER::GetConquerorNextExp() const noexcept
{
	if (PLAYER_CONQUEROR_LEVEL + 1 < GetConquerorLevel())
		return 188537554;
	else
		return conqueror_exp_table[GetConquerorLevel()];
}
#endif

int CHARACTER::GetSkillPowerByLevel(int level, bool bMob) const
{
	return CTableBySkill::Instance().GetSkillPowerByLevelFromType(GetJob(), GetSkillGroup(), MINMAX(0, level, SKILL_MAX_LEVEL), bMob);
}

bool CHARACTER::IsInSafezone() const
{
	LPSECTREE sectree = GetSectree();
	return (sectree && sectree->IsAttr(GetX(), GetY(), ATTR_BANPK));
}

void CHARACTER::ban(char* reason2, unsigned int myid2)
{
	uint32_t myself = GetAID();
	char query[1024];
	snprintf(query, sizeof(query), "UPDATE account.account SET status='BLOCK' WHERE id='%u'", myself);
	DBManager::Instance().Query(query);

	char query2[1024];
	snprintf(query2, sizeof(query), "INSERT INTO account.ban_list (account, reason, source, date, action) VALUES('%u', '%s', '%u', NOW(), 'ban')", myself, reason2, myid2);
	//const char * query2 = "INSERT INTO account.ban_list (account, reason, source) VALUES('%d', '%d', '%s')", GetAID(), myid2, reason2;
	DBManager::Instance().Query(query2);

	char resulttxt[1024];
	snprintf(resulttxt, sizeof(query), "%s %s", LC_TEXT("You were banned! Reason:"), reason2);
	ChatPacket(CHAT_TYPE_INFO, resulttxt);
	GetDesc()->DelayedDisconnect(3);
}

void CHARACTER::unban(char* name)
{
	char query[1024];
	snprintf(query, sizeof(query), "UPDATE account.account INNER JOIN player.player ON player.account_id=account.id SET status='OK' WHERE player.name='%s'", name);
	//snprintf(query, 1024, "UPDATE account.account SET status='OK' WHERE id='%u'", GetAID());
	DBManager::Instance().Query(query);

	char query2[1024];
	snprintf(query2, sizeof(query), "INSERT INTO account.ban_list (account, reason, source, date, action) VALUES(0, '%s', '%u', NOW(), 'unban')", name, GetPlayerID());
	//snprintf(query, 1024, "UPDATE account.account SET status='OK' WHERE id='%u'", GetAID());
	DBManager::Instance().Query(query2);
}

bool CHARACTER::IsRestricted()	//@fixme500
{
	if (GetQuestFlag("restrict.player") < get_global_time())
		return false;

	return true;
}

#ifdef ENABLE_12ZI
void CHARACTER::BeadTime()
{
	int animaSphere = GetAnimaSphere();
	int lastTime = GetQuestFlag("12zi_temple.beadtime");
	int remainTime = 3600 - (get_global_time() - lastTime);

	if (animaSphere >= 36)
	{
		ChatPacket(CHAT_TYPE_COMMAND, "Bead_count %d", GetAnimaSphere());
		ChatPacket(CHAT_TYPE_COMMAND, "Bead_time %d", 0);
		return;
	}

	if ((animaSphere == 0) && (lastTime == 0))
	{
		SetAnimaSphere(36);
		SetQuestFlag("12zi_temple.beadtime", get_global_time());
		ChatPacket(CHAT_TYPE_COMMAND, "Bead_count %d", GetAnimaSphere());
		ChatPacket(CHAT_TYPE_COMMAND, "Bead_time %d", 0);
		return;
	}

	if (animaSphere < 36 && ((get_global_time() - lastTime) > 3600))
	{
		const int iTime = get_global_time() - lastTime;
		const int iCount = iTime/3600;

		if ((animaSphere + iCount) <= 36)
		{
			SetAnimaSphere(iCount);
		}
		else if ((animaSphere + iCount) > 36)
		{
			int jCount = 36-animaSphere;
			if (jCount <= 36)
				SetAnimaSphere(jCount);

			ChatPacket(CHAT_TYPE_COMMAND, "Bead_count %d", GetAnimaSphere());
			ChatPacket(CHAT_TYPE_COMMAND, "Bead_time %d", 0);
		}

		SetQuestFlag("12zi_temple.beadtime", get_global_time());
		ChatPacket(CHAT_TYPE_COMMAND, "Bead_count %d", GetAnimaSphere());
		ChatPacket(CHAT_TYPE_COMMAND, "Bead_time %d", remainTime);
		return;
	}

	ChatPacket(CHAT_TYPE_COMMAND, "Bead_count %d", animaSphere);
	ChatPacket(CHAT_TYPE_COMMAND, "Bead_time %d", remainTime);
}

void CHARACTER::MarkTime()
{
	int markLastTime = GetQuestFlag("12zi_temple.MarkTime");
	int markRemainTime = markLastTime - get_global_time();

	if (markRemainTime == 0)
	{
		if (FindAffect(AFFECT_CZ_UNLIMIT_ENTER))
			RemoveAffect(AFFECT_CZ_UNLIMIT_ENTER);
	}
	else
	{
		RemoveAffect(AFFECT_CZ_UNLIMIT_ENTER);
		AddAffect(AFFECT_CZ_UNLIMIT_ENTER, 0, 0, AFF_CZ_UNLIMIT_ENTER, markRemainTime, 0, false);
	}
}

int CHARACTER::GetAnimaSphere()
{
	auto pMsg(DBManager::Instance().DirectQuery("SELECT bead FROM player.player WHERE id = '%d';", GetPlayerID()));
	if (pMsg->Get()->uiNumRows == 0)
		return 0;

	MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
	int bBead = 0;
	str_to_number(bBead, row[0]);
	return bBead;
}

void CHARACTER::SetAnimaSphere(int amount)
{
	int value = abs(amount);

	if (amount > 0)
	{
		DBManager::Instance().DirectQuery("UPDATE player.player SET bead = bead + '%d' WHERE id = '%d'", value, GetPlayerID());
	}
	else
	{
		SetQuestFlag("12zi_temple.beadtime", get_global_time());
		DBManager::Instance().DirectQuery("UPDATE player.player SET bead = bead - '%d' WHERE id = '%d'", value, GetPlayerID());
	}

	ChatPacket(CHAT_TYPE_COMMAND, "Bead_count %d", GetAnimaSphere());
}

void CHARACTER::IsZodiacEffectMob()
{
	if (!this)
		return;

	if (!IsMonster())
		return;

	if (IsDead())
		return;

	uint32_t Monster = GetRaceNum();

	if (!Monster || Monster == 0)
		return;

	if (Monster == 2750 || Monster == 2860) //Officer (Zi or Hai)
	{
		if (number(1, 2) == 1)
		{
			EffectPacket(SE_SKILL_DAMAGE_ZONE);
		}
		else
		{
			EffectPacket(SE_SKILL_SAFE_ZONE);
		}
	}
}

void CHARACTER::IsZodiacEffectPC(uint32_t Monster)
{
	if (!this)
		return;

	if (!IsPC())
		return;

	if (IsDead())
		return;

	if (!Monster || Monster == 0)
		return;

	if (!GetDesc() || !GetDesc()->GetCharacter())
	{
		sys_err("Character::IsZodiacEffectPC : cannot get desc or character");
		return;
	}

	if (Monster == 20464) //Canon
		EffectPacket(SE_DEAPO_BOOM);
	else if (Monster == 2770 || Monster == 2771 || Monster == 2772) //Yin
		EffectPacket(SE_METEOR);
	else if (Monster == 2790 || Monster == 2791 || Monster == 2792) //Chen
		EffectPacket(SE_BEAD_RAIN);
	else if (Monster == 2830 || Monster == 2831 || Monster == 2832) //Shen
		EffectPacket(SE_FALL_ROCK);
	else if (Monster == 2800 || Monster == 2801 || Monster == 2802) //Si
		EffectPacket(SE_ARROW_RAIN);
	else if (Monster == 2810 || Monster == 2811 || Monster == 2812) //Wu
		EffectPacket(SE_HORSE_DROP);
	else if (Monster == 2840 || Monster == 2841 || Monster == 2842) //Yu
		EffectPacket(SE_EGG_DROP);
}

void CHARACTER::ZodiacFloorMessage(uint8_t Floor)
{
	if (!IsPC())
		return;

#ifdef ENABLE_CHAT_MISSION_ALTERNATIVE
	if ((Floor >= 1 && Floor <= 5) || (Floor == 9) || (Floor == 10) || (Floor == 16) || (Floor == 20) || (Floor == 23) || (Floor == 25) || (Floor == 26) || (Floor >= 31 && Floor <= 33))
		ChatPacket(CHAT_TYPE_MISSION, "[LS;246]");
	else if (Floor == 6)
		ChatPacket(CHAT_TYPE_MISSION, "[LS;256]");
	else if (Floor == 7 || Floor == 21)
		ChatPacket(CHAT_TYPE_MISSION, "[LS;262]");
	else if (Floor == 8 || Floor == 27 || Floor == 30)
		ChatPacket(CHAT_TYPE_MISSION, "[LS;242]");
	else if (Floor == 11 || Floor == 17)
		ChatPacket(CHAT_TYPE_MISSION, "[LS;254]");
	else if (Floor == 12 || Floor == 19 || Floor == 24)
		ChatPacket(CHAT_TYPE_MISSION, "[LS;257]");
	else if (Floor == 13 || Floor == 18 || Floor == 29)
		ChatPacket(CHAT_TYPE_MISSION, "[LS;234]");
	else if (Floor == 14 || Floor == 28)
		ChatPacket(CHAT_TYPE_MISSION, "[LS;235]");
	else if (Floor == 15 || Floor == 34)
		ChatPacket(CHAT_TYPE_MISSION, "[LS;248]");
	else if (Floor == 22)
		ChatPacket(CHAT_TYPE_MISSION, "[LS;252]");
	else if (Floor >= 35 && Floor <= 39)
		ChatPacket(CHAT_TYPE_MISSION, "[LS;261]");
	else if (Floor == 40)
		ChatPacket(CHAT_TYPE_MISSION, "[LS;239]");
	else if (Floor == 41)
		ChatPacket(CHAT_TYPE_MISSION, "[LS;225]");
	else if (Floor == 42)
		ChatPacket(CHAT_TYPE_MISSION, "[LS;251]");
	else if (Floor == 43)
		ChatPacket(CHAT_TYPE_MISSION, "[LS;227]");
#endif
}

void CHARACTER::EffectZodiacPacket(long X, long Y, int enumEffectType, int enumEffectType2)
{
	TPacketGCSpecialZodiacEffect p;

	p.header = HEADER_GC_SEPCIAL_ZODIAC_EFFECT;
	p.type = enumEffectType;
	p.type2 = enumEffectType2;
	p.vid = GetVID();
	p.x = X;
	p.y = Y;

	PacketAround(&p, sizeof(p));
}

uint32_t CHARACTER::CountZodiacItems(uint32_t Vnum)
{
	auto pMsg(DBManager::Instance().DirectQuery("SELECT count FROM player.zodiac_npc WHERE item_vnum = '%u' and owner_id = '%d'", Vnum, GetPlayerID()));
	if (pMsg->Get()->uiNumRows == 0)
		return 0;

	MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
	uint32_t dwCount = 0;
	str_to_number(dwCount, row[0]);
	return dwCount;
}

void CHARACTER::SetZodiacItems(uint32_t Vnum, int Count)
{
	auto pMsg(DBManager::Instance().DirectQuery("SELECT owner_id FROM player.zodiac_npc WHERE item_vnum = '%u' and owner_id = '%u'", Vnum, GetPlayerID()));
	if (pMsg->Get()->uiNumRows == 0)
	{
		char szQuery[512];
		snprintf(szQuery, sizeof(szQuery), "INSERT INTO player.zodiac_npc(owner_id, item_vnum, count) VALUES(%u, %u, %d)", GetPlayerID(), Vnum, Count);
		DBManager::Instance().DirectQuery(szQuery);
		return;
	}
	else
	{
		char szQuery2[512];
		snprintf(szQuery2, sizeof(szQuery2), "UPDATE player.zodiac_npc SET count = '%d' WHERE item_vnum = %u and owner_id = '%u'", Count, Vnum, GetPlayerID());
		DBManager::Instance().DirectQuery(szQuery2);
	}
}

uint32_t CHARACTER::PurchaseCountZodiacItems(uint32_t Vnum)
{
	auto pMsg(DBManager::Instance().DirectQuery("SELECT count FROM player.zodiac_npc_sold WHERE item_vnum = '%u' and owner_id = '%d'", Vnum, GetPlayerID()));
	if (pMsg->Get()->uiNumRows == 0)
		return 0;

	MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
	uint32_t dwCount = 0;
	str_to_number(dwCount, row[0]);
	return dwCount;
}

void CHARACTER::SetPurchaseZodiacItems(uint32_t Vnum, int Count)
{
	auto pMsg(DBManager::Instance().DirectQuery("SELECT owner_id FROM player.zodiac_npc_sold WHERE item_vnum = '%u' and owner_id = '%u'", Vnum, GetPlayerID()));
	if (pMsg->Get()->uiNumRows == 0)
	{
		char szQuery[512];
		snprintf(szQuery, sizeof(szQuery), "INSERT INTO player.zodiac_npc_sold(owner_id, item_vnum, count) VALUES(%u, %u, %d)", GetPlayerID(), Vnum, Count);
		DBManager::Instance().DirectQuery(szQuery);
		return;
	}
	else
	{
		char szQuery2[512];
		snprintf(szQuery2, sizeof(szQuery2), "UPDATE player.zodiac_npc_sold SET count = '%d' WHERE item_vnum = '%u' and owner_id = '%u'", Count, Vnum, GetPlayerID());
		DBManager::Instance().DirectQuery(szQuery2);
	}
}
#endif

#ifdef ENABLE_MEDAL_OF_HONOR
void CHARACTER::CheckMedals()
{
	if ((get_global_time() - GetShopExDailyTimePoint(SHOP_COIN_TYPE_MEDAL_OF_HONOR)) > 86400)
	{
		SetShopExUsablePoint(SHOP_COIN_TYPE_MEDAL_OF_HONOR, LIMIT_DAILY_MAX_MEDALS);
		SetShopExDailyTimePoint(SHOP_COIN_TYPE_MEDAL_OF_HONOR, get_global_time());
	}
}
#endif

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
uint8_t CHARACTER::GetGemShopOpenSlotCount() noexcept
{
	if (m_bGemShopAllSlotUnlocked)
		return GEM_SHOP_HOST_ITEM_MAX_NUM;

	uint8_t bGemShopSlotCount = 0;
	for (uint8_t i = 0; i < GEM_SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		if (m_gemItems[i].bSlotUnlocked)
			bGemShopSlotCount++;
	}
	return bGemShopSlotCount;
}

uint8_t CHARACTER::GetGemShopOpenSlotItemCount(uint8_t slot_count)
{
	const uint8_t needCount[] = { 0, 0, 0, 1, 2, 4, 8, 8, 8 };
	return needCount[slot_count];
}

void CHARACTER::OpenGemShopFirstTime()
{
	for (uint8_t i = 0; i < GEM_SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		m_gemItems[i].bItemId = CShopManager::Instance().GemShopGetRandomId(i + 1);
	}

	for (uint8_t i = 0; i < GEM_SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		bool isEnable = ((i < 3) ? true : false);
		m_gemItems[i].bSlotStatus = isEnable;
		m_gemItems[i].bSlotUnlocked = isEnable;
	}

	const time_t next_time = get_global_time() + GEM_REFRESH_PULSE;
	SetGemNextRefresh(next_time);
}

void CHARACTER::RefreshGemShopItems()
{
	for (uint8_t i = 0; i < GEM_SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		m_gemItems[i].bItemId = CShopManager::Instance().GemShopGetRandomId(i + 1);
		m_gemItems[i].bSlotStatus = m_gemItems[i].bSlotUnlocked ? 1 : 0;
	}

	const time_t next_time = get_global_time() + GEM_REFRESH_PULSE;
	SetGemNextRefresh(next_time);
}

void CHARACTER::RefreshGemShopWithItem()
{
	if (CountSpecifyItem(GEM_REFRESH_ITEM_VNUM) < 1)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;180]");
		return;
	}

	RemoveSpecifyItem(GEM_REFRESH_ITEM_VNUM, 1);
	ChatPacket(CHAT_TYPE_INFO, "[LS;175]");

	for (uint8_t i = 0; i < GEM_SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		m_gemItems[i].bItemId = CShopManager::Instance().GemShopGetRandomId(i + 1);
		m_gemItems[i].bSlotStatus = m_gemItems[i].bSlotUnlocked ? 1 : 0;
	}

	const time_t next_time = get_global_time() + GEM_REFRESH_PULSE;
	SetGemNextRefresh(next_time);

	RefreshGemShop();
}

void CHARACTER::GemShopBuy(uint8_t bPos)
{
	if (bPos >= GEM_SHOP_HOST_ITEM_MAX_NUM)
		return;

	const uint32_t dwVnum = CShopManager::Instance().GemShopGetVnumById(m_gemItems[bPos].bItemId);
	const uint8_t bCount = CShopManager::Instance().GemShopGetCountById(m_gemItems[bPos].bItemId);
	const uint32_t dwPrice = CShopManager::Instance().GemShopGetPriceById(m_gemItems[bPos].bItemId);

	if (GetGemPoint() < dwPrice)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;177]");
		return;
	}

	if ((!m_bGemShopAllSlotUnlocked && (m_gemItems[bPos].bSlotStatus == 0 || m_gemItems[bPos].bSlotUnlocked == 0)) || m_gemItems[bPos].bSlotStatus == 2)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;178]");
		return;
	}

	LPITEM item = ITEM_MANAGER::Instance().CreateItem(dwVnum, bCount);

	if (item)
	{
		int iEmptyPos = -1;
		if (item->IsDragonSoul())
			iEmptyPos = GetEmptyDragonSoulInventory(item);
		else
#ifdef ENABLE_SPECIAL_INVENTORY
			iEmptyPos = GetEmptyInventory(item);
#else
			iEmptyPos = GetEmptyInventory(item->GetSize());
#endif

		if (iEmptyPos < 0)
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;179]");
			return;
		}

		PointChange(POINT_GEM, -dwPrice, false);
#ifdef ENABLE_BATTLE_PASS_SYSTEM
		UpdateExtBattlePassMissionProgress(GAYA_BUY_ITEM_GAYA_COST, dwPrice, 0);
#endif

		if (item->IsDragonSoul())
			item->AddToCharacter(this, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
		else
			item->AddToCharacter(this, TItemPos(INVENTORY, iEmptyPos));

		ITEM_MANAGER::Instance().FlushDelayedSave(item);

		m_gemItems[bPos].bSlotStatus = 2;

		TPacketGCGemShopBuy pack{};
		pack.header = HEADER_GC_GEM_SHOP_BUY;
		pack.bSlotIndex = bPos;
		pack.bEnable = 0;

		if (GetDesc())
			GetDesc()->Packet(&pack, sizeof(TPacketGCGemShopBuy));
	}
}

void CHARACTER::GemShopAdd()
{
	if (GetGemShopOpenSlotCount() >= GEM_SHOP_HOST_ITEM_MAX_NUM)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;182]");
		return;
	}

	for (uint8_t i = 0; i < GEM_SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		if (m_gemItems[i].bSlotUnlocked == 0)
		{
			const uint8_t needCount = GetGemShopOpenSlotItemCount(i);

			const uint32_t dwGemUnlockVnum = GEM_UNLOCK_ITEM_VNUM;
			if (CountSpecifyItem(dwGemUnlockVnum) < needCount)
			{
				const TItemTable* pTable = ITEM_MANAGER::Instance().GetTable(dwGemUnlockVnum);
				if (pTable)
				{
					ChatPacket(CHAT_TYPE_INFO, "[LS;183;[IN;%d];%d]", dwGemUnlockVnum, (needCount - CountSpecifyItem(dwGemUnlockVnum)));
				}

				return;
			}

			RemoveSpecifyItem(dwGemUnlockVnum, needCount);

			m_gemItems[i].bSlotStatus = 1;
			m_gemItems[i].bSlotUnlocked = 1;

			TPacketGCGemShopAdd pack{};
			pack.header = HEADER_GC_GEM_SHOP_ADD;
			pack.bSlotIndex = i;
			pack.bEnable = 1;
			GetDesc()->Packet(&pack, sizeof(TPacketGCGemShopAdd));

			break;
		}
	}
}

void CHARACTER::RefreshGemShop()
{
	TPacketGCGemShopItems pack{};
	pack.header = HEADER_GC_GEM_SHOP_REFRESH;
	pack.nextRefreshTime = GetGemNextRefresh();
	pack.bGemShopOpenSlotCount = GetGemShopOpenSlotCount();

	for (uint8_t i = 0; i < GEM_SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		pack.gemShopItems[i].display_pos = i;
		pack.gemShopItems[i].status = m_gemItems[i].bSlotStatus;
		pack.gemShopItems[i].unlocked = m_gemItems[i].bSlotUnlocked;
		pack.bGemShopOpenSlotItemCount[i] = GetGemShopOpenSlotItemCount(i);

		if (m_bGemShopAllSlotUnlocked)
		{
			if (m_gemItems[i].bSlotUnlocked == 0 && m_gemItems[i].bSlotStatus == 0)
				pack.gemShopItems[i].status = 1;
		}

		pack.gemShopItems[i].vnum = CShopManager::Instance().GemShopGetVnumById(m_gemItems[i].bItemId);
		pack.gemShopItems[i].count = CShopManager::Instance().GemShopGetCountById(m_gemItems[i].bItemId);
		pack.gemShopItems[i].price = CShopManager::Instance().GemShopGetPriceById(m_gemItems[i].bItemId);
	}

	if (GetDesc())
		GetDesc()->Packet(&pack, sizeof(TPacketGCGemShopItems));
}

void CHARACTER::OpenGemShop(bool bAllSlotUnlocked)
{
	m_bGemShopAllSlotUnlocked = bAllSlotUnlocked;

	if (GetGemNextRefresh() == 0)
		OpenGemShopFirstTime();

	if (GetGemNextRefresh() <= get_global_time())
		RefreshGemShopItems();

	TPacketGCGemShopItems pack{};
	pack.header = HEADER_GC_GEM_SHOP_OPEN;
	pack.nextRefreshTime = GetGemNextRefresh();
	pack.bGemShopOpenSlotCount = GetGemShopOpenSlotCount();

	for (uint8_t i = 0; i < GEM_SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		pack.gemShopItems[i].display_pos = i;
		pack.gemShopItems[i].status = m_gemItems[i].bSlotStatus < 2 ? m_gemItems[i].bSlotStatus : 0;
		pack.gemShopItems[i].unlocked = m_gemItems[i].bSlotUnlocked;
		pack.bGemShopOpenSlotItemCount[i] = GetGemShopOpenSlotItemCount(i);

		if (m_bGemShopAllSlotUnlocked)
		{
			if (m_gemItems[i].bSlotStatus == 2)
				pack.gemShopItems[i].status = 0;
			else if (m_gemItems[i].bSlotUnlocked == 0 && m_gemItems[i].bSlotStatus == 0)
				pack.gemShopItems[i].status = 1;
		}

		pack.gemShopItems[i].vnum = CShopManager::Instance().GemShopGetVnumById(m_gemItems[i].bItemId);
		pack.gemShopItems[i].count = CShopManager::Instance().GemShopGetCountById(m_gemItems[i].bItemId);
		pack.gemShopItems[i].price = CShopManager::Instance().GemShopGetPriceById(m_gemItems[i].bItemId);
	}

	if (GetDesc())
		GetDesc()->Packet(&pack, sizeof(TPacketGCGemShopItems));
}

bool CHARACTER::CreateGem(int glimmerstone_count, LPITEM metinstone_item, int cost, uint8_t pct)
{
	if (!this)
		return false;

	if (!metinstone_item)
		return false;

	if (CountSpecifyItem(GEM_GLIMMERSTONE_VNUM) < glimmerstone_count)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;170]");
		return false;
	}

	if (GetGold() < cost)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;171]");
		return false;
	}

	if ((GetGemPoint() + 3) >= (GEM_MAX - 1))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;172]");
		return false;
	}

	RemoveSpecifyItem(GEM_GLIMMERSTONE_VNUM, glimmerstone_count);

	if (metinstone_item)
	{
#ifdef ENABLE_STACKABLES_STONES
		metinstone_item->SetCount(metinstone_item->GetCount() - 1);
#else
		ITEM_MANAGER::Instance().RemoveItem(metinstone_item, "REMOVE (GAYA)");
#endif
	}

	PointChange(POINT_GOLD, -cost, false);

	if (number(1, 100) <= pct)
	{
		PointChange(POINT_GEM, 3, false);
#ifdef ENABLE_BATTLE_PASS_SYSTEM
		UpdateExtBattlePassMissionProgress(GAYA_CRAFT_GAYA, 3, 0);
#endif
		ChatPacket(CHAT_TYPE_INFO, "[LS;173]");
		return true;
	}
	else
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;174]");
		return false;
	}

	return false;
	}
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
void CHARACTER::OpenPrivateShopSearch(uint32_t dwVnum)
{
	if (GetDesc() == nullptr)
		return;

	if (IsPrivateShopSearchOpen())
		return;

	if ((GetExchange() || IsOpenSafebox()
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		|| GetViewingShopOwner())
#else
		|| GetShopOwner())
#endif
		|| IsCubeOpen()
#ifdef ENABLE_CHANGED_ATTR
		|| IsSelectAttr()
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		|| IsChangeLookWindowOpen()
#endif
		)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANT_DO_THIS_BECAUSE_OTHER_WINDOW_OPEN"));
		return;
	}

	TPacketGCPrivateShopSearchOpen p{};
	p.header = HEADER_GC_PRIVATE_SHOP_SEARCH_OPEN;

	switch (dwVnum)
	{
		case PRIVATE_SHOP_SEARCH_TRADING_GLASS:
			p.subheader = 1;
			break;

		default:
			p.subheader = 0;
			break;
	}

	GetDesc()->Packet(&p, sizeof(p));
}
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
void CHARACTER::SetSkillColor(uint32_t* dwSkillColor)
{
	memcpy(m_dwSkillColor, dwSkillColor, sizeof(m_dwSkillColor));
	UpdatePacket();
}
#endif

#ifdef ENABLE_BIOLOG_SYSTEM
EVENTFUNC(reminder_event)
{
	const char_event_info* info = dynamic_cast<char_event_info*>(event->info);
	if (info == nullptr)
	{
		sys_err("<reminder_event_data> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;
	if (ch == nullptr) // <Factor>
		return 0;

	if (ch->IsBiologRemiderEvent())
		return 0;

	const TBiologMissionsProto* pMission = CBiologSystemManager::Instance().GetMission(ch->GetBiologMissions());
	if (!pMission)
		return 0;

	if (ch->GetLevel() < pMission->bRequiredLevel)
		return 0;

	if (get_global_time() > ch->GetBiologCooldown())
	{
		LPDESC desc = ch->GetDesc();
		if (desc)
		{
			char buf[255];
			int len;
			len = snprintf(buf, sizeof(buf), LC_TEXT("You can deliver the next material."));
			const CMob* pkMob = CMobManager::Instance().Get(pMission->dwNpcVnum);

			char szNpcName[CHARACTER_NAME_MAX_LEN + 1];
			snprintf(szNpcName, sizeof(szNpcName), pkMob ? pkMob->m_table.szLocaleName : "[Biologist]");

			TPacketGCWhisper pack{};
			pack.bHeader = HEADER_GC_WHISPER;
			pack.bType = WHISPER_TYPE_SYSTEM;
			pack.wSize = sizeof(TPacketGCWhisper) + len;
			strlcpy(pack.szNameFrom, szNpcName, sizeof(pack.szNameFrom));
			desc->BufferedPacket(&pack, sizeof(pack));
			desc->Packet(buf, len);

			ch->SetBiologRemiderEvent(true);
			return 0;
		}
	}

	return PASSES_PER_SEC(1);
}

void CHARACTER::SetBiologCooldownReminder(uint8_t value)
{
	m_BiologCooldownReminder = value;

	if (s_pkReminderEvent)
		event_cancel(&s_pkReminderEvent);

	if (value == 0)
		return;

	if (IsBiologRemiderEvent())
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();
	info->ch = this;
	s_pkReminderEvent = event_create(reminder_event, info, PASSES_PER_SEC(3));
}
#endif

#if defined(ENABLE_CHEQUE_SYSTEM) && defined(ENABLE_CHEQUE_EXCHANGE_WINDOW)
void CHARACTER::WonExchange(uint8_t bOption, uint16_t wValue)
{
	if (!CanWarp())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot execute the exchange process during you are doing something else."));
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Close every window and/or wait, then try it again."));
		return;
	}

	const auto fMul = (bOption == WON_EXCHANGE_CG_SUBHEADER_BUY) ? 1.0f + (static_cast<float>(TAX_MUL) / 100.0f) : 1.0f;
	const auto wVal = abs(wValue);
	if (bOption == WON_EXCHANGE_CG_SUBHEADER_SELL)
	{
		if (GetCheque() < static_cast<uint32_t>(wVal))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You don't have enough Won to proceed the exchange."));
			return;
		}
		if (GetGold() + static_cast<long long>(wVal) * static_cast<long long>(CHEQUE_NAME_VALUE * fMul) > GOLD_MAX)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot proceed the exchange because your Yang would step over the limit."));
			return;
		}

		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("The exchange was succesfull."));
		PointChange(POINT_GOLD, static_cast<long long>(wVal) * static_cast<long long>(CHEQUE_NAME_VALUE * fMul), true);
		PointChange(POINT_CHEQUE, -wVal, true);
	}
	else if (bOption == WON_EXCHANGE_CG_SUBHEADER_BUY)
	{
		if (GetGold() < static_cast<long long>(wVal) * static_cast<long long>(CHEQUE_NAME_VALUE * fMul))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You don't have enough Yang to proceed the exchange."));
			return;
		}
		if (GetCheque() + wVal > CHEQUE_MAX - 1)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot proceed the exchange because your Won would step over the limit."));
			return;
		}

		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("The exchange was succesfull."));
		PointChange(POINT_GOLD, -(static_cast<long long>(wVal) * static_cast<long long>(CHEQUE_NAME_VALUE * fMul)), true);
		PointChange(POINT_CHEQUE, wVal, true);
	}
	else
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Oups. Something went wrong. Unknown exchange operation."));
	}
}
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
void CHARACTER::RemoveShopItemByPos(uint8_t pos)
{
	TPacketPlayerShopSet pack{};
	pack.header = HEADER_GC_PLAYER_SHOP_SET;
	pack.pos = pos;

	pack.count = 0;
	pack.vnum = 0;
	pack.price = 0;

	memset(pack.alSockets, 0, sizeof(pack.alSockets));
	memset(pack.aAttr, 0, sizeof(pack.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
	memset(pack.aApplyRandom, 0, sizeof(pack.aApplyRandom));
	memset(pack.alRandomValues, 0, sizeof(pack.alRandomValues));
#endif
	GetDesc()->Packet(&pack, sizeof(TPacketPlayerShopSet));
}

/*void CHARACTER::SetClearShop()
{
	m_shopItems.clear();
}*/

#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
void CHARACTER::ShopSellResult(uint32_t itemVnum, int amount, uint32_t gold, uint32_t cheque, uint8_t pos)
#else
void CHARACTER::ShopSellResult(uint32_t itemVnum, int amount, uint32_t gold, uint8_t pos)
#endif
{
	TItemTable* itemTable = ITEM_MANAGER::Instance().GetTable(itemVnum);
	if (!itemTable)
	{
		sys_err("Error informing of a sale of %d of %d for %d (Item does not exist)", amount, itemVnum, gold);
		return;
	}

	std::vector<TPlayerItem> shopItems = GetShopItems(); 	//Remove it from the list of the shop items on the receiver
	for (auto it = shopItems.begin(); it != shopItems.end(); ++it)
	{
		if (it->pos == pos)
		{
			shopItems.erase(it);
			break;
		}
	}
	SetShopItems(shopItems);

	// Inform
	ChatPacket(CHAT_TYPE_INFO, "[LS;1718;[IN;%d]]", itemVnum);
	ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Quantity sold: %d"), amount);

	// Inform
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	if (gold > 0 && cheque > 0)
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Total earnings: %d Yang and %d Won."), gold, cheque);
	else if (gold > 0)
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Total earnings: %d Yang."), gold);
	else if (cheque > 0)
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Total earnings: %d Won."), cheque);
#else
	ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Total earnings: %d Yang."), gold);
#endif

	if (shopItems.empty())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Your private shop has been closed after all items have been sold."));
#ifdef ENABLE_CHEQUE_SYSTEM //OFF_CHEQUE
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Total earnings: %d Yang and %d Won."), (gold + GetShopGoldStash()), (cheque + GetShopChequeStash()));
#else
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Total earnings: %d Yang."), (gold + GetShopGoldStash()));
#endif
	}

	// Remove the sold item from the shop UI
	RemoveShopItemByPos(pos);

	// Update stash local value and sync in client
	AlterShopGoldStash(gold);
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	AlterShopChequeStash(cheque);
#endif

	TPacketGCShopStashSync stash{};
	stash.bHeader = HEADER_GC_SYNC_SHOP_STASH;
	stash.value = GetShopGoldStash();
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	stash.cheque = GetShopChequeStash();
#endif
	GetDesc()->Packet(&stash, sizeof(TPacketGCShopStashSync));

	// Log
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	sys_log(0, "ShopSellNotify: %dx %u for %u gold %u cheque [%s] gold_stash [%d] cheque_stash [%d]", amount, itemVnum, gold, cheque, GetName(), stash.value, stash.cheque);
#else
	sys_log(0, "ShopSellNotify: %dx %u for %u gold [%s] stash [%d]", amount, itemVnum, gold, GetName(), stash.value);
#endif
}

EVENTINFO(shop_offline_event_info)
{
	DynamicCharacterPtr ch;
	int loops;
};

EVENTFUNC(shop_offline_event)
{
	shop_offline_event_info* info = dynamic_cast<shop_offline_event_info*>(event->info);
	if (!info)
	{
		sys_err("shop_offline_event char nullptr");
		return 0;
	}

	LPCHARACTER	ch = info->ch;
	if (!ch)
		return 0;

	ch->SetShopOfflineEventRunning(true);

	if (!ch->AlterShopOfflineTime(-1)) // Time ran out.
	{
		if (test_server)
			sys_log(0, "Closing shop #%u after having ran out of offline time", ch->GetPlayerID());

		ch->SetShopOfflineEventRunning(false);
		ch->GetMyShop()->SetClosed(true);
		ch->GetMyShop()->SaveOffline();
		ch->GetMyShop()->Save();
		//M2_DESTROY_CHARACTER(ch);
		return 0;
	}

	// Save every 5 loops (i.e 5 minutes)
	++info->loops;
	if (info->loops % 5 == 0)
		ch->GetMyShop()->SaveOffline();

	return PASSES_PER_SEC(60);
}

bool CHARACTER::AlterShopOfflineTime(int delta)
{
	if (!GetMyShop())
	{
		sys_err("No open shop to alter time on.");
		return false;
	}

	int offlineMinutes = GetMyShop()->GetOfflineMinutes();

	// Let's exhaust free minutes first
	if (offlineMinutes > 0)
	{
		offlineMinutes = MAX(0, offlineMinutes + delta);
		GetMyShop()->SetOfflineMinutes(offlineMinutes);
	}

	return offlineMinutes > 0; // Return whether the time has ran out.
}

int CHARACTER::GetShopOfflineTime()
{
	if (!GetMyShop())
		return 0;

	return GetMyShop()->GetOfflineMinutes();
}

void CHARACTER::StartShopOfflineEvent()
{
	shop_offline_event_info* info = AllocEventInfo<shop_offline_event_info>();
	info->ch = this;
	info->loops = 0;

	m_shopOfflineEvent = event_create(shop_offline_event, info, test_server ? PASSES_PER_SEC(1 * 60) : PASSES_PER_SEC(5 * 60));

	TPacketGGShopStartOffline p{};
	p.bHeader = HEADER_GG_SHOP_OFFLINE_START;
	p.bChannel = g_bChannel;
	p.lMapIndex = GetMapIndex();
	p.bEmpire = GetEmpire();
	p.dwPID = GetPlayerID();

	P2P_MANAGER::Instance().Send(&p, sizeof(TPacketGGShopStartOffline));
}

void CHARACTER::CancelShopOfflineEvent()
{
	if (!m_shopOfflineEvent)
		return;

	if (IsShopOfflineEventRunning()) // Only if the event did not already destroy the shop
		GetMyShop()->SaveOffline();

	event_cancel(&m_shopOfflineEvent);
	SetShopOfflineEventRunning(false);

	TPacketGGShopEndOffline p{};
	p.bHeader = HEADER_GG_SHOP_OFFLINE_END;
	p.dwPID = GetPlayerID();

	P2P_MANAGER::Instance().Send(&p, sizeof(TPacketGGShopEndOffline));
}

#ifdef ENABLE_OPEN_SHOP_WITH_PASSWORD
void CHARACTER::OpenShopEditorWithPassword(const LPCHARACTER& ch, bool bMatchPass)
{
	if (!ch || !ch->GetDesc())
		return;

	if (bMatchPass)
	{
		TPacketGCShop pack{};
		pack.header = HEADER_GC_SHOP;
		pack.subheader = SHOP_SUBHEADER_GC_OPEN_SHOP_EDITOR;

#ifdef ENABLE_MYSHOP_DECO
		TPacketGCMyPrivShopOpen pack2{};
		pack2.bCashItem = m_bMyPrivShopIsCashItem;

		const LPCHARACTER& myShopChar = CHARACTER_MANAGER::Instance().FindPCShopCharacterByPID(GetPlayerID());
		if (myShopChar && myShopChar->GetMyShop())
			if (myShopChar->GetMyShop()->IsShopDeco())
				pack2.bTabCount = 2;
			else
				pack2.bTabCount = 1;
		else
			pack2.bTabCount = 1;

		pack.size = sizeof(pack) + sizeof(pack2);

		if (GetDesc())
		{
			GetDesc()->BufferedPacket(&pack, sizeof(TPacketGCShop));
			GetDesc()->Packet(&pack2, sizeof(TPacketGCMyPrivShopOpen));
		}
#else
		pack.size = sizeof(TPacketGCShop);
		ch->GetDesc()->Packet(&pack, sizeof(pack));
#endif

		// Sync Offline Time
		if (GetDesc())
		{
			TPacketGCShopOffTimeSync pack3{};
			pack3.bHeader = HEADER_GC_SYNC_SHOP_OFFTIME;
			pack3.value = GetPremiumPrivateShopRemainSeconds();
			GetDesc()->Packet(&pack3, sizeof(TPacketGCShopOffTimeSync));
			//ChatPacket(7, "time %d", pack.value);
		}
	}
	else
	{
		TPacketGCSafeboxWrongPassword p{};
		p.bHeader = HEADER_GC_SAFEBOX_WRONG_PASSWORD;
		GetDesc()->Packet(&p, sizeof(p));
	}
}
#endif
#endif

#ifdef ENABLE_REFINE_ELEMENT	
uint8_t CHARACTER::GetElementsEffect()
{
	LPITEM w_weapon = GetWear(WEAR_WEAPON);
	if (w_weapon)
	{
		if (w_weapon->GetElementGrade() == 3)
		{
			return w_weapon->GetElementsType();
		}
	}
	return 0;
}
#endif

#ifdef ENABLE_AFTERDEATH_SHIELD
EVENTINFO(shield_countdown_data)
{
	int seconds;
	LPCHARACTER ch;
};

static LPEVENT timer_shieldcountdown_check = nullptr;

EVENTFUNC(ShieldCountdown_Timer_Event)
{
	shield_countdown_data* info = dynamic_cast<shield_countdown_data*>(event->info);

	if (info == nullptr)
	{
		sys_err("Afterdeath Shield - ERROR: ShieldCountdown_Timer_Event> <Factor> Time 0 - Error");
		return 0;
	}

	int* pTimer = &(info->seconds);
	LPCHARACTER ch = info->ch;

	if (ch == nullptr || ch->GetDesc() == nullptr || !ch->IsAffectFlag(AFF_AFTERDEATH_SHIELD))
	{
		if (!ch->IsAffectFlag(AFF_AFTERDEATH_SHIELD))
			ch->UpdatePacket();
		timer_shieldcountdown_check = nullptr;
		return 0;
	}

	if (*pTimer == 0)
	{
		ch->RemoveShieldAffect();
		ch->UpdatePacket();
	}
	else
	{
		--* pTimer;
		return passes_per_sec;
	}

	timer_shieldcountdown_check = nullptr;
	return 0;
}

void CHARACTER::StartShieldCountdownEvent(LPCHARACTER ch, int iSec)
{

	if (ch == nullptr || ch->GetDesc() == nullptr)
		return;

	shield_countdown_data* info = AllocEventInfo<shield_countdown_data>();
	info->seconds = iSec;
	info->ch = ch;
	timer_shieldcountdown_check = event_create(ShieldCountdown_Timer_Event, info, 1);
}
#endif

#ifdef ENABLE_BATTLE_FIELD
bool CHARACTER::SetBattleKill(uint32_t dwPlayerID)
{
	const auto it = m_BattleFieldKillMap.find(dwPlayerID);
	if (it != m_BattleFieldKillMap.end())
	{
		if (get_global_time() < it->second)
			return false;
	}

	m_BattleFieldKillMap.emplace(std::make_pair(dwPlayerID, (get_global_time() + TIME_BETWEEN_KILLS)));
	return true;
}
#endif

#ifdef ENABLE_PET_SYSTEM
void CHARACTER::PetSummon(LPITEM petItem)
{
	CPetSystem* petSystem = GetPetSystem();
	uint32_t mobVnum = 0;

	if (!petSystem || !petItem)
		return;

	if (!(petItem->GetType() == ITEM_PET && petItem->GetSubType() == PET_PAY))
		return;

	if (petItem->GetValue(0) != 0)
		mobVnum = petItem->GetValue(0);

	petSystem->Summon(mobVnum, petItem, false);
}

void CHARACTER::PetUnsummon(LPITEM petItem)
{
	CPetSystem* petSystem = GetPetSystem();
	uint32_t mobVnum = 0;

	if (!petSystem || !petItem)
		return;

	if (!(petItem->GetType() == ITEM_PET && petItem->GetSubType() == PET_PAY))
		return;

	if (petItem->GetValue(0) != 0)
		mobVnum = petItem->GetValue(0);

	petSystem->Unsummon(mobVnum);
}

void CHARACTER::CheckPet()
{
	CPetSystem* petSystem = GetPetSystem();
	LPITEM petItem = GetWear(WEAR_PET);
	uint32_t mobVnum = 0;

	if (!petSystem || !petItem)
		return;

	if (!(petItem->GetType() == ITEM_PET && petItem->GetSubType() == PET_PAY))
		return;

	if (petItem->GetValue(0) != 0)
		mobVnum = petItem->GetValue(0);

	if (petSystem->CountSummoned() == 0)
		petSystem->Summon(mobVnum, petItem, false);
}

bool CHARACTER::IsPetSummon()
{
	return (GetWear(WEAR_PET));
}
#endif

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
void CHARACTER::ExtendInvenRequest()
{
	// Max Inventory
	if (GetExtendInvenStage() >= INVENTORY_LOCK_COVER_COUNT)
	{
		TPacketGCExtendInvenResult pPacket{};
		pPacket.bHeader = HEADER_GC_EXTEND_INVEN_RESULT;
		pPacket.dwVnum = 0;
		pPacket.bMsgResult = EX_INVEN_FAIL_FOURTH_PAGE_STAGE_MAX;
		pPacket.bEnoughCount = 0;
		if (GetDesc())
			GetDesc()->Packet(&pPacket, sizeof(TPacketGCExtendInvenResult));
		return;
	}

	// Check Keys
	std::vector<uint32_t>vecNeedKeys;
	for (int i = INVENTORY_NEED_KEY_START; i <= (INVENTORY_LOCKED_PAGE_COUNT * INVENTORY_NEED_KEY_INCREASE) + 1; i++)
	{
		for (int j = 0; j < 3; j++)
			vecNeedKeys.emplace_back(i);
	}

	const uint8_t dwNeedKeys = vecNeedKeys[GetExtendInvenStage()];
	const uint8_t dwCountKeys = CountSpecifyItem(INVENTORY_OPEN_KEY_GOLD) + CountSpecifyItem(INVENTORY_OPEN_KEY_SILVER);
	if (dwCountKeys < dwNeedKeys)
	{
		TPacketGCExtendInvenResult pPacket{};
		pPacket.bHeader = HEADER_GC_EXTEND_INVEN_RESULT;
		pPacket.dwVnum = INVENTORY_OPEN_KEY_GOLD;
		pPacket.bMsgResult = EX_INVEN_FAIL_FALL_SHORT;
		pPacket.bEnoughCount = (dwNeedKeys - dwCountKeys);
		if (GetDesc())
			GetDesc()->Packet(&pPacket, sizeof(TPacketGCExtendInvenResult));
		return;
	}

	// Send Result
	TPacketGCExtendInvenResult pPacket{};
	pPacket.bHeader = HEADER_GC_EXTEND_INVEN_RESULT;
	pPacket.dwVnum = INVENTORY_OPEN_KEY_GOLD;
	pPacket.bMsgResult = EX_INVEN_SUCCESS;
	pPacket.bEnoughCount = dwNeedKeys;
	if (GetDesc())
		GetDesc()->Packet(&pPacket, sizeof(TPacketGCExtendInvenResult));
}

void CHARACTER::ExtendInvenUpgrade()
{
	// Max Inventory
	if (GetExtendInvenStage() >= INVENTORY_LOCK_COVER_COUNT)
	{
		// Max Stage
		//ChatPacket(CHAT_TYPE_INFO, "MAX_STAGE");
		return;
	}

	// Check Keys
	std::vector<uint32_t>vecNeedKeys;
	for (int i = INVENTORY_NEED_KEY_START; i <= (INVENTORY_LOCKED_PAGE_COUNT * INVENTORY_NEED_KEY_INCREASE) + 1; i++)
	{
		for (int j = 0; j < 3; j++)
			vecNeedKeys.emplace_back(i);
	}

	 uint8_t dwNeedKeys = vecNeedKeys[GetExtendInvenStage()];
	 const uint8_t dwCountKeys = CountSpecifyItem(INVENTORY_OPEN_KEY_GOLD) + CountSpecifyItem(INVENTORY_OPEN_KEY_SILVER);
	if (dwCountKeys >= dwNeedKeys)
	{
		while (dwNeedKeys)
		{
			uint32_t dwKeyVnum = INVENTORY_OPEN_KEY_GOLD;
			if (!CountSpecifyItem(dwKeyVnum))
				dwKeyVnum = INVENTORY_OPEN_KEY_SILVER;

			RemoveSpecifyItem(dwKeyVnum);
			dwNeedKeys--;
		}

		SetExtendInvenStage(GetExtendInvenStage() + 1);
		SendExtendInvenInfo();

		//ChatPacket(CHAT_TYPE_INFO, "SUCCESS");
	}
	else
	{
		//ChatPacket(CHAT_TYPE_INFO, "MISSING KEYS: %d", (dwNeedKeys - dwCountKeys));
		return;
	}
}

void CHARACTER::SendExtendInvenInfo()
{
	TPacketGCExtendInvenInfo pExtendInvenInfo{};
	pExtendInvenInfo.bHeader = HEADER_GC_EXTEND_INVEN_INFO;
	pExtendInvenInfo.bExtendStage = GetExtendInvenStage();
	pExtendInvenInfo.bExtendMax = GetExtendInvenMax();
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	for (int i = 0; i < 3; i++)
	{
		pExtendInvenInfo.bExtendSpecialStage[i] = GetExtendSpecialInvenStage(i);
		pExtendInvenInfo.bExtendSpecialMax[i] = GetExtendSpecialInvenMax(i);
	}
# endif

	if (GetDesc())
		GetDesc()->Packet(&pExtendInvenInfo, sizeof(pExtendInvenInfo));
}

# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
void CHARACTER::ExtendSpecialInvenRequest(uint8_t bWindow)
{
	// Max Inventory
	if (GetExtendSpecialInvenStage(bWindow) >= SPECIAL_INVENTORY_LOCK_COVER_COUNT)
	{
		TPacketGCExtendInvenResult pPacket{};
		pPacket.bHeader = HEADER_GC_EXTEND_INVEN_RESULT;
		pPacket.dwVnum = 0;
		pPacket.bMsgResult = EX_INVEN_FAIL_FOURTH_PAGE_STAGE_MAX;
		pPacket.bEnoughCount = 0;
		if (GetDesc())
			GetDesc()->Packet(&pPacket, sizeof(TPacketGCExtendInvenResult));
		return;
	}

	// Check Keys
	std::vector<uint32_t>vecNeedKeys;
	for (int i = INVENTORY_NEED_KEY_START; i <= (SPECIAL_INVENTORY_LOCKED_PAGE_COUNT * INVENTORY_NEED_KEY_INCREASE) + 1; i++)
	{
		for (int j = 0; j < 3; j++)
			vecNeedKeys.emplace_back(i);
	}

	uint32_t dwNeedKeys = vecNeedKeys[GetExtendSpecialInvenStage(bWindow)];
	uint32_t dwCountKeys = CountSpecifyItem(INVENTORY_OPEN_KEY_GOLD) + CountSpecifyItem(INVENTORY_OPEN_KEY_SILVER);
	if (dwCountKeys < dwNeedKeys)
	{
		TPacketGCExtendInvenResult pPacket{};
		pPacket.bHeader = HEADER_GC_EXTEND_INVEN_RESULT;
		pPacket.dwVnum = INVENTORY_OPEN_KEY_GOLD;
		pPacket.bMsgResult = EX_INVEN_FAIL_FALL_SHORT;
		pPacket.bEnoughCount = (dwNeedKeys - dwCountKeys);
		if (GetDesc())
			GetDesc()->Packet(&pPacket, sizeof(TPacketGCExtendInvenResult));
		return;
	}

	// Send Result
	TPacketGCExtendInvenResult pPacket{};
	pPacket.bHeader = HEADER_GC_EXTEND_INVEN_RESULT;
	pPacket.dwVnum = INVENTORY_OPEN_KEY_GOLD;
	pPacket.bMsgResult = EX_INVEN_SUCCESS;
	pPacket.bEnoughCount = dwNeedKeys;
	if (GetDesc())
		GetDesc()->Packet(&pPacket, sizeof(TPacketGCExtendInvenResult));
}

void CHARACTER::ExtendSpecialInvenUpgrade(uint8_t bWindow)
{
	// Max Inventory
	if (GetExtendSpecialInvenStage(bWindow) >= SPECIAL_INVENTORY_LOCK_COVER_COUNT)
	{
		// Max Stage
		//ChatPacket(CHAT_TYPE_INFO, "MAX_STAGE");
		return;
	}

	// Check Keys
	std::vector<uint32_t>vecNeedKeys;
	for (int i = INVENTORY_NEED_KEY_START; i <= (SPECIAL_INVENTORY_LOCKED_PAGE_COUNT * INVENTORY_NEED_KEY_INCREASE) + 1; i++)
	{
		for (int j = 0; j < 3; j++)
			vecNeedKeys.emplace_back(i);
	}

	uint32_t dwNeedKeys = vecNeedKeys[GetExtendSpecialInvenStage(bWindow)];
	uint32_t dwCountKeys = CountSpecifyItem(INVENTORY_OPEN_KEY_GOLD) + CountSpecifyItem(INVENTORY_OPEN_KEY_SILVER);
	if (dwCountKeys >= dwNeedKeys)
	{
		while (dwNeedKeys)
		{
			uint32_t dwKeyVnum = INVENTORY_OPEN_KEY_GOLD;
			if (!CountSpecifyItem(dwKeyVnum))
				dwKeyVnum = INVENTORY_OPEN_KEY_SILVER;

			RemoveSpecifyItem(dwKeyVnum);
			dwNeedKeys--;
		}

		SetExtendSpecialInvenStage(GetExtendSpecialInvenStage(bWindow) + 1, bWindow);
		SendExtendInvenInfo();

		//ChatPacket(CHAT_TYPE_INFO, "SUCCESS");
	}
	else
	{
		//ChatPacket(CHAT_TYPE_INFO, "MISSING KEYS: %d", (dwNeedKeys - dwCountKeys));
		return;
	}
}
# endif
#endif

#ifdef ENABLE_MYSHOP_DECO
void CHARACTER::SetMyPrivShopAdd(uint8_t bType, uint32_t dwPolyVnum)
{
	SetMyShopType(bType);
	SetMyShopDecoPolyVnum(dwPolyVnum);
}

void CHARACTER::SendMyPrivShopOpen(uint8_t bTabCount, bool bIsCashItem)
{
	m_bMyPrivShopIsCashItem = bIsCashItem;
	SetMyShopTabCount(bTabCount);

	TPacketGCShop pack{};
	pack.header = HEADER_GC_SHOP;
	pack.subheader = SHOP_SUBHEADER_GC_MYPRIV_SHOP_OPEN;

	TPacketGCMyPrivShopOpen pack2{};
	pack2.bCashItem = bIsCashItem;
	pack2.bTabCount = bTabCount;

	pack.size = sizeof(pack) + sizeof(pack2);

	if (GetDesc())
	{
		GetDesc()->BufferedPacket(&pack, sizeof(TPacketGCShop));
		GetDesc()->Packet(&pack2, sizeof(TPacketGCMyPrivShopOpen));
	}
}
#endif

#ifdef ENABLE_MULTI_FARM_BLOCK
void CHARACTER::SetProtectTime(const std::string& flagname, int value)
{
	itertype(m_protection_Time) it = m_protection_Time.find(flagname);
	if (it != m_protection_Time.end())
		it->second = value;
	else
		m_protection_Time.insert(make_pair(flagname, value));
}

int CHARACTER::GetProtectTime(const std::string& flagname) const
{
	itertype(m_protection_Time) it = m_protection_Time.find(flagname);
	if (it != m_protection_Time.end())
		return it->second;

	return 0;
}
#endif

#ifdef ENABLE_GUILDWAR_BUTTON
void CHARACTER::SendWarTeleportButton(bool show) 
{
	if (CWarMapManager::Instance().IsWarMap(GetMapIndex()))
		show = false;

	ChatPacket(CHAT_TYPE_COMMAND, "guild_war %d", show);
}
#endif

#ifdef ENABLE_SET_ITEM
void CHARACTER::RefreshSetBonus()
{
	int setID, setCount;
	GetSetCount(setID, setCount);

	{
		CAffect* pSetItem = FindAffect(AFFECT_SET_ITEM);
		if (pSetItem)
			RemoveAffect(pSetItem);

		for (int idx = AFFECT_SET_ITEM_SET_VALUE_1; idx <= AFFECT_SET_ITEM_SET_VALUE_3; ++idx) {
			CAffect* pAffect = FindAffect(idx);
			if (pAffect)
				RemoveAffect(pAffect);
		}
	}

	if (setID < 0 && setID > 5)
		return;

	struct BonusInfo {
		bool piece;	// true == 2,3 - false == 3
		std::map<int, std::vector<int>> bonusInfo = {};
	};
	std::map<int, BonusInfo> setInfo = {
		// true == 2,3 pieces || false == only 3 pieces
		{1, BonusInfo{true, {{2, {APPLY_MAX_HP, 500, APPLY_MALL_EXPBONUS, 5}}, {3, {APPLY_MAX_HP, 1500, APPLY_MALL_EXPBONUS, 15}}}}},
		{2, BonusInfo{true, {{2, {APPLY_MAX_HP, 500, APPLY_MOV_SPEED, 3}}, {3, {APPLY_MAX_HP, 1500, APPLY_MOV_SPEED, 7}}}}},
		{3, BonusInfo{true, {{2, {APPLY_MAX_HP, 500, APPLY_ATTBONUS_MONSTER, 1}}, {3, {APPLY_MAX_HP, 500, APPLY_ATTBONUS_MONSTER, 5}}}}},
		{4, BonusInfo{false, {{3, {APPLY_MAX_HP, 2500, APPLY_MELEE_MAGIC_ATTBONUS_PER, 10, APPLY_MOV_SPEED, 5}}}}},
		{5, BonusInfo{false, {{3, {APPLY_MAX_HP, 3500, APPLY_MELEE_MAGIC_ATTBONUS_PER, 15, APPLY_MOV_SPEED, 7}}}}},
	};

	auto tVec = setInfo[setID].bonusInfo[setCount];
	if (tVec.size() % 2 != 0) {
		//ChatPacket(7, "Error at bonus vector");
	}
	else
	{
		if (!setInfo[setID].piece && setCount == 3)
		{
			int idx = 0;
			for (int i = 0; i < tVec.size() / 2; ++i)
			{
				const int bID = tVec[i * 2];
				const int bVal = tVec[(i * 2) + 1];
				AddAffect(AFFECT_SET_ITEM, POINT_NONE, 0, 0, INFINITE_AFFECT_DURATION, 0, true);
				AddAffect(AFFECT_SET_ITEM_SET_VALUE_1 + idx, aApplyInfo[bID].wPointType, bVal, 0, 60*60*24, 0, true, true);
				++idx;
			}
		}
		else if (setInfo[setID].piece)
		{
			int idx = 0;
			for (int i = 0; i < tVec.size() / 2; ++i)
			{
				const int bID = tVec[i * 2];
				const int bVal = tVec[(i * 2) + 1];
				AddAffect(AFFECT_SET_ITEM, POINT_NONE, 0, 0, INFINITE_AFFECT_DURATION, 0, true);
				AddAffect(AFFECT_SET_ITEM_SET_VALUE_1 + idx, aApplyInfo[bID].wPointType, bVal, 0, 60 * 60 * 24, 0, true, true);
				++idx;
			}
		}
	}
}
#endif

#ifdef ENABLE_FISH_EVENT
void CHARACTER::FishEventGeneralInfo()
{
	if (!GetDesc())
		return;

	TPacketGCFishEventInfo packFishEvent{};
	packFishEvent.bHeader = HEADER_GC_FISH_EVENT_INFO;
	packFishEvent.bSubheader = FISH_EVENT_SUBHEADER_GC_ENABLE;
	packFishEvent.dwFirstArg = quest::CQuestManager::Instance().GetEventFlag("fish_event");
	packFishEvent.dwSecondArg = GetFishEventUseCount();
	GetDesc()->Packet(&packFishEvent, sizeof(TPacketGCFishEventInfo));

	if (GetFishEventUseCount() == 0)
	{
		for (int i = 0; i < FISH_EVENT_SLOTS_NUM; i++)
		{
			m_fishSlots[i].bIsMain = false;
			m_fishSlots[i].bShape = 0;
		}
	}

	for (int i = 0; i < FISH_EVENT_SLOTS_NUM; i++)
	{
		if (m_fishSlots[i].bIsMain)
		{
			TPacketGCFishEventInfo packFishEvent2{};
			packFishEvent2.bHeader = HEADER_GC_FISH_EVENT_INFO;
			packFishEvent2.bSubheader = FISH_EVENT_SUBHEADER_SHAPE_ADD;
			packFishEvent2.dwFirstArg = i;
			packFishEvent2.dwSecondArg = m_fishSlots[i].bShape;
			GetDesc()->Packet(&packFishEvent2, sizeof(TPacketGCFishEventInfo));
		}
	}
}

void CHARACTER::FishEventUseBox(TItemPos itemPos)
{
	if (itemPos.window_type != INVENTORY)
		return;

	if (!GetDesc())
		return;

	if (!CanHandleItem())
		return;

	LPITEM item;
	if (!IsValidItemPosition(itemPos) || !(item = GetItem(itemPos)))
		return;

	if (!item)
		return;

	if (item->IsExchanging())
		return;

	if (item->GetCount() <= 0)
		return;

	if (item->GetVnum() == ITEM_FISH_EVENT_BOX)
	{
		const uint8_t randomShape = number(FISH_EVENT_SHAPE_1, FISH_EVENT_SHAPE_6);
		SetFishAttachedShape(randomShape);
		FishEventIncreaseUseCount();
		item->SetCount(item->GetCount() - 1);
	}
	else if (item->GetVnum() == ITEM_FISH_EVENT_BOX_SPECIAL)
	{
		SetFishAttachedShape(FISH_EVENT_SHAPE_7);
		FishEventIncreaseUseCount();
		item->SetCount(item->GetCount() - 1);
	}
	else
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can not use this item here."));
		return;
	}

	TPacketGCFishEventInfo packFishEvent{};
	packFishEvent.bHeader = HEADER_GC_FISH_EVENT_INFO;
	packFishEvent.bSubheader = FISH_EVENT_SUBHEADER_BOX_USE;
	packFishEvent.dwFirstArg = GetFishAttachedShape();
	packFishEvent.dwSecondArg = GetFishEventUseCount();
	GetDesc()->Packet(&packFishEvent, sizeof(TPacketGCFishEventInfo));
}

bool CHARACTER::FishEventIsValidPosition(uint8_t shapePos, uint8_t shapeType)
{
	bool finalReturn = false;

	const uint8_t positionList[7][8] = {
		{ FISH_EVENT_SHAPE_1, 12, 0, 6, 12, 0, 0, 0 },
		{ FISH_EVENT_SHAPE_2, 24, 0, 0, 0, 0, 0, 0 },
		{ FISH_EVENT_SHAPE_3, 17, 0, 6, 7, 0, 0, 0 },
		{ FISH_EVENT_SHAPE_4, 17, 0, 1, 7, 0, 0, 0 },
		{ FISH_EVENT_SHAPE_5, 17, 0, 1, 6, 7, 0, 0 },
		{ FISH_EVENT_SHAPE_6, 16, 0, 1, 7, 8, 0, 0 },
		{ FISH_EVENT_SHAPE_7, 16, 0, 1, 2, 6, 7, 8 },
	};

	for (uint8_t i = 0; i < sizeof(positionList) / sizeof(positionList[0]); i++)
	{
		if (positionList[i][0] == shapeType)
		{
			for (int j = 0; j < positionList[i][1]; j++)
			{
				if (shapePos == j
					&& m_fishSlots[j + positionList[i][2]].bShape == 0
					&& m_fishSlots[j + positionList[i][3]].bShape == 0
					&& m_fishSlots[j + positionList[i][4]].bShape == 0
					&& m_fishSlots[j + positionList[i][5]].bShape == 0
					&& m_fishSlots[j + positionList[i][6]].bShape == 0
					&& m_fishSlots[j + positionList[i][7]].bShape == 0)
				{
					finalReturn = true;
					break;
				}
			}

			break;
		}
	}

	return finalReturn;
}

void CHARACTER::FishEventPlaceShape(uint8_t shapePos, uint8_t shapeType)
{
	const uint8_t positionList[7][7] =
	{
		{ FISH_EVENT_SHAPE_1, 0, 6, 12, 0, 0, 0 },
		{ FISH_EVENT_SHAPE_2, 0, 0, 0, 0, 0, 0 },
		{ FISH_EVENT_SHAPE_3, 0, 6, 7, 0, 0, 0 },
		{ FISH_EVENT_SHAPE_4, 0, 1, 7, 0, 0, 0 },
		{ FISH_EVENT_SHAPE_5, 0, 1, 6, 7, 0, 0 },
		{ FISH_EVENT_SHAPE_6, 0, 1, 7, 8, 0, 0 },
		{ FISH_EVENT_SHAPE_7, 0, 1, 2, 6, 7, 8 },
	};

	for (uint8_t i = 0; i < sizeof(positionList) / sizeof(positionList[0]); i++)
	{
		if (positionList[i][0] == shapeType)
		{
			for (int j = 1; j < 7; j++)
			{
				if (j > 1 && positionList[i][j] == 0)
					continue;

				if (positionList[i][j] == 0)
				{
					m_fishSlots[shapePos].bIsMain = true;
					m_fishSlots[shapePos].bShape = shapeType;
				}
				else
				{
					m_fishSlots[shapePos + positionList[i][j]].bIsMain = false;
					m_fishSlots[shapePos + positionList[i][j]].bShape = shapeType;
				}
			}

			break;
		}
	}
}

void CHARACTER::FishEventCheckEnd()
{
	bool isComplete = true;

	for (int i = 0; i < FISH_EVENT_SLOTS_NUM; i++)
	{
		if (m_fishSlots[i].bShape == 0)
		{
			isComplete = false;
			break;
		}
	}

	if (isComplete)
	{
		const uint32_t dwUseCount = GetFishEventUseCount();
		const uint32_t dwRewardVnum = dwUseCount <= 10 ? ITEM_FISH_EVENT_REWARD_L : (dwUseCount <= 24 ? ITEM_FISH_EVENT_REWARD_M : ITEM_FISH_EVENT_REWARD_S);

		for (int i = 0; i < FISH_EVENT_SLOTS_NUM; i++)
		{
			m_fishSlots[i].bIsMain = false;
			m_fishSlots[i].bShape = 0;
		}

		AutoGiveItem(dwRewardVnum);
		PointChange(POINT_EXP, 30000);

		m_dwFishUseCount = 0;
		SetFishAttachedShape(0);

		TPacketGCFishEventInfo packFishEvent{};
		packFishEvent.bHeader = HEADER_GC_FISH_EVENT_INFO;
		packFishEvent.bSubheader = FISH_EVENT_SUBHEADER_GC_REWARD;
		packFishEvent.dwFirstArg = dwRewardVnum;
		packFishEvent.dwSecondArg = 0;
		GetDesc()->Packet(&packFishEvent, sizeof(TPacketGCFishEventInfo));
	}
}

void CHARACTER::FishEventAddShape(uint8_t shapePos)
{
	if (!GetDesc())
		return;

	if (shapePos >= FISH_EVENT_SLOTS_NUM)
		return;

	const uint8_t lastAttachedShape = GetFishAttachedShape();

	if (lastAttachedShape == 0 || lastAttachedShape > ITEM_FISH_EVENT_BOX_SPECIAL)
		return;

	if (!FishEventIsValidPosition(shapePos, lastAttachedShape))
	{
		TPacketGCFishEventInfo packFishEvent{};
		packFishEvent.bHeader = HEADER_GC_FISH_EVENT_INFO;
		packFishEvent.bSubheader = FISH_EVENT_SUBHEADER_BOX_USE;
		packFishEvent.dwFirstArg = GetFishAttachedShape();
		packFishEvent.dwSecondArg = GetFishEventUseCount();
		GetDesc()->Packet(&packFishEvent, sizeof(TPacketGCFishEventInfo));
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("That shape can not fit in this position."));
		return;
	}

	FishEventPlaceShape(shapePos, lastAttachedShape);

	TPacketGCFishEventInfo packFishEvent{};
	packFishEvent.bHeader = HEADER_GC_FISH_EVENT_INFO;
	packFishEvent.bSubheader = FISH_EVENT_SUBHEADER_SHAPE_ADD;
	packFishEvent.dwFirstArg = shapePos;
	packFishEvent.dwSecondArg = lastAttachedShape;
	GetDesc()->Packet(&packFishEvent, sizeof(TPacketGCFishEventInfo));

	FishEventCheckEnd();
}
#endif

#ifdef ENABLE_MINI_GAME_FINDM
#include "minigame_manager.h"
EVENTFUNC(reveal_timer)
{
	const char_event_info* info = dynamic_cast<char_event_info*>(event->info);

	if (!info)
	{
		sys_err("<reveal_timer> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (!ch)
		return 0;

	if (!ch->GetDesc())
		return 0;

	const int iFirstCard = ch->iRevealedIndex[0];
	const int iSecondCard = ch->iRevealedIndex[1];
	uint8_t bPickNumber = 0;

	if (iFirstCard == -1 && iSecondCard == -1)
		bPickNumber = 3;
	else if (iFirstCard != -1 && iSecondCard == -1)
		bPickNumber = 2;
	else if (iFirstCard != -1 && iSecondCard != -1)
		bPickNumber = 1;

	while (bPickNumber > 0)
	{
		const int bCardIndex = CMiniGameManager::Instance().MiniGameFindMPickRandomCard(ch);
		if (bCardIndex == -1)
			return 0;

		const uint8_t bSkipFlag = (bPickNumber == 1) ? 2 : 1;
		CMiniGameManager::Instance().MiniGameFindMClickCard(ch, bCardIndex, bSkipFlag);
		bPickNumber--;
	}

	return PASSES_PER_SEC(FIND_M_REVEAL_TIME);
}

void CHARACTER::MiniGameFindMStartGame(std::vector<TFindMCard> cardsVector, uint32_t dwStartTime)
{
	m_vecFindMFieldCards = cardsVector;
	dwFindMStartTime = dwStartTime;
	wFindMTryCount = 0;
	bFindMGameStatus = true;
}

bool CHARACTER::MiniGameFindMIsAllRevealed()
{
	bool bIsAllRevealed = true;

	for (size_t i = 0; i < m_vecFindMFieldCards.size(); i++)
	{
		if (!m_vecFindMFieldCards[i].bIsRevealed)
		{
			bIsAllRevealed = false;
			break;
		}
	}

	return bIsAllRevealed;
}

void CHARACTER::MiniGameFindMStartReveal(int seconds)
{
	if (seconds == -1)
	{
		event_cancel(&m_revealEvent);
		dwFindMNextReveal = 0;
		return;
	}

	if (m_revealEvent)
		event_reset_time(m_revealEvent, PASSES_PER_SEC(seconds));
	else
	{
		char_event_info* info = AllocEventInfo<char_event_info>();
		info->ch = this;

		m_revealEvent = event_create(reveal_timer, info, PASSES_PER_SEC(seconds));
	}

	dwFindMNextReveal = time(0) + seconds;
}
#endif

#ifdef ENABLE_MAILBOX
void CHARACTER::SetMailBox(CMailBox* m)
{
	if (m_pkMailBox)
		delete m_pkMailBox;

	m_pkMailBox = m;

#ifdef ENABLE_CHECK_WINDOW_RENEWAL
	SetOpenedWindow(W_MAILBOX, m_pkMailBox ? true : false);
#endif
}
#endif

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
void CHARACTER::GetGlobalRankRoulette(char* buffer, size_t buflen)
{
	auto pMsg(DBManager::Instance().DirectQuery("SELECT name, SUM(soulre) as ssoulres FROM player.player GROUP BY id ORDER BY ssoulres DESC LIMIT 10"));
	if (pMsg->Get()->uiNumRows == 0)
	{
		return;
	}

	MYSQL_ROW row;
	int pos = 1;
	int len = 0, len2;
	*buffer = '\0';
	while (nullptr != (row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
	{
		int soulre = 0;
		str_to_number(soulre, row[1]);
		len2 = snprintf(buffer + len, buflen - len, "[ENTER]");

		if (len2 < 0 || len2 >= (int)buflen - len)
			len += (buflen - len) - 1;
		else
			len += len2;

		len2 = snprintf(buffer + len, buflen - len, "%d. - %s - %d Ruh",
			pos,
			row[0],
			soulre);
		++pos;
		if (len2 < 0 || len2 >= (int)buflen - len)
			len += (buflen - len) - 1;
		else
			len += len2;
	}
}
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
void CHARACTER::SetTransmutation(CTransmutation* c)
{
	if (m_pkTransmutation != nullptr)
		delete m_pkTransmutation;

	m_pkTransmutation = c;

# ifdef ENABLE_CHECK_WINDOW_RENEWAL
	SetOpenedWindow(W_CHANGELOOK, m_pkTransmutation ? true : false);
# endif
}

CTransmutation* CHARACTER::IsChangeLookWindowOpen() const
{
	return m_pkTransmutation;
}
#endif

#ifdef ENABLE_MOVE_CHANNEL
EVENTINFO(move_channel_info)
{
	LPCHARACTER		ch;
	int				left_second;
	long			lAddr;
	uint16_t			wPort;

	move_channel_info(const LPCHARACTER m_ch, const int m_sec, const long m_Addr, const uint16_t m_Port) :
		ch(m_ch),
		left_second(m_sec),
		lAddr(m_Addr),
		wPort(m_Port)
	{}
};

EVENTFUNC(move_channel_event)
{
	move_channel_info* info = dynamic_cast<move_channel_info*>(event->info);

	if (info == nullptr)
	{
		sys_err("<move_channel_event> <Factor> Null pointer");
		return 0;
	}

	const LPCHARACTER ch = info->ch;
	if (ch == nullptr)
		return 0;

	if (info->left_second <= 0)
	{
		ch->m_pkTimedEvent = nullptr;
		if (ch->CanWarp())
			ch->WarpSet(ch->GetX(), ch->GetY(), 0, info->lAddr, info->wPort);
		else
			ch->ChatPacket(CHAT_TYPE_INFO, "Switching canceled.");
		return 0;
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d초 남았습니다."), info->left_second);
		--info->left_second;
	}

	return PASSES_PER_SEC(1);
}

void CHARACTER::MoveChannel(const TRespondMoveChannel* p)
{
	if (p == nullptr)
		return;

	const LPDESC d = GetDesc();
	if (d == nullptr)
		return;

	if (p->wPort == 0 || p->lAddr == 0)
	{
		ChatPacket(CHAT_TYPE_INFO, "Currently, you cannot switch to that channel.");
		return;
	}

	if (m_pkTimedEvent)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("취소 되었습니다."));
		event_cancel(&m_pkTimedEvent);
		return;
	}

	ChatPacket(CHAT_TYPE_INFO, "Please wait a moment. Changing channel...");
	if (IsHack(false, true, 10))
		return;

	m_pkTimedEvent = event_create(move_channel_event,
		new move_channel_info(this, IsPosition(POS_FIGHTING) ? 10 : 3, p->lAddr, p->wPort), 1);
}
#endif

#ifdef ENABLE_DUNGEON_INFO_SYSTEM
#include "DungeonInfo.h"
EVENTFUNC(ReloadDungeonInfoEventFunc)
{
	char_event_info* info = dynamic_cast<char_event_info*>(event->info);

	if (info == nullptr)
	{
		sys_err("ReloadDungeonInfoEventFunc> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == nullptr) // <Factor>
		return 0;

	if (!ch->IsDungeonInfoOpen())
	{
		ch->StopDungeonInfoReloadEvent();
		return 0;
	}

	CDungeonInfoManager::Instance().SendInfo(ch);

	return PASSES_PER_SEC(EDungeonInfoData::MAX_REFRESH_DELAY);
}

void CHARACTER::StartDungeonInfoReloadEvent()
{
	if (m_pkDungeonInfoReloadEvent)
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();

	info->ch = this;
	m_pkDungeonInfoReloadEvent = event_create(ReloadDungeonInfoEventFunc, info, PASSES_PER_SEC(EDungeonInfoData::MAX_REFRESH_DELAY));
}

void CHARACTER::StopDungeonInfoReloadEvent()
{
	event_cancel(&m_pkDungeonInfoReloadEvent);
}

bool CHARACTER::UpdateDungeonRanking(const std::string c_strQuestName)
{
	long lMapIndex = GetMapIndex() >= 10000 ? GetMapIndex() / 10000 : GetMapIndex();

	int iEnterTime = GetQuestFlag(c_strQuestName + "." + "enter_time");
	int iRunTime = MAX(get_global_time() - iEnterTime, 0);
	int iDamage = MAX(GetLastDamage(), 0);

	bool bQueryUpdate = true;
	// CHECK_DUNGEON_ID
	char szQuery[QUERY_MAX_LEN];
	snprintf(szQuery, sizeof(szQuery), "SELECT `id` FROM dungeon_ranking%s WHERE `pid` = '%u' AND `dungeon_index` = '%ld'", get_table_postfix(), GetPlayerID(), lMapIndex);
	auto pMsg(DBManager::Instance().DirectQuery(szQuery));
	if (pMsg->Get()->uiNumRows <= 0)
	{
		snprintf(szQuery, sizeof(szQuery), "INSERT INTO dungeon_ranking%s (`pid`, `dungeon_index`, `finish`, `finish_time`, `finish_damage`) VALUES ('%u', '%ld', '%d', '%d', '%d')",
			get_table_postfix(), GetPlayerID(), lMapIndex, 1, iRunTime, iDamage);
		auto pMsg(DBManager::Instance().DirectQuery(szQuery));
		bQueryUpdate = false;
	}
	// END_OF_CHECK_DUNGEON_ID

	std::initializer_list<const char*> c_szList = { "finish", "finish_time", "finish_damage" };

	int iRow = 0;
	for (const char* c_szRow : c_szList)
	{
		char szUpdateQuery[QUERY_MAX_LEN];
		std::string strQuestFlagName = c_strQuestName + "." + static_cast<std::string>(c_szRow);

		if (iRow == 0) // finish
		{
			SetQuestFlag(strQuestFlagName, GetQuestFlag(strQuestFlagName) + 1);

			if (bQueryUpdate)
			{
				snprintf(szUpdateQuery, sizeof(szUpdateQuery), "UPDATE dungeon_ranking%s SET `%s` = `%s` + 1 WHERE `pid` = '%u' AND `dungeon_index` = '%ld'",
					get_table_postfix(), c_szRow, c_szRow, GetPlayerID(), lMapIndex);
			}
		}
		else if (iRow == 1) // finish_time
		{
			SetQuestFlag(strQuestFlagName, iRunTime);

			if (bQueryUpdate)
			{
				snprintf(szUpdateQuery, sizeof(szUpdateQuery), "UPDATE dungeon_ranking%s SET `%s` = '%d' WHERE `pid` = '%u' AND `dungeon_index` = '%ld'",
					get_table_postfix(), c_szRow, iRunTime, GetPlayerID(), lMapIndex);
			}
		}
		else if (iRow == 2) // finish_damage
		{
			SetQuestFlag(strQuestFlagName, iDamage);

			if (bQueryUpdate)
			{
				snprintf(szUpdateQuery, sizeof(szUpdateQuery), "UPDATE dungeon_ranking%s SET `%s` = '%d' WHERE `pid` = '%u' AND `dungeon_index` = '%ld'",
					get_table_postfix(), c_szRow, iDamage, GetPlayerID(), lMapIndex);
			}
		}

		++iRow;

		if (bQueryUpdate)
			auto pMsg(DBManager::Instance().DirectQuery(szUpdateQuery));
	}

	return true;
}
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
void CHARACTER::SetLastReciveExtBattlePassInfoTime(uint32_t time)
{
	m_dwLastReciveExtBattlePassInfoTime = time;
}

void CHARACTER::SetLastReciveExtBattlePassOpenRanking(uint32_t time)
{
	m_dwLastExtBattlePassOpenRankingTime = time;
}

void CHARACTER::LoadExtBattlePass(uint32_t dwCount, TPlayerExtBattlePassMission* data)
{
	m_bIsLoadedExtBattlePass = false;

	for (int i = 0; i < dwCount; ++i, ++data)
	{
		TPlayerExtBattlePassMission* newMission = new TPlayerExtBattlePassMission;
		newMission->dwPlayerId = data->dwPlayerId;
		newMission->dwBattlePassType = data->dwBattlePassType;
		newMission->dwMissionIndex = data->dwMissionIndex;
		newMission->dwMissionType = data->dwMissionType;
		newMission->dwBattlePassId = data->dwBattlePassId;
		newMission->dwExtraInfo = data->dwExtraInfo;
		newMission->bCompleted = data->bCompleted;
		newMission->bIsUpdated = data->bIsUpdated;

		m_listExtBattlePass.push_back(newMission);
	}

	m_bIsLoadedExtBattlePass = true;
}

uint32_t CHARACTER::GetExtBattlePassMissionProgress(uint32_t dwBattlePassType, uint8_t bMissionIndex, uint8_t bMissionType)
{
	uint32_t BattlePassID;
	if (dwBattlePassType == 1)
		BattlePassID = CBattlePassManager::instance().GetNormalBattlePassID();
	else if (dwBattlePassType == 2)
		BattlePassID = CBattlePassManager::instance().GetPremiumBattlePassID();
	else if (dwBattlePassType == 3)
		BattlePassID = CBattlePassManager::instance().GetEventBattlePassID();
	else
	{
		sys_err("Unknown BattlePassType (%d)", dwBattlePassType);
		return 0;
	}

	ListExtBattlePassMap::iterator it = m_listExtBattlePass.begin();
	while (it != m_listExtBattlePass.end())
	{
		TPlayerExtBattlePassMission* pkMission = *it++;
		if (pkMission->dwBattlePassType == dwBattlePassType && pkMission->dwMissionIndex == bMissionIndex && pkMission->dwMissionType == bMissionType && pkMission->dwBattlePassId == BattlePassID)
			return pkMission->dwExtraInfo;
	}

	return 0;
}

bool CHARACTER::IsExtBattlePassCompletedMission(uint32_t dwBattlePassType, uint8_t bMissionIndex, uint8_t bMissionType)
{
	uint32_t BattlePassID;
	if (dwBattlePassType == 1)
		BattlePassID = CBattlePassManager::instance().GetNormalBattlePassID();
	else if (dwBattlePassType == 2)
		BattlePassID = CBattlePassManager::instance().GetPremiumBattlePassID();
	else if (dwBattlePassType == 3)
		BattlePassID = CBattlePassManager::instance().GetEventBattlePassID();
	else
	{
		sys_err("Unknown BattlePassType (%d)", dwBattlePassType);
		return false;
	}

	ListExtBattlePassMap::iterator it = m_listExtBattlePass.begin();
	while (it != m_listExtBattlePass.end())
	{
		TPlayerExtBattlePassMission* pkMission = *it++;
		if (pkMission->dwBattlePassType == dwBattlePassType && pkMission->dwMissionIndex == bMissionIndex && pkMission->dwMissionType == bMissionType && pkMission->dwBattlePassId == BattlePassID)
			return (pkMission->bCompleted ? true : false);
	}

	return false;
}

bool CHARACTER::IsExtBattlePassRegistered(uint8_t bBattlePassType, uint32_t dwBattlePassID)
{
	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT * FROM player.battlepass_playerindex WHERE player_id = %d and battlepass_type = %d and battlepass_id = %d", GetPlayerID(), bBattlePassType, dwBattlePassID));
	if (pMsg->Get()->uiNumRows != 0)
		return true;

	return false;
}

void CHARACTER::UpdateExtBattlePassMissionProgress(uint32_t dwMissionType, uint32_t dwUpdateValue, uint32_t dwCondition, bool isOverride)
{
	if (!GetDesc())
		return;

	if (!m_bIsLoadedExtBattlePass)
		return;

	uint32_t dwSafeCondition = dwCondition;
	for (uint8_t bBattlePassType = 1; bBattlePassType <= 3; ++bBattlePassType)
	{
		bool foundMission = false;
		uint32_t dwSaveProgress = 0;
		dwCondition = dwSafeCondition;

		uint8_t bBattlePassID;
		uint8_t bMissionIndex = CBattlePassManager::instance().GetMissionIndex(bBattlePassType, dwMissionType, dwCondition);

		if (bBattlePassType == 1)
			bBattlePassID = CBattlePassManager::instance().GetNormalBattlePassID();
		if (bBattlePassType == 2)
		{
			bBattlePassID = CBattlePassManager::instance().GetPremiumBattlePassID();
			if (bBattlePassID != GetExtBattlePassPremiumID())
				continue;
		}
		if (bBattlePassType == 3)
			bBattlePassID = CBattlePassManager::instance().GetEventBattlePassID();

		uint32_t dwFirstInfo, dwSecondInfo;
		if (CBattlePassManager::instance().BattlePassMissionGetInfo(bBattlePassType, bMissionIndex, bBattlePassID, dwMissionType, &dwFirstInfo, &dwSecondInfo))
		{
			if (dwFirstInfo == 0)
				dwCondition = 0;

			if (dwMissionType == 2 and dwFirstInfo <= dwCondition or dwMissionType == 4 and dwFirstInfo <= dwCondition or dwMissionType == 20 and dwFirstInfo <= dwCondition)
				dwCondition = dwFirstInfo;

			if (dwFirstInfo == dwCondition && GetExtBattlePassMissionProgress(bBattlePassType, bMissionIndex, dwMissionType) < dwSecondInfo)
			{
				ListExtBattlePassMap::iterator it = m_listExtBattlePass.begin();
				while (it != m_listExtBattlePass.end())
				{
					TPlayerExtBattlePassMission* pkMission = *it++;

					if (pkMission->dwBattlePassType == bBattlePassType && pkMission->dwMissionIndex == bMissionIndex && pkMission->dwMissionType == dwMissionType && pkMission->dwBattlePassId == bBattlePassID)
					{
						pkMission->bIsUpdated = 1;

						if (isOverride)
							pkMission->dwExtraInfo = dwUpdateValue;
						else
							pkMission->dwExtraInfo += dwUpdateValue;

						if (pkMission->dwExtraInfo >= dwSecondInfo)
						{
							pkMission->dwExtraInfo = dwSecondInfo;
							pkMission->bCompleted = 1;

							std::string stMissionName = CBattlePassManager::instance().GetMissionNameByType(pkMission->dwMissionType);
							std::string stBattlePassName = CBattlePassManager::instance().GetNormalBattlePassNameByID(pkMission->dwBattlePassId);

							CBattlePassManager::instance().BattlePassRewardMission(this, bBattlePassType, bBattlePassID, bMissionIndex);
							if (bBattlePassType == 1)
							{
								EffectPacket(SE_EFFECT_BP_NORMAL_MISSION_COMPLETED);
								ChatPacket(CHAT_TYPE_NOTICE, "BATTLEPASS_COMPLETE_NORMAL_MISSION");
							}
							if (bBattlePassType == 2)
							{
								EffectPacket(SE_EFFECT_BP_PREMIUM_MISSION_COMPLETED);
								ChatPacket(CHAT_TYPE_NOTICE, "BATTLEPASS_COMPLETE_PREMIUM_MISSION");
							}
							if (bBattlePassType == 3)
							{
								EffectPacket(SE_EFFECT_BP_EVENT_MISSION_COMPLETED);
								ChatPacket(CHAT_TYPE_NOTICE, "BATTLEPASS_COMPLETE_EVENT_MISSION");
							}

							TPacketGCExtBattlePassMissionUpdate packet;
							packet.bHeader = HEADER_GC_EXT_BATTLE_PASS_MISSION_UPDATE;
							packet.bBattlePassType = bBattlePassType;
							packet.bMissionIndex = bMissionIndex;
							packet.dwNewProgress = pkMission->dwExtraInfo;
							GetDesc()->Packet(&packet, sizeof(TPacketGCExtBattlePassMissionUpdate));
						}

						dwSaveProgress = pkMission->dwExtraInfo;
						foundMission = true;

						if (pkMission->bCompleted != 1)
						{
							TPacketGCExtBattlePassMissionUpdate packet;
							packet.bHeader = HEADER_GC_EXT_BATTLE_PASS_MISSION_UPDATE;
							packet.bBattlePassType = bBattlePassType;
							packet.bMissionIndex = bMissionIndex;
							packet.dwNewProgress = dwSaveProgress;
							GetDesc()->Packet(&packet, sizeof(TPacketGCExtBattlePassMissionUpdate));
						}
						break;
					}

				}

				if (!foundMission)
				{
					if (!IsExtBattlePassRegistered(bBattlePassType, bBattlePassID))
						DBManager::instance().DirectQuery("INSERT INTO player.battlepass_playerindex SET player_id=%d, player_name='%s', battlepass_type=%d, battlepass_id=%d, start_time=NOW()", GetPlayerID(), GetName(), bBattlePassType, bBattlePassID);

					TPlayerExtBattlePassMission* newMission = new TPlayerExtBattlePassMission;
					newMission->dwPlayerId = GetPlayerID();
					newMission->dwBattlePassType = bBattlePassType;
					newMission->dwMissionType = dwMissionType;
					newMission->dwBattlePassId = bBattlePassID;

					if (dwUpdateValue >= dwSecondInfo)
					{
						newMission->dwMissionIndex = CBattlePassManager::instance().GetMissionIndex(bBattlePassType, dwMissionType, dwCondition);
						newMission->dwExtraInfo = dwSecondInfo;
						newMission->bCompleted = 1;

						CBattlePassManager::instance().BattlePassRewardMission(this, bBattlePassType, bBattlePassID, bMissionIndex);
						if (bBattlePassType == 1)
						{
							EffectPacket(SE_EFFECT_BP_NORMAL_MISSION_COMPLETED);
							ChatPacket(CHAT_TYPE_NOTICE, "BATTLEPASS_COMPLETE_NORMAL_MISSION");
						}
						if (bBattlePassType == 2)
						{
							EffectPacket(SE_EFFECT_BP_PREMIUM_MISSION_COMPLETED);
							ChatPacket(CHAT_TYPE_NOTICE, "BATTLEPASS_COMPLETE_PREMIUM_MISSION");
						}
						if (bBattlePassType == 3)
						{
							EffectPacket(SE_EFFECT_BP_EVENT_MISSION_COMPLETED);
							ChatPacket(CHAT_TYPE_NOTICE, "BATTLEPASS_COMPLETE_EVENT_MISSION");
						}

						dwSaveProgress = dwSecondInfo;
					}
					else
					{
						newMission->dwMissionIndex = CBattlePassManager::instance().GetMissionIndex(bBattlePassType, dwMissionType, dwCondition);
						newMission->dwExtraInfo = dwUpdateValue;
						newMission->bCompleted = 0;

						dwSaveProgress = dwUpdateValue;
					}

					newMission->bIsUpdated = 1;

					m_listExtBattlePass.push_back(newMission);

					TPacketGCExtBattlePassMissionUpdate packet;
					packet.bHeader = HEADER_GC_EXT_BATTLE_PASS_MISSION_UPDATE;
					packet.bBattlePassType = bBattlePassType;
					packet.bMissionIndex = bMissionIndex;
					packet.dwNewProgress = dwSaveProgress;
					GetDesc()->Packet(&packet, sizeof(TPacketGCExtBattlePassMissionUpdate));
				}
			}
		}
	}
}

void CHARACTER::SetExtBattlePassMissionProgress(uint8_t bBattlePassType, uint32_t dwMissionIndex, uint32_t dwMissionType, uint32_t dwUpdateValue)
{
	if (!GetDesc())
		return;

	if (!m_bIsLoadedExtBattlePass)
		return;

	bool foundMission = false;
	uint32_t dwSaveProgress = 0;
	
	uint8_t bBattlePassID;
	if (bBattlePassType == 1)
		bBattlePassID = CBattlePassManager::instance().GetNormalBattlePassID();
	else if (bBattlePassType == 2)
		bBattlePassID = CBattlePassManager::instance().GetPremiumBattlePassID();
	else if (bBattlePassType == 3)
		bBattlePassID = CBattlePassManager::instance().GetEventBattlePassID();
	else
	{
		sys_err("Unknown BattlePassType (%d)", bBattlePassType);
		return;
	}

	uint32_t dwFirstInfo, dwSecondInfo;
	if (CBattlePassManager::instance().BattlePassMissionGetInfo(bBattlePassType, dwMissionIndex, bBattlePassID, dwMissionType, &dwFirstInfo, &dwSecondInfo))
	{
		ListExtBattlePassMap::iterator it = m_listExtBattlePass.begin();
		while (it != m_listExtBattlePass.end())
		{
			TPlayerExtBattlePassMission* pkMission = *it++;

			if (pkMission->dwBattlePassType == bBattlePassType && pkMission->dwMissionIndex == dwMissionIndex && pkMission->dwMissionType == dwMissionType && pkMission->dwBattlePassId == bBattlePassID)
			{
				pkMission->bIsUpdated = 1;
				pkMission->bCompleted = 0;
				
				pkMission->dwExtraInfo = dwUpdateValue;

				if (pkMission->dwExtraInfo >= dwSecondInfo)
				{
					pkMission->dwExtraInfo = dwSecondInfo;
					pkMission->bCompleted = 1;

					std::string stMissionName = CBattlePassManager::instance().GetMissionNameByType(pkMission->dwMissionType);
					std::string stBattlePassName = CBattlePassManager::instance().GetNormalBattlePassNameByID(pkMission->dwBattlePassId);
					//ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("New Value : %d"), pkMission->dwExtraInfo);

					CBattlePassManager::instance().BattlePassRewardMission(this, bBattlePassType, bBattlePassID, dwMissionIndex);
					if (bBattlePassType == 1)
					{
						EffectPacket(SE_EFFECT_BP_NORMAL_MISSION_COMPLETED);
						ChatPacket(CHAT_TYPE_NOTICE, "BATTLEPASS_COMPLETE_NORMAL_MISSION");
					}
					if (bBattlePassType == 2)
					{
						EffectPacket(SE_EFFECT_BP_PREMIUM_MISSION_COMPLETED);
						ChatPacket(CHAT_TYPE_NOTICE, "BATTLEPASS_COMPLETE_PREMIUM_MISSION");
					}
					if (bBattlePassType == 3)
					{
						EffectPacket(SE_EFFECT_BP_EVENT_MISSION_COMPLETED);
						ChatPacket(CHAT_TYPE_NOTICE, "BATTLEPASS_COMPLETE_EVENT_MISSION");
					}

					TPacketGCExtBattlePassMissionUpdate packet;
					packet.bHeader = HEADER_GC_EXT_BATTLE_PASS_MISSION_UPDATE;
					packet.bBattlePassType = bBattlePassType;
					packet.bMissionIndex = dwMissionIndex;
					packet.dwNewProgress = pkMission->dwExtraInfo;
					GetDesc()->Packet(&packet, sizeof(TPacketGCExtBattlePassMissionUpdate));
				}

				dwSaveProgress = pkMission->dwExtraInfo;
				foundMission = true;

				if (pkMission->bCompleted != 1)
				{
					TPacketGCExtBattlePassMissionUpdate packet;
					packet.bHeader = HEADER_GC_EXT_BATTLE_PASS_MISSION_UPDATE;
					packet.bBattlePassType = bBattlePassType;
					packet.bMissionIndex = dwMissionIndex;
					packet.dwNewProgress = dwSaveProgress;
					GetDesc()->Packet(&packet, sizeof(TPacketGCExtBattlePassMissionUpdate));
				}
				break;
			}
		}

		if (!foundMission)
		{
			if (!IsExtBattlePassRegistered(bBattlePassType, bBattlePassID))
				DBManager::instance().DirectQuery("INSERT INTO player.battlepass_playerindex SET player_id=%d, player_name='%s', battlepass_type=%d, battlepass_id=%d, start_time=NOW()", GetPlayerID(), GetName(), bBattlePassType, bBattlePassID);

			TPlayerExtBattlePassMission* newMission = new TPlayerExtBattlePassMission;
			newMission->dwPlayerId = GetPlayerID();
			newMission->dwBattlePassType = bBattlePassType;
			newMission->dwMissionType = dwMissionType;
			newMission->dwBattlePassId = bBattlePassID;

			if (dwUpdateValue >= dwSecondInfo)
			{
				newMission->dwMissionIndex = dwMissionIndex;
				newMission->dwExtraInfo = dwSecondInfo;
				newMission->bCompleted = 1;

				CBattlePassManager::instance().BattlePassRewardMission(this, bBattlePassType, bBattlePassID, dwMissionIndex);
				if (bBattlePassType == 1)
				{
					EffectPacket(SE_EFFECT_BP_NORMAL_MISSION_COMPLETED);
					ChatPacket(CHAT_TYPE_NOTICE, "BATTLEPASS_COMPLETE_NORMAL_MISSION");
				}
				if (bBattlePassType == 2)
				{
					EffectPacket(SE_EFFECT_BP_PREMIUM_MISSION_COMPLETED);
					ChatPacket(CHAT_TYPE_NOTICE, "BATTLEPASS_COMPLETE_PREMIUM_MISSION");
				}
				if (bBattlePassType == 3)
				{
					EffectPacket(SE_EFFECT_BP_EVENT_MISSION_COMPLETED);
					ChatPacket(CHAT_TYPE_NOTICE, "BATTLEPASS_COMPLETE_EVENT_MISSION");
				}

				dwSaveProgress = dwSecondInfo;
			}
			else
			{
				newMission->dwMissionIndex = dwMissionIndex;
				newMission->dwExtraInfo = dwUpdateValue;
				newMission->bCompleted = 0;

				dwSaveProgress = dwUpdateValue;
			}

			newMission->bIsUpdated = 1;

			m_listExtBattlePass.push_back(newMission);

			TPacketGCExtBattlePassMissionUpdate packet;
			packet.bHeader = HEADER_GC_EXT_BATTLE_PASS_MISSION_UPDATE;
			packet.bBattlePassType = bBattlePassType;
			packet.bMissionIndex = dwMissionIndex;
			packet.dwNewProgress = dwSaveProgress;
			GetDesc()->Packet(&packet, sizeof(TPacketGCExtBattlePassMissionUpdate));
		}
	}
}
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
void CHARACTER::SetGrowthPetInfo(TGrowthPetInfo petInfo)
{
	const auto it = m_GrowthPetInfo.find(petInfo.pet_id);
	if (it != m_GrowthPetInfo.end())
	{
		(*it).second = petInfo;
		return;
	}

	m_GrowthPetInfo.insert(std::make_pair(petInfo.pet_id, petInfo));
}

std::vector<TGrowthPetInfo> CHARACTER::GetPetList() const
{
	std::vector<TGrowthPetInfo> pets{};
	pets.reserve(m_GrowthPetInfo.size());
	for (auto& iter : m_GrowthPetInfo)
	{
		pets.emplace_back(iter.second);
	}
	return pets;
}

void CHARACTER::SendGrowthPetInfoPacket()
{
	if (!GetDesc())
		return;

#ifdef _DEBUG
	if (m_GrowthPetInfo.empty())
		return;
#endif

	TPacketGCGrowthPetInfo packet{};
	packet.bHeader = HEADER_GC_GROWTH_PET_INFO;
	packet.wSize = static_cast<uint16_t>(sizeof(packet) + sizeof(TGrowthPetInfo) * m_GrowthPetInfo.size());

	GetDesc()->BufferedPacket(&packet, sizeof(packet));
	GetDesc()->Packet(&GetPetList()[0], sizeof(TGrowthPetInfo) * m_GrowthPetInfo.size());
}

void CHARACTER::SetPetWindowType(uint8_t pet_window_type)
{
#ifdef ENABLE_PET_ATTR_DETERMINE
	if (pet_window_type == 1)
	{
		const int iPulse = thecore_pulse();
		if (iPulse - GetGrowthPetDetermineLoadTime() < PASSES_PER_SEC(3))
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;1351]");
			return;
		}

		if (GetExchange()
#	ifdef ENABLE_PREMIUM_PRIVATE_SHOP
			|| GetMyShop()
			|| GetViewingShopOwner()
#	else
			|| GetMyShop()
			|| GetShopOwner()
#	endif
			|| IsOpenSafebox() || IsCubeOpen()
# ifdef ENABLE_GUILDSTORAGE_SYSTEM
			|| IsOpenGuildstorage()
# endif
			)
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;1462]");
			return;
		}
	}
#endif

	TEMP_BUFFER buf;
	TPacketGCGrowthPet pack;
	TSubPacketCGGrowthPetWindowType sub{};
	pack.wSize = sizeof(TPacketGCGrowthPet) + sizeof(TSubPacketCGGrowthPetWindowType);
	pack.bSubHeader = GROWTHPET_SUBHEADER_GC_WINDOW;
	sub.bPetWindowType = pet_window_type;

	LPDESC desc = GetDesc();
	if (!desc)
	{
		sys_err("User(%s)'s DESC is nullptr POINT.", GetName());
		return;
	}

	buf.write(&pack, sizeof(TPacketGCGrowthPet));
	buf.write(&sub, sizeof(TSubPacketCGGrowthPetWindowType));

	m_GrowthPetWindowType = pet_window_type;
#ifdef ENABLE_PET_ATTR_DETERMINE
	if (pet_window_type == 1)
		SetGrowthPetDetermineLoadTime();
#endif

	desc->Packet(buf.read_peek(), buf.size());
}
#endif

#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
uint8_t CHARACTER::GetAlignRank()
{
	uint8_t bAlignIndex = 0;

	if (GetRealAlignment() >= 120000)
		bAlignIndex = PVP_LEVEL_CHIVALRIC;
	else if (GetRealAlignment() >= 80000)
		bAlignIndex = PVP_LEVEL_NOBLE;
	else if (GetRealAlignment() >= 40000)
		bAlignIndex = PVP_LEVEL_GOOD;
	else if (GetRealAlignment() >= 10000)
		bAlignIndex = PVP_LEVEL_FRIENDLY;
	else if (GetRealAlignment() >= 0)
		bAlignIndex = PVP_LEVEL_NEUTRAL;
	else if (GetRealAlignment() > -40000)
		bAlignIndex = PVP_LEVEL_AGGRESSIVE;
	else if (GetRealAlignment() > -80000)
		bAlignIndex = PVP_LEVEL_FRAUDULENT;
	else if (GetRealAlignment() > -120000)
		bAlignIndex = PVP_LEVEL_MALICIOUS;
	else
		bAlignIndex = PVP_LEVEL_CRUEL;

	return bAlignIndex;
}
#endif

#if defined(ENABLE_PENDANT) || defined(ENABLE_ELEMENT_ADD) || defined(ENABLE_REFINE_ELEMENT)
uint16_t CHARACTER::GetPointElementSum()
{
	const int iAttElec = GetPoint(POINT_ENCHANT_ELECT);
	const int iAttFire = GetPoint(POINT_ENCHANT_FIRE);
	const int iAttIce = GetPoint(POINT_ENCHANT_ICE);
	const int iAttWind = GetPoint(POINT_ENCHANT_WIND);
	const int iAttEarth = GetPoint(POINT_ENCHANT_EARTH);
	const int iAttDark = GetPoint(POINT_ENCHANT_DARK);
	const int element_total_sum = iAttElec + iAttFire + iAttIce + iAttWind + iAttEarth + iAttDark;

	return element_total_sum;
}
#endif

#ifdef ENABLE_HIDE_COSTUME_SYSTEM
void CHARACTER::SetBodyCostumeHidden(bool hidden) noexcept
{
	m_bHideBodyCostume = hidden;
	ChatPacket(CHAT_TYPE_COMMAND, "SetBodyCostumeHidden %d", m_bHideBodyCostume ? 1 : 0);
	SetQuestFlag("costume_option.hide_body", m_bHideBodyCostume ? 1 : 0);
}

void CHARACTER::SetHairCostumeHidden(bool hidden) noexcept
{
	m_bHideHairCostume = hidden;
	ChatPacket(CHAT_TYPE_COMMAND, "SetHairCostumeHidden %d", m_bHideHairCostume ? 1 : 0);
	SetQuestFlag("costume_option.hide_hair", m_bHideHairCostume ? 1 : 0);
}

# ifdef ENABLE_ACCE_COSTUME_SYSTEM
void CHARACTER::SetAcceCostumeHidden(bool hidden) noexcept
{
	m_bHideAcceCostume = hidden;
	ChatPacket(CHAT_TYPE_COMMAND, "SetAcceCostumeHidden %d", m_bHideAcceCostume ? 1 : 0);
	SetQuestFlag("costume_option.hide_acce", m_bHideAcceCostume ? 1 : 0);
}
# endif

# ifdef ENABLE_WEAPON_COSTUME_SYSTEM
void CHARACTER::SetWeaponCostumeHidden(bool hidden) noexcept
{
	m_bHideWeaponCostume = hidden;
	ChatPacket(CHAT_TYPE_COMMAND, "SetWeaponCostumeHidden %d", m_bHideWeaponCostume ? 1 : 0);
	SetQuestFlag("costume_option.hide_weapon", m_bHideWeaponCostume ? 1 : 0);
}
# endif

#ifdef ENABLE_AURA_SYSTEM
void CHARACTER::SetAuraCostumeHidden(bool hidden) noexcept
{
	m_bHideAuraCostume = hidden;
	ChatPacket(CHAT_TYPE_COMMAND, "SetAuraCostumeHidden %d", m_bHideAuraCostume ? 1 : 0);
	SetQuestFlag("costume_option.hide_aura", m_bHideAuraCostume ? 1 : 0);
}
# endif
#endif

#ifdef ENABLE_CHECK_WINDOW_RENEWAL
/*
	USAGE:
	if (ch->GetOpenedWindow(W_SAFEBOX | W_SHOP_OWNER | W_CUBE | W_EXCHANGE | W_SKILLBOOK_COMB | W_ATTR_6TH_7TH | W_AURA))
		// do something
*/
uint32_t CHARACTER::GetOpenedWindow(uint32_t dwWindow) const noexcept
{
	return (IS_SET(m_dwOpenedWindow, dwWindow));
}

void CHARACTER::SetOpenedWindow(uint32_t dwWindow, bool bIsOpen) noexcept
{
	if (dwWindow >= W_MAX_NUM)
		return;

	if (bIsOpen)
	{
		if (IS_SET(m_dwOpenedWindow, dwWindow))
			return;

		SET_BIT(m_dwOpenedWindow, dwWindow);
	}
	else
	{
		if (!IS_SET(m_dwOpenedWindow, dwWindow))
			return;

		REMOVE_BIT(m_dwOpenedWindow, dwWindow);
	}
}
#endif

#ifdef ENABLE_DEFENSE_WAVE
bool CHARACTER::IsMast() const noexcept
{
	if (GetRaceNum() == DEFENSE_WAVE_MAST_VNUM)
		return true;

	return false;
}

bool CHARACTER::IsShipSteeringWheel() const noexcept
{
	if (GetRaceNum() == 20436)
		return true;

	return false;
}

bool CHARACTER::IsDefenseWavePortal() const noexcept
{
	if (GetRaceNum() == DEFENSE_WAVE_PORTAL)
		return true;

	return false;
}

bool CHARACTER::IsHydraBoss() const noexcept
{
	if (GetRaceNum() >= DEFENSE_WAVE_HYDRA1 && GetRaceNum() <= DEFENSE_WAVE_HYDRA3)
		return true;

	return false;
}

bool CHARACTER::IsDefenseWaveMobs() const noexcept
{
	if ((GetRaceNum() >= 3950 && GetRaceNum() <= 3955) && GetRaceNum() != 3953)
		return true;

	return false;
}

bool CHARACTER::IsDefenseWaveMastAttackMob(int iMobVnum) const noexcept
{
	const auto wRaceNum = iMobVnum != 0 ? iMobVnum : GetRaceNum();

	if ((wRaceNum >= 3950 && wRaceNum <= 3964))
		return true;

	return false;
}
#endif

#ifdef ENABLE_MOUNT_CHECK
void CHARACTER::Unmount(LPCHARACTER ch)
{
	if (!ch)
		return;

	if (ch->IsHorseRiding())
	{
		ch->StopRiding();
		ch->HorseSummon(false);
	}

	ch->RemoveAffect(AFFECT_MOUNT);
	ch->RemoveAffect(AFFECT_MOUNT_BONUS);
	if (ch->IsRiding())
		ch->StopRiding();

#	ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	CItem* pMount = ch->GetWear(WEAR_COSTUME_MOUNT);
	if (pMount)
	{
		if (pMount->IsEquipped())
			ch->UnequipItem(pMount);
	}
#	endif
}
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
std::unique_ptr<TAchievementsMap>& CHARACTER::GetAchievementInfo()
{
	return _achievement;
}

void CHARACTER::SetAchievementInfo(TAchievementsMap& achievement)
{
	_achievement = std::make_unique<TAchievementsMap>(achievement);
}
#endif

#ifdef ENABLE_SUNG_MAHI_TOWER
void CHARACTER::SetDungeonMultipliers(uint8_t dungeonLevel)
{
	if (dungeonLevel < 1 || dungeonLevel > 50)
		return;

	PointChange(POINT_DEF_GRADE, GetPoint(POINT_DEF_GRADE) + (dungeonLevel * SUNG_MAHI_DAMANGE_MULTIPLIER));
	PointChange(POINT_ATT_GRADE, GetPoint(POINT_ATT_GRADE) + (dungeonLevel * SUNG_MAHI_DEFENCE_MULTIPLIER));

	SetMaxHP(GetMaxHP() + (dungeonLevel * SUNG_MAHI_HP_MULTIPLIER));
	SetHP(GetMaxHP());
}

uint8_t CHARACTER::GetSungMahiTowerDungeonValue(uint16_t wPoint)
{
	int aSungmaTowerValue[4][51] =
	{
		{ /*POINT_SUNGMA_STR*/ 0, 35, 35, 35, 35, 35, 38, 38, 38, 38, 38, 42, 42, 42, 42, 42, 45, 45, 45, 45, 45, 48, 48, 48, 48, 48, 52, 52, 52, 52, 52, 56, 56, 56, 56, 56, 60, 60, 60, 60, 60, 65, 65, 65, 65, 65, 70, 70, 70, 70, 70},
		{ /*POINT_SUNGMA_HP */ 0 , 25, 25, 25, 25, 25, 27, 27, 27, 27, 27, 30, 30, 30, 30, 30, 33, 33, 33, 33, 33, 36, 36, 36, 36, 36, 40, 40, 40, 40, 40, 44, 44, 44, 44, 44, 48, 48, 48, 48, 48, 53, 53, 53, 53, 53, 58, 58, 58, 58, 58},
		{ /*POINT_SUNGMA_MOVE */ 0 , 35, 35, 35, 35, 35, 38, 38, 38, 38, 38, 42, 42, 42, 42, 42, 45, 45, 45, 45, 45, 48, 48, 48, 48, 48, 52, 52, 52, 52, 52, 56, 56, 56, 56, 56, 60, 60, 60, 60, 60, 65, 65, 65, 65, 65, 70, 70, 70, 70, 70},
		{ /*POINT_SUNGMA_IMMUNE */ 0 , 55, 55, 55, 55, 55, 60, 60, 60, 60, 60, 64, 64, 64, 64, 64, 67, 67, 67, 67, 67, 70, 70, 70, 70, 70, 73, 73, 73, 73, 73, 77, 77, 77, 77, 77, 81, 81, 81, 81, 81, 85, 85, 85, 85, 85, 90, 90, 90, 90, 90}
	};

	uint8_t sungmaPoint = 0;
	LPDUNGEON dungeonInstance = GetDungeon();
	if (dungeonInstance)
	{
		const uint8_t bDungeonState = dungeonInstance->GetFlag("dungeonLevel");
		sys_err("DUNGEON STATE %d", bDungeonState);
		if (bDungeonState)
			sungmaPoint = aSungmaTowerValue[wPoint][bDungeonState];
	}

	return sungmaPoint;
}
#endif
