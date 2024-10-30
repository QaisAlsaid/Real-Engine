#ifndef GUI_LAYER_H
#define GUI_LAYER_H

#include "Real-Engine/Core/Core.h"
#include "Real-Engine/Core/Window.h"
#include "Real-Engine/Core/Layers/Layer.h"

namespace Real
{
  class REAL_API GuiLayer : public Layer
  {
  public:
    GuiLayer(const std::string& name = DEFAULT_LAYER_NAME);
    virtual ~GuiLayer();
    virtual void onAttach()      override;
    virtual void onDetach()      override;
    virtual void onGuiUpdate()   override;
    virtual void onEvent(Event&) override;
    void         begin();
    void         end();
    
    inline void setBlocking(bool b) { m_blocking = b;    }
    inline bool isBlocking()        { return m_blocking; }
  private:
    float m_time = 0.0f;
    bool  m_blocking = false;
  private:
    void init();
  };
}
#endif //GUI_LAYER_H
