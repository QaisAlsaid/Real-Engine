#ifndef PROJECT_SELECTION_LAYER_H
#define PROJECT_SELECTION_LAYER_H

#include <Real-Engine/Real-Engine.h>
#include "EditorEvents/EditorEvents.h"
#include "Real-Engine/Render/API/Texture.h"


namespace Real
{
  class ProjectSelectionLayer : public Layer
  {
  public:
    ProjectSelectionLayer();

    void onAttach() override;
    void onUpdate(Timestep ts) override;
    void onGuiUpdate() override;
    void onEvent(Event& e) override;
    void onDetach() override;
    void onEditorEvent(EditorEvent& e);
  private:
    void drawCreateProjectPopup();
    void drawRecentProjects();
  private:
    uint32_t m_sub_id;
    bool m_done = false, m_create_new_dir = true;
    ARef<Texture2D> m_logo, m_simple_icon;
    Project::Config m_temp_proj_config;
  };
}


#endif //PROJECT_SELECTION_LAYER_H
