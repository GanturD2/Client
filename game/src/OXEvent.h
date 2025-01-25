struct tag_Quiz
{
	char level;
	char Quiz[256];
	bool answer;
};

enum OXEventStatus
{
	OXEVENT_FINISH = 0, // OX event is completely finished
	OXEVENT_OPEN = 1, // OX event started. Accessible through Eulduji (20012)
	OXEVENT_CLOSE = 2, // OX event participation is over. Entry through Eulduji (20012) is blocked
	OXEVENT_QUIZ = 3, // Take the quiz.

	OXEVENT_ERR = 0xff
};

class COXEventManager : public singleton<COXEventManager>
{
private:
	std::map<uint32_t, uint32_t> m_map_char;
	std::map<uint32_t, uint32_t> m_map_attender;
	std::map<uint32_t, uint32_t> m_map_miss;

	std::vector<std::vector<tag_Quiz> > m_vec_quiz;

#ifdef OX_REWARD_UPDATE
	struct OX_reward {
		OX_reward() {
			item = 0;
			count = 1;
			gold = 0;
		}
		uint32_t item;
		uint32_t count;
		uint32_t gold;
	};

	std::map<uint8_t, std::vector<OX_reward>> m_ox_reward;
#endif

	LPEVENT m_timedEvent;

protected:
	bool CheckAnswer();

	bool EnterAudience(LPCHARACTER pChar);
	bool EnterAttender(LPCHARACTER pChar);

public:
	bool Initialize();
	void Destroy();

#ifdef OX_REWARD_UPDATE
	bool GiveReward();
	void LoadRewardTable();
#endif

	//Determine whether it is an OX Event mapindex or not
	static inline bool IsEventMap(int mapindex) { return mapindex == MAP_OXEVENT; }

	OXEventStatus GetStatus();
	void SetStatus(OXEventStatus status);

	bool LoadQuizScript(const char* szFileName);

	bool Enter(LPCHARACTER pChar);

	bool CloseEvent();

	void ClearQuiz();
	bool AddQuiz(uint8_t level, const char* pszQuestion, bool answer);
	bool ShowQuizList(LPCHARACTER pChar);

	bool Quiz(uint8_t level, int timelimit);
	bool GiveItemToAttender(uint32_t dwItemVnum, uint8_t count);

	bool CheckAnswer(bool answer);
	void WarpToAudience();

	bool LogWinner();

	uint32_t GetAttenderCount() { return m_map_attender.size(); }
};
