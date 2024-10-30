#include <Real-Engine/Core/EntryPoint.h>
#include <Real-Engine/Real-Engine.h>

#include "EditorLayer.h"
#include "Real-Engine/Project/Project.h"
#include "ProjectSelectionLayer.h"

namespace Real
{

  class EditorApp : public App
  {
  public:
    EditorApp() 
    {
      RenderCommands::init();
      Renderer2D::init("../res/shaders/Shaders2D/config.xml");
      
      EditorLayer* al = new EditorLayer();
      pushLayer(al);

      ProjectSelectionLayer* psl = new ProjectSelectionLayer();
      pushLayer(psl);
    }
    ~EditorApp()
    {

    }
  };

  App* createApp()
  {
    REAL_SET_LOGLEVEL(Log::LogLevel::Trace);
    Project::save("Temp.reproj");
    return new EditorApp();
  }
}
