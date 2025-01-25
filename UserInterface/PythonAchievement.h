#pragma once
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "GameType.h"
#include "Packet.h"

namespace achievements
{
	const int TITLE_CHARACTER_MAX_NAME = 384;
	enum EAchievementStatus : int8_t {
		LOCKED,
		UNLOCKED,
		FINISHED
	};

	enum EAchievementSubHeadersClient : int8_t {
		HEADER_GC_INITIAL,
		HEADER_GC_UPDATE,
		HEADER_GC_UPDATE_POINTS,
		HEADER_GC_RANKING,

		HEADER_GC_NOTIFICATION,
		HEADER_GC_UPDATE_TITLES,
		HEADER_GC_RELOAD_TITLES,

	};

	enum EAchievementSubHeadersGame : int8_t {
		HEADER_CG_SELECT_TITLE,
		HEADER_CG_OPEN_SHOP,
		HEADER_CG_OPEN_RANKING,

	};

	typedef struct SAchievement {
		int8_t status;
		uint8_t percentage;
		uint32_t finish_timestamp;
		SAchievement() : status(LOCKED), percentage(0), finish_timestamp(0) {}
	} TAchievement;

	typedef struct STitle {
		uint32_t achievement;
		char szName[TITLE_CHARACTER_MAX_NAME + 1];
		bool bUnlocked;
		STitle() : achievement(0), bUnlocked(false) {
			szName[TITLE_CHARACTER_MAX_NAME] = '\0';
		}
	} TTitle;
#pragma pack(1)
	struct GC_packet {
		uint8_t header;
		uint16_t size;
		uint8_t subHeader;
	};

	struct GC_load {
		uint32_t points;
		uint8_t achievements;
		uint8_t titles;
	};

	struct GC_Update {
		uint32_t id;
		TAchievement achievement;
	};

	struct GC_ranking {
		uint8_t level;
		char name[CHARACTER_NAME_MAX_LEN + 1];
		char guild[GUILD_NAME_MAX_LEN + 1];
		uint32_t progress;
	};

	struct CG_packet {
		uint8_t header;
		uint8_t subHeader;
		uint32_t dummy;
	};
#pragma pack()
};

class CAchievementSystem : public CSingleton<CAchievementSystem>
{
public:
	CAchievementSystem();
	~CAchievementSystem();

public:
	void SetAchievementHandler(PyObject& handler);
	void SetAchievementNotificationHandler(PyObject& handler);
	void ResetAchievementHandler();
	void ResetAchievementNotificationHandler();

	void SetAchievementPoints(uint32_t points) { _achievementPoints = points; }

public:
	int8_t GetAchievementStatus(uint32_t achievement);
	float GetAchievementPercentage(uint32_t achievement);
	uint32_t GetAchievementFinishTimeStamp(uint32_t achievement);

	uint32_t GetAchievementPoints() const { return _achievementPoints; }

	bool LoadTitle(uint32_t id, uint32_t achievement, const char* name);
	bool GetTitle(uint32_t id, achievements::TTitle** title);

	bool GetTitleName(uint32_t id, std::string& name);

	bool ProcessPackets();

private:
	PyObject* _achievementHandler;
	PyObject* _achievementNotificationsHandler;

	uint32_t _achievementPoints;

	std::map<uint32_t, achievements::TAchievement> _achievements;
	std::map<uint32_t, achievements::TTitle> _titles;
};
#endif
