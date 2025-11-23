#pragma once
#include <string>
#include <vector>

class Console {
private:
    bool autoScroll = false;
    bool showInfo = true;
    bool showDebug = false;
    bool showWarnings = true;
    bool showErrors = true;

public:
    void update();
};