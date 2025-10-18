/* Start Header ************************************************************************/
/*!
\file       ui.cpp
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       September, 17th, 2025
\brief      definition of functions, drawPerformance, drawControls, drawDebug, shapeControl
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

    // temp, get ready to load from json in the future
	std::unordered_map<ShapeType, ShapeDefaults> shapePresets = {
		{ ShapeType::Leaves, { 0.0f, 0.0f, 0.0f, 5.0f, 5.0f, 5.0f,
		{-20.f, 20.f}, {-20.f, 20.f}, {0.f, 100.f}, { 0.1f, 5.f }, { 0.1f, 5.f } } },
		{ ShapeType::Bullet, { 0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.0f } }
	};

    void drawPerformance() {
        ImGui::Begin("Performance Display");
        //time frame:
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
            1000.0f / ImGui::GetIO().Framerate,
            ImGui::GetIO().Framerate);
        ImGui::Separator(); 
        //average time

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
			ShapeDefaults& defaults = shapePresets[currentShape];
            
            if (ImGui::CollapsingHeader("Transform")) {
                if (ImGui::TreeNode("Position")) {
                    ImGui::SliderFloat("X", &t->x, defaults.posXRange.min, defaults.posXRange.max);
                    ImGui::SliderFloat("Y", &t->y, defaults.posYRange.min, defaults.posYRange.max);
                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Rotation")) {
                    ImGui::SliderFloat("Angle", &t->rotation, defaults.rotRange.min, defaults.rotRange.max);
                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Scale")) {
                    ImGui::SliderFloat("X", &t->scaleX, defaults.scaleXRange.min, defaults.scaleXRange.max);
                    ImGui::SliderFloat("Y", &t->scaleY, defaults.scaleYRange.min, defaults.scaleYRange.max);
                    //ImGui::SliderFloat("Z", &t->scaleZ, 0.1f, 10.f);
                    ImGui::TreePop();
                }
            }

            //reset
            if (ImGui::Button("Reset All")) {
				if (shapePresets.find(currentShape) != shapePresets.end()) {
					const ShapeDefaults& defaults = shapePresets[currentShape];
					t->x = defaults.posX;
					t->y = defaults.posY;
					t->rotation = defaults.rotation;
					t->scaleX = defaults.scaleX;
					t->scaleY = defaults.scaleY;
					t->scaleZ = defaults.scaleZ;
				}
                else {
					debugMessages.push_back("No presets found for the selected shape.");
                }
            }
        }
        ImGui::End();
    }

}