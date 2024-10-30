#ifndef SCENE_HIERARCHY_PANEL_EVENTS_H
#define SCENE_HIERARCHY_PANEL_EVENTS_H

#include <Real-Engine/Real-Engine.h>
#include "EditorEvents.h"


namespace Real 
{
  class SCHPContextChangedEvent : public EditorEvent
  {
  public:
    //chat do we take ARef<Scene> or UUID to get it from AssetManager ?????????
    SCHPContextChangedEvent(const ARef<Scene>& n_context) : m_context(n_context) {}
    
    inline ARef<Scene> getContext() { return m_context; }


    int getCategory() const override { return EditorEventCategory::ECATSCENEHIERARCHY; }

    std::string toString() const override
    {
      std::stringstream ss;
      ss << "SCHPContextChangedEvent: "<< m_context;
      return ss.str();
    }
    
    static  EditorEventType  getStaticType()               { return EditorEventType::SCHPContextChanged; }
    EditorEventType          getEventType() const override { return getStaticType();                     }
    std::string              getName()      const override { return "SCHPContextChangedEvent";           }
  private:
    ARef<Scene> m_context;
  };

  class SCHPSelectionChangedEvent : public EditorEvent
  {
  public:
    SCHPSelectionChangedEvent(Entity n_entity) : m_entity(n_entity) {}

    inline Entity getEntity() { return m_entity; }


    int getCategory() const override { return EditorEventCategory::ECATSCENEHIERARCHY; }
    
    std::string toString() const override
    {
      std::stringstream ss;
      ss << "SCHPSelectionChangedEvent: " << (uint64_t)m_entity;
      return ss.str();
    }
    
    static  EditorEventType  getStaticType()               { return EditorEventType::SCHPSelectionChanged; }
    EditorEventType          getEventType() const override { return getStaticType();                       }
    std::string              getName()      const override { return "SCHPSelectionChangedEvent";           }

  private:
    Entity m_entity;
  };
}


#endif //SCENE_HIERARCHY_PANEL_EVENTS_H
