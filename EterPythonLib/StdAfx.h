#pragma once

#ifndef _CRT_SECURE_NO_WARNINGS
#	define _CRT_SECURE_NO_WARNINGS
#endif
#include "../EterBase/StdAfx.h"
#include "../EterLib/StdAfx.h"
#include "../ScriptLib/StdAfx.h"
#include "../UserInterface/Locale_inc.h"

#include "PythonGraphic.h"
#include "PythonWindowManager.h"

void initgrp();
void initgrpImage();
void initgrpText();
void initgrpThing();
void initscriptWindow();
void initwndMgr();