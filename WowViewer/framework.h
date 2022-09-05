// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <atlbase.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <directxpackedvector.h>

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <span>
#include <ranges>
#include <array>
#include <algorithm>


using namespace ATL;
using namespace DirectX;
using namespace DirectX::PackedVector;

#include "CascLib.h"

#include "utils.hpp"