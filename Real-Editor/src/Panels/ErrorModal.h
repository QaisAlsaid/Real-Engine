#ifndef ERROR_MODAL_H
#define ERROR_MODAL_H

#include "Real-Engine/Core/Events/ScriptEvents.h"
#include "Real-Engine/Scripting/Lua.h"
#include <Real-Engine/Real-Engine.h>

namespace Real
{
  class ErrorModal
  {
  public:
    static void show();
    static void onImGuiUpdate();
    static void setContext(ScriptErrorEvent& e);
    static void setContext(const ARef<ScriptError>& e);
  private:
    static bool s_is_shown;
    static ARef<ScriptError> s_context;
  };
}


#endif //ERROR_MODAL_H
