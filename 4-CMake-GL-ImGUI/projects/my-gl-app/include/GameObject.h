/* Start Header ************************************************************************/
/*!
\file		GameObject.h
\author     Hugo Low Ren Hao, low.h, 2402272
\par        low.h@digipen.edu
\date		October, 1st, 2025
\brief      This file contains the GameObject class. A GameObject is an entity in the 
            game world that can have multiple components attached to it.
*/
/* End Header **************************************************************************/

#pragma once

#include "Component.h"
#include <vector>
#include <memory> //for unique_ptr
#include <unordered_map>
#include <typeindex>//use this for typeid to make it map-friendly
#include <string>

class GameObject {
public:
	//constructor
    GameObject(const std::string& name);

	// Adds a component of type T to the game object
    template <typename T, typename... TArgs>//typename... is a variadic template
    T* addComponent(TArgs&&... args) {//forwarding reference

        //a variadic template can hold as many types as it wants 
        //basically allows me to make a template for a component
        //with any number of arguments
        //pretty cool tbh

        //prasanna my GOAT
        //std::forward reverts the argument to its state before
        //it was passed through the function
        auto newComponent = std::make_unique<T>(std::forward<TArgs>(args)...);
        T* componentPtr = newComponent.get();

        //attach this new component to a map of them
        m_components[std::type_index(typeid(T))] = std::move(newComponent);
        //typeid() creates a unique id of type type_info
		//type_info is a unique type information that can only have one instance
		//type_index is a wrapper around type_info that allows it to be used in any container

        return componentPtr;
    }

	// Gets the component of type T if it exists, otherwise returns nullptr
    template <typename T>
    T* getComponent() {
        auto iterator = m_components.find(std::type_index(typeid(T)));//look through map
        if (iterator != m_components.end()) {//if component was found
            //get that thang
            return static_cast<T*>(iterator->second.get());
        }

        return nullptr;
    }
	// Checks if the game object has a component of type T
    template <typename T>
    bool hasComponent() const {
        //count returns number of keys found in the map
        //unordered map does not allow duplicate keys
        //so will return 1 or 0
        return m_components.count(std::type_index(typeid(T)));
    }

	// Returns the name of the game object
    const std::string& getName() const {
        return m_name;
    }

private:
    std::string m_name;//name of object

	// Map of components attached to the game object
    std::unordered_map<std::type_index, std::unique_ptr<Component>> m_components;
    
};