#pragma once

#include <Windows.h>
#include <cstdint>
#include "../Framework/SimpleMath/SimpleMath.h"

class Cheat;

class Aimbot
{
public:
    void Start(Cheat* cheat, HWND overlayWindow);

private:
    void SetViewAngles(uintptr_t ped, const DirectX::SimpleMath::Vector3& bonePosition);
    uintptr_t GetClosestPed(Cheat* cheat);
};

extern Aimbot aimbot;
