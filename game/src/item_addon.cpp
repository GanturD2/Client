#include "stdafx.h"
#include "config.h"
#include "constants.h"
#include "utils.h"
#include "item.h"
#include "item_addon.h"

int minAddonFKS = MIN_ADDON_FKS;
int maxAddonFKS = MAX_ADDON_FKS;
int minAddonDSS = MIN_ADDON_DSS;
int maxAddonDSS = MAX_ADDON_DSS;

CItemAddonManager::CItemAddonManager() {}

CItemAddonManager::~CItemAddonManager() {}

void CItemAddonManager::ApplyAddonTo(int iAddonType, LPITEM pItem)
{
	if (!pItem) {
		sys_err("ITEM pointer null");
		return;
	}

	if (minAddonFKS>maxAddonFKS) {
		sys_err("item_addon: minAddonFKS [%d] is bigger than maxAddonFKS [%d]", minAddonFKS, maxAddonFKS);
		int bufferFKS = minAddonFKS;
		minAddonFKS = maxAddonFKS;
		maxAddonFKS = bufferFKS;
	}

	if (minAddonDSS == 32767)
		minAddonDSS = (minAddonFKS * 2);

	if (maxAddonDSS == 32767)
		maxAddonDSS = (maxAddonFKS * 2);

	if (minAddonDSS>maxAddonDSS) {
		sys_err("item_addon: minAddonDSS [%d] is bigger than maxAddonDSS [%d]", minAddonDSS, maxAddonDSS);
		int bufferDSS = minAddonDSS;
		minAddonDSS = maxAddonDSS;
		maxAddonDSS = bufferDSS;
	}


//	int iSkillBonus = MINMAX(-30, (int)(gauss_random(0, 5) + 0.5f), 30);	// Original
	int iSkillBonus = MINMAX(minAddonFKS, (int)(gauss_random(0, 5) + 0.5f), maxAddonFKS);	// <- Vanilla
	int iNormalHitBonus = 0;
	if (abs(iSkillBonus) <= 20)
		iNormalHitBonus = -2 * iSkillBonus + abs(number(-8, 8) + number(-8, 8)) + number(1, 4);
	else
		iNormalHitBonus = -2 * iSkillBonus + number(1, 5);

	iNormalHitBonus = MINMAX(minAddonDSS, iNormalHitBonus, maxAddonDSS);	// <- Vanilla

	pItem->RemoveAttributeType(APPLY_SKILL_DAMAGE_BONUS);
	pItem->RemoveAttributeType(APPLY_NORMAL_HIT_DAMAGE_BONUS);
	pItem->AddAttribute(APPLY_NORMAL_HIT_DAMAGE_BONUS, iNormalHitBonus);
	pItem->AddAttribute(APPLY_SKILL_DAMAGE_BONUS, iSkillBonus);
}
