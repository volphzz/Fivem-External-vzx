#pragma once
#include "../Game/GameSDK.h"
#include "../Game/offset.h"
#include <unordered_map>
#include <vector>

enum eAnimBoneTag : int {
    BONETAG_NULL        = -1,
    BONETAG_ROOT        = 0,
    BONETAG_PELVIS      = 11816,
    BONETAG_SPINE       = 23553,
    BONETAG_SPINE1      = 24816,
    BONETAG_SPINE2      = 24817,
    BONETAG_SPINE3      = 24818,
    BONETAG_NECK        = 39317,
    BONETAG_HEAD        = 31086,
    BONETAG_R_CLAVICLE  = 10706,
    BONETAG_R_UPPERARM  = 40269,
    BONETAG_R_FOREARM   = 28252,
    BONETAG_R_HAND      = 57005,
    BONETAG_L_CLAVICLE  = 64729,
    BONETAG_L_UPPERARM  = 45509,
    BONETAG_L_FOREARM   = 61163,
    BONETAG_L_HAND      = 18905,
    BONETAG_L_THIGH     = 58271,
    BONETAG_L_CALF      = 63931,
    BONETAG_L_FOOT      = 14201,
    BONETAG_L_TOE       = 2108,
    BONETAG_R_THIGH     = 51826,
    BONETAG_R_CALF      = 36864,
    BONETAG_R_FOOT      = 52301,
    BONETAG_R_TOE       = 20781,
    MH_R_Elbow          = 0xBB0,
    MH_L_Elbow          = 0x58B7,
};

class CPed
{
public:
    uintptr_t address;

    uintptr_t player_info;
    uintptr_t current_weapon;
    uintptr_t crSkeletonData;
    std::unordered_map<int, unsigned int> MaskToBoneId;

    float m_flHealth;
    float m_flArmor;
    float m_flMaxHealth;
    Vector3 m_vecPosition;
    Matrix m_bMatrix; // BoneMatrix

    bool Update();
    void UpdateStatic();
    Vector3 GetVelocity();
    std::string GetName();
    Vector3 GetBoneByID(BoneID id);
    Vector3 GetBonePos(int BoneMask);
    bool GetPedBoneIndex(int BoneMask, unsigned int& BoneId);
    Vector3 GetBonePosByInstFragAndID(unsigned int BoneId);
    std::vector<Vector3> GetBoneList();

    bool IsDead();
    bool IsPlayer();
    bool InVehicle();
    bool IsInvisible(); // alpha == 0 → admin/cloaked
};