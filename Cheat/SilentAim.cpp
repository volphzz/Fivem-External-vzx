#include "SilentAim.h"
#include "Cheat.h"
#include "SDK/Game/offset.h"
#include <Windows.h>
#include <chrono>
#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <thread>

SilentAim silentAim;

namespace
{
    unsigned char shellcode[] = {
        0xF3, 0x0F, 0x10, 0x1D, 0x18, 0x0B, 0x00, 0x00,
        0xF3, 0x0F, 0x10, 0x05, 0x14, 0x0B, 0x00, 0x00,
        0xF3, 0x0F, 0x10, 0x15, 0x10, 0x0B, 0x00, 0x00,
        0xE9, 0x00, 0x00, 0x00, 0x00
    };

    static bool SafeWriteBytes(uintptr_t address, const void* buffer, size_t size)
    {
        HANDLE hProcess = m.GetProcessHandle();
        if (!hProcess || !address) return false;

        DWORD oldProtect = 0;
        if (VirtualProtectEx(hProcess, reinterpret_cast<LPVOID>(address), size, PAGE_EXECUTE_READWRITE, &oldProtect))
        {
            SIZE_T written = 0;
            BOOL result = WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(address), buffer, size, &written);
            VirtualProtectEx(hProcess, reinterpret_cast<LPVOID>(address), size, oldProtect, &oldProtect);
            return result && (written == size);
        }
        return false;
    }

    template<typename T>
    static bool SafeWrite(uintptr_t address, const T& value)
    {
        return SafeWriteBytes(address, &value, sizeof(T));
    }

    BoneID GetSilentAimBoneID(int boneConfig)
    {
        switch (boneConfig)
        {
        case 1: return HIP;        // Peito / Torso
        case 2: return RIGHTFOOT;  // Pé Direito
        case 3: return LEFTFOOT;   // Pé Esquerdo
        case 4: return RIGHTHAND;  // Mão Direita
        case 5: return LEFTHAND;   // Mão Esquerda
        case 6: return NECK;       // Pescoço
        default: return HEAD;      // Cabeça
        }
    }

    Vector3 GetSilentTargetBone(CPed& ped)
    {
        Vector3 pos = ped.GetBoneByID(GetSilentAimBoneID(g.SilentBone));
        if (!Vec3_Empty(pos))
            return pos;

        Matrix matrix = m.Read<Matrix>(ped.address + 0x60);
        Vector3 rawPos = m.Read<Vector3>(ped.address + 0x410 + GetSilentAimBoneID(g.SilentBone) * 0x10);
        return Vec3_Transform(&rawPos, &matrix);
    }
}

void SilentAim::Hook()
{
    bulletHandler = offset::BulletHandler;
    if (!bulletHandler || !m.m_gClientBaseAddr)
        return;

    uintptr_t jmpAddress = m.m_gClientBaseAddr + bulletHandler + 0x11;
    uintptr_t shellcodeBase = m.m_gClientBaseAddr + 0x3D8;
    int32_t jmpOffset = static_cast<int32_t>(jmpAddress - (shellcodeBase + 25 + 4));
    std::memcpy(&shellcode[25], &jmpOffset, sizeof(jmpOffset));

    SafeWriteBytes(shellcodeBase, shellcode, sizeof(shellcode));
    jmpValue = offset::BulletPosition;
}

void SilentAim::Thread(Cheat* cheat) noexcept
{
    while (g.process_active)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (g.SilentEnableKey != 0 && (GetAsyncKeyState(g.SilentEnableKey) & 1))
            g.SilentEnabled = !g.SilentEnabled;

        if (g.SilentEnabled)
            Start(cheat);
    }
}

bool SilentAim::IsVisible(uintptr_t ped) const noexcept
{
    BYTE visibilityFlag = m.Read<BYTE>(ped + offset::VisibleFlag);
    if (visibilityFlag == 36 || visibilityFlag == 0 || visibilityFlag == 4)
        return false;
    return true;
}

uintptr_t SilentAim::BestTarget(Cheat* cheat) noexcept
{
    uintptr_t closestPlayer = 0;
    float closestDistance = FLT_MAX;
    uintptr_t localAddress = m.Read<uintptr_t>(Game->GetWorld() + 0x8);
    if (!localAddress)
        return 0;

    Vector3 localPosition = m.Read<Vector3>(localAddress + offset::m_vecPosition);
    auto snapshot = cheat->GetEntitySnapshot();
    if (!snapshot)
        return 0;

    float centerX = (float)g.GameRect.right / 2.0f;
    float centerY = (float)g.GameRect.bottom / 2.0f;
    if (centerX <= 0.0f || centerY <= 0.0f)
    {
        centerX = (float)GetSystemMetrics(SM_CXSCREEN) / 2.0f;
        centerY = (float)GetSystemMetrics(SM_CYSCREEN) / 2.0f;
    }

    float effectiveFov = g.SilentIsLegitFov ? g.SilentLegitFov * 10.0f : static_cast<float>(g.SilentFov);
    Matrix viewMatrix = m.Read<Matrix>(Game->GetViewPort() + 0x24C);

    for (const CPed& source : *snapshot)
    {
        CPed player = source;
        player.UpdateStatic();
        player.Update();

        if (player.address == localAddress)
            continue;
        if (g.SilentIgnorePed && !player.IsPlayer())
            continue;
        if (g.SilentIgnoreDead && player.m_flHealth <= 101.0f)
            continue;
        if (g.SilentVisibleCheck && !IsVisible(player.address))
            continue;

        float distance = GetDistance(player.m_vecPosition, localPosition);
        if (distance > static_cast<float>(g.SilentDistance))
            continue;

        Vector3 bonePosition = GetSilentTargetBone(player);
        Vector2 screen{};
        if (!WorldToScreen(viewMatrix, bonePosition, screen))
            continue;

        float crosshairDistance = std::hypot(screen.x - centerX, screen.y - centerY);
        if (crosshairDistance <= effectiveFov && crosshairDistance < closestDistance)
        {
            closestDistance = crosshairDistance;
            closestPlayer = player.address;
        }
    }

    return closestPlayer;
}

void SilentAim::Start(Cheat* cheat) noexcept
{
    if (!hook)
    {
        Hook();
        hook = true;
    }

    uintptr_t player = BestTarget(cheat);

    // Se houver tecla de bind configurada, exige que a tecla esteja pressionada.
    // Se a tecla estiver como 'NONE' (0), ativa ao disparar com o mouse (VK_LBUTTON).
    bool isActivationKeyHeld = false;
    if (g.SilentKey != 0)
        isActivationKeyHeld = (GetAsyncKeyState(g.SilentKey) & 0x8000) != 0;
    else
        isActivationKeyHeld = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

    if (player != 0 && isActivationKeyHeld)
    {
        CPed targetPed{};
        targetPed.address = player;
        targetPed.UpdateStatic();
        targetPed.Update();

        Vector3 bonePosition = GetSilentTargetBone(targetPed);

        if (!patch)
        {
            SafeWrite<uintptr_t>(m.m_gClientBaseAddr + bulletHandler, jmpValue);
            patch = true;
        }

        if (g.SilentLegitHit)
        {
            uintptr_t localPlayer = m.Read<uintptr_t>(Game->GetWorld() + 0x8);
            uintptr_t weaponManager = m.Read<uintptr_t>(localPlayer + offset::m_pWeaponManager);
            uintptr_t weaponInfo = m.Read<uintptr_t>(weaponManager + 0x20);
            uintptr_t ammoInfo = m.Read<uintptr_t>(weaponInfo + 0x60);
            int currentAmmo = m.Read<int>(ammoInfo + 0x8 + 0x18);

            if (currentAmmo < g.SilentLastAmmo)
                g.SilentShotCount++;
            g.SilentLastAmmo = currentAmmo;

            if (g.SilentShotCount >= 1 && g.SilentShotCount <= 4)
            {
                bonePosition.x += ((std::rand() % 300) - 150) / 100.0f;
                bonePosition.y += ((std::rand() % 300) - 150) / 100.0f;
                bonePosition.z += ((std::rand() % 300) - 150) / 100.0f;
            }
            else if (g.SilentShotCount >= 5)
            {
                bonePosition = targetPed.GetBoneByID(HEAD);
                g.SilentShotCount = 0;
            }
        }
        else if (g.SilentMissChance > 0)
        {
            int randomValue = std::rand() % 100;
            if (randomValue < g.SilentMissChance)
            {
                bonePosition.x += ((std::rand() % 200) - 100) / 100.0f;
                bonePosition.y += ((std::rand() % 200) - 100) / 100.0f;
                bonePosition.z += ((std::rand() % 200) - 100) / 100.0f;
            }
        }

        SafeWrite<Vector3>(m.m_gClientBaseAddr + 0xEF8, bonePosition);
    }
    else if (patch)
    {
        SafeWrite<uintptr_t>(m.m_gClientBaseAddr + bulletHandler, 0x0F41F319100F41F3);
        patch = false;
    }
}

