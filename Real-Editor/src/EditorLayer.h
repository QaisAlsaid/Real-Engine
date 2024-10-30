#ifndef EDITOR_LAYER_H
#define EDITOR_LAYER_H


#include <Real-Engine/Real-Engine.h>
#include "EditorEvents/EditorEvents.h"
#include "EditorEvents/ProjectEvents.h"
#include "Panels/SceneHierarchy.h"
#include "Panels/Inspector.h"
#include "HelperWindows.h"
#include "Panels/ContentBrowser.h"
#include "Panels/ErrorModal.h"
#include "Panels/ProjectPanel.h"
#include "EditorCamera.h"
#include "SceneTab.h"


namespace Real
{
  class EditorLayer : public Real::Layer
  {
  public:
    enum class SceneState {Stop, Play, Invalid};
    enum class GizmoOp 
    {
      TranslateX       = (1u << 0),
      TranslateY       = (1u << 1),
      TranslateZ       = (1u << 2),
      RotateX          = (1u << 3),
      RotateY          = (1u << 4),
      RotateZ          = (1u << 5),
      RotateCamera     = (1u << 6),
      ScaleX           = (1u << 7),
      ScaleY           = (1u << 8),
      ScaleZ           = (1u << 9),
      Bounds           = (1u << 10),
      ScaleXU          = (1u << 11),
      ScaleYU          = (1u << 12),
      ScaleZU          = (1u << 13),

      Translate = TranslateX | TranslateY | TranslateZ,
      Rotate = RotateX | RotateY | RotateZ | RotateCamera,
      Scale = ScaleX | ScaleY | ScaleZ,
      ScaleU = ScaleXU | ScaleYU | ScaleZU,
      Universal = Translate | Rotate | ScaleU
    };
  public: 
    EditorLayer();
  
    void onAttach() override;

    void onDetach() override;

    void onUpdate(Real::Timestep ts) override;

    void onEvent(Real::Event& e) override;

    void onGuiUpdate() override;
  private:
    void changeScene(UUID id);
    void changeScene(const ARef<Scene>& s);
    void reloadScene();
    void onScenePlay();
    void onSceneStop();
    
    void serializeEditor(const char* path);
    void deSerializeEditor(const char* path);
    
    void setColorScheme();
    void initImGui();

    void updateMenuBar();
    void updatePanels();
    void updateGizmos();
    void exportVars();

    bool onMouseScrolledEvent(MouseScrolledEvent& e);
    bool onKeyPressedEvent(KeyPressedEvent& e);
    bool onKeyReleasedEvent(KeyReleasedEvent& e);
    void handelCMD(int key);

    void onEditorEvent(EditorEvent& e);
    bool onSceneChangedCall(SceneChangedEvent& e);
    bool onScenePlayCall(ScenePlayEvent& e);
    bool onSceneStopCall(SceneStopEvent& e);
    bool onProjectSetCall(ProjectSetEvent& e);

    bool showSaveScene(bool&);
    void showSaveNewScne(bool&);
  private:
    Timestep m_time_step;
    UUID m_scene_handle = UUID::invalid;
    SceneState m_scene_state = SceneState::Stop;
    bool m_show_imgui_demo = true;
    bool m_default_editor = false;
    bool m_scene_selection = false;
    bool m_viewport_focused = false;
    std::string m_default_font;
    uint8_t m_default_font_size = 18;
    int m_mouse_picked_entity_id = -1;
    uint32_t m_sub_id;
    EditorCamera m_camera;

    std::string m_imgui_ini_path;
    Vec2 m_viewport_size = {1280.0f, 720.0f};
    Vec2 m_min_vp_bounds, m_max_vp_bounds;
    ARef<Scene> m_scene;
    ARef<Scene> m_editor_scene;
    GizmoOp m_op = GizmoOp::Bounds;
    
    ARef<Real::FrameBuffer> m_frame_buff;
    
    ProjectPanel m_proj_panel;
    ContentBrowser m_content_browser;
    SceneHierarchy m_scene_hierarchy_panel;
    Inspector m_inspector_panel;

    std::vector<SceneTab> m_opend_scenes;
    std::unordered_map<std::string, Scoped<HelperWindow>> m_helper_windows;
    std::unordered_map<std::string, Vec4> m_colors;
  private:
    friend class EditorSerializer;
    friend class MenuBar;
  };
}

#endif //EDITOR_LAYER_H
