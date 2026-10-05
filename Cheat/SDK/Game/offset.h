#pragma once
#include <cstdint>

namespace offset
{
    // =========================================================
    // Build 3095 offsets (Standard / Recommended for b3095)
    // =========================================================
    namespace b3095
    {
        constexpr std::ptrdiff_t World           = 0x2593320;
        constexpr std::ptrdiff_t ReplayInterface = 0x1F58B58;
        constexpr std::ptrdiff_t ViewPort        = 0x20019E0;
        constexpr std::ptrdiff_t Camera          = 0x2002888;
        constexpr std::ptrdiff_t fragInstNmGTA   = 0x1430;
        constexpr std::ptrdiff_t BulletHandler   = 0x100F5A4;
        constexpr std::uint64_t  BulletPosition  = 0x0F41F3FEFF0E2FE9ULL;
        constexpr std::ptrdiff_t VisibleFlag     = 0x145C;
    }

    // =========================================================
    // Build 3258 offsets
    // =========================================================
    namespace b3258
    {
        constexpr std::ptrdiff_t World           = 0x25B14B0;
        constexpr std::ptrdiff_t ReplayInterface = 0x1FBD4F0;
        constexpr std::ptrdiff_t ViewPort        = 0x201DBA0;
        constexpr std::ptrdiff_t Camera          = 0x201ED50;
        constexpr std::ptrdiff_t fragInstNmGTA   = 0x1430;
        constexpr std::ptrdiff_t BulletHandler   = 0x101A660;
        constexpr std::uint64_t  BulletPosition  = 0x0F41F3FEFE5D73E9ULL;
        constexpr std::ptrdiff_t VisibleFlag     = 0x145C;
    }

    // Default core pointers (b3095)
    inline std::ptrdiff_t World               = b3095::World;
    inline std::ptrdiff_t ReplayInterface     = b3095::ReplayInterface;
    inline std::ptrdiff_t ViewPort            = b3095::ViewPort;
    inline std::ptrdiff_t Camera              = b3095::Camera;
    constexpr std::ptrdiff_t fragInstNmGTA    = 0x1430;
    inline std::ptrdiff_t BulletHandler       = b3095::BulletHandler;
    inline std::uint64_t  BulletPosition      = b3095::BulletPosition;
    inline std::ptrdiff_t VisibleFlag         = b3095::VisibleFlag;

    constexpr std::ptrdiff_t BlipList            = 0x2023400;
    constexpr std::ptrdiff_t AimCped             = 0x202C8D0;
    constexpr std::ptrdiff_t SkySetting          = 0x2721250;

    // Patches / code offsets
    constexpr std::ptrdiff_t ArmsKinematics      = 0x138441C;
    constexpr std::ptrdiff_t LegsKinematics      = 0x1385368;
    constexpr std::ptrdiff_t GiveWeapon          = 0x101E374;
    constexpr std::ptrdiff_t SilentAim           = 0x1038886;
    constexpr std::ptrdiff_t InfiniteAmmo0       = 0x106957C;
    constexpr std::ptrdiff_t InfiniteAmmo1       = 0x10695C1;
    constexpr std::ptrdiff_t MagicBullets        = 0x104EA3E;
    constexpr std::ptrdiff_t InfiniteCombatRoll  = 0xB26061;
    constexpr std::ptrdiff_t OffsetPool          = 0x1198558;
    constexpr std::ptrdiff_t Speed               = 0x70FFAF;
    constexpr std::ptrdiff_t VehicleEngineHealth = 0xF3F808;
    constexpr std::ptrdiff_t Spread              = 0x6EF59F;
    constexpr std::ptrdiff_t Recoil              = 0x262280;
    constexpr std::ptrdiff_t CObject             = 0x3BB209;
    constexpr std::ptrdiff_t CWeapon             = 0x77869B;
    constexpr std::ptrdiff_t VehicleDriver       = 0xF349A0;
    constexpr std::ptrdiff_t NoRagDoll           = 0x60E48C;
    constexpr std::ptrdiff_t SeatBelt            = 0x60E456;
    constexpr std::ptrdiff_t VehicleDoorsLock    = 0x8CEA71;
    constexpr std::ptrdiff_t Handling            = 0xFED018;
    constexpr std::ptrdiff_t Bullet              = 0x101A660;
    constexpr std::ptrdiff_t SetPedIntoVehicle   = 0x7C3E88;

    // =========================================================
    // CPed struct member offsets (b3095 / b3258)
    // =========================================================
    constexpr auto m_pBoneList      = 0x410;
    constexpr auto m_pInfo          = 0x10A8;  // PlayerInfo
    constexpr auto m_pWeaponManager = 0x10B8;  // WeaponManager

    constexpr auto m_bMatrix        = 0x60;
    constexpr auto m_vecPosition    = 0x90;
    constexpr auto m_bGodMode       = 0x189;
    constexpr auto m_flHealth       = 0x280;
    constexpr auto m_flHealthMax    = 0x284;
    constexpr auto m_vecVelocity    = 0x300;
    constexpr auto m_bPedTask       = 0x144B;
    constexpr auto m_flArmor        = 0x150C;
    constexpr auto m_CName          = 0xE0;
    constexpr auto m_lockState      = 0x13C0;
}