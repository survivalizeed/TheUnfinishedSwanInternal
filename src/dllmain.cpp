#include "pch.h"
#include "include.h"

#include <map>
#include <thread>
#include <sstream>

struct Vec3 {
	double* x, * y, * z;
};

struct Rot2 {
	float* y, * p;
};

namespace intern {
	void patch(void* addr, std::vector<BYTE> bytes) {
		DWORD oldProtect;
		VirtualProtect(addr, bytes.size(), PAGE_EXECUTE_READWRITE, &oldProtect);
		for (int i = 0; i < bytes.size(); ++i) {
			*((BYTE*)addr + i) = bytes[i];
		}
		VirtualProtect(addr, bytes.size(), oldProtect, &oldProtect);
	}

	uintptr_t calcAddN(uintptr_t ptr, std::vector<unsigned int> offsets)
	{
		uintptr_t addr = ptr;
		for (unsigned int i = 0; i < offsets.size(); ++i)
		{
			addr = *(uintptr_t*)addr;
			addr += offsets[i];
		}
		return addr;
	}

	uintptr_t calcAddS(uintptr_t ptr, std::vector<unsigned int> offsets)
	{
		uintptr_t addr = ptr;
		for (unsigned int i = 0; i < offsets.size() - 1; ++i)
			addr = *(uintptr_t*)(addr + offsets[i]);
		return addr + offsets[offsets.size() - 1];
	}
}

void refresh(ScreenInteractive& screen, bool& run) {
	while (run) {
		if(GetForegroundWindow() != GetConsoleWindow())
			screen.PostEvent(Event::Custom);
		Sleep(1);
	}
}

void doAutowalk(std::map<std::string, bool>& states, bool* autowalk, bool& run) {
	while (run) {
		if (states["autowalk"])
			*autowalk = TRUE;
		else
			*autowalk = FALSE;
		Sleep(1);
	}
}

void doFly(std::map<std::string, bool>& states, Vec3& playerPos, Rot2& playerRot, bool& run) {
	bool once = false;
	uintptr_t base = (uintptr_t)GetModuleHandleA(NULL);
	float increment = 1.f;
	while (run) {
		if (!states["fly"]) {
			if (once) {
				intern::patch((void*)(base + 0x172679), { 0xF2, 0x0F, 0x11, 0x8E, 0x80, 0x01, 0x00, 0x00 });
				intern::patch((void*)(base + 0x1725B1), { 0x0F, 0x11, 0x86, 0x58, 0x01, 0x00, 0x00 });
				once = false;
			}
			continue;
		}
		else if(!once) {
			intern::patch((void*)(base + 0x172679), { 0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90 });
			intern::patch((void*)(base + 0x1725B1), { 0x90,0x90,0x90,0x90,0x90,0x90,0x90 });
			once = true;
		}
		if (input(VK_SHIFT))
			increment = 3;
		else
			increment = 1;
		if (input('W')) {
			*playerPos.x += (double)cosf(*playerRot.y) * 0.01f * increment;
			*playerPos.y += (double)sinf(*playerRot.p) * 0.01f * increment;
			*playerPos.z += (double)sinf(*playerRot.y + 3.141592f) * 0.01f * increment;
		}
		if (input('S')) {
			*playerPos.x += (double)-cosf(*playerRot.y) * 0.01f * increment;
			*playerPos.y += (double)-sinf(*playerRot.p) * 0.01f * increment;
			*playerPos.z += (double)-sinf(*playerRot.y + 3.141592f) * 0.01f * increment;
		}
		if (input('A')) {
			*playerPos.x += (double)cosf(*playerRot.y + 3.141592f + (3.141592f / 2)) * 0.01f * increment;
			*playerPos.z += (double)sinf(*playerRot.y + (3.141592f / 2)) * 0.01f * increment;
		}
		if (input('D')) {
			*playerPos.x += (double)cosf(*playerRot.y + (3.141592f / 2)) * 0.01f * increment;
			*playerPos.z += (double)sinf(*playerRot.y + 3.141592f + (3.141592f / 2)) * 0.01f * increment;
		}
		Sleep(1);
	}
}



DWORD WINAPI MainThread(LPVOID param) {
	using namespace std;
	
	MessageBoxA(NULL, "Cheat injected", "Injected", MB_ICONINFORMATION);

	bool run = true;

	std::thread refreshT, doAutowalkT, doFlyT;

	uintptr_t base = (uintptr_t)GetModuleHandleA(NULL);
	stringstream stream;
	stream << hex << base;
	string baseStr = stream.str();

	Vec3 playerPos{};

	playerPos.x = (double*)intern::calcAddS(*(uintptr_t*)(base + 0x0161CBD0), { 0xB8, 0x68, 0x78, 0xD8, 0x80, 0x70, 0x160 });
	playerPos.y = (double*)intern::calcAddS(*(uintptr_t*)(base + 0x0160C818), { 0x10, 0x8, 0x0, 0x180 });
	playerPos.z = (double*)intern::calcAddS(*(uintptr_t*)(base + 0x0161CBD0), { 0xB8, 0x68, 0x78, 0xD8, 0x80, 0x70, 0x168 });

	Rot2 playerRot{};

	playerRot.p = (float*)intern::calcAddS(*(uintptr_t*)(base + 0x0160C290), { 0x30, 0x50, 0x60, 0x18, 0x30, 0x78, 0x13C });
	playerRot.y = (float*)intern::calcAddS(*(uintptr_t*)(base + 0x0160C290), { 0x30, 0x50, 0x60, 0x18, 0x30, 0x78, 0x138 });

	bool* autowalk = (bool*)(base + 0x15F7048);

	console::createNewConsole(1024);

	HWND hwnd = GetConsoleWindow();
	MoveWindow(hwnd, 0, 0, 400, 300, false);
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetWindowLong(hwnd, GWL_STYLE, 0);
	ShowWindow(hwnd, SW_SHOW);

	map<std::string, bool> states;
	auto container = Container::Vertical({});

	states["autowalk"] = false;
	container->Add(Checkbox("Autowalk", &states["autowalk"]));

	states["fly"] = false;
	container->Add(Checkbox("Fly", &states["fly"]));

	//auto style = ButtonOption::Animated(Color::Default, Color::GrayDark, Color::Default, Color::White);
	//container->Add(Button("Unject", [&] {	//Not working
	//	run = false;
	//	refreshT.join();
	//	doAutowalkT.join();
	//	console::releaseConsole();
	//	FreeLibraryAndExitThread((HMODULE)param, 0);
	//	}, style) | border | center);

	auto renderer = Renderer(container, [&] {
		return vbox({
				vbox({
					text("survivalizeed's TUS internal") | color(Color::Color(209, 202, 0)),
					hbox({
						vbox({
							text("X: " + to_string(*playerPos.x)),
							text("Y: " + to_string(*playerPos.y)),
							text("Z: " + to_string(*playerPos.z)),
						}) | border | size(WIDTH, LESS_THAN, 30),
						vbox({
							text("Module Base: " + baseStr),
							text("Pitch: " + to_string(*playerRot.p)),
							text("Yaw: " + to_string(*playerRot.y)),
						}) | border | size(WIDTH, LESS_THAN, 30),
					}),
				}) | border,
			    container->Render() | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 15) | border,
			}) | border;
		});

	auto screen = ScreenInteractive::FitComponent();
	refreshT = thread(refresh, ref(screen), ref(run));
	doAutowalkT = thread(doAutowalk, ref(states), autowalk, ref(run));
	doFlyT = thread(doFly, ref(states), ref(playerPos), ref(playerRot), ref(run));
	screen.Loop(renderer);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		CreateThread(NULL, NULL, MainThread, hModule, NULL, NULL);
		break;
	default:
		break;
	}
	return TRUE;
}
