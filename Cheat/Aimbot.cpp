#include "Aimbot.h"
#include "Cheat.h"
#include <algorithm>
#include <cfloat>
#include <chrono>
#include <cmath>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

Aimbot aimbot;

namespace
{
    BoneID GetConfiguredAimBone()
    {
        switch (g.AimbotBone)
        {
        case 1: return NECK;
        case 2: return HIP;
        case 3: return LEFTHAND;
        case 4: return RIGHTHAND;
        case 5: return LEFTFOOT;
        case 6: return RIGHTFOOT;
        default: return HEAD;
        }
    }

    uint32_t GetPhysicsFlags(int16_t losFlags)
    {
        uint32_t result = 0;
        if (losFlags & 0x001) result |= 0x4;
        if (losFlags & 0x002) result |= 0x20000C0;
        if (losFlags & 0x004) result |= 0x200;
        if (losFlags & 0x008) result |= 0x400;
        if (losFlags & 0x010) result |= 0x2000;
        if (losFlags & 0x020) result |= 0x40000;
        if (losFlags & 0x040) result |= 0x4000000;
        if (losFlags & 0x080) result |= 0x8000000;
        if (losFlags & 0x100) result |= 0x80000;
        return result;
    }

    class LineOfSight
    {
    public:
        bool HasClearLos(uintptr_t localPed, uintptr_t targetPed)
        {
            if (!localPed || !targetPed)
                return false;

            if (!resolved)
            {
                if (!Initialize())
                    return false;
            }
            if (!wrapper)
                return false;

            const auto now = std::chrono::steady_clock::now();
            {
                std::lock_guard<std::mutex> lock(callMutex);
                auto it = cache.find(targetPed);
                if (it != cache.end() && (now - it->second.stamp) < cacheTtl)
                    return it->second.visible;
            }

            const bool visible = RemoteCall(localPed, targetPed);
            {
                std::lock_guard<std::mutex> lock(callMutex);
                cache[targetPed] = CacheEntry{ visible, now };
                if (cache.size() > 64)
                {
                    for (auto it = cache.begin(); it != cache.end();)
                    {
                        if ((now - it->second.stamp) > cacheTtl * 4)
                            it = cache.erase(it);
                        else
                            ++it;
                    }
                }
            }
            return visible;
        }

    private:
        struct CacheEntry
        {
            bool visible = false;
            std::chrono::steady_clock::time_point stamp{};
        };

        static constexpr int16_t defaultLosFlags = 0x1 | 0x2 | 0x10 | 0x100;
        static constexpr const char* wrapperSignature = "48 83 EC 48 8A 44 24 ? 0F 28 05";
        static constexpr auto cacheTtl = std::chrono::milliseconds(50);
        static constexpr DWORD remoteTimeoutMs = 100;

        uintptr_t wrapper = 0;
        bool resolved = false;
        bool resolveAttempted = false;
        std::mutex callMutex;
        std::unordered_map<uintptr_t, CacheEntry> cache;

        uintptr_t FindPatternWildcard(uintptr_t base, size_t size, const char* pattern)
        {
            std::vector<int> bytes;
            std::string source(pattern);
            for (size_t i = 0; i < source.size();)
            {
                if (source[i] == ' ' || source[i] == '\t')
                {
                    ++i;
                    continue;
                }
                if (source[i] == '?')
                {
                    bytes.push_back(-1);
                    ++i;
                    if (i < source.size() && source[i] == '?')
                        ++i;
                }
                else
                {
                    bytes.push_back(static_cast<int>(std::strtol(source.substr(i, 2).c_str(), nullptr, 16)));
                    i += 2;
                }
            }

            if (bytes.empty() || !base || !size)
                return 0;

            const size_t patternLength = bytes.size();
            const size_t chunkSize = 0x400000;
            for (size_t offset = 0; offset < size;)
            {
                size_t remaining = size - offset;
                size_t readLength = (std::min)(chunkSize, remaining);
                std::vector<uint8_t> buffer = m.ReadBytes(base + offset, readLength);
                if (buffer.size() >= patternLength)
                {
                    for (size_t j = 0; j + patternLength <= buffer.size(); ++j)
                    {
                        bool matches = true;
                        for (size_t k = 0; k < patternLength; ++k)
                        {
                            if (bytes[k] >= 0 && buffer[j + k] != static_cast<uint8_t>(bytes[k]))
                            {
                                matches = false;
                                break;
                            }
                        }
                        if (matches)
                            return base + offset + j;
                    }
                }

                if (remaining <= chunkSize)
                    break;
                offset += chunkSize - (patternLength - 1);
            }
            return 0;
        }

        bool Initialize()
        {
            if (resolved)
                return wrapper != 0;
            if (resolveAttempted)
                return false;
            resolveAttempted = true;

            MODULEINFO module = m.GetModuleInfo(m.GetModuleName());
            uintptr_t base = m.m_gClientBaseAddr;
            size_t size = module.SizeOfImage ? module.SizeOfImage : 0x7000000;
            if (!base)
                return false;

            wrapper = FindPatternWildcard(base, size, wrapperSignature);
            resolved = wrapper != 0;
            return resolved;
        }

        bool RemoteCall(uintptr_t localPed, uintptr_t targetPed)
        {
            std::lock_guard<std::mutex> lock(callMutex);
            HANDLE process = m.GetProcessHandle();
            const uint32_t includeFlags = GetPhysicsFlags(defaultLosFlags);

            std::vector<uint8_t> shellcode = {
                0x48, 0x83, 0xEC, 0x28,
                0xC6, 0x44, 0x24, 0x20, 0x00,
                0x48, 0xB9, 0,0,0,0,0,0,0,0,
                0x48, 0xBA, 0,0,0,0,0,0,0,0,
                0x41, 0xB8, 0x01, 0x00, 0x00, 0x00,
                0x41, 0xB9, 0,0,0,0,
                0x48, 0xB8, 0,0,0,0,0,0,0,0,
                0xFF, 0xD0,
                0x48, 0xB9, 0,0,0,0,0,0,0,0,
                0x88, 0x01,
                0x48, 0x83, 0xC4, 0x28,
                0x33, 0xC0,
                0xC3
            };

            constexpr size_t offLocal = 11;
            constexpr size_t offTarget = 21;
            constexpr size_t offFlags = 37;
            constexpr size_t offWrapper = 43;
            constexpr size_t offResult = 55;
            const size_t codeSize = shellcode.size();
            const size_t caveSize = codeSize + 8;
            uintptr_t cave = reinterpret_cast<uintptr_t>(VirtualAllocEx(
                process, nullptr, caveSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
            if (!cave)
                return false;

            uintptr_t resultAddress = cave + codeSize;
            std::memcpy(&shellcode[offLocal], &localPed, 8);
            std::memcpy(&shellcode[offTarget], &targetPed, 8);
            std::memcpy(&shellcode[offFlags], &includeFlags, 4);
            std::memcpy(&shellcode[offWrapper], &wrapper, 8);
            std::memcpy(&shellcode[offResult], &resultAddress, 8);

            SIZE_T written = 0;
            if (!WriteProcessMemory(process, reinterpret_cast<void*>(cave), shellcode.data(), shellcode.size(), &written) || written != shellcode.size())
            {
                VirtualFreeEx(process, reinterpret_cast<void*>(cave), 0, MEM_RELEASE);
                return false;
            }
            m.Write<uint8_t>(resultAddress, 0);

            bool visible = false;
            HANDLE thread = CreateRemoteThread(process, nullptr, 0,
                reinterpret_cast<LPTHREAD_START_ROUTINE>(cave), nullptr, 0, nullptr);
            if (!thread)
            {
                VirtualFreeEx(process, reinterpret_cast<void*>(cave), 0, MEM_RELEASE);
                return false;
            }

            DWORD wait = WaitForSingleObject(thread, remoteTimeoutMs);
            if (wait == WAIT_OBJECT_0)
                visible = m.Read<uint8_t>(resultAddress) != 0;
            else
                WaitForSingleObject(thread, 3000);

            CloseHandle(thread);
            VirtualFreeEx(process, reinterpret_cast<void*>(cave), 0, MEM_RELEASE);
            return visible;
        }
    } lineOfSight;
}

uintptr_t Aimbot::GetClosestPed(Cheat* cheat)
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
        if (g.AimbotIgnoreNPCs && !ped.IsPlayer())
            continue;
        if (g.AimbotOnlyVisible && !lineOfSight.HasClearLos(localPed, ped.address))
            continue;

        float dx = ped.m_vecPosition.x - localPosition.x;
        float dy = ped.m_vecPosition.y - localPosition.y;
        float distance = std::sqrt(dx * dx + dy * dy);
        if (distance > g.AimbotMaxDistance)
            continue;

        Vector2 pedPosition{};
        const Vector3 aimPosition = ped.GetBoneByID(GetConfiguredAimBone());
        if (!WorldToScreen(viewMatrix, aimPosition, pedPosition))
            continue;
        float x = pedPosition.x - (float)g.GameRect.right / 2.0f;
        float y = pedPosition.y - (float)g.GameRect.bottom / 2.0f;
        float aimDistance = std::sqrt(x * x + y * y);
        if (aimDistance > g.AimbotFov)
            continue;

        const float score = g.AimbotPrioritizeDistance ? distance : aimDistance;
        if (score < closestDistance)
        {
            closestDistance = score;
            closestPed = ped.address;
        }
    }
    return closestPed;
}

void Aimbot::SetViewAngles(uintptr_t ped, const Vector3& bonePosition)
{
    uintptr_t cameraDirector = Game->GetCamera();
    if (!cameraDirector)
        return;

    uintptr_t followPedCamera = m.Read<uintptr_t>(cameraDirector + 0x2C0);
    if (!followPedCamera)
        return;

    uintptr_t followVehicle = m.Read<uintptr_t>(followPedCamera + 0x10);
    uintptr_t localAddress = m.Read<uintptr_t>(Game->GetWorld() + 0x8);
    CPed local{};
    local.address = localAddress;
    local.UpdateStatic();
    local.Update();

    if (local.InVehicle() && followVehicle)
    {
        if (m.Read<float>(followVehicle + 0x2AC) == -2.0f)
        {
            m.Write<float>(followVehicle + 0x2AC, 0.0f);
            m.Write<float>(followVehicle + 0x2C0, 111.0f);
            m.Write<float>(followVehicle + 0x2C4, 111.0f);
        }
    }

    Vector3 cameraPosition = m.Read<Vector3>(followPedCamera + 0x60);
    Vector3 currentViewAngles = m.Read<Vector3>(followPedCamera + 0x40);
    if (currentViewAngles.LengthSquared() > 0.0001f)
        currentViewAngles.Normalize();
    else
        currentViewAngles = Vector3(0.0f, 1.0f, 0.0f);

    Vector3 targetViewAngles = bonePosition - cameraPosition;
    if (targetViewAngles.LengthSquared() > 0.0001f)
        targetViewAngles.Normalize();
    else
        return;

    if (g.AimbotSpeed > 1)
    {
        Vector3 delta = targetViewAngles - currentViewAngles;
        Vector3 finalAngles(
            currentViewAngles.x + delta.x / g.AimbotSpeed,
            currentViewAngles.y + delta.y / g.AimbotSpeed,
            currentViewAngles.z + delta.z / g.AimbotSpeed);

        if (finalAngles.LengthSquared() > 0.0001f)
            finalAngles.Normalize();
        else
            return;

        m.Write<Vector3>(followPedCamera + 0x40, finalAngles);
        m.Write<Vector3>(followPedCamera + 0x3D0, finalAngles);
    }
    else
    {
        m.Write<Vector3>(followPedCamera + 0x40, targetViewAngles);
        m.Write<Vector3>(followPedCamera + 0x3D0, targetViewAngles);
    }
}

void Aimbot::Start(Cheat* cheat, HWND overlayWindow)
{
    while (g.process_active)
    {
        int effectiveKey = g.AimbotKey ? g.AimbotKey : g.AimKey0;
        const bool activationHeld = g.AimbotAlwaysActive ||
            (effectiveKey && (GetAsyncKeyState(effectiveKey) & 0x8000));
        if (g.AimbotEnabled && activationHeld &&
            GetForegroundWindow() != overlayWindow)
        {
            uintptr_t pedAddress = GetClosestPed(cheat);
            if (!pedAddress)
                continue;

            CPed ped{};
            ped.address = pedAddress;
            ped.UpdateStatic();
            ped.Update();
            Vector3 aimPosition = ped.GetBoneByID(GetConfiguredAimBone());
            if (g.AimbotPrediction)
                aimPosition += m.Read<Vector3>(pedAddress + 0x320) * 0.075f;
            Matrix viewMatrix = m.Read<Matrix>(Game->GetViewPort() + 0x24C);
            Vector2 screenAimPosition{};

            if (WorldToScreen(viewMatrix, aimPosition, screenAimPosition))
            {
                int fov = (int)std::hypot(
                    screenAimPosition.x - (float)g.GameRect.right / 2.0f,
                    screenAimPosition.y - (float)g.GameRect.bottom / 2.0f);
                if (fov < g.AimbotFov)
                    SetViewAngles(pedAddress, aimPosition + Vector3(0.0f, 0.0f, 0.08f));
            }
        }
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }
}
