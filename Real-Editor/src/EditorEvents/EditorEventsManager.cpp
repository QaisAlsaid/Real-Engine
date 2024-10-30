#include "EditorEventsManager.h"

namespace Real
{
  uint32_t EditorEventsManager::s_sub_id = 0;
  std::map<uint32_t, std::function<void(EditorEvent&)>> EditorEventsManager::m_subscribers; 
  void EditorEventsManager::onEvent(EditorEvent& e)
  {
    for(auto subscriber : m_subscribers)
    {
      subscriber.second(e);
    }
  }

  uint32_t EditorEventsManager::subscribe(std::function<void (EditorEvent &)> func)
  {
    m_subscribers[s_sub_id] = (func);
    return s_sub_id++;
  }

  void EditorEventsManager::unsubscribe(uint32_t sub_id)
  {
    auto iter = m_subscribers.find(sub_id); 
    if(iter != m_subscribers.end())
      m_subscribers.erase(iter);
    else REAL_CORE_WARN("sub_id: {0} isn't subscribed to EditorEvents");
  }
}
