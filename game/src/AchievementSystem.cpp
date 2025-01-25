#include "stdafx.h"

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
#include "AchievementSystem.h"
#include "char.h"
#include "desc.h"
#include "config.h"
#include "event.h"
#include "p2p.h"
#include "utils.h"
#include "../../libachievement/xml.hpp"
#include "../../libachievement/Document.hpp"
#include "locale_service.h"
#include "log.h"
#include <fstream>
#include "desc_client.h"
#include "desc_manager.h"
#include "char_manager.h"
#include "PetSystem.h"
#include "../../libachievement/serializer.hpp"
#include "../../libachievement/deserializer.hpp"
#include "shop_manager.h"
#include "shop.h"

EVENTINFO(player_load_achievement_info)
{
	DWORD pid;
	TAchievementsMap achievements;
	uint32_t points, title;
	player_load_achievement_info() : pid(0), points(0), title(0) {}
};

EVENTFUNC(player_load_achievement)
{
	player_load_achievement_info* info =
		static_cast<player_load_achievement_info*>(event->info);

	if (info == NULL || info->pid == 0)
	{
		sys_err("player_load_achievement_info> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(info->pid);

	if (!ch || !ch->GetDesc())
	{
		sys_err("player_load_achievement_info> <Factor> Null pointer");
		return 0;
	}

	if (ch->GetDesc()->IsPhase(PHASE_HANDSHAKE) ||
		ch->GetDesc()->IsPhase(PHASE_LOGIN) ||
		ch->GetDesc()->IsPhase(PHASE_SELECT) ||
		ch->GetDesc()->IsPhase(PHASE_DEAD) ||
		ch->GetDesc()->IsPhase(PHASE_LOADING))
	{
		return PASSES_PER_SEC(1);
	}
	else if (ch->GetDesc()->IsPhase(PHASE_CLOSE))
	{
		return 0;
	}
	else if (ch->GetDesc()->IsPhase(PHASE_GAME))
	{
		CAchievementSystem::Instance().OnLogin(ch, info->achievements, info->points, info->title);
		return 0;
	}
	else
		return 0;
}

CAchievementSystem::CAchievementSystem()
{
	_achievements.clear();
	_titles.clear();
}

CAchievementSystem::~CAchievementSystem() {}

void CAchievementSystem::Initialize()
{
	_achievements.clear();
	_titles.clear();

	char szFileName[128];
	snprintf(szFileName, sizeof(szFileName), "%s/%s",
		LocaleService_GetBasePath().c_str(), achievements::CONFIG_FILE);

	xml::Document document(szFileName);
	auto rootNode = document.first_node("Achievements");
	if (!rootNode)
	{
		sys_err("Root node in %s not found: achievements",
			achievements::CONFIG_FILE);
		return;
	}

	for (auto currentAchievement = rootNode->first_node("Achievement");
		currentAchievement;
		currentAchievement = currentAchievement->next_sibling())
	{
		uint32_t achievement_id;

		achievements::TAchievementData achievement;
		achievement.tasks.clear();
		achievement.rewards.clear();
		try {
			xml::GetAttribute(currentAchievement, "id", achievement_id);
			xml::GetAttribute(currentAchievement, "max_value",
				achievement.max_value);
		}
		catch (const xml::Exception& e) {
			sys_err(e.what());
			continue;
		}

		auto tasksNode = currentAchievement->first_node("Tasks");

		if (!tasksNode)
		{
			sys_err("Task node in %s not found: achievements",
				achievements::CONFIG_FILE);
			return;
		}

		for (auto currentTask = tasksNode->first_node("Task"); currentTask;
			currentTask = currentTask->next_sibling())
		{
			achievements::TTask task;
			uint32_t task_id = 1;
			try {
				xml::GetAttribute(currentTask, "id", task_id);
				xml::GetAttribute(currentTask, "type", task.type);
				xml::GetAttribute(currentTask, "max_value", task.max_value);
				xml::GetAttribute(currentTask, "min_level", task.min_level);
				xml::GetAttribute(currentTask, "max_level", task.max_level);

			}
			catch (const xml::Exception& e) {
				sys_err("%s (achievement %d, task %d)", e.what(), achievement_id, task_id);
				continue;
			}

			auto restrictionsNode = currentTask->first_node("Restrictions");
			if (!restrictionsNode)
			{
				sys_err("Restrictions node in %s not found: achievements",
					achievements::CONFIG_FILE);
				return;
			}

			for (auto currentRestriction =
				restrictionsNode->first_node("Restriction");
				currentRestriction;
				currentRestriction = currentRestriction->next_sibling())
			{
				try {
					uint8_t r;
					uint32_t value;

					xml::GetAttribute(currentRestriction, "type", r);
					xml::GetAttribute(currentRestriction, "value", value);

					if (r < 0 || r >= achievements::RESTRICTIONS_MAX_NUM)
						continue;

					task.restrictions[r] = value;
				}
				catch (const xml::Exception& e) {
					sys_err(e.what());
					continue;
				}
			}

			achievement.tasks.insert({ task_id, task });
		}

		auto rewardsNode = currentAchievement->first_node("Rewards");

		if (!rewardsNode)
		{
			sys_err("Reward node in %s not found: achievements",
				achievements::CONFIG_FILE);
			return;
		}

		for (auto currentReward = rewardsNode->first_node("Reward");
			currentReward; currentReward = currentReward->next_sibling())
		{
			achievements::TReward reward;

			try {
				xml::GetAttribute(currentReward, "type", reward.type);
				xml::GetAttribute(currentReward, "value", reward.value);
				xml::GetAttribute(currentReward, "count", reward.count);

			}
			catch (const xml::Exception& e) {
				sys_err(e.what());
				continue;
			}

			achievement.rewards.push_back(reward);
		}

		_achievements.insert({ achievement_id, achievement });
	}

	auto rootNodeTitles = document.first_node("Titles");
	if (!rootNodeTitles)
	{
		sys_err("Root node in %s not found: titles",
			achievements::CONFIG_FILE);
		return;
	}

	_titles.clear();
	for (auto currentTitle = rootNodeTitles->first_node("Title"); currentTitle;
		currentTitle = currentTitle->next_sibling())
	{
		uint32_t title_id;
		uint8_t bonusType;
		uint32_t bonusValue;
		try {
			std::string type;
			xml::GetAttribute(currentTitle, "bonus_type", type);
			if (!TryGetAttributeFromString(type, bonusType)) {
				sys_log(0, 0, "Invalid apply type %d.", type);
				bonusType = APPLY_NONE;
			}
			xml::GetAttribute(currentTitle, "id", title_id);
			xml::GetAttribute(currentTitle, "bonus_value", bonusValue);
		}
		catch (const xml::Exception& e) {
			sys_err(e.what());
			continue;
		}

		_titles.insert({ title_id, {bonusType, bonusValue} });
	}
}

void CAchievementSystem::OnLogin(LPCHARACTER player, TAchievementsMap& achievement, uint32_t points, uint32_t title)
{
	if (!player)
		return;

	if (player->IsGM())
		player->ChatPacket(CHAT_TYPE_INFO, "Achievements data loaded successfully!");

	player->SetAchievementInfo(achievement);
	player->SetAchievementPoints(points);
	player->SetAchievementTitle(title);
	RefreshAchievementTitles(player);

	achievements::GC_packet packet = { HEADER_GC_ACHIEVEMENT, 0, achievements::HEADER_GC_INITIAL };

	achievements::GC_load subpacket = { points, 0, 0 };

	subpacket.achievements = achievement.size();
	const std::vector<uint32_t> titles = player->GetAchievementTitles();
	subpacket.titles = titles.size();

	packet.size += sizeof(achievements::GC_packet);
	packet.size += sizeof(achievements::GC_load);
	packet.size += (sizeof(achievements::TAchievement) + sizeof(uint32_t)) * subpacket.achievements;
	packet.size += sizeof(uint32_t) * subpacket.titles;

	TEMP_BUFFER buff;
	buff.write(&packet, sizeof(packet));
	buff.write(&subpacket, sizeof(subpacket));
	for (const auto& ach : achievement)
	{
		achievements::TAchievement a;
		uint32_t id = ach.first;
		const auto& task = ach.second.find(0);
		if (ach.second.size() == 1 && task != ach.second.end())
		{
			a.status = achievements::EAchievementStatus::FINISHED;
			a.percentage = 100;
			a.finish_timestamp = task->second;
			buff.write(&a, sizeof(achievements::TAchievement));
			buff.write(&id, sizeof(uint32_t));
			continue;
		}
		a.percentage = static_cast<uint8_t>(GetAchievementProgress(player, ach.first));
		a.finish_timestamp = 0;

		if (ach.second.size() == 0 && a.percentage == 0)
			a.status = achievements::EAchievementStatus::LOCKED;
		else
			a.status = achievements::EAchievementStatus::UNLOCKED;
		buff.write(&a, sizeof(achievements::TAchievement));
		buff.write(&id, sizeof(uint32_t));
	}
	for (const auto& title : titles)
	{
		uint32_t id = title;
		buff.write(&id, sizeof(uint32_t));
	}
	player->GetDesc()->BufferedPacket(buff.read_peek(), buff.size());

	player->UpdatePacket();
	if (player->GetGuild())
		OnSocial(player, achievements::ETaskTypes::TYPE_JOIN_GUILD);
	OnCharacterUpdate(player);
	OnVisitMap(player);
	for (const auto& achievement : _achievements)
	{
		if (achievement.second.tasks.size() == 0 && !IsAchievementFinished(player, achievement.first))
			FinishAchievement(player, achievement.first);
	}
}

void CAchievementSystem::OnLogout(LPCHARACTER player)
{
	if (!player)
		return;

	if (!player->GetAchievementInfo())
		return;

	TAchievementsMap& Achievements = *(player->GetAchievementInfo().get());

	if (player->GetQuestFlag("pet.summon_time") > 0) // This should not be possible.
		player->SetQuestFlag("pet.summon_time", 0);

	uint8_t bSubHeader = ACHIEVEMENT_SUBHEADER_LOGOUT;
	uint32_t pid = player->GetPlayerID();
	uint32_t achievement_points = player->GetAchievementPoints();
	uint32_t selected_title = player->GetAchievementTitle();

	auto achievementsBuffer = SERIALIZE_NAMESPACE::serialize(Achievements);
	uint32_t achievementsBufferSize = achievementsBuffer.size();

	db_clientdesc->DBPacketHeader(HEADER_GD_ACHIEVEMENT, 0, sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + achievementsBuffer.size());
	db_clientdesc->Packet(&bSubHeader, sizeof(uint8_t));
	db_clientdesc->Packet(&pid, sizeof(uint32_t));
	db_clientdesc->Packet(&achievement_points, sizeof(uint32_t));
	db_clientdesc->Packet(&selected_title, sizeof(uint32_t));
	db_clientdesc->Packet(&achievementsBufferSize, sizeof(uint32_t));
	db_clientdesc->Packet(achievementsBuffer.data(), achievementsBuffer.size());
}

void CAchievementSystem::ProcessDBPackets(LPDESC desc, const char* c_pData)
{
	if (!desc)
		return;

	uint8_t bSubHeader = *(uint8_t*)c_pData;
	c_pData += sizeof(uint8_t);
	switch (bSubHeader)
	{
		case ACHIEVEMENT_SUBHEADER_LOGIN:
		{
			uint32_t pid = *(uint32_t*)c_pData;
			c_pData += sizeof(uint32_t);

			uint32_t dwAchievementPoints = *(uint32_t*)c_pData;
			c_pData += sizeof(uint32_t);

			uint32_t dwSelectedTitle = *(uint32_t*)c_pData;
			c_pData += sizeof(uint32_t);

			uint32_t bAchievementsSize = *(uint32_t*)c_pData;
			c_pData += sizeof(uint32_t);

			TAchievementsMap Achievements;
			Achievements.clear();

			if (bAchievementsSize > 0)
			{
				Achievements = SERIALIZE_NAMESPACE::deserialize<TAchievementsMap>( c_pData, bAchievementsSize);
				c_pData += bAchievementsSize;
			}

			for (const auto& achievement : _achievements)
			{
				auto ach = Achievements.find(achievement.first);
				if (ach == Achievements.end())
				{
					TTasksList tasks;
					tasks.clear();
					ach = Achievements.insert({ achievement.first, tasks }).first;
				}
				if (ach->second.find(0) == ach->second.end())
				{
					for (const auto& task : achievement.second.tasks)
					{
						auto t = ach->second.find(task.first);
						if (t == ach->second.end()) {
							t = ach->second.insert({ task.first, 0 }).first;
						}
					}
				}
			}

			if (desc->GetCharacter() && desc->IsPhase(PHASE_GAME))
				OnLogin(desc->GetCharacter(), Achievements, dwAchievementPoints, dwSelectedTitle);
			else
			{
				player_load_achievement_info* info = AllocEventInfo<player_load_achievement_info>();
				info->pid = pid;
				info->achievements = Achievements;
				info->points = dwAchievementPoints;
				info->title = dwSelectedTitle;
				event_create(player_load_achievement, info, PASSES_PER_SEC(1));
			}
		}
		break;

		case ACHIEVEMENT_SUBHEADER_RANKING:
		{
			if (!desc->GetCharacter())
			{
				sys_err("Achievement ranking requested but there is no player?");
				return;
			}

			uint32_t bRankingSize = *(uint32_t*)c_pData;
			c_pData += sizeof(uint32_t);

			std::vector<TAchievementRanking> m_dwAchievementRanking;

			while (bRankingSize > 0)
			{
				TAchievementRanking rank = *(TAchievementRanking*)c_pData;
				c_pData += sizeof(TAchievementRanking);
				m_dwAchievementRanking.push_back(rank);

				--bRankingSize;
			}

			achievements::GC_packet packet = { HEADER_GC_ACHIEVEMENT, 0, achievements::HEADER_GC_RANKING };

			uint32_t size = m_dwAchievementRanking.size();

			packet.size += sizeof(achievements::GC_packet);
			packet.size += sizeof(uint32_t);
			packet.size += sizeof(achievements::GC_ranking) * size;

			TEMP_BUFFER buff;
			buff.write(&packet, sizeof(packet));
			buff.write(&size, sizeof(uint32_t));

			for (int i = 0; i < size; ++i)
			{
				achievements::GC_ranking rank;
				rank.level = m_dwAchievementRanking[i].level;
				rank.progress = m_dwAchievementRanking[i].finished;
				strlcpy(rank.name, m_dwAchievementRanking[i].szName,
					sizeof(rank.name));

				strlcpy(rank.guild, m_dwAchievementRanking[i].szGuild,
					sizeof(rank.guild));

				buff.write(&rank, sizeof(achievements::GC_ranking));
			}
			desc->BufferedPacket(buff.read_peek(), buff.size());
			m_dwAchievementRanking.clear();

		}
		break;

		default:
			break;
	}
}

void CAchievementSystem::ProcessClientPackets(LPCHARACTER player, const char* c_pData)
{
	if (!player || !player->IsPC())
	{
		sys_err("Received achievement client packet but not from a player?");
		return;
	}

	achievements::CG_packet packet = *(achievements::CG_packet*)c_pData;
	switch (packet.subHeader)
	{
		case achievements::HEADER_CG_SELECT_TITLE:
		{
			SelectTitle(player, packet.dummy);
		}
		break;

		case achievements::HEADER_CG_OPEN_SHOP:
		{
			//player->ChatPacket(CHAT_TYPE_INFO, "You have to open this from the NPC.");
			LPSHOP shop = CShopManager::Instance().Get(104);
			if (shop)
			{
				shop->AddGuest(player, 0, false);
	#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
				player->SetViewingShopOwner(nullptr);
	#else
				player->SetShopOwner(nullptr);
	#endif
			}
		}
		break;

		case achievements::HEADER_CG_OPEN_RANKING:
		{
			uint8_t bSubHeader = ACHIEVEMENT_SUBHEADER_RANKING;
			db_clientdesc->DBPacketHeader(HEADER_GD_ACHIEVEMENT, player->GetDesc()->GetHandle(), sizeof(uint8_t));
			db_clientdesc->Packet(&bSubHeader, sizeof(uint8_t));
		}
		break;

		default:
			break;
	}
}

void CAchievementSystem::SelectTitle(LPCHARACTER player, uint32_t titleID)
{
	if (!player)
		return;

	if (player->GetAchievementTitle() == titleID)
	{
		player->SetAchievementTitle(0);
		player->UpdatePacket();
	}
	else
	{
		const std::vector<uint32_t> titles = player->GetAchievementTitles();
		const auto& title = std::find(titles.begin(), titles.end(), titleID);
		if (title == titles.end())
		{
			player->ChatPacket(CHAT_TYPE_INFO, "[LS;1928]");
			return;
		}

		uint32_t oldTitle = player->GetAchievementTitle();

		player->SetAchievementTitle(titleID);
		player->UpdatePacket();
		achievements::GC_packet packet = { HEADER_GC_ACHIEVEMENT, 0, achievements::HEADER_GC_RELOAD_TITLES };

		packet.size += sizeof(achievements::GC_packet);
		packet.size += sizeof(uint32_t);
		packet.size += sizeof(uint32_t);

		TEMP_BUFFER buff;
		buff.write(&packet, sizeof(packet));
		buff.write(&oldTitle, sizeof(uint32_t));
		buff.write(&titleID, sizeof(uint32_t));
		player->GetDesc()->BufferedPacket(buff.read_peek(), buff.size());
	}

	player->ChatPacket(CHAT_TYPE_INFO, "[LS;1929]");
}

bool CAchievementSystem::GetAchievementInfo( uint32_t id, achievements::TAchievementData** achievement)
{
	const auto& it = _achievements.find(id);
	if (it == _achievements.end())
		return false;

	*achievement = &(it->second);

	return true;
}

bool CAchievementSystem::IsTaskFinished(LPCHARACTER player, const uint32_t value, const achievements::TTask& category_task)
{
	if (!player)
		return false;

	if (category_task.type == achievements::ETaskTypes::TYPE_SKILL)
	{
		if (!player->IsLearnableSkillAchievement(category_task.restrictions[achievements::ERestrictionTypes::RESTRICTION_VNUM]))
			return false;
	}
	/*
		if (category_task.max_value <= value ||
			(player->GetLevel() >= category_task.max_level &&
				category_task.max_level > 0))
			return true;
	*/
	if (category_task.max_value <= value)
		return true;

	return false;
}

bool CAchievementSystem::IsTaskFinished(LPCHARACTER player, const uint32_t achievement_id, const uint8_t task_id)
{
	if (!player)
		return false;

	achievements::TAchievementData* target_achievement;
	if (!GetAchievementInfo(achievement_id, &target_achievement))
		return 0;

	TAchievementsMap& player_achievements = *(player->GetAchievementInfo().get());
	const auto& achievement = player_achievements.find(achievement_id);
	if (achievement == player_achievements.end())
		return false;

	if (achievement->second.size() == 1 &&
		achievement->second.begin()->first == 0)
		return true;

	TTasksList player_tasks = achievement->second;
	for (const auto& task : player_tasks)
	{
		if (task.first != task_id)
			continue;

		const auto& T = target_achievement->tasks.find(task.first);
		if (T == target_achievement->tasks.end())
			continue;

		return IsTaskFinished(player, task.second, T->second);
	}

	return false;
}

uint32_t CAchievementSystem::GetTaskProgress(LPCHARACTER player, const uint64_t task_value, const achievements::TTask category_task)
{
	if (!player)
		return 0;

	if (player->GetLevel() >= category_task.max_level && category_task.max_level > 0)
		return 100;

	if (player->GetLevel() < category_task.min_level && category_task.min_level > 0)
		return 0;

	if (category_task.type == achievements::ETaskTypes::TYPE_SKILL)
	{
		if (!player->IsLearnableSkillAchievement(category_task.restrictions[achievements::ERestrictionTypes::RESTRICTION_VNUM]))
			return 0;
	}

	if (category_task.max_value <= task_value)
		return 100;

	return (task_value * 100) / category_task.max_value;
}

uint32_t CAchievementSystem::GetTotalAvailableTasks(LPCHARACTER player, uint32_t achievement_id)
{
	if (!player)
		return 0;

	achievements::TAchievementData* target_achievement;
	if (!GetAchievementInfo(achievement_id, &target_achievement))
		return 0;

	if (target_achievement->max_value > 0)
	{
		uint32_t num = 0;

		for (const auto& task : target_achievement->tasks)
		{
			if (task.second.type == achievements::ETaskTypes::TYPE_SKILL)
			{
				if (!player->IsLearnableSkillAchievement(task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_VNUM]))
					continue;
			}

			++num;
		}

		return num;
	}

	return target_achievement->tasks.size();
}

uint32_t CAchievementSystem::GetAchievementProgressFast(LPCHARACTER player, uint32_t achievement_id)
{
	if (!player)
		return 0;

	achievements::TAchievementData* target_achievement;
	if (!GetAchievementInfo(achievement_id, &target_achievement))
		return 0;

	TAchievementsMap& player_achievements = *(player->GetAchievementInfo().get());

	const auto& achievement = player_achievements.find(achievement_id);

	if (achievement == player_achievements.end())
		return 0;

	if (achievement->second.size() == 1 &&
		achievement->second.begin()->first == 0)
		return 100;

	return 0;
}

uint32_t CAchievementSystem::GetAchievementProgress(LPCHARACTER player, uint32_t achievement_id)
{
	if (!player)
		return 0;

	achievements::TAchievementData* target_achievement;
	if (!GetAchievementInfo(achievement_id, &target_achievement))
		return 0;

	TAchievementsMap& player_achievements = *(player->GetAchievementInfo().get());

	const auto& achievement = player_achievements.find(achievement_id);

	if (achievement == player_achievements.end())
		return 0;

	if (achievement->second.size() == 1 && achievement->second.begin()->first == 0)
		return 100;

	TTasksList player_tasks = achievement->second;

	uint32_t max_procent = GetTotalAvailableTasks(player, achievement_id) * 100;
	uint32_t tasks_num = 0;

	if (target_achievement->max_value > 0)
	{
		uint64_t current_value = 0;
		for (const auto& task : player_tasks)
		{
			const auto& cTask = target_achievement->tasks.find(task.first);
			if (cTask->second.type == achievements::ETaskTypes::TYPE_SKILL)
			{
				if (!player->IsLearnableSkillAchievement(cTask->second.restrictions[achievements::ERestrictionTypes::RESTRICTION_VNUM]))
					continue;
			}

			current_value += task.second;

		}

		if (current_value >= target_achievement->max_value)
			return 100;
		else
			return MIN(100, ((current_value * 100) / target_achievement->max_value));
	}

	for (const auto& task : player_tasks)
	{
		const auto& T = target_achievement->tasks.find(task.first);
		if (T == target_achievement->tasks.end())
			continue;

		tasks_num += GetTaskProgress(player, task.second, T->second);
	}

	if (tasks_num == 0 || max_procent == 0)
		return 0;

	return (100 * tasks_num) / max_procent;
}

void CAchievementSystem::RefreshAchievementTitles(LPCHARACTER player)
{
	if (!player)
		return;

	std::vector<uint32_t>& titles = player->GetAchievementTitles();
	titles.clear();

	for (const auto& achievement : _achievements)
	{
		if (GetAchievementProgressFast(player, achievement.first) != 100)
			continue;

		for (const auto& reward : achievement.second.rewards)
		{
			if (reward.type == achievements::ERewardTypes::REWARD_TITLE)
				titles.push_back(reward.value);
		}
	}
}

void CAchievementSystem::OnKill(LPCHARACTER player, LPCHARACTER victim, bool isDuel)
{
	if (!player || !victim)
		return;

	// Checking if the victim is a NPC (For NPC Kills).
	if (player->IsPC())
	{
		if (!player->GetAchievementInfo())
			return;

		TAchievementsMap& player_achievements = *(player->GetAchievementInfo().get());

		for (const auto& achievement : _achievements)
		{
			TAchievementsMap::iterator pAchievement = player_achievements.find(achievement.first);
			bool needUpdate = false;
			// Checking if the player has in his map the current achievement and insert it otherwise.
			if (pAchievement == player_achievements.end())
			{
				TTasksList tasks;
				for (const auto& task : achievement.second.tasks)
					tasks.insert({ task.first, 0 });

				pAchievement = player_achievements.insert({ achievement.first, tasks }).first;
			}
			else {
				// Checking if the player's achievement is already finished and skipping it if true.
				if (GetAchievementProgressFast(player, achievement.first) == 100) {
					continue;
				}
			}

			// Check if the there is a 'player kill' task in the achievement's task list.
			for (const auto& task : achievement.second.tasks)
			{
				// Checking if the task is of 'kill' type and skipping it if not.
				if (task.second.type != achievements::ETaskTypes::TYPE_KILL)
					continue;

				// Getting the player's current task data.
				TTasksList::iterator pTask =
					pAchievement->second.find(task.first);

				// Checking if the task is already finished and skipping it if true.
				if (IsTaskFinished(player, pTask->second, task.second)) {
					continue;
				}

				// Checking if the player fulfills the task's restrictions
				{
					// Checking if the player's level allows him to start the task.
					if (task.second.min_level > player->GetLevel())
						continue;

					// Checking if the player has a higher level than that allowed for the task and finishing the task if so.
					if (player->GetLevel() >= task.second.max_level && task.second.max_level > 0)
					{
						pTask->second = task.second.max_value;
						needUpdate = true;
						continue;
					}

					// Checking if 'only monsters' is enabled
					if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_MONSTERS] > 0 && !victim->IsMonster())
						continue;

					// Checking if a specific vnum for the NPC is set.
					if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_VNUM] > 0 && victim->GetRaceNum() != task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_VNUM])
						continue;

					// Checking if a specific map for the NPC to be in is set.
					if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_MAP_INDEX] > 0 && victim->GetMapIndex() !=task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_MAP_INDEX])
						continue;

					// Checking if 'only stones' is enabled
					if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_STONES] > 0 && !victim->IsStone())
						continue;

					// Checking if 'only bosses' is enabled
					if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_BOSS] > 0 && (victim->GetMobRank() < MOB_RANK_BOSS || victim->IsStone()))
						continue;

					// Checking if 'only players' is enabled
					if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_PLAYER] > 0 &&!victim->IsPC())
						continue;

					// Checking if 'only players from empire' is enabled
					if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_EMPIRE] > 0 && victim->GetEmpire() != task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_EMPIRE])
						continue;

					// Checking if 'only players with race' is enabled
					if (victim->IsPC()) {
						if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_RACE] > 0 && victim->GetRaceNum() != task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_RACE])
							continue;
					}
					else {
						if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_RACE] > 0 && !victim->IsRaceFlag(task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_RACE]))
							continue;
					}
					// Checking if 'only players with pk' is enabled
					if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_PK] > 0 && victim->GetPKMode() != task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_PK])
						continue;

					// Checking if 'only players from duel' is enabled
					if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_DUEL] > 0 && !isDuel)
						continue;

					if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_DUEL_ROW] > 0 && !isDuel)
						continue;
				}

				// Incrementing the task's value.
				++(pTask->second);
				needUpdate = true;
			}

			// Checking if the player's achievement has been finished.
			if (needUpdate && GetAchievementProgress(player, achievement.first) == 100)
			{
				FinishAchievement(player, achievement.first);
				needUpdate = false;
				sys_log(0, "Player %s has finished achievement %d", player->GetName(), achievement.first);
			}
			if (needUpdate)
				UpdatePacket(player, achievement.first);
		}
	}

	// Checking if the victim is a PC (For PC Death).
	if (victim->IsPC())
	{
		if (!victim->GetAchievementInfo())
			return;

		TAchievementsMap& player_achievements = *(victim->GetAchievementInfo().get());

		for (const auto& achievement : _achievements)
		{
			TAchievementsMap::iterator pAchievement = player_achievements.find(achievement.first);
			bool needUpdate = false;
			// Checking if the player has in his map the current achievement and insert it otherwise.
			if (pAchievement == player_achievements.end())
			{
				TTasksList tasks;
				for (const auto& task : achievement.second.tasks)
					tasks.insert({ task.first, 0 });

				pAchievement = player_achievements.insert({ achievement.first, tasks }).first;
			}
			else
			{
				// Checking if the player's achievement is already finished and skipping it if true.
				if (GetAchievementProgressFast(victim, achievement.first) == 100)
					continue;
			}

			// Check if the there is a 'player kill' task in the achievement's task list.
			for (const auto& task : achievement.second.tasks)
			{
				// Checking if the task is of 'kill' type and skipping it if not.
				if (task.second.type != achievements::ETaskTypes::TYPE_DIE)
					continue;

				// Getting the player's current task data.
				TTasksList::iterator pTask = pAchievement->second.find(task.first);

				// Checking if the task is already finished and skipping it if true.
				if (IsTaskFinished(player, pTask->second, task.second))
				{
					continue;
				}

				// Checking if the player fulfills the task's restrictions
				{
					// Checking if the player's level allows him to start the task.
					if (task.second.min_level > victim->GetLevel())
						continue;

					// Checking if the player has a higher level than that allowed for the task and finishing the task if so.
					if (victim->GetLevel() >= task.second.max_level && task.second.max_level > 0)
					{
						pTask->second = task.second.max_value;
						needUpdate = true;
						continue;
					}

					// Checking if 'only players' is enabled
					if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_PLAYER] > 0 && !player->IsPC())
						continue;

					// Checking if 'only players from empire' is enabled
					if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_EMPIRE] > 0 && player->GetEmpire() != task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_EMPIRE])
						continue;

					// Checking if 'only players with race' is enabled
					if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_RACE] > 0 && player->GetRaceNum() != task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_RACE])
						continue;

					// Checking if 'only players with pk' is enabled
					if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_PK] > 0 && player->GetPKMode() != task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_PK])
						continue;

					// Checking if 'only players from duel' is enabled
					if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_DUEL] > 0 && !isDuel)
						continue;

					if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_DUEL_ROW] > 0 && !isDuel)
						continue;
				}

				// Incrementing the task's value.
				if (task.second.restrictions[achievements::ERestrictionTypes::
					RESTRICTION_DUEL_ROW] > 0 &&
					isDuel)
					pTask->second = 0;
				else
					++(pTask->second);

				needUpdate = true;
			}

			// Checking if the player's achievement has been finished.
			if (needUpdate && GetAchievementProgress(victim, achievement.first) == 100)
			{
				FinishAchievement(victim, achievement.first);
				needUpdate = false;
				sys_log(0, "Player %s has finished achievement %d",
					victim->GetName(), achievement.first);
			}
			if (needUpdate)
				UpdatePacket(victim, achievement.first);
		}
	}
}

void CAchievementSystem::OnCharacterUpdate(LPCHARACTER player)
{
	if (!player)
		return;

	if (!player->GetAchievementInfo())
		return;

	TAchievementsMap& player_achievements = *(player->GetAchievementInfo().get());

	for (const auto& achievement : _achievements)
	{
		TAchievementsMap::iterator pAchievement = player_achievements.find(achievement.first);
		bool needUpdate = false;
		// Checking if the player has in his map the current achievement and insert it otherwise.
		if (pAchievement == player_achievements.end())
		{
			TTasksList tasks;
			for (const auto& task : achievement.second.tasks)
				tasks.insert({ task.first, 0 });

			pAchievement = player_achievements.insert({ achievement.first, tasks }).first;
		}
		else
		{
			// Checking if the player's achievement is already finished and skipping it if true.
			if (GetAchievementProgressFast(player, achievement.first) == 100)
			{
				continue;
			}
		}

		// Check if the there is a 'player kill' task in the achievement's task list.
		for (const auto& task : achievement.second.tasks)
		{
			// Checking if the task is of 'reach level' type and skipping it if not.
			if (task.second.type ==achievements::ETaskTypes::TYPE_REACH_LEVEL)
			{
				// Getting the player's current task data.
				TTasksList::iterator pTask = pAchievement->second.find(task.first);

				// Checking if the task is already finished and skipping it if true.
				if (IsTaskFinished(player, pTask->second, task.second))
				{
					continue;
				}

				// Checking if the player fulfills the task's restrictions
				{
					if (task.second.max_value <= player->GetLevel())
					{
						pTask->second = task.second.max_value;
						needUpdate = true;
					}
					else
					{
						pTask->second = player->GetLevel();
						needUpdate = true;
					}
				}
			}
			else if (task.second.type == achievements::ETaskTypes::TYPE_REACH_PLAYTIME)
			{
				// Getting the player's current task data.
				TTasksList::iterator pTask =
					pAchievement->second.find(task.first);

				// Checking if the task is already finished and skipping it if true.
				if (IsTaskFinished(player, pTask->second, task.second)) {
					continue;
				}

				// Checking if the player fulfills the task's restrictions
				{
					pTask->second = player->GetRealPoint(POINT_PLAYTIME) + (get_dword_time() - player->GetStartPlayTime()) / 6000;
					needUpdate = true;
				}
			}
			else if (task.second.type == achievements::ETaskTypes::TYPE_REACH_SPEED)
			{
				// Getting the player's current task data.
				TTasksList::iterator pTask =
					pAchievement->second.find(task.first);

				// Checking if the task is already finished and skipping it if true.
				if (IsTaskFinished(player, pTask->second, task.second)) {
					continue;
				}

				// Checking if the player fulfills the task's restrictions
				{
					pTask->second = player->GetPoint(POINT_MOV_SPEED);
					needUpdate = true;
				}
			}
		}

		// Checking if the player's achievement has been finished.
		if (needUpdate && GetAchievementProgress(player, achievement.first) == 100)
		{
			needUpdate = false;
			FinishAchievement(player, achievement.first);
			sys_log(0, "Player %s has finished achievement %d", player->GetName(), achievement.first);
		}

		if (player->IsPC() && needUpdate)
			UpdatePacket(player, achievement.first);
	}
}

void CAchievementSystem::OnSummon(LPCHARACTER player, uint8_t type, uint32_t value, uint32_t time, bool maxed)
{
	if (!player)
		return;

	if (!player->GetAchievementInfo())
		return;

	TAchievementsMap& player_achievements = *(player->GetAchievementInfo().get());

	for (const auto& achievement : _achievements)
	{
		TAchievementsMap::iterator pAchievement = player_achievements.find(achievement.first);
		bool needUpdate = false;
		// Checking if the player has in his map the current achievement and insert it otherwise.
		if (pAchievement == player_achievements.end())
		{
			TTasksList tasks;
			for (const auto& task : achievement.second.tasks)
				tasks.insert({ task.first, 0 });

			pAchievement = player_achievements.insert({ achievement.first, tasks }).first;
		}
		else
		{
			// Checking if the player's achievement is already finished and skipping it if true.
			if (GetAchievementProgressFast(player, achievement.first) == 100)
			{
				continue;
			}
		}
		// Check if the there is a 'player kill' task in the achievement's task list.
		for (const auto& task : achievement.second.tasks)
		{
			if (task.second.type != type)
				continue;

			// Getting the player's current task data.
			TTasksList::iterator pTask = pAchievement->second.find(task.first);

			// Checking if the task is already finished and skipping it if true.
			if (IsTaskFinished(player, pTask->second, task.second))
			{
				continue;
			}
			{
				{
					if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_VNUM] > 0 && task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_VNUM] != value)
						continue;

					if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_MAXED_BONUSES] > 0 && !maxed)
						continue;
				}

				if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_TIME] > 0)
				{
					pTask->second += time;
					needUpdate = true;
				}
				else
				{
					if (time == 0)
					{
						++pTask->second;
						needUpdate = true;
					}
				}
			}
		}

		// Checking if the player's achievement has been finished.
		if (needUpdate && GetAchievementProgress(player, achievement.first) == 100)
		{
			FinishAchievement(player, achievement.first);
			needUpdate = false;
			sys_log(0, "Player %s has finished achievement %d", player->GetName(), achievement.first);
		}

		if (player->IsPC() && needUpdate)
			UpdatePacket(player, achievement.first);
	}
}

void CAchievementSystem::OnToggle(LPCHARACTER player, uint32_t vnum)
{
	if (!player)
		return;

	if (!player->GetAchievementInfo())
		return;

	TAchievementsMap& player_achievements =
		*(player->GetAchievementInfo().get());

	for (const auto& achievement : _achievements)
	{
		TAchievementsMap::iterator pAchievement = player_achievements.find(achievement.first);
		bool needUpdate = false;
		// Checking if the player has in his map the current achievement and insert it otherwise.
		if (pAchievement == player_achievements.end())
		{
			TTasksList tasks;
			for (const auto& task : achievement.second.tasks)
				tasks.insert({ task.first, 0 });

			pAchievement = player_achievements.insert({ achievement.first, tasks }).first;
		}
		else
		{
			// Checking if the player's achievement is already finished and skipping it if true.
			if (GetAchievementProgressFast(player, achievement.first) == 100)
			{
				continue;
			}
		}

		// Check if the there is a 'player kill' task in the achievement's task list.
		for (const auto& task : achievement.second.tasks)
		{
			if (task.second.type != achievements::ETaskTypes::TYPE_ACTIVATE_TOGGLE)
				continue;

			// Getting the player's current task data.
			TTasksList::iterator pTask = pAchievement->second.find(task.first);

			// Checking if the task is already finished and skipping it if true.
			if (IsTaskFinished(player, pTask->second, task.second))
			{
				continue;
			}
			{
				if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_VNUM] > 0 && task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_VNUM] != vnum)
					continue;
			}

			++(pTask->second);
			needUpdate = true;
		}

		// Checking if the player's achievement has been finished.
		if (needUpdate && GetAchievementProgress(player, achievement.first) == 100)
		{
			FinishAchievement(player, achievement.first);
			needUpdate = false;
			sys_log(0, "Player %s has finished achievement %d", player->GetName(), achievement.first);
		}

		if (player->IsPC() && needUpdate)
			UpdatePacket(player, achievement.first);
	}
}

void CAchievementSystem::OnFishItem(LPCHARACTER player, uint8_t type, uint32_t vnum, uint32_t count)
{
	if (!player)
		return;

	if (!player->GetAchievementInfo())
		return;

	TAchievementsMap& player_achievements = *(player->GetAchievementInfo().get());

	for (const auto& achievement : _achievements)
	{
		TAchievementsMap::iterator pAchievement = player_achievements.find(achievement.first);
		bool needUpdate = false;
		// Checking if the player has in his map the current achievement and insert it otherwise.
		if (pAchievement == player_achievements.end())
		{
			TTasksList tasks;
			for (const auto& task : achievement.second.tasks)
				tasks.insert({ task.first, 0 });

			pAchievement = player_achievements.insert({ achievement.first, tasks }).first;
		}
		else
		{
			// Checking if the player's achievement is already finished and skipping it if true.
			if (GetAchievementProgressFast(player, achievement.first) == 100)
			{
				continue;
			}
		}

		// Check if the there is a 'player kill' task in the achievement's task list.
		for (const auto& task : achievement.second.tasks)
		{
			if (task.second.type != type)
				continue;

			// Getting the player's current task data.
			TTasksList::iterator pTask = pAchievement->second.find(task.first);

			// Checking if the task is already finished and skipping it if true.
			if (IsTaskFinished(player, pTask->second, task.second)) {
				continue;
			}
			if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_VNUM] > 0 && task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_VNUM] != vnum)
				continue;

			pTask->second += count;
			needUpdate = true;
		}

		// Checking if the player's achievement has been finished.
		if (needUpdate && GetAchievementProgress(player, achievement.first) == 100)
		{
			FinishAchievement(player, achievement.first);
			needUpdate = false;
			sys_log(0, "Player %s has finished achievement %d", player->GetName(), achievement.first);
		}
		if (player->IsPC() && needUpdate)
			UpdatePacket(player, achievement.first);
	}
}

void CAchievementSystem::OnWinGuildWar(LPCHARACTER player)
{
	if (!player)
		return;

	if (!player->GetAchievementInfo())
		return;

	TAchievementsMap& player_achievements = *(player->GetAchievementInfo().get());

	for (const auto& achievement : _achievements)
	{
		TAchievementsMap::iterator pAchievement = player_achievements.find(achievement.first);
		bool needUpdate = false;
		// Checking if the player has in his map the current achievement and insert it otherwise.
		if (pAchievement == player_achievements.end())
		{
			TTasksList tasks;
			for (const auto& task : achievement.second.tasks)
				tasks.insert({ task.first, 0 });

			pAchievement = player_achievements.insert({ achievement.first, tasks }).first;
		}
		else
		{
			// Checking if the player's achievement is already finished and skipping it if true.
			if (GetAchievementProgressFast(player, achievement.first) == 100) {
				continue;
			}
		}

		// Check if the there is a 'player kill' task in the achievement's task list.
		for (const auto& task : achievement.second.tasks)
		{
			if (task.second.type != achievements::ETaskTypes::TYPE_WIN_WARS)
				continue;

			// Getting the player's current task data.
			TTasksList::iterator pTask = pAchievement->second.find(task.first);

			// Checking if the task is already finished and skipping it if true.
			if (IsTaskFinished(player, pTask->second, task.second))
			{
				continue;
			}

			++(pTask->second);
			needUpdate = true;
		}

		// Checking if the player's achievement has been finished.
		if (needUpdate && GetAchievementProgress(player, achievement.first) == 100)
		{
			FinishAchievement(player, achievement.first);
			needUpdate = false;
			sys_log(0, "Player %s has finished achievement %d", player->GetName(), achievement.first);
		}
		if (player->IsPC() && needUpdate)
			UpdatePacket(player, achievement.first);
	}
}

void CAchievementSystem::DamageDealt(LPCHARACTER player, LPCHARACTER victim, uint32_t skill, uint32_t damage)
{
	if (!player || !victim)
		return;
#
	if (player->IsPC())
	{
		if (!player->GetAchievementInfo())
			return;

		TAchievementsMap& player_achievements = *(player->GetAchievementInfo().get());

		for (const auto& achievement : _achievements)
		{
			TAchievementsMap::iterator pAchievement = player_achievements.find(achievement.first);
			bool needUpdate = false;
			// Checking if the player has in his map the current achievement and insert it otherwise.
			if (pAchievement == player_achievements.end())
			{
				TTasksList tasks;
				for (const auto& task : achievement.second.tasks)
					tasks.insert({ task.first, 0 });

				pAchievement = player_achievements.insert({ achievement.first, tasks }).first;
			}
			else
			{
				// Checking if the player's achievement is already finished and skipping it if true.
				if (GetAchievementProgressFast(player, achievement.first) == 100)
				{
					continue;
				}
			}

			// Check if the there is a 'player kill' task in the achievement's task list.
			for (const auto& task : achievement.second.tasks)
			{
				if (task.second.type != achievements::ETaskTypes::TYPE_DEAL_DAMAGE)
					continue;

				// Getting the player's current task data.
				TTasksList::iterator pTask = pAchievement->second.find(task.first);

				// Checking if the task is already finished and skipping it if true.
				if (IsTaskFinished(player, pTask->second, task.second))
				{
					continue;
				}
				if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_VNUM] > 0 && task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_VNUM] != skill)
					continue;

				if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_PLAYER] > 0 && !victim->IsPC())
					continue;

				pTask->second = damage;
				needUpdate = true;
			}

			// Checking if the player's achievement has been finished.
			if (needUpdate &&
				GetAchievementProgress(player, achievement.first) == 100)
			{
				FinishAchievement(player, achievement.first);
				needUpdate = false;
				sys_log(0, "Player %s has finished achievement %d", player->GetName(), achievement.first);
			}
			if (needUpdate)
				UpdatePacket(player, achievement.first);
		}
	}
	if (victim->IsPC())
	{
		if (!victim->GetAchievementInfo())
			return;

		TAchievementsMap& player_achievements = *(victim->GetAchievementInfo().get());

		for (const auto& achievement : _achievements)
		{
			TAchievementsMap::iterator pAchievement = player_achievements.find(achievement.first);
			bool needUpdate = false;
			// Checking if the player has in his map the current achievement and insert it otherwise.
			if (pAchievement == player_achievements.end())
			{
				TTasksList tasks;
				for (const auto& task : achievement.second.tasks)
					tasks.insert({ task.first, 0 });

				pAchievement = player_achievements.insert({ achievement.first, tasks }).first;
			}
			else
			{
				// Checking if the player's achievement is already finished and skipping it if true.
				if (GetAchievementProgressFast(victim, achievement.first) == 100)
				{
					continue;
				}
			}

			// Check if the there is a 'player kill' task in the achievement's task list.
			for (const auto& task : achievement.second.tasks)
			{
				if (task.second.type != achievements::ETaskTypes::TYPE_GET_DAMAGAE)
					continue;
				// Getting the player's current task data.
				TTasksList::iterator pTask = pAchievement->second.find(task.first);

				// Checking if the task is already finished and skipping it if true.
				if (IsTaskFinished(victim, pTask->second, task.second))
				{
					continue;
				}
				if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_VNUM] > 0 && task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_VNUM] != skill)
					continue;

				if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_PLAYER] > 0 && !player->IsPC())
					continue;

				pTask->second = damage;
				needUpdate = true;
			}

			// Checking if the player's achievement has been finished.
			if (needUpdate &&
				GetAchievementProgress(victim, achievement.first) == 100)
			{
				FinishAchievement(victim, achievement.first);
				needUpdate = false;
				sys_log(0, "Player %s has finished achievement %d", victim->GetName(), achievement.first);
			}
			if (needUpdate)
				UpdatePacket(victim, achievement.first);
		}
	}
}

void CAchievementSystem::Collect(LPCHARACTER player, uint8_t type, uint32_t value, uint32_t count)
{
	if (!player)
		return;

	if (!player->GetAchievementInfo())
		return;

	TAchievementsMap& player_achievements =
		*(player->GetAchievementInfo().get());

	for (const auto& achievement : _achievements)
	{
		TAchievementsMap::iterator pAchievement = player_achievements.find(achievement.first);
		bool needUpdate = false;
		// Checking if the player has in his map the current achievement and insert it otherwise.
		if (pAchievement == player_achievements.end())
		{
			TTasksList tasks;
			for (const auto& task : achievement.second.tasks)
				tasks.insert({ task.first, 0 });

			pAchievement = player_achievements.insert({ achievement.first, tasks }).first;
		}
		else
		{
			// Checking if the player's achievement is already finished and skipping it if true.
			if (GetAchievementProgressFast(player, achievement.first) == 100)
			{
				continue;
			}
		}

		// Check if the there is a 'player kill' task in the achievement's task list.
		for (const auto& task : achievement.second.tasks)
		{
			if (task.second.type != type)
				continue;

			// Getting the player's current task data.
			TTasksList::iterator pTask = pAchievement->second.find(task.first);

			// Checking if the task is already finished and skipping it if true.
			if (IsTaskFinished(player, pTask->second, task.second)) {
				continue;
			}
			if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_VNUM] > 0 && task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_VNUM] != value)
				continue;

			if (task.second.max_level > 0 && task.second.max_level < count)
				continue;

			if (type == achievements::ETaskTypes::TYPE_COLLECT_ALIGNMENT || type == achievements::ETaskTypes::TYPE_COLLECT_GOLD)
			{
				(pTask->second) += value;
				needUpdate = true;
			}
			else
			{
				++(pTask->second);
				needUpdate = true;
			}
		}
		// Checking if the player's achievement has been finished.
		if (needUpdate &&
			GetAchievementProgress(player, achievement.first) == 100)
		{
			FinishAchievement(player, achievement.first);
			needUpdate = false;
			sys_log(0, "Player %s has finished achievement %d", player->GetName(), achievement.first);
		}
		if (needUpdate)
			UpdatePacket(player, achievement.first);
	}
}

void CAchievementSystem::OnGoldChange(LPCHARACTER player, uint8_t type, uint32_t value)
{
	if (!player)
		return;

	if (!player->GetAchievementInfo())
		return;

	TAchievementsMap& player_achievements = *(player->GetAchievementInfo().get());

	for (const auto& achievement : _achievements)
	{
		TAchievementsMap::iterator pAchievement = player_achievements.find(achievement.first);
		bool needUpdate = false;
		// Checking if the player has in his map the current achievement and insert it otherwise.
		if (pAchievement == player_achievements.end())
		{
			TTasksList tasks;
			for (const auto& task : achievement.second.tasks)
				tasks.insert({ task.first, 0 });

			pAchievement = player_achievements.insert({ achievement.first, tasks }).first;
		}
		else
		{
			// Checking if the player's achievement is already finished and skipping it if true.
			if (GetAchievementProgressFast(player, achievement.first) == 100)
			{
				continue;
			}
		}

		// Check if the there is a 'player kill' task in the achievement's task list.
		for (const auto& task : achievement.second.tasks)
		{
			if (task.second.type != type)
				continue;

			// Getting the player's current task data.
			TTasksList::iterator pTask = pAchievement->second.find(task.first);

			// Checking if the task is already finished and skipping it if true.
			if (IsTaskFinished(player, pTask->second, task.second)) {
				continue;
			}
			if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_VNUM] > 0 && task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_VNUM] != value)
				continue;

			pTask->second += value;
			needUpdate = true;
		}

		// Checking if the player's achievement has been finished.
		if (needUpdate &&
			GetAchievementProgress(player, achievement.first) == 100)
		{
			FinishAchievement(player, achievement.first);
			needUpdate = false;
			sys_log(0, "Player %s has finished achievement %d", player->GetName(), achievement.first);
		}
		if (needUpdate)
			UpdatePacket(player, achievement.first);
	}
}

void CAchievementSystem::OnTrade(LPCHARACTER player)
{
	if (!player)
		return;

	if (!player->GetAchievementInfo())
		return;

	TAchievementsMap& player_achievements = *(player->GetAchievementInfo().get());

	for (const auto& achievement : _achievements)
	{
		TAchievementsMap::iterator pAchievement = player_achievements.find(achievement.first);
		bool needUpdate = false;
		// Checking if the player has in his map the current achievement and insert it otherwise.
		if (pAchievement == player_achievements.end())
		{
			TTasksList tasks;
			for (const auto& task : achievement.second.tasks)
				tasks.insert({ task.first, 0 });

			pAchievement = player_achievements.insert({ achievement.first, tasks }).first;
		}
		else
		{
			// Checking if the player's achievement is already finished and skipping it if true.
			if (GetAchievementProgressFast(player, achievement.first) == 100)
			{
				continue;
			}
		}

		// Check if the there is a 'player kill' task in the achievement's task list.
		for (const auto& task : achievement.second.tasks)
		{
			if (task.second.type != achievements::ETaskTypes::TYPE_TRADE)
				continue;

			// Getting the player's current task data.
			TTasksList::iterator pTask = pAchievement->second.find(task.first);

			// Checking if the task is already finished and skipping it if true.
			if (IsTaskFinished(player, pTask->second, task.second))
			{
				continue;
			}

			++pTask->second;
			needUpdate = true;
		}

		// Checking if the player's achievement has been finished.
		if (needUpdate && GetAchievementProgress(player, achievement.first) == 100)
		{
			FinishAchievement(player, achievement.first);
			needUpdate = false;
			sys_log(0, "Player %s has finished achievement %d", player->GetName(), achievement.first);
		}
		if (needUpdate)
			UpdatePacket(player, achievement.first);
	}
}

void CAchievementSystem::OnUpgrade(LPCHARACTER player, uint32_t vnum, bool _to15)
{
	if (!player)
		return;

	if (!player->GetAchievementInfo())
		return;

	TAchievementsMap& player_achievements =
		*(player->GetAchievementInfo().get());

	for (const auto& achievement : _achievements)
	{
		TAchievementsMap::iterator pAchievement = player_achievements.find(achievement.first);
		bool needUpdate = false;
		// Checking if the player has in his map the current achievement and insert it otherwise.
		if (pAchievement == player_achievements.end())
		{
			TTasksList tasks;
			for (const auto& task : achievement.second.tasks)
				tasks.insert({ task.first, 0 });

			pAchievement = player_achievements.insert({ achievement.first, tasks }).first;
		}
		else
		{
			// Checking if the player's achievement is already finished and skipping it if true.
			if (GetAchievementProgressFast(player, achievement.first) == 100)
			{
				continue;
			}
		}

		// Check if the there is a 'player kill' task in the achievement's task list.
		for (const auto& task : achievement.second.tasks)
		{
			if (task.second.type != achievements::ETaskTypes::TYPE_UPGRADE_9 && task.second.type != achievements::ETaskTypes::TYPE_UPGRADE_15)
				continue;

			// Getting the player's current task data.
			TTasksList::iterator pTask = pAchievement->second.find(task.first);

			// Checking if the task is already finished and skipping it if true.
			if (IsTaskFinished(player, pTask->second, task.second))
			{
				continue;
			}
			if (task.second.type == achievements::ETaskTypes::TYPE_UPGRADE_9 && _to15)
				continue;

			if (task.second.type == achievements::ETaskTypes::TYPE_UPGRADE_15 && !_to15)
				continue;

			if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_VNUM] > 0 && task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_VNUM] != vnum)
				continue;

			++pTask->second;
			needUpdate = true;
		}

		// Checking if the player's achievement has been finished.
		if (needUpdate &&GetAchievementProgress(player, achievement.first) == 100)
		{
			FinishAchievement(player, achievement.first);
			needUpdate = false;
			sys_log(0, "Player %s has finished achievement %d", player->GetName(), achievement.first);
		}
		if (needUpdate)
			UpdatePacket(player, achievement.first);
	}
}

void CAchievementSystem::OnSocial(LPCHARACTER player, uint8_t type)
{
	if (!player)
		return;

	if (!player->GetAchievementInfo())
		return;

	TAchievementsMap& player_achievements = *(player->GetAchievementInfo().get());

	for (const auto& achievement : _achievements)
	{
		TAchievementsMap::iterator pAchievement = player_achievements.find(achievement.first);
		bool needUpdate = false;
		// Checking if the player has in his map the current achievement and insert it otherwise.
		if (pAchievement == player_achievements.end())
		{
			TTasksList tasks;
			for (const auto& task : achievement.second.tasks)
				tasks.insert({ task.first, 0 });

			pAchievement = player_achievements.insert({ achievement.first, tasks }).first;
		}
		else
		{
			// Checking if the player's achievement is already finished and skipping it if true.
			if (GetAchievementProgressFast(player, achievement.first) == 100)
			{
				continue;
			}
		}

		// Check if the there is a 'player kill' task in the achievement's task list.
		for (const auto& task : achievement.second.tasks)
		{
			if (task.second.type != type)
				continue;
			// Getting the player's current task data.
			TTasksList::iterator pTask = pAchievement->second.find(task.first);

			// Checking if the task is already finished and skipping it if true.
			if (IsTaskFinished(player, pTask->second, task.second))
			{
				continue;
			}
			if (task.second.max_level != 0 &&
				player->GetLevel() >= task.second.max_level)
				continue;

			if (task.second.min_level != 0 &&
				player->GetLevel() < task.second.min_level)
				continue;

			++pTask->second;
			needUpdate = true;
		}
		// Checking if the player's achievement has been finished.
		if (needUpdate &&
			GetAchievementProgress(player, achievement.first) == 100)
		{
			FinishAchievement(player, achievement.first);
			needUpdate = false;
			sys_log(0, "Player %s has finished achievement %d", player->GetName(), achievement.first);
		}
		if (needUpdate)
			UpdatePacket(player, achievement.first);
	}
}

void CAchievementSystem::SendTasksInfo(LPCHARACTER player, uint32_t achievement)
{
	if (!player)
		return;

	if (!player->GetAchievementInfo())
		return;

	TAchievementsMap& player_achievements = *(player->GetAchievementInfo().get());

	for (const auto& ach : player_achievements)
	{
		if (ach.first == achievement)
		{
			for (const auto& task : ach.second)
			{
				player->ChatPacket(CHAT_TYPE_INFO, "Task %d value %lld",
					task.first, task.second);
			}
			return;
		}
	}
}

void CAchievementSystem::OnMasterSkill(LPCHARACTER player, uint32_t skill, uint32_t value)
{
	if (!player)
		return;

	if (!player->GetAchievementInfo())
		return;

	TAchievementsMap& player_achievements =
		*(player->GetAchievementInfo().get());

	for (const auto& achievement : _achievements)
	{
		TAchievementsMap::iterator pAchievement = player_achievements.find(achievement.first);
		bool needUpdate = false;
		// Checking if the player has in his map the current achievement and insert it otherwise.
		if (pAchievement == player_achievements.end())
		{
			TTasksList tasks;
			for (const auto& task : achievement.second.tasks)
				tasks.insert({ task.first, 0 });

			pAchievement = player_achievements.insert({ achievement.first, tasks }).first;
		}
		else
		{
			// Checking if the player's achievement is already finished and skipping it if true.
			if (GetAchievementProgressFast(player, achievement.first) == 100)
			{
				continue;
			}
		}

		// Check if the there is a 'player kill' task in the achievement's task list.
		for (const auto& task : achievement.second.tasks)
		{
			// Checking if the task is already finished and skipping it if true.
			if (task.second.type != achievements::ETaskTypes::TYPE_SKILL)
				continue;

			// Getting the player's current task data.
			TTasksList::iterator pTask = pAchievement->second.find(task.first);

			// Checking if the task is already finished and skipping it if true.
			if (IsTaskFinished(player, pTask->second, task.second))
				continue;

			if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_VNUM] != skill)
				continue;

			if (pTask->second > value)
				continue;

			pTask->second = value;
			needUpdate = true;
		}

		// Checking if the player's achievement has been finished.
		if (needUpdate && GetAchievementProgress(player, achievement.first) == 100)
		{
			FinishAchievement(player, achievement.first);
			needUpdate = false;
			sys_log(0, "Player %s has finished achievement %d", player->GetName(), achievement.first);
		}
		if (needUpdate)
			UpdatePacket(player, achievement.first);
	}
}

void CAchievementSystem::OnFinishDungeon(LPCHARACTER player, uint32_t dungeon_id, uint32_t time, uint32_t members)
{
	if (!player)
		return;

	if (!player->GetAchievementInfo())
		return;

	TAchievementsMap& player_achievements =
		*(player->GetAchievementInfo().get());

	for (const auto& achievement : _achievements)
	{
		TAchievementsMap::iterator pAchievement = player_achievements.find(achievement.first);
		bool needUpdate = false;
		// Checking if the player has in his map the current achievement and insert it otherwise.
		if (pAchievement == player_achievements.end())
		{
			TTasksList tasks;
			for (const auto& task : achievement.second.tasks)
				tasks.insert({ task.first, 0 });

			pAchievement = player_achievements.insert({ achievement.first, tasks }).first;
		}
		else
		{
			// Checking if the player's achievement is already finished and skipping it if true.
			if (GetAchievementProgressFast(player, achievement.first) == 100)
			{
				continue;
			}
		}

		// Check if the there is a 'player kill' task in the achievement's task list.
		for (const auto& task : achievement.second.tasks)
		{
			if (task.second.type != achievements::ETaskTypes::TYPE_DUNGEON)
				continue;

			// Getting the player's current task data.
			TTasksList::iterator pTask = pAchievement->second.find(task.first);

			// Checking if the task is already finished and skipping it if true.
			if (IsTaskFinished(player, pTask->second, task.second))
			{
				continue;
			}
			if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_DUNGEON] > 0 && task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_DUNGEON] != dungeon_id)
				continue;

			if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_TIME] > 0 && task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_TIME] < time)
				continue;

			if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_PARTY] > 0 && task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_PARTY] != members)
				continue;

			++(pTask->second);
			needUpdate = true;
		}

		// Checking if the player's achievement has been finished.
		if (needUpdate && GetAchievementProgress(player, achievement.first) == 100)
		{
			FinishAchievement(player, achievement.first);
			needUpdate = false;
			sys_log(0, "Player %s has finished achievement %d", player->GetName(), achievement.first);
		}
		if (needUpdate)
			UpdatePacket(player, achievement.first);
	}
}

void CAchievementSystem::OnVisitMap(LPCHARACTER player)
{
	if (!player)
		return;

	if (!player->GetAchievementInfo())
		return;

	TAchievementsMap& achievements = *(player->GetAchievementInfo().get());

	for (const auto& achievement : _achievements)
	{
		TAchievementsMap::iterator pAchievement = achievements.find(achievement.first);
		bool needUpdate = false;
		// Checking if the player has in his map the current achievement and insert it otherwise.
		if (pAchievement == achievements.end())
		{
			TTasksList tasks;
			for (const auto& task : achievement.second.tasks)
				tasks.insert({ task.first, 0 });

			pAchievement = achievements.insert({ achievement.first, tasks }).first;
		}
		else
		{
			// Checking if the player's achievement is already finished and skipping it if true.
			if (GetAchievementProgressFast(player, achievement.first) == 100)
			{
				continue;
			}
		}

		// Check if the there is a 'player kill' task in the achievement's task list.
		for (const auto& task : achievement.second.tasks)
		{
			if (task.second.type != achievements::ETaskTypes::TYPE_EXPLORE)
				continue;

			// Getting the player's current task data.
			TTasksList::iterator pTask = pAchievement->second.find(task.first);

			// Checking if the task is already finished and skipping it if true.
			if (IsTaskFinished(player, pTask->second, task.second))
			{
				continue;
			}
			if (task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_MAP_INDEX] > 0 && task.second.restrictions[achievements::ERestrictionTypes::RESTRICTION_MAP_INDEX] != player->GetMapIndex())
				continue;

			if (task.second.min_level != 0 && player->GetLevel() < task.second.min_level)
				continue;

			if (task.second.max_level != 0 && player->GetLevel() > task.second.max_level)
			{
				pTask->second = task.second.max_value;
				needUpdate = true;
				continue;
			}

			++(pTask->second);
			needUpdate = true;
		}

		// Checking if the player's achievement has been finished.
		if (needUpdate && GetAchievementProgress(player, achievement.first) == 100)
		{
			FinishAchievement(player, achievement.first);
			needUpdate = false;
			sys_log(0, "Player %s has finished achievement %d", player->GetName(), achievement.first);
		}
		if (needUpdate)
			UpdatePacket(player, achievement.first);
	}
}

void CAchievementSystem::UpdatePacket(LPCHARACTER player, uint32_t achievement_id)
{
	if (!player)
		return;

	achievements::GC_packet packet = { HEADER_GC_ACHIEVEMENT, 0, achievements::HEADER_GC_UPDATE };
	achievements::GC_Update subpacket = {};

	TAchievementsMap& achievement = *(player->GetAchievementInfo().get());

	const TAchievementsMap::iterator& update_achievement = achievement.find(achievement_id);

	if (update_achievement == achievement.end())
	{
		sys_err("Trying to update achievement id %d but non existing?", achievement_id);
		return;
	}
	subpacket.id = achievement_id;
	const auto& task = update_achievement->second.find(0);
	if (update_achievement->second.size() == 1 && task != update_achievement->second.end())
	{
		subpacket.achievement.status = achievements::EAchievementStatus::FINISHED;
		subpacket.achievement.percentage = 100;
		subpacket.achievement.finish_timestamp = task->second;
	}
	else
	{
		subpacket.achievement.percentage = static_cast<uint8_t>(GetAchievementProgress(player, achievement_id));
		subpacket.achievement.finish_timestamp = 0;

		if (update_achievement->second.size() == 0 && subpacket.achievement.percentage == 0)
			subpacket.achievement.status = achievements::EAchievementStatus::LOCKED;
		else
			subpacket.achievement.status = achievements::EAchievementStatus::UNLOCKED;
	}
	packet.size += sizeof(achievements::GC_packet);
	packet.size += sizeof(achievements::GC_Update);
	TEMP_BUFFER buff;
	buff.write(&packet, sizeof(packet));
	buff.write(&subpacket, sizeof(subpacket));
	player->GetDesc()->BufferedPacket(buff.read_peek(), buff.size());
}

void CAchievementSystem::FinishAchievement(LPCHARACTER player, uint32_t achievement_id)
{
	if (!player)
		return;

	if (!player->GetAchievementInfo())
		return;

	TAchievementsMap& achievement = *(player->GetAchievementInfo().get());
	TAchievementsMap::iterator pAchievement = achievement.find(achievement_id);

	if (pAchievement == achievement.end())
	{
		sys_err("Force finish achievement, achievement does not exist.");
		return;
	}

	pAchievement->second.clear();
	pAchievement->second.insert({ 0, get_global_time() });

	UpdatePacket(player, achievement_id);

	achievements::GC_packet packet = { HEADER_GC_ACHIEVEMENT, 0, achievements::HEADER_GC_NOTIFICATION };

	packet.size += sizeof(achievements::GC_packet);
	packet.size += sizeof(uint32_t);
	packet.size += sizeof(uint8_t);
	TEMP_BUFFER buff;
	buff.write(&packet, sizeof(packet));
	buff.write(&achievement_id, sizeof(uint32_t));
	uint8_t type = 2;
	buff.write(&type, sizeof(uint8_t));

	player->GetDesc()->BufferedPacket(buff.read_peek(), buff.size());
	RewardPlayer(player, achievement_id);
}

bool CAchievementSystem::IsAchievementFinished(LPCHARACTER player, uint32_t achievement_id)
{
	if (!player)
		return false;

	if (!player->GetAchievementInfo())
		return false;

	TAchievementsMap& achievement = *(player->GetAchievementInfo().get());
	TAchievementsMap::iterator pAchievement = achievement.find(achievement_id);

	if (pAchievement == achievement.end())
		return false;

	if (GetAchievementProgress(player, achievement_id) == 100)
		return true;

	return false;

}
void CAchievementSystem::FinishAchievementTask(LPCHARACTER player, uint32_t achievement_id, uint32_t task_id)
{
	if (!player)
		return;

	if (!player->GetAchievementInfo())
		return;

	TAchievementsMap& achievement = *(player->GetAchievementInfo().get());
	TAchievementsMap::iterator pAchievement = achievement.find(achievement_id);

	if (pAchievement == achievement.end())
	{
		sys_err("Force finish task, achievement does not exist.");
		return;
	}

	achievements::TAchievementData* target_achievement;
	if (!GetAchievementInfo(achievement_id, &target_achievement))
	{
		sys_err("Force finish task, achievement does not exist.");
		return;
	}

	if (GetAchievementProgress(player, achievement_id) == 100)
		return;

	uint32_t max_value = 0;

	for (const auto& task : target_achievement->tasks)
	{
		if (task.first == task_id)
		{
			max_value = task.second.max_value;
			break;
		}
	}

	for (auto& task : pAchievement->second)
	{
		if (task.first == task_id)
		{
			task.second = max_value;
			break;
		}
	}

	if (GetAchievementProgress(player, achievement_id) == 100)
	{
		FinishAchievement(player, achievement_id);
		sys_log(0, "Player %s has finished achievement %d", player->GetName(), achievement_id);
		return;
	}

	UpdatePacket(player, achievement_id);
	sys_log(0, "Player %s finished task %d from achievement %d", player->GetName(), task_id, achievement_id);
}

void CAchievementSystem::UpdateTitleList(LPCHARACTER player)
{
	if (!player)
		return;

	const std::vector<uint32_t> titles = player->GetAchievementTitles();

	achievements::GC_packet packet = { HEADER_GC_ACHIEVEMENT, 0, achievements::HEADER_GC_UPDATE_TITLES };

	uint32_t title_size = titles.size();

	TEMP_BUFFER buff;
	packet.size += sizeof(achievements::GC_packet);
	packet.size += sizeof(uint32_t);
	packet.size += sizeof(uint32_t) * title_size;

	buff.write(&packet, sizeof(packet));
	buff.write(&title_size, sizeof(uint32_t));

	for (int i = 0; i < title_size; ++i)
	{
		uint32_t title = titles[i];
		buff.write(&title, sizeof(uint32_t));
	}

	player->GetDesc()->BufferedPacket(buff.read_peek(), buff.size());
}

void CAchievementSystem::UpdateAchievementPoints(LPCHARACTER player)
{
	if (!player)
		return;

	achievements::GC_packet packet = { HEADER_GC_ACHIEVEMENT, 0, achievements::HEADER_GC_UPDATE_POINTS };

	uint32_t achievement_points = player->GetAchievementPoints();
	packet.size += sizeof(achievements::GC_packet);
	packet.size += sizeof(uint32_t);

	TEMP_BUFFER buff;
	buff.write(&packet, sizeof(packet));
	buff.write(&achievement_points, sizeof(uint32_t));
	player->GetDesc()->BufferedPacket(buff.read_peek(), buff.size());
}

void CAchievementSystem::ChangeAchievementPoints(LPCHARACTER player, int32_t value)
{
	if (!player)
		return;

	if (value > 0)
	{
		player->SetAchievementPoints(player->GetAchievementPoints() + value);
	}
	else
	{
		if (player->GetAchievementPoints() < (-value))
		{
			sys_err("[Achievement]: Trying to charge player %s with %d achievement points, but he doesn't have enough?", player->GetName(), -value);
			player->SetAchievementPoints(0);
		}
		else player->SetAchievementPoints(player->GetAchievementPoints() + value);
	}
	UpdateAchievementPoints(player);
}

void CAchievementSystem::RewardPlayer(LPCHARACTER player, uint32_t achievement_id)
{
	if (!player)
		return;

	achievements::TAchievementData* achievement;
	if (!GetAchievementInfo(achievement_id, &achievement))
	{
		sys_err("Reward player for inexistent achievement");
		return;
	}

	auto& titles = player->GetAchievementTitles();
	bool bMustCompute = false;

	for (const auto& reward : achievement->rewards)
	{
		if (reward.type == achievements::ERewardTypes::REWARD_ITEM)
		{
			player->AutoGiveItem(reward.value, reward.count);
		}
		else if (reward.type == achievements::ERewardTypes::REWARD_GOLD)
		{
			player->PointChange(POINT_GOLD, reward.value);
		}
		else if (reward.type == achievements::ERewardTypes::REWARD_TITLE)
		{
			titles.push_back(reward.value);
			UpdateTitleList(player);
			bMustCompute = true;
		}
		else if (reward.type == achievements::ERewardTypes::REWARD_ACHIEVEMENT_POINTS)
		{
			ChangeAchievementPoints(player, reward.value);
		}
	}

	if (bMustCompute)
		player->ComputePoints();
}

bool CAchievementSystem::GetTitleInfo(uint32_t title_id, uint8_t& type, uint32_t& value)
{
	const auto& it = _titles.find(title_id);
	if (it == _titles.end())
		return false;

	type = it->second.first;
	value = it->second.second;

	return true;
}

void CAchievementSystem::ComputePoints(LPCHARACTER player)
{
	if (!player)
		return;

	if (!player->GetAchievementInfo())
		return;

	TAchievementsMap& player_achievements = *(player->GetAchievementInfo().get());

	const std::vector<uint32_t> titles = player->GetAchievementTitles();
	for (const auto& title : titles)
	{
		uint8_t type;
		uint32_t value;
		if (!GetTitleInfo(title, type, value) || type == 0 || value == 0)
			continue;

		player->PointChange(aApplyInfo[type].wPointType, value);
	}
}
#endif
