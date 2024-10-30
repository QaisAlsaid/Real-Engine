#ifndef EDITOR_EVENTS_H
#define EDITOR_EVENTS_H

#include <Real-Engine/Real-Engine.h>
#include "EditorEventsManager.h"

namespace Real
{
  enum class EditorEventType
  {
    SCHPContextChanged, SCHPSelectionChanged, SceneSet,InspectorContextChanged,
    SceneChanged, ScenePlay, SceneStop, MousePickedChanged, 
    ProjectSet, ProjectChanged
  };

  enum EditorEventCategory
  {
    ECATNONE           = 0,
    ECATSCENE          = 1,
    ECATECS            = 2,
    ECATGIZMO          = 3,
    ECATSCENEHIERARCHY = 4,
    ECATINSPECTOR      = 5,
    ECATMOUSEPICKED    = 6,
    ECATProject        = 7
  }; 
  
  class REAL_API EditorEvent
  {
  public:
    virtual EditorEventType getEventType()     const = 0;
    virtual std::string     getName()          const = 0;
    virtual int             getCategory()      const = 0;
    virtual std::string     toString ()        const { return getName(); }
    
    inline bool isInCategory(EditorEventCategory cat) 
    {
      return getCategory() == cat;
    }
    inline bool isHandled() const
    {
      return m_is_handled;
    }
    inline void setHandled()
    {
      m_is_handled = true;
    }
  private:
    bool m_is_handled = false;
  
    friend class EditorEventDispatcher;
  };
  
  class REAL_API EditorEventDispatcher
  {
  public:
    template<typename T>
    using EventFunc = std::function<bool(T&)>;
    EditorEventDispatcher(EditorEvent& event) : m_event(event) {}

    template<typename T>      
    bool dispatch(EventFunc<T> func)
    {
      constexpr bool hasgetEvent = requires(const T& t)
      {
        t.getEventType();
      };
      if constexpr(hasgetEvent)
      {
        if(m_event.getEventType() == T::getStaticType())
        {
         m_event.m_is_handled = func(*(T*)&m_event);
         return true;
        }
        return false;
      }
      else
      {
        REAL_CORE_CRITICAL("Trying to access member function getEventType() which is not in class");
        exit(1);
      }
    }
  private:
    EditorEvent& m_event;
  };
  inline std::ostream& operator<< (std::ostream& os, const EditorEvent& event)
  {
    return os << event.toString();
  }
}


#endif //EDITOR_EVENTS_H
