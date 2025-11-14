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

#include "Editor/editorManager.h"

AddObjWindow::AddObjWindow(Editor::ShapeControlState& Cshape, Editor::ShapeControlState& Rshape) :
    m_circleInput(Cshape),
    m_rectangleInput(Rshape) {}

void AddObjWindow::render(GameObjectManager& manager){
    ImGui::Begin("Create Object");

    //// spawning objects (double up every click)
    static int count = 0;
    static int twentyCount = 0;
    if (ImGui::Button("Spawn Object")) {
        int spawnCount = (count == 0) ? 1 : count * 2;
        if (spawnCount == 2048)
            spawnCount = 2500; //cap at 400 objects

        // if player2 not exist then create player2
        if (!manager.getGameObject("player2")) {
            GameObject* player2 = manager.createGameObject("player2");

            player2->addComponent<Transform>();
            player2->addComponent<Physics>();
            Render* render = player2->addComponent<Render>();

            render->modelRef = renderer::models[static_cast<int>(shape::square)];
            render->hasTex = true;
            render->texFile = "./assets/Sample_tilemap.jpg";
            render->texChanged = true;
        }

        for (int i = count; i < spawnCount; ++i) {
            std::string cloneName = "Player_Clone(" + std::to_string(i + 1) + ")";
            GameObject* newObj = manager.CloneGameObject("player2", cloneName);
            newObj->changeAutoMove(true); // set to true so it auto move

            Transform* transform = newObj->getComponent<Transform>();
            Physics* physics = newObj->getComponent<Physics>();
            if (!physics) {
                physics = newObj->addComponent<Physics>();
            }

            float randomX = (rand() % 2000) / 100.0f;
            float randomY = (rand() % 1000) / 100.0f;     // 0.0 to 9.99
            float rotation = (rand() % 36000) / 100.0f;   // 0.0 to 359.99
            float scaleX = ((rand() % 300) / 100.0f) + 1; // 1.00 to 3.00
            float scaleY = ((rand() % 300) / 100.0f) + 1;
            float velX = ((rand() % 300) / 100.0f) - 1;   // -1.00 to 2.00
            float velY = ((rand() % 300) / 100.0f) - 1;

            // avoid zero velocity
            if (velX == 0.0f) velX = 0.5f;
            if (velY == 0.0f) velY = 0.5f;

            // make sure 
            if (rand() % 2 == 0)
                randomX = -randomX;
            if (rand() % 2 == 1)
                randomY = -randomY;

            transform->x = randomX;
            transform->y = randomY;
            transform->rotation = rotation;
            transform->scaleX = scaleX;
            transform->scaleY = scaleY;

            physics->gravity = 0.0;
            physics->dynamics.velocity.x = velX;
            physics->dynamics.velocity.y = velY;
        }

        count = spawnCount;
    }


    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    //ImGui::PushItemWidth(150.0f);

    // For drawing circle
    ImGui::Text("X");
    ImGui::SameLine();
    ImGui::InputFloat("##X", &m_circleInput.inputX, 0.1f);

    ImGui::Text("Y");
    ImGui::SameLine();
    ImGui::InputFloat("##Y", &m_circleInput.inputY, 0.1f);

    ImGui::Text("Z");
    ImGui::SameLine();
    ImGui::InputFloat("##Z", &m_circleInput.inputZ, 0.1f);
    ImGui::Spacing();

    //ImGui::PopItemWidth();

    if (ImGui::Button("Add Circle")) {
        std::vector<GameObject*> gameObjects;
        manager.getAllGameObjects(gameObjects);

        GameObject* circleObj = manager.createGameObject("Circle_" + std::to_string(gameObjects.size()));

        Transform* transform = circleObj->addComponent<Transform>();
        Render* render = circleObj->addComponent<Render>();
        render->modelRef = renderer::models[static_cast<int>(shape::circle) + static_cast<int>(shape::circle)];
        //render->modelRef = renderer::models[static_cast<int>(shape::circle)];
        transform->x = m_circleInput.inputX;
        transform->y = m_circleInput.inputY;
        transform->z = m_circleInput.inputZ;

        //auto* coll = circleObj->addComponent<CollisionInfo>();
        DebugLog::addMessage("Circle created at (" + std::to_string(transform->x) + ", " + std::to_string(transform->y) + ", " + std::to_string(transform->z) + ")\n");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // For drawing rectangle        
    ImGui::Text("X");
    ImGui::SameLine();
    ImGui::InputFloat("##X1", &m_rectangleInput.inputX, 0.1f);

    ImGui::Text("Y");
    ImGui::SameLine();
    ImGui::InputFloat("##Y1", &m_rectangleInput.inputY, 0.1f);

    ImGui::Text("Z");
    ImGui::SameLine();
    ImGui::InputFloat("##Z1", &m_rectangleInput.inputZ, 0.1f);
    ImGui::Spacing();

    if (ImGui::Button("Add Rectangle")) {
        std::vector<GameObject*> gameObjects;
        manager.getAllGameObjects(gameObjects);

        GameObject* recobj = manager.createGameObject("Rectangle_" + std::to_string(gameObjects.size()));

        Transform* transform = recobj->addComponent<Transform>();
        Render* render = recobj->addComponent<Render>();
        render->modelRef = renderer::models[static_cast<int>(shape::square)];
        //render->modelRef = renderer::setup_square({ 1.0f, 0.f, 0.f });

        transform->x = m_rectangleInput.inputX;
        transform->y = m_rectangleInput.inputY;
        transform->z = m_rectangleInput.inputZ;

        //auto* col = recobj->addComponent<CollisionInfo>();
        DebugLog::addMessage("Rectangle created at (" + std::to_string(transform->x) + ", " + std::to_string(transform->y) + ", " + std::to_string(transform->z) + ")\n");
    }

    ImGui::End();
}