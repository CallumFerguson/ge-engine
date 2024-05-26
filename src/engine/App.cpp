#include "App.hpp"

Scene &App::getActiveScene() {
    return m_scene;
}

void App::onUpdate() {
    getActiveScene().onUpdate();
}

void App::run() {
    while (!m_window.shouldClose()) {
        m_window.onUpdate();
        onUpdate();

    }
}
