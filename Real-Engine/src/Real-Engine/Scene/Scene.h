#ifndef KR_SCENE_H
#define KR_SCENE_H

#include "Real-Engine/Core/Core.h"
#include "Real-Engine/Core/Math/math.h"
#include "Real-Engine/Core/Timestep.h"
#include "Real-Engine/Core/UUID.h"
#include "box2d/id.h"
#include <entt/entt.hpp>



struct b2WorldId;

namespace Real
{
  class ScriptComponent;
  class Entity;

  class REAL_API Scene
  {
    struct EditorCamera { Mat4 view, projection; };
  public:
    Scene(const std::string& name = "Scene");
    Scene(const Scene& other);
    
    static ARef<Scene> copy(const ARef<Scene>&);

    Entity addEntity(const std::string& tag = "");
    Entity addEntity(UUID uuid, const std::string& tag = "");
    Entity copyEntity(Entity& e);
    void removeEntity(const Entity& entity);
    void clear();
    
    template<typename T>//TODO: ...
    void forEach(std::function<void(entt::entity, T&)> func)
    {
      m_registry.view<T>().each(func);
    }

    template<typename T>
    entt::view<T> view()
    {
      return m_registry.view<T>();
    }

    Entity getEntity(const std::string& name);
    Entity getEntity(UUID id);

    uint32_t getViewportWidth()  { return m_viewport_width;  };
    uint32_t getViewportHeight() { return m_viewport_height; };

    void onLoad();
    void onStart();
    void onUpdate(Timestep ts);
    void onEditorUpdate(Timestep ts);
    void setEditorCamera(const Mat4& view, const Mat4& projection) { m_editor_camera = { view, projection }; };
    void onViewportResize(uint32_t width, uint32_t height);
    void onEnd();
    
    void setName(const std::string& name) { m_name = name; }
    std::string getName() const           { return m_name; }

    UUID getUUID() { return m_id; }

    inline bool operator==(const Scene& rhs)
    {
      return this->m_id == rhs.m_id;
    }
    inline bool operator!=(const Scene& rhs)
    {
      return this->m_id != rhs.m_id;
    }
  private:
    std::string m_name;
    UUID m_id;
    entt::registry m_registry;
    b2WorldId m_physics_world = b2_nullWorldId;
    uint32_t m_viewport_width = 1280, m_viewport_height = 720;
    EditorCamera m_editor_camera;
                                //     Entity
    std::unordered_map<uint64_t, std::pair<uint32_t, Scene*>> m_fast_access; 
  private:
    friend class Entity;
    friend class SceneHierarchy;
    friend class SceneSerializer;
  };

  bool operator==(const ARef<Scene>& lhs, const ARef<Scene>& rhs);

  bool operator!=(const ARef<Scene>& lhs, const ARef<Scene>& rhs);

}


#endif //KR_SCENE_H
