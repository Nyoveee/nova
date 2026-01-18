#pragma once

struct RenderConfig;
class Editor;

class RenderConfigUI {
public:
    RenderConfigUI(Editor& editor);

public:
    void update();

private:
    Editor& editor;

    // Configuration data
    RenderConfig& renderConfig;
};