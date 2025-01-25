#include "StdAfx.h"
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
#include "PythonAchievement.h"
#include "PythonNetworkStream.h"
#include "PythonCharacterManager.h"
#include "PythonPlayer.h"

#define PY_METHOD(name) PyObject* name(PyObject* self, PyObject* args)
#define PY_METHOD_DEF(name, function) {name, function, METH_VARARGS}

CAchievementSystem::CAchievementSystem()
	: _achievementHandler(nullptr),
	_achievementNotificationsHandler(nullptr),
	_achievementPoints(0)
{
	_achievements.clear();
	_titles.clear();
}

CAchievementSystem::~CAchievementSystem()
{
	_achievementHandler = _achievementNotificationsHandler = nullptr;
	_achievements.clear();
	_titles.clear();
}

void CAchievementSystem::SetAchievementHandler(PyObject& handler)
{
	_achievementHandler = &handler;
}

void CAchievementSystem::SetAchievementNotificationHandler(PyObject& handler)
{
	_achievementNotificationsHandler = &handler;
}

void CAchievementSystem::ResetAchievementHandler()
{
	_achievementHandler = nullptr;
}

void CAchievementSystem::ResetAchievementNotificationHandler()
{
	_achievementNotificationsHandler = nullptr;
}

int8_t CAchievementSystem::GetAchievementStatus(uint32_t achievement)
{
	const auto& ach = _achievements.find(achievement);
	if (ach == _achievements.end())
		return achievements::EAchievementStatus::LOCKED;

	return ach->second.status;
}

float CAchievementSystem::GetAchievementPercentage(uint32_t achievement)
{
	const auto& ach = _achievements.find(achievement);
	if (ach == _achievements.end())
		return 0.0f;

	if (ach->second.status == achievements::EAchievementStatus::UNLOCKED)
		return ach->second.percentage;
	else if ((ach->second.status == achievements::EAchievementStatus::FINISHED))
		return 100.0;

	return 0.0;
}

uint32_t CAchievementSystem::GetAchievementFinishTimeStamp(uint32_t achievement)
{
	const auto& ach = _achievements.find(achievement);
	if (ach == _achievements.end() || ach->second.status != achievements::EAchievementStatus::FINISHED)
		return 0;

	return ach->second.finish_timestamp;
}

bool CAchievementSystem::LoadTitle(uint32_t id, uint32_t achievement, const char* name)
{
	const auto& ach = _titles.find(id);
	if (ach != _titles.end())
	{
		ach->second.achievement = achievement;
		strncpy_s(ach->second.szName, name, sizeof(ach->second.szName));
		ach->second.szName[strlen(ach->second.szName)] = '\0';
		return true;
	}
	else
	{
		achievements::TTitle title;
		title.bUnlocked = false;
		title.achievement = achievement;
		strncpy_s(title.szName, name, sizeof(title.szName));
		title.szName[strlen(title.szName)] = '\0';
		_titles.insert({ id, title });
	}

	return false;
}

bool CAchievementSystem::GetTitle(uint32_t id, achievements::TTitle** title)
{
	const auto& ach = _titles.find(id);
	if (ach != _titles.end())
	{
		*title = &ach->second;
		return true;
	}

	return false;
}

bool CAchievementSystem::GetTitleName(uint32_t id, std::string& name)
{
	const auto& ach = _titles.find(id);
	if (ach != _titles.end())
	{
		name = ach->second.szName;
		return true;
	}

	return false;
}

bool CAchievementSystem::ProcessPackets()
{
	achievements::GC_packet headerPacket;
	if (!CPythonNetworkStream::Instance().Recv(sizeof(headerPacket), &headerPacket))
	{
		return false;
	}

	switch (headerPacket.subHeader)
	{
		case achievements::HEADER_GC_INITIAL:
		{
			_achievements.clear();

			achievements::GC_load subheaderPacket;
			if (!CPythonNetworkStream::Instance().Recv(sizeof(subheaderPacket),
				&subheaderPacket)) {
				return false;
			}

			SetAchievementPoints(subheaderPacket.points);
			while (subheaderPacket.achievements > 0) {
				achievements::TAchievement achievement;

				uint32_t id;
				if (!CPythonNetworkStream::Instance().Recv(sizeof(achievement),
					&achievement) ||
					!CPythonNetworkStream::Instance().Recv(sizeof(id), &id)) {
					return false;
				}
				_achievements.insert({ id, achievement });

				--subheaderPacket.achievements;

			}
			while (subheaderPacket.titles > 0) {
				uint32_t id;
				if (!CPythonNetworkStream::Instance().Recv(sizeof(id), &id)) {
					return false;
				}

				const auto& it = _titles.find(id);
				if (it != _titles.end())
					it->second.bUnlocked = true;

				--subheaderPacket.titles;

			}
			PyCallClassMemberFunc(_achievementHandler, "UpdateAchievepoints",
				Py_BuildValue("()"));
			PyCallClassMemberFunc(_achievementHandler, "LoadTitles",
				Py_BuildValue("()"));
			PyCallClassMemberFunc(_achievementHandler, "ReloadTitles",
				Py_BuildValue("()"));

			PyCallClassMemberFunc(_achievementHandler, "UpdateAchievements",
				Py_BuildValue("()"));


		}
		break;

		case achievements::HEADER_GC_UPDATE:
		{
			achievements::GC_Update subheaderPacket;
			if (!CPythonNetworkStream::Instance().Recv(sizeof(subheaderPacket),
				&subheaderPacket)) {
				return false;
			}
			const auto& it = _achievements.find(subheaderPacket.id);
			if (it != _achievements.end())
				it->second = subheaderPacket.achievement;
			else
				_achievements.insert(
					{ subheaderPacket.id, subheaderPacket.achievement });
			PyCallClassMemberFunc(_achievementHandler, "UpdateAchievements",
				Py_BuildValue("(i)", subheaderPacket.id));

		}
		break;

		case achievements::HEADER_GC_UPDATE_POINTS:
		{
			uint32_t points;
			if (!CPythonNetworkStream::Instance().Recv(sizeof(points),
				&points)) {
				return false;
			}

			SetAchievementPoints(points);
			PyCallClassMemberFunc(_achievementHandler, "UpdateAchievepoints",
				Py_BuildValue("()"));
		}
		break;

		case achievements::HEADER_GC_RANKING:
		{
			uint32_t rank_size = 0;
			if (!CPythonNetworkStream::Instance().Recv(sizeof(rank_size),
				&rank_size)) {
				return false;
			}
			PyCallClassMemberFunc(_achievementHandler, "ClearRanking",
				Py_BuildValue("()"));

			for (size_t i = 1; i <= rank_size; ++i) {
				achievements::GC_ranking rank;
				if (!CPythonNetworkStream::Instance().Recv(sizeof(rank),
					&rank)) {
					return false;
				}
				PyCallClassMemberFunc(
					_achievementHandler, "AppendRanking",
					Py_BuildValue("(IissI)", i, rank.level, rank.name, rank.guild, rank.progress));
			}
		}
		break;

		case achievements::HEADER_GC_NOTIFICATION:
		{
			uint32_t achievement = 0;
			uint8_t type = 0;
			if (!CPythonNetworkStream::Instance().Recv(sizeof(achievement),
				&achievement) ||
				!CPythonNetworkStream::Instance().Recv(sizeof(type), &type)) {
				return false;
			}
			PyCallClassMemberFunc(_achievementNotificationsHandler,
				"AppendNotification",
				Py_BuildValue("(Ii)", achievement, type));

		}
		break;

		case achievements::HEADER_GC_UPDATE_TITLES:
		{
			uint32_t title_size = 0;
			if (!CPythonNetworkStream::Instance().Recv(sizeof(uint32_t),
				&title_size))
				return false;
			while (title_size > 0) {
				uint32_t title_id = 0;
				if (!CPythonNetworkStream::Instance().Recv(sizeof(uint32_t),
					&title_id))
					return false;

				const auto& it = _titles.find(title_id);
				if (it != _titles.end())
					it->second.bUnlocked = true;

				--title_size;
			}
			PyCallClassMemberFunc(_achievementHandler, "LoadTitles",
				Py_BuildValue("()"));
			PyCallClassMemberFunc(_achievementHandler, "ReloadTitles",
				Py_BuildValue("()"));


		}
		break;

		case achievements::HEADER_GC_RELOAD_TITLES:
		{
			uint32_t title_old = 0, title_new = 0;
			if (!CPythonNetworkStream::Instance().Recv(sizeof(uint32_t),
				&title_old))
				return false;

			if (!CPythonNetworkStream::Instance().Recv(sizeof(uint32_t),
				&title_new))
				return false;


			PyCallClassMemberFunc(_achievementHandler, "ReloadTitles",
				Py_BuildValue("(ii)", title_old, title_new));


		}
		break;

		default:
			break;
	}
	return true;
}

// Python module
PY_METHOD(netGetAchievementStatus)
{
	int32_t achievement;
	if (!PyTuple_GetInteger(args, 0, &achievement))
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", CAchievementSystem::Instance().GetAchievementStatus(achievement));
}

PY_METHOD(netGetAchievementProgress)
{
	int32_t achievement;
	if (!PyTuple_GetInteger(args, 0, &achievement))
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", static_cast<uint32_t>(CAchievementSystem::Instance().GetAchievementPercentage(achievement)));
}

PY_METHOD(netGetAchievementFinishDate)
{
	int32_t achievement;
	if (!PyTuple_GetInteger(args, 0, &achievement))
		return Py_BuildValue("I", 0);

	return Py_BuildValue("I", CAchievementSystem::Instance().GetAchievementFinishTimeStamp(achievement));
}

PY_METHOD(netGetAchievementPoints)
{
	return Py_BuildValue("I", CAchievementSystem::Instance().GetAchievementPoints());
}

PY_METHOD(netRegisterAchievementHandler)
{
	PyObject* poHandler;
	if (!PyTuple_GetObject(args, 0, &poHandler))
		return Py_BuildException();

	CAchievementSystem::Instance().SetAchievementHandler(*poHandler);
	return Py_BuildNone();
}

PY_METHOD(netRegisterAchievementNotificationHandler)
{
	PyObject* poHandler;
	if (!PyTuple_GetObject(args, 0, &poHandler))
		return Py_BuildException();

	CAchievementSystem::Instance().SetAchievementNotificationHandler(*poHandler);
	return Py_BuildNone();
}

PY_METHOD(netResetAchievementHandler)
{
	CAchievementSystem::Instance().ResetAchievementHandler();
	return Py_BuildNone();
}

PY_METHOD(netResetAchievementNotificationHandler)
{
	CAchievementSystem::Instance().ResetAchievementNotificationHandler();
	return Py_BuildNone();
}

PY_METHOD(netLoadTitle)
{
	int32_t id;
	if (!PyTuple_GetInteger(args, 0, &id))
		return Py_BuildNone();

	int32_t achievement;
	if (!PyTuple_GetInteger(args, 1, &achievement))
		return Py_BuildNone();

	char* name;
	if (!PyTuple_GetString(args, 2, &name))
		return Py_BuildNone();

	return Py_BuildValue("b", CAchievementSystem::Instance().LoadTitle(id, achievement, name));
}

PY_METHOD(netIsTitleUnlocked)
{
	int32_t id;
	if (!PyTuple_GetInteger(args, 0, &id))
		return Py_BuildNone();

	achievements::TTitle* title = nullptr;

	if (CAchievementSystem::Instance().GetTitle(id, &title))
		return Py_BuildValue("b", title->bUnlocked);

	return Py_BuildValue("b", false);
}

PY_METHOD(netIsTitleEnabled)
{
	int32_t id;
	if (!PyTuple_GetInteger(args, 0, &id))
		return Py_BuildNone();

	return Py_BuildValue("b", CPythonPlayer::Instance().NEW_GetMainActorPtr()->GetTitleID() == id);
}

PY_METHOD(netSelectTitle)
{
	int32_t id;
	if (!PyTuple_GetInteger(args, 0, &id))
		return Py_BuildNone();

	const achievements::CG_packet packet = { HEADER_CG_ACHIEVEMENT, achievements::EAchievementSubHeadersGame::HEADER_CG_SELECT_TITLE, static_cast<uint32_t>(id) };
	CPythonNetworkStream::Instance().Send(sizeof(packet), &packet);

	return Py_BuildNone();
}

PY_METHOD(netRequestOpenShop)
{
	achievements::CG_packet packet = { HEADER_CG_ACHIEVEMENT, achievements::EAchievementSubHeadersGame::HEADER_CG_OPEN_SHOP };
	CPythonNetworkStream::Instance().Send(sizeof(packet), &packet);
	return Py_BuildNone();
}

PY_METHOD(netRequestOpenRanking)
{
	achievements::CG_packet packet = { HEADER_CG_ACHIEVEMENT, achievements::EAchievementSubHeadersGame::HEADER_CG_OPEN_RANKING };
	CPythonNetworkStream::Instance().Send(sizeof(packet), &packet);
	return Py_BuildNone();
}

void InitAchievementModule()
{
	static PyMethodDef s_methods[] =
	{
		PY_METHOD_DEF("RegisterAchievementHandler", netRegisterAchievementHandler),
		PY_METHOD_DEF("RegisterAchievementNotificationHandler", netRegisterAchievementNotificationHandler),

		PY_METHOD_DEF("ResetAchievementHandler", netResetAchievementHandler),
		PY_METHOD_DEF("ResetAchievementNotificationHandler", netResetAchievementNotificationHandler),


		PY_METHOD_DEF("GetAchievementStatus", netGetAchievementStatus),
		PY_METHOD_DEF("GetAchievementProgress", netGetAchievementProgress),
		PY_METHOD_DEF("GetAchievementFinishDate", netGetAchievementFinishDate),
		PY_METHOD_DEF("GetAchievementPoints", netGetAchievementPoints),


		PY_METHOD_DEF("LoadTitle", netLoadTitle),
		PY_METHOD_DEF("IsTitleUnlocked", netIsTitleUnlocked),
		PY_METHOD_DEF("IsTitleEnabled", netIsTitleEnabled),
		PY_METHOD_DEF("SelectTitle", netSelectTitle),

		PY_METHOD_DEF("RequestOpenShop", netRequestOpenShop),
		PY_METHOD_DEF("RequestOpenRanking", netRequestOpenRanking),

		{NULL, NULL, NULL},
	};

	PyObject* poModule = Py_InitModule("ach", s_methods);
};
#endif
