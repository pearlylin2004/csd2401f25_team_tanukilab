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

    //to add or remove circles
    void drawControls(std::vector<Collision::Circle>& circles) {
        ImGui::Begin("Controls");
        ImGui::Text("Player Controls");
        if (ImGui::Button("Add Circle")) {
            circles.emplace_back(Collision::Circle({ 0.0f, 0.0f }, 50.0f));
        }
        if (!circles.empty() && ImGui::Button("Remove Last Circle")) {
            circles.pop_back();
        }
        ImGui::Separator();
        for (size_t i = 0; i < circles.size(); i++) {
            ImGui::Text("Circle %d", static_cast<int>(i));

            // Unique IDs for sliders without std::to_string
            ImGui::PushID(static_cast<int>(i));

            ImGui::SliderFloat2("Center",
                (float*)&circles[i].getCenterRef(), -500.0f, 500.0f);
            ImGui::SliderFloat("Radius",
                &circles[i].getRadiusRef(), 1.0f, 200.0f);

            ImGui::PopID();
        }
        ImGui::End();
    }

    //debugging window
    void drawDebug(std::string messages) {
        ImGui::Begin("Debug Info");
        ImGui::Text("This is a debug window.");
        ImGui::Text("%s", messages.c_str());
		//for multiple messages
        /*for (const auto& msg : messages) {
            ImGui::Text("%s", msg.c_str());
        }

        // auto-scroll
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);*/
        ImGui::End();
	}
}