#pragma once
#include <string>
#include <vector>

class Console {
private:
    bool autoScroll = true;
    bool showInfo = true;
    bool showWarnings = true;
    bool showErrors = true;

public:
    void update();
};