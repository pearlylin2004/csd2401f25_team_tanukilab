/* Start Header ************************************************************************/
/*!
\file		GameObject.cpp
\author     Hugo Low Ren Hao, low.h, 2402272
\par        low.h@digipen.edu
\date		October, 1st, 2025
\brief      This file contains the GameObject class. A GameObject is an entity in the
            game world that can have multiple components attached to it.
*/
/* End Header **************************************************************************/
#include "GameObject.h"

//constructor
GameObject::GameObject(const std::string& name) : m_name(name) {}

const std::string& GameObject::getObjectName() const {
    return m_name;
}