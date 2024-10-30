#ifndef APP_H
#define APP_H

#include "Real-Engine/Core/Core.h"
#include "Real-Engine/Core/Window.h"
#include "Real-Engine/Core/Layers/Layer.h"
#include "Real-Engine/Core/Layers/LayerStack.h"
#include "Real-Engine/Core/Layers/GuiLayer.h"
#include "Real-Engine/Core/Events/Events.h"
#include "Real-Engine/Core/AssetManager.h"
#include "Export.h"


namespace Real
{
  class REAL_API App
  {
  public:
    App();
    App(const App&) = delete;
    virtual ~App();
    
    void run();
    void onEvent(Event&);
    
    //can just return pointer because the lifetime of App is the whole lifetime of the Application
    inline Window& getWindow() const
    {
      return *m_window.get();
    }

    inline static App* get()
    {
      REAL_CORE_ASSERT_MSG(s_instance, "No App Created");
      return s_instance;
    }
    //Debug
    void print()
    {
      std::cout << "s_instance* : " << std::hex << s_instance << std::endl;
    }

    //O(N)
    Layer* getLayer(const std::string& name);
    inline GuiLayer* getGuiLayer() const { return m_gui_layer; }

  private:
    float m_last_time = 0.0f;
    Scoped<Window> m_window;
    GuiLayer* m_gui_layer = nullptr;
    bool m_running = true;
    LayerStack m_layers;
    static App* s_instance;
  protected:
    void pushLayer(Layer* layer);
    void pushOverlay(Layer* layer);
    void popLayer(Layer* layer);
    void popOverlay(Layer* layer);
    bool onResizeCall(WindowResizeEvent& event);
    bool onCloseCall(WindowClosedEvent&);
  };

  REAL_API App* createApp();
}
#endif //APP_H
