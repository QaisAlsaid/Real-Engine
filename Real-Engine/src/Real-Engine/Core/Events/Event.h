#ifndef EVENT_H
#define EVENT_H

#include "pch.h"
#include "Real-Engine/Core/Core.h"

namespace Real
{
  enum class EventType
  {
   NONE = 0,
   WindowClosed, WindowResized, WindowFocused, WindowLostFocus, WindowMoved,
   KeyPressed, KeyReleased,
   MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled, 
   ScriptErrorEvent
  };

  enum EventCategory
  {
    CATNONE        = 0,
    CATAPP         = BITSHL(0),
    CATInput       = BITSHL(1),
    CATKeyboard    = BITSHL(2),
    CATMouse       = BITSHL(3),
    CATMouseButton = BITSHL(4)
  };


  class REAL_API Event
  {
    friend class EventDispatcher;
  public:
    virtual EventType          getEventType()     const = 0;
    virtual std::string        getName()          const = 0;
    virtual int                getCategoryNum()   const = 0;
    virtual std::string        toString ()        const {return getName();}
    inline  bool               isInCategory(EventCategory cat) 
    {
      return getCategoryNum() & cat;
    }
    inline bool                isHandled() const
    {
      return m_is_handled;
    }
    inline void                setHandled()
    {
      m_is_handled = true;
    }
  private:
    bool m_is_handled = false;
  };

  class REAL_API EventDispatcher
  {
  public:
    template<typename T>
    using EventFunc = std::function<bool(T&)>;
    EventDispatcher(Event& event) : m_event(event) {}

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
    Event& m_event;
  };
  
  inline std::ostream& operator<< (std::ostream& os, const Event& event)
  {
    return os << event.toString();
  }
}
#endif //EVENT_H
