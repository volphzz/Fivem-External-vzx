#pragma once

#include <Windows.h>
#include <cstdint>

class Cheat;

class MagicBullets
{
public:
    void Start(Cheat* cheat, HWND overlayWindow);

private:
    bool ResolveOffsets();
    uintptr_t GetCWeaponObject() const;
    uintptr_t FindTarget(Cheat* cheat) const;
    void Initialize();
    void Restore();

    uintptr_t m_patchAddress = 0;
    int m_cObjectOffset = 0;
    int m_cWeaponOffset = 0;
    uintptr_t m_cWeapon = 0;
    bool m_offsetsResolved = false;
    bool m_initialized = false;
};

extern MagicBullets magicBullets;
