#ifndef EDITOR_EVENTS_MANAGER_H
#define EDITOR_EVENTS_MANAGER_H

#include <functional>

namespace Real
{
  class EditorEvent;
  class REAL_API EditorEventsManager 
  {
  public:
    static uint32_t subscribe(std::function<void(EditorEvent&)> func); 
    static void onEvent(EditorEvent&);
    static void unsubscribe(uint32_t sub_id);

  private:
    static std::map<uint32_t, std::function<void(EditorEvent&)>> m_subscribers;
    static uint32_t s_sub_id;
  };
}



#endif //EDITOR_EVENTS_MANAGER_H
