#include "StdAfx.h"
#include "../UserInterface/Locale_inc.h"

#include <ctime>
#include <cstdio>
#include "Debug.h"
#include "Singleton.h"
#include "Timer.h"
#ifdef ENABLE_SYSERR_BY_PID
#	include <fmt/fmt.h>
#endif

const uint32_t DEBUG_STRING_MAX_LEN = 1024;

static int isLogFile = false;
HWND g_PopupHwnd = nullptr;

class CLogFile : public CSingleton<CLogFile>
{
public:
	CLogFile() = default;

	~CLogFile() override = default;

	void Initialize() { m_fPtr.open("log.txt", "w"); }

	void Write(const char * c_pszMsg) const
	{
		if (!m_fPtr)
			return;

		time_t ct = time(nullptr);
		struct tm ctm = *localtime(&ct);

		fprintf(m_fPtr.get(), "%02d%02d %02d:%02d:%05u :: %s", ctm.tm_mon + 1, ctm.tm_mday, ctm.tm_hour, ctm.tm_min,
				ELTimer_GetMSec() % 60000, c_pszMsg);

		fflush(m_fPtr.get());
	}

protected:
	msl::file_ptr m_fPtr;
};

static CLogFile gs_logfile;
static uint32_t gs_uLevel = 0;

void SetLogLevel(uint32_t uLevel)
{
	gs_uLevel = uLevel;
}

void Log(uint32_t uLevel, const char * c_szMsg)
{
	if (uLevel >= gs_uLevel)
		Trace(c_szMsg);
}

void Logn(uint32_t uLevel, const char * c_szMsg)
{
	if (uLevel >= gs_uLevel)
		Tracen(c_szMsg);
}

void Logf(uint32_t uLevel, const char * c_szFormat, ...)
{
	if (uLevel < gs_uLevel)
		return;

	char szBuf[DEBUG_STRING_MAX_LEN + 1];

	va_list args;
	va_start(args, c_szFormat);
	_vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);
	va_end(args);
#ifdef _DEBUG
	OutputDebugString(szBuf);
	fputs(szBuf, stdout);
#endif

	if (isLogFile)
		LogFile(szBuf);
}

void Lognf(uint32_t uLevel, const char * c_szFormat, ...)
{
	if (uLevel < gs_uLevel)
		return;

	va_list args;
	va_start(args, c_szFormat);

	char szBuf[DEBUG_STRING_MAX_LEN + 2];
	int len = _vsnprintf(szBuf, sizeof(szBuf) - 1, c_szFormat, args);

	if (len > 0)
	{
		szBuf[len] = '\n';
		szBuf[len + 1] = '\0';
	}
	va_end(args);
#ifdef _DEBUG
	OutputDebugString(szBuf);
	puts(szBuf);
#endif

	if (isLogFile)
		LogFile(szBuf);
}


void Trace(const char * c_szMsg)
{
#ifdef _DEBUG
	OutputDebugString(c_szMsg);
	printf("%s", c_szMsg);
#endif

	if (isLogFile)
		LogFile(c_szMsg);
}

void Tracen(const char * c_szMsg)
{
#ifdef _DEBUG
	char szBuf[DEBUG_STRING_MAX_LEN + 1];
	_snprintf(szBuf, sizeof(szBuf), "%s\n", c_szMsg);
	OutputDebugString(szBuf);
	puts(c_szMsg);

	if (isLogFile)
		LogFile(szBuf);

	puts(c_szMsg);
	putc('\n', stdout);
#else
	if (isLogFile)
	{
		LogFile(c_szMsg);
		LogFile("\n");
	}
#endif
}

void Tracenf(const char * c_szFormat, ...)
{
#ifndef NDEBUG // disabled on release
	va_list args;
	va_start(args, c_szFormat);

	char szBuf[DEBUG_STRING_MAX_LEN + 2];
	int len = _vsnprintf(szBuf, sizeof(szBuf) - 1, c_szFormat, args);

	if (len > 0)
	{
		szBuf[len] = '\n';
		szBuf[len + 1] = '\0';
	}
	va_end(args);
#	ifdef _DEBUG
	OutputDebugString(szBuf);
	printf("%s", szBuf);
#	endif

	if (isLogFile)
		LogFile(szBuf);
#endif
}

void Tracef(const char * c_szFormat, ...)
{
	char szBuf[DEBUG_STRING_MAX_LEN + 1];

	va_list args;
	va_start(args, c_szFormat);
	_vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);
	va_end(args);

#ifdef _DEBUG
	OutputDebugString(szBuf);
	fputs(szBuf, stdout);
#endif

	if (isLogFile)
		LogFile(szBuf);
}

void TraceError(const char * c_szFormat, ...)
{
#ifndef _DISTRIBUTE

	char szBuf[DEBUG_STRING_MAX_LEN + 2];

	strncpy(szBuf, "SYSERR: ", DEBUG_STRING_MAX_LEN);
	int len = strlen(szBuf);

	va_list args;
	va_start(args, c_szFormat);
	len = _vsnprintf(szBuf + len, sizeof(szBuf) - (len + 1), c_szFormat, args) + len;
	va_end(args);

	szBuf[len] = '\n';
	szBuf[len + 1] = '\0';

	time_t ct = time(nullptr);
	struct tm ctm = *localtime(&ct);

	fprintf(stderr, "%02d%02d %02d:%02d:%05u :: %s", ctm.tm_mon + 1, ctm.tm_mday, ctm.tm_hour, ctm.tm_min, ELTimer_GetMSec() % 60000,
			szBuf + 8);
	fflush(stderr);

#	ifdef _DEBUG
	OutputDebugString(szBuf);
	fputs(szBuf, stdout);
#	endif

	if (isLogFile)
		LogFile(szBuf);

#endif
}

void TraceErrorWithoutEnter(const char * c_szFormat, ...)
{
#ifndef _DISTRIBUTE

	char szBuf[DEBUG_STRING_MAX_LEN];

	va_list args;
	va_start(args, c_szFormat);
	_vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);
	va_end(args);

	time_t ct = time(nullptr);
	struct tm ctm = *localtime(&ct);

	fprintf(stderr, "%02d%02d %02d:%02d:%05u :: %s", ctm.tm_mon + 1, ctm.tm_mday, ctm.tm_hour, ctm.tm_min, ELTimer_GetMSec() % 60000,
			szBuf + 8);
	fflush(stderr);

#	ifdef _DEBUG
	OutputDebugString(szBuf);
	fputs(szBuf, stdout);
#	endif

	if (isLogFile)
		LogFile(szBuf);
#endif
}

void LogBoxf(const char * c_szFormat, ...)
{
	va_list args;
	va_start(args, c_szFormat);

	char szBuf[2048];
	_vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);

	LogBox(szBuf);
}

void LogBox(const char * c_szMsg, const char * c_szCaption, HWND hWnd)
{
	if (!hWnd)
		hWnd = g_PopupHwnd;

	MessageBox(hWnd, c_szMsg, c_szCaption ? c_szCaption : "LOG", MB_OK);
	Tracen(c_szMsg);
}

void LogFile(const char * c_szMsg)
{
	CLogFile::Instance().Write(c_szMsg);
}

void LogFilef(const char * c_szMessage, ...)
{
	va_list args;
	va_start(args, c_szMessage);
	char szBuf[DEBUG_STRING_MAX_LEN + 1];
	_vsnprintf(szBuf, sizeof(szBuf), c_szMessage, args);

	CLogFile::Instance().Write(szBuf);
}

#ifdef ENABLE_EXTENDED_SYSERR
void OpenLogFile(bool bUseLogFIle)
{

	time_t rawtime;
	struct tm * timeinfo;
	char buffer[80];
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buffer, sizeof(buffer), "./syserr/%d-%m-%Y-%H-%M-%S-syserr.txt", timeinfo);
	const char* str(buffer);

#if !defined(_DISTRIBUTE) || defined(_USE_LOG_FILE)
#	ifdef ENABLE_SYSERR_BY_PID
	freopen(fmt::format("./syserr/syserr-{}.txt", GetCurrentProcessId()).c_str(), "w", stderr);
#	else
	freopen(str, "w", stderr);
#	endif

	if (bUseLogFIle)
	{
		isLogFile = true;
		CLogFile::Instance().Initialize();
	}
#endif
}
#else
void OpenLogFile(bool bUseLogFIle)
{
#ifndef _DISTRIBUTE
#	ifdef ENABLE_SYSERR_BY_PID
	freopen(fmt::format("./syserr/syserr-{}.txt", GetCurrentProcessId()).c_str(), "w", stderr);
#	else
	freopen("syserr.txt", "w", stderr);
#	endif

	if (bUseLogFIle)
	{
		isLogFile = true;
		CLogFile::Instance().Initialize();
	}
#endif
}
#endif

void OpenConsoleWindow()
{
	AllocConsole();

	freopen("CONOUT$", "a", stdout);
	freopen("CONIN$", "r", stdin);
}
