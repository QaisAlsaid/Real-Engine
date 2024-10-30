#include "Inspector.h"
#include "EditorEvents/EditorEvents.h"
#include "Panels/ErrorModal.h"
#include "Real-Engine/Core/Macros.h"
#include "Real-Engine/Scene/Components.h"
#include "glm/trigonometric.hpp"
#include "imgui.h"



namespace Real
{
  static const char* projection_type_str[2] = {"Orthographic", "Perspective"};
  static const char* texture_type_str[2] = {"None", "Texture2D"};
  static const char* body_type_str[3] = {"Static", "Kinematic", "Dynamic"};

  template<typename T>
  void drawComponent(Entity& e, const char* label, std::function<void(T*, bool removed)> func)
  {
    T* comp = e.tryGetComponent<T>();
    if(comp)
    {
      ImGui::Text("%s", label);
      ImGui::SameLine();
      bool removed = false;
      if(ImGui::Button((std::string("Remove ") + label).c_str()))
        removed = true;
      ImGui::Separator();
      func(comp, removed);
    }
  }

  Inspector::Inspector()
  {
    m_sub_id = EditorEventsManager::subscribe(BIND_EVENT_FUNCTION(Inspector::onEditorEvent));
  }

  void Inspector::onGuiUpdate()
  {
    ImGui::Begin("Inspector");
    
    if(m_current)
    {

      UUID uuid;
      drawComponent<IDComponent>(m_current, "UUID", [&](auto* id_comp, bool removed)
      {
        uuid = id_comp->ID;
        ImGui::Text("UUID: %lu", (uint64_t)uuid);
      }); 

      //export vars
      {
        if(m_current.hasComponent<ScriptComponent>())
          drawExportedVars(uuid);
      }

      drawComponent<TagComponent>(m_current, "Tag", [](auto* tag_comp, bool removed)
      {
        auto& tag = tag_comp->name;
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, tag.c_str());
        if(ImGui::InputText("##Tag", buffer, sizeof(buffer)))
        {
          tag = std::string(buffer);
        }
      }); 

      drawComponent<ScriptComponent>(m_current, "Script", [&](auto* script_comp, bool removed)
      {
        bool change = false;
        auto script = AssetManager::get<AssetManager::ScriptAsset>(script_comp->script_handle);
        auto& path = script->meta.path;
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, path.c_str());
        if(ImGui::InputText("ScriptPath", buffer, sizeof(buffer)))
        {
          path = std::string(buffer);
        }
        if(ImGui::BeginDragDropTarget())
        {
          if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCRIPT_ASSET_HANDEL"))
          {
            UUID asset_handle = *(UUID*)payload->Data;
            REAL_CORE_WARN("Accepting drag and drop id: {0}", asset_handle);
            auto scrp = AssetManager::get<AssetManager::ScriptAsset>(asset_handle);
            if(scrp->is_valid)
              script_comp->script_handle = asset_handle;
            else 
            {
              auto r = createARef<ScriptError>(scrp->err);
              ErrorModal::setContext(r);
              ErrorModal::show();
            }
          }
          ImGui::EndDragDropTarget();
        }

        if(removed)
          m_current.removeComponent<ScriptComponent>();
      });

      drawComponent<TransformComponent>(m_current, "Transform", [&](auto* trans_comp, bool removed)
      {
        ImGui::DragFloat3("Position", glm::value_ptr(trans_comp->position), 0.25f/((trans_comp->scale.x + trans_comp->scale.y)/2.0f));
        ImGui::DragFloat3("Scale", glm::value_ptr(trans_comp->scale), 0.25f/((trans_comp->scale.x + trans_comp->scale.y)/2.0f));
        Vec3& rad_rot = trans_comp->rotation;
        Vec3 deg_rot = glm::degrees(rad_rot);
        ImGui::DragFloat3("Rotation", glm::value_ptr(deg_rot), 0.25f);
        rad_rot = glm::radians(deg_rot);
        
        //if(removed)
        //  m_current.removeComponent<TransformComponent>();
      });

      drawComponent<CameraComponent>(m_current, "Camera", [&](auto* cam_comp, bool removed)
      {
        bool any_changes = false;
        SceneCamera& cam = cam_comp->camera;
        const char* current_projection_type_str = projection_type_str[(int)cam.getType()];
        ImGui::Checkbox("Main Camera", &cam_comp->is_primary);
        if(ImGui::BeginCombo("Projection Type", current_projection_type_str))
        {
          for(uint8_t i = 0; i < 2; ++i)
          {
            bool is_selected = projection_type_str[i] == current_projection_type_str; //its ok because the ptrs are the same and we are not making string comparision
            if(ImGui::Selectable(projection_type_str[i], is_selected))
            {
              current_projection_type_str = projection_type_str[i];
              cam.setType((SceneCamera::ProjectionType)i);
            }
            if(is_selected) ImGui::SetItemDefaultFocus();
          }
          ImGui::EndCombo();
        }
        switch((int)cam.getType())
        {
          case (int)SceneCamera::ProjectionType::Orthographic:
          {
            auto od = cam.getOrthographicData();
            
            if(ImGui::DragFloat("Size", &od.size)
            || ImGui::DragFloat("Near", &od.near_clip)
            || ImGui::DragFloat("Far", &od.far_clip)) any_changes = true;
            
            if(any_changes)
              cam.setOrthographicData(od);
            
            break;
          }
          case (int)SceneCamera::ProjectionType::Perspective:
          {
            auto pd = cam.getPerspectiveData();
            float deg_fov = glm::degrees(pd.fov);
            if(ImGui::DragFloat("FOV", &deg_fov)
            || ImGui::DragFloat("Near", &pd.near_clip)
            || ImGui::DragFloat("Far", &pd.far_clip)) any_changes = true;
            
            if(any_changes)
            {
              pd.fov = glm::radians(deg_fov);
              cam.setPerspectiveData(pd);
            }
            break;
          }
        }
        if(removed)
          m_current.removeComponent<CameraComponent>();
      });

      drawComponent<SpriteComponent>(m_current, "Sprite", [&](auto* sprite_comp, bool removed)
      {
        const char* current_texture_type_str = texture_type_str[0];
        if(ImGui::BeginCombo("Select Texture", current_texture_type_str))
        {
          for(uint8_t i = 0; i < 2; ++i)
          {
            bool is_selected = texture_type_str[i] == current_texture_type_str;
            if(ImGui::Selectable(texture_type_str[i], is_selected))
            {
              current_texture_type_str = texture_type_str[i];
              //open select from Asset manager modal
            }
            if(is_selected) ImGui::SetItemDefaultFocus();
          }
          ImGui::EndCombo();
        }
        if(ImGui::BeginDragDropTarget())
        {
          if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE2D_ASSET_HANDEL"))
          {
            UUID asset_handle = *(UUID*)payload->Data;
            REAL_CORE_WARN("Accepting drag and drop id: {0}", asset_handle);
            sprite_comp->texture_handle = asset_handle;
            current_texture_type_str = texture_type_str[1];
          }
          ImGui::EndDragDropTarget();
        }
        ImGui::Text("TextureHandel %li", (uint64_t)sprite_comp->texture_handle);
        ImGui::ColorEdit4("Color", glm::value_ptr(sprite_comp->color)); 
        if(removed)
        {
          m_current.removeComponent<SpriteComponent>();
        }
      });

      drawComponent<CircleComponent>(m_current, "Circle", [&](auto* circle_comp, bool removed)
      {
        ImGui::ColorEdit4("Color", glm::value_ptr(circle_comp->color));
        ImGui::DragFloat("Thickness", &circle_comp->thickness, 0.05f, 0.0f, 1.0f);
        ImGui::DragFloat("Blur", &circle_comp->blur, 0.05f, 0.0f, 1.0f);
        
        if(removed)
          m_current.removeComponent<CircleComponent>();
      });

      drawComponent<RigidBody2DComponent>(m_current, "Rigid Body", [&](auto* rb2dc, bool removed)
      {
        const char* current_body_type_str = body_type_str[(int)rb2dc->type];
        if(ImGui::BeginCombo("Body Type", current_body_type_str))
        {
          for(uint8_t i = 0; i < 3; ++i)
          {
            bool is_selected = body_type_str[i] == current_body_type_str;
            if(ImGui::Selectable(body_type_str[i], is_selected))
            {
              current_body_type_str = body_type_str[i];
              rb2dc->type = (RigidBody2DComponent::BodyType)i;  
            }
            if(is_selected) ImGui::SetItemDefaultFocus();
          }
          ImGui::EndCombo();
        }
        
        ImGui::Checkbox("Fixed Rotation", &rb2dc->fixed_rotation);
        ImGui::DragFloat("Gravity Scale", &rb2dc->gravity_scale);

        if(removed)
          m_current.removeComponent<RigidBody2DComponent>();
      });

      /*drawComponent<MovmentComponent>(m_current, "Movment", [&](MovmentComponent* movment_comp, bool removed)
      {
        ImGui::DragFloat3("Linear Velocity", glm::value_ptr(movment_comp->linear_velocity));
        ImGui::DragFloat("Angular Velocity", &movment_comp->angular_velocity);
        
        if(removed)
          m_current.removeComponent<MovmentComponent>();
      });
*/
      drawComponent<BoxColliderComponent>(m_current, "Box Collider", [&](auto* bcc, bool removed)
      {
        ImGui::DragFloat2("Offset", glm::value_ptr(bcc->offset));
        ImGui::DragFloat2("Size", glm::value_ptr(bcc->size));
        ImGui::DragFloat("Density", &bcc->density);
        ImGui::DragFloat("Friction", &bcc->friction);
        ImGui::DragFloat("Restitution", &bcc->restitution, 1.0f, 0.0f, 1.0f);
        ImGui::SameLine();
        ImGui::DragFloat("Threshold", &bcc->restitution_threshold, 1.0f, 0.0f, 1.0f);
        if(removed)
          m_current.removeComponent<BoxColliderComponent>();
        });
      
      drawComponent<CircleColliderComponent>(m_current, "Circle Collider", [&](auto* ccc, bool removed)
      {
        ImGui::DragFloat2("Offset", glm::value_ptr(ccc->offset));
        ImGui::DragFloat("Radius", &ccc->radius);
        ImGui::DragFloat("Density", &ccc->density);
        ImGui::DragFloat("Friction", &ccc->friction);
        ImGui::DragFloat("Restitution", &ccc->restitution, 1.0f, 0.0f, 1.0f);
        ImGui::SameLine();
        ImGui::DragFloat("Threshold", &ccc->restitution_threshold, 1.0f, 0.0f, 1.0f);
        if(removed)
          m_current.removeComponent<CircleColliderComponent>();
        });

      if(ImGui::Button("Add Component"))
        ImGui::OpenPopup("AddComponent");
      if(ImGui::BeginPopupModal("AddComponent"))
      {
        if(ImGui::MenuItem("Transform"))
          m_current.insertComponent<TransformComponent>();
        if(ImGui::MenuItem("Script"))
          m_current.insertComponent<ScriptComponent>();
        //if(ImGui::MenuItem("Tag"))
        //  m_current.addComponent<TagComponent>();
        if(ImGui::MenuItem("Camera"))
          m_current.insertComponent<CameraComponent>();
        if(ImGui::MenuItem("Sprite"))
          m_current.insertComponent<SpriteComponent>();
        if(ImGui::MenuItem("Circle"))
          m_current.insertComponent<CircleComponent>();
        if(ImGui::MenuItem("Rigid Body 2D"))
          m_current.insertComponent<RigidBody2DComponent>();
        if(ImGui::MenuItem("Movment Component"))
          m_current.insertComponent<MovmentComponent>();
        if(ImGui::MenuItem("Box Collider"))
          m_current.insertComponent<BoxColliderComponent>();
        if(ImGui::MenuItem("Circle Collider"))
          m_current.insertComponent<CircleColliderComponent>();
        if(ImGui::Button("Cancel"))
          ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
        ImGui::CloseCurrentPopup();
      }
    }
    ImGui::End();
  }

  void Inspector::drawExportedVars(UUID uuid)
  {
    const auto* ev = Export::getExportVariablesFor(uuid); 
    if(!ev) return;
    for(const auto& exp : *ev)
    {
      REAL_CORE_WARN("Recived var: {0}", exp.first);
      switch(exp.second.getType())
      {
        case ExportType::Type::Vec4:
        {
          auto* data = exp.second.getAs<Vec4>();
          if(glm::abs(*data) - glm::abs(*data) == glm::vec4(0))
            ImGui::DragFloat4(exp.first, glm::value_ptr(*data));
          REAL_CORE_WARN("var; {0}, data: {1}", exp.first, *data);
          break;
        }
        case ExportType::Type::Vec3:
        {
          auto* data = exp.second.getAs<Vec3>();
          if(glm::abs(*data) - glm::abs(*data) == glm::vec3(0))
            ImGui::DragFloat3(exp.first, glm::value_ptr(*data));
          REAL_CORE_WARN("var; {0}, data: {1}", exp.first, *data);
          break;
        }
        case ExportType::Type::Vec2:
        {
          if(exp.second.getRawData() == nullptr) REAL_CORE_ERROR("Data is null");
          auto* data = exp.second.getAs<Vec2>();
          if(glm::abs(*data) - glm::abs(*data) == glm::vec2(0))
            ImGui::DragFloat2(exp.first, glm::value_ptr(*data));
          REAL_CORE_WARN("var; {0}, data: {1}", exp.first, *data);
          break;
        }
        case ExportType::Type::Float:
        {
          auto* data = exp.second.getAs<Float>();
          if(std::abs(data->_float) - std::abs(data->_float) == 0.0f)
            ImGui::DragFloat(exp.first, &data->_float); 
          REAL_CORE_WARN("var; {0}, data: {1}", exp.first, data->_float);
          break;
        }
        case ExportType::Type::Int:
        {
          auto* data = exp.second.getAs<Int>();
          if(std::abs(data->_int) - std::abs(data->_int) == 0i)
            ImGui::DragInt(exp.first, &data->_int);
          REAL_CORE_WARN("var; {0}, data: {1}", exp.first, data->_int);
          break;
        }
        case ExportType::Type::String:
        {
          auto* data = exp.second.getAs<String>();
          char buffer[256];
          memset(buffer, 0, sizeof(buffer));
          strcpy(buffer, data->_string.c_str());
          if(ImGui::InputText((exp.first + std::string("##") + std::to_string(uuid)).c_str(), buffer, sizeof(buffer)))
          {
            *data = buffer;
          }
          REAL_CORE_WARN("var; {0}, data: {1}", exp.first, data->_string);
          break;
        }
        case ExportType::Type::RGBA_Color:
        {
          ImGui::ColorEdit4(exp.first, glm::value_ptr(*exp.second.getAs<Vec4>()));
          break;
        }
        case ExportType::Type::Scene:
        {
          UUID* data = exp.second.getAs<UUID>();
          char buffer[1];
          memset(buffer, 0, sizeof(buffer));
          strcpy(buffer, "");
          if(ImGui::InputText((exp.first + std::string("##") + std::to_string(uuid)).c_str(), buffer, sizeof(buffer)))
          {
          }
          if(ImGui::BeginDragDropTarget())
          {
            if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_ASSET_HANDEL"))
            {
              UUID asset_handle = *(UUID*)payload->Data;
              REAL_CORE_WARN("Accepting drag and drop id: {0}", asset_handle);
              auto sc = AssetManager::get<AssetManager::SceneAsset>(asset_handle);
              if(sc->is_valid)
                *data = *((UUID*)payload->Data);
            }
            ImGui::EndDragDropTarget();
          }
          auto sc = AssetManager::get<AssetManager::SceneAsset>(*data);
          REAL_CORE_ERROR("is valid: {0}", sc->is_valid);

          REAL_CORE_WARN("var; {0}, data: {1}", exp.first, *data);
          break;

        }
        default: break;
      }
    }
  }

  void Inspector::onEditorEvent(EditorEvent& e)
  {
    EditorEventDispatcher dp(e);
    dp.dispatch<SCHPSelectionChangedEvent>(BIND_EVENT_FUNCTION(Inspector::onSCHPSelectionChangedEvent));
  }

  bool Inspector::onSCHPSelectionChangedEvent(SCHPSelectionChangedEvent& e)
  {
    m_current = e.getEntity();
    return false;
  }
}
