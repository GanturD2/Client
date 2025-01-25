
#ifndef __INC_METIN_II_UTILS_H__
#define __INC_METIN_II_UTILS_H__

#include <math.h>
#include <random>
#include <sstream>
#include <iostream>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define IS_SET(flag, bit) ((flag) & (bit))
#define SET_BIT(var, bit) ((var) |= (bit))
#define REMOVE_BIT(var, bit) ((var) &= ~(bit))
#define TOGGLE_BIT(var, bit) ((var) = (var) ^ (bit))

#define M2_NEW new
#define M2_DELETE(p) delete (p)
// No M2_DELETE_EX
#define M2_DELETE_ARRAY(p) delete[] (p)
#define M2_SAFE_DELETE_ARRAY(p) if(p) delete[] (p);  (p) = nullptr
#define M2_PTR_REF(p) (p)
#define M2_PTR_DEREF(p) (*(p))

// Default get_pointer() free function template.
template<typename T>
T* get_pointer(T* p) {
	return p;
}

inline float DISTANCE_SQRT(long dx, long dy)
{
	return ::sqrt((float)dx * dx + (float)dy * dy);
}

inline int DISTANCE_APPROX(int dx, int dy)
{
	int min, max;

	if (dx < 0)
		dx = -dx;

	if (dy < 0)
		dy = -dy;

	if (dx < dy)
	{
		min = dx;
		max = dy;
	}
	else
	{
		min = dy;
		max = dx;
	}

	// coefficients equivalent to ( 123/128 * max ) and ( 51/128 * min )
	return (((max << 8) + (max << 3) - (max << 4) - (max << 1) +
		(min << 7) - (min << 5) + (min << 3) - (min << 1)) >> 8);
}

#ifdef ENABLE_YOHARA_SYSTEM
extern void split_value(std::string str, std::string splitBy, std::vector<std::string>& tokens);
#endif

#ifndef __WIN32__
inline uint16_t MAKEWORD(uint8_t a, uint8_t b)
{
	return static_cast<uint16_t>(a) | (static_cast<uint16_t>(b) << 8);
}
#endif

extern void set_global_time(time_t t);
extern time_t get_global_time();

#include <string>
std::string mysql_hash_password(const char* tmp_pwd);

extern int dice(int number, int size);
extern size_t str_lower(const char* src, char* dest, size_t dest_size);

extern void skip_spaces(char** string);

extern const char* one_argument(const char* argument, char* first_arg, size_t first_size);
extern const char* two_arguments(const char* argument, char* first_arg, size_t first_size, char* second_arg, size_t second_size);
extern const char* three_arguments(const char* argument, char* first_arg, size_t first_size, char* second_arg, size_t second_size, char* third_flag, size_t third_size);
extern const char* four_arguments(const char* argument, char* first_arg, size_t first_size, char* second_arg, size_t second_size, char* third_flag, size_t third_size, char* four_flag, size_t four_size);
extern const char* five_arguments(const char* argument, char* first_arg, size_t first_size, char* second_arg, size_t second_size, char* third_flag, size_t third_size, char* four_flag, size_t four_size, char* five_flag, size_t five_size);
extern const char* six_arguments(const char* argument, char* first_arg, size_t first_size, char* second_arg, size_t second_size, char* third_flag, size_t third_size, char* four_flag, size_t four_size, char* five_flag, size_t five_size, char* six_flag, size_t six_size);
extern const char* first_cmd(const char* argument, char* first_arg, size_t first_arg_size, size_t* first_arg_len_result);
extern void split_argument(const char* argument, std::vector<std::string>& vecArgs);

extern int CalculateDuration(int iSpd, int iDur);

extern float gauss_random(float avg = 0, float sigma = 1);

extern int parse_time_str(const char* str);

extern bool WildCaseCmp(const char* w, const char* s);

extern bool is_digits(const std::string& str);
#ifdef ENABLE_ELEMENTAL_WORLD
extern int firstDigit(int n);
#endif
#endif /* __INC_METIN_II_UTILS_H__ */
