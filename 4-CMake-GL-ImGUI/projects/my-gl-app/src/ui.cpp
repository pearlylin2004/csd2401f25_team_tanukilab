/* Start Header ************************************************************************/
/*!
\file       ui.cpp
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       September, 17th, 2025
\brief      definition of functions, drawDemo, drawControls, drawDebug, shapeControl
			for the imgui windows

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "ui.h"

namespace UI {
    std::vector<std::string> debugMessages;
    float xAdd = 0.0f, yAdd = 0.0f;

    void drawDemo() {
        ImGui::Begin("OpenGL 3.3 Demo");
        ImGui::Text("Hello, ImGui!");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
            1000.0f / ImGui::GetIO().Framerate,
            ImGui::GetIO().Framerate);
        ImGui::End();
    }

    //to add or remove circles
    void drawControls(std::vector<Collision::Circle>& circles) {
        ImGui::Begin("Controls");
        ImGui::Text("Player Controls");
		
        if (ImGui::Button("Add Circle")) {
            circles.emplace_back(Collision::Circle({ UI::xAdd, UI::yAdd }, 5.0f));
            UI::xAdd++;
            UI::yAdd++;
			debugMessages.push_back("Added a circle. Total circles: " + std::to_string(circles.size()));
        }
        if (!circles.empty() && ImGui::Button("Remove Last Circle")) {
            circles.pop_back();
            UI::xAdd--;
            UI::yAdd--;
            debugMessages.push_back("Removed a circle. Total circles: " + std::to_string(circles.size()));
        }
        ImGui::Separator();
        for (size_t i = 0; i < circles.size(); i++) {
            ImGui::Text("Circle %d", static_cast<int>(i));

            //unique IDs for sliders without std::to_string
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
    void drawDebug() {
        ImGui::Begin("Debug Info");
        ImGui::Text("This is a debug window.");

        for (const auto& msg : debugMessages) {
            ImGui::Text("%s", msg.c_str());
        }

        //auto-scroll if at the bottom
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::End();
	}

    //shape control
    void shapeControl(Transform& t1, Transform& t2) {
        if (ImGui::Begin("Shape Control")) {
            ImGui::Text("Transform Component");
            static ShapeType currentShape = ShapeType::Leaves;

            if (ImGui::Button("Leaves")) {
				currentShape = ShapeType::Leaves;
            }
			ImGui::SameLine();
            if (ImGui::Button("Bullet")) {
				currentShape = ShapeType::Bullet;
            }

			//check which shape is selected
            Transform* t = (currentShape == ShapeType::Leaves) ? &t1 : &t2;
            
            //position 
            ImGui::SliderFloat("Position X", &t->x, -500.f, 500.f);
            ImGui::SliderFloat("Position Y", &t->y, -500.f, 500.f);

            //rotation
            ImGui::SliderFloat("Rotation", &t->rotation, 0.f, 360.f);

            //scale
            ImGui::SliderFloat("Scale X", &t->scaleX, 0.1f, 10.f);
            ImGui::SliderFloat("Scale Y", &t->scaleY, 0.1f, 10.f);
            ImGui::SliderFloat("Scale Z", &t->scaleZ, 0.1f, 10.f);

            //this need change cuz its hardcoded for now
            //reset
            if (ImGui::Button("Reset All")) {
                if (currentShape == ShapeType::Leaves) {
                    t->x = 0.0f;
                    t->y = 0.0f;
                    t->rotation = 0.0f;
                    t->scaleX = 5.0f;
                    t->scaleY = 5.0f;
                    t->scaleZ = 5.0f;
                }
                else {
                    t->x = 0.0f;
                    t->y = 0.0f;
                    t->rotation = 0.5f;
                    t->scaleX = 0.5f;
                    t->scaleY = 0.5f;
                    t->scaleZ = 0.0f;
                }
            }
        }
        ImGui::End();
    }

	//docking setup
    /*void dockingSetUp() {
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking
            | ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoBringToFrontOnFocus
            | ImGuiWindowFlags_NoNavFocus
            | ImGuiWindowFlags_NoBackground;

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        ImGui::Begin("DockSpace Demo", nullptr, window_flags);
        ImGui::PopStyleVar(2);

        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

        ImGui::End();
    }*/
}