#include <pch.h>
#include "ErrorModal.h"
#include "Real-Engine/Real-Engine.h"
#include "Real-Engine/Scripting/Lua.h"

#include <imgui.h>


namespace Real
{
  bool ErrorModal::s_is_shown = false;
 
  ARef<ScriptError> ErrorModal::s_context = nullptr;

  void ErrorModal::show()
  {
    s_is_shown = true;
  }

  void ErrorModal::setContext(const ARef<ScriptError>& e)
  {
    s_context = e;
  }

  void ErrorModal::setContext(ScriptErrorEvent& e)
  {
    s_context = e.getScriptError();
  }

  void ErrorModal::onImGuiUpdate()
  {
    if(!s_is_shown || ( s_context->status == ScriptError::ErrorCode::ok && s_context->load_status == ScriptError::LoadErrorCode::ok )) return;
    ImGui::OpenPopup("ErrorModal0");
    if(ImGui::BeginPopup("ErrorModal0", ImGuiWindowFlags_MenuBar))
    {
      if(s_context->load_status != ScriptError::LoadErrorCode::ok)
      {
        ImGui::TextColored(ImVec4(230, 40, 40, 255), ("Error Loadind Script: " + s_context->load_error_message).c_str());
      }
      if(s_context->status != ScriptError::ErrorCode::ok)
      {
        ImGui::TextColored(ImVec4(230, 40, 40, 255), ("Error in Script: " + s_context->error_message).c_str());
      }
      if(ImGui::Button("OK"))
      {
        s_is_shown = false;
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }
  }
}
