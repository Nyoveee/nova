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

    // testAutoScroll();

    static bool previousAutoScroll = true; // remember previous frame state
    static int lastLogCount = 0;           // remember previous number of logs

    int currentLogCount = (int)logEntries.size();

    //For this one, once the auto-scroll is on and you move up it stays at the place and it dosent move back down.
    /*if (autoScroll && (!previousAutoScroll || currentLogCount != lastLogCount)) {
        ImGui::SetScrollHereY(1.0f);
    }*/

    //This one forces it to go down as long as tge auto-scroll is on, if off then can move around freely
    if (autoScroll) {
        ImGui::SetScrollHereY(1.0f);
    }

    previousAutoScroll = autoScroll;
    lastLogCount = currentLogCount;


    ImGui::EndChild();
    ImGui::End();
}


//void Console::testAutoScroll() {
//    //// Test with rapid logging
//    //for (int i = 0; i < 20; i++) {
//    //    Logger::log(LogLevel::Info, "Test message " + std::to_string(i));
//    //}
//
//    //// Test with different log levels
//    //Logger::log(LogLevel::Warning, "This should trigger auto-scroll");
//    //Logger::log(LogLevel::Error, "Error message test");
//    //Logger::log(LogLevel::Debug, "Debug message");
//    
//    static bool hasRun = false; // ensures this runs only once
//    if (hasRun) return;         // skip after first time
//    hasRun = true;
//
//    // Test with rapid logging
//    for (int i = 0; i < 20; i++) {
//        Logger::log(LogLevel::Info, "Test message " + std::to_string(i));
//    }
//
//    // Test with different log levels
//    Logger::log(LogLevel::Warning, "This should trigger auto-scroll");
//    Logger::log(LogLevel::Error, "Error message test");
//    Logger::log(LogLevel::Debug, "Debug message");
//}
//
//
