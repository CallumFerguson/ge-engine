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

HAS_MEMBER_FUNCTION(onStart, hasOnStart)
HAS_MEMBER_FUNCTION(onUpdate, hasOnUpdate)
HAS_MEMBER_FUNCTION(onImGui, hasOnImGui)
HAS_MEMBER_FUNCTION(onRender, hasOnRender)

struct NativeScriptComponent {
    ScriptableEntity *instance = nullptr;

    std::function<void()> instantiate;
    std::function<void()> destroyInstance;

    std::function<void(ScriptableEntity *passedInstance)> onStart;
    std::function<void(ScriptableEntity *passedInstance)> onUpdate;
    std::function<void(ScriptableEntity *passedInstance)> onImGui;
    std::function<void(ScriptableEntity *passedInstance)> onRender;

    ~NativeScriptComponent() {
        destroyInstance();
        delete instance;
    }

    template<typename T, typename... Args>
    void bind(Args &&... args) {
        instantiate = [this, args...]() {
            instance = new T(args...);
        };

        destroyInstance = [&]() {
            delete (T *) instance;
            instance = nullptr;
        };

        onStart = [](ScriptableEntity *passedInstance) {
            if constexpr (hasOnStart<T>::value) {
                ((T *) passedInstance)->onStart();
            }
        };
        onUpdate = [](ScriptableEntity *passedInstance) {
            if constexpr (hasOnUpdate<T>::value) {
                ((T *) passedInstance)->onUpdate();
            }
        };
        onImGui = [](ScriptableEntity *passedInstance) {
            if constexpr (hasOnImGui<T>::value) {
                ((T *) passedInstance)->onImGui();
            }
        };
        onRender = [](ScriptableEntity *passedInstance) {
            if constexpr (hasOnRender<T>::value) {
                ((T *) passedInstance)->onRender();
            }
        };
    }
};
