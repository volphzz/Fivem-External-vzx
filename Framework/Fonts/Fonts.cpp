#include "Fonts.hpp"
#include "FontsGlobal.hpp"
#include "FontAwesome.hpp"
#include "FontsTwo.hpp"
#include "FontInter.hpp"

namespace FWork {
    void Fonts::Initialize(ID3D11Device* Device) {
        ImGuiIO& io = ImGui::GetIO();

        // ── High-Performance Streamlined Glyph Range (Instant Rendering + Universal Symbols) ──
        ImFontGlyphRangesBuilder builder;
        builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
        builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
        builder.AddRanges(io.Fonts->GetGlyphRangesJapanese());
        builder.AddRanges(io.Fonts->GetGlyphRangesKorean());
        builder.AddRanges(io.Fonts->GetGlyphRangesThai());
        builder.AddRanges(io.Fonts->GetGlyphRangesVietnamese());
        builder.AddRanges(io.Fonts->GetGlyphRangesChineseSimplifiedCommon());

        // Custom Special Nickname Symbols (Latin Extended, Math Operators, Arrows, Geometric Shapes, Dingbats)
        static const ImWchar specialRanges[] = {
            0x0020, 0x00FF, // Basic Latin + Latin Supplement
            0x0100, 0x024F, // Latin Extended-A & B
            0x0370, 0x03FF, // Greek
            0x0400, 0x052F, // Cyrillic & Supplement
            0x2000, 0x206F, // General Punctuation
            0x20A0, 0x20CF, // Currency Symbols
            0x2100, 0x214F, // Letterlike Symbols (™ , ℃, №, etc.)
            0x2190, 0x21FF, // Arrows (←, ↑, →, ↓, etc.)
            0x2200, 0x22FF, // Mathematical Operators (√, ∞, ≈, ≠, etc.)
            0x2460, 0x24FF, // Enclosed Alphanumerics (①, ②, Ⓐ, Ⓑ)
            0x2500, 0x257F, // Box Drawing
            0x25A0, 0x25FF, // Geometric Shapes (■, ◆, ▲, ▼, ★, etc.)
            0x2600, 0x26FF, // Misc Symbols (⚡, ☠, ☯, ⚔, ⚓, ♠, ♣, ♥, ♦, etc.)
            0x2700, 0x27BF, // Dingbats (✂, ✈, ✉, ✌, ✍, ❄, ❌, ❖, etc.)
            0
        };
        builder.AddRanges(specialRanges);

        static ImVector<ImWchar> fullRanges;
        fullRanges.clear();
        builder.BuildRanges(&fullRanges);

        ImFontConfig fontCfg;
        fontCfg.OversampleH = 2;
        fontCfg.OversampleV = 2;
        fontCfg.PixelSnapH = false;

        // Core ESP Fonts (Instantly loaded)
        InterRegular = Fonts::InterRegular = io.Fonts->AddFontFromMemoryCompressedTTF(InterRegular_compressed_data, InterRegular_compressed_size, 16, &fontCfg, fullRanges.Data);
        InterSemiBold = Fonts::InterSemiBold = io.Fonts->AddFontFromMemoryCompressedTTF(InterSemiBold_compressed_data, InterSemiBold_compressed_size, 16, &fontCfg, fullRanges.Data);
        InterBold = Fonts::InterBold = io.Fonts->AddFontFromMemoryCompressedTTF(InterBold_compressed_data, InterBold_compressed_size, 16, &fontCfg, fullRanges.Data);
        GeistBold = Fonts::GeistBold = io.Fonts->AddFontFromMemoryCompressedTTF(GeistBold_compressed_data, GeistBold_compressed_size, 20, &fontCfg, fullRanges.Data);

        // Fast Aliases to share the high-performance font textures
        InterRegular14 = InterRegular;
        InterMedium = InterRegular;
        InterBold12 = InterBold;
        InterExtraBold = InterBold;
        InterBlack = InterBold;
        InterLight = InterRegular;
        InterExtraLight = InterRegular;
        InterThin = InterRegular;
        GeistRegular = InterRegular;
        GeistRegularMedium = InterSemiBold;
        GeistMedium = InterRegular;
        GeistBoldMedium = GeistBold;

        // Merge Fallback Symbol/Emoji System Fonts for all player nicknames
        static const char* sysFallbackFonts[] = {
            "C:/Windows/Fonts/seguiemj.ttf",
            "C:/Windows/Fonts/seguisym.ttf",
            "C:/Windows/Fonts/segoeui.ttf"
        };

        ImFontConfig mergeCfg;
        mergeCfg.MergeMode = true;
        mergeCfg.OversampleH = 1;
        mergeCfg.OversampleV = 1;
        mergeCfg.PixelSnapH = true;

        for (const char* fPath : sysFallbackFonts) {
            if (GetFileAttributesA(fPath) != INVALID_FILE_ATTRIBUTES) {
                io.Fonts->AddFontFromFileTTF(fPath, 16.0f, &mergeCfg, specialRanges);
                break; // Merge the first best available system symbol font
            }
        }

        ImFontConfig customFontConfig;
        customFontConfig.MergeMode = true;
        customFontConfig.OversampleH = 1;
        customFontConfig.OversampleV = 1;
        customFontConfig.PixelSnapH = true;

        static const ImWchar customRanges[] = { 0xe000, 0xe204, 0x00 };
        IconWeapon = Fonts::IconWeapon = io.Fonts->AddFontFromMemoryCompressedTTF(weapon_compressed_data, weapon_compressed_size, 41.0f, &customFontConfig, customRanges);

        ImFontConfig FontAwesomeConfig;
        FontAwesomeConfig.GlyphMinAdvanceX = 25.f * (2.0f / 3.0f);
        static const ImWchar IconRanges[] = {
            ICON_MIN_FA, ICON_MAX_FA, 0
        };

        FontAwesomeSolid = Fonts::FontAwesomeSolid = io.Fonts->AddFontFromMemoryCompressedTTF(FontAwesomeSolid_compressed_data, FontAwesomeSolid_compressed_size, 25.f * (2.0f / 3.0f), &FontAwesomeConfig, &IconRanges[0]);
        FontAwesomeRegular = Fonts::FontAwesomeRegular = FontAwesomeSolid;
        FontAwesomeSolid14 = FontAwesomeSolid;
        FontAwesomeSolid18 = FontAwesomeSolid;
        FontAwesomeSolidBig = FontAwesomeSolid;
    }
}
