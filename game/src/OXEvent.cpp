#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "questmanager.h"
#include "start_position.h"
#include "packet.h"
#include "buffer_manager.h"
#include "log.h"
#include "char.h"
#include "char_manager.h"
#include "OXEvent.h"
#include "desc.h"
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
#include "LocaleNotice.hpp"
#endif

bool COXEventManager::Initialize()
{
	m_timedEvent = nullptr;
	m_map_char.clear();
	m_map_attender.clear();
#ifndef ENABLE_EVENT_MANAGER
	m_vec_quiz.clear();
#endif

	SetStatus(OXEVENT_FINISH);

	return true;
}

void COXEventManager::Destroy()
{
	CloseEvent();

	m_map_char.clear();
	m_map_attender.clear();
	m_vec_quiz.clear();

	SetStatus(OXEVENT_FINISH);
}

OXEventStatus COXEventManager::GetStatus()
{
	uint8_t ret = static_cast<uint8_t>(quest::CQuestManager::Instance().GetEventFlag("oxevent_status"));

	switch (ret)
	{
		case 0:
			return OXEVENT_FINISH;

		case 1:
			return OXEVENT_OPEN;

		case 2:
			return OXEVENT_CLOSE;

		case 3:
			return OXEVENT_QUIZ;

		default:
			return OXEVENT_ERR;
	}

	return OXEVENT_ERR;
}

void COXEventManager::SetStatus(OXEventStatus status)
{
	uint8_t val = 0;

	switch (status)
	{
		case OXEVENT_OPEN:
			val = 1;
			break;

		case OXEVENT_CLOSE:
			val = 2;
			break;

		case OXEVENT_QUIZ:
			val = 3;
			break;

		case OXEVENT_FINISH:
		case OXEVENT_ERR:
		default:
			val = 0;
			break;
	}
	quest::CQuestManager::Instance().RequestSetEventFlag("oxevent_status", val);
}

bool COXEventManager::Enter(LPCHARACTER pkChar)
{
	if (!pkChar)
		return false;

	if (GetStatus() == OXEVENT_FINISH)
	{
		sys_log(0, "OXEVENT : map finished. but char enter. %s", pkChar->GetName());
		return false;
	}

	const PIXEL_POSITION pos = pkChar->GetXYZ();

	if (pos.x == 896500 && pos.y == 24600)
	{
		return EnterAttender(pkChar);
	}
	else if (pos.x == 896300 && pos.y == 28900)
	{
		return EnterAudience(pkChar);
	}
	else
	{
		sys_log(0, "OXEVENT : wrong pos enter %d %d", pos.x, pos.y);
		return false;
	}

	return false;
}

bool COXEventManager::EnterAttender(LPCHARACTER pkChar)
{
	if (!pkChar)
		return false;

	uint32_t pid = pkChar->GetPlayerID();

	m_map_char.insert(std::make_pair(pid, pid));
	m_map_attender.insert(std::make_pair(pid, pid));

	return true;
}

bool COXEventManager::EnterAudience(LPCHARACTER pkChar)
{
	if (!pkChar)
		return false;

	uint32_t pid = pkChar->GetPlayerID();

	m_map_char.insert(std::make_pair(pid, pid));

	return true;
}

bool COXEventManager::AddQuiz(uint8_t level, const char* pszQuestion, bool answer)
{
	if (m_vec_quiz.size() < (size_t)level + 1)
		m_vec_quiz.resize(level + 1);

	struct tag_Quiz tmpQuiz;

	tmpQuiz.level = level;
	strlcpy(tmpQuiz.Quiz, pszQuestion, sizeof(tmpQuiz.Quiz));
	tmpQuiz.answer = answer;

	m_vec_quiz[level].emplace_back(tmpQuiz);
	return true;
}

bool COXEventManager::ShowQuizList(LPCHARACTER pkChar)
{
	if (!pkChar)
		return false;

	int c = 0;

	for (size_t i = 0; i < m_vec_quiz.size(); ++i)
	{
		for (size_t j = 0; j < m_vec_quiz[i].size(); ++j, ++c)
		{
			pkChar->ChatPacket(CHAT_TYPE_INFO, "%d %s %s", m_vec_quiz[i][j].level, m_vec_quiz[i][j].Quiz, m_vec_quiz[i][j].answer ? LC_TEXT("[858]TRUE") : LC_TEXT("[869]FALSE"));
		}
	}

	pkChar->ChatPacket(CHAT_TYPE_INFO, "[LS;880;%d]", c);
	return true;
}

void COXEventManager::ClearQuiz()
{
	for (uint32_t i = 0; i < m_vec_quiz.size(); ++i)
	{
		m_vec_quiz[i].clear();
	}

	m_vec_quiz.clear();
}

EVENTINFO(OXEventInfoData)
{
	bool answer;

	OXEventInfoData()
		: answer(false)
	{
	}
};

EVENTFUNC(oxevent_timer)
{
	static uint8_t flag = 0;
	const OXEventInfoData* info = dynamic_cast<OXEventInfoData*>(event->info);

	if (info == nullptr)
	{
		sys_err("oxevent_timer> <Factor> Null pointer");
		return 0;
	}

	switch (flag)
	{
	case 0:
#ifndef ENABLE_OX_RENEWAL
		SendNoticeMap(LC_TEXT("[892]The result will follow in 10 seconds."), MAP_OXEVENT, true);
#endif
		flag++;
#ifdef ENABLE_OX_RENEWAL
		return PASSES_PER_SEC(5);
#else
		return PASSES_PER_SEC(10);
#endif

	case 1:
#ifdef ENABLE_OX_RENEWAL
		SendNoticeOxMap("#start", MAP_OXEVENT);
		SendNoticeOxMap(LC_TEXT("[903]The correct answer is:"), MAP_OXEVENT);
#else
		SendNoticeMap(LC_TEXT("[903]The correct answer is:"), MAP_OXEVENT, true);
#endif

		if (info->answer == true)
		{
			COXEventManager::Instance().CheckAnswer(true);
#ifdef ENABLE_OX_RENEWAL
			SendNoticeOxMap(LC_TEXT("[914]Yes (O)"), MAP_OXEVENT);
#else
			SendNoticeMap(LC_TEXT("[914]Yes (O)"), MAP_OXEVENT, true);
#endif
		}
		else
		{
			COXEventManager::Instance().CheckAnswer(false);
#ifdef ENABLE_OX_RENEWAL
			SendNoticeOxMap(LC_TEXT("[925]No (X)"), MAP_OXEVENT);
#else
			SendNoticeMap(LC_TEXT("[925]No (X)"), MAP_OXEVENT, true);
#endif
		}

#ifdef ENABLE_OX_RENEWAL
		SendNoticeOxMap(LC_TEXT("[936]In 5 sec. everyone who gave an incorrect answer will be removed."), MAP_OXEVENT);
		SendNoticeOxMap(LC_TEXT("[947]Ready for the next question?"), MAP_OXEVENT);
		SendNoticeOxMap("#send", MAP_OXEVENT);
#else
		SendNoticeMap(LC_TEXT("[936]In 5 sec. everyone who gave an incorrect answer will be removed."), MAP_OXEVENT, true);
#endif

		flag++;
#ifdef ENABLE_OX_RENEWAL
		return PASSES_PER_SEC(15);
#else
		return PASSES_PER_SEC(5);
#endif

	case 2:
		COXEventManager::Instance().WarpToAudience();
		COXEventManager::Instance().SetStatus(OXEVENT_CLOSE);
#ifdef ENABLE_OX_RENEWAL
		SendNoticeOxMap("#end", MAP_OXEVENT);
#else
		SendNoticeMap(LC_TEXT("[947]Ready for the next question?"), MAP_OXEVENT, true);
#endif
		flag = 0;
		break;
	default:
		break;
	}

	return 0;
}

bool COXEventManager::Quiz(uint8_t level, int timelimit)
{
	if (m_vec_quiz.size() == 0)
		return false;
	if (level > m_vec_quiz.size())
		level = static_cast<uint8_t>(m_vec_quiz.size() - 1);
	if (m_vec_quiz[level].size() <= 0)
		return false;

	if (timelimit < 0)
		timelimit = 30;

	const int idx = number(0, m_vec_quiz[level].size() - 1);

#ifdef ENABLE_OX_RENEWAL
	SendNoticeOxMap(LC_TEXT("[958]Question."), MAP_OXEVENT);

	SendNoticeOxMap(m_vec_quiz[level][idx].Quiz, MAP_OXEVENT);
	SendNoticeOxMap(LC_TEXT("[969]If it's correct, then go to O. If it's wrong, go to X."), MAP_OXEVENT);
	SendNoticeOxMap(LC_TEXT("[1142]The answer will appear in 20 seconds."), MAP_OXEVENT);
	SendNoticeOxMap("#start", MAP_OXEVENT);
#else
	SendNoticeMap(LC_TEXT("[958]Question."), MAP_OXEVENT, true);
	SendNoticeMap(m_vec_quiz[level][idx].Quiz, OXEVENT_MAP_INDEX, true);
	SendNoticeMap(LC_TEXT("[969]If it's correct, then go to O. If it's wrong, go to X."), MAP_OXEVENT, true);
#endif

	if (m_timedEvent != nullptr)
		event_cancel(&m_timedEvent);

	OXEventInfoData* answer = AllocEventInfo<OXEventInfoData>();

	answer->answer = m_vec_quiz[level][idx].answer;

	timelimit -= 15;
	m_timedEvent = event_create(oxevent_timer, answer, PASSES_PER_SEC(timelimit));

	SetStatus(OXEVENT_QUIZ);

	m_vec_quiz[level].erase(m_vec_quiz[level].begin() + idx);
	return true;
}

bool COXEventManager::CheckAnswer(bool answer)
{
	if (m_map_attender.size() <= 0)
		return true;

	auto iter = m_map_attender.begin();

	m_map_miss.clear();

	int rect[4];
	if (answer != true)
	{
		rect[0] = 892600;
		rect[1] = 22900;
		rect[2] = 896300;
		rect[3] = 26400;
	}
	else
	{
		rect[0] = 896600;
		rect[1] = 22900;
		rect[2] = 900300;
		rect[3] = 26400;
	}

	LPCHARACTER pkChar = nullptr;
	PIXEL_POSITION pos;
	for (; iter != m_map_attender.end();)
	{
		pkChar = CHARACTER_MANAGER::Instance().FindByPID(iter->second);
		if (pkChar != nullptr)
		{
			pos = pkChar->GetXYZ();

			if (pos.x < rect[0] || pos.x > rect[2] || pos.y < rect[1] || pos.y > rect[3])
			{
				pkChar->EffectPacket(SE_FAIL);
				auto iter_tmp = iter;
				iter++;
				m_map_attender.erase(iter_tmp);
				m_map_miss.insert(std::make_pair(pkChar->GetPlayerID(), pkChar->GetPlayerID()));
			}
			else
			{
				pkChar->ChatPacket(CHAT_TYPE_INFO, "[LS;980]");
				// pkChar->CreateFly(number(FLY_FIREWORK1, FLY_FIREWORK6), pkChar);
				char chatbuf[256];
				int len = snprintf(chatbuf, sizeof(chatbuf),
					"%s %u %u", number(0, 1) == 1 ? "cheer1" : "cheer2", (uint32_t)pkChar->GetVID(), 0);

				// If the return value is greater than or equal to sizeof(chatbuf), it means truncated.
				if (len < 0 || len >= static_cast<int>(sizeof(chatbuf)))
					len = sizeof(chatbuf) - 1;

				// Includes \0 character
				++len;

				TPacketGCChat pack_chat{};
				pack_chat.header = HEADER_GC_CHAT;
				pack_chat.size = sizeof(TPacketGCChat) + len;
				pack_chat.type = CHAT_TYPE_COMMAND;
				pack_chat.id = 0;

				TEMP_BUFFER buf;
				buf.write(&pack_chat, sizeof(TPacketGCChat));
				buf.write(chatbuf, len);

				pkChar->PacketAround(buf.read_peek(), buf.size());
				pkChar->EffectPacket(SE_SUCCESS);

				++iter;
			}
		}
		else
		{
			const auto err = m_map_char.find(iter->first);
			if (err != m_map_char.end())
				m_map_char.erase(err);

			const auto err2 = m_map_miss.find(iter->first);
			if (err2 != m_map_miss.end())
				m_map_miss.erase(err2);

			auto iter_tmp = iter;
			++iter;
			m_map_attender.erase(iter_tmp);
		}
	}
	return true;
}

void COXEventManager::WarpToAudience()
{
	if (m_map_miss.size() <= 0)
		return;

	auto iter = m_map_miss.begin();
	LPCHARACTER pkChar = nullptr;

	for (; iter != m_map_miss.end(); ++iter)
	{
		pkChar = CHARACTER_MANAGER::Instance().FindByPID(iter->second);

		if (pkChar != nullptr)
		{
			switch (number(0, 3))
			{
				case 0:
					pkChar->Show(MAP_OXEVENT, 896300, 28900);
					break;
				case 1:
					pkChar->Show(MAP_OXEVENT, 890900, 28100);
					break;
				case 2:
					pkChar->Show(MAP_OXEVENT, 896600, 20500);
					break;
				case 3:
					pkChar->Show(MAP_OXEVENT, 902500, 28100);
					break;
				default:
					pkChar->Show(MAP_OXEVENT, 896300, 28900);
					break;
			}
		}
	}

	m_map_miss.clear();
}

bool COXEventManager::CloseEvent()
{
	if (m_timedEvent != nullptr)
		event_cancel(&m_timedEvent);

	auto iter = m_map_char.begin();

	LPCHARACTER pkChar = nullptr;
	for (; iter != m_map_char.end(); ++iter)
	{
		pkChar = CHARACTER_MANAGER::Instance().FindByPID(iter->second);

		if (pkChar != nullptr)
			pkChar->WarpSet(EMPIRE_START_X(pkChar->GetEmpire()), EMPIRE_START_Y(pkChar->GetEmpire()));
	}

	Initialize();

	return true;
}

bool COXEventManager::LogWinner()
{
	auto iter = m_map_attender.begin();

	for (; iter != m_map_attender.end(); ++iter)
	{
		LPCHARACTER pkChar = CHARACTER_MANAGER::Instance().FindByPID(iter->second);

		if (pkChar)
			LogManager::Instance().CharLog(pkChar, 0, "OXEVENT", "LastManStanding");
	}

	return true;
}

bool COXEventManager::GiveItemToAttender(uint32_t dwItemVnum, uint8_t count)
{
	auto iter = m_map_attender.begin();

	for (; iter != m_map_attender.end(); ++iter)
	{
		LPCHARACTER pkChar = CHARACTER_MANAGER::Instance().FindByPID(iter->second);

		if (pkChar)
		{
			pkChar->AutoGiveItem(dwItemVnum, count);
			LogManager::Instance().ItemLog(pkChar->GetPlayerID(), 0, count, dwItemVnum, "OXEVENT_REWARD", "", pkChar->GetDesc()->GetHostName(), dwItemVnum);
		}
	}

	return true;
}
