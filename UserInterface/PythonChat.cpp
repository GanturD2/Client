#include "StdAfx.h"
#include "PythonChat.h"

#include "AbstractApplication.h"
#include "PythonCharacterManager.h"
#include "../EterBase/Timer.h"
#ifdef ENABLE_DICE_SYSTEM
#	include "PythonSystem.h"
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
#	include "PythonSystem.h"
#	include "PythonLocale.h"
#endif

int CPythonChat::TChatSet::ms_iChatModeSize = CHAT_TYPE_MAX_NUM;

const float c_fStartDisappearingTime = 5.0f;
const int c_iMaxLineCount = 5;

///////////////////////////////////////////////////////////////////////////////////////////////////

CDynamicPool<CPythonChat::SChatLine> CPythonChat::SChatLine::ms_kPool;

void CPythonChat::SetChatColor(uint32_t eType, uint32_t r, uint32_t g, uint32_t b)
{
	if (eType >= CHAT_TYPE_MAX_NUM)
		return;

	uint32_t dwColor = (0xff000000) | (r << 16) | (g << 8) | (b);
	m_akD3DXClrChat[eType] = D3DXCOLOR(dwColor);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

CPythonChat::SChatLine * CPythonChat::SChatLine::New()
{
	return ms_kPool.Alloc();
}

void CPythonChat::SChatLine::Delete(SChatLine * pkChatLine)
{
	pkChatLine->Instance.Destroy();
	ms_kPool.Free(pkChatLine);
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	if (pkChatLine->pLanguageImageInstance)
		CGraphicImageInstance::Delete(pkChatLine->pLanguageImageInstance);
#endif
#ifdef ENABLE_EMPIRE_FLAG
	if (pkChatLine->pEmpireImageInstance)
		CGraphicImageInstance::Delete(pkChatLine->pEmpireImageInstance);
#endif
}

void CPythonChat::SChatLine::DestroySystem()
{
	ms_kPool.Destroy();
}

void CPythonChat::SChatLine::SetColor(uint32_t dwID, uint32_t dwColor)
{
	assert(dwID < CHAT_LINE_COLOR_ARRAY_MAX_NUM);

	if (dwID >= CHAT_LINE_COLOR_ARRAY_MAX_NUM)
		return;

	aColor[dwID] = dwColor;
}

void CPythonChat::SChatLine::SetColorAll(uint32_t dwColor)
{
	for (auto & i : aColor)
		i = dwColor;
}

D3DXCOLOR & CPythonChat::SChatLine::GetColorRef(uint32_t dwID)
{
#ifdef ENABLE_CHAT_SETTINGS
	if (dwID >= CHAT_LINE_COLOR_ARRAY_MAX_NUM)
		return aColor[0];
#else
	assert(dwID < CHAT_LINE_COLOR_ARRAY_MAX_NUM);

	if (dwID >= CHAT_LINE_COLOR_ARRAY_MAX_NUM)
	{
		static D3DXCOLOR color(1.0f, 0.0f, 0.0f, 1.0f);
		return color;
	}
#endif

	return aColor[dwID];
}

CPythonChat::SChatLine::SChatLine()
{
	for (auto & i : aColor)
		i = 0xff0000ff;

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	pLanguageImageInstance = nullptr;
#endif
#ifdef ENABLE_EMPIRE_FLAG
	pEmpireImageInstance = nullptr;
#endif
}
CPythonChat::SChatLine::~SChatLine()
{
	Instance.Destroy();

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	if (pLanguageImageInstance)
		CGraphicImageInstance::Delete(pLanguageImageInstance);
#endif
#ifdef ENABLE_EMPIRE_FLAG
	if (pEmpireImageInstance)
		CGraphicImageInstance::Delete(pEmpireImageInstance);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int CPythonChat::CreateChatSet(uint32_t dwID)
{
#ifdef ENABLE_CHAT_SETTINGS
	auto it = m_ChatLineNewDeque.find(dwID);
	if (it != m_ChatLineNewDeque.end())
		return dwID;


	TChatSet pChatSet;
	TChatLineDeque map_vec;
	map_vec.clear();

	auto itChat = m_ChatLineNewDeque.find(0);

	CGraphicText* pkDefaultFont = static_cast<CGraphicText*>(DefaultFont_GetResource());

	if (itChat != m_ChatLineNewDeque.end())
	{
		if (itChat->second.size())
		{
			for (auto chat = itChat->second.begin(); chat != itChat->second.end(); ++chat)
			{
				SChatLine* chatLine = *(chat);
				SChatLine* pChatLine = SChatLine::New();

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
				pChatLine->countryIndex = chatLine->countryIndex;
				if (chatLine->countryIndex)
				{
					CGraphicImage* pImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(GetLanguageIconPath(chatLine->countryIndex).c_str());
					if (pImage)
					{
						CGraphicImageInstance*& pLanguageImage = pChatLine->pLanguageImageInstance;
						if (!pLanguageImage)
							pLanguageImage = CGraphicImageInstance::New();

						pLanguageImage->SetImagePointer(pImage);
						pLanguageImage->SetScale(0.73f, 0.73f);
					}
				}
#endif

#ifdef ENABLE_EMPIRE_FLAG
				pChatLine->empireIndex = chatLine->empireIndex;
				if (chatLine->empireIndex)
				{
					CGraphicImage* pImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(GetEmpireIconPath(chatLine->empireIndex).c_str());
					if (pImage)
					{
						CGraphicImageInstance*& pEmpireImage = pChatLine->pEmpireImageInstance;
						if (!pEmpireImage)
						{
							pEmpireImage = CGraphicImageInstance::New();
						}

						pEmpireImage->SetImagePointer(pImage);
						pEmpireImage->SetScale(0.8f, 0.8f);
					}
				}
#endif


				pChatLine->iType = chatLine->iType;
				pChatLine->Instance.SetValue(chatLine->text.c_str());
				pChatLine->Instance.SetTextPointer(pkDefaultFont);
#ifdef WJ_MULTI_TEXTLINE
				pChatLine->Instance.DisableEnterToken();
#endif
				pChatLine->fAppendedTime = chatLine->fAppendedTime;
				pChatLine->SetColorAll(GetChatColor(pChatLine->iType));
				map_vec.push_back(pChatLine);
			}
		}
	}

	m_ChatSetMap.emplace(dwID, pChatSet);
	m_ChatLineNewDeque.emplace(dwID, map_vec);
	ArrangeShowingChat(dwID);
#else
	m_ChatSetMap.emplace(dwID, TChatSet());
#endif
	return dwID;
}

void CPythonChat::UpdateViewMode(uint32_t dwID)
{
	IAbstractApplication & rApp = IAbstractApplication::GetSingleton();

	float fcurTime = rApp.GetGlobalTime();
	//float felapsedTime = rApp.GetGlobalElapsedTime();

	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	TChatLineList * pLineList = &(pChatSet->m_ShowingChatLineList);
	int iLineIndex = pLineList->size();
	int iHeight = -(int(pLineList->size() + 1) * pChatSet->m_iStep);

	for (auto itor = pLineList->begin(); itor != pLineList->end();)
	{
		TChatLine * pChatLine = (*itor);

		D3DXCOLOR & rColor = pChatLine->GetColorRef(dwID);

		float fElapsedTime = (fcurTime - pChatLine->fAppendedTime);
		if (fElapsedTime >= c_fStartDisappearingTime || iLineIndex >= c_iMaxLineCount)
		{
			rColor.a -= rColor.a / 10.0f;

			if (rColor.a <= 0.1f)
				itor = pLineList->erase(itor);
			else
				++itor;
		}
		else
			++itor;

		/////

		iHeight += pChatSet->m_iStep;
		--iLineIndex;

#if defined(ENABLE_MULTI_LANGUAGE_SYSTEM) || defined(ENABLE_EMPIRE_FLAG)
		int iWidth = pChatSet->m_ix;
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		if (pChatLine->pLanguageImageInstance && CPythonSystem::Instance().IsShowChatLanguage())
		{
			if (GetDefaultCodePage() == CP_ARABIC)
			{
				pChatLine->pLanguageImageInstance->SetPosition(iWidth - 18, pChatSet->m_iy + iHeight + 2);
				iWidth -= pChatLine->pLanguageImageInstance->GetWidth();
			}
			else
			{
				pChatLine->pLanguageImageInstance->SetPosition(iWidth, pChatSet->m_iy + iHeight + 2);
				iWidth += pChatLine->pLanguageImageInstance->GetWidth();
			}
		}
#endif

#ifdef ENABLE_EMPIRE_FLAG
		if (pChatLine->pEmpireImageInstance)
		{
			if (GetDefaultCodePage() == CP_ARABIC)
			{
				pChatLine->pEmpireImageInstance->SetPosition(iWidth - 12, pChatSet->m_iy + iHeight + 4);
				iWidth -= pChatLine->pEmpireImageInstance->GetWidth();
			}
			else
			{
				pChatLine->pEmpireImageInstance->SetPosition(iWidth, pChatSet->m_iy + iHeight + 4);
				iWidth += pChatLine->pEmpireImageInstance->GetWidth();
			}
		}
#endif

#if defined(ENABLE_MULTI_LANGUAGE_SYSTEM) || defined(ENABLE_EMPIRE_FLAG)
		pChatLine->Instance.SetPosition(iWidth, pChatSet->m_iy + iHeight);
#else
		pChatLine->Instance.SetPosition(pChatSet->m_ix, pChatSet->m_iy + iHeight);
#endif
		pChatLine->Instance.SetColor(rColor);
		pChatLine->Instance.Update();
	}
}

void CPythonChat::UpdateEditMode(uint32_t dwID)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	const int c_iAlphaLine = std::max(0, GetVisibleLineCount(dwID) - GetEditableLineCount(dwID) + 2);

	int iLineIndex = 0;
	float fAlpha = 0.0f;
	float fAlphaStep = 0.0f;

	if (c_iAlphaLine > 0)
		fAlphaStep = 1.0f / float(c_iAlphaLine);

	TChatLineList * pLineList = &(pChatSet->m_ShowingChatLineList);
	int iHeight = -(int(pLineList->size() + 1) * pChatSet->m_iStep);

	for (auto & pChatLine : *pLineList)
	{
		D3DXCOLOR & rColor = pChatLine->GetColorRef(dwID);

		if (iLineIndex < c_iAlphaLine)
		{
			rColor.a += (fAlpha - rColor.a) / 10.0f;
			fAlpha = fMIN(fAlpha + fAlphaStep, 1.0f);
		}
		else
			rColor.a = fMIN(rColor.a + 0.05f, 1.0f);

		iHeight += pChatSet->m_iStep;

#if defined(ENABLE_MULTI_LANGUAGE_SYSTEM) || defined(ENABLE_EMPIRE_FLAG)
		int iWidth = pChatSet->m_ix;
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		if (pChatLine->pLanguageImageInstance && CPythonSystem::Instance().IsShowChatLanguage())
		{
			if (GetDefaultCodePage() == CP_ARABIC)
			{
				pChatLine->pLanguageImageInstance->SetPosition(iWidth - 18, pChatSet->m_iy + iHeight + 2);
				iWidth -= pChatLine->pLanguageImageInstance->GetWidth();
			}
			else
			{
				pChatLine->pLanguageImageInstance->SetPosition(iWidth, pChatSet->m_iy + iHeight + 2);
				iWidth += pChatLine->pLanguageImageInstance->GetWidth();
			}
		}
#endif

#ifdef ENABLE_EMPIRE_FLAG
		if (pChatLine->pEmpireImageInstance)
		{
			if (GetDefaultCodePage() == CP_ARABIC)
			{
				pChatLine->pEmpireImageInstance->SetPosition(iWidth - 12, pChatSet->m_iy + iHeight + 4);
				iWidth -= pChatLine->pEmpireImageInstance->GetWidth();
			}
			else
			{
				pChatLine->pEmpireImageInstance->SetPosition(iWidth, pChatSet->m_iy + iHeight + 4);
				iWidth += pChatLine->pEmpireImageInstance->GetWidth();
			}
		}
#endif

#if defined(ENABLE_MULTI_LANGUAGE_SYSTEM) || defined(ENABLE_EMPIRE_FLAG)
		pChatLine->Instance.SetPosition(iWidth, pChatSet->m_iy + iHeight);
#else
		pChatLine->Instance.SetPosition(pChatSet->m_ix, pChatSet->m_iy + iHeight);
#endif
		pChatLine->Instance.SetColor(rColor);
		pChatLine->Instance.Update();
	}
}

void CPythonChat::UpdateLogMode(uint32_t dwID)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	TChatLineList * pLineList = &(pChatSet->m_ShowingChatLineList);
	int iHeight = 0;

	for (auto itor = pLineList->rbegin(); itor != pLineList->rend(); ++itor)
	{
		TChatLine * pChatLine = (*itor);

		iHeight -= pChatSet->m_iStep;

#if defined(ENABLE_MULTI_LANGUAGE_SYSTEM) || defined(ENABLE_EMPIRE_FLAG)
		int iWidth = pChatSet->m_ix;
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		if (pChatLine->pLanguageImageInstance && CPythonSystem::Instance().IsShowChatLanguage())
		{
			if (GetDefaultCodePage() == CP_ARABIC)
			{
				pChatLine->pLanguageImageInstance->SetPosition(iWidth - 18, pChatSet->m_iy + iHeight + 2);
				iWidth -= pChatLine->pLanguageImageInstance->GetWidth();
			}
			else
			{
				pChatLine->pLanguageImageInstance->SetPosition(iWidth, pChatSet->m_iy + iHeight + 2);
				iWidth += pChatLine->pLanguageImageInstance->GetWidth();
			}
		}
#endif

#ifdef ENABLE_EMPIRE_FLAG
		if (pChatLine->pEmpireImageInstance)
		{
			if (GetDefaultCodePage() == CP_ARABIC)
			{
				pChatLine->pEmpireImageInstance->SetPosition(iWidth - 12, pChatSet->m_iy + iHeight + 4);
				iWidth -= pChatLine->pEmpireImageInstance->GetWidth();
			}
			else
			{
				pChatLine->pEmpireImageInstance->SetPosition(iWidth, pChatSet->m_iy + iHeight + 4);
				iWidth += pChatLine->pEmpireImageInstance->GetWidth();
			}
		}
#endif

#if defined(ENABLE_MULTI_LANGUAGE_SYSTEM) || defined(ENABLE_EMPIRE_FLAG)
		pChatLine->Instance.SetPosition(iWidth, pChatSet->m_iy + iHeight);
#else
		pChatLine->Instance.SetPosition(pChatSet->m_ix, pChatSet->m_iy + iHeight);
#endif
		pChatLine->Instance.SetColor(pChatLine->GetColorRef(dwID));
		pChatLine->Instance.Update();
	}
}

void CPythonChat::Update(uint32_t dwID)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	switch (pChatSet->m_iBoardState)
	{
		case BOARD_STATE_VIEW:
			UpdateViewMode(dwID);
			break;
		case BOARD_STATE_EDIT:
			UpdateEditMode(dwID);
			break;
		case BOARD_STATE_LOG:
			UpdateLogMode(dwID);
			break;
	}

	uint32_t dwcurTime = CTimer::Instance().GetCurrentMillisecond();
	for (auto itor = m_WaitChatList.begin(); itor != m_WaitChatList.end();)
	{
		TWaitChat & rWaitChat = *itor;

		if (rWaitChat.dwAppendingTime < dwcurTime)
		{
			AppendChat(rWaitChat.iType, rWaitChat.strChat.c_str());

			itor = m_WaitChatList.erase(itor);
		}
		else
			++itor;
	}
}

void CPythonChat::Render(uint32_t dwID)
{
	TChatLineList * pLineList = GetChatLineListPtr(dwID);
	if (!pLineList)
		return;

	for (auto & itor : *pLineList)
	{
		CGraphicTextInstance & rInstance = itor->Instance;
		rInstance.Render();
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		CGraphicImageInstance* pLanguageImageInstance = itor->pLanguageImageInstance;
		if (pLanguageImageInstance && CPythonSystem::Instance().IsShowChatLanguage())
			pLanguageImageInstance->Render();
#endif
#ifdef ENABLE_EMPIRE_FLAG
		CGraphicImageInstance* pEmpireImageInstance = itor->pEmpireImageInstance;
		if (pEmpireImageInstance)
			pEmpireImageInstance->Render();
#endif
	}
}

void CPythonChat::SetBoardState(uint32_t dwID, int iState)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_iBoardState = iState;
	ArrangeShowingChat(dwID);
}

void CPythonChat::SetPosition(uint32_t dwID, int ix, int iy)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_ix = ix;
	pChatSet->m_iy = iy;
}
void CPythonChat::SetHeight(uint32_t dwID, int iHeight)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_iHeight = iHeight;
}
void CPythonChat::SetStep(uint32_t dwID, int iStep)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_iStep = iStep;
}
void CPythonChat::ToggleChatMode(uint32_t dwID, int iMode)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_iMode[iMode] = 1 - pChatSet->m_iMode[iMode];
	// 	Tracef("ToggleChatMode : %d\n", iMode);
	ArrangeShowingChat(dwID);
}
void CPythonChat::EnableChatMode(uint32_t dwID, int iMode)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_iMode[iMode] = TRUE;
	// 	Tracef("EnableChatMode : %d\n", iMode);
	ArrangeShowingChat(dwID);
}
void CPythonChat::DisableChatMode(uint32_t dwID, int iMode)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_iMode[iMode] = FALSE;
	// 	Tracef("DisableChatMode : %d\n", iMode);
	ArrangeShowingChat(dwID);
}
void CPythonChat::SetEndPos(uint32_t dwID, float fPos)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	fPos = std::max(0.0f, fPos);
	fPos = std::min(1.0f, fPos);
	if (pChatSet->m_fEndPos != fPos)
	{
		pChatSet->m_fEndPos = fPos;
		ArrangeShowingChat(dwID);
	}
}


int CPythonChat::GetVisibleLineCount(uint32_t dwID)
{
	TChatLineList * pLineList = GetChatLineListPtr(dwID);
	if (!pLineList)
		return 0;

	return pLineList->size();
}

int CPythonChat::GetEditableLineCount(uint32_t dwID)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return 0;

	return pChatSet->m_iHeight / pChatSet->m_iStep + 1;
}

int CPythonChat::GetLineCount(uint32_t dwID)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return 0;

#ifdef ENABLE_CHAT_SETTINGS
	auto it = m_ChatLineNewDeque.find(dwID);
	if (it == m_ChatLineNewDeque.end())
		return 0;
#endif

	int iCount = 0;
#ifdef ENABLE_CHAT_SETTINGS
	for (uint32_t i = 0; i < it->second.size(); ++i)
	{
		if (!pChatSet->CheckMode(it->second[i]->iType))
			continue;

#	ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		if (!pChatSet->CheckCountryMode(it->second[i]->countryIndex) && it->second[i]->iType == CHAT_TYPE_SHOUT)
			continue;
#	endif

		++iCount;
	}
#else
	for (auto & i : m_ChatLineDeque)
	{
		if (!pChatSet->CheckMode(i->iType))
			continue;

		++iCount;
	}
#endif

	return iCount;
}

int CPythonChat::GetLineStep(uint32_t dwID)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return 0;

	return pChatSet->m_iStep;
}

CPythonChat::TChatLineList * CPythonChat::GetChatLineListPtr(uint32_t dwID)
{
	auto itor = m_ChatSetMap.find(dwID);
	if (m_ChatSetMap.end() == itor)
		return nullptr;

	TChatSet & rChatSet = itor->second;
	return &(rChatSet.m_ShowingChatLineList);
}

CPythonChat::TChatSet * CPythonChat::GetChatSetPtr(uint32_t dwID)
{
	auto itor = m_ChatSetMap.find(dwID);
	if (m_ChatSetMap.end() == itor)
		return nullptr;

	TChatSet & rChatSet = itor->second;
	return &rChatSet;
}

void CPythonChat::ArrangeShowingChat(uint32_t dwID)
{
	TChatSet * pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_ShowingChatLineList.clear();

#ifdef ENABLE_CHAT_SETTINGS
	auto it = m_ChatLineNewDeque.find(dwID);
	if (it == m_ChatLineNewDeque.end())
		return;
#endif

	TChatLineDeque TempChatLineDeque;
#ifdef ENABLE_CHAT_SETTINGS
	for (auto itor = it->second.begin(); itor != it->second.end(); ++itor)
#else
	for (auto itor = m_ChatLineDeque.begin(); itor != m_ChatLineDeque.end(); ++itor)
#endif
	{
		TChatLine * pChatLine = *itor;
		if (pChatSet->CheckMode(pChatLine->iType))
			TempChatLineDeque.emplace_back(pChatLine);

#if defined (ENABLE_CHAT_SETTINGS) && defined(ENABLE_MULTI_LANGUAGE_SYSTEM)
		if (!pChatSet->CheckCountryMode(pChatLine->countryIndex) && pChatLine->iType == CHAT_TYPE_SHOUT)
			continue;
#endif
	}

	int icurLineCount = TempChatLineDeque.size();
	int iVisibleLineCount = std::min(icurLineCount, (pChatSet->m_iHeight + pChatSet->m_iStep) / pChatSet->m_iStep);
	int iEndLine = iVisibleLineCount + int(float(icurLineCount - iVisibleLineCount - 1) * pChatSet->m_fEndPos);

	/////

	int iHeight = 12;
	for (int i = std::min(icurLineCount - 1, iEndLine); i >= 0; --i)
	{
		if (!pChatSet->CheckMode(TempChatLineDeque[i]->iType))
			continue;

#if defined (ENABLE_CHAT_SETTINGS) && defined(ENABLE_MULTI_LANGUAGE_SYSTEM)
		if (!pChatSet->CheckCountryMode(TempChatLineDeque[i]->countryIndex) && TempChatLineDeque[i]->iType == CHAT_TYPE_SHOUT)
			continue;
#endif

		if (pChatSet->m_iHeight + pChatSet->m_iStep <= iHeight)
			break;

		pChatSet->m_ShowingChatLineList.push_front(TempChatLineDeque[i]);

		iHeight += pChatSet->m_iStep;
	}
}

void CPythonChat::AppendChat(int iType, const char * c_szChat)
{
#ifdef ENABLE_OX_RENEWAL
	if (strstr(c_szChat, "#start") || strstr(c_szChat, "#send") || strstr(c_szChat, "#end"))
		return;
#endif

#ifdef ENABLE_DICE_SYSTEM
	if (CHAT_TYPE_DICE_INFO == iType && !CPythonSystem::Instance().IsDiceChatShow())
		return;
#endif

	// DEFAULT_FONT
	//static CResource * s_pResource = CResourceManager::Instance().GetResourcePointer(g_strDefaultFontName.c_str());

	auto * pkDefaultFont = msl::inherit_cast<CGraphicText *>(DefaultFont_GetResource());
	if (!pkDefaultFont)
	{
		TraceError("CPythonChat::AppendChat - CANNOT_FIND_DEFAULT_FONT");
		return;
	}
	// END_OF_DEFAULT_FONT

	IAbstractApplication & rApp = IAbstractApplication::GetSingleton();

#ifdef ENABLE_CHAT_SETTINGS
	if (m_ChatLineNewDeque.size())
	{
		for (auto it = m_ChatLineNewDeque.begin(); it != m_ChatLineNewDeque.end(); ++it)
		{
			SChatLine* pChatLine = SChatLine::New();

			pChatLine->text = c_szChat;
			pChatLine->iType = iType;
			pChatLine->Instance.SetValue(pChatLine->text.c_str());
#ifdef WJ_MULTI_TEXTLINE
			pChatLine->Instance.DisableEnterToken();
#endif
			pChatLine->Instance.SetTextPointer(pkDefaultFont);
			pChatLine->fAppendedTime = rApp.GetGlobalTime();
			pChatLine->SetColorAll(GetChatColor(iType));
			it->second.push_back(pChatLine);
			if (it->second.size() > CHAT_LINE_MAX_NUM)
			{
				SChatLine* pChatLine = it->second.front();
				SChatLine::Delete(pChatLine);
				it->second.pop_front();
			}

			auto itChatSet = m_ChatSetMap.find(it->first);
			if (itChatSet != m_ChatSetMap.end())
			{
				TChatSet* pChatSet = &(itChatSet->second);

				if (BOARD_STATE_EDIT == pChatSet->m_iBoardState)
					ArrangeShowingChat(itChatSet->first);
				else
				{
					pChatSet->m_ShowingChatLineList.push_back(pChatLine);
					if (pChatSet->m_ShowingChatLineList.size() > CHAT_LINE_MAX_NUM)
						pChatSet->m_ShowingChatLineList.pop_front();
				}
			}
		}
	}
#else
	SChatLine* pChatLine = SChatLine::New();

	pChatLine->iType = iType;
	pChatLine->Instance.SetValue(c_szChat);
#ifdef WJ_MULTI_TEXTLINE
	pChatLine->Instance.DisableEnterToken();
#endif

	// DEFAULT_FONT
	pChatLine->Instance.SetTextPointer(pkDefaultFont);
	// END_OF_DEFAULT_FONT

	pChatLine->fAppendedTime = rApp.GetGlobalTime();
	pChatLine->SetColorAll(GetChatColor(iType));

	m_ChatLineDeque.emplace_back(pChatLine);
	if (m_ChatLineDeque.size() > CHAT_LINE_MAX_NUM)
	{
		SChatLine * pChatLine = m_ChatLineDeque.front();
		SChatLine::Delete(pChatLine);
		m_ChatLineDeque.pop_front();
	}

	for (auto & itor : m_ChatSetMap)
	{
		TChatSet * pChatSet = &(itor.second);
		//pChatLine->SetColor(itor->first, GetChatColor(iType));

		// Edit Mode 를 억지로 끼워 맞추기 위해 추가
		if (BOARD_STATE_EDIT == pChatSet->m_iBoardState)
			ArrangeShowingChat(itor.first);
		else // if (BOARD_STATE_VIEW == pChatSet->m_iBoardState)
		{
			pChatSet->m_ShowingChatLineList.emplace_back(pChatLine);
			if (pChatSet->m_ShowingChatLineList.size() > CHAT_LINE_MAX_NUM)
				pChatSet->m_ShowingChatLineList.pop_front();
		}
	}
#endif
}

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
#include "PythonLocale.h"
#	ifdef ENABLE_EMPIRE_FLAG
void CPythonChat::AppendChat(int iType, uint8_t bLanguage, uint8_t bEmpire, const char* c_szChat)
#	else
void CPythonChat::AppendChat(int iType, uint8_t bLanguage, const char* c_szChat)
#	endif
{
#ifdef ENABLE_OX_RENEWAL
	if (strstr(c_szChat, "#start") || strstr(c_szChat, "#send") || strstr(c_szChat, "#end"))
		return;
#endif

#ifdef ENABLE_DICE_SYSTEM
	if (CHAT_TYPE_DICE_INFO == iType && !CPythonSystem::Instance().IsDiceChatShow())
		return;
#endif

	CGraphicText* pkDefaultFont = static_cast<CGraphicText*>(DefaultFont_GetResource());
	if (!pkDefaultFont)
	{
		TraceError("CPythonChat::AppendChat - CANNOT_FIND_DEFAULT_FONT");
		return;
	}

	IAbstractApplication& rApp = IAbstractApplication::GetSingleton();

#ifdef ENABLE_CHAT_SETTINGS
	if (m_ChatLineNewDeque.size())
	{
		for (auto it = m_ChatLineNewDeque.begin(); it != m_ChatLineNewDeque.end(); ++it)
		{
			SChatLine* pChatLine = SChatLine::New();

			std::string stChatValue(c_szChat);
			CPythonLocale::Instance().HandleItemHyperlink(stChatValue);

			pChatLine->text = stChatValue;
			pChatLine->iType = iType;
			pChatLine->Instance.SetValue(pChatLine->text.c_str());

#ifdef WJ_MULTI_TEXTLINE
			pChatLine->Instance.DisableEnterToken();
#endif
			pChatLine->Instance.SetTextPointer(pkDefaultFont);
			pChatLine->fAppendedTime = rApp.GetGlobalTime();
			pChatLine->SetColorAll(GetChatColor(iType));

			pChatLine->countryIndex = bLanguage;
			if (bLanguage)
			{
				CGraphicImage* pImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(GetLanguageIconPath(bLanguage).c_str());
				if (pImage)
				{
					CGraphicImageInstance*& pLanguageImage = pChatLine->pLanguageImageInstance;
					if (!pLanguageImage)
						pLanguageImage = CGraphicImageInstance::New();

					pLanguageImage->SetImagePointer(pImage);
					pLanguageImage->SetScale(0.73f, 0.73f);
				}
			}

#ifdef ENABLE_EMPIRE_FLAG
			pChatLine->empireIndex = bEmpire;
			if (bEmpire)
			{
				CGraphicImage* pImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(GetEmpireIconPath(bEmpire).c_str());
				if (pImage)
				{
					CGraphicImageInstance*& pEmpireImage = pChatLine->pEmpireImageInstance;
					if (!pEmpireImage)
					{
						pEmpireImage = CGraphicImageInstance::New();
					}

					pEmpireImage->SetImagePointer(pImage);
					pEmpireImage->SetScale(0.8f, 0.8f);
				}
			}
#endif

			it->second.push_back(pChatLine);
			if (it->second.size() > CHAT_LINE_MAX_NUM)
			{
				SChatLine* pChatLine = it->second.front();
				SChatLine::Delete(pChatLine);
				it->second.pop_front();
			}

			auto itor = m_ChatSetMap.find(it->first);
			if (itor != m_ChatSetMap.end())
			{
				TChatSet* pChatSet = &(itor->second);

				if (BOARD_STATE_EDIT == pChatSet->m_iBoardState)
					ArrangeShowingChat(itor->first);
				else
				{
					pChatSet->m_ShowingChatLineList.push_back(pChatLine);
					if (pChatSet->m_ShowingChatLineList.size() > CHAT_LINE_MAX_NUM)
						pChatSet->m_ShowingChatLineList.pop_front();
				}
			}
		}
	}
#else
	SChatLine* pChatLine = SChatLine::New();

	std::string stChatValue(c_szChat);
	PythonLocaleManager::Instance().HandleItemHyperlink(stChatValue);

	pChatLine->iType = iType;
	pChatLine->Instance.SetValue(stChatValue.c_str());
#ifdef WJ_MULTI_TEXTLINE
	pChatLine->Instance.DisableEnterToken();
#endif
	pChatLine->Instance.SetTextPointer(pkDefaultFont);

	pChatLine->fAppendedTime = rApp.GetGlobalTime();
	pChatLine->SetColorAll(GetChatColor(iType));

	if (bLanguage)
	{
		CGraphicImage* pImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(GetLanguageIconPath(bLanguage).c_str());
		if (pImage)
		{
			CGraphicImageInstance*& pLanguageImage = pChatLine->pLanguageImageInstance;
			if (!pLanguageImage)
				pLanguageImage = CGraphicImageInstance::New();

			pLanguageImage->SetImagePointer(pImage);
			pLanguageImage->SetScale(0.73f, 0.73f);
		}
	}

#ifdef ENABLE_EMPIRE_FLAG
	if (bEmpire)
	{
		CGraphicImage* pImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(GetEmpireIconPath(bEmpire).c_str());
		if (pImage)
		{
			CGraphicImageInstance*& pEmpireImage = pChatLine->pEmpireImageInstance;
			if (!pEmpireImage)
			{
				pEmpireImage = CGraphicImageInstance::New();
			}

			pEmpireImage->SetImagePointer(pImage);
			pEmpireImage->SetScale(0.8f, 0.8f);
		}
	}
#endif

	m_ChatLineDeque.emplace_back(pChatLine);
	if (m_ChatLineDeque.size() > CHAT_LINE_MAX_NUM)
	{
		SChatLine* pChatLine = m_ChatLineDeque.front();
		SChatLine::Delete(pChatLine);
		m_ChatLineDeque.pop_front();
	}

	for (auto& itor : m_ChatSetMap)
	{
		TChatSet* pChatSet = &(itor.second);

		if (BOARD_STATE_EDIT == pChatSet->m_iBoardState)
			ArrangeShowingChat(itor.first);
		else
		{
			pChatSet->m_ShowingChatLineList.emplace_back(pChatLine);
			if (pChatSet->m_ShowingChatLineList.size() > CHAT_LINE_MAX_NUM)
				pChatSet->m_ShowingChatLineList.pop_front();
		}
	}
#endif
}
#endif

void CPythonChat::AppendChatWithDelay(int iType, const char * c_szChat, int iDelay)
{
	TWaitChat WaitChat;
	WaitChat.iType = iType;
	WaitChat.strChat = c_szChat;
	WaitChat.dwAppendingTime = CTimer::Instance().GetCurrentMillisecond() + iDelay;
	m_WaitChatList.emplace_back(WaitChat);
}

uint32_t CPythonChat::GetChatColor(int iType) const
{
	if (iType < CHAT_TYPE_MAX_NUM)
		return m_akD3DXClrChat[iType];

	return D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f);
}

void CPythonChat::IgnoreCharacter(const char * c_szName)
{
	auto itor = m_IgnoreCharacterSet.find(c_szName);
	// NOTE : 이미 차단 중이라면..
	if (m_IgnoreCharacterSet.end() != itor)
	{
		m_IgnoreCharacterSet.erase(itor);
		// NOTE : 차단이 되지 않은 캐릭터라면..
	}
	else
		m_IgnoreCharacterSet.emplace(c_szName);
}

BOOL CPythonChat::IsIgnoreCharacter(const char * c_szName)
{
	auto itor = m_IgnoreCharacterSet.find(c_szName);

	if (m_IgnoreCharacterSet.end() == itor)
		return FALSE;

	return TRUE;
}

CWhisper * CPythonChat::CreateWhisper(const char * c_szName)
{
	CWhisper * pWhisper = CWhisper::New();
	m_WhisperMap.emplace(c_szName, pWhisper);
	return pWhisper;
}

void CPythonChat::AppendWhisper(int iType, const char * c_szName, const char * c_szChat)
{
	CWhisper * pWhisper{};
	if (auto itor = m_WhisperMap.find(c_szName); itor != m_WhisperMap.end())
		pWhisper = itor->second;
	else
		pWhisper = CreateWhisper(c_szName);

	pWhisper->AppendChat(iType, c_szChat);
}

void CPythonChat::ClearWhisper(const char * c_szName)
{
	if (auto itor = m_WhisperMap.find(c_szName); itor != m_WhisperMap.end())
	{
		CWhisper * pWhisper = itor->second;
		CWhisper::Delete(pWhisper);

		m_WhisperMap.erase(itor);
	}
}

BOOL CPythonChat::GetWhisper(const char * c_szName, CWhisper ** ppWhisper)
{
	if (auto itor = m_WhisperMap.find(c_szName); itor != m_WhisperMap.end())
	{
		*ppWhisper = itor->second;
		return TRUE;
	}
	return FALSE;
}

void CPythonChat::InitWhisper(PyObject * ppyObject)
{
	for (auto itor = m_WhisperMap.begin(); itor != m_WhisperMap.end(); ++itor)
	{
		std::string strName = itor->first;
		PyCallClassMemberFunc(ppyObject, "MakeWhisperButton", Py_BuildValue("(s)", strName.c_str()));
	}
}

void CPythonChat::__DestroyWhisperMap()
{
	for (auto itor = m_WhisperMap.begin(); itor != m_WhisperMap.end(); ++itor)
		CWhisper::Delete(itor->second);
	m_WhisperMap.clear();
}

void CPythonChat::Close()
{
	for (auto itor = m_ChatSetMap.begin(); itor != m_ChatSetMap.end(); ++itor)
	{
		TChatSet & rChatSet = itor->second;
		TChatLineList * pLineList = &(rChatSet.m_ShowingChatLineList);
		for (auto & pChatLine : *pLineList)
			pChatLine->fAppendedTime = 0.0f;
	}
}

void CPythonChat::Destroy()
{
	__DestroyWhisperMap();

	m_ShowingChatLineList.clear();
	m_ChatSetMap.clear();
	m_ChatLineDeque.clear();
#ifdef ENABLE_CHAT_SETTINGS
	m_ChatLineNewDeque.clear();
#endif

	SChatLine::DestroySystem();
	CWhisper::DestroySystem();

	__Initialize();
}

void CPythonChat::__Initialize()
{
	m_akD3DXClrChat[CHAT_TYPE_TALKING] = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_INFO] = D3DXCOLOR(1.0f, 0.785f, 0.785f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_NOTICE] = D3DXCOLOR(1.0f, 0.902f, 0.730f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_PARTY] = D3DXCOLOR(0.542f, 1.0f, 0.949f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_GUILD] = D3DXCOLOR(0.906f, 0.847f, 1.0f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_COMMAND] = D3DXCOLOR(0.658f, 1.0f, 0.835f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_SHOUT] = D3DXCOLOR(0.658f, 1.0f, 0.835f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_WHISPER] = D3DXCOLOR(0xff4AE14A);
	m_akD3DXClrChat[CHAT_TYPE_BIG_NOTICE] = D3DXCOLOR(1.0f, 0.902f, 0.730f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_MONARCH_NOTICE] = D3DXCOLOR(1.0f, 0.902f, 0.730f, 1.0f);
#ifdef ENABLE_OX_RENEWAL
	m_akD3DXClrChat[CHAT_TYPE_CONTROL_NOTICE] = D3DXCOLOR(1.0f, 0.902f, 0.730f, 1.0f);
#endif
#ifdef ENABLE_DICE_SYSTEM
	m_akD3DXClrChat[CHAT_TYPE_DICE_INFO]	= D3DXCOLOR(0xFFcc00cc);
#endif
#ifdef ENABLE_12ZI
	m_akD3DXClrChat[CHAT_TYPE_MISSION]		= D3DXCOLOR(1.0f, 0.785f, 0.785f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_SUB_MISSION]	= D3DXCOLOR(1.0f, 0.785f, 0.785f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_CLEAR_MISSION] = D3DXCOLOR(1.0f, 0.785f, 0.785f, 1.0f);
#endif
#ifdef ENABLE_CHAT_SETTINGS_EXTEND
	m_akD3DXClrChat[CHAT_TYPE_EXP_INFO] = D3DXCOLOR(1.0f, 0.785f, 0.785f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_ITEM_INFO] = D3DXCOLOR(1.0f, 0.785f, 0.785f, 1.0f);
	m_akD3DXClrChat[CHAT_TYPE_MONEY_INFO] = D3DXCOLOR(1.0f, 0.785f, 0.785f, 1.0f);
#endif
#ifdef ENABLE_CHAT_SETTINGS
	TChatLineDeque map_vec;
	m_ChatLineNewDeque.emplace(0, map_vec);
	m_ChatLineNewDeque.emplace(1, map_vec);
	m_ChatSetMap.emplace(0, TChatSet());//normal chat
	m_ChatSetMap.emplace(1, TChatSet());//log chat
#endif
}

CPythonChat::CPythonChat()
{
	__Initialize();
}

CPythonChat::~CPythonChat()
{
#ifdef ENABLE_CHAT_SETTINGS
	m_ChatSetMap.clear();
#endif
	assert(m_ChatLineDeque.empty());
	assert(m_ShowingChatLineList.empty());
	assert(m_ChatSetMap.empty());
	assert(m_WhisperMap.empty());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CDynamicPool<CWhisper> CWhisper::ms_kPool;

CWhisper * CWhisper::New()
{
	return ms_kPool.Alloc();
}

void CWhisper::Delete(CWhisper * pkWhisper)
{
	pkWhisper->Destroy();
	ms_kPool.Free(pkWhisper);
}

void CWhisper::DestroySystem()
{
	ms_kPool.Destroy();

	SChatLine::DestroySystem();
}

void CWhisper::SetPosition(float fPosition)
{
	m_fcurPosition = fPosition;
	__ArrangeChat();
}

void CWhisper::SetBoxSize(float fWidth, float fHeight)
{
	m_fWidth = fWidth;
	m_fHeight = fHeight;

	for (auto itor = m_ChatLineDeque.begin(); itor != m_ChatLineDeque.end(); ++itor)
	{
		TChatLine * pChatLine = *itor;
		pChatLine->Instance.SetLimitWidth(fWidth);
	}
}

void CWhisper::AppendChat(int iType, const char * c_szChat)
{
	// DEFAULT_FONT
	//static CResource * s_pResource = CResourceManager::Instance().GetResourcePointer(g_strDefaultFontName.c_str());

#if defined(LOCALE_SERVICE_YMIR) || defined(LOCALE_SERVICE_JAPAN) || defined(LOCALE_SERVICE_HONGKONG) || defined(LOCALE_SERVICE_TAIWAN) || \
	defined(LOCALE_SERVICE_NEWCIBN)
	CGraphicText * pkDefaultFont = static_cast<CGraphicText *>(DefaultFont_GetResource());
#else
	CGraphicText * pkDefaultFont = (iType == CPythonChat::WHISPER_TYPE_GM) ? msl::inherit_cast<CGraphicText *>(DefaultFont_GetResource()) : msl::inherit_cast<CGraphicText *>(DefaultFont_GetResource());	//@fixme423
#endif

	if (!pkDefaultFont)
	{
		TraceError("CWhisper::AppendChat - CANNOT_FIND_DEFAULT_FONT");
		return;
	}
	// END_OF_DEFAULT_FONT

	SChatLine * pChatLine = SChatLine::New();
	pChatLine->Instance.SetValue(c_szChat);

	// DEFAULT_FONT
	pChatLine->Instance.SetTextPointer(pkDefaultFont);
	// END_OF_DEFAULT_FONT

	pChatLine->Instance.SetLimitWidth(m_fWidth);
	pChatLine->Instance.SetMultiLine(TRUE);
#ifdef WJ_MULTI_TEXTLINE
	pChatLine->Instance.DisableEnterToken();
#endif

	switch (iType)
	{
		case CPythonChat::WHISPER_TYPE_SYSTEM:
			pChatLine->Instance.SetColor(D3DXCOLOR(1.0f, 0.785f, 0.785f, 1.0f));
			break;
		case CPythonChat::WHISPER_TYPE_GM:
			pChatLine->Instance.SetColor(D3DXCOLOR(1.0f, 0.632f, 0.0f, 1.0f));
			break;
		case CPythonChat::WHISPER_TYPE_CHAT:
		default:
			pChatLine->Instance.SetColor(0xffffffff);
			break;
	}

	m_ChatLineDeque.emplace_back(pChatLine);

	__ArrangeChat();
}

void CWhisper::__ArrangeChat()
{
	for (auto itor = m_ChatLineDeque.begin(); itor != m_ChatLineDeque.end(); ++itor)
	{
		TChatLine * pChatLine = *itor;
		pChatLine->Instance.Update();
	}
}

void CWhisper::Render(float fx, float fy)
{
	float fHeight = fy + m_fHeight;

	int iViewCount = int(m_fHeight / m_fLineStep) - 1;
	auto iLineCount = int(m_ChatLineDeque.size());
	int iStartLine = -1;
	if (iLineCount > iViewCount)
		iStartLine = int(float(iLineCount - iViewCount) * m_fcurPosition) + iViewCount - 1;
	else if (!m_ChatLineDeque.empty())
		iStartLine = iLineCount - 1;

	RECT Rect = {fx, fy, fx + m_fWidth, fy + m_fHeight};

	for (int i = iStartLine; i >= 0; --i)
	{
		assert(i >= 0 && i < int(m_ChatLineDeque.size()));
		TChatLine * pChatLine = m_ChatLineDeque[i];

		uint16_t wLineCount = pChatLine->Instance.GetTextLineCount();
		fHeight -= wLineCount * m_fLineStep;

		pChatLine->Instance.SetPosition(fx, fHeight);
		pChatLine->Instance.Render(&Rect);

		if (fHeight < fy)
			break;
	}
}

void CWhisper::__Initialize()
{
	m_fLineStep = 15.0f;
	m_fWidth = 300.0f;
	m_fHeight = 120.0f;
	m_fcurPosition = 1.0f;
}

void CWhisper::Destroy()
{
	std::for_each(m_ChatLineDeque.begin(), m_ChatLineDeque.end(), SChatLine::Delete);
	m_ChatLineDeque.clear();
	m_ShowingChatLineList.clear();
}

CWhisper::CWhisper()
{
	__Initialize();
}

CWhisper::~CWhisper()
{
	Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

CDynamicPool<CWhisper::SChatLine> CWhisper::SChatLine::ms_kPool;

CWhisper::SChatLine * CWhisper::SChatLine::New()
{
	return ms_kPool.Alloc();
}

void CWhisper::SChatLine::Delete(SChatLine * pkChatLine)
{
	pkChatLine->Instance.Destroy();
	ms_kPool.Free(pkChatLine);
}

void CWhisper::SChatLine::DestroySystem()
{
	ms_kPool.Destroy();
}

#ifdef ENABLE_CHAT_SETTINGS
void CPythonChat::DeleteChatSet(uint32_t dwID)
{
	auto it = m_ChatSetMap.find(dwID);
	if (it != m_ChatSetMap.end())
		m_ChatSetMap.erase(it);

	auto itChat = m_ChatLineNewDeque.find(dwID);
	if (itChat != m_ChatLineNewDeque.end())
	{
		if (itChat->second.size())
		{
			auto itChatLine = itChat->second.begin();
			while (itChatLine != itChat->second.end())
			{
				SChatLine* chatLine = *(itChatLine);
				SChatLine::Delete(chatLine);
				itChatLine = itChat->second.erase(itChatLine);
			}
		}

		itChat->second.clear();
		m_ChatLineNewDeque.erase(itChat);
	}
}

#if defined (ENABLE_CHAT_SETTINGS) && defined(ENABLE_MULTI_LANGUAGE_SYSTEM)
void CPythonChat::EnableCountryMode(uint32_t dwID, int iMode)
{
	TChatSet* pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_iCountryMode[iMode] = TRUE;
	ArrangeShowingChat(dwID);
}

void CPythonChat::DisableCountryMode(uint32_t dwID, int iMode)
{
	TChatSet* pChatSet = GetChatSetPtr(dwID);
	if (!pChatSet)
		return;

	pChatSet->m_iCountryMode[iMode] = FALSE;
	ArrangeShowingChat(dwID);
}
#endif
#endif
