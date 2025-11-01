#include "console.h"
#include "Internal/Logger.h"
#include "imgui.h"

void Console::update() {
    ImGui::Begin("Console");

    // Filter checkboxes
    ImGui::Checkbox("Info", &showInfo);
    ImGui::SameLine();
    ImGui::Checkbox("Warnings", &showWarnings);
    ImGui::SameLine();
    ImGui::Checkbox("Errors", &showErrors);
    ImGui::SameLine();
    ImGui::Checkbox("Debug", &showDebug);
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &autoScroll);

    // Clear button
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        Logger::clearLogs();
    }

    ImGui::Separator();

    // Log display area
    ImGui::BeginChild("LogArea", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    const auto logEntries = Logger::getLogEntries();

    //static int lastLogCount = 0; // For tracking new log entries

    for (const auto& entry : logEntries) {
        bool shouldShow = false;
        ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // default white

        switch (entry.level) {
        case LogLevel::Info:
            shouldShow = showInfo;
            color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f); // Light gray
            break;
        case LogLevel::Warning:
            shouldShow = showWarnings;
            color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
            break;
        case LogLevel::Debug:
            shouldShow = showDebug;
            color = ImVec4(0.3f, 0.6f, 0.3f, 1.0f);  // Green
            break;
        case LogLevel::Error:
            shouldShow = showErrors;
            color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); // Red
            break;
        }

        if (shouldShow) {
            ImGui::PushStyleColor(ImGuiCol_Text, color);

            std::string levelStr;
            switch (entry.level) {
            case LogLevel::Info: levelStr = "[Info]"; break;
            case LogLevel::Debug: levelStr = "[Debug]"; break;
            case LogLevel::Warning: levelStr = "[Warning]"; break;
            case LogLevel::Error: levelStr = "[Error]"; break;
            }

            ImGui::TextUnformatted((entry.timestamp + " " + levelStr + " " + entry.message).c_str());
            ImGui::PopStyleColor();
        }
    }

    static bool previousAutoScroll = true; // remember previous frame state
    static int lastLogCount = 0;           // remember previous number of logs

    int currentLogCount = (int)logEntries.size();

    if (autoScroll && (!previousAutoScroll || currentLogCount != lastLogCount)) {
        ImGui::SetScrollHereY(1.0f);
    }

    previousAutoScroll = autoScroll;
    lastLogCount = currentLogCount;


    ImGui::EndChild();
    ImGui::End();
}