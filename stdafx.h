#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
#include <windows.h>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <WTypes.h>
#include <OAIdl.h>
//#include "UIlib.h"
#define LINK_DUILIB
#include "UIlib.h"
using namespace DuiLib;
