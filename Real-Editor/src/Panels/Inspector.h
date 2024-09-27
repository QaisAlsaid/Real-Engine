#ifndef KR_INSPECTOR_H
#define KR_INSPECTOR_H

#include "Real-Engine/Real-Engine.h"


namespace Real
{
  class Inspector
  {
  public:
    Inspector() = default;
    
    //set to null to unselect;
    void setCurrentSelected(Entity entity) { m_current = entity; }
    Entity getCurrentSelected()            { return m_current;   }
    void onGuiUpdate();
  private:
    void drawExportedVars(UUID uuid);
    Entity m_current;
  };
}


#endif //KR_INSPECTOR_H
