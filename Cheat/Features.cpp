#include "Cheat.h"
#include <thread>
#include <chrono>
#include <cmath>

const int ReadCount = 256;

struct Entity {
    uint64_t address;
    uint64_t junk0;
};

struct EntityList_t {
    Entity entity[ReadCount];
};

void Cheat::UpdateList()
{
    while (g.process_active)
    {
        std::vector<CPed> list;
        local.address = m.Read<uintptr_t>(Game->GetWorld() + 0x8);

        // Validate ReplayInterface before reading entity list
        uintptr_t pReplay = Game->GetReplayInterface();
        if (pReplay)
        {
            uintptr_t pEntityList = m.ReadChain(pReplay, { 0x18, 0x100 });
            if (pEntityList)
            {
                EntityList_t entitylist = m.Read<EntityList_t>(pEntityList);

                for (int i = 0; i < ReadCount; i++)
                {
                    if (entitylist.entity[i].address == NULL ||
                        entitylist.entity[i].address == local.address)
                        continue;

                    CPed p = CPed();
                    p.address = entitylist.entity[i].address;
                    p.UpdateStatic();
                    list.push_back(p);
                }
            }
        }

        // Lock, update EntityList e publica snapshot para os módulos de aim
        {
            std::lock_guard<std::mutex> lock(EntityListMutex);
            EntityList = list;
            pEntitySnapshot = std::make_shared<const std::vector<CPed>>(list);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void Cheat::UpdateVehicles()
{
    while (g.process_active)
    {
        std::vector<VehicleEntry> vlist;

        uintptr_t replayInterface = Game->GetReplayInterface();
        if (replayInterface)
        {
            uintptr_t vehicleInterface = m.Read<uintptr_t>(replayInterface + 0x10);
            if (vehicleInterface)
            {
                uintptr_t vehListPtr = m.Read<uintptr_t>(vehicleInterface + 0x180);
                int vehCount         = m.Read<int>(vehicleInterface + 0x188);

                if (vehListPtr && vehCount > 0 && vehCount <= 1024)
                {
                    HANDLE hProc = m.GetProcessHandle();

                    for (int i = 0; i < vehCount; i++)
                    {
                        uintptr_t vehicle = m.Read<uintptr_t>(vehListPtr + (i * 0x10));
                        if (!vehicle) continue;

                        Vector3 pos = m.Read<Vector3>(vehicle + 0x90);
                        if (Vec3_Empty(pos)) continue;

                        VehicleEntry entry{};
                        entry.address   = vehicle;
                        entry.worldPos  = pos;
                        entry.lockState = m.Read<uint32_t>(vehicle + offset::m_lockState);

                        // Read vehicle name from carInfo
                        uintptr_t carInfo = m.Read<uintptr_t>(vehicle + 0x20);
                        if (carInfo && hProc) {
                            ReadProcessMemory(hProc,
                                reinterpret_cast<LPCVOID>(carInfo + 0x298),
                                entry.name, sizeof(entry.name) - 1, nullptr);
                        }
                        if (entry.name[0] == '\0' || (unsigned char)entry.name[0] < 32)
                            strcpy_s(entry.name, "Vehicle");

                        vlist.push_back(entry);
                    }
                }
            }
        }

        {
            std::lock_guard<std::mutex> lock(VehicleListMutex);
            VehicleList = std::move(vlist);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}

void Cheat::Misc()
{
    if (g.GodMode)
    {
        uintptr_t world = Game->GetWorld();
        uintptr_t localPed = world ? m.Read<uintptr_t>(world + 0x8) : 0;
        if (localPed > 0x10000)
        {
            m.Write<bool>(localPed + offset::m_bGodMode, true);
            m.Write<float>(localPed + offset::m_flHealth, 200.0f);
        }
    }
}
