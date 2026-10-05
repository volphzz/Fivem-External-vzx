#pragma once

#include <cstdint>

class Cheat;

class SilentAim
{
public:
    void Thread(Cheat* cheat) noexcept;
    void Start(Cheat* cheat) noexcept;

private:
    void Hook();
    uintptr_t BestTarget(Cheat* cheat) noexcept;
    bool IsVisible(uintptr_t ped) const noexcept;

    uintptr_t bulletHandler = 0;
    uintptr_t jmpValue = 0;
    bool hook = false;
    bool patch = false;
};

extern SilentAim silentAim;
