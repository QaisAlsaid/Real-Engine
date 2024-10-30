#include "pch.h"
#include "Real-Engine/Core/Layers/GuiLayer.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "ImGuizmo.h"

#include "Real-Engine/Core/App.h"
//TODO: remove it
#include <GLFW/glfw3.h>
#include "IconsFontAwesome6.h"

namespace Real 
{
  GuiLayer::GuiLayer(const std::string& name) : Layer(name)
  {
    init();
  }

  GuiLayer::~GuiLayer()
  {

  }

  void GuiLayer::onAttach()
  {
    //TODO: paths shouldn't be this way fffffffff
    auto& io = ImGui::GetIO();
    io.FontDefault = io.Fonts->AddFontFromFileTTF("../res/fonts/Roboto/Roboto-Regular.ttf", 18);

    ImFontConfig config;
    config.MergeMode = true;
    config.GlyphMinAdvanceX = 13.0f; // Use if you want to make the icon monospaced
    static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    io.Fonts->AddFontFromFileTTF("../res/fonts/Font-Awesome/fa-solid-900.ttf", 18.0f, &config, icon_ranges);

    auto& colors = ImGui::GetStyle().Colors;

    colors[ImGuiCol_WindowBg] = {0.1f, 0.1f, 0.1f, 1.0f};
    colors[ImGuiCol_HeaderHovered] = {0.2f, 0.2f, 0.2f, 1.0f};
    colors[ImGuiCol_HeaderActive] = {0.15, 0.15, 0.15, 1.0f};

    colors[ImGuiCol_Button] = {0.2, 0.2, 0.2, 1.0f};
    colors[ImGuiCol_ButtonHovered] = {0.3, 0.3, 0.3, 1.0f};
    colors[ImGuiCol_ButtonActive] = {0.15, 0.15, 0.15, 1.0f};

    colors[ImGuiCol_FrameBg] = {0.2, 0.2, 0.2, 1.0};
    colors[ImGuiCol_FrameBgHovered] = {0.3, 0.3, 0.3, 1.0f};
    colors[ImGuiCol_FrameBgActive] = {0.17, 0.1, 0.2, 1.0f};

    colors[ImGuiCol_Tab] = {0.15, 0.16, 0.14, 1.0f};
    colors[ImGuiCol_TabHovered] = {0.3, 0.34, 0.345, 1.0f};
    colors[ImGuiCol_TabActive] = {0.23, 0.25, 0.189, 1.0f};
    colors[ImGuiCol_TabUnfocused] = {0.07, 0.2, 0.13, 1.0f};
    colors[ImGuiCol_TabUnfocusedActive] = {0.21, 0.23, 0.22, 1.0f};

    colors[ImGuiCol_TitleBg] = {0.15, 0.154, 0.145, 1.0f};
    colors[ImGuiCol_TitleBgActive] = {0.23, 0.26, 0.32, 1.0f};
    colors[ImGuiCol_TitleBgCollapsed] = {0.75, 0.32, 0.98, 1.0f};
  }

  void GuiLayer::onDetach()
  {

  }
  void GuiLayer::onGuiUpdate()
  {
  }
  
  void GuiLayer::begin()
  {
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
  }

  void GuiLayer::end()
  {
    App* app = App::get();
    auto& window = app->getWindow();
    ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(window.getWidth(), window.getHeight());
 
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
		}
  }
  
  void GuiLayer::init()
  {
    const App* app = App::get();
    auto* window = (GLFWwindow*)app->getWindow().getNativeWindow();

    IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#ifdef REAL_PLATFORM_WINDOWS
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

		
		ImGui::StyleColorsDark();
		
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init();
  }

  void GuiLayer::onEvent(Event& event)
  {
    if(m_blocking)
    {
    auto& io = ImGui::GetIO();
    auto block = (io.WantCaptureMouse && event.isInCategory(EventCategory::CATMouse)) ||
      (io.WantCaptureKeyboard && event.isInCategory(EventCategory::CATKeyboard));
    if(block)
      event.setHandled();
    }
  }
}
