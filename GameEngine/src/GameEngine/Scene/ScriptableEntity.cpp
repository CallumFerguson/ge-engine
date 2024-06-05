#include "ScriptableEntity.hpp"

namespace GameEngine {

Entity &ScriptableEntity::getEntity() {
    return m_entity;
}

std::string &ScriptableEntity::imGuiName() {
    static std::string s_name;
    return s_name;
}

void ScriptableEntity::onImGuiInspector() {

}

}
