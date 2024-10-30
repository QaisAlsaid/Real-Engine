#include "ProjectPanel.h"
#include "Real-Engine/Core/Utils/FileDialogs.h"
#include "Real-Engine/Project/Project.h"
#include "imgui.h"
#include <filesystem>

namespace Real
{

  static Project::Config temp;
  static bool done = false;
  static bool exist = false;

bool once = true;
  bool ProjectPanel::onImGuiUpdate()
  {
    if(once)
    {
      temp.name = Project::getConfig().name;
      temp.asset_dir = "res/";
    }
    once = false;
    ImGui::OpenPopup("ProjectPanel");
    auto window_flags = ImGuiWindowFlags_NoCollapse;
    auto popup_flags = ImGuiPopupFlags_None;
    if(ImGui::BeginPopupModal("ProjectPanel"), popup_flags, window_flags)
    {
      char buffer[128];
      memset(buffer, 0, sizeof(buffer));
      strcpy(buffer, temp.name.c_str());
      if(exist)
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
      if(ImGui::InputText("##ProjectName", buffer, sizeof(buffer)))
      {
        temp.name = std::string(buffer);
      }
      if(exist)
        ImGui::PopStyleColor();


      char _buffer[256 + 128];
      memset(_buffer, 0, sizeof(_buffer));
      strcpy(_buffer, ( temp.proj_dir / temp.name ).string().c_str());
      if(ImGui::InputText("##ProjectPath", _buffer, sizeof(_buffer)))
      {
        temp.proj_dir = std::string(_buffer);
      }
      ImGui::SameLine();
      if(ImGui::Button("Open in File Manager"))
      {
        FileDialogs fd;
        temp.proj_dir = std::filesystem::absolute(fd.OpenDir());
      }

      char __buffer[128];
      memset(__buffer, 0, sizeof(__buffer));
      strcpy(__buffer, temp.asset_dir.string().c_str());
      if(ImGui::InputText("##ResPath", __buffer, sizeof(__buffer)))
      {
        temp.asset_dir = std::string(__buffer);
      }

      if(std::filesystem::is_directory(temp.proj_dir/temp.name))
      {
        exist = true;
      }
      else exist = false;

      if(ImGui::Button("Save"))
      {
        if(!exist)
        {
          std::filesystem::create_directories(temp.proj_dir / temp.name);
          std::filesystem::create_directories(temp.proj_dir / temp.name / temp.asset_dir);
          done = true;
        }
        /*else 
        {
          bool sure = false;
          ImGui::OpenPopup("BraveSave");
          if(ImGui::BeginPopupModal("BraveSave"))
          {
            ImGui::TextColored(ImVec4(255, 0, 0, 255), "Are You Sure You Want To Save It will overwrite old project %s", temp.name.c_str());
            if(ImGui::Button("Yes"))
            {
              sure = true;
              ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if(ImGui::Button("No"))
            {
              sure = false;
              ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
          }
          if(sure) 
          {
            std::filesystem::create_directories(temp.proj_dir / temp.name);
            done = true;
          }
        }*/
      }
      if(done) 
      {
        ImGui::CloseCurrentPopup();
        Project::setConfig(temp);
      }
      ImGui::EndPopup();
    }
    return done;
  }
}
