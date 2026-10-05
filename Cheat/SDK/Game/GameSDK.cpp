#include "GameSDK.h"
#include "offset.h"

GameSDK* Game = new GameSDK;

bool GameSDK::InitOffset()
{
    std::string mod = m.GetModuleName();
    bool is3258 = (mod.find("3258") != std::string::npos);

    if (is3258)
    {
        m_dwWorld           = offset::b3258::World;
        m_dwReplayInterface = offset::b3258::ReplayInterface;
        m_dwCamera          = offset::b3258::Camera;
        m_dwViewPort        = offset::b3258::ViewPort;
        offset::BulletHandler  = offset::b3258::BulletHandler;
        offset::BulletPosition = offset::b3258::BulletPosition;
        offset::VisibleFlag    = offset::b3258::VisibleFlag;
        offset::World          = m_dwWorld;
        offset::ReplayInterface= m_dwReplayInterface;
        offset::Camera         = m_dwCamera;
        offset::ViewPort       = m_dwViewPort;
        printf("[GameSDK] Detected FiveM b3258\n");
    }
    else
    {
        // Default: b3095 (as requested)
        m_dwWorld           = offset::b3095::World;
        m_dwReplayInterface = offset::b3095::ReplayInterface;
        m_dwCamera          = offset::b3095::Camera;
        m_dwViewPort        = offset::b3095::ViewPort;
        offset::BulletHandler  = offset::b3095::BulletHandler;
        offset::BulletPosition = offset::b3095::BulletPosition;
        offset::VisibleFlag    = offset::b3095::VisibleFlag;
        offset::World          = m_dwWorld;
        offset::ReplayInterface= m_dwReplayInterface;
        offset::Camera         = m_dwCamera;
        offset::ViewPort       = m_dwViewPort;
        printf("[GameSDK] Using FiveM b3095 offsets\n");
    }

    printf("World           : 0x%llX\n", m_dwWorld);
    printf("ReplayInterface : 0x%llX\n", m_dwReplayInterface);
    printf("Camera          : 0x%llX\n", m_dwCamera);
    printf("ViewPort        : 0x%llX\n", m_dwViewPort);

    return true;
}

uintptr_t GameSDK::GetWorld()
{
    return m.Read<uintptr_t>(m.m_gClientBaseAddr + m_dwWorld);
}

uintptr_t GameSDK::GetCamera()
{
    return m.Read<uintptr_t>(m.m_gClientBaseAddr + m_dwCamera);
}

uintptr_t GameSDK::GetViewPort()
{
    return m.Read<uintptr_t>(m.m_gClientBaseAddr + m_dwViewPort);
}

uintptr_t GameSDK::GetReplayInterface()
{
    return m.Read<uintptr_t>(m.m_gClientBaseAddr + m_dwReplayInterface);
}

bool Vec3_Empty(const Vector3& value)
{
    return value == Vector3(0.f, 0.f, 0.f);
}

Vector3 Vec3_Transform(Vector3* vIn, Matrix* mIn)
{
    Vector3 vOut{};
    vOut.x = (vIn->x * mIn->_11) + (vIn->y * mIn->_21) + (vIn->z * mIn->_31) + mIn->_41;
    vOut.y = (vIn->x * mIn->_12) + (vIn->y * mIn->_22) + (vIn->z * mIn->_32) + mIn->_42;
    vOut.z = (vIn->x * mIn->_13) + (vIn->y * mIn->_23) + (vIn->z * mIn->_33) + mIn->_43;
    return vOut;
}

Vector3 CalcAngle(Vector3 local_cam, Vector3 to_point)
{
    Vector3 vAngle{};
    Vector3 delta = to_point - local_cam;
    float len = delta.Length();

    if (len == 0.f)
        return Vector3();

    vAngle.x = delta.x / len;
    vAngle.y = delta.y / len;
    vAngle.z = delta.z / len;

    return vAngle;
}

float GetDistance(Vector3 value1, Vector3 value2)
{
    return (value1 - value2).Length();
}

bool WorldToScreen(const Matrix& ViewMatrix, const Vector3& vWorld, Vector2& vOut)
{
    Matrix vMatrix = ViewMatrix.Transpose();

    Vector4 vec_x = Vector4(vMatrix._21, vMatrix._22, vMatrix._23, vMatrix._24);
    Vector4 vec_y = Vector4(vMatrix._31, vMatrix._32, vMatrix._33, vMatrix._34);
    Vector4 vec_z = Vector4(vMatrix._41, vMatrix._42, vMatrix._43, vMatrix._44);

    Vector3 vScreen{};
    vScreen.x = (vec_x.x * vWorld.x) + (vec_x.y * vWorld.y) + (vec_x.z * vWorld.z) + vec_x.w;
    vScreen.y = (vec_y.x * vWorld.x) + (vec_y.y * vWorld.y) + (vec_y.z * vWorld.z) + vec_y.w;
    vScreen.z = (vec_z.x * vWorld.x) + (vec_z.y * vWorld.y) + (vec_z.z * vWorld.z) + vec_z.w;

    if (vScreen.z <= 0.1f)
        return false;

    vScreen.z = 1.0f / vScreen.z;
    vScreen.x *= vScreen.z;
    vScreen.y *= vScreen.z;

    float x_temp = (float)g.GameRect.right / 2.f;
    float y_temp = (float)g.GameRect.bottom / 2.f;

    vOut.x = x_temp + (0.5f * vScreen.x * (float)g.GameRect.right + 0.5f);
    vOut.y = y_temp - (0.5f * vScreen.y * (float)g.GameRect.bottom + 0.5f);

    return true;
}