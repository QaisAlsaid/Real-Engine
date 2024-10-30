#include "imgui.h"
#include "pch.h"
#include "Real-Engine/Core/Timestep.h"
#include "Real-Engine/Core/Events/AppEvents.h"
#include "Real-Engine/Core/App.h"
#include "Real-Engine/Core/Log.h"
#include "Real-Engine/Render/API/RendererAPI.h"
#include "Real-Engine/Scripting/Lua.h"


#ifdef REAL_EMSCRIPTEN
#include <emscripten.h>
static void callMain(void* fp)
{
  std::function<void()>* func = (std::function<void()>*)fp;
  (*func)();
}
#endif //REAL_EMSCRIPTEN

//FIXME:remove it
#include <GLFW/glfw3.h>

namespace Real
{
  App* App::s_instance = nullptr;
  App::App()
  {
    REAL_CORE_INFO("App Created");
    RendererAPI::setAPI(RendererAPI::API::OpenGl);
    REAL_CORE_ASSERT_MSG(!s_instance, "App Exist");
    s_instance = this;
    m_window = std::unique_ptr<Window>(Window::create());
    m_window->setEventCallbackFunction(BIND_EVENT_FUNCTION(App::onEvent));
    m_gui_layer = new GuiLayer("BaseGuiLayer");
    m_gui_layer->activate();
    pushOverlay(m_gui_layer);
    Lua::init(); 
  }

  App::~App()
  {
    for(auto it = m_layers.begin(); it != m_layers.end(); ++it)
    {
      (*it)->onDetach();
    }
    AssetManager::shutDown();
  }

  void App::onEvent(Event& event)
  {
    EventDispatcher dp(event);
    dp.dispatch<WindowClosedEvent>(BIND_EVENT_FUNCTION(App::onCloseCall));
    dp.dispatch<WindowResizeEvent>(BIND_EVENT_FUNCTION(App::onResizeCall));
    
    for(auto it = m_layers.end(); it != s_instance->m_layers.begin();)
    {
      --it;
      if((*it)->isActive())
      {
        if(event.isHandled())
          break;
        (*it)->onEvent(event);
      }
    }
  }

  void App::run()
  {
#ifndef REAL_EMSCRIPTEN
    while(m_running)
    {
#else 
      std::function<void()> MainLoop = [&] () {
#endif
      float time = glfwGetTime();
      float delta_time = time - m_last_time;
      m_last_time = time;
      Timestep ts(delta_time);
      for(auto it = m_layers.end(); it != m_layers.begin();)
      {
        --it;
        if((*it)->isActive())
        {
          (*it)->onUpdate(ts);
        }
      }

      m_gui_layer->begin();
      for(auto it = m_layers.end(); it != m_layers.begin();)
      {
        --it;
        if((*it)->isActive() && (*it)->isGuiActive())
        {
          (*it)->onGuiUpdate();
        }
      }
      m_gui_layer->end();


      m_window->onUpdate();
#ifndef REAL_EMSCRIPTEN
    }
#else 
    };
    emscripten_set_main_loop_arg(callMain, &MainLoop, 0, 1);
#endif //REAL_EMSCRIPTEN
  }
  bool App::onCloseCall(WindowClosedEvent& event)
  {
    m_running = false;
    return true;
  }

  bool App::onResizeCall(WindowResizeEvent& event)
  {
    return false;
  }
  
  void App::pushLayer(Layer* layer)
  {
    m_layers.pushLayer(layer);
    if(layer->isActive())
      layer->onAttach();
  }

  void App::pushOverlay(Layer* layer)
  {
    m_layers.pushOverlay(layer);
    if(layer->isActive())
      layer->onAttach();
  }

  void App::popLayer(Layer* layer)
  {
    m_layers.popLayer(layer);
    layer->onDetach();
  }

  void App::popOverlay(Layer* layer)
  {
    m_layers.popOverlay(layer);
    layer->onDetach();
  }

  Layer* App::getLayer(const std::string& name)
  {
    for(auto* layer : m_layers)
    {
      if(layer->getName() == name)
      {
        return layer;
      }
    }
    REAL_CORE_WARN("Layer: {0} Not found in stack", name);
    return nullptr;
  }
}
