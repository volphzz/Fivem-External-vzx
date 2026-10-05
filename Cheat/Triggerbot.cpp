#include "Triggerbot.h"
#include "Cheat.h"
#include <cfloat>
#include <chrono>
#include <cmath>
#include <thread>

Triggerbot triggerbot;

void Triggerbot::Shoot(int delay)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    std::this_thread::sleep_for(std::chrono::nanoseconds(500));
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

uintptr_t Triggerbot::GetClosestPed(Cheat* cheat)
{
    uintptr_t closestPed = 0;
    float closestDistance = FLT_MAX;
    uintptr_t localPed = m.Read<uintptr_t>(Game->GetWorld() + 0x8);
    if (!localPed)
        return 0;

    Vector3 localPosition = m.Read<Vector3>(localPed + offset::m_vecPosition);
    Matrix viewMatrix = m.Read<Matrix>(Game->GetViewPort() + 0x24C);
    auto snapshot = cheat->GetEntitySnapshot();
    if (!snapshot)
        return 0;

    for (const CPed& source : *snapshot)
    {
        CPed ped = source;
        ped.UpdateStatic();
        ped.Update();

        if (ped.address == localPed)
            continue;
        if (std::abs(ped.m_flHealth) <= 101.0f)
            continue;
        if (g.TriggerIgnoreNPCs && !ped.IsPlayer())
            continue;

        float dx = ped.m_vecPosition.x - localPosition.x;
        float dy = ped.m_vecPosition.y - localPosition.y;
        float distance = std::sqrt(dx * dx + dy * dy);
        if (distance > g.TriggerMaxDistance)
            continue;

        Vector2 screenHead{};
        if (!WorldToScreen(viewMatrix, ped.GetBoneByID(HEAD), screenHead))
            continue;

        float x = screenHead.x - (float)g.GameRect.right / 2.0f;
        float y = screenHead.y - (float)g.GameRect.bottom / 2.0f;
        float aimDistance = std::sqrt(x * x + y * y);
        if (aimDistance < closestDistance)
        {
            closestDistance = aimDistance;
            closestPed = ped.address;
        }
    }

    return closestPed;
}

void Triggerbot::Start(Cheat* cheat, HWND overlayWindow)
{
    while (g.process_active)
    {
        bool triggerHeld = (g.TriggerKey != 0) ? ((GetAsyncKeyState(g.TriggerKey) & 0x8000) != 0) : ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0);
        if (g.TriggerEnabled && triggerHeld &&
            GetForegroundWindow() != overlayWindow)
        {
            uintptr_t pedAddress = GetClosestPed(cheat);
            if (pedAddress)
            {
                CPed ped{};
                ped.address = pedAddress;
                ped.UpdateStatic();
                ped.Update();

                Matrix viewMatrix = m.Read<Matrix>(Game->GetViewPort() + 0x24C);
                Vector2 screenHead{};
                if (WorldToScreen(viewMatrix, ped.GetBoneByID(HEAD), screenHead))
                {
                    int fov = (int)std::hypot(
                        screenHead.x - (float)g.GameRect.right / 2.0f,
                        screenHead.y - (float)g.GameRect.bottom / 2.0f);
                    if (fov < g.TriggerFov)
                        Shoot(g.TriggerDelay);
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }
}
