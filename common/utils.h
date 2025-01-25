/*----- atoi function -----*/
#include "CommonDefines.h"
#include <msl/utils.h>

/*----- atoi function -----*/
inline bool str_to_number(bool& out, const char* in) noexcept
{
	if (0 == in || 0 == in[0])	return false;

	out = (strtol(in, nullptr, 10) != 0);
	return true;
}

inline bool str_to_number(char& out, const char* in) noexcept
{
	if (0 == in || 0 == in[0])	return false;

	out = (char)strtol(in, nullptr, 10);
	return true;
}

inline bool str_to_number(uint8_t& out, const char* in) noexcept
{
	if (0 == in || 0 == in[0])	return false;

	out = (uint8_t)strtoul(in, nullptr, 10);
	return true;
}

inline bool str_to_number(int16_t& out, const char* in) noexcept
{
	if (0 == in || 0 == in[0])	return false;

	out = (int16_t)strtol(in, nullptr, 10);
	return true;
}

inline bool str_to_number(uint16_t& out, const char* in) noexcept
{
	if (0 == in || 0 == in[0])	return false;

	out = (uint16_t)strtoul(in, nullptr, 10);
	return true;
}

inline bool str_to_number(int& out, const char* in) noexcept
{
	if (0 == in || 0 == in[0])	return false;

	out = (int)strtol(in, nullptr, 10);
	return true;
}

inline bool str_to_number(uint32_t& out, const char* in) noexcept
{
	if (0 == in || 0 == in[0])	return false;

	out = (uint32_t)strtoul(in, nullptr, 10);
	return true;
}

inline bool str_to_number(long& out, const char* in) noexcept
{
	if (0 == in || 0 == in[0])	return false;

	out = (long)strtol(in, nullptr, 10);
	return true;
}

inline bool str_to_number(unsigned long& out, const char* in) noexcept
{
	if (0 == in || 0 == in[0])	return false;

	out = (unsigned long)strtoul(in, nullptr, 10);
	return true;
}

inline bool str_to_number(long long& out, const char* in) noexcept
{
	if (0 == in || 0 == in[0])	return false;

	out = (long long)strtoull(in, nullptr, 10);
	return true;
}

inline bool str_to_number(float& out, const char* in) noexcept
{
	if (0 == in || 0 == in[0])	return false;

	out = (float)strtof(in, nullptr);
	return true;
}

inline bool str_to_number(double& out, const char* in) noexcept
{
	if (0 == in || 0 == in[0])	return false;

	out = (double)strtod(in, nullptr);
	return true;
}

#ifdef __FreeBSD__
inline bool str_to_number(long double& out, const char* in) noexcept
{
	if (0 == in || 0 == in[0])	return false;

	out = (long double)strtold(in, nullptr);
	return true;
}
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
namespace m2
{
	template<typename T>
	T minmax(T min, T value, T max)
	{
		T tv;

		tv = (min > value ? min : value);
		return (max < tv) ? max : tv;
	}
}

inline bool str_to_bool(bool& out, const std::string& in)
{
	out = in.at(0) == '1';
	return true;
}
#endif
#if defined(ENABLE_PREMIUM_PRIVATE_SHOP) || defined(ENABLE_ACHIEVEMENT_SYSTEM)
inline bool str_to_number(unsigned long long& out, const char* in) noexcept
{
	if (0 == in || 0 == in[0])  return false;

	out = (unsigned long long) strtoull(in, nullptr, 10);
	//out = (uint64_t)strtoul(in, NULL, 10);
	return true;
}
#endif

/*----- atoi function -----*/

int uniform_random(int a, int b);
double uniform_random(double a, double b);
