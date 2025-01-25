#include "../../common/stl.h"
#include <unordered_map>

class CDragonLair
{
public:
	CDragonLair(uint32_t dwGuildID, long BaseMapID, long PrivateMapID);
	virtual ~CDragonLair();

	uint32_t GetEstimatedTime() const;

	void OnDragonDead(LPCHARACTER pDragon);

private:
	uint32_t StartTime_;
	uint32_t GuildID_;
	long BaseMapIndex_;
	long PrivateMapIndex_;
};

class CDragonLairManager : public singleton<CDragonLairManager>
{
public:
	CDragonLairManager();
	virtual ~CDragonLairManager();

	bool Start(long MapIndexFrom, long BaseMapIndex, uint32_t GuildID);
	void OnDragonDead(LPCHARACTER pDragon, uint32_t KillerGuildID);

	size_t GetLairCount() const { return LairMap_.size(); }

private:
	std::unordered_map<uint32_t, CDragonLair*> LairMap_;
};
