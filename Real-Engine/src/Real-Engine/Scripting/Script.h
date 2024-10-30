#ifndef KR_SCRIPT_H
#define KR_SCRIPT_H

#include "Real-Engine/Core/Timestep.h"
#include "Real-Engine/Scene/Entity.h"


namespace Real
{
  class Script
  {
  public:
    std::function<void(void)> onCreate;
    std::function<void(void)> onUpdate;
    std::function<void(void)> onDestroy;
    std::function<void(void)> setExport;
    Entity entity;
    Timestep getTimestep() { return timestep; }
  private:
    Timestep timestep;
    friend class Scene;
  };
}

#endif //KR_SCRIPT_H
