#include "Framework\Overlay\Overlay.h"
#include "Framework\Memory\Memory.h"
#include "Cheat\Cheat.h"
#include <thread>
#include <memory>
#include <chrono>

std::unique_ptr<Cheat>   fivem   = std::make_unique<Cheat>();
std::unique_ptr<Overlay> overlay = std::make_unique<Overlay>();

// Render thread
void Overlay::OverlayUserFunction()
{
	fivem->Misc();
	fivem->RenderInfo();

	// Hotkey: toggle lock nearest vehicle
	static bool lockKeyPressed = false;
	if (g.VehicleLockKey != 0 && IsKeyDown(g.VehicleLockKey)) {
		if (!lockKeyPressed) {
			lockKeyPressed = true;
			fivem->ToggleClosestVehicleLock(-1);
		}
	} else {
		lockKeyPressed = false;
	}

	if (g.ShowMenu)
		fivem->RenderMenu();

	if (g.ESP)
		fivem->RenderESP();

	if (g.ESP_Vehicle)
		fivem->RenderVehicles();
}


#include "Cheat\ServerDumper.h"
#include "Cheat\Aimbot.h"
#include "Cheat\SilentAim.h"
#include "Cheat\Triggerbot.h"
#include "Cheat\MagicBullets.h"

// Debug時のみコンソールあり
#if _DEBUG
int main()
#else 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif
{
	// Fix DPI Scale
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);

	// Overlay init (done once)
	if (!overlay->InitOverlay("grcWindow", WINDOW_CLASS))
		return 2;

	// Auto-reconnect loop: if FiveM closes, wait and re-attach
	while (true)
	{
		// Attach to FiveM process
		if (!m.AttachProcess("grcWindow", WINDOW_CLASS))
		{
			// Game not running yet — wait and retry
			std::this_thread::sleep_for(std::chrono::seconds(3));

			// Check if the overlay window is still alive; if user closed it, exit
			MSG msg;
			while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			if (msg.message == WM_QUIT)
				break;

			continue;
		}

		// Offset init
		if (!Game->InitOffset())
		{
			m.DetachProcess();
			std::this_thread::sleep_for(std::chrono::seconds(3));
			continue;
		}

		// Start background threads
		g.process_active = true;
		ServerDumper::Start();
		std::thread([]() { fivem->UpdateList();     }).detach();
		std::thread([]() { fivem->UpdateVehicles(); }).detach();
		std::thread([]() { silentAim.Thread(fivem.get()); }).detach();
		std::thread([]() { aimbot.Start(fivem.get(), overlay->GetHwnd()); }).detach();
		std::thread([]() { triggerbot.Start(fivem.get(), overlay->GetHwnd()); }).detach();
		std::thread([]() { magicBullets.Start(fivem.get(), overlay->GetHwnd()); }).detach();

		// Run the overlay render loop (returns when process_active == false)
		overlay->OverlayLoop();

		// If the user clicked Exit — stop completely, don't reconnect
		if (g.exit_requested)
			break;

		// Otherwise FiveM closed — cleanup and wait to re-attach
		g.process_active = false;
		m.DetachProcess();
		delete Game;
		Game = new GameSDK(); // recreate for next attach cycle

		// Wait a moment before trying to re-attach
		std::this_thread::sleep_for(std::chrono::seconds(4));
	}

	overlay->DestroyOverlay();
	return 0;
}