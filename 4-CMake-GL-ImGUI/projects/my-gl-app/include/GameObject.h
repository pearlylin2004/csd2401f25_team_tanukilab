#pragma once

#include "Component.h"
#include <vector>
#include <memory> //for unique_ptr
#include <unordered_map>
#include <typeindex>//use this for typeid to make it map-friendly
#include <string>

class GameObject {
public:
    GameObject(const std::string& name);

    // Add components
    // Figure out templates NOW

    //this took awhile to figure out
    template <typename T, typename... TArgs>//typename... is a variadic template
    T* addComponent(TArgs&&... args) {//forwarding reference

        //a variadic template can hold as many types as it wants 
        //basically allows me to make a template for a component
        //with any number of arguments
        //pretty cool tbh

        //prasanna my GOAT
        auto newComponent = std::make_unique<T>(std::forward<TArgs>(args)...);
        T* componentPtr = newComponent.get();

        //attach this new component to a map of them
        m_components[std::type_index(typeid(T))] = std::move(newComponent);
        //typeid() creates a unique id so its easier to find in the map
        //type_index() makes it actually usable in an unordered map

        return componentPtr;
    }

    //used to check if a object has a certain component
    template <typename T>
    T* getComponent() {
        auto iterator = m_components.find(std::type_index(typeid(T)));//look through map
        if (iterator != m_components.end()) {//if component was found
            //get that thang
            return static_cast<T*>(iterator->second.get());
        }

        return nullptr;
    }

    template <typename T>
    bool hasComponent() const {
        //count returns number of keys found in the map
        //unordered map does not allow duplicate keys
        //so will return 1 or 0
        return m_components.count(std::type_index(typeid(T)));
    }

    const std::string& getName() const {
        return m_name;
    }
private:
    std::string m_name;//name of object

    std::unordered_map<std::type_index, std::unique_ptr<Component>> m_components;
    
};