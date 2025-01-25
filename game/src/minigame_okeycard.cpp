#include "stdafx.h"

#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
#include "minigame_manager.h"

#include "../../common/length.h"
#include "../../common/tables.h"

#include "char.h"
#include "desc_client.h"
#include "log.h"
#include "db.h"
#include "item.h"

void CHARACTER::Cards_open(uint8_t safemode)
{
	if (GetExchange() ||
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		GetViewingShopOwner() ||
#else
		GetMyShop() || GetShopOwner() ||
#endif
		IsOpenSafebox()
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
		|| IsOpenGuildstorage()
#endif
		)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANNOT_USE_CONSTRUCTION_WITH_CONDITIONS"));
		return;
	}

	if (character_cards.cards_left <= 0)
	{
		if (GetGold() < RUMI_PLAY_YANG)
		{
			ChatPacket(CHAT_TYPE_INFO, "You dont have enough money.");
			return;
		}
		if (CountSpecifyItem(RUMI_PLAY_ITEM) < 1)
		{
			ChatPacket(CHAT_TYPE_INFO, "You dont have any cardsets.");
			return;
		}

		PointChange(POINT_GOLD, -RUMI_PLAY_YANG, true);
		RemoveSpecifyItem(RUMI_PLAY_ITEM, 1);
		Cards_clean_list();
		character_cards.cards_left = EMonsterOkeyCardEvent::DECK_COUNT_MAX;
	}

	TEMP_BUFFER buf;
	TPacketGCMiniGameOkeyCard pack;
	TSubPacketGCMiniGameCardOpenClose sub{};
	pack.wSize = sizeof(TPacketGCMiniGameOkeyCard) + sizeof(TSubPacketGCMiniGameCardOpenClose);
	pack.bSubHeader = SUBHEADER_GC_RUMI_OPEN;
	sub.bSafeMode = safemode;

	LPDESC desc = GetDesc();
	if (!desc)
	{
		sys_err("User(%s)'s DESC is nullptr POINT.", GetName());
		return;
	}

	buf.write(&pack, sizeof(TPacketGCMiniGameOkeyCard));
	buf.write(&sub, sizeof(TSubPacketGCMiniGameCardOpenClose));
	desc->Packet(buf.read_peek(), buf.size());
	sys_log(0, "Send - MINI_GAME_RUMI_OPEN");
	//ChatPacket(CHAT_TYPE_COMMAND, "cards open %d", safemode);
	SendUpdatedInformations();
}

void CHARACTER::Cards_clean_list()
{
	memset(&character_cards, 0, sizeof(character_cards));
	memset(&randomized_cards, 0, sizeof(randomized_cards));
}

uint32_t CHARACTER::GetEmptySpaceInHand()
{
	for (int i = 0; i < EMonsterOkeyCardEvent::HAND_CARD_INDEX_MAX; ++i)
	{
		if (character_cards.cards_in_hand[i].type == 0)
			return i;
	}

	return -1;
}

void CHARACTER::Cards_pullout()
{
	uint32_t empty_space = GetEmptySpaceInHand();
	if (empty_space == -1)
	{
		ChatPacket(CHAT_TYPE_INFO, "You dont have space in hands.");
		return;
	}

	if (character_cards.cards_left < 1)
	{
		ChatPacket(CHAT_TYPE_INFO, "You dont have cards.");
		return;
	}

	if (GetAllCardsCount() >= EMonsterOkeyCardEvent::FIELD_CARD_SLOT_MAX)
	{
		ChatPacket(CHAT_TYPE_INFO, "You dont have space on table.");
		return;
	}

	RandomizeCards();
	SendUpdatedInformations();
}

void CHARACTER::RandomizeCards()
{
	uint32_t card_type = number(1, EMonsterOkeyCardEvent::CARD_COLOR_MAX);
	uint32_t card_value = number(1, EMonsterOkeyCardEvent::CARD_NUMBER_END);
	if (CardWasRandomized(card_type, card_value) != false)
	{
		RandomizeCards();
	}
	else
	{
		uint32_t empty_space = GetEmptySpaceInHand();
		character_cards.cards_in_hand[empty_space].type = card_type;
		character_cards.cards_in_hand[empty_space].value = card_value;
		character_cards.cards_left -= 1;
	}
}

bool CHARACTER::CardWasRandomized(uint32_t type, uint32_t value)
{
	for (int i = 0; i < EMonsterOkeyCardEvent::DECK_COUNT_MAX; ++i)
	{
		if (randomized_cards[i].type == type && randomized_cards[i].value == value)
			return true;
	}

	for (int i = 0; i < EMonsterOkeyCardEvent::DECK_COUNT_MAX; ++i)
	{
		if (randomized_cards[i].type == 0)
		{
			randomized_cards[i].type = type;
			randomized_cards[i].value = value;
			return false;
		}
	}

	return false;
}

void CHARACTER::SendUpdatedInformations()
{
	TEMP_BUFFER buf;
	TPacketGCMiniGameOkeyCard pack;
	TSubPacketGCMiniGameCardsInfo sub{};
	pack.wSize = sizeof(TPacketGCMiniGameOkeyCard) + sizeof(TSubPacketGCMiniGameCardsInfo);
	pack.bSubHeader = SUBHEADER_GC_RUMI_CARD_UPDATE;

	for (int i = 0; i < EMonsterOkeyCardEvent::HAND_CARD_INDEX_MAX; ++i)
	{
		sub.cardHandType[i] = character_cards.cards_in_hand[i].type;
		sub.cardHandValue[i] = character_cards.cards_in_hand[i].value;
	}
	sub.cardHandLeft = character_cards.cards_left;
	sub.cHandPoint = character_cards.points;

	for (int i = 0; i < EMonsterOkeyCardEvent::FIELD_CARD_INDEX_MAX; ++i)
	{
		sub.cardFieldType[i] = character_cards.cards_in_field[i].type;
		sub.cardFieldValue[i] = character_cards.cards_in_field[i].value;
	}
	sub.cFieldPoint = character_cards.field_points;

	LPDESC desc = GetDesc();
	if (!desc)
	{
		sys_err("User(%s)'s DESC is nullptr POINT.", GetName());
		return;
	}

	buf.write(&pack, sizeof(TPacketGCMiniGameOkeyCard));
	buf.write(&sub, sizeof(TSubPacketGCMiniGameCardsInfo));
	desc->Packet(buf.read_peek(), buf.size());
}

void CHARACTER::SendReward()
{
	TEMP_BUFFER buf;
	TPacketGCMiniGameOkeyCard pack;
	TSubPacketGCMiniGameCardsReward sub{};
	pack.wSize = sizeof(TPacketGCMiniGameOkeyCard) + sizeof(TSubPacketGCMiniGameCardsReward);
	pack.bSubHeader = SUBHEADER_GC_RUMI_CARD_REWARD;

	for (int i = 0; i < EMonsterOkeyCardEvent::FIELD_CARD_INDEX_MAX; ++i)
	{
		sub.cardType[i] = character_cards.cards_in_field[i].type;
		sub.cardValue[i] = character_cards.cards_in_field[i].value;
	}
	sub.cPoint = character_cards.field_points;

	LPDESC desc = GetDesc();
	if (!desc)
	{
		sys_err("User(%s)'s DESC is nullptr POINT.", GetName());
		return;
	}

	buf.write(&pack, sizeof(TPacketGCMiniGameOkeyCard));
	buf.write(&sub, sizeof(TSubPacketGCMiniGameCardsReward));
	desc->Packet(buf.read_peek(), buf.size());
}

void CHARACTER::CardsDestroy(uint32_t reject_index)
{
	if (reject_index + 1 > EMonsterOkeyCardEvent::HAND_CARD_INDEX_MAX)
		return;

	if (character_cards.cards_in_hand[reject_index].type == 0)
		return;

	character_cards.cards_in_hand[reject_index].type = 0;
	character_cards.cards_in_hand[reject_index].value = 0;
	SendUpdatedInformations();
}

void CHARACTER::CardsAccept(uint32_t accept_index)
{
	if (accept_index + 1 > EMonsterOkeyCardEvent::HAND_CARD_INDEX_MAX)
		return;

	if (character_cards.cards_in_hand[accept_index].type == 0)
		return;

	uint32_t empty_space = GetEmptySpaceInField();
	if (empty_space != -1)
	{
		character_cards.cards_in_field[empty_space].type = character_cards.cards_in_hand[accept_index].type;
		character_cards.cards_in_field[empty_space].value = character_cards.cards_in_hand[accept_index].value;
		character_cards.cards_in_hand[accept_index].type = 0;
		character_cards.cards_in_hand[accept_index].value = 0;
	}

	if (GetEmptySpaceInField() == -1)
	{
		if (CheckReward())
		{
			SendReward();
			ResetField();
		}
		else
			RestoreField();
	}

	SendUpdatedInformations();
}

void CHARACTER::CardsRestore(uint32_t restore_index)
{
	if (restore_index + 1 > EMonsterOkeyCardEvent::FIELD_CARD_INDEX_MAX)
		return;

	if (character_cards.cards_in_field[restore_index].type == 0)
		return;

	uint32_t empty_space = GetEmptySpaceInHand();
	character_cards.cards_in_hand[empty_space].type = character_cards.cards_in_field[restore_index].type;
	character_cards.cards_in_hand[empty_space].value = character_cards.cards_in_field[restore_index].value;
	character_cards.cards_in_field[restore_index].type = 0;
	character_cards.cards_in_field[restore_index].value = 0;
	SendUpdatedInformations();
}

uint32_t CHARACTER::GetEmptySpaceInField()
{
	for (int i = 0; i < EMonsterOkeyCardEvent::FIELD_CARD_INDEX_MAX; ++i)
	{
		if (character_cards.cards_in_field[i].type == 0)
			return i;
	}

	return -1;
}

uint32_t CHARACTER::GetAllCardsCount()
{
	uint32_t count = 0;
	for (int i = 0; i < EMonsterOkeyCardEvent::HAND_CARD_INDEX_MAX; ++i)
	{
		if (character_cards.cards_in_hand[i].type != 0)
			count += 1;
	}

	for (int i = 0; i < EMonsterOkeyCardEvent::FIELD_CARD_INDEX_MAX; ++i)
	{
		if (character_cards.cards_in_field[i].type != 0)
			count += 1;
	}

	return count;
}
bool CHARACTER::TypesAreSame()
{
	if (character_cards.cards_in_field[0].type == character_cards.cards_in_field[1].type && character_cards.cards_in_field[1].type == character_cards.cards_in_field[2].type)
		return true;

	return false;
}

bool CHARACTER::ValuesAreSame()
{
	if (character_cards.cards_in_field[0].value == character_cards.cards_in_field[1].value && character_cards.cards_in_field[1].value == character_cards.cards_in_field[2].value)
		return true;

	return false;
}

bool CHARACTER::CardsMatch()
{
	if (character_cards.cards_in_field[0].value == character_cards.cards_in_field[1].value - 1 && character_cards.cards_in_field[1].value == character_cards.cards_in_field[2].value - 1)
		return true;
	else if (character_cards.cards_in_field[0].value == character_cards.cards_in_field[2].value - 1 && character_cards.cards_in_field[2].value == character_cards.cards_in_field[1].value - 1)
		return true;
	else if (character_cards.cards_in_field[1].value == character_cards.cards_in_field[0].value - 1 && character_cards.cards_in_field[0].value == character_cards.cards_in_field[2].value - 1)
		return true;
	else if (character_cards.cards_in_field[0].value == character_cards.cards_in_field[1].value - 1 && character_cards.cards_in_field[2].value == character_cards.cards_in_field[0].value - 1)
		return true;
	else if (character_cards.cards_in_field[1].value == character_cards.cards_in_field[0].value - 1 && character_cards.cards_in_field[2].value == character_cards.cards_in_field[1].value - 1)
		return true;
	else if (character_cards.cards_in_field[1].value == character_cards.cards_in_field[2].value - 1 && character_cards.cards_in_field[2].value == character_cards.cards_in_field[0].value - 1)
		return true;

	return false;
}

uint32_t CHARACTER::GetLowestCard()
{
	return MIN(character_cards.cards_in_field[0].value, MIN(character_cards.cards_in_field[1].value, character_cards.cards_in_field[2].value));
}

bool CHARACTER::CheckReward()
{
	if (TypesAreSame() && ValuesAreSame())
	{
		character_cards.field_points = 150;
		character_cards.points += 150;
		return true;
	}
	else if (TypesAreSame() && CardsMatch())
	{
		character_cards.field_points = 100;
		character_cards.points += 100;
		return true;
	}
	else if (ValuesAreSame())
	{
		character_cards.field_points = GetLowestCard() * 10 + 10;
		character_cards.points += GetLowestCard() * 10 + 10;
		return true;
	}
	else if (CardsMatch())
	{
		character_cards.field_points = GetLowestCard() * 10;
		character_cards.points += GetLowestCard() * 10;
		return true;
	}
	else
	{
		RestoreField();
		return false;
	}
}

void CHARACTER::RestoreField()
{
	for (int i = 0; i < EMonsterOkeyCardEvent::FIELD_CARD_INDEX_MAX; ++i)
	{
		uint32_t empty_space = GetEmptySpaceInHand();
		character_cards.cards_in_hand[empty_space].type = character_cards.cards_in_field[i].type;
		character_cards.cards_in_hand[empty_space].value = character_cards.cards_in_field[i].value;
		character_cards.cards_in_field[i].type = 0;
		character_cards.cards_in_field[i].value = 0;
		SendUpdatedInformations();
	}
}

void CHARACTER::ResetField()
{
	for (int i = 0; i < EMonsterOkeyCardEvent::FIELD_CARD_INDEX_MAX; ++i)
	{
		character_cards.cards_in_field[i].type = 0;
		character_cards.cards_in_field[i].value = 0;
	}
}

void CHARACTER::CardsEnd()
{
	if (character_cards.points >= EMonsterOkeyCardEvent::MID_TOTAL_SCORE)
		AutoGiveItem(RUMI_REWARD_L); //Golden box
	else if (character_cards.points >= EMonsterOkeyCardEvent::LOW_TOTAL_SCORE)
		AutoGiveItem(RUMI_REWARD_M); //Silver box
	else if (character_cards.points > 0)
		AutoGiveItem(RUMI_REWARD_S); //Bronze box

	LogManager::Instance().OkayEventLog(GetPlayerID(), GetName(), character_cards.points);
	Cards_clean_list();
	SendUpdatedInformations();
}

void CHARACTER::GetGlobalRank(char* buffer, size_t buflen)
{
	auto pMsg(DBManager::Instance().DirectQuery("SELECT name, SUM(points) as spoints FROM log.okay_event GROUP BY pid ORDER BY spoints DESC LIMIT 10"));
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
		int points = 0;
		str_to_number(points, row[1]);
		len2 = snprintf(buffer + len, buflen - len, "[ENTER]");

		if (len2 < 0 || len2 >= (int)buflen - len)
			len += (buflen - len) - 1;
		else
			len += len2;

		len2 = snprintf(buffer + len, buflen - len, "%d. Place - %s - %d points.",
			pos,
			row[0],
			points);
		++pos;
		if (len2 < 0 || len2 >= (int)buflen - len)
			len += (buflen - len) - 1;
		else
			len += len2;
	}
}

void CHARACTER::GetRundRank(char* buffer, size_t buflen)
{
	auto pMsg(DBManager::Instance().DirectQuery("SELECT name, points FROM log.okay_event ORDER BY points DESC LIMIT 10"));
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
		int points = 0;
		str_to_number(points, row[1]);
		len2 = snprintf(buffer + len, buflen - len, "[ENTER]");

		if (len2 < 0 || len2 >= (int)buflen - len)
			len += (buflen - len) - 1;
		else
			len += len2;

		len2 = snprintf(buffer + len, buflen - len, "%d. Place - %s - %d points.",
			pos,
			row[0],
			points);
		++pos;
		if (len2 < 0 || len2 >= (int)buflen - len)
			len += (buflen - len) - 1;
		else
			len += len2;
	}
}
#	endif	// ENABLE_MINI_GAME_OKEY_NORMAL
