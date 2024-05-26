#pragma once

#include "ScriptableEntity.hpp"
#include <glm/glm.hpp>

struct TransformComponent {
    glm::vec3 position;
};

struct NameComponent {
    std::string name;
};

// Helper to detect if a class has a specific member function
#define HAS_MEMBER_FUNCTION(func, name)                                    \
template<typename T, typename = std::void_t<>>                             \
struct name : std::false_type {};                                          \
                                                                           \
template<typename T>                                                       \
struct name<T, std::void_t<decltype(std::declval<T>().func())>> : std::true_type {};

HAS_MEMBER_FUNCTION(onUpdate, hasOnUpdate)
HAS_MEMBER_FUNCTION(onFirstUpdate, hasOnFirstUpdate)

struct NativeScriptComponent {
    ScriptableEntity *instance = nullptr;

    std::function<void()> instantiate;

    std::function<void(ScriptableEntity *passedInstance)> onFirstUpdate;
    std::function<void(ScriptableEntity *passedInstance)> onUpdate;

    ~NativeScriptComponent() {
        delete instance;
    }

    template<typename T>
    void bind() {
        instantiate = [&]() { instance = new T(); };

        onFirstUpdate = [](ScriptableEntity *passedInstance) {
            if constexpr (hasOnFirstUpdate<T>::value) {
                ((T *) passedInstance)->onFirstUpdate();
            }
        };
        onUpdate = [](ScriptableEntity *passedInstance) {
            if constexpr (hasOnUpdate<T>::value) {
                ((T *) passedInstance)->onUpdate();
            }
        };
    }
};
