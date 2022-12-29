#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <string>
#include <Psapi.h>
#include <vector>
#include <cstdio>

#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Button, Horizontal, Renderer
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"
#undef input

#define input(key)\
GetAsyncKeyState(key) & 0x8000

using namespace ftxui;

// Internal related:
#include "console.h"