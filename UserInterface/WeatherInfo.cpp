#include "StdAfx.h"

#ifdef ENABLE_WEATHER_INFO
#include "WeatherInfo.h"

bool IWeatherInfo::IsWinter(int Month)	// 0 = Summer, 1 = Winter
{
	if (Month < 3)
		return false;
	else if (Month >= 3 && Month < 12)
		return true;
	else if (Month == 12)
		return false;

	return true;
}

uint8_t GetWeatherDayMode(int Hour)
{
	if (Hour >= DailyWeather::MIDNIGHT && Hour < DailyWeather::SUNRISE)
		return IWeatherInfo::DayMode::NIGHT;		//Nacht [Früh]
	else if (Hour >= DailyWeather::SUNRISE && Hour < DailyWeather::NOON)
		return IWeatherInfo::DayMode::MORNING;	// Morgens
	else if (Hour >= DailyWeather::NOON && Hour < DailyWeather::SUNSET)
		return IWeatherInfo::DayMode::DAY;		// Mittags
	else if (Hour >= DailyWeather::SUNSET && Hour < DailyWeather::DARKNESS)
		return IWeatherInfo::DayMode::EVENING;	// Nachmittags
	else if (Hour >= DailyWeather::DARKNESS && Hour <= DailyWeather::BEFORE_MID)
		return IWeatherInfo::DayMode::NIGHT;		// Nacht [Abend]
	else
		return IWeatherInfo::DayMode::DAY;

	return IWeatherInfo::DayMode::DAY;
}

std::string GetEnvironment(int dMode)
{
	std::string envData = "sunhorizon.msenv";
	switch (dMode)
	{
	case IWeatherInfo::DayMode::DAY:
		envData = "sunhorizon.msenv";
		break;

	case IWeatherInfo::DayMode::NIGHT:
		envData = "cloudymonth.msenv";
		break;

	case IWeatherInfo::DayMode::MORNING:
		envData = "sunset.msenv";
		break;

	case IWeatherInfo::DayMode::EVENING:
		envData = "eveningsun.msenv";
		break;

	case IWeatherInfo::DayMode::RAIN:
		envData = "rainyday.msenv";
		break;

	default:
	{
		TraceError("No GetEnvironment");
		envData = "sunhorizon.msenv";
	}
	break;
	}

	return envData;
}

bool GetMapEnvData(std::string strMapName)
{
	// Allowed Custom Environment here
	if (strMapName == "metin2_map_a1" || strMapName == "metin2_map_b1" || strMapName == "metin2_map_c1" || strMapName == "metin2_map_a3" || strMapName == "metin2_map_b3" || strMapName == "metin2_map_c3"
		|| strMapName == "map_n_snowm_01" || strMapName == "metin2_map_n_desert_01" || strMapName == "metin2_map_milgyo")
		return true;

	// Disable Environment
	return false;
}

void IWeatherInfo::WeatherRefresh(uint8_t byDayMode, bool season, uint8_t bRain)
{
	time_t ct = time(nullptr);
	struct tm tm = *localtime(&ct);

	CPythonBackground& rkBG = CPythonBackground::Instance();
	CPythonSystem& rkSystem = CPythonSystem::Instance();

	uint8_t bDayMode = GetWeatherDayMode(tm.tm_hour) != byDayMode ? byDayMode : GetWeatherDayMode(tm.tm_hour);
	uint8_t bSeason = IsWinter(tm.tm_hour) != season ? season : IsWinter(tm.tm_hour);
	//TraceError("AUTO DayMode %d Season %d Rain %d", bDayMode, bSeason, bRainActive);

	/* ~~~| DAYMODE |~~~ */
	std::string strEnvironment = "d:/ymir work/environment/";
	strEnvironment += GetEnvironment(GetWeatherDayMode(tm.tm_hour));
	//TraceError("AUTO ENV CHANGE: %s", strEnvironment.c_str());

	if (GetMapEnvData(rkBG.GetWarpMapName()))
	{
		rkBG.RegisterEnvironmentData(GetWeatherDayMode(tm.tm_hour), strEnvironment.c_str());
		//rkBG.SetEnvironmentData(iBG.GetWeatherDayMode(tm.tm_hour));

		const TEnvironmentData* c_pEnvironmenData;
		if (rkBG.GetEnvironmentData(GetWeatherDayMode(tm.tm_hour), &c_pEnvironmenData))
			rkBG.ResetEnvironmentDataPtr(c_pEnvironmenData);

		switch (bSeason)
		{
		case WINTER:
		{
			/* ~~~| ENVIRONMENT TEXTURSET |~~~ */
			// Change if Summer to Snow Environment & Save it.
			if (!CPythonBackground::Instance().IsSnowTextureModeOption())
			{
				rkSystem.SetSnowTextureModeOption(1);
				CPythonBackground::Instance().EnableSnowTextureMode(1);
			}
			CPythonMiniMap::Instance().ReloadAtlas(true);

			/* ~~~| SNOWFLAKES |~~~ */
			if (bRain)
			{
				// If activeState is True and Snow disabled -> Enable Snow!
				if (!CPythonBackground::Instance().IsSnowModeOption())
					CPythonBackground::Instance().EnableSnowEnvironment();
			}
			else
			{
				// If activeState is False and Snow enabled -> Disable Snow!
				if (CPythonBackground::Instance().IsSnowModeOption())
					CPythonBackground::Instance().DisableSnowEnvironment();
			}
		}
		break;

		case SUMMER:
		{
			/* ~~~| ENVIRONMENT TEXTURSET |~~~ */
			// Change if Snow to Summer Environment
			if (CPythonBackground::Instance().IsSnowTextureModeOption())
			{
				rkSystem.SetSnowTextureModeOption(0);
				CPythonBackground::Instance().EnableSnowTextureMode(0);
			}
			CPythonMiniMap::Instance().ReloadAtlas(false);

# ifdef ENABLE_ENVIRONMENT_RAIN
			/* ~~~| RAINDROPS |~~~ */
			if (bRain)
			{
				// If activeState is True and Rain disabled -> Enable Rain!
				if (!CPythonBackground::Instance().IsRainModeOption())
					CPythonBackground::Instance().EnableRainEnvironment();
			}
			else
			{
				// If activeState is False and Rain enabled -> Disable Rain!
				if (CPythonBackground::Instance().IsRainModeOption())
					CPythonBackground::Instance().DisableRainEnvironment();
			}
# endif
		}
		break;

		default:
			break;
		}
	}
	else
	{
		if (CPythonBackground::Instance().IsSnowModeOption())
			CPythonBackground::Instance().DisableSnowEnvironment();

# ifdef ENABLE_ENVIRONMENT_RAIN
		if (CPythonBackground::Instance().IsRainModeOption())
			CPythonBackground::Instance().DisableRainEnvironment();
# endif
	}
}
#endif
