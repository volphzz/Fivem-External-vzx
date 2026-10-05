#pragma once
#include "../../../Globals/Globals.h"
#include "../../../Framework/Memory/Memory.h"
#include "../../../Framework/SimpleMath/SimpleMath.h"
using namespace DirectX::SimpleMath;

// Offset manager
class GameSDK
{
public:
    bool InitOffset();

    uintptr_t GetWorld();
    uintptr_t GetCamera();
    uintptr_t GetViewPort();
    uintptr_t GetReplayInterface();

    uintptr_t m_dwWorld;
    uintptr_t m_dwReplayInterface;
    uintptr_t m_dwViewPort;
    uintptr_t m_dwCamera;
};

extern GameSDK* Game;

enum BoneID : int
{
    // Simple skeleton (original 9 bones)
    HEAD,
    LEFTFOOT,
    RIGHTFOOT,
    LEFTANKLE,
    RIGHTANKLE,
    LEFTHAND,
    RIGHTHAND,
    NECK,
    HIP,

    // Complex skeleton — matching GTA5 bone indices used by Rocket
    PELVIS        = 0,
    SPINE_ROOT    = 1,
    SPINE1        = 2,
    SPINE2        = 3,
    SPINE3        = 4,
    NECK_COMPLEX  = 5,
    HEAD_COMPLEX  = 6,
    CLAVICLE_L    = 7,
    CLAVICLE_R    = 8,
    UPPERARM_L    = 9,
    UPPERARM_R    = 10,
    ELBOW_L       = 11,
    ELBOW_R       = 12,
    FOREARM_L     = 13,
    FOREARM_R     = 14,
    HAND_L        = 15,
    HAND_R        = 16,
    THIGH_L       = 17,
    THIGH_R       = 18,
    CALF_L        = 19,
    CALF_R        = 20,
    FOOT_L        = 21,
    FOOT_R        = 22,
    TOE_L         = 23,
    TOE_R         = 24,
};

struct CBone {
    Vector3 position;
    int junk0;
};

struct AllBone {
    CBone bone[9]{};
};

extern bool Vec3_Empty(const Vector3& value);
extern Vector3 Vec3_Transform(Vector3* vIn, Matrix* mIn);
extern Vector3 CalcAngle(Vector3 local_cam, Vector3 to_point);
extern float GetDistance(Vector3 value1, Vector3 value2);
extern bool WorldToScreen(const Matrix& ViewMatrix, const Vector3& vWorld, Vector2& vOut);