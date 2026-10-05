#include "MagicBullets.h"
#include "Cheat.h"
#include "SDK/Game/offset.h"
#include <array>
#include <chrono>
#include <cmath>
#include <cfloat>
#include <thread>

MagicBullets magicBullets;

namespace
{
    uintptr_t FindSignature(const std::vector<uint8_t>& bytes, const std::vector<int>& signature, uintptr_t moduleBase)
    {
        if (!moduleBase || signature.empty() || bytes.size() < signature.size()) return 0;

        for (size_t i = 0; i + signature.size() <= bytes.size(); ++i)
        {
            bool match = true;
            for (size_t j = 0; j < signature.size(); ++j)
            {
                if (signature[j] >= 0 && bytes[i + j] != static_cast<uint8_t>(signature[j]))
                {
                    match = false;
                    break;
                }
            }
            if (match) return moduleBase + i;
        }
        return 0;
    }

    bool IsOnScreen(const Vector2& point)
    {
        // The td7 implementation only rejects a failed projection, represented by (0, 0).
        return point.x != 0.0f || point.y != 0.0f;
    }

    bool IsVisible(uintptr_t ped)
    {
        const BYTE visibilityFlag = m.Read<BYTE>(ped + offset::VisibleFlag);
        return visibilityFlag != 36 && visibilityFlag != 0 && visibilityFlag != 4;
    }
}

bool MagicBullets::ResolveOffsets()
{
    if (m_offsetsResolved) return m_patchAddress && m_cObjectOffset && m_cWeaponOffset;

    const std::string moduleName = m.GetModuleName();
    const MODULEINFO module = m.GetModuleInfo(moduleName);
    const uintptr_t moduleBase = reinterpret_cast<uintptr_t>(module.lpBaseOfDll);
    if (!moduleBase || module.SizeOfImage == 0) return false;

    const std::vector<uint8_t> image = m.ReadBytes(moduleBase, module.SizeOfImage);
    const uintptr_t magicPatch = FindSignature(image, { 0x0F, 0x29, 0x4F, -1, 0x83, 0x8F, -1, -1, -1, -1, -1, 0x48, 0x8B, 0x4F }, moduleBase);
    const uintptr_t objectSig = FindSignature(image, { 0x4C, 0x8B, 0x41, -1, 0x4D, 0x85, 0xC9 }, moduleBase);
    const uintptr_t weaponSig = FindSignature(image, { 0x48, 0x8B, 0x8B, -1, -1, -1, -1, 0x48, 0x8B, 0xD7, 0xE8, -1, -1, -1, -1, 0x48, 0x8B, 0x8B, -1, -1, -1, -1, 0x48, 0x8B, 0xD7, 0xE8 }, moduleBase);
    if (!magicPatch || !objectSig || !weaponSig) return false;

    m_patchAddress = magicPatch;
    m_cObjectOffset = static_cast<int>(m.Read<uint8_t>(objectSig + 3));
    m_cWeaponOffset = m.Read<int>(weaponSig + 3);
    m_offsetsResolved = m_cObjectOffset != 0 && m_cWeaponOffset != 0;
    return m_offsetsResolved;
}

uintptr_t MagicBullets::GetCWeaponObject() const
{
    const uintptr_t localPed = m.Read<uintptr_t>(Game->GetWorld() + 0x8);
    if (!localPed) return 0;

    const uintptr_t weaponManager = m.Read<uintptr_t>(localPed + offset::m_pWeaponManager);
    const uintptr_t cObject = weaponManager ? m.Read<uintptr_t>(weaponManager + m_cObjectOffset) : 0;
    return cObject ? m.Read<uintptr_t>(cObject + m_cWeaponOffset) : 0;
}

uintptr_t MagicBullets::FindTarget(Cheat* cheat) const
{
    if (!cheat) return 0;

    const uintptr_t localPed = m.Read<uintptr_t>(Game->GetWorld() + 0x8);
    if (!localPed) return 0;

    const Vector3 localPos = m.Read<Vector3>(localPed + offset::m_vecPosition);
    const Matrix viewMatrix = m.Read<Matrix>(Game->GetViewPort() + 0x24C);
    const float centerX = static_cast<float>(g.GameRect.right) / 2.0f;
    const float centerY = static_cast<float>(g.GameRect.bottom) / 2.0f;
    float closestScreenDistance = FLT_MAX;
    uintptr_t closest = 0;
    const auto snapshot = cheat->GetEntitySnapshot();
    if (!snapshot) return 0;

    for (const CPed& source : *snapshot)
    {
        CPed ped = source;
        ped.UpdateStatic();
        ped.Update();

        if (ped.address == localPed || std::fabs(ped.m_flHealth) <= 101.0f || cheat->IsFriend(ped.address)) continue;
        if (g.SilentIgnorePed && !ped.IsPlayer()) continue;
        if (g.SilentVisibleCheck && !IsVisible(ped.address)) continue;

        const float worldDistance = std::hypot(ped.m_vecPosition.x - localPos.x, ped.m_vecPosition.y - localPos.y);
        if (worldDistance > static_cast<float>(g.SilentDistance)) continue;

        // td7 chooses the nearest entity centre first. The default head bone is
        // resolved only after selection, immediately before writing CWeapon + 0x20.
        Vector2 entityToScreen{};
        if (!WorldToScreen(viewMatrix, ped.m_vecPosition, entityToScreen)) continue;

        const float screenDistance = std::hypot(entityToScreen.x - centerX, entityToScreen.y - centerY);
        if (screenDistance < closestScreenDistance)
        {
            closestScreenDistance = screenDistance;
            closest = ped.address;
        }
    }
    return closest;
}

void MagicBullets::Initialize()
{
    m_cWeapon = GetCWeaponObject();
    if (!m_cWeapon || !m_patchAddress) return;

    HANDLE hProc = m.GetProcessHandle();
    DWORD oldProtect = 0;
    const std::array<uint8_t, 4> nops{ 0x90, 0x90, 0x90, 0x90 };
    if (VirtualProtectEx(hProc, reinterpret_cast<void*>(m_patchAddress), nops.size(), PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        WriteProcessMemory(hProc, reinterpret_cast<void*>(m_patchAddress), nops.data(), nops.size(), nullptr);
        VirtualProtectEx(hProc, reinterpret_cast<void*>(m_patchAddress), nops.size(), oldProtect, &oldProtect);
    }
}

void MagicBullets::Restore()
{
    if (!m_patchAddress) return;

    HANDLE hProc = m.GetProcessHandle();
    DWORD oldProtect = 0;
    const std::array<uint8_t, 4> original{ 0x0F, 0x29, 0x4F, 0x20 };
    if (VirtualProtectEx(hProc, reinterpret_cast<void*>(m_patchAddress), original.size(), PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        WriteProcessMemory(hProc, reinterpret_cast<void*>(m_patchAddress), original.data(), original.size(), nullptr);
        VirtualProtectEx(hProc, reinterpret_cast<void*>(m_patchAddress), original.size(), oldProtect, &oldProtect);
    }
}

void MagicBullets::Start(Cheat* cheat, HWND overlayWindow)
{
    while (g.process_active)
    {
        bool isKeyHeld = false;
        if (g.SilentKey != 0)
            isKeyHeld = (GetAsyncKeyState(g.SilentKey) & 0x8000) != 0;
        else
            isKeyHeld = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

        if (g.MagicBulletsEnabled && isKeyHeld && GetForegroundWindow() != overlayWindow && ResolveOffsets())
        {
            const uintptr_t targetAddress = FindTarget(cheat);
            if (targetAddress)
            {
                CPed target{};
                target.address = targetAddress;
                target.UpdateStatic();
                target.Update();

                const Vector3 headPos = target.GetBoneByID(HEAD);
                const Matrix viewMatrix = m.Read<Matrix>(Game->GetViewPort() + 0x24C);
                Vector2 headToScreen{};

                if (WorldToScreen(viewMatrix, headPos, headToScreen) && IsOnScreen(headToScreen))
                {
                    const int fov = static_cast<int>(std::hypot(headToScreen.x - static_cast<float>(g.GameRect.right) / 2.0f,
                                                                headToScreen.y - static_cast<float>(g.GameRect.bottom) / 2.0f));
                    if (fov < g.SilentFov)
                    {
                        if (!m_initialized)
                        {
                            Initialize();
                            m_initialized = m_cWeapon != 0;
                        }

                        if (m_initialized)
                        {
                            const bool miss = g.SilentMissChance >= (std::rand() % 101);
                            const Vector3 bulletStartPos = g.SilentEnabled ? headPos + Vector3(0.0f, 0.0f, 0.3f) : headPos;
                            const Vector3 finalPos = miss ? bulletStartPos + Vector3(0.0f, 0.3f, 0.0f) : bulletStartPos;
                            m.Write<Vector3>(m_cWeapon + 0x20, finalPos);
                        }
                    }
                    else if (m_initialized)
                    {
                        Restore();
                        m_initialized = false;
                    }
                }
                else if (m_initialized)
                {
                    Restore();
                    m_initialized = false;
                }
            }
        }
        else if (m_initialized)
        {
            Restore();
            m_initialized = false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (m_initialized) Restore();
}
