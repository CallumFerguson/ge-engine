#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace GameEngine {

struct TransformComponent {
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::quat rotation = glm::identity<glm::quat>();
    glm::vec3 scale{1.0f, 1.0f, 1.0f};

    glm::mat4 localModel();
};

struct NameComponent {
    std::string name;
};

class CameraComponent {
public:
    explicit CameraComponent(float fieldOfView);

    const glm::mat4 &projection();

    static glm::mat4 transformToView(const TransformComponent &transform);

private:
    glm::mat4 m_projection{};
    float m_aspectRatio;
    float m_fov;
    float m_nearClippingPlane = 0.1;
    float m_farClippingPlane = 1000;
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
HAS_MEMBER_FUNCTION(onCustomRenderPass, hasOnCustomRenderPass)
HAS_MEMBER_FUNCTION(onMainRenderPass, hasOnMainRenderPass)

class ScriptableEntity;

struct NativeScriptComponent {
    struct NSCInstanceFunctions {
        bool instantiated = false;
        std::function<ScriptableEntity *()> instance;
        std::function<void()> destroyInstance;

        std::function<void(ScriptableEntity *passedInstance)> onStart;
        std::function<void(ScriptableEntity *passedInstance)> onUpdate;
        std::function<void(ScriptableEntity *passedInstance)> onImGui;
        std::function<void(ScriptableEntity *passedInstance)> onCustomRenderPass;
        std::function<void(ScriptableEntity *passedInstance)> onMainRenderPass;

        NSCInstanceFunctions() = default;

        ~NSCInstanceFunctions() {
            if (destroyInstance) {
                destroyInstance();
            }
        }

        NSCInstanceFunctions(const NSCInstanceFunctions &) = default;

        NSCInstanceFunctions &operator=(const NSCInstanceFunctions &) = default;

        NSCInstanceFunctions(NSCInstanceFunctions &&other) noexcept = default;

        NSCInstanceFunctions &operator=(NSCInstanceFunctions &&) = default;
    };

    std::vector<NSCInstanceFunctions> instancesFunctions;

    template<typename T, typename... Args>
    T &bind(Args &&... args) {
        T *scriptableEntityInstance = new T(args...);

        auto &nscInstanceFunctions = instancesFunctions.emplace_back();

        nscInstanceFunctions.instance = [scriptableEntityInstance]() {
            return scriptableEntityInstance;
        };
        nscInstanceFunctions.destroyInstance = [scriptableEntityInstance]() mutable {
            delete (T *) scriptableEntityInstance;
            scriptableEntityInstance = nullptr;
        };
        nscInstanceFunctions.onStart = [](ScriptableEntity *passedInstance) {
            if constexpr (hasOnStart<T>::value) {
                ((T *) passedInstance)->onStart();
            }
        };
        nscInstanceFunctions.onUpdate = [](ScriptableEntity *passedInstance) {
            if constexpr (hasOnUpdate<T>::value) {
                ((T *) passedInstance)->onUpdate();
            }
        };
        nscInstanceFunctions.onImGui = [](ScriptableEntity *passedInstance) {
            if constexpr (hasOnImGui<T>::value) {
                ((T *) passedInstance)->onImGui();
            }
        };
        nscInstanceFunctions.onCustomRenderPass = [](ScriptableEntity *passedInstance) {
            if constexpr (hasOnCustomRenderPass<T>::value) {
                ((T *) passedInstance)->onCustomRenderPass();
            }
        };
        nscInstanceFunctions.onMainRenderPass = [](ScriptableEntity *passedInstance) {
            if constexpr (hasOnMainRenderPass<T>::value) {
                ((T *) passedInstance)->onMainRenderPass();
            }
        };

        return *scriptableEntityInstance;
    }
};

}
