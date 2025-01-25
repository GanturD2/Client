#include "stdafx.h"
#include "start_position.h"


char g_nation_name[4][32] =
{
	"",
	"�ż���",
	"õ����",
	"���뱹",
};

long g_start_map[4] =
{
	0,	// reserved
	1,	// Rotes Reich
	21,	// Gelbes Reich
	41	// Blaues Reich
};

// Stadtport
uint32_t g_start_position[4][2] =
{
	{      0,      0 },	// reserved
	{ 469300, 964200 },	// Rotes Reich
	{  55700, 157900 },	// Gelbes Reich
	{ 969600, 278400 }	// Blaues Reich
};

// Gildenkriegwarp
uint32_t arena_return_position[4][2] =
{
	{       0,  0       },
	{   347600, 882700  }, //
	{   138600, 236600  }, //
	{   857200, 251800  }  //
};

// Charaktererstellung
#ifdef ENABLE_NEW_RETARDED_GF_START_POSITION
uint32_t g_create_position[4][2] =
{
	{		0,		0 },	// reserviert

	{ 474800, 966000 },		// Rotes Reich
	{ 60000, 155700 },		// Gelbes Reich
	{ 963700, 278400 },		// Blaues Reich
};
#else
uint32_t g_create_position[4][2] =
{
	{		0,		0 },	// reserviert
	{ 459800, 953900 },		// Rotes Reich
	{ 52070, 166600 },		// Gelbes Reich
	{ 957300, 255200 },		// Blaues Reich
};
#endif

uint32_t g_create_position_canada[4][2] =
{
	{		0,		0 },
	{ 457100, 946900 },
	{ 45700, 166500 },
	{ 966300, 288300 },
};

