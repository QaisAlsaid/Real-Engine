#ifndef KR_COMPONENTS_H
#define KR_COMPONENTS_H

#include "Real-Engine/Scripting/Script.h"
#include "Real-Engine/Core/Core.h"
#include "Real-Engine/Core/UUID.h"
#include "SceneCamera.h"
#include "ScriptEntity.h"
#include "Real-Engine/Core/Math/math.h"

class b2Body;
class b2Fixture;

namespace Real
{
  struct ScriptComponent
  {
    UUID script_handle = UUID::invalid;
  };

  struct REAL_API IDComponent
  {
    UUID ID;
  };

  struct REAL_API TagComponent
  {
    std::string name;
    TagComponent() = default;
    TagComponent(const std::string& s) : name(s) {}
  };

  struct REAL_API TransformComponent
  {
    Vec3 position = { 0.0f, 0.0f, 0.0f };
    Vec3 scale    = { 1.0f, 1.0f, 1.0f };
    Vec3 rotation = { 0.0f, 0.0f, 0.0f };
    
    TransformComponent() = default;
    TransformComponent(const Vec3& pos, const Vec3& scale, const Vec3& rotation)
      : position(pos), scale(scale), rotation(rotation) {}
    Mat4 getTransformationMatrix()
    {
      Mat4 trans(1.0f);
      trans = glm::translate(trans, position);
      trans = glm::rotate(trans, rotation.x, {1, 0, 0});
      trans = glm::rotate(trans, rotation.y, {0, 1, 0});
      trans = glm::rotate(trans, rotation.z, {0, 0, 1});
      trans = glm::scale(trans, scale);
      return trans;
    }
  };

  struct REAL_API SpriteComponent
  {
    UUID texture_handle = UUID::invalid;
    Vec4 color;
    SpriteComponent() = default;
    SpriteComponent(const Vec4& color, UUID tux) : texture_handle(tux), color(color) {}
  };

  struct REAL_API CircleComponent
  {
    Vec4 color;
    float thickness = 0.1f;
    float blur = 0.005f;
    CircleComponent() = default;
  };

  struct REAL_API CameraComponent
  {
    SceneCamera camera;
    bool is_primary = false;
    bool is_fixed_aspect_ratio = false;
    CameraComponent() = default;
  };

  struct REAL_API NativeScriptComponent
  {
    ScriptEntity* instance = nullptr;
     
    void bind(ScriptEntity* trans_ownership)
    {
      instantiateScript = [&](){ return trans_ownership; };
      DestroyScript = [](NativeScriptComponent* native_script)
      {
        delete native_script->instance;
        native_script->instance = nullptr;
      };
    }
    template<typename T>
    void bind()
    {
      instantiateScript = [](){return static_cast<ScriptEntity*>(new T());};
      DestroyScript = [](NativeScriptComponent* native_script)
      {
        delete native_script->instance;
        native_script->instance = nullptr;
      };
    }
    private:
    std::function<ScriptEntity*(void)> instantiateScript;  
    //ScriptEntity* (*instantiateScript)();
      void (*DestroyScript)(NativeScriptComponent*);
    friend class Scene;
  };

  struct REAL_API RigidBody2DComponent
  {
    enum class BodyType {Static = 0, Kinematic, Dynamic};
    
    BodyType type = BodyType::Dynamic;
    
    float gravity_scale    = 1.0f;
    bool  fixed_rotation   = false;

    b2BodyId body = b2_nullBodyId;

    std::function<void(void)> distruct_function;

    ~RigidBody2DComponent()
    {
     // distruct_function();
    }
  };

  struct MovmentComponent 
  {
    Vec2 new_transform = Vec2(0);
    float new_angle = 0;
  };

  struct LinearImpulseComponent
  {
    Vec2 magnitude = Vec2(0);
    Vec2 point = Vec2(0);
  };

  struct ForceComponent
  {
    Vec2 magnitude = Vec2(0);
    Vec2 point = Vec2(0);
  };

  struct AngularImpulseComponent
  {
    float magnitude = 0;
  };

  struct TorqueComponent
  {
    float magnitude = 0;
  };

  struct REAL_API BoxColliderComponent 
  {
    Vec2 offset = {0.0f, 0.0f};
    Vec2 size = {0.5f, 0.5f};
    
    float density = 1.0f;
    float friction = 0.5f;
    float restitution = 0.0f;
    float restitution_threshold = 0.5f;

    b2ShapeId shape = b2_nullShapeId;
    
    std::function<void(void)> distruct_function;

    ~BoxColliderComponent()
    {
   //   distruct_function();
    }
  };

  struct REAL_API CircleColliderComponent
  {
    Vec2 offset = {0.0f, 0.0f};
    float radius = 0.5f;
 
    float density = 1.0f;
    float friction = 0.5f;
    float restitution = 0.0f;
    float restitution_threshold = 0.5f;

    b2ShapeId shape = b2_nullShapeId;
    std::function<void(void)> distruct_function;

    ~CircleColliderComponent()
    {
   //   distruct_function();
    }
  };
}
#endif //KR_COMPONENTS_H
