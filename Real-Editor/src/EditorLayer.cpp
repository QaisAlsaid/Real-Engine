#include <pch.h>
#include "EditorLayer.h"
#include "EditorEvents/EditorEventsManager.h"
#include "EditorEvents/SceneEvents.h"
#include "EditorSerializer.h"
#include <Real-Engine/Real-Engine.h>
#include <imgui.h>
#include <ImGuizmo.h>
#include <vector>

#include "IconsFontAwesome6.h"
#include "Real-Engine/Core/Math/math.h"

namespace Real
{
  EditorLayer::EditorLayer()
    : Layer("EditorLayer"), m_content_browser("")
  {
    REAL_START_INSTRUMENTOR();

    m_editor_scene = createARef<Scene>();
    m_scene = m_editor_scene;
    m_scene_state = SceneState::Invalid;
    SceneSetEvent e(m_scene);
    EditorEventsManager::onEvent(e);
    m_helper_windows["Stats"] = createScoped<StatsWindow>();


    m_default_font_size = 18;
    m_default_font = "../res/fonts/Roboto/Roboto-Regular.ttf"; // TODO: Editor path or something
    m_imgui_ini_path = ".";

    setColorScheme();
    deSerializeEditor("../res/config/test.xml");
    if(m_default_editor)
      setColorScheme();
    initImGui();

    FrameBuffer::Specs s;
    s.attachment_specs = {
      {FrameBuffer::TextureFormat::RGBA, FrameBuffer::TextureInternalFormat::RGBA8, "render_buffer", 0},
      {FrameBuffer::TextureFormat::RedInt, FrameBuffer::TextureInternalFormat::Int32, "id_buffer", 1},
      {FrameBuffer::TextureFormat::DepthStencil, FrameBuffer::TextureInternalFormat::Depth24Stencil8, "depth_buffer", 2}
    };
    s.width = 1280;
    s.height = 720;
    s.is_swap_chain_target = true;
    m_frame_buff = FrameBuffer::create(s);
    REAL_CORE_SET_LOGLEVEL(Log::LogLevel::Warn);

    App::get()->getWindow().setVsync(true);

    m_sub_id = EditorEventsManager::subscribe(BIND_EVENT_FUNCTION(EditorLayer::onEditorEvent));
  }


static float* speed = new float;
  void EditorLayer::onAttach()
  {
    if(!active) return;

    m_editor_scene = createARef<Scene>();
    m_scene = m_editor_scene;

    SceneSetEvent event(m_scene);
    EditorEventsManager::onEvent(event);

    if(Project::getConfig().main_scene.empty())
      m_scene_selection = true;
    else
    {
      AssetManager::Asset::Meta meta;
      meta.path = Project::getConfig().main_scene;
      meta.type = AssetManager::Asset::Type::Scene;
      auto uuid = AssetManager::loadOrGet(meta);
      if(uuid == UUID::invalid)
      {
        REAL_CORE_ERROR("Invalid Main Scene");
        std::cin.get();
        m_scene_selection = true;//TODO: due to error
      }
      else
      {
        m_scene_state = SceneState::Stop;
        changeScene(uuid);
        auto s = m_opend_scenes.at(0);
      }
    }
  }


  static bool first_time = true;

  void EditorLayer::onUpdate(Timestep ts)
  {
    auto it = std::unique(m_opend_scenes.begin(), m_opend_scenes.end());
    m_opend_scenes.erase(it, m_opend_scenes.end());
    App::get()->getGuiLayer()->setBlocking(!m_viewport_focused);
    if(m_scene_state != SceneState::Invalid)
    {
      changeScene(m_scene_handle);
      m_scene->setEditorCamera(m_camera.getView(), m_camera.getProjection());
    }
    if(m_frame_buff->getSpecs().width != m_viewport_size.x ||
       m_frame_buff->getSpecs().height != m_viewport_size.y)
        m_frame_buff->reSize(m_viewport_size.x, m_viewport_size.y);
    m_camera.onUpdate(ts);
    if(Input::isKeyPressed(Keyboard::LeftControl) || Input::isKeyPressed(Keyboard::RightControl))
      handelCMD((int)Keyboard::LeftControl);
    m_time_step = ts;
    Renderer2D::resetStats();
    m_frame_buff->bind();
    Renderer2D::clear(Vec4(0.25f, 0.25f, 0.25f, 1.0f));
    m_frame_buff->clearColorAttachment(1, -1);
    switch(m_scene_state)
    {
      case SceneState::Play:
      {
        m_scene->onUpdate(ts);
        break;
      }
      case SceneState::Stop:
      {
        m_scene->onEditorUpdate(ts);
        break;
      }
      case SceneState::Invalid:
      {
        break;
      }
    }
      auto abs_inv_mouse = ImGui::GetMousePos();
      Vec2 mouse(abs_inv_mouse.x, abs_inv_mouse.y);
      mouse -= m_min_vp_bounds;

      Vec2 vp_size = m_max_vp_bounds - m_min_vp_bounds;
      mouse.y = vp_size.y - mouse.y;


      if(mouse.x > 0 && mouse.y > 0 && mouse.x < vp_size.x && mouse.y < vp_size.y && m_scene_state != SceneState::Invalid &&
          !App::get()->getGuiLayer()->isBlocking())
      {
        if(ImGui::IsMouseDoubleClicked(0))
        {
          m_scene_hierarchy_panel.clearSelection();
          m_op = GizmoOp::Bounds;
        }
        if(ImGui::IsMouseClicked(0) && m_op == GizmoOp::Bounds)
        {
          m_mouse_picked_entity_id = m_frame_buff->readPixelI(1, mouse.x, mouse.y);
          if(m_mouse_picked_entity_id > -1)
          {
            Entity e(m_mouse_picked_entity_id, m_scene.get());
            MousePickedChangedEvent event(e);
            EditorEventsManager::onEvent(event);
          }
        }
      }
      m_frame_buff->unbind();
    }

  void EditorLayer::onGuiUpdate()
  {
    static bool* p_open = new bool;
    *p_open = true;
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;

    if (opt_fullscreen)
    {
      const ImGuiViewport* viewport = ImGui::GetMainViewport();

      ImGui::SetNextWindowPos(viewport->WorkPos);
      ImGui::SetNextWindowSize(viewport->WorkSize);
      ImGui::SetNextWindowViewport(viewport->ID);

      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

      window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
      window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    else
    {
      dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
      window_flags |= ImGuiWindowFlags_NoBackground;

    if (!opt_padding)
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 min_win_size = style.WindowMinSize;
    style.WindowMinSize = ImVec2(230, 100);//TODO: make it changeable from Editor Settings
    ImGui::Begin("DockSpace", p_open, window_flags);

    if (!opt_padding)
      ImGui::PopStyleVar();

    if (opt_fullscreen)
      ImGui::PopStyleVar(2);

    ImGuiID dockspace_id = ImGui::GetID("DockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);


    ImGui::End();
    style.WindowMinSize = min_win_size;

    const Stats stats = {Renderer2D::getStats(), m_time_step};
    ((StatsWindow*)m_helper_windows["Stats"].get())->stats = stats;

    updateMenuBar();
    updatePanels();

    ImGui::Begin("__DEBUG__");
    const char * label = m_scene_state == SceneState::Stop ? "Play" : "Stop";
    if(ImGui::Button(label))
    {
      switch(m_scene_state)
      {
        case SceneState::Play:
          onSceneStop();
          break;
        case SceneState::Stop:
          onScenePlay();
          break;
        case SceneState::Invalid:
          break;
      }
    }
      ImGui::DragFloat("Cam run speed", speed);

//Editor Camera
    ImGui::Separator();
    ImGui::Text("Editor Camera");
    ImGui::DragFloat3("Position", glm::value_ptr(m_camera.position));
    auto yaw_deg = glm::degrees(m_camera.yaw), pitch_deg = glm::degrees(m_camera.pitch);
    ImGui::DragFloat("Yaw", &yaw_deg);
    m_camera.yaw = glm::radians(yaw_deg);
    ImGui::DragFloat("Pitch", &pitch_deg);
    m_camera.pitch = glm::radians(pitch_deg);
    auto fov_deg = glm::degrees(m_camera.fov);
    ImGui::DragFloat("Fov", &fov_deg, 0.5f, glm::degrees(m_camera.min_fov), glm::degrees(m_camera.max_fov));
    m_camera.fov = glm::radians(fov_deg);

    auto max_fov_deg = glm::degrees(m_camera.max_fov);
    ImGui::DragFloat("Max Fov", &max_fov_deg);
    m_camera.max_fov = glm::radians(max_fov_deg);

    auto min_fov_deg = glm::degrees(m_camera.min_fov);
    ImGui::DragFloat("Min Fov", &min_fov_deg);
    m_camera.min_fov = glm::radians(min_fov_deg);

    ImGui::DragFloat("Far", &m_camera.far);
    ImGui::DragFloat("Near", &m_camera.near);
    ImGui::DragFloat("Sensitivaty", &m_camera.mouse_sensitivaty);
    ImGui::Checkbox("F.A.R", &m_camera.fixed_aspect_ratio);
    ImGui::End();


    auto& io = ImGui::GetIO();
    if(io.WantSaveIniSettings)
      ImGui::SaveIniSettingsToDisk(m_imgui_ini_path.c_str());

  }

  void EditorLayer::changeScene(UUID id)
  {
    if(m_scene_handle == id)
    {
      if(m_scene_handle != UUID::invalid && m_scene_state != SceneState::Play)
      {
        auto scene = AssetManager::get<AssetManager::SceneAsset>(m_scene_handle)->scene;
        changeScene(scene);
        return;
      }
    }
    m_opend_scenes.emplace_back(id);
    m_scene_handle = id;
    if(m_scene_handle != UUID::invalid)
    {
      if(m_scene_state != SceneState::Play)
      {
        m_scene_handle = id;
        auto scene = AssetManager::get<AssetManager::SceneAsset>(m_scene_handle)->scene;
        SceneChangedEvent event(m_editor_scene, scene);
        EditorEventsManager::onEvent(event);
        changeScene(scene);
      }
    }
    else REAL_CORE_ERROR("UUID::invalid Can't be used as Scene Handle");
  }

  void EditorLayer::changeScene(const ARef<Scene>& s)
  {
    if(m_scene_state != SceneState::Play)
    {
      m_scene = s;
      m_editor_scene = m_scene;
    }
  }

  void EditorLayer::reloadScene()
  {
    if(m_scene_handle != UUID::invalid)
    {
      if(m_scene_state != SceneState::Play)
      {
        auto scene = AssetManager::get<AssetManager::SceneAsset>(m_scene_handle)->scene;
        SceneChangedEvent event(m_editor_scene, scene);
        EditorEventsManager::onEvent(event);
      }
    }
    else REAL_CORE_ERROR("UUID::invalid Can't be used as Scene Handle");
  }

  void EditorLayer::onScenePlay()
  {
    m_scene_state = SceneState::Play;
    m_editor_scene = Scene::copy(m_scene);
    ScenePlayEvent event(m_editor_scene, m_scene);
    EditorEventsManager::onEvent(event);
    //TODO: make it call the setExport on asset reloading when file watcher is present
    Export::getExportedVariables().clear();
    //so scripts have the acsess to the same registry // dont know if its good way
    AssetManager::get<AssetManager::SceneAsset>(m_scene_handle)->scene = m_scene;
    m_scene->onViewportResize(m_viewport_size.x, m_viewport_size.y);
    m_scene->onStart();
  }

  void EditorLayer::onSceneStop()
  {
    m_scene_state = SceneState::Stop;
    m_scene->onEnd();
    m_scene = m_editor_scene;
    AssetManager::get<AssetManager::SceneAsset>(m_scene_handle)->scene = m_editor_scene;
    SceneStopEvent event(m_editor_scene, m_scene);
    EditorEventsManager::onEvent(event);
  }

  void EditorLayer::onDetach()
  {
    auto& io = ImGui::GetIO();
    if(io.WantSaveIniSettings)
      ImGui::SaveIniSettingsToDisk(m_imgui_ini_path.c_str());

    serializeEditor("../res/config/test.xml");
    AssetManager::saveConfig(Project::getConfig().proj_dir / Project::getConfig().asset_dir);
  }

  static bool done = false;
  void EditorLayer::updatePanels()
  {

    //TODO: move into class
    {
      if(m_scene_selection)
      {
        ImGui::OpenPopup("SceneSelection");
        if(ImGui::BeginPopupModal("SceneSelection"))
        {
          ImGui::Text("No main Scene in Current Project");
          ImGui::Text("make a new scene an set it as main scene");
          if(ImGui::Button("New"))
          {
            m_scene_hierarchy_panel.clearSelection();
            m_scene->clear();
            m_editor_scene->clear();
            m_editor_scene = createARef<Scene>();
            m_scene = m_editor_scene;
            m_scene_hierarchy_panel.setContext(m_scene);
            m_inspector_panel.setCurrentSelected({});
            ImGui::OpenPopup("SceneName");
            //if(ImGui::BeginPopup("SceneName"))
            {
              //char buffer[128];
              //memset(buffer, 0, sizeof(buffer));
              //strcpy(buffer, m_scene->getName().c_str());
              //if(ImGui::InputText("Res Path", buffer, sizeof(buffer)))
              //{
              //  m_scene->setName(std::string(buffer));
              //}
              SceneSerializer ss(m_scene);
              FileDialogs fd;
              //TODO: no saves in other dirs only in res
              auto path = fd.SaveFile(".Real");
              ss.serializeText(path.c_str());
              auto conf = Project::getConfig();
              conf.main_scene = std::filesystem::relative(path, conf.proj_dir / conf.asset_dir);
              Project::setConfig(conf);
              Project::save(conf.proj_dir / "project.reproj");
              m_scene_state = SceneState::Stop;
              AssetManager::Asset::Meta meta;
              meta.path = conf.main_scene;
              meta.type = AssetManager::Asset::Type::Scene;
              auto id = AssetManager::loadOrGet(meta);
              changeScene(id);
              ImGui::CloseCurrentPopup();
              m_scene_selection = false;
            }
          }

          ImGui::Text("or select existing scene");
          if(ImGui::Button("Open"))
          {
            FileDialogs fd;
            auto path = fd.OpenFile(".Real");
            auto conf = Project::getConfig();
              conf.main_scene = std::filesystem::relative(path, conf.proj_dir / conf.asset_dir);
              Project::setConfig(conf);
              Project::save(conf.proj_dir / "project.reproj");
              m_scene_state = SceneState::Stop;
              AssetManager::Asset::Meta meta;
              meta.path = conf.main_scene;
              meta.type = AssetManager::Asset::Type::Scene;
              auto id = AssetManager::loadOrGet(meta);
              changeScene(id);
              ImGui::CloseCurrentPopup();
              m_scene_selection = false;
          }

          ImGui::EndPopup();
        }
      }
    }
    auto need_close = m_content_browser.onImGuiUpdate();

    m_helper_windows["Stats"]->onImGuiUpdate();

    m_scene_hierarchy_panel.onGuiUpdate();

    m_inspector_panel.onGuiUpdate();


    ErrorModal::onImGuiUpdate();
    if(need_close) ImGui::EndPopup();

    ImGui::ShowDemoWindow(&m_show_imgui_demo);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoDecoration);

    ImGui::BeginTabBar("##scenetabbar");
    for(auto s : m_opend_scenes)
    {
      REAL_CORE_ERROR("sdfd {0}", s.getID());
      auto scene = AssetManager::get<AssetManager::SceneAsset>(s.getID());
      auto flags = ImGuiTabItemFlags_NoCloseWithMiddleMouseButton;
      if(ImGui::BeginTabItem(scene->scene->getName().c_str(), &s.is_opend, flags))
      {
       changeScene(s.getID());
        ImGui::EndTabItem();
      }
    }
    ImGui::EndTabBar();
    m_viewport_focused = ImGui::IsWindowFocused();
    const auto vp_offset = ImGui::GetWindowPos();
    auto vp_min_reg = ImGui::GetWindowContentRegionMin();
    auto vp_max_reg = ImGui::GetWindowContentRegionMax();

    m_min_vp_bounds = {vp_min_reg.x + vp_offset.x, vp_min_reg.y + vp_offset.y};
    m_max_vp_bounds = {vp_max_reg.x + vp_offset.x, vp_max_reg.y + vp_offset.y};



    const ImVec2 panel_size = ImGui::GetContentRegionAvail();
    Vec2 r_panel_size = {panel_size.x, panel_size.y};

    ImGui::Image((void*)(uintptr_t)m_frame_buff->getColorAttachmentId("render_buffer"), panel_size, ImVec2(0, 1), ImVec2(1, 0));

    if(m_viewport_size != r_panel_size)
    {
      //m_frame_buff->reSize(r_panel_size.x, r_panel_size.y);
      m_viewport_size = r_panel_size;
      m_scene->onViewportResize(m_viewport_size.x, m_viewport_size.y);
      m_camera.onResize(m_viewport_size.x, m_viewport_size.y);
    }

    if(ImGui::BeginDragDropTarget())
    {
      if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_ASSET_HANDEL"))
      {
        UUID asset_handle = *(UUID*)payload->Data;
        REAL_CORE_WARN("Accepting drag and drop id: {0}", asset_handle);
        m_opend_scenes.emplace_back(asset_handle);
        //changeScene(asset_handle);
      }
      ImGui::EndDragDropTarget();
    }
    const ImVec2 win_pos = ImGui::GetWindowPos();

    updateGizmos();
    ImGui::PopStyleVar();
    ImGui::End();

  }

  void EditorLayer::updateMenuBar()
  {
    auto& conf = Project::getConfig();
    static bool new_scene = false;
    static bool show_save_scene = false;

    if(ImGui::BeginMainMenuBar())
    {
      if(ImGui::BeginMenu("Scene"))
      {
        if(ImGui::MenuItem("New"))
        {
          if(m_scene_state != SceneState::Play)
          {
            show_save_scene = true;
            if(new_scene)
            {
              //m_scene_hierarchy_panel.clearSelection();
              //m_scene->clear();
              //m_editor_scene->clear();
              //m_editor_scene = createARef<Scene>();
              //m_scene = m_editor_scene;
              //m_scene_hierarchy_panel.setContext(m_scene);
              //m_inspector_panel.setCurrentSelected({});

            }
          }
        }

        ImGui::Separator();
        if(ImGui::MenuItem("Open", "Ctrl+O"))
        {
          //TODO : redirect to asset manager after open file
          const auto& path = FileDialogs::OpenFile("Real", "Real-Engine Scene (.Real)");
          REAL_TRACE("path: {0}", path);
          if(!path.empty() && m_scene_state != SceneState::Play)
          {
            auto t_scene = createARef<Scene>();
            SceneSerializer ss(t_scene);
            if(!ss.deSerializeText(path.c_str()))
            {
              REAL_CORE_ERROR("Error in deSerializeText(): Scene: {0}", path);
            }
            else
            {
              changeScene(t_scene);
              SceneChangedEvent event(m_editor_scene, t_scene);
              EditorEventsManager::onEvent(event);
            }
          }
          else REAL_TRACE("Cancel");
        }

        ImGui::Separator();

        if(ImGui::MenuItem("Save As.."))
        {
          const auto& path = FileDialogs::SaveFile("Real", "Real-Engine Scene (.Real)");
          REAL_TRACE("path: {0}", path);
          if(!path.empty())
          {
            SceneSerializer ss(m_scene);
            ss.serializeText(path.c_str());
          }
        }

        ImGui::EndMenu();
      }
      if(ImGui::BeginMenu("Editor"))
      {
        ImGui::Separator();
        const auto& stats_window = m_helper_windows["Stats"];
        ImGui::Checkbox("Show Stats Window", &stats_window->is_active);
        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }

    if(show_save_scene)
    {
      new_scene = showSaveScene(show_save_scene);
    }

    if(new_scene)
      showSaveNewScne(new_scene);
  }

  void EditorLayer::updateGizmos()
  {
    Mat4 scene_cam_view, scene_cam_proj;
    static bool tf = false;
    static bool usc = false;
    ImGuizmo::SetOrthographic(tf);
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(m_min_vp_bounds.x, m_min_vp_bounds.y,
        m_max_vp_bounds.x - m_min_vp_bounds.x, m_max_vp_bounds.y - m_min_vp_bounds.y);
    ImGui::Begin("__DEBUG__");
    ImGui::Checkbox("ortho", &tf);
    ImGui::Checkbox("use Scene Camera", &usc);
    ImGui::End();
    if(usc)
    {
      SceneCamera* camera = nullptr;
      TransformComponent* tc = nullptr;
      m_scene->forEach<CameraComponent>([&](auto e_id, CameraComponent& cc)
      {
        Entity e(e_id, m_scene.get());
        camera = cc.is_primary ? &cc.camera : nullptr;
        tc = e.tryGetComponent<TransformComponent>();
        REAL_CORE_ASSERT(tc);
      });
      if(camera && tc)
      {
        scene_cam_proj = camera->getProjection();
        scene_cam_view = glm::inverse(tc->getTransformationMatrix());
      }
    }
    auto current_selected = m_scene_hierarchy_panel.getCurrentSelected();
    if(current_selected)
    {
      auto* current_transform_component = current_selected.tryGetComponent<TransformComponent>();
      if(current_transform_component)
      {
        auto current_transformation = current_transform_component->getTransformationMatrix();
        const auto& cam_proj = usc ? scene_cam_proj : m_camera.getProjection();
        const auto& cam_view = usc ? scene_cam_view : m_camera.getView();
        ImGuizmo::Manipulate(glm::value_ptr(cam_view), glm::value_ptr(cam_proj),
          (ImGuizmo::OPERATION)m_op, ImGuizmo::LOCAL, glm::value_ptr(current_transformation));
        if(ImGuizmo::IsUsing())
        {
    Vec3 rotate;
          decompose(current_transformation, current_transform_component->position,
              rotate, current_transform_component->scale);
          auto droatate = rotate - current_transform_component->rotation;
          current_transform_component->rotation = rotate;//+= droatate;
        }
      }
    }
  }

  bool EditorLayer::showSaveScene(bool& show_save_scene)
  {
    const auto& conf = Project::getConfig();
    bool new_scene = false;
    ImGui::OpenPopup("Save Scene");
    if(ImGui::BeginPopupModal("Save Scene"))
    {
      ImGui::TextColored(ImVec4(0.1, 0.7, 0.3, 1), "Save Current Changes");
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1, 0.1, 0.8, 1));
      if(ImGui::Button("Save"))
      {
        SceneSerializer ss(m_scene);
        ss.serializeText((conf.proj_dir / conf.asset_dir / AssetManager::getPath(m_scene_handle)).string().c_str());
        new_scene = true;
        ImGui::CloseCurrentPopup();
        show_save_scene = false;
      }
      ImGui::PopStyleColor();
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9, 0.1, 0.1, 1));
      if(ImGui::Button("Discard Changes"))
      {
        new_scene = true;
        ImGui::CloseCurrentPopup();
        show_save_scene = false;
      }
      ImGui::PopStyleColor();
      if(ImGui::Button("Cancel"))
      {
        ImGui::CloseCurrentPopup();
        show_save_scene = false;
        new_scene = false;
      }
      ImGui::EndPopup();
    }
    return new_scene;
  }

  void EditorLayer::showSaveNewScne(bool& new_scene)
  {
    const auto& conf = Project::getConfig();
    ImGui::OpenPopup("Save New Scene");
    if(ImGui::BeginPopupModal("Save New Scene"))
    {
      if(ImGui::Button("Open in File Manager"))
      {
        FileDialogs fd;//TODO: only in res dir
        auto path = fd.SaveFile(".Real");
        m_scene = createARef<Scene>();
        SceneSerializer ss(m_scene);
        ss.serializeText(path.c_str());
        AssetManager::Asset::Meta meta;
        meta.path = std::filesystem::relative(path, conf.proj_dir / conf.asset_dir);
        meta.type = AssetManager::Asset::Type::Scene;
        auto uuid = AssetManager::load(meta);
        if(uuid)
        {
          auto temp = m_scene;
          m_scene_handle = uuid;
          AssetManager::get<AssetManager::SceneAsset>(uuid)->scene = createARef<Scene>();
          m_editor_scene = AssetManager::get<AssetManager::SceneAsset>(uuid)->scene;
          m_scene = m_editor_scene;
          SceneChangedEvent e(temp, m_scene);
          EditorEventsManager::onEvent(e);
        }
        else REAL_CORE_ERROR("Invalid Scene UUID");
        new_scene = false;
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }
  }

  void EditorLayer::onEditorEvent(EditorEvent& e)
  {
    EditorEventDispatcher dp(e);
    dp.dispatch<SceneChangedEvent>(BIND_EVENT_FUNCTION(EditorLayer::onSceneChangedCall));
    dp.dispatch<ScenePlayEvent>(BIND_EVENT_FUNCTION(EditorLayer::onScenePlayCall));
    dp.dispatch<SceneStopEvent>(BIND_EVENT_FUNCTION(EditorLayer::onSceneStopCall));
    dp.dispatch<ProjectSetEvent>(BIND_EVENT_FUNCTION(EditorLayer::onProjectSetCall));
  }

  bool EditorLayer::onSceneChangedCall(SceneChangedEvent& e)
  {
    return false;
  }

  bool EditorLayer::onScenePlayCall(ScenePlayEvent& e)
  {
    return false;
  }

  bool EditorLayer::onSceneStopCall(SceneStopEvent& e)
  {
    return false;
  }

  bool EditorLayer::onProjectSetCall(ProjectSetEvent& e)
  {
    auto& config = Project::getConfig();
    activate();
    App::get()->getLayer("ProjectSelectionLayer")->setActive(false);
    m_content_browser = { config.proj_dir / config.asset_dir };
    return false;
  }

  void EditorLayer::serializeEditor(const char* path)
  {
    EditorSerializer es(this);
    if(!es.serialize(path))
      REAL_ASSERT(false);
  }

  void EditorLayer::deSerializeEditor(const char* path)
  {
    EditorSerializer es(this);
    if(!es.deSerialize(path))
      m_default_editor = true;
  }


  void EditorLayer::setColorScheme()
  {
    m_colors["WindowBg"] = Vec4(0.1f, 0.1f, 0.1f, 1.0f);
    m_colors["HeaderHovered"] = Vec4(0.2f, 0.2f, 0.2f, 1.0f);
    m_colors["HeaderActive"] = Vec4(0.17f, 0.17f, 0.17f, 1.0f);
    m_colors["ChildBg"] = Vec4(0.1f, 0.1f, 0.12f, 1.0f);
    m_colors["PopupBg"] = Vec4(0.32f, 0.2f, 0.2f, 1.0f);
    m_colors["Border"] = Vec4(0.45f, 0.45f, 0.54f, 1.0f);
    m_colors["BorderShadow"] = Vec4(0.1f, 0.1f, 0.1f, 0.7f);
    m_colors["MenuBarBg"] = Vec4(0.1f, 0.1f, 0.1f, 1.0f);
    m_colors["ScrollbarBg"] = Vec4(0.3f, 0.3f ,0.32f, 1.0f);
    m_colors["ScrollbarGrab"] = Vec4(0.4f, 0.4f, 0.43f, 1.0f);
    m_colors["ScrollbarGrabHovered"] = Vec4(0.5f, 0.5f, 0.5f, 1.0f);
    m_colors["ScrollbarGrabActive"] = Vec4(0.4f, 0.4f, 0.6f, 1.0f);
    m_colors["CheckMark"] = Vec4(0.4f, 0.47f, 0.4f, 1.0f);
    m_colors["SliderGrab"] = Vec4(0.3f, 0.3f, 0.3f, 1.0f);
    m_colors["SliderGrabActive"] = Vec4(0.3f, 0.3f, 0.4f, 1.0f);
    m_colors["Header"] = Vec4(0.32f, 0.32f, 0.39f, 1.0f);
    m_colors["Separator"] = Vec4(0.6f, 0.6f, 0.6f, 0.8f);
    m_colors["SeparatorHovered"] = Vec4(0.3f, 0.3f, 0.3f, 0.9f);
    m_colors["SeparatorActive"] = Vec4(0.3f, 0.43f, 0.3f ,1.0f);
    m_colors["ResizeGrip"] = Vec4(0.3f, 0.3f, 0.5f, 0.6f);
    m_colors["ResizeGripHovered"] = Vec4(0.3f, 0.3f, 0.6f, 0.7f);
    m_colors["ResizeGripActive"] = Vec4(0.3f, 0.3f, 0.7f, 0.8f);
    m_colors["DockingPreview"] = Vec4(0.32f, 0.32f, 0.32f, 1.0f);
    m_colors["DockingEmptyBg"] = Vec4(0.32f, 0.32f, 0.32f, 1.0f);
    m_colors["PlotLines"] = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    m_colors["PlotLinesHovered"] = Vec4(1.0f);
    m_colors["PlotHistogram"] = Vec4(1.0f);
    m_colors["PlotHistogramHovered"] = Vec4(1.0f);
    m_colors["TableHeaderBg"] = Vec4(1.0f);
    m_colors["TableBorderStrong"] = Vec4(1.0f);
    m_colors["TableBorderLight"] = Vec4(1.0f);
    m_colors["TableRowBg"] = Vec4(1.0f);
    m_colors["TableRowBgAlt"] = Vec4(1.0f);
    m_colors["TextSelectedBg"] = Vec4(1.0f, 1.0f, 1.0f, 0.4f);
    m_colors["DragDropTarget"] = Vec4(0.2f, 0.45f, 0.35f, 1.0f);
    m_colors["NavHighlight"] = Vec4(1.0f);
    m_colors["NavWindowingHighlight"] = Vec4(1.0f);
    m_colors["NavWindowingDimBg"] = Vec4(0.2f, 0.2f, 0.2f , 0.4f);
    m_colors["ModalWindowDimBg"] = Vec4(0.2f, 0.2f, 0.2f, 0.4f);
    m_colors["Button"] = Vec4(0.2f, 0.2f, 0.2f, 1.0f);
    m_colors["ButtonHovered"] = Vec4(0.3f, 0.3f, 0.3f, 1.0f);
    m_colors["ButtonActive"] = Vec4(0.15f, 0.15f, 0.15f, 1.0f);
    m_colors["FrameBg"] = Vec4(0.2f, 0.2f, 0.2f, 1.0f);
    m_colors["FrameBgHovered"] = Vec4(0.3f, 0.3f, 0.3f, 1.0f);
    m_colors["FrameBgActive"] = Vec4(0.17f, 0.1f, 0.19f, 1.0f);
    m_colors["Tab"] = Vec4(0.14f, 0.17f, 0.13f, 1.0f);
    m_colors["TabHovered"] = Vec4(0.3f, 0.34f, 0.35f, 1.0f);
    m_colors["TabActive"] = Vec4(0.1f, 0.25f, 0.25f, 1.0f);
    m_colors["TabUnfocused"] = Vec4(0.07f, 0.2f, 0.13f, 1.0f);
    m_colors["TabUnfocusedActive"] = Vec4(0.21f, 0.23f, 0.22f, 1.0f);
    m_colors["TitleBg"] = Vec4(0.15f, 0.154f, 0.145f, 1.0f);
    m_colors["TitleBgActive"] = Vec4(0.2f, 0.25f, 0.3f, 1.0f);
    m_colors["TitleBgCollapsed"] = Vec4(0.2f, 0.3f, 0.6f, 1.0f);

  }

  void EditorLayer::initImGui()
  {
    //Ini
    ImGui::LoadIniSettingsFromDisk(m_imgui_ini_path.c_str());

    //Font
    auto& io = ImGui::GetIO();
    io.FontDefault = io.Fonts->AddFontFromFileTTF(m_default_font.c_str(), m_default_font_size);

    ImFontConfig config;
    config.MergeMode = true;
    config.GlyphMinAdvanceX = 13.0f; // Use if you want to make the icon monospaced
    static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    io.Fonts->AddFontFromFileTTF("../res/fonts/Font-Awesome/fa-solid-900.ttf", 18.0f, &config, icon_ranges);

    //Colors
    auto& colors = ImGui::GetStyle().Colors;

    colors[ImGuiCol_WindowBg] = *(ImVec4*)&m_colors.at("WindowBg");
    colors[ImGuiCol_HeaderHovered] = *(ImVec4*)&m_colors.at("HeaderHovered");
    colors[ImGuiCol_HeaderActive] = *(ImVec4*)&m_colors.at("HeaderActive");
    colors[ImGuiCol_Button] = *(ImVec4*)&m_colors.at("Button");
    colors[ImGuiCol_ButtonHovered] = *(ImVec4*)&m_colors.at("ButtonHovered");
    colors[ImGuiCol_ButtonActive] = *(ImVec4*)&m_colors.at("ButtonActive");
    colors[ImGuiCol_FrameBg] = *(ImVec4*)&m_colors.at("FrameBg");
    colors[ImGuiCol_FrameBgHovered] = *(ImVec4*)&m_colors.at("FrameBgHovered");
    colors[ImGuiCol_FrameBgActive] = *(ImVec4*)&m_colors.at("FrameBgActive");
    colors[ImGuiCol_Tab] = *(ImVec4*)&m_colors.at("Tab");
    colors[ImGuiCol_TabHovered] = *(ImVec4*)&m_colors.at("TabHovered");
    colors[ImGuiCol_TabActive] = *(ImVec4*)&m_colors.at("TabActive");
    colors[ImGuiCol_TabUnfocused] = *(ImVec4*)&m_colors.at("TabUnfocused");
    colors[ImGuiCol_TabUnfocusedActive] = *(ImVec4*)&m_colors.at("TabUnfocusedActive");
    colors[ImGuiCol_TitleBg] = *(ImVec4*)&m_colors.at("TitleBg");
    colors[ImGuiCol_TitleBgActive] = *(ImVec4*)&m_colors.at("TitleBgActive");
    colors[ImGuiCol_TitleBgCollapsed] = *(ImVec4*)&m_colors.at("TitleBgCollapsed");
    colors[ImGuiCol_ChildBg] = *(ImVec4*)&m_colors.at("ChildBg");
    colors[ImGuiCol_PopupBg] = *(ImVec4*)&m_colors.at("PopupBg");
    colors[ImGuiCol_Border] = *(ImVec4*)&m_colors.at("Border");
    colors[ImGuiCol_BorderShadow] = *(ImVec4*)&m_colors.at("BorderShadow");
    colors[ImGuiCol_MenuBarBg] = *(ImVec4*)&m_colors.at("MenuBarBg");
    colors[ImGuiCol_ScrollbarBg] = *(ImVec4*)&m_colors.at("ScrollbarBg");
    colors[ImGuiCol_ScrollbarBg] = *(ImVec4*)&m_colors.at("ScrollbarGrab");
    colors[ImGuiCol_ScrollbarBg] = *(ImVec4*)&m_colors.at("ScrollbarGrabHovered");
    colors[ImGuiCol_ScrollbarGrabActive] =  *(ImVec4*)&m_colors.at("ScrollbarGrabActive");
    colors[ImGuiCol_CheckMark] =  *(ImVec4*)&m_colors.at("CheckMark");
    colors[ImGuiCol_SliderGrab] =  *(ImVec4*)&m_colors.at("SliderGrab");
    colors[ImGuiCol_SliderGrabActive] =  *(ImVec4*)&m_colors.at("SliderGrabActive");
    colors[ImGuiCol_Header] =  *(ImVec4*)&m_colors["Header"];
    colors[ImGuiCol_Separator] =  *(ImVec4*)&m_colors["Separator"];
    colors[ImGuiCol_SeparatorHovered] =  *(ImVec4*)&m_colors.at("SeparatorHovered");
    colors[ImGuiCol_SeparatorActive] =  *(ImVec4*)&m_colors.at("SeparatorActive");
    colors[ImGuiCol_ResizeGrip] =  *(ImVec4*)&m_colors.at("ResizeGrip");
    colors[ImGuiCol_ResizeGripHovered] =  *(ImVec4*)&m_colors.at("ResizeGripHovered");
    colors[ImGuiCol_ResizeGripActive] =  *(ImVec4*)&m_colors.at("ResizeGripActive");
    colors[ImGuiCol_DockingPreview] =  *(ImVec4*)&m_colors.at("DockingPreview");
    colors[ImGuiCol_DockingEmptyBg] =  *(ImVec4*)&m_colors.at("DockingEmptyBg");
    colors[ImGuiCol_PlotLines] =  *(ImVec4*)&m_colors.at("PlotLines");
    colors[ImGuiCol_PlotLinesHovered] =  *(ImVec4*)&m_colors.at("PlotLinesHovered");
    colors[ImGuiCol_PlotHistogram] =  *(ImVec4*)&m_colors.at("PlotHistogram");
    colors[ImGuiCol_PlotHistogramHovered] =  *(ImVec4*)&m_colors.at("PlotHistogramHovered");
    colors[ImGuiCol_TableHeaderBg] =  *(ImVec4*)&m_colors.at("TableHeaderBg");
    colors[ImGuiCol_TableBorderStrong] =  *(ImVec4*)&m_colors.at("TableBorderStrong");
    colors[ImGuiCol_TableBorderLight] =  *(ImVec4*)&m_colors.at("TableBorderLight");
    colors[ImGuiCol_TableRowBg] =  *(ImVec4*)&m_colors.at("TableRowBg");
    colors[ImGuiCol_TableRowBgAlt] =  *(ImVec4*)&m_colors.at("TableRowBgAlt");
    colors[ImGuiCol_TextSelectedBg] =  *(ImVec4*)&m_colors.at("TextSelectedBg");
    colors[ImGuiCol_DragDropTarget] =  *(ImVec4*)&m_colors.at("DragDropTarget");
    colors[ImGuiCol_NavHighlight] =  *(ImVec4*)&m_colors.at("NavHighlight");
    colors[ImGuiCol_NavWindowingHighlight] =  *(ImVec4*)&m_colors.at("NavWindowingHighlight");
    colors[ImGuiCol_NavWindowingDimBg] =  *(ImVec4*)&m_colors.at("NavWindowingDimBg");
    colors[ImGuiCol_ModalWindowDimBg] = *(ImVec4*)&m_colors.at("ModalWindowDimBg");

  }

  void EditorLayer::onEvent(Event& e)
  {
    EventDispatcher dp(e);
    dp.dispatch<MouseScrolledEvent>(BIND_EVENT_FUNCTION(EditorLayer::onMouseScrolledEvent));
    dp.dispatch<KeyPressedEvent>(BIND_EVENT_FUNCTION(EditorLayer::onKeyPressedEvent));
    dp.dispatch<KeyReleasedEvent>(BIND_EVENT_FUNCTION(EditorLayer::onKeyReleasedEvent));
    dp.dispatch<ScriptErrorEvent>([&](auto& e){ ErrorModal::setContext(e); ErrorModal::show();  return false; });
  }

  bool EditorLayer::onKeyPressedEvent(KeyPressedEvent& e)
  {
    return false;
  }

  bool EditorLayer::onKeyReleasedEvent(KeyReleasedEvent& e)
  {
    return false;
  }

  bool EditorLayer::onMouseScrolledEvent(MouseScrolledEvent& e)
  {
    m_camera.onMouseScrolledEvent(e);
    return false;
  }

  void EditorLayer::handelCMD(int cmdkey)
  {
    if(Input::isKeyPressed(Keyboard::Q))
      m_op = GizmoOp::Bounds;
    else if(Input::isKeyPressed(Keyboard::W))
      m_op = GizmoOp::Translate;
    else if(Input::isKeyPressed(Keyboard::R))
      m_op = GizmoOp::Rotate;
    else if(Input::isKeyPressed(Keyboard::S))
      m_op = GizmoOp::Scale;
    else if(Input::isKeyPressed(Keyboard::U))
      m_op = GizmoOp::Universal;
    else if(Input::isKeyPressed(Keyboard::D))
    {
      auto e = m_scene_hierarchy_panel.getCurrentSelected();
      if(e)
        m_scene->copyEntity(e);
    }
  }
}
