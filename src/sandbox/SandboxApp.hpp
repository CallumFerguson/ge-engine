#pragma once

#include "../engine/App.hpp"
#include <iostream>

class SandboxApp {
public:
    SandboxApp();

    void run();

private:
    App m_app;
};
