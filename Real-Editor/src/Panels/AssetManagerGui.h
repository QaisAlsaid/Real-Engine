#ifndef ASSET_MANAGER_GUI_H
#define ASSET_MANAGER_GUI_H

#include <Real-Engine/Real-Engine.h>

namespace Real
{
  class AssetManagerModal
  {
  public:
    void show() { m_is_shown = true; }
    void onImGuiUpdate();
    void setContext(UUID ctx) { m_context = ctx; }
    bool isShown() { return m_is_shown; }
  private:
    UUID m_context = UUID::invalid;
    bool m_is_shown = false;
  };
}


#endif //ASSET_MANAGER_PANEL_H
