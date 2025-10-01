/* Start Header ************************************************************************/
/*!
\file       ui.cpp
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       September, 17th, 2025
\brief      

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "ui.h"

namespace UI {
    std::vector<std::string> debugMessages; //not sure if this needs to be change

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
			debugMessages.push_back("Added a circle. Total circles: " + std::to_string(circles.size()));
        }
        if (!circles.empty() && ImGui::Button("Remove Last Circle")) {
            circles.pop_back();
            debugMessages.push_back("Removed a circle. Total circles: " + std::to_string(circles.size()));
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
    void drawDebug(std::vector<std::string> debugMessages) {
        ImGui::Begin("Debug Info");
        ImGui::Text("This is a debug window.");
        //ImGui::Text("%s", messages.c_str());

        for (const auto& msg : debugMessages) {
            ImGui::Text("%s", msg.c_str());
        }

        //auto-scroll if at the bottom
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::End();
	}
    //one imgui window to control all "shapes"
}