#pragma once

#ifdef ENABLE_GRAPHIC_ON_OFF
class CPythonGraphicOnOff : public CSingleton<CPythonGraphicOnOff>
{
	private:
		enum EActorType
		{
			TYPE_ENEMY,
			TYPE_NPC,
			TYPE_STONE,
			TYPE_WARP,
			TYPE_DOOR,
			TYPE_BUILDING,
			TYPE_PC,
			TYPE_POLY,
			TYPE_HORSE,
			TYPE_GOTO,
			TYPE_OBJECT,
		};

		enum EMonsterRanks
		{
			MOB_RANK_PAWN,
			MOB_RANK_S_PAWN,
			MOB_RANK_KNIGHT,
			MOB_RANK_S_KNIGHT,
			MOB_RANK_BOSS,
			MOB_RANK_KING,
			MOB_RANK_MAX_NUM
		};

	public:
		enum EEffectLevels
		{
			EFFECT_ONOFF_LEVEL_ALL,
			EFFECT_ONOFF_LEVEL_SELF_MONSTER,
			EFFECT_ONOFF_LEVEL_SELF_PLAYER,
			EFFECT_ONOFF_LEVEL_SELF_ONLY,
			EFFECT_ONOFF_LEVEL_NONE,
		};
		enum EPrivateShopLevels
		{
			PRIVATE_SHOP_ONOFF_LEVEL_FAR,
			PRIVATE_SHOP_ONOFF_LEVEL_BIG_RANGE,
			PRIVATE_SHOP_ONOFF_LEVEL_MIDLE_RANGE,
			PRIVATE_SHOP_ONOFF_LEVEL_SMALL_RANGE,
			PRIVATE_SHOP_ONOFF_LEVEL_CLOSE,
		};
		enum EItemDropLevels
		{
			ITEM_DROP_ONOFF_LEVEL_NAME_N_EFFECT,
			ITEM_DROP_ONOFF_LEVEL_HIDE_EFFECT,
			ITEM_DROP_ONOFF_LEVEL_HIDE_EFFECT_N_NAME,
			ITEM_DROP_ONOFF_LEVEL_HIDE_EX_CURSOR,
			ITEM_DROP_ONOFF_LEVEL_NONE,
		};
		enum EPetStatus
		{
			PET_STATUS_SHOW,
			PET_STATUS_HIDE,
		};
		enum ENPCNameStatus
		{
			NPC_NAME_STATUS_SHOW,
			NPC_NAME_STATUS_HIDE,
		};

		CPythonGraphicOnOff();
		virtual ~CPythonGraphicOnOff();

		void Destroy();

		bool SetEffectOnOffLevel(uint8_t bLevel);
		uint8_t GetEffectOnOffLevel() const;
		bool IsEffectOnOffLevel(uint8_t bActorType, bool isMain, uint8_t bRank) const;

		bool SetPrivateShopOnOffLevel(uint8_t bLevel);
		uint8_t GetPrivateShopOnOffLevel() const;
		bool IsPrivateShopOnOffLevel(uint32_t dwRange) const;

		bool SetItemDropOnOffLevel(uint8_t bLevel);
		uint8_t GetItemDropOnOffLevel();
		bool IsItemDropOnOffLevel(uint8_t bLevel) const;

		bool SetPetOnOffStatus(uint8_t bStatus);
		uint8_t GetPetOnOffStatus();
		bool IsPetOnOffStatus() const;

		bool SetNPCNameOnOffStatus(uint8_t bStatus);
		uint8_t GetNPCNameOnOffStatus();
		bool IsNPCNameOnOffStatus() const;

	protected:
		uint8_t m_bEffectLevelMin, m_bEffectLevelMax;
		uint8_t m_bPrivateShopLevelMin, m_bPrivateShopLevelMax;
		uint8_t m_bItemDropLevelMin, m_bItemDropLevelMax;
		uint8_t m_bPetStatusMin, m_bPetStatusMax;
		uint8_t m_bNPCNameStatusMin, m_bNPCNameStatusMax;

		uint8_t m_bEffectLevel;
		uint8_t m_bPrivateShopLevel;
		uint8_t m_bItemDropLevel;
		uint8_t m_bPetStatus;
		uint8_t m_bNPCNameStatus;
};
#endif
