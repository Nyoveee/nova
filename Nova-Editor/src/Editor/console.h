#pragma once
#include <string>
#include <vector>
#include "imgui.h"
#include "Internal/Logger.h"

class Console {
private:
    bool autoScroll = false;
    bool showInfo = true;
    bool showDebug = false;
    bool showWarnings = true;
    bool showErrors = true;
    ImVec4 consoleTabColor;
    LogLevel maxLogLevel;
    
public:
    void update();
};