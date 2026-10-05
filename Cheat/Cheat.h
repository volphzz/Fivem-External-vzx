#pragma once
#include "../Framework/ImGui/imgui.h"
#include "../Framework/ImGui/imgui_impl_win32.h"
#include "../Framework/ImGui/imgui_impl_dx11.h"
#include "../Framework/Memory/Memory.h"
#include "../Framework/SimpleMath/SimpleMath.h"
#include "../Globals/Globals.h"
#include "SDK/CPed/CPed.h"
#include <vector>
#include <mutex>
#include <memory>
#include <unordered_set>
using namespace DirectX::SimpleMath;

// Render.cpp : rendering logic
// Features.cpp : non-rendering logic

class Cheat
{
public:
	void RenderInfo();
	void RenderMenu();
	void RenderESP();
	void RenderVehicles();
	void ToggleClosestVehicleLock(int forceState = -1);

	void Misc();
	void UpdateList();
	void UpdateVehicles();

    // Interface thread-safe para os módulos de aim (Aimbot, SilentAim, Triggerbot, MagicBullets)
    std::shared_ptr<const std::vector<CPed>> GetEntitySnapshot() {
        std::lock_guard<std::mutex> lock(EntityListMutex);
        return pEntitySnapshot;
    }

    bool IsFriend(uintptr_t address) const {
        return address != 0 && FriendPlayers.find(address) != FriendPlayers.end();
    }

private:
    CPed local, *pLocal = &local;
    std::vector<CPed> EntityList;
    std::mutex EntityListMutex; // protects EntityList from data race
    std::shared_ptr<const std::vector<CPed>> pEntitySnapshot; // snapshot imutável para leitura concorrente

    // Friend list (escopo de sessão)
    std::unordered_set<uintptr_t> FriendPlayers;

    // Colors for FOV circles
    ImColor Silent_Fov_Color  = { 0.52f, 0.47f, 0.91f, 1.0f };
    ImColor Aimbot_Fov_Color  = { 1.0f,  1.0f,  1.0f,  1.0f };

    // Vehicle list (thread-safe, updated by UpdateVehicles())
    struct VehicleEntry {
        uintptr_t address;
        Vector3   worldPos;
        uint32_t  lockState;
        char      name[64];
    };
    std::vector<VehicleEntry> VehicleList;
    std::mutex VehicleListMutex;

    // Colors (pure white default)
    ImColor FOV_User          = { 1.f, 1.f, 1.f, 1.f };
    ImColor ESP_NPC           = { 1.f, 1.f, 1.f, 1.f };
    ImColor ESP_Player        = { 1.f, 1.f, 1.f, 1.f };
    ImColor ESP_Skeleton      = { 1.f, 1.f, 1.f, 1.f };

    // Vehicle ESP colors
    ImColor ESP_Veh_Name      = { 1.f, 1.f, 1.f, 1.f };
    ImColor ESP_Veh_Dist      = { 0.9f, 0.9f, 0.9f, 1.f };
    ImColor ESP_Veh_Unlocked  = { 0.2f, 1.0f, 0.2f, 1.f };
    ImColor ESP_Veh_Locked    = { 1.0f, 0.2f, 0.2f, 1.f };
    ImColor ESP_Veh_Inside    = { 1.0f, 0.6f, 0.0f, 1.f };

    void RectFilled(float x0, float y0, float x1, float y1, ImColor color, float rounding, int rounding_corners_flags)
    {
        ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), color, rounding, rounding_corners_flags);
    }
    void HealthBar(float x, float y, float w, float h, int value, int v_max)
    {
        RectFilled(x, y, x + w, y + h, ImColor(0.f, 0.f, 0.f, 0.725f), 0.f, 0);
        RectFilled(x, y, x + w, y + ((h / float(v_max)) * (float)value), ImColor(min(510 * (v_max - value) / 100, 255), min(510 * value / 100, 255), 25, 255), 0.0f, 0);
    }
    void ArmorBar(float x, float y, float w, float h, int value, int v_max)
    {
        RectFilled(x, y, x + w, y + h, ImColor(0.f, 0.f, 0.f, 0.725f), 0.f, 0);
        RectFilled(x, y, x + w, y + ((h / float(v_max)) * (float)value), ImColor(0.f, 0.65f, 1.f, 1.f), 0.0f, 0);
    }
};