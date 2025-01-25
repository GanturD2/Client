#pragma once
#include "StdAfx.h"
#include <functional>
#include "PythonApplication.h"

#define now CPythonApplication::Instance().GetGlobalTime()

struct HandlerEventInfo
{
	HandlerEventInfo(const std::function<void()> &fn, const int &_wait, const int &_count) : func(fn), wait(_wait), count(_count), time(now + _wait), bind(0) {}
	int wait, count, bind;
	float time;
	std::function<void()> func;
};

class EventHandler : public CSingleton<EventHandler>
{
public:
	EventHandler() { Destroy();}

	virtual ~EventHandler() { Destroy(); }

	void Destroy() { EventInfoMap.clear(); }

	void AddEvent(const std::string& name, const std::function<void()> &func, const int &wait, const int &count)
	{
		if (GetHandler(name))
			DeleteEvent(name);

#ifdef _HAS_CXX17
		EventInfoMap[name] = std::make_unique<HandlerEventInfo>(func, wait, count);
#else
		EventInfoMap[name] = std::auto_ptr<HandlerEventInfo>(new HandlerEventInfo(func, wait, count));
#endif
	}

	void DeleteEvent(const std::string& name)
	{
		if (GetHandler(name))
			EventInfoMap.erase(name);
	}

	void DeleteProccess()
	{
		int size = EventInfoMap.size();
		if (size > 0)
		{
			for (size_t i = 0; i < size; i++)
			{
				auto it = EventInfoMap.begin();
				if (it == EventInfoMap.end())
				{
					break;
				}

				while (it != EventInfoMap.end())
				{
					auto hand = GetHandler(it->first);
					auto get_data = hand->get();
					if (!get_data->count)
					{
						DeleteEvent(it->first);
						break;
					}
					break;
				}
			}
		}

	}

	void Proccess()
	{
		if (!EventInfoMap.size())
			return;

		for (const auto &event : EventInfoMap)
		{
			if (GetHandler(event.first))
			{
				if (!event.second->count)
				{
					DeleteEvent(event.first);
					continue;
				}
				if (event.second->time <= now)
				{
					event.second->bind++;
					event.second->count--;
					event.second->func();					
					event.second->time = now + static_cast<float>(event.second->wait);
					if (event.second->count <= 0)
					{
						DeleteEvent(event.first);
					}
				}
			}
		}
	};

	bool FindEvent(const std::string & name)
	{
		const auto &it = EventInfoMap.find(name);
		return it != EventInfoMap.end();
	};

#ifdef _HAS_CXX17
	std::unique_ptr<HandlerEventInfo>* GetHandler(const std::string& name) { return FindEvent(name) ? &EventInfoMap.find(name)->second : nullptr; };
private:
	std::map<std::string, std::unique_ptr<HandlerEventInfo>> EventInfoMap;
#else
	std::auto_ptr<HandlerEventInfo>* GetHandler(const std::string& name) { return FindEvent(name) ? &EventInfoMap.find(name)->second : nullptr; };
private:
	std::map<std::string, std::auto_ptr<HandlerEventInfo>> EventInfoMap;
#endif
};
