#include "Cheat.h"
#include "../Framework/Fonts/Fonts.hpp"
#include <cmath>

// ─── Modern outlined text helper with Inter font support (EXTERNO-VZX Style) ────
static inline void DrawOutlinedText(const std::string& text, const ImVec2& pos,
                                    ImColor col, ImColor borderCol = ImColor(0.f, 0.f, 0.f, 1.f),
                                    bool center = true, ImFont* font = nullptr, float fontSize = 13.0f)
{
    if (!font)
        font = FWork::Fonts::InterSemiBold ? FWork::Fonts::InterSemiBold : ImGui::GetFont();

    ImVec2 textSize = font ? font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text.c_str()) : ImGui::CalcTextSize(text.c_str());
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    // Contorno 8-way para máxima nitidez (mesmo padrão do EXTERNO-VZX)
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if (i != 0 || j != 0) {
                float outlineX = (float)i;
                float outlineY = (float)j;
                ImVec2 outlinePos = ImVec2(pos.x + outlineX, pos.y + outlineY);

                if (center) {
                    drawList->AddText(font, fontSize,
                        ImVec2(outlinePos.x - textSize.x / 2.0f, outlinePos.y),
                        borderCol, text.c_str());
                } else {
                    drawList->AddText(font, fontSize, outlinePos, borderCol, text.c_str());
                }
            }
        }
    }

    if (center) {
        ImVec2 textPos = ImVec2(pos.x - textSize.x / 2.0f, pos.y);
        drawList->AddText(font, fontSize, textPos, col, text.c_str());
    } else {
        drawList->AddText(font, fontSize, pos, col, text.c_str());
    }
}

// ─── Rainbow color (cycles hue ~3 seconds per full rotation) ─────────────────
static inline ImColor RainbowColor(float speed = 3.f)
{
    float hue = fmodf((float)ImGui::GetTime() * speed / 10.f, 1.f);
    float r, g, b;
    ImGui::ColorConvertHSVtoRGB(hue, 1.f, 1.f, r, g, b);
    return ImColor(r, g, b, 1.f);
}

// ─── Box Styles (EXTERNO-VZX) ────────────────────────────────────────────────
static inline void DrawFullBox(float x, float y, float w, float h, ImColor color, float thickness) {
    auto dl = ImGui::GetBackgroundDrawList();
    float boxThick = (std::max)(1.0f, thickness);
    dl->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), IM_COL32(0, 0, 0, 180), 0.0f, 0, boxThick + 1.6f);
    dl->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), color, 0.0f, 0, boxThick);
}

static inline void DrawCorneredBox(float x, float y, float w, float h, ImColor color, float thickness) {
    auto dl = ImGui::GetBackgroundDrawList();
    float cornerThick = (std::max)(1.2f, thickness * 1.35f);
    float lineW = w / 3.5f;
    float lineH = h / 3.5f;

    auto AddLineH = [&](float x_start, float y_pos, float len) {
        dl->AddLine(ImVec2(x_start - 0.5f, y_pos), ImVec2(x_start + len + 0.5f, y_pos), IM_COL32(0, 0, 0, 180), cornerThick + 1.6f);
        dl->AddLine(ImVec2(x_start, y_pos), ImVec2(x_start + len, y_pos), color, cornerThick);
    };
    auto AddLineV = [&](float x_pos, float y_start, float len) {
        dl->AddLine(ImVec2(x_pos, y_start - 0.5f), ImVec2(x_pos, y_start + len + 0.5f), IM_COL32(0, 0, 0, 180), cornerThick + 1.6f);
        dl->AddLine(ImVec2(x_pos, y_start), ImVec2(x_pos, y_start + len), color, cornerThick);
    };

    AddLineV(x, y, lineH);
    AddLineH(x, y, lineW);
    AddLineH(x + w - lineW, y, lineW);
    AddLineV(x + w, y, lineH);
    AddLineV(x, y + h - lineH, lineH);
    AddLineH(x, y + h, lineW);
    AddLineH(x + w - lineW, y + h, lineW);
    AddLineV(x + w, y + h - lineH, lineH);
}

static inline void DrawRoundedBox(float x, float y, float w, float h, ImColor color, float thickness, float rounding = 4.0f) {
    auto dl = ImGui::GetBackgroundDrawList();
    float boxThick = (std::max)(1.0f, thickness);
    dl->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), IM_COL32(0, 0, 0, 180), rounding, 0, boxThick + 1.6f);
    dl->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), color, rounding, 0, boxThick);
}

// ─── Snapline Styles (Solid, Dashed, Gradient) ───────────────────────────────
static inline void DrawSnaplineStyle(ImVec2 srcPos, ImVec2 targetPos, ImColor color, float thickness, int style) {
    auto dl = ImGui::GetBackgroundDrawList();
    if (style == 1) { // Dashed
        float dx = targetPos.x - srcPos.x, dy = targetPos.y - srcPos.y;
        for (int s = 0; s < 10; s += 2) {
            float t0 = s / 10.0f;
            float t1 = (s + 1) / 10.0f;
            dl->AddLine(ImVec2(srcPos.x + dx * t0, srcPos.y + dy * t0), ImVec2(srcPos.x + dx * t1, srcPos.y + dy * t1), color, thickness);
        }
    } else if (style == 2) { // Gradient (Fade)
        float dx = targetPos.x - srcPos.x, dy = targetPos.y - srcPos.y;
        int segs = 16;
        for (int i = 0; i < segs; ++i) {
            float t0 = (float)i / (float)segs;
            float t1 = (float)(i + 1) / (float)segs;
            float alpha = 0.08f + 0.92f * (t0 + t1) * 0.5f;
            ImColor segCol(color.Value.x, color.Value.y, color.Value.z, color.Value.w * alpha);
            dl->AddLine(ImVec2(srcPos.x + dx * t0, srcPos.y + dy * t0), ImVec2(srcPos.x + dx * t1, srcPos.y + dy * t1), segCol, thickness);
        }
    } else { // Solid
        dl->AddLine(srcPos, targetPos, color, thickness);
    }
}

// ─── HUD Pro Header Style (EXTERNO-VZX Squad Bar + Badge + Triangle) ─────────
static inline void DrawPlayerHeaderHUDPro(const std::string& name, float distance, ImVec2 headPos, ImColor color, bool isAdmin) {
    auto dl = ImGui::GetBackgroundDrawList();
    ImFont* fontBold = FWork::Fonts::InterBold ? FWork::Fonts::InterBold : ImGui::GetFont();

    std::string displayName = name;
    if (g.ESP_Distance) {
        displayName += " [" + std::to_string((int)distance) + "m]";
    }

    float fontSize = 12.0f;
    ImVec2 nameSz = fontBold ? fontBold->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, displayName.c_str()) : ImGui::CalcTextSize(displayName.c_str());

    float badgeW = 18.0f;
    float padX = 6.0f;
    float padY = 3.0f;
    float barW = (std::max)(55.0f, nameSz.x + badgeW + padX * 2.0f);
    float barH = (std::max)(16.0f, nameSz.y + padY * 2.0f);

    float triangleH = 4.0f;
    float triangleW = 6.0f;
    float triangleTipY = headPos.y - 3.0f;
    float triangleBaseY = triangleTipY - triangleH;
    float nameBarY = triangleBaseY - barH;

    float barX = headPos.x - (barW * 0.5f);

    ImVec2 barMin(barX, nameBarY);
    ImVec2 barMax(barX + barW, nameBarY + barH);

    ImU32 bgCol = IM_COL32(11, 14, 20, 230);
    ImU32 borderCol = isAdmin ? (ImU32)color : IM_COL32(25, 45, 70, 255);
    ImU32 badgeCol = isAdmin ? (ImU32)color : IM_COL32(0, 190, 240, 255);

    dl->AddRectFilled(barMin, barMax, bgCol, 3.0f);
    dl->AddRect(barMin, barMax, borderCol, 3.0f, 0, 1.0f);

    ImVec2 badgeMin(barMin.x, barMin.y);
    ImVec2 badgeMax(barMin.x + badgeW, barMax.y);
    dl->AddRectFilled(badgeMin, badgeMax, badgeCol, 3.0f, ImDrawFlags_RoundCornersLeft);

    dl->AddText(fontBold, 11.0f, ImVec2(badgeMin.x + 5.0f, barMin.y + 2.0f), IM_COL32(255, 255, 255, 255), isAdmin ? "A" : "V");

    ImVec2 textPos(badgeMax.x + padX, barMin.y + (barH - nameSz.y) * 0.5f);
    dl->AddText(fontBold, fontSize, textPos, IM_COL32(245, 248, 255, 255), displayName.c_str());

    ImVec2 p1(headPos.x - (triangleW * 0.5f), triangleBaseY);
    ImVec2 p2(headPos.x + (triangleW * 0.5f), triangleBaseY);
    ImVec2 p3(headPos.x, triangleTipY);
    dl->AddTriangleFilled(p1, p2, p3, badgeCol);
}

// ─── Simple bone drawer (9-bone matrix fallback) ──────────────────────────────
static inline void DrawBoneSimple(const Matrix& vm,
                                  const std::vector<Vector3>& bl,
                                  int a, int b,
                                  ImColor col, float thickness)
{
    if (a >= (int)bl.size() || b >= (int)bl.size()) return;
    if (bl[a] == Vector3(0, 0, 0) || bl[b] == Vector3(0, 0, 0)) return;

    Vector2 sa{}, sb{};
    if (!WorldToScreen(vm, bl[a], sa)) return;
    if (!WorldToScreen(vm, bl[b], sb)) return;

    ImGui::GetBackgroundDrawList()->AddLine(
        ImVec2(sa.x, sa.y), ImVec2(sb.x, sb.y), col, thickness);
}

// ─── Simple skeleton (Rocket case 0) ──────────────────────────────────────────
static void DrawSkeletonSimple(const Matrix& vm,
                               const std::vector<Vector3>& bl,
                               ImColor col, float thickness)
{
    DrawBoneSimple(vm, bl, 0, 7, col, thickness); // HEAD → NECK
    DrawBoneSimple(vm, bl, 7, 6, col, thickness); // NECK → RIGHTHAND
    DrawBoneSimple(vm, bl, 7, 5, col, thickness); // NECK → LEFTHAND
    DrawBoneSimple(vm, bl, 7, 8, col, thickness); // NECK → HIP
    DrawBoneSimple(vm, bl, 8, 3, col, thickness); // HIP  → LEFTANKLE
    DrawBoneSimple(vm, bl, 8, 4, col, thickness); // HIP  → RIGHTANKLE
    DrawBoneSimple(vm, bl, 3, 1, col, thickness); // LEFTANKLE  → LEFTFOOT
    DrawBoneSimple(vm, bl, 4, 2, col, thickness); // RIGHTANKLE → RIGHTFOOT
}

// ─── Complex Full-Body Skeleton (Rocket case 1: crSkeletonData) ───────────────
static void DrawFullRocketSkeleton(const Matrix& vm, CPed* ped,
                                   const std::vector<Vector3>& fallbackBones,
                                   ImColor col, float thickness)
{
    Vector3 Pelvis          = ped->GetBonePos(BONETAG_PELVIS);
    Vector3 Neck            = ped->GetBonePos(BONETAG_NECK);
    Vector3 Head            = ped->GetBonePos(BONETAG_HEAD);

    // If crSkeletonData is not available, fall back to simple bone matrix
    if (Neck == Vector3(0, 0, 0) || Head == Vector3(0, 0, 0)) {
        DrawSkeletonSimple(vm, fallbackBones, col, thickness);
        return;
    }

    Vector3 Clavicle_Left   = ped->GetBonePos(BONETAG_L_CLAVICLE);
    Vector3 Clavicle_Right  = ped->GetBonePos(BONETAG_R_CLAVICLE);
    Vector3 Uperarm_left    = ped->GetBonePos(BONETAG_L_UPPERARM);
    Vector3 Uperarm_right   = ped->GetBonePos(BONETAG_R_UPPERARM);
    Vector3 Elbow_l         = ped->GetBonePos(MH_L_Elbow);
    Vector3 Elbow_r         = ped->GetBonePos(MH_R_Elbow);
    Vector3 L_FormArm       = ped->GetBonePos(BONETAG_L_FOREARM);
    Vector3 R_FormArm       = ped->GetBonePos(BONETAG_R_FOREARM);
    Vector3 SKEL_L_Hand     = ped->GetBonePos(BONETAG_L_HAND);
    Vector3 SKEL_R_Hand     = ped->GetBonePos(BONETAG_R_HAND);
    Vector3 spine           = ped->GetBonePos(BONETAG_SPINE);
    Vector3 SKEL_L_Thigh    = ped->GetBonePos(BONETAG_L_THIGH);
    Vector3 SKEL_R_Thigh    = ped->GetBonePos(BONETAG_R_THIGH);
    Vector3 SKEL_L_Calf     = ped->GetBonePos(BONETAG_L_CALF);
    Vector3 SKEL_R_Calf     = ped->GetBonePos(BONETAG_R_CALF);
    Vector3 SKEL_L_Foot     = ped->GetBonePos(BONETAG_L_FOOT);
    Vector3 SKEL_R_Foot     = ped->GetBonePos(BONETAG_R_FOOT);
    Vector3 SKEL_L_TOE      = ped->GetBonePos(BONETAG_L_TOE);
    Vector3 SKEL_R_TOE      = ped->GetBonePos(BONETAG_R_TOE);

    auto DrawSegment = [&](const Vector3& a, const Vector3& b) {
        if (a == Vector3(0, 0, 0) || b == Vector3(0, 0, 0)) return;
        Vector2 sa{}, sb{};
        if (WorldToScreen(vm, a, sa) && WorldToScreen(vm, b, sb)) {
            ImGui::GetBackgroundDrawList()->AddLine(
                ImVec2(sa.x, sa.y), ImVec2(sb.x, sb.y), col, thickness);
        }
    };

    // Head and neck
    DrawSegment(Head, Neck);

    // Left arm: Neck -> Clavicle -> Upperarm -> Elbow -> Forearm -> Hand
    DrawSegment(Neck, Clavicle_Left);
    DrawSegment(Clavicle_Left, Uperarm_left);
    DrawSegment(Uperarm_left, Elbow_l);
    DrawSegment(Elbow_l, L_FormArm);
    DrawSegment(L_FormArm, SKEL_L_Hand);

    // Right arm: Neck -> Clavicle -> Upperarm -> Elbow -> Forearm -> Hand
    DrawSegment(Neck, Clavicle_Right);
    DrawSegment(Clavicle_Right, Uperarm_right);
    DrawSegment(Uperarm_right, Elbow_r);
    DrawSegment(Elbow_r, R_FormArm);
    DrawSegment(R_FormArm, SKEL_R_Hand);

    // Spine & pelvis
    DrawSegment(Neck, spine);
    if (Pelvis != Vector3(0, 0, 0)) {
        DrawSegment(spine, Pelvis);
        DrawSegment(Pelvis, SKEL_L_Thigh);
        DrawSegment(Pelvis, SKEL_R_Thigh);
    } else {
        DrawSegment(spine, SKEL_L_Thigh);
        DrawSegment(spine, SKEL_R_Thigh);
    }

    // Left leg: Thigh -> Calf -> Foot -> Toe
    DrawSegment(SKEL_L_Thigh, SKEL_L_Calf);
    DrawSegment(SKEL_L_Calf, SKEL_L_Foot);
    DrawSegment(SKEL_L_Foot, SKEL_L_TOE);

    // Right leg: Thigh -> Calf -> Foot -> Toe
    DrawSegment(SKEL_R_Thigh, SKEL_R_Calf);
    DrawSegment(SKEL_R_Calf, SKEL_R_Foot);
    DrawSegment(SKEL_R_Foot, SKEL_R_TOE);
}

// ─────────────────────────────────────────────────────────────────────────────

void Cheat::RenderInfo()
{
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImFont* fontBold = FWork::Fonts::InterBold ? FWork::Fonts::InterBold : ImGui::GetFont();

    // Watermark moderno estilo EXTERNO-VZX
    char fpsBuf[64];
    snprintf(fpsBuf, sizeof(fpsBuf), "%d FPS", (int)ImGui::GetIO().Framerate);

    ImVec2 bMin(12.0f, 12.0f);
    ImVec2 bMax(185.0f, 38.0f);
    dl->AddRectFilled(bMin, bMax, IM_COL32(11, 14, 20, 220), 6.0f);
    dl->AddRect(bMin, bMax, IM_COL32(25, 45, 70, 200), 6.0f, 0, 1.0f);

    // Mini badge cyan com 'V'
    dl->AddRectFilled(ImVec2(bMin.x + 6.0f, bMin.y + 4.5f), ImVec2(bMin.x + 22.0f, bMin.y + 20.5f), IM_COL32(0, 190, 240, 255), 3.0f);
    dl->AddText(fontBold, 12.0f, ImVec2(bMin.x + 10.5f, bMin.y + 4.5f), IM_COL32(255, 255, 255, 255), "V");

    dl->AddText(fontBold, 12.0f, ImVec2(bMin.x + 28.0f, bMin.y + 5.0f), IM_COL32(245, 245, 250, 255), "VZX");
    dl->AddText(fontBold, 10.0f, ImVec2(bMin.x + 57.0f, bMin.y + 6.5f), IM_COL32(0, 210, 255, 255), "MENU");

    // Divisória sutil
    dl->AddLine(ImVec2(bMin.x + 98.0f, bMin.y + 7.0f), ImVec2(bMin.x + 98.0f, bMin.y + 19.0f), IM_COL32(35, 55, 80, 255), 1.0f);

    // Bolinha verde de FPS
    dl->AddCircleFilled(ImVec2(bMin.x + 110.0f, bMin.y + 13.0f), 3.0f, IM_COL32(0, 230, 120, 255));
    dl->AddText(fontBold, 11.5f, ImVec2(bMin.x + 118.0f, bMin.y + 5.5f), IM_COL32(220, 230, 240, 255), fpsBuf);

    if (g.Crosshair)
    {
        float cx = (float)g.GameRect.right  / 2.f;
        float cy = (float)g.GameRect.bottom / 2.f;
        dl->AddCircleFilled(ImVec2(cx, cy), 3, ImColor(0.f, 0.f, 0.f, 1.f));
        dl->AddCircleFilled(ImVec2(cx, cy), 2, ImColor(0.f, 0.824f, 1.f, 1.f));
    }


    if (g.SilentShowFov && g.SilentEnabled)
    {
        float cx = (float)g.GameRect.right / 2.f;
        float cy = (float)g.GameRect.bottom / 2.f;
        float effectiveFov = g.SilentIsLegitFov ? g.SilentLegitFov * 10.0f : (float)g.SilentFov;
        dl->AddCircle(ImVec2(cx, cy), effectiveFov, Silent_Fov_Color, 100, 1.0f);
    }

    if (g.AimbotShowFov && g.AimbotEnabled)
    {
        float cx = (float)g.GameRect.right / 2.f;
        float cy = (float)g.GameRect.bottom / 2.f;
        dl->AddCircle(ImVec2(cx, cy), (float)g.AimbotFov, Aimbot_Fov_Color, 100, 1.0f);
    }

    if (g.TriggerShowFov && g.TriggerEnabled)
    {
        float cx = (float)g.GameRect.right / 2.f;
        float cy = (float)g.GameRect.bottom / 2.f;
        dl->AddCircle(ImVec2(cx, cy), (float)g.TriggerFov, ImColor(0.94f, 0.15f, 0.27f, 1.0f), 100, 1.0f);
    }
}

void Cheat::RenderESP()
{
    if (!pLocal->Update())
        return;

    CPed target;
    float MinFov = FLT_MAX;
    float scrW = (float)g.GameRect.right;
    float scrH = (float)g.GameRect.bottom;
    Vector2 Center    = Vector2(scrW / 2.f, scrH / 2.f);
    Matrix ViewMatrix = m.Read<Matrix>(Game->GetViewPort() + 0x24C);

    std::vector<CPed> snapshot;
    {
        std::lock_guard<std::mutex> lock(EntityListMutex);
        snapshot = EntityList;
    }

    for (auto& ped : snapshot)
    {
        CPed* pEntity = &ped;

        if (!pEntity->Update())
            continue;

        float pDistance = GetDistance(pEntity->m_vecPosition, pLocal->m_vecPosition);
        if (pDistance >= g.ESP_MaxDistance)
            continue;

        std::vector<Vector3> BoneList = pEntity->GetBoneList();
        if (BoneList.size() < 9)
            continue;

        Vector2 pBase{}, pHead{}, pNeck{}, pLeftFoot{}, pRightFoot{};
        if (!WorldToScreen(ViewMatrix, pEntity->m_vecPosition, pBase) ||
            !WorldToScreen(ViewMatrix, BoneList[HEAD],      pHead)    ||
            !WorldToScreen(ViewMatrix, BoneList[NECK],      pNeck)    ||
            !WorldToScreen(ViewMatrix, BoneList[LEFTFOOT],  pLeftFoot)||
            !WorldToScreen(ViewMatrix, BoneList[RIGHTFOOT], pRightFoot))
            continue;

        float HeadToNeck = pNeck.y - pHead.y;
        float pTop    = pHead.y - (HeadToNeck * 2.5f);
        float pBottom = (pLeftFoot.y > pRightFoot.y ? pLeftFoot.y : pRightFoot.y) * 1.001f;
        float pHeight = pBottom - pTop;
        float pWidth  = pHeight / 3.5f;
        ImColor color = pEntity->IsPlayer() ? ESP_Player : ESP_NPC;

        // ── Admin ESP: invisible entities get a cycling RGB color ─────────
        bool isInvisible = g.ESP_Admin && pEntity->IsInvisible();
        if (isInvisible)
            color = RainbowColor();

        // ── Snapline (origin & style configurable) ─────────────────────────
        if (g.ESP_Line)
        {
            float originY;
            switch (g.ESP_LineOrigin)
            {
            case 0:  originY = 0.f;        break; // Top of screen
            case 1:  originY = scrH / 2.f; break; // Center
            default: originY = scrH;        break; // Bottom
            }
            DrawSnaplineStyle(ImVec2(scrW / 2.f, originY), ImVec2(pBase.x, pBottom), color, g.ESP_LineThickness, g.ESP_LineStyle);
        }

        // ── Box Styles (Full 2D, Cornered, Rounded, Filled) ────────────────
        if (g.ESP_Box)
        {
            float bx = pBase.x - pWidth;
            float by = pTop;
            float bw = pWidth * 2.f;
            float bh = pHeight;

            if (g.ESP_BoxFilled)
            {
                ImGui::GetBackgroundDrawList()->AddRectFilled(
                    ImVec2(bx, by), ImVec2(bx + bw, by + bh),
                    ImColor(color.Value.x, color.Value.y, color.Value.z, 0.18f),
                    g.ESP_BoxStyle == 2 ? 4.0f : 0.0f);
            }

            switch (g.ESP_BoxStyle)
            {
            case 1:
                DrawCorneredBox(bx, by, bw, bh, color, g.ESP_BoxThickness);
                break;
            case 2:
                DrawRoundedBox(bx, by, bw, bh, color, g.ESP_BoxThickness, 4.0f);
                break;
            default:
                DrawFullBox(bx, by, bw, bh, color, g.ESP_BoxThickness);
                break;
            }
        }

        // ── Skeleton (Rocket V14 style) ───────────────────────────────────
        if (g.ESP_Skeleton)
        {
            float thick = g.ESP_SkeletonThickness;
            ImColor skelCol = isInvisible ? RainbowColor() : ESP_Skeleton;
            if (g.ESP_SkeletonType == 0)
                DrawSkeletonSimple(ViewMatrix, BoneList, skelCol, thick);
            else
                DrawFullRocketSkeleton(ViewMatrix, pEntity, BoneList, skelCol, thick);
        }

        // ── Name & Header ESP (Classic Stacked vs HUD Pro EXTERNO-VZX) ──────
        if (g.ESP_Name)
        {
            std::string name = pEntity->GetName();
            if (!name.empty()) {
                if (isInvisible)
                    name = "[ADMIN] " + name;
                ImColor nameCol = isInvisible ? RainbowColor() : ImColor(1.f, 1.f, 1.f, 1.f);

                if (g.ESP_HeaderStyle == 1) {
                    // Estilo HUD Pro (Squad Badge + Triângulo indicador apontando pra cabeça)
                    DrawPlayerHeaderHUDPro(name, pDistance, ImVec2(pBase.x, pTop), color, isInvisible);
                } else {
                    // Estilo Classic Stacked
                    ImFont* nameFont = FWork::Fonts::InterBold ? FWork::Fonts::InterBold : ImGui::GetFont();
                    DrawOutlinedText(name, ImVec2(pBase.x, pTop - 16.f),
                                     nameCol, ImColor(0.f, 0.f, 0.f, 1.f), true, nameFont, 13.5f);
                }
            }
        }

        if (g.ESP_HealthBar)
        {
            HealthBar((pBase.x - pWidth) - 4.f, pBottom, 2.f, -pHeight,
                      (int)pEntity->m_flHealth, (int)pEntity->m_flMaxHealth);

            if (pEntity->m_flArmor > 0.f)
                ArmorBar((pBase.x + pWidth) + 3.f, pBottom, 2.f, -pHeight,
                         (int)pEntity->m_flArmor, 100);
        }

        // No HUD Pro a distância já fica embutida na barra de nome
        if (g.ESP_Distance && g.ESP_HeaderStyle == 0)
        {
            std::string dist = std::to_string((int)pDistance) + "m";
            ImFont* distFont = FWork::Fonts::InterSemiBold ? FWork::Fonts::InterSemiBold : ImGui::GetFont();
            DrawOutlinedText(dist, ImVec2(pBase.x, pBottom + 2.f),
                             ImColor(1.f, 1.f, 1.f, 1.f), ImColor(0.f, 0.f, 0.f, 1.f), true, distFont, 12.0f);
        }
    }
}

void Cheat::RenderVehicles()
{
    // Take a snapshot of the background-thread vehicle list
    std::vector<VehicleEntry> snapshot;
    {
        std::lock_guard<std::mutex> lock(VehicleListMutex);
        snapshot = VehicleList;
    }

    if (snapshot.empty()) return;

    Matrix ViewMatrix = m.Read<Matrix>(Game->GetViewPort() + 0x24C);
    Vector3 localPos  = pLocal->m_vecPosition;

    for (const auto& veh : snapshot)
    {
        float distance = GetDistance(veh.worldPos, localPos);
        if (distance > g.ESP_VehicleMaxDistance) continue;

        Vector2 screenPos{};
        if (!WorldToScreen(ViewMatrix, veh.worldPos, screenPos)) continue;

        float spaceY = -14.f; // stack upward from world pos

        // 1. Lock Status
        if (g.ESP_VehicleLock)
        {
            std::string lockText;
            ImColor lockColor;

            switch (veh.lockState)
            {
            case 0:
            case 1:
                lockText  = "[Unlocked]";
                lockColor = ESP_Veh_Unlocked;
                break;
            case 2:
                lockText  = "[Locked]";
                lockColor = ESP_Veh_Locked;
                break;
            case 3:
                lockText  = "[LockedForPlayer]";
                lockColor = ESP_Veh_Locked;
                break;
            case 4:
                lockText  = "[Inside]";
                lockColor = ESP_Veh_Inside;
                break;
            default:
                if (veh.lockState > 1) {
                    lockText  = "[Locked]";
                    lockColor = ESP_Veh_Locked;
                } else {
                    lockText  = "[Unlocked]";
                    lockColor = ESP_Veh_Unlocked;
                }
                break;
            }

            ImFont* lockFont = FWork::Fonts::InterBold ? FWork::Fonts::InterBold : ImGui::GetFont();
            DrawOutlinedText(lockText, ImVec2(screenPos.x, screenPos.y + spaceY),
                             lockColor, ImColor(0.f, 0.f, 0.f, 1.f), true, lockFont, 12.5f);
            spaceY += 15.f;
        }

        // 2. Vehicle Name
        if (g.ESP_VehicleName)
        {
            ImFont* vehNameFont = FWork::Fonts::InterBold ? FWork::Fonts::InterBold : ImGui::GetFont();
            DrawOutlinedText(veh.name, ImVec2(screenPos.x, screenPos.y + spaceY),
                             ESP_Veh_Name, ImColor(0.f, 0.f, 0.f, 1.f), true, vehNameFont, 13.0f);
            spaceY += 15.f;
        }

        // 3. Distance
        if (g.ESP_VehicleDistance)
        {
            std::string distStr = std::to_string((int)distance) + "m";
            ImFont* vehDistFont = FWork::Fonts::InterSemiBold ? FWork::Fonts::InterSemiBold : ImGui::GetFont();
            DrawOutlinedText(distStr, ImVec2(screenPos.x, screenPos.y + spaceY),
                             ESP_Veh_Dist, ImColor(0.f, 0.f, 0.f, 1.f), true, vehDistFont, 11.5f);
        }
    }
}

void Cheat::ToggleClosestVehicleLock(int forceState)
{
    uintptr_t replayInterface = Game->GetReplayInterface();
    if (!replayInterface) return;

    uintptr_t vehicleInterface = m.Read<uintptr_t>(replayInterface + 0x10);
    if (!vehicleInterface) return;

    uintptr_t vehList = m.Read<uintptr_t>(vehicleInterface + 0x180);
    int vehCount = m.Read<int>(vehicleInterface + 0x188);

    if (!vehList || vehCount <= 0 || vehCount > 1024) return;

    Vector3 localPos = pLocal->m_vecPosition;
    uintptr_t closestVehicle = 0;
    float closestDist = 50.f; // Max 50m to lock/unlock

    for (int i = 0; i < vehCount; i++)
    {
        uintptr_t vehicle = m.Read<uintptr_t>(vehList + (i * 0x10));
        if (!vehicle) continue;

        Vector3 vehPos = m.Read<Vector3>(vehicle + 0x90);
        if (Vec3_Empty(vehPos)) continue;

        float dist = GetDistance(vehPos, localPos);
        if (dist < closestDist)
        {
            closestDist = dist;
            closestVehicle = vehicle;
        }
    }

    if (closestVehicle)
    {
        uint32_t currentLock = m.Read<uint32_t>(closestVehicle + offset::m_lockState);
        uint32_t newLock = 1;

        if (forceState == 1) {
            newLock = 1; // Force Unlock
        } else if (forceState == 2) {
            newLock = 2; // Force Lock
        } else {
            // Toggle
            newLock = (currentLock >= 2) ? 1 : 2;
        }

        m.Write<uint32_t>(closestVehicle + offset::m_lockState, newLock);
    }
}