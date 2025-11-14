/* Start Header ************************************************************************/
/*!
\file       ui.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 90%
            Pearly Lin Lee Ying, p.lin, 2401591
            - 10% of the file
\par        p.lin@digipen.edu
\par		cheeqing.tan@digipen.edu
\date       September, 17th, 2025
\brief      definition of functions, drawPerformance, drawControls, drawDebug, shapeControl
            for the imgui windows

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "Editor/themeManager.h"

void ThemeManager::themeSinLeDark() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // start with dark theme
    ImGui::StyleColorsDark();

    // button
    colors[ImGuiCol_Button] = ImVec4(0.75f, 0.35f, 0.55f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.85f, 0.45f, 0.65f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.65f, 0.25f, 0.45f, 1.00f);

    // header
    colors[ImGuiCol_Header] = ImVec4(0.75f, 0.35f, 0.55f, 0.45f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.85f, 0.45f, 0.65f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.65f, 0.25f, 0.45f, 1.00f);

    // slider
    colors[ImGuiCol_SliderGrab] = ImVec4(0.75f, 0.35f, 0.55f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.85f, 0.45f, 0.65f, 1.00f);

    colors[ImGuiCol_CheckMark] = ImVec4(0.75f, 0.35f, 0.55f, 1.00f);

    // tab
    colors[ImGuiCol_Tab] = ImVec4(0.88f, 0.70f, 0.80f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.95f, 0.75f, 0.85f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.80f, 0.60f, 0.70f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.90f, 0.80f, 0.85f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.85f, 0.65f, 0.75f, 1.00f);

    // pinkie borders
    colors[ImGuiCol_Border] = ImVec4(0.40f, 0.20f, 0.30f, 1.00f);
}

void ThemeManager::themeSinLeLight() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Light pink backgrounds
    colors[ImGuiCol_WindowBg] = ImVec4(0.95f, 0.90f, 0.92f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.92f, 0.87f, 0.89f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.94f, 0.89f, 0.91f, 0.98f);

    // Dark text for contrast
    colors[ImGuiCol_Text] = ImVec4(0.20f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.45f, 0.48f, 1.00f);

    // Soft pink buttons
    colors[ImGuiCol_Button] = ImVec4(0.85f, 0.65f, 0.75f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.90f, 0.70f, 0.80f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.80f, 0.60f, 0.70f, 1.00f);

    // Input fields
    colors[ImGuiCol_FrameBg] = ImVec4(0.88f, 0.78f, 0.82f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.92f, 0.82f, 0.86f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.96f, 0.86f, 0.90f, 1.00f);

    // tab
    colors[ImGuiCol_Tab] = ImVec4(0.88f, 0.70f, 0.80f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.95f, 0.75f, 0.85f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.80f, 0.60f, 0.70f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.90f, 0.80f, 0.85f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.85f, 0.65f, 0.75f, 1.00f);

    style.WindowRounding = 8.0f;
    style.FrameRounding = 6.0f;
}

void ThemeManager::themeForest() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Dark forest green background
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.12f, 0.08f, 0.94f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.14f, 0.10f, 1.00f);

    // Light tan text
    colors[ImGuiCol_Text] = ImVec4(0.92f, 0.88f, 0.80f, 1.00f);

    // Moss green buttons
    colors[ImGuiCol_Button] = ImVec4(0.35f, 0.55f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.45f, 0.65f, 0.45f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.25f, 0.45f, 0.25f, 1.00f);

    // Dark green frames
    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.18f, 0.12f, 1.00f);
}
