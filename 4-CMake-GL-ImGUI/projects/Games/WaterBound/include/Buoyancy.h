/* Start Header ************************************************************************/
/*!
\file        WaterSystem.h
\author      Asha Mathyalakan, asha.m, 
             2402886 , 100%
\par         asha.m@digipen.edu
\date        January, 28, 2026
\brief       Buoyancy system to handling water physics

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "GameObjectManager.h"
 
class BuoyancySystem {
public:
    // Apply buoyancy forces and water drag to all floating objects.
    // Call once per frame after WaterRenderer::update().
    void update(GameObjectManager& manager, float deltaTime);
};