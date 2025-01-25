#include "stdafx.h"
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
#include "constants.h"
#include "desc.h"
#include "log.h"
#include "char.h"
#include "db.h"
#include "item.h"
#include "item_manager.h"
#include "packet.h"

void CHARACTER::OpenAcceCombination()
{
	if (m_bAcceCombination || m_bAcceAbsorption)
	{
		//ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You already have a acce window open."));
		return;
	}

	m_bAcceCombination = true;
#ifdef ENABLE_CHECK_WINDOW_RENEWAL
	SetOpenedWindow(W_ACCE, true);
#endif
	ChatPacket(CHAT_TYPE_COMMAND, "ShowAcceCombineDialog");
}

void CHARACTER::OpenAcceAbsorption()
{
	if (m_bAcceCombination || m_bAcceAbsorption)
	{
		//ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You already have a acce window open."));
		return;
	}

	m_bAcceAbsorption = true;
#ifdef ENABLE_CHECK_WINDOW_RENEWAL
	SetOpenedWindow(W_ACCE, true);
#endif
	ChatPacket(CHAT_TYPE_COMMAND, "ShowAcceAbsorbDialog");
}

void CHARACTER::AcceClose()
{
	m_bAcceCombination = false;
	m_bAcceAbsorption = false;
#ifdef ENABLE_CHECK_WINDOW_RENEWAL
	SetOpenedWindow(W_ACCE, false);
#endif
}

bool CHARACTER::AcceRefine(uint8_t bAcceWindow, uint8_t bSlotAcce, uint8_t bSlotMaterial)
{
	LPITEM AcceItem = GetInventoryItem(bSlotAcce);
	if (!AcceItem)
		return false;

	LPITEM AcceMaterial = GetInventoryItem(bSlotMaterial);
	if (!AcceMaterial)
		return false;

	if (AcceItem->IsEquipped() || AcceMaterial->IsEquipped())
		return false;

	if (AcceItem->isLocked() || AcceMaterial->isLocked())
		return false;

#ifdef ENABLE_SEALBIND_SYSTEM
	if (AcceItem->IsSealed() || AcceMaterial->IsSealed())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1132]");
		return false;
	}
#endif

	if (bAcceWindow == 0)
	{
		if ((AcceItem->GetType() != ITEM_COSTUME && AcceItem->GetSubType() != COSTUME_ACCE) ||
			(AcceMaterial->GetType() != ITEM_COSTUME && AcceMaterial->GetSubType() != COSTUME_ACCE))
			return false;

		if ((AcceItem->GetApplyType(0) != APPLY_ACCEDRAIN_RATE) || (AcceMaterial->GetApplyType(0) != APPLY_ACCEDRAIN_RATE))
			return false;

		if (AcceItem->GetApplyValue(0) != AcceMaterial->GetApplyValue(0))
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;1125]");
			return false;
		}

		const int dwPrice = AcceItem->GetShopBuyPrice();
		if (GetGold() < dwPrice)
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;1128]");
			return false;
		}

		int iSuccessChance = 0;
		const auto lVal = AcceItem->GetApplyValue(0);
		switch (lVal)
		{
			case 1:
				iSuccessChance = ACCE_COMBINE_GRADE_1;
				break;
			case 5:
				iSuccessChance = ACCE_COMBINE_GRADE_2;
				break;
			case 10:
				iSuccessChance = ACCE_COMBINE_GRADE_3;
				break;
			case 20:
				iSuccessChance = ACCE_COMBINE_GRADE_4;
				break;
			default:
				break;
		}

		if (iSuccessChance == 0)
			return false;

		const int iChance = number(1, 100);
		const bool bSuccess = (iChance <= iSuccessChance ? true : false);

		if (bSuccess)
		{
			uint32_t dwRefinedVnum = AcceItem->GetRefinedVnum();
			if (dwRefinedVnum == 0)
			{
				dwRefinedVnum = AcceItem->GetVnum();

				if (AcceItem->GetSocket(1) >= 25 || AcceMaterial->GetSocket(1) >= 25)
					return false;
			}

			LPITEM pkItem = ITEM_MANAGER::Instance().CreateItem(dwRefinedVnum, 1, 0, true);
			if (!pkItem)
			{
				sys_err("<Acce> %d can't be created.", dwRefinedVnum);
				return false;
			}

			if (pkItem)
			{
				pkItem->SetSocket(0, AcceItem->GetSocket(0));

				if (pkItem->GetRefinedVnum() == 0)
				{
					const int currentDrainValue = AcceItem->GetSocket(1);
					const int nextDrainValue = (currentDrainValue + 5);

					int newDrainValue = number(currentDrainValue, (nextDrainValue > 25 ? 25 : nextDrainValue));

					if (currentDrainValue >= 25)
						newDrainValue = 25;

					pkItem->SetSocket(1, newDrainValue);
				}

				for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
				{
					pkItem->SetForceAttribute(i, AcceItem->GetAttributeType(i), AcceItem->GetAttributeValue(i));
				}
#ifdef ENABLE_REFINE_ELEMENT //ACCE_ELEMENT
				pkItem->SetElement(AcceItem->GetElementGrade(), AcceItem->GetElementAttacks(), AcceItem->GetElementsType(), AcceItem->GetElementsValues());
#endif
#ifdef ENABLE_YOHARA_SYSTEM
				for (int r = 0; r < APPLY_RANDOM_SLOT_MAX_NUM; ++r)
					pkItem->SetItemApplyRandom(r, AcceItem->GetApplyRandomType(r), AcceItem->GetApplyRandomValue(r), AcceItem->GetApplyRandomRow(r));

				for (int s = 0; s < ITEM_RANDOM_VALUES_MAX_NUM; ++s)
					pkItem->SetRandomDefaultAttr(s, AcceItem->GetRandomDefaultAttr(s));
#endif
#ifdef ENABLE_SET_ITEM
				pkItem->SetItemSetValue(AcceItem->GetItemSetValue());
#endif
			}

			PointChange(POINT_GOLD, -dwPrice);
			DBManager::Instance().SendMoneyLog(MONEY_LOG_REFINE, AcceItem->GetVnum(), -dwPrice);

			const uint16_t wCell = AcceItem->GetCell();

			ITEM_MANAGER::Instance().RemoveItem(AcceItem, "COMBINE (REFINE SUCCESS)");
			ITEM_MANAGER::Instance().RemoveItem(AcceMaterial, "COMBINE (REFINE SUCCESS)");

			pkItem->AddToCharacter(this, TItemPos(INVENTORY, wCell));

			EffectPacket(SE_ACCE_SUCESS_ABSORB);

			if (lVal == 20)
				ChatPacket(CHAT_TYPE_NOTICE, "[LS;1131;%s;%d;[IN;%d]]", GetName(), pkItem->GetSocket(1), pkItem->GetVnum());
			else
				ChatPacket(CHAT_TYPE_INFO, "[LS;1127]");
		}
		else
		{
			PointChange(POINT_GOLD, -dwPrice);
			DBManager::Instance().SendMoneyLog(MONEY_LOG_REFINE, AcceItem->GetVnum(), -dwPrice);
			ITEM_MANAGER::Instance().RemoveItem(AcceMaterial, "COMBINE (REFINE FAIL)");

			if (lVal == 20)
				ChatPacket(CHAT_TYPE_INFO, "[LS;1204;%d]", AcceItem->GetSocket(1));
			else
				ChatPacket(CHAT_TYPE_INFO, "[LS;1126]");
		}

		// Send packet result
		TPacketGCSendAcceRefine sPacket{};
		sPacket.bHeader = HEADER_GC_ACCE_REFINE_REQUEST;
		sPacket.bAcceWindow = bAcceWindow;
		sPacket.bResult = bSuccess;
		if (GetDesc())
			GetDesc()->Packet(&sPacket, sizeof(TPacketGCSendAcceRefine));

		return true;
	}

	if (bAcceWindow == 1)
	{
		if (AcceItem->GetType() != ITEM_COSTUME && AcceItem->GetSubType() != COSTUME_ACCE)
			return false;

		if (AcceItem->GetApplyType(0) != APPLY_ACCEDRAIN_RATE)
			return false;

		if (AcceMaterial->GetType() != ITEM_WEAPON && AcceMaterial->GetType() != ITEM_ARMOR && AcceMaterial->GetType() != ARMOR_BODY)
			return false;

		if (IS_SET(AcceMaterial->GetAntiFlag(), ITEM_ANTIFLAG_ACCE))
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;162]");
			return false;
		}

		AcceItem->SetSocket(0, AcceMaterial->GetVnum());
		for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
		{
			AcceItem->SetForceAttribute(i, AcceMaterial->GetAttributeType(i), AcceMaterial->GetAttributeValue(i));
		}

#ifdef ENABLE_REFINE_ELEMENT //ACCE_ELEMENT
		AcceItem->SetElement(AcceMaterial->GetElementGrade(), AcceMaterial->GetElementAttacks(), AcceMaterial->GetElementsType(), AcceMaterial->GetElementsValues());
#endif
#ifdef ENABLE_YOHARA_SYSTEM
		for (int r = 0; r < APPLY_RANDOM_SLOT_MAX_NUM; ++r)
			AcceItem->SetItemApplyRandom(r, AcceMaterial->GetApplyRandomType(r), AcceMaterial->GetApplyRandomValue(r), AcceMaterial->GetApplyRandomRow(r));

		for (int s = 0; s < ITEM_RANDOM_VALUES_MAX_NUM; ++s)
			AcceItem->SetRandomDefaultAttr(s, AcceMaterial->GetRandomDefaultAttr(s));
#endif
#ifdef ENABLE_SET_ITEM
		AcceItem->SetItemSetValue(AcceMaterial->GetItemSetValue());
#endif

		ITEM_MANAGER::Instance().RemoveItem(AcceMaterial, "ABSORBED (REFINE SUCCESS)");
		ChatPacket(CHAT_TYPE_INFO, "[LS;1127]");

		// Send packet result
		TPacketGCSendAcceRefine sPacket{};
		sPacket.bHeader = HEADER_GC_ACCE_REFINE_REQUEST;
		sPacket.bAcceWindow = bAcceWindow;
		sPacket.bResult = true;
		if (GetDesc())
			GetDesc()->Packet(&sPacket, sizeof(TPacketGCSendAcceRefine));

		return true;
	}

	return true;
}
#endif
