#ifndef __GAME_SRC_STDAFX_H__
#define __GAME_SRC_STDAFX_H__

#include "../../libthecore/include/stdafx.h"

#include "../../common/singleton.h"
#include "../../common/utils.h"
#include "../../common/service.h"
#include "../../common/CommonDefines.h"

#include <algorithm>
#include <math.h>
#include <list>
#include <map>
#include <set>
#include <queue>
#include <string>
#include <vector>
#include <random>

#ifdef __GNUC__
#include <float.h>
#include <unordered_map>
#include <unordered_set>
#else
#include <iterator>
#include <unordered_map>
#include <unordered_set>
#define isdigit iswdigit
#define isspace iswspace
#endif

#include "typedef.h"
#include "Locale.hpp"
#include "event.h"
#include "../../../Extern/include/libprotect.h"

#define PASSES_PER_SEC(sec) ((sec) * passes_per_sec)

#ifndef M_PI
#define M_PI    3.14159265358979323846 /* pi */
#endif
#ifndef M_PI_2
#define M_PI_2  1.57079632679489661923 /* pi/2 */
#endif

#define IN
#define OUT

#define LC_TEXT(x) x

#endif
