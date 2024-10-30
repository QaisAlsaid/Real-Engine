//WTF is this

#include "HelperWindows.h"
#include <imgui.h>

namespace Real
{ 
  void StatsWindow::onImGuiUpdate()
  {
    if(is_active)
    {
      auto window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
      ImGui::SetNextWindowBgAlpha(0.35f);
      ImGui::Begin("Stats", &is_active, window_flags);
      ImGui::Text("MainLoop: Timestep: %f", stats.mainloop_time_step.getTime());
      ImGui::Separator();
      ImGui::Text("Renderer2D: Draw Calls: %d", stats.renderer2d_draw_calls_count);
      ImGui::Text("Renderer2D: Quad Count: %d", stats.renderer2d_quad_count);
      ImGui::Text("Renderer2D: Vertex Count: %d", stats.renderer2d_vertex_count);
      ImGui::Text("Renderer2D: Index Count: %d", stats.renderer2d_index_count);
      ImGui::End();
    }
  }
}
