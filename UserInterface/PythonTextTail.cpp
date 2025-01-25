//
// 캐릭터를 따라다니는 텍스트 관련 소스 (이름, 길드이름, 길드마크 등)
//
#include "StdAfx.h"
#include "InstanceBase.h"
#include "resource.h"
#include "PythonTextTail.h"
#include "PythonCharacterManager.h"
#include "PythonGuild.h"
#include "Locale.h"
#include "MarkManager.h"
#ifdef WJ_SHOW_MOB_INFO
#	include "PythonNonPlayer.h"
#endif
#if defined(WJ_SHOW_MOB_INFO) || defined(ENABLE_EXTENDED_CONFIGS)
#	include "PythonSystem.h"
#endif
#ifdef ENABLE_BATTLE_FIELD
#	include "PythonBackground.h"
#endif
#ifdef ENABLE_GRAPHIC_ON_OFF
#	include "PythonPlayer.h"
#endif

const D3DXCOLOR c_TextTail_Player_Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
const D3DXCOLOR c_TextTail_Monster_Color = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);
const D3DXCOLOR c_TextTail_Item_Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
#ifdef ENABLE_EXTENDED_ITEMNAME_ON_GROUND
const D3DXCOLOR c_TextTail_SpecialItem_Color = D3DXCOLOR(1.0f, 0.67f, 0.0f, 1.0f); //Golden
#endif
const D3DXCOLOR c_TextTail_Chat_Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
const D3DXCOLOR c_TextTail_Info_Color = D3DXCOLOR(1.0f, 0.785f, 0.785f, 1.0f);
const D3DXCOLOR c_TextTail_Guild_Name_Color = 0xFFEFD3FF;
const float c_TextTail_Name_Position = -10.0f;
const float c_fxMarkPosition = 1.5f;
const float c_fyGuildNamePosition = 15.0f;
const float c_fyMarkPosition = 15.0f + 11.0f;
BOOL bPKTitleEnable = TRUE;
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
#if defined(ENABLE_TEAM_GAMEMASTER)
const float c_fxLanguageIconPosition = 8.0f;
#else
const float c_fxLanguageIconPosition = 12.0f;
#endif
const float c_fyLanguageIconPosition = 10.0f;
#endif

// TEXTTAIL_LIVINGTIME_CONTROL
long gs_TextTail_LivingTime = 5000;

long TextTail_GetLivingTime()
{
	assert(gs_TextTail_LivingTime > 1000);
	return gs_TextTail_LivingTime;
}

void TextTail_SetLivingTime(long livingTime)
{
	gs_TextTail_LivingTime = livingTime;
}
// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

CGraphicText * ms_pFont = nullptr;

void CPythonTextTail::GetInfo(std::string * pstInfo)
{
	char szInfo[256];
	sprintf(szInfo, "TextTail: ChatTail %u, ChrTail (Map %u, List %u), ItemTail (Map %u, List %u), Pool %u", m_ChatTailMap.size(),
			m_CharacterTextTailMap.size(), m_CharacterTextTailList.size(), m_ItemTextTailMap.size(), m_ItemTextTailList.size(),
			m_TextTailPool.GetCapacity());

	pstInfo->append(szInfo);
}

void CPythonTextTail::UpdateAllTextTail()
{
	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
	if (pInstance)
	{
		TPixelPosition pixelPos;
		pInstance->NEW_GetPixelPosition(&pixelPos);

		TTextTailMap::iterator itorMap;

		for (itorMap = m_CharacterTextTailMap.begin(); itorMap != m_CharacterTextTailMap.end(); ++itorMap)
		{
#ifdef ENABLE_GRAPHIC_ON_OFF
			CInstanceBase* pInst = CPythonCharacterManager::Instance().GetInstancePtr(itorMap->second->dwVirtualID);
			if (!pInst)
				continue;

			if (!pInst->IsShowActor())
				continue;

			uint32_t dwPickedActorID = 0;
			if (!pInst->IsNPC()
#	ifdef ENABLE_GROWTH_PET_SYSTEM
				|| !pInst->IsGrowthPet()
#	endif
				|| CPythonGraphicOnOff::Instance().IsNPCNameOnOffStatus()
				|| CPythonPlayer::Instance().GetPickedActorID(&dwPickedActorID)
				&& dwPickedActorID == itorMap->second->dwVirtualID)
			{
				_asm nop;
			}
			else
			{
				continue;
			}
#endif
			UpdateDistance(pixelPos, itorMap->second);
		}

#ifdef ENABLE_GRAPHIC_ON_OFF
		uint32_t dwPickedItemID;
		if (CPythonGraphicOnOff::Instance().IsItemDropOnOffLevel(CPythonGraphicOnOff::ITEM_DROP_ONOFF_LEVEL_HIDE_EFFECT_N_NAME))
		{
			for (itorMap = m_ItemTextTailMap.begin(); itorMap != m_ItemTextTailMap.end(); ++itorMap)
			{
				UpdateDistance(pixelPos, itorMap->second);
			}
		}
		else if (CPythonGraphicOnOff::Instance().IsItemDropOnOffLevel(CPythonGraphicOnOff::ITEM_DROP_ONOFF_LEVEL_NONE)
			&& CPythonPlayer::Instance().GetPickedItemID(&dwPickedItemID))
		{
			itorMap = m_ItemTextTailMap.find(dwPickedItemID);
			if (itorMap != m_ItemTextTailMap.end())
				UpdateDistance(pixelPos, itorMap->second);
		}
#else
		for (itorMap = m_ItemTextTailMap.begin(); itorMap != m_ItemTextTailMap.end(); ++itorMap)
			UpdateDistance(pixelPos, itorMap->second);
#endif

		for (auto & itorChat : m_ChatTailMap)
		{
			UpdateDistance(pixelPos, itorChat.second);

			// NOTE : Chat TextTail이 있으면 캐릭터 이름도 출력한다.
			if (itorChat.second->bNameFlag)
			{
				uint32_t dwVID = itorChat.first;
				ShowCharacterTextTail(dwVID);
			}
		}
	}
}

void CPythonTextTail::UpdateShowingTextTail()
{
	TTextTailList::iterator itor;

#ifdef ENABLE_GRAPHIC_ON_OFF
	if (CPythonGraphicOnOff::Instance().IsItemDropOnOffLevel(CPythonGraphicOnOff::ITEM_DROP_ONOFF_LEVEL_NONE))
#endif
	for (itor = m_ItemTextTailList.begin(); itor != m_ItemTextTailList.end(); ++itor)
		UpdateTextTail(*itor);

	for (auto & itorChat : m_ChatTailMap)
		UpdateTextTail(itorChat.second);

	for (itor = m_CharacterTextTailList.begin(); itor != m_CharacterTextTailList.end(); ++itor)
	{
		TTextTail * pTextTail = *itor;

#ifdef ENABLE_GRAPHIC_ON_OFF
		CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(pTextTail->dwVirtualID);
		if (!pInstance)
			continue;

		if (!pInstance->IsShowActor())
			continue;
#endif

		UpdateTextTail(pTextTail);

		// NOTE : Chat TextTail이 있을 경우 위치를 바꾼다.
		auto itor = m_ChatTailMap.find(pTextTail->dwVirtualID);
		if (m_ChatTailMap.end() != itor)
		{
			TTextTail * pChatTail = itor->second;
			if (pChatTail->bNameFlag)
				pTextTail->y = pChatTail->y - 17.0f;
		}
	}
}

void CPythonTextTail::UpdateTextTail(TTextTail * pTextTail) const
{
	if (!pTextTail->pOwner)
		return;

	/////

	CPythonGraphic & rpyGraphic = CPythonGraphic::Instance();
	rpyGraphic.Identity();

	const D3DXVECTOR3 & c_rv3Position = pTextTail->pOwner->GetPosition();
	rpyGraphic.ProjectPosition(c_rv3Position.x, c_rv3Position.y, c_rv3Position.z + pTextTail->fHeight, &pTextTail->x, &pTextTail->y,
							   &pTextTail->z);

	pTextTail->x = floorf(pTextTail->x);
	pTextTail->y = floorf(pTextTail->y);

	// NOTE : 13m 밖에 있을때만 깊이를 넣습니다 - [levites]
	if (pTextTail->fDistanceFromPlayer < 1300.0f)
		pTextTail->z = 0.0f;
	else
	{
		pTextTail->z = pTextTail->z * CPythonGraphic::Instance().GetOrthoDepth() * -1.0f;
		pTextTail->z += 10.0f;
	}
}

void CPythonTextTail::ArrangeTextTail()
{
	TTextTailList::iterator itor;

	uint32_t dwTime = CTimer::Instance().GetCurrentMillisecond();

#ifdef ENABLE_GRAPHIC_ON_OFF
	uint32_t dwPickedItemID;
	if (CPythonGraphicOnOff::Instance().IsItemDropOnOffLevel(CPythonGraphicOnOff::ITEM_DROP_ONOFF_LEVEL_HIDE_EFFECT_N_NAME))
	{
		for (itor = m_ItemTextTailList.begin(); itor != m_ItemTextTailList.end(); ++itor)
		{
			TTextTail* pInsertTextTail = *itor;

			int yTemp = 5;
			int LimitCount = 0;

			for (auto itorCompare = m_ItemTextTailList.begin(); itorCompare != m_ItemTextTailList.end();)
			{
				TTextTail* pCompareTextTail = *itorCompare;

				if (*itorCompare == *itor)
				{
					++itorCompare;
					continue;
				}

				if (LimitCount >= 20)
					break;

				if (isIn(pInsertTextTail, pCompareTextTail))
				{
					pInsertTextTail->y = (pCompareTextTail->y + pCompareTextTail->yEnd + yTemp);

					itorCompare = m_ItemTextTailList.begin();
					++LimitCount;
					continue;
				}

				++itorCompare;
			}

			if (pInsertTextTail->pOwnerTextInstance)
			{
				pInsertTextTail->pOwnerTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y, pInsertTextTail->z);
				pInsertTextTail->pOwnerTextInstance->Update();
				pInsertTextTail->pTextInstance->SetColor(pInsertTextTail->Color.r, pInsertTextTail->Color.g, pInsertTextTail->Color.b);
				pInsertTextTail->pTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y + 15.0f, pInsertTextTail->z);
			}
			else
			{
				pInsertTextTail->pTextInstance->SetColor(pInsertTextTail->Color.r, pInsertTextTail->Color.g, pInsertTextTail->Color.b);
				pInsertTextTail->pTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y, pInsertTextTail->z);
			}
			pInsertTextTail->pTextInstance->Update();
		}
	}
	else if (CPythonGraphicOnOff::Instance().IsItemDropOnOffLevel(CPythonGraphicOnOff::ITEM_DROP_ONOFF_LEVEL_NONE)
		&& CPythonPlayer::Instance().GetPickedItemID(&dwPickedItemID))
	{
		TTextTailMap::iterator itor2 = m_ItemTextTailMap.find(dwPickedItemID);
		if (itor2 != m_ItemTextTailMap.end())
		{
			TTextTail* pTextTail = itor2->second;
			itor = std::find(m_ItemTextTailList.begin(), m_ItemTextTailList.end(), pTextTail);
			if (m_ItemTextTailList.end() != itor)
			{
				TTextTail* pInsertTextTail = *itor;
				if (pInsertTextTail->pOwnerTextInstance)
				{
					pInsertTextTail->pOwnerTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y, pInsertTextTail->z);
					pInsertTextTail->pOwnerTextInstance->Update();
					pInsertTextTail->pTextInstance->SetColor(pInsertTextTail->Color.r, pInsertTextTail->Color.g, pInsertTextTail->Color.b);
					pInsertTextTail->pTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y + 15.0f, pInsertTextTail->z);
				}
				else
				{
					pInsertTextTail->pTextInstance->SetColor(pInsertTextTail->Color.r, pInsertTextTail->Color.g, pInsertTextTail->Color.b);
					pInsertTextTail->pTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y, pInsertTextTail->z);
				}
				pInsertTextTail->pTextInstance->Update();
			}
		}
	}
#else
	for (itor = m_ItemTextTailList.begin(); itor != m_ItemTextTailList.end(); ++itor)
	{
		TTextTail * pInsertTextTail = *itor;

		int yTemp = 5;
		int LimitCount = 0;

		for (auto itorCompare = m_ItemTextTailList.begin(); itorCompare != m_ItemTextTailList.end();)
		{
			TTextTail * pCompareTextTail = *itorCompare;

			if (*itorCompare == *itor)
			{
				++itorCompare;
				continue;
			}

			if (LimitCount >= 20)
				break;

			if (isIn(pInsertTextTail, pCompareTextTail))
			{
				pInsertTextTail->y = (pCompareTextTail->y + pCompareTextTail->yEnd + yTemp);

				itorCompare = m_ItemTextTailList.begin();
				++LimitCount;
				continue;
			}

			++itorCompare;
		}


		if (pInsertTextTail->pOwnerTextInstance)
		{
			pInsertTextTail->pOwnerTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y, pInsertTextTail->z);
			pInsertTextTail->pOwnerTextInstance->Update();

			pInsertTextTail->pTextInstance->SetColor(pInsertTextTail->Color.r, pInsertTextTail->Color.g, pInsertTextTail->Color.b);
			pInsertTextTail->pTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y + 15.0f, pInsertTextTail->z);
			pInsertTextTail->pTextInstance->Update();
		}
		else
		{
			pInsertTextTail->pTextInstance->SetColor(pInsertTextTail->Color.r, pInsertTextTail->Color.g, pInsertTextTail->Color.b);
			pInsertTextTail->pTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y, pInsertTextTail->z);
			pInsertTextTail->pTextInstance->Update();
		}
	}
#endif

	for (itor = m_CharacterTextTailList.begin(); itor != m_CharacterTextTailList.end(); ++itor)
	{
		TTextTail * pTextTail = *itor;

#ifdef ENABLE_GRAPHIC_ON_OFF
		CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(pTextTail->dwVirtualID);
		if (!pInstance)
			continue;

		if (!pInstance->IsShowActor())
			continue;

		uint32_t dwPickedActorID = 0;
		if (!pInstance->IsNPC()
#	ifdef ENABLE_GROWTH_PET_SYSTEM
			|| !pInstance->IsGrowthPet()
#	endif
			|| CPythonGraphicOnOff::Instance().IsNPCNameOnOffStatus()
			|| CPythonPlayer::Instance().GetPickedActorID(&dwPickedActorID)
			&& dwPickedActorID == pTextTail->dwVirtualID)
		{
			_asm nop;
		}
		else
		{
			continue;
		}
#endif

		float fxAdd = 0.0f;
#ifdef ENABLE_BATTLE_FIELD
		float fyRanking = 5.0f;
#endif

		// Mark
		CGraphicMarkInstance * pMarkInstance = pTextTail->pMarkInstance;
		CGraphicTextInstance * pGuildNameInstance = pTextTail->pGuildNameTextInstance;

		if (pMarkInstance && pGuildNameInstance)
		{
			int iWidth, iHeight;
			int iImageHalfSize = pMarkInstance->GetWidth() / 2 + c_fxMarkPosition;
			pGuildNameInstance->GetTextSize(&iWidth, &iHeight);
#ifdef ENABLE_BATTLE_FIELD
			fyRanking = 20.0f;
#endif

			pMarkInstance->SetPosition(pTextTail->x - iWidth / 2 - iImageHalfSize, pTextTail->y - c_fyMarkPosition);
			pGuildNameInstance->SetPosition(pTextTail->x + iImageHalfSize, pTextTail->y - c_fyGuildNamePosition, pTextTail->z);
			pGuildNameInstance->Update();
		}

#ifdef ENABLE_BATTLE_FIELD
		CGraphicImageInstance* pRankingInstance = pTextTail->pRankingInstance;
		if (pRankingInstance)
		{
			int iWidthHalfSize = pRankingInstance->GetWidth() / 2;
			pRankingInstance->SetPosition(pTextTail->x - iWidthHalfSize + 1.5f, pTextTail->y - pRankingInstance->GetHeight() - fyRanking);
		}
#endif

		int iNameWidth, iNameHeight;
		pTextTail->pTextInstance->GetTextSize(&iNameWidth, &iNameHeight);

		// Title
		CGraphicTextInstance * pTitle = pTextTail->pTitleTextInstance;
		if (pTitle)
		{
			int iTitleWidth, iTitleHeight;
			pTitle->GetTextSize(&iTitleWidth, &iTitleHeight);
			fxAdd = 4.0f;	// @fixme400	(8.0f -> 4.0f)
			if (GetDefaultCodePage() == CP_ARABIC)
				pTitle->SetPosition(pTextTail->x - (iNameWidth / 2) - iTitleWidth - fxAdd, pTextTail->y, pTextTail->z);
			else
				pTitle->SetPosition(pTextTail->x - (iNameWidth / 2), pTextTail->y, pTextTail->z);
			pTitle->Update();

			// Level 위치 업데이트
			CGraphicTextInstance * pLevel = pTextTail->pLevelTextInstance;
			if (pLevel)
			{
				int iLevelWidth, iLevelHeight;
				pLevel->GetTextSize(&iLevelWidth, &iLevelHeight);
				if (GetDefaultCodePage() == CP_ARABIC)
					pLevel->SetPosition(pTextTail->x - (iNameWidth / 2) - iLevelWidth - iTitleWidth - fxAdd, pTextTail->y, pTextTail->z);
				else
					pLevel->SetPosition(pTextTail->x - (iNameWidth / 2) - iTitleWidth, pTextTail->y, pTextTail->z);
				pLevel->Update();

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
				CGraphicImageInstance* pLanguageImage = pTextTail->pLanguageImageInstance;
				if (pLanguageImage)
				//	pLanguageImage->SetPosition(pTextTail->x + fxAdd + (iNameWidth / 2) + 1.0f + c_fxLanguageIconPosition, pTextTail->y - c_fyLanguageIconPosition);
				{
					int iLevelWidth, iLevelHeight;
					pLevel->GetTextSize(&iLevelWidth, &iLevelHeight);
					if (GetDefaultCodePage() == CP_ARABIC)
						pLanguageImage->SetPosition(pTextTail->x - (iNameWidth / 2) - iTitleWidth - iLevelWidth - fxAdd - c_fxLanguageIconPosition/*pLanguageImage->GetWidth()*/ - 12.0f, pTextTail->y - c_fyLanguageIconPosition);
					else
						pLanguageImage->SetPosition(pTextTail->x - (iNameWidth / 2) - iTitleWidth - iLevelWidth - c_fxLanguageIconPosition/*pLanguageImage->GetWidth()*/ - 12.0f, pTextTail->y - c_fyLanguageIconPosition);
				}
#endif
			}
		}
		else
		{
			fxAdd = 4.0f;

			// Level
			CGraphicTextInstance * pLevel = pTextTail->pLevelTextInstance;
			if (pLevel)
			{
				int iLevelWidth, iLevelHeight;
				pLevel->GetTextSize(&iLevelWidth, &iLevelHeight);
				if (GetDefaultCodePage() == CP_ARABIC)
					pLevel->SetPosition(pTextTail->x - (iNameWidth / 2) - iLevelWidth - 4.0f, pTextTail->y, pTextTail->z);
				else
					pLevel->SetPosition(pTextTail->x - (iNameWidth / 2), pTextTail->y, pTextTail->z);
				pLevel->Update();

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
				CGraphicImageInstance* pLanguageImage = pTextTail->pLanguageImageInstance;
				if (pLanguageImage)
				//	pLanguageImage->SetPosition(pTextTail->x + fxAdd + (iNameWidth / 2) + 1.0f + c_fxLanguageIconPosition, pTextTail->y - c_fyLanguageIconPosition);
					if (GetDefaultCodePage() == CP_ARABIC)
						pLanguageImage->SetPosition(pTextTail->x - (iNameWidth / 2) - iLevelWidth - fxAdd - c_fxLanguageIconPosition/*pLanguageImage->GetWidth()*/ - 8.0f, pTextTail->y - c_fyLanguageIconPosition);
					else
						pLanguageImage->SetPosition(pTextTail->x - (iNameWidth / 2) - iLevelWidth - c_fxLanguageIconPosition/*pLanguageImage->GetWidth()*/ - 8.0f, pTextTail->y - c_fyLanguageIconPosition);
#endif
			}
		}

		pTextTail->pTextInstance->SetColor(pTextTail->Color.r, pTextTail->Color.g, pTextTail->Color.b);
		pTextTail->pTextInstance->SetPosition(pTextTail->x + fxAdd, pTextTail->y, pTextTail->z);
		pTextTail->pTextInstance->Update();

#ifdef WJ_SHOW_MOB_INFO
		CGraphicTextInstance* pAIFlag = pTextTail->pAIFlagTextInstance;
		if (pAIFlag)
		{
			pAIFlag->SetColor(pTextTail->Color.r, pTextTail->Color.g, pTextTail->Color.b);
			if (GetDefaultCodePage() == CP_ARABIC)
				pAIFlag->SetPosition(pTextTail->x - (iNameWidth / 2) - fxAdd + 10.0f, pTextTail->y, pTextTail->z);
			else
				pAIFlag->SetPosition(pTextTail->x + fxAdd + (iNameWidth / 2) + 1.0f, pTextTail->y, pTextTail->z);//+1.0f is not neccesarry
			pAIFlag->Update();
		}
#endif
	}

	for (auto itorChat = m_ChatTailMap.begin(); itorChat != m_ChatTailMap.end();)
	{
		TTextTail * pTextTail = itorChat->second;

		if (pTextTail->LivingTime < dwTime)
		{
			DeleteTextTail(pTextTail);
			itorChat = m_ChatTailMap.erase(itorChat);
			continue;
		}
		{
			++itorChat;
		}

		pTextTail->pTextInstance->SetColor(pTextTail->Color);
		pTextTail->pTextInstance->SetPosition(pTextTail->x, pTextTail->y, pTextTail->z);
		pTextTail->pTextInstance->Update();
	}
}

void CPythonTextTail::Render()
{
	TTextTailList::iterator itor;

	for (itor = m_CharacterTextTailList.begin(); itor != m_CharacterTextTailList.end(); ++itor)
	{
		TTextTail * pTextTail = *itor;

#ifdef ENABLE_GRAPHIC_ON_OFF
		CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(pTextTail->dwVirtualID);
		if (!pInstance)
			continue;

		if (!pInstance->IsShowActor())
			continue;

		uint32_t dwPickedActorID = 0;
		if (!pInstance->IsNPC()
#	ifdef ENABLE_GROWTH_PET_SYSTEM
			|| !pInstance->IsGrowthPet()
#	endif
			|| CPythonGraphicOnOff::Instance().IsNPCNameOnOffStatus()
			|| CPythonPlayer::Instance().GetPickedActorID(&dwPickedActorID)
			&& dwPickedActorID == pTextTail->dwVirtualID)
		{
			_asm nop;
		}
		else
		{
			continue;
		}
#endif

		pTextTail->pTextInstance->Render();
		if (pTextTail->pMarkInstance && pTextTail->pGuildNameTextInstance)
		{
			pTextTail->pMarkInstance->Render();
			pTextTail->pGuildNameTextInstance->Render();
		}
		if (pTextTail->pTitleTextInstance)
			pTextTail->pTitleTextInstance->Render();
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		if (pTextTail->pLanguageImageInstance && CPythonSystem::Instance().IsShowPlayerLanguage())
			pTextTail->pLanguageImageInstance->Render();
#endif
#ifdef ENABLE_BATTLE_FIELD
		if (pTextTail->pRankingInstance)
			pTextTail->pRankingInstance->Render();
#endif
#ifdef WJ_SHOW_MOB_INFO
		if (pTextTail->pLevelTextInstance && (pTextTail->bIsPC == TRUE || CPythonSystem::Instance().IsShowMobLevel()))
#else
		if (pTextTail->pLevelTextInstance)
#endif
			pTextTail->pLevelTextInstance->Render();
#ifdef WJ_SHOW_MOB_INFO
		if (pTextTail->pAIFlagTextInstance && CPythonSystem::Instance().IsShowMobAIFlag())
			pTextTail->pAIFlagTextInstance->Render();
#endif
	}

#ifdef ENABLE_GRAPHIC_ON_OFF
	uint32_t dwPickedItemID;
	if (CPythonGraphicOnOff::Instance().IsItemDropOnOffLevel(CPythonGraphicOnOff::ITEM_DROP_ONOFF_LEVEL_HIDE_EFFECT_N_NAME))
	{
		for (itor = m_ItemTextTailList.begin(); itor != m_ItemTextTailList.end(); ++itor)
		{
			TTextTail* pTextTail = *itor;

			RenderTextTailBox(pTextTail);
			pTextTail->pTextInstance->Render();
			if (pTextTail->pOwnerTextInstance)
				pTextTail->pOwnerTextInstance->Render();
		}
	}
	else if (CPythonGraphicOnOff::Instance().IsItemDropOnOffLevel(CPythonGraphicOnOff::ITEM_DROP_ONOFF_LEVEL_NONE)
		&& CPythonPlayer::Instance().GetPickedItemID(&dwPickedItemID))
	{
		TTextTailMap::iterator itor2 = m_ItemTextTailMap.find(dwPickedItemID);
		if (itor2 != m_ItemTextTailMap.end())
		{
			itor = std::find(m_ItemTextTailList.begin(), m_ItemTextTailList.end(), itor2->second);
			if (itor != m_ItemTextTailList.end())
			{
				TTextTail* pTextTail = *itor;

				RenderTextTailBox(pTextTail);
				pTextTail->pTextInstance->Render();
				if (pTextTail->pOwnerTextInstance)
					pTextTail->pOwnerTextInstance->Render();
			}
		}
	}
#else
	for (itor = m_ItemTextTailList.begin(); itor != m_ItemTextTailList.end(); ++itor)
	{
		TTextTail * pTextTail = *itor;

		RenderTextTailBox(pTextTail);
		pTextTail->pTextInstance->Render();
		if (pTextTail->pOwnerTextInstance)
			pTextTail->pOwnerTextInstance->Render();
	}
#endif

	for (auto & itorChat : m_ChatTailMap)
	{
		TTextTail * pTextTail = itorChat.second;
		if (pTextTail->pOwner->isShow())
			RenderTextTailName(pTextTail);
	}
}

void CPythonTextTail::RenderTextTailBox(const TTextTail * pTextTail) const
{
	// 검은색 테두리
	CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 1.0f);
	CPythonGraphic::Instance().RenderBox2d(pTextTail->x + pTextTail->xStart, pTextTail->y + pTextTail->yStart,
										   pTextTail->x + pTextTail->xEnd, pTextTail->y + pTextTail->yEnd, pTextTail->z);

	// 검은색 투명박스
	CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 0.3f);
	CPythonGraphic::Instance().RenderBar2d(pTextTail->x + pTextTail->xStart, pTextTail->y + pTextTail->yStart,
										   pTextTail->x + pTextTail->xEnd, pTextTail->y + pTextTail->yEnd, pTextTail->z);
}

void CPythonTextTail::RenderTextTailName(TTextTail * pTextTail) const
{
	pTextTail->pTextInstance->Render();
}

void CPythonTextTail::HideAllTextTail()
{
	// NOTE : Show All을 해준뒤 Hide All을 해주지 않으면 문제 발생 가능성 있음
	//        디자인 자체가 그렇게 깔끔하게 되지 않았음 - [levites]
	m_CharacterTextTailList.clear();
	m_ItemTextTailList.clear();
}

void CPythonTextTail::UpdateDistance(const TPixelPosition & c_rCenterPosition, TTextTail * pTextTail) const
{
	const D3DXVECTOR3 & c_rv3Position = pTextTail->pOwner->GetPosition();
	D3DXVECTOR2 v2Distance(c_rv3Position.x - c_rCenterPosition.x, -c_rv3Position.y - c_rCenterPosition.y);
	pTextTail->fDistanceFromPlayer = D3DXVec2Length(&v2Distance);
}

void CPythonTextTail::ShowAllTextTail()
{
	TTextTailMap::iterator itor;
	for (itor = m_CharacterTextTailMap.begin(); itor != m_CharacterTextTailMap.end(); ++itor)
	{
		TTextTail * pTextTail = itor->second;
		if (pTextTail->fDistanceFromPlayer < 3500.0f)
			ShowCharacterTextTail(itor->first);
	}
	for (itor = m_ItemTextTailMap.begin(); itor != m_ItemTextTailMap.end(); ++itor)
	{
		TTextTail * pTextTail = itor->second;
		if (pTextTail->fDistanceFromPlayer < 3500.0f)
			ShowItemTextTail(itor->first);
	}
}

void CPythonTextTail::ShowCharacterTextTail(uint32_t VirtualID)
{
	auto itor = m_CharacterTextTailMap.find(VirtualID);

	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail * pTextTail = itor->second;

	if (m_CharacterTextTailList.end() != std::find(m_CharacterTextTailList.begin(), m_CharacterTextTailList.end(), pTextTail))
		return;

	// NOTE : ShowAll 시에는 모든 Instance 의 Pointer 를 찾아서 체크하므로 부하가 걸릴 가능성도 있다.
	//        CInstanceBase 가 TextTail 을 직접 가지고 있는 것이 가장 좋은 형태일 듯..
	if (!pTextTail->pOwner->isShow())
		return;

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(pTextTail->dwVirtualID);
	if (!pInstance)
		return;

	if (pInstance->IsGuildWall())
		return;

#ifdef ENABLE_EVENT_BANNER_FLAG
	if (pInstance->IsBannerFlag())
		return;
#endif

	if (pInstance->CanPickInstance())
		m_CharacterTextTailList.emplace_back(pTextTail);
}

void CPythonTextTail::ShowItemTextTail(uint32_t VirtualID)
{
	auto itor = m_ItemTextTailMap.find(VirtualID);

	if (m_ItemTextTailMap.end() == itor)
		return;

	TTextTail * pTextTail = itor->second;

	if (m_ItemTextTailList.end() != std::find(m_ItemTextTailList.begin(), m_ItemTextTailList.end(), pTextTail))
		return;

	m_ItemTextTailList.emplace_back(pTextTail);
}

bool CPythonTextTail::isIn(const TTextTail * pSource, TTextTail * pTarget) const
{
	float x1Source = pSource->x + pSource->xStart;
	float y1Source = pSource->y + pSource->yStart;
	float x2Source = pSource->x + pSource->xEnd;
	float y2Source = pSource->y + pSource->yEnd;
	float x1Target = pTarget->x + pTarget->xStart;
	float y1Target = pTarget->y + pTarget->yStart;
	float x2Target = pTarget->x + pTarget->xEnd;
	float y2Target = pTarget->y + pTarget->yEnd;

	if (x1Source <= x2Target && x2Source >= x1Target && y1Source <= y2Target && y2Source >= y1Target)
		return true;

	return false;
}

#ifdef ENABLE_BATTLE_FIELD
bool CPythonTextTail::IsBattleFieldMap()
{
	return CPythonBackground::Instance().IsBattleFieldMap();
}
#endif

#ifdef ENABLE_SHOW_GUILD_LEADER
void CPythonTextTail::RegisterCharacterTextTail(uint32_t dwGuildID, uint8_t dwNewIsGuildName, uint32_t dwVirtualID, const D3DXCOLOR & c_rColor, float fAddHeight)
#else
void CPythonTextTail::RegisterCharacterTextTail(uint32_t dwGuildID, uint32_t dwVirtualID, const D3DXCOLOR & c_rColor, float fAddHeight)
#endif
{
	CInstanceBase * pCharacterInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwVirtualID);

	if (!pCharacterInstance)
		return;

#ifndef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
	std::string name = pCharacterInstance->GetNameString();
	if (pCharacterInstance->IsShop())
	{
		CInstanceBase* pMainCharacterInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
		//Do not display shop names for those shops that aren't ours
		if (!pMainCharacterInstance || pMainCharacterInstance->GetNameString() != name)
			return;

		//Override the display shop name
		name = pMainCharacterInstance->GetNameString(); // My Shop
	}

	TTextTail* pTextTail =
		RegisterTextTail(dwVirtualID, name.c_str(), pCharacterInstance->GetGraphicThingInstancePtr(),
			pCharacterInstance->GetGraphicThingInstanceRef().GetHeight() + fAddHeight, c_rColor);
#else
	TTextTail * pTextTail =
		RegisterTextTail(dwVirtualID, pCharacterInstance->GetNameString(), pCharacterInstance->GetGraphicThingInstancePtr(),
			pCharacterInstance->GetGraphicThingInstanceRef().GetHeight() + fAddHeight, c_rColor);
#endif

	CGraphicTextInstance * pTextInstance = pTextTail->pTextInstance;
	pTextInstance->SetOutline(true);
	pTextInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);

	pTextTail->pMarkInstance = nullptr;
	pTextTail->pGuildNameTextInstance = nullptr;
	pTextTail->pTitleTextInstance = nullptr;
	pTextTail->pLevelTextInstance = nullptr;
#ifdef WJ_SHOW_MOB_INFO
	pTextTail->pAIFlagTextInstance = nullptr;
#endif
#ifdef ENABLE_BATTLE_FIELD
	pTextTail->pRankingInstance = nullptr;

	CGraphicImageInstance*& prRanking = pTextTail->pRankingInstance;
	if (!prRanking)
	{
		if (pCharacterInstance->IsAffect(CInstanceBase::AFFECT_BATTLE_RANKER_1))
		{
			CGraphicImage* pRankingImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer("D:/ymir work/effect/etc/ranking_battle/ranker_1.tga");
			if (pRankingImage) {
				prRanking = CGraphicImageInstance::New();
				prRanking->SetImagePointer(pRankingImage);
			}
		}
		else if (pCharacterInstance->IsAffect(CInstanceBase::AFFECT_BATTLE_RANKER_2))
		{
			CGraphicImage* pRankingImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer("D:/ymir work/effect/etc/ranking_battle/ranker_2.tga");
			if (pRankingImage) {
				prRanking = CGraphicImageInstance::New();
				prRanking->SetImagePointer(pRankingImage);
			}
		}
		else if (pCharacterInstance->IsAffect(CInstanceBase::AFFECT_BATTLE_RANKER_3))
		{
			CGraphicImage* pRankingImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer("D:/ymir work/effect/etc/ranking_battle/ranker_3.tga");
			if (pRankingImage) {
				prRanking = CGraphicImageInstance::New();
				prRanking->SetImagePointer(pRankingImage);
			}
		}
	}
#endif

	if (0 != dwGuildID)
	{
		pTextTail->pMarkInstance = CGraphicMarkInstance::New();

		uint32_t dwMarkID = CGuildMarkManager::Instance().GetMarkID(dwGuildID);

		if (dwMarkID != CGuildMarkManager::INVALID_MARK_ID)
		{
			std::string markImagePath;

			if (CGuildMarkManager::Instance().GetMarkImageFilename(dwMarkID / CGuildMarkImage::MARK_TOTAL_COUNT, markImagePath))
			{
				pTextTail->pMarkInstance->SetImageFileName(markImagePath.c_str());
				pTextTail->pMarkInstance->Load();
				pTextTail->pMarkInstance->SetIndex(dwMarkID % CGuildMarkImage::MARK_TOTAL_COUNT);
			}
		}

		std::string strGuildName;
		if (!CPythonGuild::Instance().GetGuildName(dwGuildID, &strGuildName))
			strGuildName = "Noname";

#ifdef ENABLE_SHOW_GUILD_LEADER
		if (dwNewIsGuildName == 3)
			strGuildName.insert(0, "[Leader] - ");
		else if (dwNewIsGuildName == 2)
			strGuildName.insert(0, "[CoLeader] - ");
#endif

		CGraphicTextInstance *& prGuildNameInstance = pTextTail->pGuildNameTextInstance;
		prGuildNameInstance = CGraphicTextInstance::New();
		prGuildNameInstance->SetTextPointer(ms_pFont);
		prGuildNameInstance->SetOutline(true);
		prGuildNameInstance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
		prGuildNameInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
		prGuildNameInstance->SetValue(strGuildName.c_str());
		prGuildNameInstance->SetColor(c_TextTail_Guild_Name_Color.r, c_TextTail_Guild_Name_Color.g, c_TextTail_Guild_Name_Color.b);
		prGuildNameInstance->Update();
	}

#ifdef WJ_SHOW_MOB_INFO
	if (IS_SET(pCharacterInstance->GetAIFlag(), CPythonNonPlayer::AIFLAG_AGGRESSIVE))
	{
		CGraphicTextInstance*& prAIFlagInstance = pTextTail->pAIFlagTextInstance;
		prAIFlagInstance = CGraphicTextInstance::New();
		prAIFlagInstance->SetTextPointer(ms_pFont);
		prAIFlagInstance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_LEFT);
		prAIFlagInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
		prAIFlagInstance->SetValue("*");
		prAIFlagInstance->SetOutline(true);
		prAIFlagInstance->SetColor(c_rColor.r, c_rColor.g, c_rColor.b);
		prAIFlagInstance->Update();

	}
	pTextTail->bIsPC = pCharacterInstance->IsPC() != FALSE;
#endif

	m_CharacterTextTailMap.emplace(dwVirtualID, pTextTail);
}

#ifdef ENABLE_EXTENDED_ITEMNAME_ON_GROUND
void CPythonTextTail::RegisterItemTextTail(uint32_t VirtualID, const char * c_szText, CGraphicObjectInstance * pOwner, bool bHasAttr)
#else
void CPythonTextTail::RegisterItemTextTail(uint32_t VirtualID, const char * c_szText, CGraphicObjectInstance * pOwner)
#endif
{
#ifdef __DEBUG
	char szName[256];
	spritnf(szName, "%s[%d]", c_szText, VirtualID);
#endif

	D3DXCOLOR c_d3dColor = c_TextTail_Item_Color;
#ifdef ENABLE_EXTENDED_ITEMNAME_ON_GROUND
	if (bHasAttr)
		c_d3dColor = c_TextTail_SpecialItem_Color;
#endif

	TTextTail * pTextTail = RegisterTextTail(VirtualID, c_szText, pOwner, c_TextTail_Name_Position, c_d3dColor);
	m_ItemTextTailMap.emplace(VirtualID, pTextTail);
}

void CPythonTextTail::RegisterChatTail(uint32_t VirtualID, const char * c_szChat)
{
	CInstanceBase * pCharacterInstance = CPythonCharacterManager::Instance().GetInstancePtr(VirtualID);

	if (!pCharacterInstance)
		return;

	auto itor = m_ChatTailMap.find(VirtualID);

	if (m_ChatTailMap.end() != itor)
	{
		TTextTail * pTextTail = itor->second;

		pTextTail->pTextInstance->SetValue(c_szChat);
#ifdef WJ_MULTI_TEXTLINE
		pTextTail->pTextInstance->DisableEnterToken();
#endif
		pTextTail->pTextInstance->Update();
		pTextTail->Color = c_TextTail_Chat_Color;
		pTextTail->pTextInstance->SetColor(c_TextTail_Chat_Color);

		// TEXTTAIL_LIVINGTIME_CONTROL
		pTextTail->LivingTime = CTimer::Instance().GetCurrentMillisecond() + TextTail_GetLivingTime();
		// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

		pTextTail->bNameFlag = TRUE;

		return;
	}

	TTextTail * pTextTail = RegisterTextTail(VirtualID, c_szChat, pCharacterInstance->GetGraphicThingInstancePtr(),
											 pCharacterInstance->GetGraphicThingInstanceRef().GetHeight() + 10.0f, c_TextTail_Chat_Color);

	// TEXTTAIL_LIVINGTIME_CONTROL
	pTextTail->LivingTime = CTimer::Instance().GetCurrentMillisecond() + TextTail_GetLivingTime();
	// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

	pTextTail->bNameFlag = TRUE;
	pTextTail->pTextInstance->SetOutline(true);
	pTextTail->pTextInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	m_ChatTailMap.emplace(VirtualID, pTextTail);
}

void CPythonTextTail::RegisterInfoTail(uint32_t VirtualID, const char * c_szChat)
{
	CInstanceBase * pCharacterInstance = CPythonCharacterManager::Instance().GetInstancePtr(VirtualID);

	if (!pCharacterInstance)
		return;

	auto itor = m_ChatTailMap.find(VirtualID);

	if (m_ChatTailMap.end() != itor)
	{
		TTextTail * pTextTail = itor->second;

		pTextTail->pTextInstance->SetValue(c_szChat);
#ifdef WJ_MULTI_TEXTLINE
		pTextTail->pTextInstance->DisableEnterToken();
#endif
		pTextTail->pTextInstance->Update();
		pTextTail->Color = c_TextTail_Info_Color;
		pTextTail->pTextInstance->SetColor(c_TextTail_Info_Color);

		// TEXTTAIL_LIVINGTIME_CONTROL
		pTextTail->LivingTime = CTimer::Instance().GetCurrentMillisecond() + TextTail_GetLivingTime();
		// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

		pTextTail->bNameFlag = FALSE;

		return;
	}

	TTextTail * pTextTail = RegisterTextTail(VirtualID, c_szChat, pCharacterInstance->GetGraphicThingInstancePtr(),
											 pCharacterInstance->GetGraphicThingInstanceRef().GetHeight() + 10.0f, c_TextTail_Info_Color);

	// TEXTTAIL_LIVINGTIME_CONTROL
	pTextTail->LivingTime = CTimer::Instance().GetCurrentMillisecond() + TextTail_GetLivingTime();
	// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

	pTextTail->bNameFlag = FALSE;
	pTextTail->pTextInstance->SetOutline(true);
	pTextTail->pTextInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	m_ChatTailMap.emplace(VirtualID, pTextTail);
}

bool CPythonTextTail::GetTextTailPosition(uint32_t dwVID, float * px, float * py, float * pz)
{
	auto itorCharacter = m_CharacterTextTailMap.find(dwVID);

	if (m_CharacterTextTailMap.end() == itorCharacter)
		return false;

	TTextTail * pTextTail = itorCharacter->second;
	*px = pTextTail->x;
	*py = pTextTail->y;
	*pz = pTextTail->z;

	return true;
}

bool CPythonTextTail::IsChatTextTail(uint32_t dwVID)
{
	auto itorChat = m_ChatTailMap.find(dwVID);

	if (m_ChatTailMap.end() == itorChat)
		return false;

	return true;
}

void CPythonTextTail::SetCharacterTextTailColor(uint32_t VirtualID, const D3DXCOLOR & c_rColor)
{
	auto itorCharacter = m_CharacterTextTailMap.find(VirtualID);

	if (m_CharacterTextTailMap.end() == itorCharacter)
		return;

	TTextTail * pTextTail = itorCharacter->second;
	pTextTail->pTextInstance->SetColor(c_rColor);
	pTextTail->Color = c_rColor;
}

void CPythonTextTail::SetItemTextTailOwner(uint32_t dwVID, const char * c_szName)
{
	auto itor = m_ItemTextTailMap.find(dwVID);
	if (m_ItemTextTailMap.end() == itor)
		return;

	TTextTail * pTextTail = itor->second;

	if (strlen(c_szName) > 0)
	{
		if (!pTextTail->pOwnerTextInstance)
			pTextTail->pOwnerTextInstance = CGraphicTextInstance::New();

		std::string strName = c_szName;
		static const std::string & strOwnership = ApplicationStringTable_GetString(IDS_POSSESSIVE_MORPHENE).empty()
			? "'s"
			: ApplicationStringTable_GetString(IDS_POSSESSIVE_MORPHENE);
		strName += strOwnership;


		pTextTail->pOwnerTextInstance->SetTextPointer(ms_pFont);
		pTextTail->pOwnerTextInstance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
		pTextTail->pOwnerTextInstance->SetValue(strName.c_str());
#ifdef ENABLE_EXTENDED_ITEMNAME_ON_GROUND
		CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
		if (pInstance)
		{
			if (!strcmp(pInstance->GetNameString(), c_szName))
				pTextTail->pOwnerTextInstance->SetColor(1.0f, 1.0f, 0.0f);
			else
				pTextTail->pOwnerTextInstance->SetColor(1.0f, 0.0f, 0.0f);
		}
#else
		pTextTail->pOwnerTextInstance->SetColor(1.0f, 1.0f, 0.0f);
#endif
		pTextTail->pOwnerTextInstance->Update();

		int xOwnerSize, yOwnerSize;
		pTextTail->pOwnerTextInstance->GetTextSize(&xOwnerSize, &yOwnerSize);
		pTextTail->yStart = -2.0f;
		pTextTail->yEnd += float(yOwnerSize + 4);
		pTextTail->xStart = fMIN(pTextTail->xStart, float(-xOwnerSize / 2 - 1));
		pTextTail->xEnd = fMAX(pTextTail->xEnd, float(xOwnerSize / 2 + 1));
	}
	else
	{
		if (pTextTail->pOwnerTextInstance)
		{
			CGraphicTextInstance::Delete(pTextTail->pOwnerTextInstance);
			pTextTail->pOwnerTextInstance = nullptr;
		}

		int xSize, ySize;
		pTextTail->pTextInstance->GetTextSize(&xSize, &ySize);
		pTextTail->xStart = static_cast<float>(-xSize / 2 - 2);
		pTextTail->yStart = -2.0f;
		pTextTail->xEnd = static_cast<float>(xSize / 2 + 2);
		pTextTail->yEnd = static_cast<float>(ySize);
	}
}

void CPythonTextTail::DeleteCharacterTextTail(uint32_t VirtualID)
{
	auto itorCharacter = m_CharacterTextTailMap.find(VirtualID);
	auto itorChat = m_ChatTailMap.find(VirtualID);

	if (m_CharacterTextTailMap.end() != itorCharacter)
	{
		DeleteTextTail(itorCharacter->second);
		m_CharacterTextTailMap.erase(itorCharacter);
	}
	else
		Tracenf("CPythonTextTail::DeleteCharacterTextTail - Find VID[%d] Error", VirtualID);

	if (m_ChatTailMap.end() != itorChat)
	{
		DeleteTextTail(itorChat->second);
		m_ChatTailMap.erase(itorChat);
	}
}

void CPythonTextTail::DeleteItemTextTail(uint32_t VirtualID)
{
	auto itor = m_ItemTextTailMap.find(VirtualID);

	if (m_ItemTextTailMap.end() == itor)
	{
		Tracef(" CPythonTextTail::DeleteItemTextTail - None Item Text Tail\n");
		return;
	}

	DeleteTextTail(itor->second);
	m_ItemTextTailMap.erase(itor);
}

CPythonTextTail::TTextTail * CPythonTextTail::RegisterTextTail(uint32_t dwVirtualID, const char * c_szText, CGraphicObjectInstance * pOwner,
															   float fHeight, const D3DXCOLOR & c_rColor)
{
	TTextTail * pTextTail = m_TextTailPool.Alloc();

	pTextTail->dwVirtualID = dwVirtualID;
	pTextTail->pOwner = pOwner;
	pTextTail->pTextInstance = CGraphicTextInstance::New();
	pTextTail->pOwnerTextInstance = nullptr;
	pTextTail->fHeight = fHeight;

	pTextTail->pTextInstance->SetTextPointer(ms_pFont);
	pTextTail->pTextInstance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
	pTextTail->pTextInstance->SetValue(c_szText);
#ifdef WJ_MULTI_TEXTLINE
	pTextTail->pTextInstance->DisableEnterToken();
#endif
	pTextTail->pTextInstance->SetColor(c_rColor.r, c_rColor.g, c_rColor.b);
	pTextTail->pTextInstance->Update();

	int xSize, ySize;
	pTextTail->pTextInstance->GetTextSize(&xSize, &ySize);
	pTextTail->xStart = static_cast<float>(-xSize / 2 - 2);
	pTextTail->yStart = -2.0f;
	pTextTail->xEnd = static_cast<float>(xSize / 2 + 2);
	pTextTail->yEnd = static_cast<float>(ySize);
	pTextTail->Color = c_rColor;
	pTextTail->fDistanceFromPlayer = 0.0f;
	pTextTail->x = -100.0f;
	pTextTail->y = -100.0f;
	pTextTail->z = 0.0f;
	pTextTail->pMarkInstance = nullptr;
	pTextTail->pGuildNameTextInstance = nullptr;
	pTextTail->pTitleTextInstance = nullptr;
	pTextTail->pLevelTextInstance = nullptr;
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	pTextTail->pLanguageImageInstance = nullptr;
#endif
#ifdef WJ_SHOW_MOB_INFO
	pTextTail->pAIFlagTextInstance = nullptr;
#endif
#ifdef ENABLE_BATTLE_FIELD
	pTextTail->pRankingInstance = nullptr;
#endif
	return pTextTail;
}

void CPythonTextTail::DeleteTextTail(TTextTail * pTextTail)
{
	if (pTextTail->pTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pTextInstance);
		pTextTail->pTextInstance = nullptr;
	}
	if (pTextTail->pOwnerTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pOwnerTextInstance);
		pTextTail->pOwnerTextInstance = nullptr;
	}
	if (pTextTail->pMarkInstance)
	{
		CGraphicMarkInstance::Delete(pTextTail->pMarkInstance);
		pTextTail->pMarkInstance = nullptr;
	}
	if (pTextTail->pGuildNameTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pGuildNameTextInstance);
		pTextTail->pGuildNameTextInstance = nullptr;
	}
	if (pTextTail->pTitleTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pTitleTextInstance);
		pTextTail->pTitleTextInstance = nullptr;
	}
	if (pTextTail->pLevelTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pLevelTextInstance);
		pTextTail->pLevelTextInstance = nullptr;
	}
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	if (pTextTail->pLanguageImageInstance)
	{
		CGraphicImageInstance::Delete(pTextTail->pLanguageImageInstance);
		pTextTail->pLanguageImageInstance = nullptr;
	}
#endif
#ifdef WJ_SHOW_MOB_INFO
	if (pTextTail->pAIFlagTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pAIFlagTextInstance);
		pTextTail->pAIFlagTextInstance = nullptr;
	}
#endif
#ifdef ENABLE_BATTLE_FIELD
	if (pTextTail->pRankingInstance)
	{
		CGraphicImageInstance::Delete(pTextTail->pRankingInstance);
		pTextTail->pRankingInstance = nullptr;
	}
#endif

	m_TextTailPool.Free(pTextTail);
}

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
void CPythonTextTail::AttachLanguageImage(uint32_t dwVID, uint8_t bLanguage)
{
	auto it = m_CharacterTextTailMap.find(dwVID);
	if (it == m_CharacterTextTailMap.end())
		return;

	TTextTail* pTextTail = it->second;

	CGraphicImage* pImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(GetLanguageIconPath(bLanguage).c_str());
	if (pImage)
	{
		CGraphicImageInstance*& pLanguageImage = pTextTail->pLanguageImageInstance;
		if (!pLanguageImage)
			pLanguageImage = CGraphicImageInstance::New();

		pLanguageImage->SetImagePointer(pImage);
		pLanguageImage->SetScale(0.73f, 0.73f);
	}
	else
	{
		if (pTextTail->pLanguageImageInstance)
			CGraphicImageInstance::Delete(pTextTail->pLanguageImageInstance);
		pTextTail->pLanguageImageInstance = nullptr;

		TraceError("Could not find language icon for language '%s'", GetLanguageLocale(bLanguage));
	}
}
#endif

int CPythonTextTail::Pick(int ixMouse, int iyMouse)
{
#ifdef ENABLE_GRAPHIC_ON_OFF
	if (!CPythonGraphicOnOff::Instance().IsItemDropOnOffLevel(CPythonGraphicOnOff::ITEM_DROP_ONOFF_LEVEL_NONE))
		return -1;
#endif

	for (auto & itor : m_ItemTextTailMap)
	{
		TTextTail * pTextTail = itor.second;

		if (ixMouse >= pTextTail->x + pTextTail->xStart && ixMouse <= pTextTail->x + pTextTail->xEnd &&
			iyMouse >= pTextTail->y + pTextTail->yStart && iyMouse <= pTextTail->y + pTextTail->yEnd)
		{
			SelectItemName(itor.first);
			return (itor.first);
		}
	}

	return -1;
}

void CPythonTextTail::SelectItemName(uint32_t dwVirtualID)
{
	auto itor = m_ItemTextTailMap.find(dwVirtualID);

	if (m_ItemTextTailMap.end() == itor)
		return;

	TTextTail * pTextTail = itor->second;
	pTextTail->pTextInstance->SetColor(0.1f, 0.9f, 0.1f);
}

void CPythonTextTail::AttachTitle(uint32_t dwVID, const char * c_szName, const D3DXCOLOR & c_rColor)
{
	if (!bPKTitleEnable)
		return;

	auto itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail * pTextTail = itor->second;

	CGraphicTextInstance *& prTitle = pTextTail->pTitleTextInstance;
	if (!prTitle)
	{
		prTitle = CGraphicTextInstance::New();
		prTitle->SetTextPointer(ms_pFont);
		prTitle->SetOutline(true);
		prTitle->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_RIGHT);
		prTitle->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	}

	prTitle->SetValue(c_szName);
	prTitle->SetColor(c_rColor.r, c_rColor.g, c_rColor.b);
	prTitle->Update();
}

void CPythonTextTail::DetachTitle(uint32_t dwVID)
{
	if (!bPKTitleEnable)
		return;

	auto itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail * pTextTail = itor->second;

	if (pTextTail->pTitleTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pTitleTextInstance);
		pTextTail->pTitleTextInstance = nullptr;
	}
}

void CPythonTextTail::EnablePKTitle(BOOL bFlag) const
{
	bPKTitleEnable = bFlag;
}

void CPythonTextTail::AttachLevel(uint32_t dwVID, const char * c_szText, const D3DXCOLOR & c_rColor, const D3DXCOLOR & Outline)
{
	if (!bPKTitleEnable)
		return;

	auto itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail * pTextTail = itor->second;

	CGraphicTextInstance *& prLevel = pTextTail->pLevelTextInstance;
	if (!prLevel)
	{
		prLevel = CGraphicTextInstance::New();
		prLevel->SetTextPointer(ms_pFont);
		prLevel->SetOutline(true);

		prLevel->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_RIGHT);
		prLevel->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	}

	prLevel->SetValue(c_szText);
	prLevel->SetColor(c_rColor.r, c_rColor.g, c_rColor.b);
	prLevel->SetOutLineColor(Outline.r, Outline.g, Outline.b);	//@custom014
	prLevel->Update();
}

void CPythonTextTail::DetachLevel(uint32_t dwVID)
{
	if (!bPKTitleEnable)
		return;

	auto itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail * pTextTail = itor->second;

	if (pTextTail->pLevelTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pLevelTextInstance);
		pTextTail->pLevelTextInstance = nullptr;
	}
}


void CPythonTextTail::Initialize() const
{
	// DEFAULT_FONT
	auto * pkDefaultFont = msl::inherit_cast<CGraphicText *>(DefaultFont_GetResource());
	if (!pkDefaultFont)
	{
		TraceError("CPythonTextTail::Initialize - CANNOT_FIND_DEFAULT_FONT");
		return;
	}

	ms_pFont = pkDefaultFont;
	// END_OF_DEFAULT_FONT
}

void CPythonTextTail::Destroy()
{
	m_TextTailPool.Clear();
}

void CPythonTextTail::Clear()
{
	m_CharacterTextTailMap.clear();
	m_CharacterTextTailList.clear();
	m_ItemTextTailMap.clear();
	m_ItemTextTailList.clear();
	m_ChatTailMap.clear();

	m_TextTailPool.Clear();
}

CPythonTextTail::CPythonTextTail()
{
	Clear();
}

CPythonTextTail::~CPythonTextTail()
{
	Destroy();
}
