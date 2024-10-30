#ifndef SCENE_TAB_H
#define SCENE_TAB_H

#include <Real-Engine/Real-Engine.h>


namespace Real
{
  class SceneTab 
  {
  public:
    SceneTab(UUID id, bool opend = true, bool save = false)
      :m_id(id), is_opend(opend), need_save(save) {};
    bool is_opend = true;
    bool need_save = false;
  public:
    UUID getID() { return m_id; }
    void onUpdate() {};
    bool operator==(const SceneTab& other) 
    {
      return this->m_id == other.m_id;
    }
    bool operator!=(const SceneTab& other) 
    {
      return this->m_id != other.m_id;
    }

  private:
    UUID m_id;
  };
}


#endif //SCENE_TAB_H
