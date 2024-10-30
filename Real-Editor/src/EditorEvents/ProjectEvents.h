#ifndef PROJECT_EVENTS_H
#define PROJECT_EVENTS_H

#include "EditorEvents.h"


namespace Real
{
  class ProjectSetEvent : public EditorEvent 
  {
  public:
    EditorEventType getEventType()     const { return EditorEventType::ProjectSet;      }
    std::string     getName()          const { return "ProjectSetEvent";                }
    int             getCategory()      const { return EditorEventCategory::ECATProject; }
  
    static  EditorEventType  getStaticType() { return EditorEventType::ProjectSet;      }
  };
}

#endif //PROJECT_EVENTS_H
