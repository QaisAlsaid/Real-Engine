#define IMGUI_DEFINE_MATH_OPERATORS
#include <filesystem>
#include <pch.h>
#include "ProjectSelectionLayer.h"
#include "EditorEvents/ProjectEvents.h"
#include "Real-Engine/Project/Project.h"
#include "RecentProjects.h"
#include "RecentProjectsSerializer.h"

#include "IconsFontAwesome6.h"

#include <imgui_internal.h>
#include <imgui.h>


namespace Real
{
bool InputTextWithPreviewValue(const char* label, char* buf, size_t buf_size, const char* preview_value, ImGuiInputTextFlags flags = 0)
{
    using namespace ImGui;
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(CalcItemWidth(), label_size.y + style.FramePadding.y * 2.0f));
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id, &frame_bb, ImGuiItemFlags_Inputable))
        return false;

    const bool hovered = ItemHoverable(frame_bb, id, g.LastItemData.InFlags);
    bool make_active = false;
    if (g.ActiveId != id)
    {
        if ((hovered && IsMouseClicked(0, id)) || g.NavActivateId == id)
        {
            make_active = true;
            g.NavActivateId = id;
            g.NavActivateFlags = ImGuiActivateFlags_PreferInput;
        }
    }

    if (g.ActiveId == id || make_active)
    {
        g.CurrentWindow->DC.CursorPos = frame_bb.Min;
        return InputText(label, buf, buf_size, flags);
    }

    // Draw frame
    const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    RenderNavHighlight(frame_bb, id);
    RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, style.FrameRounding);

    if (g.LogEnabled)
        LogSetNextTextDecoration("{", "}");
    RenderTextClipped(frame_bb.Min, frame_bb.Max, preview_value, NULL, NULL, ImVec2(0.5f, 0.5f));

    if (label_size.x > 0.0f)
        RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Inputable);
    return false;
}

  static bool ButtonImageText();
  ProjectSelectionLayer::ProjectSelectionLayer()
    : Layer("ProjectSelectionLayer")
  {
    activate();
    m_sub_id = EditorEventsManager::subscribe(BIND_EVENT_FUNCTION(ProjectSelectionLayer::onEditorEvent));
    m_temp_proj_config.name = Project::getConfig().name;
    m_temp_proj_config.asset_dir = "res/";
    m_temp_proj_config.proj_dir = "";
  }

  void ProjectSelectionLayer::onAttach()
  {
    m_logo = Texture2D::create("../res/textuers/placeholder_logo1.png");
    m_simple_icon = Texture2D::create("../res/textuers/placeholder_logo2.png");
    RecentProjectsSerializer::deserialize("../res/config/recent_projects.db");
    Project::setConfig(Project::Config());
  }

  void ProjectSelectionLayer::onUpdate(Timestep ts)
  {
    if(m_done)
    {
      ProjectSetEvent e;
      EditorEventsManager::onEvent(e);
      App::get()->getLayer("EditorLayer")->onAttach();
    }
  }

  void ProjectSelectionLayer::onGuiUpdate()
  {
    auto& io = ImGui::GetIO();
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::SetNextWindowPos({0, 0});
    bool* popen;
    auto window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse; 
    ImGui::Begin("Select a Project", popen, window_flags); 
   
    ImGui::Image((void*)(uintptr_t)m_logo->getRendererID(), { 187.5, 37.5 }, { 0, 1 }, { 1, 0 });
  
    ImGui::Separator();

    if(ImGui::Button("Create " ICON_FA_PLUS))
    {
      ImGui::OpenPopup("CreateProject");
    }

    ImGui::SameLine();

    if(ImGui::Button("Open " ICON_FA_FOLDER))
    {
      FileDialogs fd;
      std::string path = fd.OpenFile(".reproj");
      if(!path.empty())
      {
        if(Project::load(path)) 
        {
          m_done = true;
          RecentProjects::append(Project::getConfig().proj_dir);
          RecentProjectsSerializer::serialize("../res/config/recent_projects.db");
        }
        else {REAL_CORE_ERROR("Cant load Project");}
      }
    }
 
    ImGui::Separator();

    drawCreateProjectPopup();
    drawRecentProjects();
   
    ImGui::End();
  }


  void ProjectSelectionLayer::drawCreateProjectPopup()
  {
    if(ImGui::BeginPopupModal("CreateProject"))
    {
      static bool state[] = { 0, 0 };
      static std::string name;
      static std::filesystem::path project_dir, res_dir = "res/";

      char name_buffer[128];
      memset(name_buffer, 0, sizeof(name_buffer));
      strcpy(name_buffer, m_temp_proj_config.name.c_str());
      if(ImGui::InputText("Project Name", name_buffer, sizeof(name_buffer)))
      {
        name = std::string(name_buffer);
      }

      name.erase(std::remove_if(name.begin (), name.end (), [](char c)
      {
        return c == '/' || c == '\\';
      })
      ,name.end());

     
      ImGui::SameLine();

      if(ImGui::Checkbox("Create New Directory " ICON_FA_FOLDER_PLUS, &m_create_new_dir)) {}
 
      if(name.empty()) 
        ImGui::TextColored({ 0.7, 0.7, 0.1, 1.0 }, "Project Name is Empty!");

      /*
      char path_buffer[256 + 128];
      memset(path_buffer, 0, sizeof(path_buffer));
      project_dir = m_create_new_dir ? project_dir / name : project_dir;
      auto project_dir_str = project_dir.string();
      strcpy(path_buffer, project_dir_str.c_str());
      if(ImGui::InputTextWithHint("##ProjectPath", "Enter Project Path",
            path_buffer, sizeof(path_buffer)))
      {
        project_dir = path_buffer;
      }
      */
      char path_buffer[256 + 128];
      memset(path_buffer, 0, sizeof(path_buffer));
      auto str = project_dir.string();
      strcpy(path_buffer, str.c_str());
      if(InputTextWithPreviewValue("##ProjectPath", path_buffer, sizeof(path_buffer), m_create_new_dir ? (project_dir / name).string().c_str() : project_dir.string().c_str()))
      {
        project_dir = path_buffer;
      }
      ImGui::SameLine();
      if(ImGui::Button(ICON_FA_FOLDER_OPEN))
      {
        FileDialogs fd;
        auto path = fd.OpenDir();
        if(!path.empty())
        {
          project_dir = std::filesystem::absolute(path);
         // if(m_create_new_dir)
         //   project_dir /= name;
        }
      }

      //m_create_new_dir ? project_dir = project_dir.parent_path() : project_dir;

      if(state[1])
      {
        ImGui::TextColored({ 0.7, 0.7, 0.1, 1.0 }, "Path %s dont exist, it will be created", 
            m_temp_proj_config.proj_dir.string().c_str());
      }
      else if(state[0])
      {
        ImGui::TextColored({ 0.7, 0.7, 0.1, 1.0 }, 
            "Path %s isn't empty creating a new project will delete other projects there", m_temp_proj_config.proj_dir.string().c_str());
      }

      char res_buffer[128];
      memset(res_buffer, 0, sizeof(res_buffer));
      auto res_dir_str = res_dir.string();
      strcpy(res_buffer, res_dir_str.c_str());
      if(ImGui::InputTextWithHint("##ResDir", "Enter Asset Directory Relative to the Project Directory",
            res_buffer, sizeof(res_buffer)))
      {
        res_dir = res_buffer;
      } 

      //path dont exist 
      
      if(!std::filesystem::exists(project_dir))
      {
        state[1] = true;
      }

      //path already exist
      if(std::filesystem::exists(project_dir/name))
      {
        if(!std::filesystem::is_empty(project_dir/name))
          state[0] = true;
      }

      m_temp_proj_config.name = name;
      if(m_create_new_dir)
        m_temp_proj_config.proj_dir = project_dir / name;
      else
        m_temp_proj_config.proj_dir = project_dir;
      m_temp_proj_config.asset_dir = res_dir;

      if(ImGui::Button("Save") && !(m_temp_proj_config.proj_dir.empty()||m_temp_proj_config.name.empty()||m_temp_proj_config.asset_dir.empty()))
      {
        std::filesystem::create_directories(std::filesystem::absolute(m_temp_proj_config.proj_dir));
        std::filesystem::create_directories(std::filesystem::absolute(m_temp_proj_config.proj_dir
            / m_temp_proj_config.asset_dir));
        ImGui::CloseCurrentPopup();
        Project::setConfig(m_temp_proj_config);
        Project::save(m_temp_proj_config.proj_dir / "project.reproj");
        m_done = true;
        RecentProjects::append(m_temp_proj_config.proj_dir);
        RecentProjectsSerializer::serialize("../res/config/recent_projects.db");
      }
      ImGui::EndPopup();

    }
  }

  void ProjectSelectionLayer::drawRecentProjects()
  {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    const auto& projs = RecentProjects::get();
    for(uint32_t i = 0; i < projs.size(); ++i)
    {
      bool dir_exist = false, proj_exist = false, res_exist = false, exist = false, main_exist = false;
      const auto& proj = projs.at(i);
      if(std::filesystem::exists(proj.proj_dir) && !proj.proj_dir.empty())
        dir_exist = true;

      if(std::filesystem::exists(proj.proj_dir / "project.reproj"))
        proj_exist = true;

      if(std::filesystem::exists(proj.proj_dir / proj.asset_dir) && !proj.asset_dir.empty())
        res_exist = true;

      exist = res_exist && dir_exist && proj_exist;
      if(std::filesystem::exists(proj.proj_dir / proj.asset_dir / proj.main_scene) && !proj.main_scene.empty())
        main_exist = true;

      if(!exist)
        ImGui::BeginDisabled();
      if(ImGui::ImageButton(proj.proj_dir.string().c_str(), (void*)(uintptr_t)m_simple_icon->getRendererID(), { 75, 75 }, { 0, 1 }, { 1, 0 }))
      {
        if(Project::load(proj.proj_dir / "project.reproj")) 
        {
          m_done = true;
        }
        else {REAL_CORE_ERROR("Cant load Project");}
      }
      if(!exist)
        ImGui::EndDisabled();
      ImGui::SameLine();
      ImGui::Text("Project Name: %s", proj.name.c_str());
      ImGui::SameLine();
      ImGui::Text("Path: %s", proj.proj_dir.c_str());
      if(i > projs.size() -1)
        ImGui::Separator();
      
      if(!dir_exist)
        ImGui::TextColored(ImVec4(0.9, 0.1, 0.1, 1), "Can't Find Project Directory");
      if(!proj_exist)
        ImGui::TextColored(ImVec4(0.1, 0.1, 0.1, 1), "Can't Find Project File");
      if(!res_exist)
        ImGui::TextColored(ImVec4(0.9, 0.1, 0.1, 1), "Can't Find Project's Res Directory");
      if(!main_exist)
      {
        ImGui::TextColored(ImVec4(0.9, 0.9, 0.1, 1), "Can't Find Project's Main Scene you will be promted to choose one in the Editor");
      }
    }
    ImGui::PopStyleColor();
  }

  void ProjectSelectionLayer::onEvent(Event& e)
  {

  }

  void ProjectSelectionLayer::onDetach()
  {

  }

  void ProjectSelectionLayer::onEditorEvent(EditorEvent& e)
  {

  }
 }
