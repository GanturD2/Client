#pragma once

#ifdef ENABLE_WEATHER_INFO
#include "PythonBackground.h"
#include "PythonMiniMap.h"
#include "PythonSystem.h"
#include "../GameLib/MapOutdoor.h"

class IWeatherInfo : public CSingleton<IWeatherInfo>
{
public:
	enum DayMode : uint8_t
	{
		DAY,
		NIGHT,
		MORNING,
		EVENING,
		RAIN,
	};

	enum eWeather
	{
		WINTER,
		SUMMER
	};

	IWeatherInfo() = default;
	~IWeatherInfo() = default;

	bool IsWinter(int Month);
	void WeatherRefresh(uint8_t byDayMode, bool season, uint8_t bRain);
};
#endif
