#pragma once

#include <vector>
#include <set>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace GameEngine {

class Entity;

struct InfoComponent {
    std::vector<std::string> componentNames;
    std::map<std::string, std::function<nlohmann::json()>> componentToJSONFunctions;

    std::vector<std::string> scriptNames;
    std::map<std::string, std::function<nlohmann::json()>> scriptToJSONFunctions;
};

struct NameComponent {
    std::string name;

    nlohmann::json toJSON();

    [[nodiscard]] const char *objectName() const {
        return "NameComponent";
    }
};

class TransformComponent {
public:
    glm::vec3 localPosition{0.0f, 0.0f, 0.0f};
    glm::quat localRotation = glm::identity<glm::quat>();
    glm::vec3 localScale{1.0f, 1.0f, 1.0f};

    glm::mat4 localModelMatrix();

    entt::entity parentENTTHandle() const;

    const std::vector<entt::entity> &childrenENTTHandles() const;

    nlohmann::json toJSON();

    [[nodiscard]] const char *objectName() const {
        return "TransformComponent";
    }

private:
    entt::entity m_parentENTTHandle = entt::null;
    std::vector<entt::entity> m_childrenENTTHandles;

    friend class Entity;

    friend class Scene;
};

class CameraComponent {
public:
    explicit CameraComponent(float fieldOfView);

    const glm::mat4 &projection();

    static glm::mat4 transformToView(const TransformComponent &transform);

    void onImGui();

    nlohmann::json toJSON();

    [[nodiscard]] const char *objectName() const {
        return "CameraComponent";
    }

private:
    glm::mat4 m_projection;
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
        ScriptableEntity *instance = nullptr;
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

        NSCInstanceFunctions(const NSCInstanceFunctions &) = delete;

        NSCInstanceFunctions &operator=(const NSCInstanceFunctions &) = delete;

        NSCInstanceFunctions(NSCInstanceFunctions &&other) noexcept:
                instantiated(other.instantiated),
                instance(other.instance),
                onStart(std::move(other.onStart)),
                onUpdate(std::move(other.onUpdate)),
                onImGui(std::move(other.onImGui)),
                onCustomRenderPass(std::move(other.onCustomRenderPass)),
                onMainRenderPass(std::move(other.onMainRenderPass)) {
            other.destroyInstance = nullptr;
            other.instance = nullptr;
        }

        NSCInstanceFunctions &operator=(NSCInstanceFunctions &&other) noexcept {
            if (this != &other) {
                instantiated = other.instantiated;
                instance = other.instance;
                destroyInstance = std::move(other.destroyInstance);
                onStart = std::move(other.onStart);
                onUpdate = std::move(other.onUpdate);
                onImGui = std::move(other.onImGui);
                onCustomRenderPass = std::move(other.onCustomRenderPass);
                onMainRenderPass = std::move(other.onMainRenderPass);

                other.destroyInstance = nullptr;
                other.instance = nullptr;
            }
            return *this;
        }
    };

    std::vector<NSCInstanceFunctions> instancesFunctions;

    NativeScriptComponent() = default;

    ~NativeScriptComponent() = default;

    NativeScriptComponent(const NativeScriptComponent &) = delete;

    NativeScriptComponent &operator=(const NativeScriptComponent &) = delete;

    NativeScriptComponent(NativeScriptComponent &&) = default;

    NativeScriptComponent &operator=(NativeScriptComponent &&) = default;

    template<typename T, typename... Args>
    T &bind(Args &&... args) {
        T *scriptableEntityInstance = new T(args...);

        auto &nscInstanceFunctions = instancesFunctions.emplace_back();

        nscInstanceFunctions.instance = scriptableEntityInstance;
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
