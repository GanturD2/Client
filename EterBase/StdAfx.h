#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#endif
#ifndef _CRT_SECURE_NO_WARNINGS
#	define _CRT_SECURE_NO_WARNINGS
#endif

#pragma warning(disable : 4100 4127 4189 4231 4505 4512 4706) // cryptopp
#pragma warning(disable : 4995) // pragma deprecated

#pragma warning(disable : 4710) // not inlined
#pragma warning(disable : 4786) // character 255 넘어가는거 끄기
#pragma warning(disable : 4244) // type conversion possible lose of data

#include <Windows.h>
#include <cassert>
#include <cstdio>
#pragma warning(disable : 4201)
#include <mmsystem.h>
#pragma warning(default : 4201)
#include <ImageHlp.h>
#include <ctime>
#include <cstdint>

#pragma warning(push, 3)

#include <algorithm>
#include <string>
#include <vector>
#include <deque>
#include <list>
#include <map>
#include <msl/msl.h> // martysama library
#include <fmt/fmt.h>
using namespace std::string_literals; // enable "smth"s std::string literals

#pragma warning(pop)

#if _MSC_VER >= 1400
#	define stricmp _stricmp
#	define strnicmp _strnicmp
#	define strupt _strupr
#	define strcmpi _strcmpi
#	define fileno _fileno
// #	define access _access_s
// #	define _access _access_s
#	define atoi _atoi64
#endif

// CCG C.21 MACRO
#define CLASS_DELETE_COPYMOVE(name)          \
	name(const name &) = delete;             \
	name(name &&) = delete;                  \
	name & operator=(const name &) = delete; \
	name & operator=(name &&) = delete

// Armadillo nanomite protection
#ifndef NANOBEGIN
#	ifdef __BORLANDC__
#		define NANOBEGIN __emit__(0xEB, 0x03, 0xD6, 0xD7, 0x01)
#		define NANOEND __emit__(0xEB, 0x03, 0xD6, 0xD7, 0x00)
#	else
#		define NANOBEGIN __asm _emit 0xEB __asm _emit 0x03 __asm _emit 0xD6 __asm _emit 0xD7 __asm _emit 0x01
#		define NANOEND __asm _emit 0xEB __asm _emit 0x03 __asm _emit 0xD6 __asm _emit 0xD7 __asm _emit 0x00
#	endif
#endif

#include "vk.h"
#include "Filename.h"
#include "ServiceDefs.h"
