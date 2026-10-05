#include "CPed.h"
#include <immintrin.h>

struct Mat34V
{
    __m128 m_col0;
    __m128 m_col1;
    __m128 m_col2;
    __m128 m_col3;
};

struct Mat34Vi
{
    __m128i m_col0;
    __m128i m_col1;
    __m128i m_col2;
    __m128i m_col3;
};

inline void Transform_Imp34(
    Mat34V* inoutMat, __m128* transform1_col0, __m128* transform1_col1, __m128* transform1_col2, const __m128* transform1_col3,
    __m128i* transform2_col0, __m128i* transform2_col1, __m128i* transform2_col2, __m128i* transform2_col3)
{
    __m128 v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;

    v9 = *transform1_col3;
    v10 = *transform1_col1;
    v11 = (__m128)_mm_castsi128_ps(_mm_shuffle_epi32(*transform2_col2, 0));
    v12 = (__m128)_mm_castsi128_ps(_mm_shuffle_epi32(*transform2_col2, 85));
    v13 = (__m128)_mm_castsi128_ps(_mm_shuffle_epi32(*transform2_col2, 170));
    v14 = *transform1_col2;
    v15 = (__m128)_mm_castsi128_ps(_mm_shuffle_epi32(*transform2_col3, 0));
    v16 = (__m128)_mm_castsi128_ps(_mm_shuffle_epi32(*transform2_col3, 85));
    v17 = (__m128)_mm_castsi128_ps(_mm_shuffle_epi32(*transform2_col3, 170));
    v18 = _mm_mul_ps(*transform1_col2, (__m128)_mm_castsi128_ps(_mm_shuffle_epi32(*transform2_col1, 170)));
    v19 = _mm_add_ps(_mm_mul_ps(*transform1_col1, (__m128)_mm_castsi128_ps(_mm_shuffle_epi32(*transform2_col1, 85))), _mm_mul_ps(*transform1_col0, (__m128)_mm_castsi128_ps(_mm_shuffle_epi32(*transform2_col1, 0))));
    v20 = *transform1_col0;
    inoutMat->m_col0 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(*transform1_col1, (__m128)_mm_castsi128_ps(_mm_shuffle_epi32(*transform2_col0, 85))), _mm_mul_ps(*transform1_col0, (__m128)_mm_castsi128_ps(_mm_shuffle_epi32(*transform2_col0, 0)))), _mm_mul_ps(*transform1_col2, (__m128)_mm_castsi128_ps(_mm_shuffle_epi32(*transform2_col0, 170))));
    inoutMat->m_col1 = _mm_add_ps(v19, v18);
    inoutMat->m_col3 = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(v20, v15), v9), _mm_mul_ps(v10, v16)), _mm_mul_ps(v14, v17));
    inoutMat->m_col2 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(v10, v12), _mm_mul_ps(v20, v11)), _mm_mul_ps(v14, v13));
}

bool CPed::IsPlayer()
{
    return player_info != NULL;
}

#include "../../ServerDumper.h"

std::string CPed::GetName()
{
    if (!player_info)
        return IsPlayer() ? "Player" : "NPC";

    // 1. Read FiveM server player ID
    int playerId = m.Read<int>(player_info + 0xE8);
    if (playerId <= 0 || playerId > 65535) {
        playerId = m.Read<int>(player_info + 0x88);
    }
    if (playerId <= 0 || playerId > 65535) {
        playerId = m.Read<int>(player_info + 0xEC);
    }

    // 2. Check if ServerDumper has the name for this ID (Rocket V14 server dumper)
    if (playerId > 0 && playerId < 65535) {
        std::string dumpedName = ServerDumper::GetName(playerId);
        if (!dumpedName.empty()) {
            return dumpedName;
        }
    }

    // 3. Try reading inline name at 0xE0 / 0xFC / 0x88
    char buf[64]{};
    HANDLE hProc = m.GetProcessHandle();
    ReadProcessMemory(hProc, reinterpret_cast<LPCVOID>(player_info + offset::m_CName), buf, sizeof(buf) - 1, nullptr);
    if (buf[0] != '\0' && (unsigned char)buf[0] >= 32 && (unsigned char)buf[0] <= 126 && strcmp(buf, "Player") != 0 && strcmp(buf, "player") != 0) {
        buf[sizeof(buf) - 1] = '\0';
        return std::string(buf);
    }

    ReadProcessMemory(hProc, reinterpret_cast<LPCVOID>(player_info + 0xFC), buf, sizeof(buf) - 1, nullptr);
    if (buf[0] != '\0' && (unsigned char)buf[0] >= 32 && (unsigned char)buf[0] <= 126 && strcmp(buf, "Player") != 0 && strcmp(buf, "player") != 0) {
        buf[sizeof(buf) - 1] = '\0';
        return std::string(buf);
    }

    // 4. If we have a server player ID, display "ID: X" (just like Rocket when name is pending)
    if (playerId > 0 && playerId < 65535) {
        return "ID: " + std::to_string(playerId);
    }

    return "Player";
}



bool CPed::IsDead()
{
    return m_flHealth <= 0 || Vec3_Empty(m_vecPosition);
}

bool CPed::InVehicle()
{
    return false;
}

// Porta exata da lógica Rocket V14 (Render.cpp / esp.cpp)
// Retorna true somente para jogadores REALMENTE invisíveis (admin/cloak)
// Jogadores visíveis atrás de objetos = false
bool CPed::IsInvisible()
{
    if (!player_info) return false; // só aplica para players, não NPCs

    bool result = false;

    // 1. Checar pNetObject (offset 0xD0)
    uintptr_t pNet = m.Read<uintptr_t>(address + 0xD0);
    if (!pNet) {
        // Sem pNetObject mas tem nome e health > 101 → admin em spectate
        float health = m.Read<float>(address + offset::m_flHealth);
        std::string name = GetName();
        if (!name.empty() && health > 101.f)
            result = true;
    } else {
        // Flag de invisibilidade forçada no pNetObject (0xD1)
        if (m.Read<uint8_t>(pNet + 0xD1) == 1)
            result = true;
    }

    // 2. entityFlag bit 0 (CEntity + 0x2C) e alpha (CEntity + 0xAC)
    if (!result) {
        uint8_t entityFlag  = m.Read<uint8_t>(address + 0x2C);
        int     entityAlpha = m.Read<int>(address + 0xAC);
        if ((entityFlag & 0x01) == 0 || entityAlpha == 0)
            result = true;
    }

    // 3. Flag 0xD1 no próprio ped
    if (!result) {
        if (m.Read<uint8_t>(address + 0xD1) == 1)
            result = true;
    }

    // 4. configFlag bit 12 (offset 0x1414 para b3095)
    if (!result) {
        uint32_t cfgFlag = m.Read<uint32_t>(address + 0x1414);
        if ((cfgFlag >> 12) & 1) {
            float health = m.Read<float>(address + offset::m_flHealth);
            std::string name = GetName();
            if (health > 101.f && !name.empty())
                result = true;
        }
    }

    return result;
}

bool CPed::Update()
{
    m_flHealth = m.Read<float>(address + offset::m_flHealth);
    m_vecPosition = m.Read<Vector3>(address + offset::m_vecPosition);

    if (IsDead())
        return false;

    m_flArmor = m.Read<float>(address + offset::m_flArmor);
    m_bMatrix = m.Read<Matrix>(address + offset::m_bMatrix);

    return true;
}

void CPed::UpdateStatic()
{
    // pInfo
    player_info = m.Read<uintptr_t>(address + offset::m_pInfo);

    // Weapon
    uintptr_t weapon_mgr = m.Read<uintptr_t>(address + offset::m_pWeaponManager);
    current_weapon = m.Read<uintptr_t>(weapon_mgr + 0x20);

    // player static value
    m_flMaxHealth = m.Read<float>(address + offset::m_flHealthMax);

    // crSkeletonData
    crSkeletonData = 0;
    MaskToBoneId.clear();
    uintptr_t fragInst = m.Read<uintptr_t>(address + offset::fragInstNmGTA);
    if (fragInst) {
        uintptr_t v1 = m.Read<uintptr_t>(fragInst + 0x68);
        if (v1) {
            crSkeletonData = m.Read<uintptr_t>(v1 + 0x178);
        }
    }
}

Vector3 CPed::GetVelocity()
{
    return m.Read<Vector3>(address + offset::m_vecVelocity);
}

Vector3 CPed::GetBoneByID(BoneID id)
{
    Vector3 pos = m.Read<Vector3>(address + offset::m_pBoneList + (id * 0x10));
    return Vec3_Transform(&pos, &m_bMatrix);
}

std::vector<Vector3> CPed::GetBoneList()
{
    constexpr int BoneCount = 9;
    std::vector<Vector3> list(BoneCount);
    for (int b = 0; b < BoneCount; b++) {
        Vector3 pos = m.Read<Vector3>(address + offset::m_pBoneList + (b * 0x10));
        list[b] = Vec3_Transform(&pos, &m_bMatrix);
    }
    return list;
}

bool CPed::GetPedBoneIndex(int BoneMask, unsigned int& BoneId)
{
    if (!crSkeletonData) return false;
    uintptr_t m_pSkeleton_data = m.Read<uintptr_t>(crSkeletonData + 0);
    if (!m_pSkeleton_data) return false;

    if (m.Read<WORD>(m_pSkeleton_data + 0x1A)) {
        if (m.Read<WORD>(m_pSkeleton_data + 0x18)) {
            uintptr_t v1 = m.Read<uintptr_t>(m_pSkeleton_data + 0x10);
            unsigned short v2 = m.Read<unsigned short>(m_pSkeleton_data + 0x18);
            if (v2 == 0 || BoneMask == 0)
                return false;

            uintptr_t v3 = m.Read<uintptr_t>(v1 + 0x8 * (BoneMask % v2));
            int count = 0;
            for (uintptr_t i = v3; i != 0; i = m.Read<uintptr_t>(i + 0x8)) {
                count++;
                if (count > 64) break;
                int v5 = m.Read<int>(i);
                if (BoneMask == v5) {
                    BoneId = (unsigned int)m.Read<int>(i + 0x4);
                    return true;
                }
            }
        }
    }
    return false;
}

Vector3 CPed::GetBonePosByInstFragAndID(unsigned int BoneId)
{
    if (!crSkeletonData) return Vector3{};
    uintptr_t m_parent = m.Read<uintptr_t>(crSkeletonData + 0x8);
    if (m_parent) {
        Mat34V ptrcol = m.Read<Mat34V>(m_parent);
        uintptr_t v5 = m.Read<uintptr_t>(crSkeletonData + 0x18) + ((unsigned __int64)BoneId << 6);
        Mat34Vi v5col = m.Read<Mat34Vi>(v5);

        Mat34V resultMat;
        Transform_Imp34(&resultMat, &ptrcol.m_col0, &ptrcol.m_col1, &ptrcol.m_col2, &ptrcol.m_col3,
                        &v5col.m_col0, &v5col.m_col1, &v5col.m_col2, &v5col.m_col3);

        float* out = reinterpret_cast<float*>(&resultMat.m_col3);
        return Vector3(out[0], out[1], out[2]);
    }
    return Vector3{};
}

Vector3 CPed::GetBonePos(int BoneMask)
{
    if (!crSkeletonData) {
        uintptr_t fragInst = m.Read<uintptr_t>(address + offset::fragInstNmGTA);
        if (fragInst) {
            uintptr_t v1 = m.Read<uintptr_t>(fragInst + 0x68);
            if (v1) {
                crSkeletonData = m.Read<uintptr_t>(v1 + 0x178);
            }
        }
    }

    if (crSkeletonData) {
        auto it = MaskToBoneId.find(BoneMask);
        if (it == MaskToBoneId.end()) {
            unsigned int BoneId = 0;
            if (GetPedBoneIndex(BoneMask, BoneId)) {
                if (BoneId) {
                    MaskToBoneId[BoneMask] = BoneId;
                    return GetBonePosByInstFragAndID(BoneId);
                }
            }
        } else {
            return GetBonePosByInstFragAndID(it->second);
        }
    }

    return Vector3{};
}