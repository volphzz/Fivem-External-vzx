#pragma once
#include <string>
#include <Windows.h>

struct Globals
{
    // System(Base
    bool process_active = false;
    bool exit_requested = false; // set by user clicking Exit — breaks reconnect loop
    bool ShowMenu = false;

    // GameData
    RECT GameRect{};
    POINT GamePoint{};

    // Aimbot avançado (porta do EXECUTOR)
    bool AimbotEnabled          = false;
    bool AimbotShowFov          = false;
    bool AimbotOnlyVisible      = false;
    bool AimbotIgnoreNPCs       = false;
    bool AimbotPrediction       = false;
    bool AimbotPrioritizeDistance = false;
    bool AimbotAlwaysActive     = false;
    int  AimbotBone             = 0;      // 0=Head 1=Neck 2=Hip 3=LHand 4=RHand 5=LFoot 6=RFoot
    int  AimbotFov              = 180;
    int  AimbotMaxDistance      = 240;
    int  AimbotSpeed            = 12;
    int  AimbotKey              = 0;

    // TriggerBot (porta do EXECUTOR)
    bool TriggerEnabled         = false;
    bool TriggerShowFov         = false;
    bool TriggerIgnoreNPCs      = false;
    int  TriggerFov             = 20;
    int  TriggerMaxDistance     = 200;
    int  TriggerDelay           = 0;
    int  TriggerKey             = 0;

    // Silent Aim / Magic Bullets (porta do EXECUTOR)
    bool  SilentEnabled         = false;
    bool  MagicBulletsEnabled   = false;
    bool  SilentShowFov         = false;
    bool  SilentIsLegitFov      = false;
    bool  SilentIgnorePed       = false;
    bool  SilentIgnoreDead      = true;
    bool  SilentVisibleCheck    = false;
    bool  SilentLegitHit        = false;
    int   SilentBone            = 0;
    int   SilentFov             = 80;
    float SilentLegitFov        = 1.0f;
    int   SilentDistance        = 500;
    int   SilentMissChance      = 0;
    int   SilentKey             = VK_XBUTTON2;
    int   SilentEnableKey       = 0;
    int   SilentShotCount       = 0;
    int   SilentLastAmmo        = 0;

    // Visual
    bool ESP = true;
    bool ESP_Box = true;
    bool ESP_Line = false;
    bool ESP_Name = true;
    bool ESP_Distance = true;
    bool ESP_HealthBar = true;
    bool ESP_Admin = true;            // rainbow ESP for invisible (cloaked/admin) entities
    bool ESP_Skeleton = true;
    int  ESP_SkeletonType = 1;       // 0 = Simple, 1 = Complex
    float ESP_SkeletonThickness = 0.5f;
    float ESP_MaxDistance = 1000.f;
    int  ESP_LineOrigin = 1;         // 0 = Top, 1 = Center, 2 = Bottom

    // Estilos Visuais ESP (EXTERNO-VZX)
    int   ESP_BoxStyle = 0;           // 0 = Full 2D, 1 = Cornered Box, 2 = Rounded Box
    bool  ESP_BoxFilled = false;      // Preenchimento interno translúcido
    float ESP_BoxThickness = 1.0f;   // Espessura da borda da box
    int   ESP_LineStyle = 0;          // 0 = Solida, 1 = Tracejada (Dashed), 2 = Gradiente (Fade)
    float ESP_LineThickness = 1.0f;  // Espessura da snapline
    int   ESP_HeaderStyle = 0;        // 0 = Classic Stacked, 1 = HUD Pro (Badge + Triangulo)

    // Vehicle ESP
    bool ESP_Vehicle = true;
    bool ESP_VehicleName = true;
    bool ESP_VehicleDistance = true;
    bool ESP_VehicleLock = true;
    float ESP_VehicleMaxDistance = 300.f;

    // Misc
    bool GodMode = false;
    bool NoRecoil = false;
    bool NoSpread = false;

    // System(Cheat
    bool Crosshair = false;
    bool StreamProof = false;

    // Key
    int MenuKey = VK_INSERT;
    int AimKey0 = VK_RBUTTON;
    int VehicleLockKey = 0;
};

extern Globals g;
extern bool IsKeyDown(int VK);
extern const char* KeyNames[];