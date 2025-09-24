/* Start Header ************************************************************************/
/*!
\file		ui.cpp
\author
\par
\date		September, 24, 2025
\brief		
*/
/* End Header **************************************************************************/

#include "ui.h"

namespace UI {
    void drawDemo(float& rotationSpeed, float& angleOfRotation) {
        ImGui::Begin("OpenGL 3.3 Demo");
        ImGui::Text("Hello, ImGui!");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
            1000.0f / ImGui::GetIO().Framerate,
            ImGui::GetIO().Framerate);
        ImGui::SliderFloat("Rotation Speed", &rotationSpeed, 0.0f, 0.1f);
        if (ImGui::Button("Reset Rotation")) {
            angleOfRotation = 0.0f;
        }
        ImGui::End();
    }

    void drawControls() {
        ImGui::Begin("Controls");
        ImGui::Text("Player Controls");
        ImGui::Text("WASD to move");
        if (ImGui::Button("Print Hello")) {
            std::cout << "Hello from second window!" << std::endl;
        }
        ImGui::End();
    }


}