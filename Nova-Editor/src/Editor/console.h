#pragma once
#include <string>
#include <vector>

class Console {
private:
    bool autoScroll = false;
    bool showInfo = true;
    bool showDebug = true;
    bool showWarnings = true;
    bool showErrors = true;

public:
    //void testAutoScroll();
    void update();
};