#include "../Header/window.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "../Header/editor.h"

Editor::Editor(Window& window) :
	window { window }
{
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

	// Setup platform/renderer bindings (example with GLFW and OpenGL)
	ImGui_ImplGlfw_InitForOpenGL(window.getGLFWwindow(), true);		// window is your GLFW window
	ImGui_ImplOpenGL3_Init("#version 450");							// or appropriate GLSL version
}

void Editor::update() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::ShowDemoWindow(); // Show demo window! :)

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

Editor::~Editor() {
	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();
}
