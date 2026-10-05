#pragma once

#include <Windows.h>
#include <cstdint>

class Cheat;

class Triggerbot
{
public:
    void Start(Cheat* cheat, HWND overlayWindow);

private:
    void Shoot(int delay);
    uintptr_t GetClosestPed(Cheat* cheat);
};

extern Triggerbot triggerbot;
