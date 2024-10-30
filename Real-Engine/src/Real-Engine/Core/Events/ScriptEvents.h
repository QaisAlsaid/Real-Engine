#ifndef SCRIPT_EVENTS_H
#define SCRIPT_EVENTS_H

#include "Real-Engine/Core/Core.h"
#include "Real-Engine/Core/Events/Event.h"


namespace Real
{
  struct ScriptError;
  class REAL_API ScriptErrorEvent : public Event
  {
  public:
    ScriptErrorEvent(const ARef<ScriptError>& err) : e(err) {}
    static EventType getStaticType() {return EventType::ScriptErrorEvent;}
    
    EventType   getEventType()     const override { return EventType::ScriptErrorEvent; }
    std::string getName()          const override { return "ScriptErrorEvent";          }
    int         getCategoryNum()   const override { return EventCategory::CATAPP;       }
    
    ARef<ScriptError> getScriptError() { return e; }
  private:
    ARef<ScriptError> e;
  };
}

#endif //SCRIPT_EVENTS_H
