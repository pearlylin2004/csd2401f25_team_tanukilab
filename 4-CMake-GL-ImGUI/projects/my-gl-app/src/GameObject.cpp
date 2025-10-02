/* Start Header ************************************************************************/
/*!
\file		GameObject.cpp
\author     Hugo Low Ren Hao, low.h, 2402272
\par        low.h@digipen.edu
\date		October, 1st, 2025
\brief      This file contains the GameObject class. A GameObject is an entity in the
            game world that can have multiple components attached to it.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#include "GameObject.h"

//constructor
GameObject::GameObject(const std::string& name) : m_name(name) {}

const std::string& GameObject::getObjectName() const {
    return m_name;
}