
#include "stdafx.h"
#include "config.h"

#include "DragonLair.h"

#include "entity.h"
#include "sectree_manager.h"
#include "char.h"
#include "guild.h"
#include "locale_service.h"
#include "regen.h"
#include "log.h"
#include "utils.h"

struct FWarpToDragronLairWithGuildMembers
{
	uint32_t dwGuildID;
	long mapIndex;
	long x, y;

	FWarpToDragronLairWithGuildMembers( uint32_t guildID, long map, long X, long Y )
		: dwGuildID(guildID), mapIndex(map), x(X), y(Y)
	{
	}

	void operator()(LPENTITY ent)
	{
		if (nullptr != ent && true == ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER pChar = static_cast<LPCHARACTER>(ent);

			if (true == pChar->IsPC())
			{
				if (nullptr != pChar->GetGuild())
				{
					if (dwGuildID == pChar->GetGuild()->GetID())
					{
						pChar->WarpSet(x, y, mapIndex);
					}
				}
			}
		}
	}
};

struct FWarpToVillage
{
	void operator() (LPENTITY ent)
	{
		if (nullptr != ent)
		{
			LPCHARACTER pChar = static_cast<LPCHARACTER>(ent);

			if (nullptr != pChar)
			{
				if (true == pChar->IsPC())
				{
					pChar->GoHome();
				}
			}
		}
	}
};

EVENTINFO(tag_DragonLair_Collapse_EventInfo)
{
	int step;
	CDragonLair* pLair;
	long InstanceMapIndex;

	tag_DragonLair_Collapse_EventInfo()
	: step( 0 )
	, pLair( 0 )
	, InstanceMapIndex( 0 )
	{
	}
};

EVENTFUNC( DragonLair_Collapse_Event )
{
	tag_DragonLair_Collapse_EventInfo* pInfo = dynamic_cast<tag_DragonLair_Collapse_EventInfo*>(event->info);

	if ( pInfo == nullptr )
	{
		sys_err( "DragonLair_Collapse_Event> <Factor> Null pointer" );
		return 0;
	}

	if (0 == pInfo->step)
	{
		char buf[512];
		snprintf(buf, 512, LC_TEXT("용가리가 %d 초만에 죽어써효ㅠㅠ"), pInfo->pLair->GetEstimatedTime());
		SendNoticeMap(buf, pInfo->InstanceMapIndex, true);

		pInfo->step++;

		return PASSES_PER_SEC( 30 );
	}
	else if (1 == pInfo->step)
	{
		LPSECTREE_MAP pMap = SECTREE_MANAGER::Instance().GetMap( pInfo->InstanceMapIndex );

		if (nullptr != pMap)
		{
			FWarpToVillage f;
			pMap->for_each( f );
		}

		pInfo->step++;

		return PASSES_PER_SEC( 30 );
	}
	else
	{
		SECTREE_MANAGER::Instance().DestroyPrivateMap( pInfo->InstanceMapIndex );
		M2_DELETE(pInfo->pLair);
	}

	return 0;
}

CDragonLair::CDragonLair(uint32_t guildID, long BaseMapID, long PrivateMapID)
	: GuildID_(guildID), BaseMapIndex_(BaseMapID), PrivateMapIndex_(PrivateMapID)
{
	StartTime_ = get_global_time();
}

CDragonLair::~CDragonLair()
{
}

uint32_t CDragonLair::GetEstimatedTime() const
{
	return get_global_time() - StartTime_;
}

void CDragonLair::OnDragonDead(LPCHARACTER pDragon)
{
	sys_log(0, "DragonLair: 도라곤이 죽어써효");

	LogManager::Instance().DragonSlayLog(  GuildID_, pDragon->GetMobTable().dwVnum, StartTime_, get_global_time() );
}

CDragonLairManager::CDragonLairManager()
{
}

CDragonLairManager::~CDragonLairManager()
{
}

bool CDragonLairManager::Start(long MapIndexFrom, long BaseMapIndex, uint32_t GuildID)
{
	long instanceMapIndex = SECTREE_MANAGER::Instance().CreatePrivateMap(BaseMapIndex);
	if (instanceMapIndex == 0) {
		sys_err("CDragonLairManager::Start() : no private map index available");
		return false;
	}

	LPSECTREE_MAP pMap = SECTREE_MANAGER::Instance().GetMap(MapIndexFrom);

	if (nullptr != pMap)
	{
		LPSECTREE_MAP pTargetMap = SECTREE_MANAGER::Instance().GetMap(BaseMapIndex);

		if (nullptr == pTargetMap)
		{
			return false;
		}

		const TMapRegion* pRegionInfo = SECTREE_MANAGER::Instance().GetMapRegion( pTargetMap->m_setting.iIndex );

		if (nullptr != pRegionInfo)
		{
			FWarpToDragronLairWithGuildMembers f(GuildID, instanceMapIndex, 844000, 1066900);

			pMap->for_each( f );

			LairMap_.insert( std::make_pair(GuildID, M2_NEW CDragonLair(GuildID, BaseMapIndex, instanceMapIndex)) );

			std::string strMapBasePath( LocaleService_GetMapPath() );

			strMapBasePath += "/" + pRegionInfo->strMapName + "/instance_regen.txt";

			sys_log(0, "%s", strMapBasePath.c_str());

			regen_do(strMapBasePath.c_str(), instanceMapIndex, pTargetMap->m_setting.iBaseX, pTargetMap->m_setting.iBaseY, nullptr, true);

			return true;
		}
	}

	return false;
}

void CDragonLairManager::OnDragonDead(LPCHARACTER pDragon, uint32_t KillerGuildID)
{
	if (nullptr == pDragon)
		return;

	if (false == pDragon->IsMonster())
		return;

	std::unordered_map<uint32_t, CDragonLair*>::iterator iter = LairMap_.find( KillerGuildID );

	if (LairMap_.end() == iter)
	{
		return;
	}

	LPSECTREE_MAP pMap = SECTREE_MANAGER::Instance().GetMap( pDragon->GetMapIndex() );

	if (nullptr == iter->second || nullptr == pMap)
	{
		LairMap_.erase( iter );
		return;
	}

	iter->second->OnDragonDead( pDragon );

	tag_DragonLair_Collapse_EventInfo* info;
	info = AllocEventInfo<tag_DragonLair_Collapse_EventInfo>();

	info->step = 0;
	info->pLair = iter->second;
	info->InstanceMapIndex = pDragon->GetMapIndex();

	event_create(DragonLair_Collapse_Event, info, PASSES_PER_SEC(10));

	LairMap_.erase( iter );
}
