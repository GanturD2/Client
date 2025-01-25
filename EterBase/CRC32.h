#ifndef __INC_CRC32_H__
#define __INC_CRC32_H__

#include <Windows.h>
#include <cstdint>
#include "../UserInterface/Locale_inc.h"

extern uint32_t GetCRC32(const char * buf, size_t len);
#ifdef ENABLE_SKILL_COLOR_SYSTEM
extern uint32_t GetCaseCRC32(const char* buf, size_t len, const char* name = nullptr);
#else
extern uint32_t GetCaseCRC32(const char * buf, size_t len);
#endif
extern uint32_t GetHFILECRC32(HANDLE hFile);
extern uint32_t GetFileCRC32(const char * c_szFileName);
extern uint32_t GetFileSize(const char * c_szFileName);

#endif
