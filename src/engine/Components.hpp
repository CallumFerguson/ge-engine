#pragma once

#include "ScriptableEntity.hpp"
#include <glm/glm.hpp>

struct TransformComponent {
    glm::vec3 position;
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
    int x = 5;

    template<typename T>
    void bind() {
        T *t = new T();
        if constexpr (hasOnFirstUpdate<T>::value) {
            t->onFirstUpdate();
        }
        if constexpr (hasOnUpdate<T>::value) {
            t->onUpdate();
        }
        delete t;
    }
};
