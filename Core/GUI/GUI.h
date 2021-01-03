/**
 * @brief GUI for App
 */

#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace GUI {

void Init(GLFWwindow *window) {
  // Dear ImGui contextの設定を行います。
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  // Dear ImGui の style を決めます。
  ImGui::StyleColorsClassic();

  // Platform/Renderer backends による設定を行います。
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init();
}

void Destroy() {
  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

} // namespace GUI