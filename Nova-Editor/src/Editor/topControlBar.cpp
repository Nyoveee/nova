//#include "topControlBar.h"
//
//#include "imgui.h"
//#include "IconsFontAwesome6.h"
//
//constexpr float buttonSize = 25.f;
//
//TopControlBar::TopControlBar() {
//
//}
//
//void TopControlBar::update() {
//	ImGui::Begin("##TopControlBar", nullptr, ImGuiWindowFlags_NoTitleBar);
//
//	float titleBarHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
//
//	ImVec2 parent_content_size = ImGui::GetContentRegionAvail();
//	ImVec2 child_size = ImVec2(buttonSize * 3, buttonSize + ImGui::GetStyle().FramePadding.y); // Desired child window size
//
//	// Calculate centered position
//	float child_pos_x = (parent_content_size.x - child_size.x) * 0.5f;
//
//	ImGui::SetCursorPos(ImVec2(child_pos_x, titleBarHeight));
//
//	ImGui::BeginChild("CenteredChild", child_size);
//
//	// Content of the centered child window
//	if (ImGui::Button(ICON_FA_PLAY "", ImVec2{ buttonSize, buttonSize })) {
//
//	}
//
//	ImGui::SameLine();
//
//	if (ImGui::Button(ICON_FA_STOP "", ImVec2{ buttonSize, buttonSize })) {
//
//	}
//	
//	ImGui::EndChild();
//
//
//
//	ImGui::End();
//}
