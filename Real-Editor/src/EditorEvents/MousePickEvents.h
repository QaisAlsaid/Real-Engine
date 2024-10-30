#ifndef MOUSE_PICK_EVENTS_H
#define MOUSE_PICK_EVENTS_H

#include "EditorEvents.h"
#include "Real-Engine/Scene/Entity.h"

namespace Real
{
  class MousePickedChangedEvent : public EditorEvent 
  {
  public:
    MousePickedChangedEvent(Entity entity) : entity(entity) {}

    inline Entity getEntity() { return entity; }


    int getCategory() const override { return EditorEventCategory::ECATMOUSEPICKED; }

    std::string toString() const override
    {
      std::stringstream ss;
      ss << "MousePickedChangedEvent: "<< (uint64_t)entity;
      return ss.str();
    }
    
    static  EditorEventType  getStaticType()               { return EditorEventType::MousePickedChanged; }
    EditorEventType          getEventType() const override { return getStaticType();                     }
    std::string              getName()      const override { return "MousePickedChangedEvent";           }

  private:
    Entity entity;
  };
}


#endif //MOUSE_PICK_EVENTS_H
