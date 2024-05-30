#pragma once

class Time {
public:
    static double time();

    static double deltaTime();

private:
    static void onUpdate();

    friend class App;
};
