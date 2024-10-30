#include "SceneSerializer.h"
#include "Components.h"
#include "Real-Engine/Core/UUID.h"
#include "yaml-cpp/emitter.h"
#include "yaml-cpp/emittermanip.h"


namespace YAML
{
  template<>
  struct convert<Real::Vec4>
  {
    static Node encode(const Real::Vec4& vec)
    {
      Node n;
      n.push_back(vec.x);
      n.push_back(vec.y);
      n.push_back(vec.z);
      n.push_back(vec.w);
      return n;
    }

    static bool decode(const Node& node, Real::Vec4& vec)
    {
      if(!node.IsSequence())
        return false;

      vec.x = node[0].as<float>();
      vec.y = node[1].as<float>();
      vec.z = node[2].as<float>();
      vec.w = node[3].as<float>();
      
      return true;
    }

  };

  template<>
  struct convert<Real::Vec3>
  {
    static Node encode(const Real::Vec3& vec)
    {
      Node n;
      n.push_back(vec.x);
      n.push_back(vec.y);
      n.push_back(vec.z);
      return n;
    }

    static bool decode(const Node& node, Real::Vec3& vec)
    {
      if(!node.IsSequence())
        return false;

      vec.x = node[0].as<float>();
      vec.y = node[1].as<float>();
      vec.z = node[2].as<float>();
      
      return true;
    }
  };

  template<>
  struct convert<Real::Vec2>
  {
    static Node encode(const Real::Vec2& vec)
    {
      Node n;
      n.push_back(vec.x);
        n.push_back(vec.y);
      return n;
    }

    static bool decode(const Node& node, Real::Vec2& vec)
    { 
      if(!node.IsSequence())
        return false;

      vec.x = node[0].as<float>();
      vec.y = node[1].as<float>();

      return true;
    }
  };

  Emitter& operator << (Emitter& emitter, const Real::Vec2& vec)
  {
    emitter << Flow;
    emitter << BeginSeq << vec.x << vec.y << EndSeq;
    return emitter;
  }

  Emitter& operator << (Emitter& emitter, const Real::Vec3& vec)
  {
    emitter << Flow;
    emitter << BeginSeq << vec.x << vec.y << vec.z << EndSeq;
    return emitter;
  }

  Emitter& operator << (Emitter& emitter, const Real::Vec4& vec)
  {
    emitter << Flow;
    emitter << BeginSeq << vec.x << vec.y << vec.z << vec.w << EndSeq;
    return emitter;
  }
}


namespace Real
{
  using namespace YAML;

  SceneSerializer::SceneSerializer(const ARef<Scene>& scene)
    : m_context(scene)
  {
  }

  void SceneSerializer::setContext(const ARef<Scene>& scene)
  {
    m_context = scene;
  }

  bool SceneSerializer::serializeText(const char* path)
  {
    Emitter emitter;
    emitter << BeginMap;
    
    emitter << Key << "Scene" << Value << m_context->m_name;

    emitter << Key << "Entities" << Value << BeginSeq;
    m_context->m_registry.view<TagComponent>().each([&](auto id, auto& tc)
    {
      Entity e(id, m_context.get());
      serializeEntity(e, emitter);
    });

    emitter << EndSeq << EndMap;

    std::ofstream fstream(path);

    fstream << emitter.c_str();

    return true;
  }

  bool SceneSerializer::deSerializeText(const char* path)
  {
    std::ifstream fstream(path);
    std::stringstream ss;
    if(!fstream.good())
    {
      REAL_CORE_ERROR("Can't Load File: {0}", path);
      return false;
    }
    ss << fstream.rdbuf();
    const auto& data = Load(ss.str().c_str());
    
    const auto& scene = data["Scene"];
    if(!scene)
    {
      REAL_CORE_ERROR("No Scene Node in Scene Serializeation File !?");
      return false;
    }
    std::string scene_name = scene.as<std::string>();
    m_context->clear();// = createARef<Scene>(scene_name);
    const auto& entities = data["Entities"];
    if(entities)
    {
      for(const auto& entity : entities)
      {
        UUID uuid = entity["UUID"].as<uint64_t>();
        std::string tag;
        const auto& ctagn = entity["TagComponent"];
        if(ctagn)
          tag = ctagn["Tag"].as<std::string>();
        
        Entity e = m_context->addEntity(uuid, tag);
        const auto& ctransform_n = entity["TransformComponent"]; 
        if(ctransform_n)
        {
          auto& ctransform = e.insertComponent<TransformComponent>();
          ctransform.position = ctransform_n["Position"].as<Vec3>();
          ctransform.rotation = ctransform_n["Rotation"].as<Vec3>();
          ctransform.scale = ctransform_n["Scale"].as<Vec3>();
        }
        const auto& csprite_n = entity["SpriteComponent"];
        if(csprite_n)
        {
          auto& csprite = e.addComponent<SpriteComponent>();
          const auto& color = csprite_n["Color"].as<Vec4>();
          csprite.color = color / 255.0f;
          const auto& texture_handel = csprite_n["TextureHandle"].as<uint64_t>();
          csprite.texture_handle = texture_handel;
        }
        const auto& ccamera_n = entity["CameraComponent"];
        if(ccamera_n)
        {
          auto& ccamera = e.addComponent<CameraComponent>();
          auto& cam = ccamera.camera;
          const auto& pd_n = ccamera_n["PerspectiveData"];
          float p_fov = pd_n["FOV"].as<float>();
          float p_near_clip = pd_n["NearClip"].as<float>();
          float p_far_clip = pd_n["FarClip"].as<float>();
          cam.setPerspectiveData({p_fov, p_near_clip, p_far_clip});
          const auto& od_n = ccamera_n["OrthographicData"];
          float o_size = od_n["Size"].as<float>();
          float o_near_clip = od_n["NearClip"].as<float>();
          float o_far_clip = od_n["FarClip"].as<float>();
          cam.setOrthographicData({o_size, o_near_clip, o_far_clip});

          cam.setType((SceneCamera::ProjectionType)ccamera_n["Type"].as<int>());
          ccamera.is_primary = ccamera_n["Primary"].as<bool>();
          ccamera.is_fixed_aspect_ratio = ccamera_n["FixedAspectRatio"].as<bool>();
        }

        const auto& crb2d_n = entity["RigidBody2DComponent"];
        if(crb2d_n)
        {
          auto& rb2dc = e.addComponent<RigidBody2DComponent>();
          rb2dc.type = (RigidBody2DComponent::BodyType)crb2d_n["Type"].as<int>();
          rb2dc.fixed_rotation = crb2d_n["FixedRotatiin"].as<bool>();
        }

        const auto& cbc_n = entity["BoxColliderComponent"];
        if(cbc_n)
        {
          auto& bcc = e.addComponent<BoxColliderComponent>();
          bcc.size = cbc_n["Size"].as<Vec2>();
          bcc.offset = cbc_n["Offset"].as<Vec2>();
          bcc.density = cbc_n["Density"].as<float>();
          bcc.friction = cbc_n["Friction"].as<float>();
          bcc.restitution = cbc_n["Restitution"].as<float>();
          bcc.restitution_threshold = cbc_n["RestitutionThreshold"].as<float>();
        }

        const auto& ccc_n = entity["CircleColliderComponent"];
        if(ccc_n)
        {
          auto& ccc = e.addComponent<CircleColliderComponent>();
          ccc.radius = ccc_n["Radius"].as<float>();
          ccc.offset = ccc_n["Offset"].as<Vec2>();
          ccc.density = ccc_n["Density"].as<float>();
          ccc.friction = ccc_n["Friction"].as<float>();
          ccc.restitution = ccc_n["Restitution"].as<float>();
          ccc.restitution_threshold = ccc_n["RestitutionThreshold"].as<float>();
        }

        const auto& script_n = entity["ScriptComponent"];
        if(script_n)
        {
          auto& script = e.addComponent<ScriptComponent>();
          script.script_handle = script_n["ScriptHandle"].as<uint64_t>();
        }

      }
    }
    return true;
  }

  void SceneSerializer::serializeEntity(Entity& e, YAML::Emitter& emitter)
  {
    emitter << BeginMap;
    emitter << Key << "UUID" <<Value << e.getComponent<IDComponent>().ID;

    if(e.hasComponent<TagComponent>())
    {
      emitter << Key << "TagComponent" << BeginMap;
      const auto& tag = e.getComponent<TagComponent>().name;
      emitter << Key << "Tag" << Value << tag;

      emitter << EndMap;
    }

    if(e.hasComponent<TransformComponent>())
    {
      emitter << Key << "TransformComponent" << BeginMap;

      auto& trans = e.getComponent<TransformComponent>();

      emitter << Key << "Position" << trans.position;
      emitter << Key << "Rotation" << trans.rotation;
      emitter << Key << "Scale" << trans.scale;

      emitter << EndMap;
    }

    if(e.hasComponent<SpriteComponent>())
    {
      emitter << Key << "SpriteComponent" << BeginMap;
      
      auto& sprite = e.getComponent<SpriteComponent>();

      emitter << Key << "Color" << Value << sprite.color * 255.0f;
      emitter << Key << "TextureHandle" << Value << sprite.texture_handle;
      emitter << EndMap;
    }

    if(e.hasComponent<CameraComponent>())
    {
      emitter << Key << "CameraComponent" << BeginMap;
      
      auto& cam = e. getComponent<CameraComponent>();

      const auto& pd = cam.camera.getPerspectiveData();
      emitter << Key << "PerspectiveData" << Value << BeginMap;
      emitter << Key << "FOV" << Value << pd.fov;
      emitter << Key << "NearClip" << Value << pd.near_clip;
      emitter << Key << "FarClip" << Value << pd.far_clip;
      emitter << EndMap;

      const auto& od = cam.camera.getOrthographicData();
      emitter << Key << "OrthographicData" << Value << BeginMap;
      emitter << Key << "Size" << Value << od.size;
      emitter << Key << "NearClip" << Value << od.near_clip;
      emitter << Key << "FarClip" << Value << od.far_clip;
      emitter << EndMap;
  
      emitter << Key << "Type" << Value <<(int)cam.camera.getType();

      emitter << Key << "Primary" << Value << cam.is_primary;
      emitter << Key << "FixedAspectRatio" << cam.is_fixed_aspect_ratio;
      
      emitter << EndMap;
    }

    if(e.hasComponent<RigidBody2DComponent>())
    {
      emitter << Key << "RigidBody2DComponent" << BeginMap;
      auto& rb2dc = e.getComponent<RigidBody2DComponent>();
      
      emitter << Key << "Type" << (int)rb2dc.type;
      emitter << Key << "FixedRotatiin" << rb2dc.fixed_rotation;

      emitter << EndMap;
    }

    if(e.hasComponent<BoxColliderComponent>())
    {
      emitter << Key << "BoxColliderComponent" << BeginMap;
      
      auto& bcc = e.getComponent<BoxColliderComponent>();
      emitter << Key << "Size" << bcc.size;
      emitter << Key << "Offset" << bcc.offset;
      emitter << Key << "Density" << bcc.density;
      emitter << Key << "Friction" << bcc.friction;
      emitter << Key << "Restitution" << bcc.restitution;
      emitter << Key << "RestitutionThreshold" << bcc.restitution_threshold;
      emitter << EndMap;
    }

    if(e.hasComponent<BoxColliderComponent>())
    {
      emitter << Key << "CircleColliderComponent" << BeginMap;
      
      auto& ccc = e.getComponent<CircleColliderComponent>();
      emitter << Key << "Radius" << ccc.radius;
      emitter << Key << "Offset" << ccc.offset;
      emitter << Key << "Density" << ccc.density;
      emitter << Key << "Friction" << ccc.friction;
      emitter << Key << "Restitution" << ccc.restitution;
      emitter << Key << "RestitutionThreshold" << ccc.restitution_threshold;
      emitter << EndMap;
    }

    if(e.hasComponent<ScriptComponent>())
    {
      emitter << Key << "ScriptComponent" << BeginMap;

      auto& script = e.getComponent<ScriptComponent>();
      emitter << Key << "ScriptHandle" << script.script_handle;
      emitter << EndMap;
    }

    emitter << EndMap;

  }

}
