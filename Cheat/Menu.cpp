#include "Cheat.h"
#include "../Framework/Overlay/Overlay.h"
#include "../Framework/ImGui/imgui_internal.h"
#include "../Framework/Fonts/Fonts.hpp"
#include "../Framework/Fonts/FontAwesome.hpp"
#include <algorithm>

// ============================================================
//  VZX MENU — Modern Dark Cyan Glassmorphism UI
//  Estilo baseado em EXTERNO-VZX (Inter Fonts + Glassmorphism)
// ============================================================

namespace UI
{
    inline void SectionHeader(const char* title)
    {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float w = ImGui::GetContentRegionAvail().x;
        ImDrawList* dl = ImGui::GetWindowDrawList();

        ImFont* fontBold = FWork::Fonts::InterBold ? FWork::Fonts::InterBold : ImGui::GetFont();
        float fSize = 12.0f;

        ImGui::Dummy(ImVec2(w, 22.0f));

        // Bolinha ciano neon
        dl->AddCircleFilled(ImVec2(pos.x + 4.0f, pos.y + 11.0f), 3.0f, IM_COL32(0, 210, 255, 255));
        dl->AddText(fontBold, fSize, ImVec2(pos.x + 14.0f, pos.y + 3.0f), IM_COL32(0, 210, 255, 255), title);

        ImVec2 tSz = fontBold ? fontBold->CalcTextSizeA(fSize, FLT_MAX, 0.0f, title) : ImGui::CalcTextSize(title);
        float lx = pos.x + 20.0f + tSz.x;
        if (lx < pos.x + w - 10.0f) {
            dl->AddLine(ImVec2(lx, pos.y + 11.0f), ImVec2(pos.x + w, pos.y + 11.0f), IM_COL32(25, 45, 70, 180), 1.0f);
        }
        ImGui::Spacing();
    }

    inline bool CustomCheckbox(const char* label, bool* v, ImColor* col = nullptr, const char* colId = nullptr, float customWidth = 0.0f)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        ImGuiContext& g_ctx = *GImGui;
        const ImGuiID id = window->GetID(label);

        float availW = customWidth > 0.0f ? customWidth : ImGui::GetContentRegionAvail().x;
        float height = 26.0f;
        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size(availW, height);
        const ImRect total_bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

        ImGui::ItemSize(total_bb, 0.0f);
        if (!ImGui::ItemAdd(total_bb, id)) return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
        if (pressed && v) {
            *v = !(*v);
            ImGui::MarkItemEdited(id);
        }

        ImDrawList* dl = window->DrawList;

        // Caixa do Checkbox (18x18)
        float squareSz = 18.0f;
        ImVec2 checkMin(pos.x + 2.0f, pos.y + (height - squareSz) * 0.5f);
        ImVec2 checkMax(checkMin.x + squareSz, checkMin.y + squareSz);

        bool isChecked = v ? *v : false;
        if (isChecked) {
            dl->AddRectFilled(checkMin, checkMax, IM_COL32(0, 190, 240, 255), 4.0f);
            dl->AddRect(checkMin, checkMax, IM_COL32(0, 230, 255, 255), 4.0f, 0, 1.2f);

            // Marca de Check em traço branco anti-aliased
            dl->AddLine(ImVec2(checkMin.x + 4.5f, checkMin.y + 9.0f), ImVec2(checkMin.x + 8.0f, checkMin.y + 12.5f), IM_COL32(255, 255, 255, 255), 2.2f);
            dl->AddLine(ImVec2(checkMin.x + 8.0f, checkMin.y + 12.5f), ImVec2(checkMin.x + 13.5f, checkMin.y + 5.0f), IM_COL32(255, 255, 255, 255), 2.2f);
        } else {
            dl->AddRectFilled(checkMin, checkMax, IM_COL32(14, 20, 30, 230), 4.0f);
            dl->AddRect(checkMin, checkMax, hovered ? IM_COL32(0, 210, 255, 180) : IM_COL32(25, 42, 65, 255), 4.0f, 0, 1.0f);
        }

        // Texto do Rótulo
        ImFont* font = FWork::Fonts::InterSemiBold ? FWork::Fonts::InterSemiBold : ImGui::GetFont();
        float fontSize = 13.0f;
        ImVec2 textPos(checkMax.x + 10.0f, pos.y + (height - fontSize) * 0.5f - 1.0f);
        ImU32 textCol = isChecked ? IM_COL32(245, 248, 255, 255) : (hovered ? IM_COL32(210, 220, 235, 255) : IM_COL32(160, 175, 195, 255));
        dl->AddText(font, fontSize, textPos, textCol, label);

        // Color Picker Integrado (Swatch no canto direito)
        if (col && colId) {
            float swatchW = 20.0f, swatchH = 15.0f;
            ImVec2 swatchMin(pos.x + availW - swatchW - 4.0f, pos.y + (height - swatchH) * 0.5f);
            ImVec2 swatchMax(swatchMin.x + swatchW, swatchMin.y + swatchH);

            ImU32 swatchColor = ImColor(col->Value.x, col->Value.y, col->Value.z, col->Value.w);
            dl->AddRectFilled(swatchMin, swatchMax, swatchColor, 3.0f);
            dl->AddRect(swatchMin, swatchMax, IM_COL32(255, 255, 255, 180), 3.0f, 0, 1.0f);

            if (ImGui::IsMouseClicked(0) && ImGui::IsMouseHoveringRect(swatchMin, swatchMax)) {
                ImGui::OpenPopup(colId);
            }
            if (ImGui::BeginPopup(colId)) {
                ImGui::ColorPicker4("##cp", &col->Value.x, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
                ImGui::EndPopup();
            }
        }

        return pressed;
    }

    inline bool CustomSlider(const char* label, float* v, float min_v, float max_v, const char* fmt = "%.1f", float customWidth = 0.0f)
    {
        float availW = customWidth > 0.0f ? customWidth : ImGui::GetContentRegionAvail().x;
        ImFont* fontSemi = FWork::Fonts::InterSemiBold ? FWork::Fonts::InterSemiBold : ImGui::GetFont();
        if (fontSemi) ImGui::PushFont(fontSemi);
        ImGui::TextColored(ImVec4(0.67f, 0.73f, 0.80f, 1.0f), "%s", label);
        if (fontSemi) ImGui::PopFont();

        ImGui::SetNextItemWidth(availW);
        char idBuf[64];
        snprintf(idBuf, sizeof(idBuf), "##sl_%s", label);
        return ImGui::SliderFloat(idBuf, v, min_v, max_v, fmt);
    }

    inline bool CustomCombo(const char* label, int* current_item, const char* const items[], int items_count, float customWidth = 0.0f)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        ImGuiContext& g_ctx = *GImGui;
        const ImGuiID id = window->GetID(label);

        float availW = customWidth > 0.0f ? customWidth : ImGui::GetContentRegionAvail().x;
        ImVec2 pos = window->DC.CursorPos;
        float totalH = 48.0f;
        const ImRect total_bb(pos, ImVec2(pos.x + availW, pos.y + totalH));

        ImGui::ItemSize(total_bb, 0.0f);
        if (!ImGui::ItemAdd(total_bb, id)) return false;

        ImDrawList* dl = window->DrawList;
        ImFont* fontBold = FWork::Fonts::InterBold ? FWork::Fonts::InterBold : ImGui::GetFont();
        ImFont* fontSemi = FWork::Fonts::InterSemiBold ? FWork::Fonts::InterSemiBold : ImGui::GetFont();

        // 1. Label Acima
        dl->AddText(fontSemi, 12.0f, pos, IM_COL32(170, 185, 205, 255), label);

        // 2. Caixa do Combo
        float boxY = pos.y + 19.0f;
        float boxH = 26.0f;
        ImVec2 boxMin(pos.x, boxY);
        ImVec2 boxMax(pos.x + availW, boxY + boxH);

        bool hovered = ImGui::IsMouseHoveringRect(boxMin, boxMax);
        if (hovered) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        dl->AddRectFilled(boxMin, boxMax, IM_COL32(14, 20, 30, 240), 4.0f);
        dl->AddRect(boxMin, boxMax, hovered ? IM_COL32(0, 210, 255, 180) : IM_COL32(25, 42, 65, 255), 4.0f, 0, 1.0f);

        // Texto do item selecionado
        const char* currentText = (*current_item >= 0 && *current_item < items_count) ? items[*current_item] : "Selecionar...";
        dl->AddText(fontBold, 12.5f, ImVec2(boxMin.x + 8.0f, boxMin.y + 5.0f), IM_COL32(245, 248, 255, 255), currentText);

        // Seta ▼
        float arrowX = boxMax.x - 14.0f;
        float arrowY = boxMin.y + boxH * 0.5f;
        dl->AddTriangleFilled(
            ImVec2(arrowX - 4.0f, arrowY - 2.0f),
            ImVec2(arrowX + 4.0f, arrowY - 2.0f),
            ImVec2(arrowX, arrowY + 3.0f),
            IM_COL32(0, 210, 255, 255)
        );

        char popupId[64];
        snprintf(popupId, sizeof(popupId), "##popup_%s", label);

        if (hovered && ImGui::IsMouseClicked(0)) {
            ImGui::OpenPopup(popupId);
        }

        bool value_changed = false;
        ImGui::SetNextWindowPos(ImVec2(boxMin.x, boxMax.y + 2.0f));
        ImGui::SetNextWindowSize(ImVec2(availW, 0.0f));

        if (ImGui::BeginPopup(popupId, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {
            for (int i = 0; i < items_count; i++) {
                bool is_selected = (*current_item == i);
                if (ImGui::Selectable(items[i], is_selected)) {
                    *current_item = i;
                    value_changed = true;
                    ImGui::MarkItemEdited(id);
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndPopup();
        }

        return value_changed;
    }
}

static void HotkeyButton(const char* label, int& key, float customWidth = 0.0f)
{
    static int waitingKeyId = -1;
    ImGuiID id = ImGui::GetID(label);

    const char* keyName = "NONE";
    if (key > 0 && key < 136 && KeyNames[key] && KeyNames[key][0] != '\0')
        keyName = KeyNames[key];
    else if (key > 0) {
        static char customBuf[32];
        sprintf_s(customBuf, "Key 0x%X", key);
        keyName = customBuf;
    }

    std::string btnText = (waitingKeyId == (int)id) ? "[ Pressione uma tecla... ]" : (std::string("[ ") + keyName + " ]");

    ImFont* fontSemi = FWork::Fonts::InterSemiBold ? FWork::Fonts::InterSemiBold : ImGui::GetFont();
    if (fontSemi) ImGui::PushFont(fontSemi);
    ImGui::TextColored(ImVec4(0.67f, 0.73f, 0.80f, 1.0f), "%s:", label);
    if (fontSemi) ImGui::PopFont();

    float availW = customWidth > 0.0f ? customWidth : ImGui::GetContentRegionAvail().x;
    float btnW = availW - 65.0f;

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.000f, 0.588f, 0.784f, 0.35f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.000f, 0.824f, 1.000f, 0.55f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.000f, 0.824f, 1.000f, 0.80f));

    if (ImGui::Button((btnText + "##" + label).c_str(), ImVec2(btnW, 26.0f)))
    {
        waitingKeyId = (int)id;
    }

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.1f, 0.1f, 0.4f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.15f, 0.15f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.2f, 0.2f, 0.9f));
    if (ImGui::Button(("Limpar##" + std::string(label)).c_str(), ImVec2(55.0f, 26.0f)))
    {
        key = 0;
        if (waitingKeyId == (int)id)
            waitingKeyId = -1;
    }
    ImGui::PopStyleColor(6);

    if (waitingKeyId == (int)id)
    {
        for (int k = 1; k < 256; k++)
        {
            if (k == VK_LBUTTON) continue; // Não vincular clique esquerdo
            if (GetAsyncKeyState(k) & 0x8000)
            {
                if (k == VK_ESCAPE)
                    key = 0; // Escape limpa
                else
                    key = k;

                waitingKeyId = -1;
                break;
            }
        }
    }
}

void Cheat::RenderMenu()
{
    // ========================================================
    // TEMA DARK CYAN GLASSMORPHISM (EXTERNO-VZX)
    // ========================================================
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding    = 8.f;
    style.FrameRounding     = 4.f;
    style.PopupRounding     = 4.f;
    style.GrabRounding      = 4.f;
    style.TabRounding       = 4.f;
    style.FrameBorderSize   = 1.f;
    style.WindowBorderSize  = 1.f;

    style.Colors[ImGuiCol_WindowBg]         = ImVec4(0.043f, 0.055f, 0.078f, 0.97f);
    style.Colors[ImGuiCol_Border]           = ImVec4(0.086f, 0.145f, 0.220f, 1.00f);
    style.Colors[ImGuiCol_FrameBg]          = ImVec4(0.063f, 0.086f, 0.122f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered]   = ImVec4(0.094f, 0.133f, 0.188f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive]    = ImVec4(0.125f, 0.180f, 0.255f, 1.00f);
    style.Colors[ImGuiCol_TitleBg]          = ImVec4(0.043f, 0.055f, 0.078f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive]    = ImVec4(0.043f, 0.055f, 0.078f, 1.00f);
    style.Colors[ImGuiCol_CheckMark]        = ImVec4(0.000f, 0.824f, 1.000f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab]       = ImVec4(0.000f, 0.824f, 1.000f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.200f, 0.900f, 1.000f, 1.00f);
    style.Colors[ImGuiCol_Button]           = ImVec4(0.000f, 0.588f, 0.784f, 0.35f);
    style.Colors[ImGuiCol_ButtonHovered]    = ImVec4(0.000f, 0.824f, 1.000f, 0.55f);
    style.Colors[ImGuiCol_ButtonActive]     = ImVec4(0.000f, 0.824f, 1.000f, 0.80f);
    style.Colors[ImGuiCol_Header]           = ImVec4(0.000f, 0.588f, 0.784f, 0.25f);
    style.Colors[ImGuiCol_HeaderHovered]    = ImVec4(0.000f, 0.824f, 1.000f, 0.40f);
    style.Colors[ImGuiCol_HeaderActive]     = ImVec4(0.000f, 0.824f, 1.000f, 0.65f);
    style.Colors[ImGuiCol_Tab]              = ImVec4(0.063f, 0.086f, 0.122f, 0.80f);
    style.Colors[ImGuiCol_TabHovered]       = ImVec4(0.000f, 0.824f, 1.000f, 0.40f);
    style.Colors[ImGuiCol_TabActive]        = ImVec4(0.000f, 0.588f, 0.784f, 0.50f);
    style.Colors[ImGuiCol_Text]             = ImVec4(0.920f, 0.940f, 0.970f, 1.00f);
    style.ScrollbarSize                     = 6.0f;
    style.ScrollbarRounding                 = 4.0f;
    style.Colors[ImGuiCol_ScrollbarBg]      = ImVec4(0.04f, 0.06f, 0.08f, 0.50f);
    style.Colors[ImGuiCol_ScrollbarGrab]    = ImVec4(0.00f, 0.58f, 0.78f, 0.40f);

    ImGui::SetNextWindowSize(ImVec2(690.f, 520.f), ImGuiCond_FirstUseEver);
    ImGuiWindowFlags winFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
    ImGui::Begin("VzxMenuWindow", &g.ShowMenu, winFlags);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 wPos = ImGui::GetWindowPos();
    ImVec2 wSize = ImGui::GetWindowSize();

    // ========================================================
    // CUSTOM TITLEBAR HEADER (VZX MENU + ONLINE + EXIT)
    // ========================================================
    float headerH = 44.0f;
    dl->AddRectFilled(wPos, ImVec2(wPos.x + wSize.x, wPos.y + headerH), IM_COL32(11, 14, 20, 255), 8.0f, ImDrawFlags_RoundCornersTop);
    dl->AddLine(ImVec2(wPos.x, wPos.y + headerH), ImVec2(wPos.x + wSize.x, wPos.y + headerH), IM_COL32(22, 36, 56, 255), 1.0f);

    // Logo Badge Ciano com 'V'
    ImVec2 logoMin(wPos.x + 12.0f, wPos.y + 10.0f);
    ImVec2 logoMax(wPos.x + 36.0f, wPos.y + 34.0f);
    dl->AddRectFilled(logoMin, logoMax, IM_COL32(0, 190, 240, 255), 4.0f);

    ImFont* fontBold = FWork::Fonts::InterBold ? FWork::Fonts::InterBold : ImGui::GetFont();
    ImFont* fontSemi = FWork::Fonts::InterSemiBold ? FWork::Fonts::InterSemiBold : ImGui::GetFont();

    dl->AddText(fontBold, 15.0f, ImVec2(logoMin.x + 8.0f, logoMin.y + 3.5f), IM_COL32(255, 255, 255, 255), "V");
    dl->AddText(fontBold, 14.5f, ImVec2(logoMax.x + 8.0f, wPos.y + 13.0f), IM_COL32(245, 245, 250, 255), "VZX");
    dl->AddText(fontBold, 11.0f, ImVec2(logoMax.x + 44.0f, wPos.y + 15.5f), IM_COL32(0, 210, 255, 255), "MENU");

    // Status Pill ● ONLINE
    ImVec2 stMin(wPos.x + wSize.x - 145.0f, wPos.y + 12.0f);
    ImVec2 stMax(wPos.x + wSize.x - 76.0f, wPos.y + 32.0f);
    dl->AddRectFilled(stMin, stMax, IM_COL32(14, 20, 28, 220), 10.0f);
    dl->AddRect(stMin, stMax, IM_COL32(25, 40, 60, 255), 10.0f, 0, 1.0f);
    dl->AddCircleFilled(ImVec2(stMin.x + 10.0f, stMin.y + 10.0f), 3.5f, IM_COL32(0, 230, 120, 255));
    dl->AddText(fontBold, 10.5f, ImVec2(stMin.x + 18.0f, stMin.y + 3.0f), IM_COL32(220, 230, 240, 255), "ONLINE");

    // Botão EXIT Vermelho no Topo
    ImGui::SetCursorPos(ImVec2(wSize.x - 68.0f, 10.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.15f, 0.15f, 0.35f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.20f, 0.20f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.00f, 0.10f, 0.10f, 1.00f));
    if (ImGui::Button("EXIT", ImVec2(56.0f, 24.0f))) {
        g.exit_requested = true;
        g.process_active = false;
    }
    ImGui::PopStyleColor(3);

    ImGui::SetCursorPosY(50.0f);

    // ========================================================
    // TAB BAR HORIZONTAL ESTILO MODERNO
    // ========================================================
    if (ImGui::BeginTabBar("VzxMainTabs", ImGuiTabBarFlags_None))
    {
        // ----------------------------------------------------
        // TAB 1: AIMBOT & COMBATE (Portado do EXECUTOR)
        // ----------------------------------------------------
        if (ImGui::BeginTabItem("AimBot"))
        {
            ImGui::Spacing();
            float fullW = ImGui::GetContentRegionAvail().x;
            float halfW = (fullW - 16.0f) * 0.5f;

            // ── COLUNA DA ESQUERDA: SILENT AIM & MAGIC BULLETS ──
            ImGui::BeginChild("AimLeft", ImVec2(halfW, 0), false);
            {
                UI::SectionHeader("SILENT AIM & MAGIC BULLETS");
                UI::CustomCheckbox("Ativar Silent Aim", &g.SilentEnabled);
                UI::CustomCheckbox("Magic Bullets", &g.MagicBulletsEnabled);
                UI::CustomCheckbox("Exibir FOV Silent", &g.SilentShowFov, &Silent_Fov_Color, "##silentfovcol");
                UI::CustomCheckbox("FOV Legitimo", &g.SilentIsLegitFov);
                UI::CustomCheckbox("Ignorar NPCs (Silent)", &g.SilentIgnorePed);
                UI::CustomCheckbox("Ignorar Mortos", &g.SilentIgnoreDead);
                UI::CustomCheckbox("Checar Visibilidade", &g.SilentVisibleCheck);
                UI::CustomCheckbox("LegitHit (5o Tiro na Cabeca)", &g.SilentLegitHit);

                ImGui::Spacing();
                const char* silentBones[] = { "Cabeca", "Peito", "Pe Direito", "Pe Esquerdo", "Mao Direita", "Mao Esquerda", "Pescoco" };
                UI::CustomCombo("Osso Alvo (Silent)", &g.SilentBone, silentBones, 7, halfW);

                float distS = (float)g.SilentDistance;
                if (UI::CustomSlider("Distancia Silent", &distS, 0.0f, 500.0f, "%.0f m", halfW))
                    g.SilentDistance = (int)distS;

                if (g.SilentIsLegitFov)
                {
                    float normalizedLegit = g.SilentLegitFov * 20.0f;
                    if (UI::CustomSlider("FOV Legitimo", &normalizedLegit, 0.0f, 100.0f, "%.1f%%", halfW))
                        g.SilentLegitFov = normalizedLegit / 20.0f;
                }
                else
                {
                    float normalizedFov = (float)g.SilentFov / 10.0f;
                    if (UI::CustomSlider("FOV Silent", &normalizedFov, 0.0f, 100.0f, "%.1f%%", halfW))
                        g.SilentFov = (int)(normalizedFov * 10.0f);

                    float missChance = (float)g.SilentMissChance;
                    if (UI::CustomSlider("Chance de Erro", &missChance, 0.0f, 100.0f, "%.0f%%", halfW))
                        g.SilentMissChance = (int)missChance;
                }

                ImGui::Spacing();
                UI::SectionHeader("ATALHOS SILENT");
                HotkeyButton("Tecla do Silent", g.SilentKey, halfW);
                HotkeyButton("Alternar Silent", g.SilentEnableKey, halfW);
                ImGui::TextColored(ImVec4(0.45f, 0.75f, 0.95f, 1.0f), "Nota: Atua ao segurar a tecla configurada.\n(Se Limpar/NONE, atua no clique do mouse)");
            }
            ImGui::EndChild();

            ImGui::SameLine(halfW + 16.0f);

            // ── COLUNA DA DIREITA: AIMBOT & TRIGGERBOT ──
            ImGui::BeginChild("AimRight", ImVec2(halfW, 0), false);
            {
                UI::SectionHeader("AIMBOT AVANCADO (TD7)");
                UI::CustomCheckbox("Ativar Aimbot", &g.AimbotEnabled);
                UI::CustomCheckbox("Exibir FOV Aimbot", &g.AimbotShowFov, &Aimbot_Fov_Color, "##aimfovcol");
                UI::CustomCheckbox("Checar Visibilidade (LOS)", &g.AimbotOnlyVisible);
                UI::CustomCheckbox("Ignorar NPCs (Aimbot)", &g.AimbotIgnoreNPCs);
                UI::CustomCheckbox("Predicao de Movimento", &g.AimbotPrediction);
                UI::CustomCheckbox("Priorizar Distancia", &g.AimbotPrioritizeDistance);
                UI::CustomCheckbox("Sempre Ativo", &g.AimbotAlwaysActive);

                ImGui::Spacing();
                const char* aimBones[] = { "Cabeca", "Pescoco", "Torso", "Mao Esquerda", "Mao Direita", "Pe Esquerdo", "Pe Direito" };
                UI::CustomCombo("Osso Alvo (Aimbot)", &g.AimbotBone, aimBones, 7, halfW);

                float aimFovF = (float)g.AimbotFov;
                if (UI::CustomSlider("Tamanho do FOV", &aimFovF, 0.0f, 500.0f, "%.0f px", halfW))
                    g.AimbotFov = (int)aimFovF;

                float aimSpeedF = (float)g.AimbotSpeed;
                if (UI::CustomSlider("Suavizacao (Smooth)", &aimSpeedF, 1.0f, 50.0f, "%.0f", halfW))
                    g.AimbotSpeed = (int)aimSpeedF;

                float aimDistF = (float)g.AimbotMaxDistance;
                if (UI::CustomSlider("Distancia Maxima", &aimDistF, 10.0f, 1000.0f, "%.0f m", halfW))
                    g.AimbotMaxDistance = (int)aimDistF;

                HotkeyButton("Tecla do Aimbot", g.AimbotKey, halfW);

                ImGui::Spacing();
                UI::SectionHeader("TRIGGERBOT");
                UI::CustomCheckbox("Ativar TriggerBot", &g.TriggerEnabled);
                UI::CustomCheckbox("Exibir FOV Trigger", &g.TriggerShowFov);
                UI::CustomCheckbox("Ignorar NPCs (Trigger)", &g.TriggerIgnoreNPCs);

                float trigFovF = (float)g.TriggerFov;
                if (UI::CustomSlider("FOV Trigger", &trigFovF, 1.0f, 100.0f, "%.0f px", halfW))
                    g.TriggerFov = (int)trigFovF;

                float trigDistF = (float)g.TriggerMaxDistance;
                if (UI::CustomSlider("Distancia Trigger", &trigDistF, 10.0f, 500.0f, "%.0f m", halfW))
                    g.TriggerMaxDistance = (int)trigDistF;

                float trigDelayF = (float)g.TriggerDelay;
                if (UI::CustomSlider("Atraso Disparo (Delay)", &trigDelayF, 0.0f, 250.0f, "%.0f ms", halfW))
                    g.TriggerDelay = (int)trigDelayF;

                HotkeyButton("Tecla do TriggerBot", g.TriggerKey, halfW);
            }
            ImGui::EndChild();

            ImGui::EndTabItem();
        }

        // ----------------------------------------------------
        // TAB 2: VISUAL (ESP)
        // ----------------------------------------------------
        if (ImGui::BeginTabItem("Visual"))
        {
            ImGui::Spacing();
            float fullW = ImGui::GetContentRegionAvail().x;
            float halfW = (fullW - 16.0f) * 0.5f;

            ImGui::BeginChild("VisLeft", ImVec2(halfW, 0), false);
            {
                UI::SectionHeader("RECURSOS & ESTILOS ESP");
                UI::CustomCheckbox("ENABLE MASTER ESP", &g.ESP);
                ImGui::Spacing();

                UI::CustomCheckbox("ESP Box 2D", &g.ESP_Box);
                if (g.ESP_Box) {
                    const char* boxStyles[] = { "Full 2D Box", "Cornered Box", "Rounded Box" };
                    UI::CustomCombo("Estilo da Box", &g.ESP_BoxStyle, boxStyles, 3, halfW);
                    UI::CustomCheckbox("Box Preenchida (Filled)", &g.ESP_BoxFilled);
                    UI::CustomSlider("Espessura da Box", &g.ESP_BoxThickness, 1.0f, 4.0f, "%.1f px", halfW);
                    ImGui::Spacing();
                }

                UI::CustomCheckbox("ESP Nomes", &g.ESP_Name);
                if (g.ESP_Name) {
                    const char* headerStyles[] = { "Classic Stacked", "HUD Pro (Badge + Triangulo)" };
                    UI::CustomCombo("Estilo do Cabecalho", &g.ESP_HeaderStyle, headerStyles, 2, halfW);
                    ImGui::Spacing();
                }

                UI::CustomCheckbox("ESP Distancia", &g.ESP_Distance);
                UI::CustomCheckbox("Barra de Vida (HP)", &g.ESP_HealthBar);
                UI::CustomCheckbox("Admin ESP (Invisivel = RGB)", &g.ESP_Admin);

                ImGui::Spacing();
                UI::SectionHeader("CORES DO ESP");
                UI::CustomCheckbox("Cor Jogadores", nullptr, &ESP_Player, "##pcol");
                UI::CustomCheckbox("Cor NPCs / Outros", nullptr, &ESP_NPC, "##npccol");
            }
            ImGui::EndChild();

            ImGui::SameLine(halfW + 16.0f);

            ImGui::BeginChild("VisRight", ImVec2(halfW, 0), false);
            {
                UI::SectionHeader("SKELETON (OSSOS)");
                UI::CustomCheckbox("ESP Skeleton", &g.ESP_Skeleton, &ESP_Skeleton, "##skelcol");
                if (g.ESP_Skeleton) {
                    const char* skelTypes[] = { "Simple (9 Ossos)", "Complex (Full Body)" };
                    UI::CustomCombo("Tipo de Skeleton", &g.ESP_SkeletonType, skelTypes, 2, halfW);
                    UI::CustomSlider("Espessura Skeleton", &g.ESP_SkeletonThickness, 0.5f, 4.0f, "%.1f px", halfW);
                }

                ImGui::Spacing();
                UI::SectionHeader("SNAPLINES (LINHAS)");
                UI::CustomCheckbox("Snaplines", &g.ESP_Line);
                if (g.ESP_Line) {
                    const char* lineOrigins[] = { "Topo da Tela", "Centro da Tela", "Base da Tela" };
                    UI::CustomCombo("Origem da Linha", &g.ESP_LineOrigin, lineOrigins, 3, halfW);

                    const char* lineStyles[] = { "Solida", "Tracejada (Dashed)", "Gradiente (Fade)" };
                    UI::CustomCombo("Estilo da Linha", &g.ESP_LineStyle, lineStyles, 3, halfW);
                    UI::CustomSlider("Espessura Snapline", &g.ESP_LineThickness, 0.5f, 4.0f, "%.1f px", halfW);
                }

                ImGui::Spacing();
                UI::SectionHeader("ALCANCE MAXIMO");
                UI::CustomSlider("Distancia Maxima ESP", &g.ESP_MaxDistance, 50.f, 1000.f, "%.0f m", halfW);
            }
            ImGui::EndChild();

            ImGui::EndTabItem();
        }

        // ----------------------------------------------------
        // TAB 3: VEHICLES
        // ----------------------------------------------------
        if (ImGui::BeginTabItem("Vehicles"))
        {
            ImGui::Spacing();
            float fullW = ImGui::GetContentRegionAvail().x;
            float halfW = (fullW - 16.0f) * 0.5f;

            ImGui::BeginChild("VehLeft", ImVec2(halfW, 0), false);
            {
                UI::SectionHeader("VEHICLE ESP");
                UI::CustomCheckbox("Ativar ESP Veiculos", &g.ESP_Vehicle);
                if (g.ESP_Vehicle) {
                    UI::CustomCheckbox("Nome do Veiculo", &g.ESP_VehicleName, &ESP_Veh_Name, "##vnamecol");
                    UI::CustomCheckbox("Status de Trava", &g.ESP_VehicleLock);
                    UI::CustomCheckbox("Distancia", &g.ESP_VehicleDistance, &ESP_Veh_Dist, "##vdistcol");
                    UI::CustomSlider("Distancia Maxima", &g.ESP_VehicleMaxDistance, 50.f, 1000.f, "%.0f m");
                }

                ImGui::Spacing();
                UI::SectionHeader("CORES DE STATUS");
                UI::CustomCheckbox("Destravado", nullptr, &ESP_Veh_Unlocked, "##vunlck");
                UI::CustomCheckbox("Travado", nullptr, &ESP_Veh_Locked, "##vlck");
                UI::CustomCheckbox("Ocupado (Inside)", nullptr, &ESP_Veh_Inside, "##vins");
            }
            ImGui::EndChild();

            ImGui::SameLine(halfW + 16.0f);

            ImGui::BeginChild("VehRight", ImVec2(halfW, 0), false);
            {
                UI::SectionHeader("CONTROLE DE TRAVA (50M)");
                HotkeyButton("Travar/Destravar", g.VehicleLockKey, halfW);
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.6f, 0.7f, 0.8f, 1.0f), "Alterne o trancamento do veiculo\nmais proximo pressionando a tecla.");

                ImGui::Spacing();
                UI::SectionHeader("CONTROLES MANUAIS");
                if (ImGui::Button("Destravar Mais Proximo", ImVec2(halfW, 30.0f))) {
                    ToggleClosestVehicleLock(1);
                }
                ImGui::Spacing();
                if (ImGui::Button("Travar Mais Proximo", ImVec2(halfW, 30.0f))) {
                    ToggleClosestVehicleLock(2);
                }
            }
            ImGui::EndChild();

            ImGui::EndTabItem();
        }

        // ----------------------------------------------------
        // TAB 4: MISC
        // ----------------------------------------------------
        if (ImGui::BeginTabItem("Misc"))
        {
            ImGui::Spacing();
            float fullW = ImGui::GetContentRegionAvail().x;
            float halfW = (fullW - 16.0f) * 0.5f;

            ImGui::BeginChild("MiscLeft", ImVec2(halfW, 0), false);
            {
                UI::SectionHeader("PLAYER MODS");
                UI::CustomCheckbox("GodMode", &g.GodMode);

                ImGui::Spacing();
                UI::SectionHeader("WEAPON MODS");
                UI::CustomCheckbox("Sem Recuo (NoRecoil)", &g.NoRecoil);
                UI::CustomCheckbox("Sem Espalhamento (NoSpread)", &g.NoSpread);
            }
            ImGui::EndChild();

            ImGui::SameLine(halfW + 16.0f);

            ImGui::BeginChild("MiscRight", ImVec2(halfW, 0), false);
            {
                UI::SectionHeader("SISTEMA & OVERLAY");
                UI::CustomCheckbox("StreamProof (Invisivel na Gravacao)", &g.StreamProof);
                UI::CustomCheckbox("Crosshair Central", &g.Crosshair);

                ImGui::Spacing();
                UI::SectionHeader("ATALHO DO MENU");
                HotkeyButton("Tecla do Menu", g.MenuKey, halfW);
            }
            ImGui::EndChild();

            ImGui::EndTabItem();
        }



        ImGui::EndTabBar();
    }

    ImGui::End();
}