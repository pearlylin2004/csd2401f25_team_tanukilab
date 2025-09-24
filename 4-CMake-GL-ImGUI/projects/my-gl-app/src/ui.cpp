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

    //testing
    void drawControls() {
        ImGui::Begin("Controls");
        ImGui::Text("Player Controls");
        ImGui::Text("WASD to move");
        if (ImGui::Button("Print Hello")) {
            std::cout << "Hello from second window!" << std::endl;
        }
        ImGui::End();
    }

    //debugging window
    void drawDebug(std::string messages) {
        ImGui::Begin("Debug Info");
        ImGui::Text("This is a debug window.");
        ImGui::Text("%s", messages.c_str());

        /*for (const auto& msg : messages) {
            ImGui::Text("%s", msg.c_str());
        }

        // auto-scroll
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);*/
        ImGui::End();
	}
}