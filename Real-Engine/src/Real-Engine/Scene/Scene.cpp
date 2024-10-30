#include "pch.h"
#include "Scene.h"
#include "Entity.h"
#include "Components.h"
#include "Real-Engine/Core/App.h"
#include "Real-Engine/Render/Renderer2D/Renderer2D.h"

#include "Real-Engine/Scripting/Script.h"
#include "Real-Engine/Scripting/Lua.h"

#include <box2d/box2d.h>


namespace Real
{
  Scene::Scene(const std::string& name)
    : m_name(name), m_id(UUID())
  {
  }

  Scene::Scene(const Scene& other)
    : m_name(other.m_name), m_id(other.m_id), m_viewport_width(other.m_viewport_width), m_viewport_height(other.m_viewport_height)
  {
    m_fast_access.clear();
    m_fast_access.reserve(other.m_fast_access.size());
    //TODO: template
    //                          old / new
    std::unordered_map<entt::entity, entt::entity> entt_ids;
    auto& cpyreg = other.m_registry;
    {
      auto view = cpyreg.view<IDComponent>();
      for(auto& e : view)
      {
        auto en = this->addEntity();
        const auto& new_uuid = en.insertComponent<IDComponent>(view.get<IDComponent>(e)).ID;
        en.insertComponent<TagComponent>(cpyreg.get<TagComponent>(e));
                                            //id
        m_fast_access[(uint64_t)new_uuid] = { en, this };
        entt_ids[e] = en;
      }
    }
    {
      auto view = cpyreg.view<TransformComponent>();
      for(auto& e : view)
      {
        m_registry.emplace_or_replace<TransformComponent>(entt_ids.at(e), view.get<TransformComponent>(e));
      }
    }
    {
      auto view = cpyreg.view<CameraComponent>();
      for(auto& e : view)
      {
        m_registry.emplace_or_replace<CameraComponent>(entt_ids.at(e), view.get<CameraComponent>(e));
      }
    }
    {
      auto view = cpyreg.view<SpriteComponent>();
      for(auto& e : view)
      {
        m_registry.emplace_or_replace<SpriteComponent>(entt_ids.at(e), view.get<SpriteComponent>(e));
      }
    }
    {
      auto view = cpyreg.view<CircleComponent>();
      for(auto& e : view)
      {
        m_registry.emplace_or_replace<CircleComponent>(entt_ids.at(e), view.get<CircleComponent>(e));
      }
    }

    {
      auto view = cpyreg.view<NativeScriptComponent>();
      for(auto& e : view)
      {
        m_registry.emplace_or_replace<NativeScriptComponent>(entt_ids.at(e), view.get<NativeScriptComponent>(e));
      }
    }
    {
      auto view = cpyreg.view<RigidBody2DComponent>();
      for(auto& e : view)
      {
        m_registry.emplace_or_replace<RigidBody2DComponent>(entt_ids.at(e), view.get<RigidBody2DComponent>(e));
      }
    }
    {
      auto view = cpyreg.view<MovmentComponent>();
      for(auto& e : view)
      {
        m_registry.emplace_or_replace<MovmentComponent>(entt_ids.at(e), view.get<MovmentComponent>(e));
      }
    }
    {
      auto view = cpyreg.view<BoxColliderComponent>();
      for(auto& e : view)
      {
        m_registry.emplace_or_replace<BoxColliderComponent>(entt_ids.at(e), view.get<BoxColliderComponent>(e));
      }
    }
    {
      auto view = cpyreg.view<CircleColliderComponent>();
      for(auto& e : view)
      {
        m_registry.emplace_or_replace<CircleColliderComponent>(entt_ids.at(e), view.get<CircleColliderComponent>(e));
      }
    }
    {
      auto view = cpyreg.view<ScriptComponent>();
      for(auto& e : view)
      {
        m_registry.emplace_or_replace<ScriptComponent>(entt_ids.at(e), view.get<ScriptComponent>(e));
      }
    }
  }

  ARef<Scene> Scene::copy(const ARef<Scene>& other)
  {
    Scene* s = new Scene(*other.get());
    auto scene = createARef<Scene>();
    scene.reset(s);
    return scene;
  }

  Entity Scene::addEntity(const std::string& tag)
  {
    Entity e(m_registry.create(), this);
    const auto& id = e.addComponent<IDComponent>().ID;
    e.addComponent<TransformComponent>();
    if(tag.empty())
      e.addComponent<TagComponent>("Entity");
    else 
      e.addComponent<TagComponent>(tag);
    m_fast_access[(uint64_t)id] = { e, this };
    return e;
  }
 
  Entity Scene::addEntity(UUID uuid, const std::string& tag)
  {
    auto e = addEntity(tag);
    e.getComponent<IDComponent>().ID = uuid;
    m_fast_access[(uint64_t)uuid] = { e, this };
    return e;
  }

  Entity Scene::copyEntity(Entity& e)
  {
    auto eid = m_registry.create();
    Entity en(e, eid, this);
    m_fast_access[(uint64_t)en.getComponent<IDComponent>().ID] = { en, this };
    return en;
  }

  void Scene::removeEntity(const Entity& e)
  {
    const auto& iter = m_fast_access.find((uint64_t)e.getComponent<IDComponent>().ID);
    m_fast_access.erase(iter);
    m_registry.destroy(e);
  }

  void Scene::clear()
  {
    m_registry.clear();
    m_fast_access.clear();
  }

  //for now it loads every thing it should load only changed scripts
  //this should be in the file watcher
  void Scene::onLoad()
  {
    auto& lua = Lua::get();
    m_registry.view<ScriptComponent>().each([&](auto e, ScriptComponent& script)
    {
      auto script_ptr = AssetManager::get<AssetManager::ScriptAsset>(script.script_handle);
      if(script_ptr->is_valid)
      {
        lua.safe_script_file(script_ptr->meta.path);
        auto res = lua["GetObject"];
        if(res.valid())
        {
          sol::function fun = res;
          script_ptr->script = fun();
          //std::cout << "from entt::registry: id: " << uint32_t(e)<<std::endl;
          script_ptr->script->entity = { e, this };
        }
        else 
        {
          sol::error e = res;
          std::cerr << e.what();
        }
      }
    });
  }

  void Scene::onStart()
  {
    // TODO: physics settings
    b2Vec2 gravity = {0.0f, -9.8f};
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = gravity;
    m_physics_world = b2CreateWorld(&worldDef);

    //scripts might add physics components
    m_registry.view<NativeScriptComponent>().each([&](auto e, auto& native_script)
    {
      native_script.instance = native_script.instantiateScript();
      native_script.instance->m_entity = Entity(e, this);
      native_script.instance->onCreate();
    });

   // auto& lua = Lua::get();
    m_registry.view<ScriptComponent>().each([&](auto e, ScriptComponent& script)
    {
      //lua.safe_script_file(script.path);
      //auto res = lua["GetObject"];
      //if(res.valid())
      //{
      //  sol::function fun = res;
      //  script.script = fun();
      //  std::cout << "from entt::registry: id: " << uint32_t(e)<<std::endl; 
      //  script.script->entity = { e, this };
      //  std::cout << "inside Real-Engine::Entity::m_id: " << (uint32_t)script.script->entity << std::endl;
      //  std::cout << "calling onCreate from c++ << ";
    
      
      auto script_ptr = AssetManager::get<AssetManager::ScriptAsset>(script.script_handle);
      ScriptErrorEvent err(createARef<ScriptError>(script_ptr->err));
      EventDispatcher dp(err);
      dp.dispatch<ScriptErrorEvent>([&](auto& ev){App::get()->onEvent(ev); return false;});
      if(script_ptr->is_valid)
      {
        script_ptr->script->entity = { e, this }; 
        script_ptr->script->onCreate();
      }
     //   std::cout << " >> " << std::endl;
     // }
     // else 
     // {
       // sol::error e = res;
       // std::cerr << e.what();
     // }
    });


    m_registry.view<TransformComponent, RigidBody2DComponent>().each([&](auto e, TransformComponent& tc, RigidBody2DComponent& rb2dc)
    {
      Entity entity(e, this);
      
      b2BodyDef def = b2DefaultBodyDef();
      def.type = b2_dynamicBody;
      def.type = (b2BodyType)rb2dc.type;
      def.position = { tc.position.x, tc.position.y };
      def.rotation.s = std::sin(tc.rotation.z);
      def.rotation.c = std::cos(tc.rotation.z);
      def.fixedRotation = rb2dc.fixed_rotation;
      b2BodyId body = b2CreateBody(m_physics_world, &def);
      
      rb2dc.body = body;
      rb2dc.distruct_function = [&rb2dc](){ b2DestroyBody(rb2dc.body); };

      auto* bcc = entity.tryGetComponent<BoxColliderComponent>();
      if(bcc)
      {
        b2Polygon poly;
        poly = b2MakeBox(bcc->size.y * tc.scale.x, bcc->size.x * tc.scale.y);
        
        b2ShapeDef shape_def = b2DefaultShapeDef();
        shape_def.density = bcc->density;
        shape_def.friction = bcc->friction;
        shape_def.restitution = bcc->restitution;
        b2ShapeId shape_id = b2CreatePolygonShape(body, &shape_def, &poly);
        bcc->shape = shape_id;
        bcc->distruct_function = [&bcc](){ b2DestroyShape(bcc->shape, false); };
      }
      auto* ccc = entity.tryGetComponent<CircleColliderComponent>();
      if(ccc)
      {
        b2Circle circle;
        circle.radius = ccc->radius;
      
        b2ShapeDef shape_def = b2DefaultShapeDef();
        shape_def.density = ccc->density;
        shape_def.friction = ccc->friction;
        shape_def.restitution = ccc->restitution;
        b2ShapeId shape_id = b2CreateCircleShape(body, &shape_def, &circle);
        ccc->shape = shape_id;
        ccc->distruct_function = [&ccc](){ b2DestroyShape(ccc->shape, false); };
      }
    });
  }

  void Scene::onUpdate(Timestep ts)
  {
    REAL_PROFILE_FUNCTION();
    m_registry.view<NativeScriptComponent>().each([&](auto e, auto& native_script)
    {
      native_script.instance->onUpdate(ts);
    });

    {
    REAL_PROFILE_SCOPE("Script => onUpdate()");
    m_registry.view<ScriptComponent>().each([&](auto e, auto& script)
    {
      const auto& script_ptr = AssetManager::get<AssetManager::ScriptAsset>(script.script_handle);
      if(script_ptr->is_valid)
      {
        script_ptr->script->entity = { e, this };
        script_ptr->script->timestep = ts;
        REAL_CORE_ERROR("entity id : {0}", (uint32_t)e);
        script_ptr->script->onUpdate();
      }
    });
    }
    {
    REAL_PROFILE_SCOPE("Updating physics params");
    m_registry.view<MovmentComponent, RigidBody2DComponent>().each([&](auto e, MovmentComponent& mc, RigidBody2DComponent& rb2dc)
    {
      b2Body_SetTransform(rb2dc.body, { mc.new_transform.x, mc.new_transform.y }, { std::sin(mc.new_angle), std::cos(mc.new_angle)});
    });
    
    m_registry.view<LinearImpulseComponent, RigidBody2DComponent>().each([&](auto e, auto& lic, auto& rb2dc)
    {
      float E = 0.01;
      if((lic.point.x - E > 0  && lic.point.x + E < 0) || (lic.point.y - E > 0  && lic.point.y + E < 0))
        b2Body_ApplyLinearImpulse(rb2dc.body, {lic.magnitude.x, lic.magnitude.y}, b2Vec2(lic.point.x, lic.point.y), true);
      else [[likely]]
        b2Body_ApplyLinearImpulseToCenter(rb2dc.body, {lic.magnitude.x, lic.magnitude.y}, true);
    });

    m_registry.view<ForceComponent, RigidBody2DComponent>().each([&](auto e, auto& fc, auto& rb2dc)
    {
      float E = 0.01;
      if((fc.point.x - E > 0  && fc.point.x + E < 0) || (fc.point.y - E > 0  && fc.point.y + E < 0))
        b2Body_ApplyForce(rb2dc.body, {fc.magnitude.x, fc.magnitude.y}, b2Vec2(fc.point.x, fc.point.y), true);
      else [[likely]]
        b2Body_ApplyForceToCenter(rb2dc.body, {fc.magnitude.x, fc.magnitude.y}, true);
    });

    m_registry.view<AngularImpulseComponent, RigidBody2DComponent>().each([&](auto e, auto& aic, auto& rb2dc)
    {
      b2Body_ApplyAngularImpulse(rb2dc.body, aic.magnitude, true);
    });

    m_registry.view<TorqueComponent, RigidBody2DComponent>().each([&](auto e, auto& tc, auto& rb2dc)
    {
      b2Body_ApplyTorque(rb2dc.body, tc.magnitude, true);
    });
    }
    
    {
      REAL_PROFILE_SCOPE("physics => Step");
      b2World_Step(m_physics_world, ts, 4);
    }
    {
      REAL_PROFILE_SCOPE("retriving physics data");
    m_registry.view<TransformComponent, RigidBody2DComponent>().each([&](auto e, TransformComponent& tc, RigidBody2DComponent& rb2dc)
    {
      b2Body_SetGravityScale(rb2dc.body, rb2dc.gravity_scale);
      auto position = b2Body_GetPosition(rb2dc.body);
      tc.position = { position.x, position.y, tc.position.z };
      tc.rotation.z = std::asin(b2Body_GetRotation(rb2dc.body).s);
    });
    }
    SceneCamera* camera = nullptr;
    Mat4 camera_trans;
    {
      REAL_PROFILE_SCOPE("Camera => onUpdate");
      auto view = m_registry.view<TransformComponent, CameraComponent>();
      for(const auto& e : view)
      {
        auto &&[trans, cam] = view.get<TransformComponent, CameraComponent>(e);
        if(cam.is_primary)
        {
          if(!cam.is_fixed_aspect_ratio)
            cam.camera.setViewport(m_viewport_width, m_viewport_height);
          cam.camera.onUpdate();
          camera_trans = trans.getTransformationMatrix();
          camera = &cam.camera;
          break;
        }
      }
    }
    

    if(camera) [[likely]]
    {
      Renderer2D::beginScene(*camera, camera_trans);
      
      //Quads
      {
        REAL_PROFILE_SCOPE("Render Quads");
        auto view = m_registry.view<TransformComponent, SpriteComponent>();
        for(const auto& e : view)
        {
          auto &&[trans, sprite] = view.get<TransformComponent, SpriteComponent>(e);
          if(sprite.texture_handle)
          {
            Renderer2D::drawQuad(trans.getTransformationMatrix(),
              AssetManager::get<AssetManager::Texture2DAsset>(sprite.texture_handle)->texture, sprite.color, (int)e);
          }
          else
            Renderer2D::drawQuad(trans.getTransformationMatrix(), sprite.color, (int)e);
        }
      }

      //Circles 
      {
        auto view = m_registry.view<TransformComponent, CircleComponent>();
        for(const auto& e : view)
        {
          auto &&[trans, circle] = view.get<TransformComponent, CircleComponent>(e);
          Renderer2D::drawCircle(trans.getTransformationMatrix(), circle.thickness, circle.blur, circle.color, (int)e);
        }
      }
      Renderer2D::endScene();
    }
    else REAL_CORE_WARN("No main Camera found in Scene");//TODO: make it use editor camera
  }

  void Scene::onEditorUpdate(Timestep ts)
  {
    /*SceneCamera* camera = nullptr;
    Mat4 camera_trans;
    {
      auto view = m_registry.view<TransformComponent, CameraComponent>();
      for(const auto& e : view)
      {
        auto &&[trans, cam] = view.get<TransformComponent, CameraComponent>(e);
        if(cam.is_primary)
        {
          if(!cam.is_fixed_aspect_ratio)
            cam.camera.setViewport(m_viewport_width, m_viewport_height);
          cam.camera.onUpdate();
          camera_trans = trans.getTransformationMatrix();
          camera = &cam.camera;
          break;
        }
      }
    }
    if(camera) [[likely]]
    {
      Renderer2D::beginScene(*camera, camera_trans);
      auto view = m_registry.view<TransformComponent, SpriteComponent>();
      for(const auto& e : view)
      {
        auto &&[trans, sprite] = view.get<TransformComponent, SpriteComponent>(e);
        if(!sprite.texture_handel.empty())
        {
          Renderer2D::drawQuad(trans.getTransformationMatrix(),
              App::get()->assetManager().getTexture2D(sprite.texture_handel), sprite.color);
        }
        else
          Renderer2D::drawQuad(trans.getTransformationMatrix(), sprite.color);
      }
      Renderer2D::endScene();
    }
    else REAL_CORE_WARN("No main Camera found in Scene");
*/
    Renderer2D::beginScene(m_editor_camera.projection, m_editor_camera.view);
   
    //Quads
    {
      auto view = m_registry.view<TransformComponent, SpriteComponent>();
      for(const auto& e : view)
      {
        auto &&[trans, sprite] = view.get<TransformComponent, SpriteComponent>(e);
        if(sprite.texture_handle)
        {
          Renderer2D::drawQuad(trans.getTransformationMatrix(),
          AssetManager::get<AssetManager::Texture2DAsset>(sprite.texture_handle)->texture, sprite.color, (int)e);
        }
        else
          Renderer2D::drawQuad(trans.getTransformationMatrix(), sprite.color, (int)e);
      }
    }

    
    //Circles 
    {
      auto view = m_registry.view<TransformComponent, CircleComponent>();
      for(const auto& e : view)
      {
        auto &&[trans, circle] = view.get<TransformComponent, CircleComponent>(e);
        Renderer2D::drawCircle(trans.getTransformationMatrix(), circle.thickness, circle.blur, circle.color, (int)e);
      }
    }
    Renderer2D::endScene();
}

  void Scene::onViewportResize(uint32_t w, uint32_t h)
  {
    m_viewport_width = w;
    m_viewport_height = h;
  }
  
  Entity Scene::getEntity(const std::string& n)
  {
    auto view = m_registry.view<TagComponent>();
    for(const auto& e : view)
    {
      if(view.get<TagComponent>(e).name == n)
        return Entity(e, this);
    }
    return {};
  }
  
  Entity Scene::getEntity(UUID id)
  {
    if(m_fast_access.find((uint64_t)id) != m_fast_access.end())
    {
      const auto& pair = m_fast_access.at((uint64_t)id);
      return { pair.first, pair.second };
    }
    REAL_CORE_WARN("Entity with UUID: {0} Not found", id);
    return {};
  }

  void Scene::onEnd()
  {
    m_registry.view<NativeScriptComponent>().each([&](auto e, auto& native_script)
    {
      native_script.instance->onDestroy();
    });
    
    m_registry.view<ScriptComponent>().each([&](auto e, auto& script_comp)
    {
      //script.script.entity = { e, this };
      auto script_asset = AssetManager::get<AssetManager::ScriptAsset>(script_comp.script_handle);
      if(script_asset->is_valid)
        script_asset->script->onDestroy();
    });

    b2DestroyWorld(m_physics_world);
    m_physics_world = b2_nullWorldId;
  }
  
  bool operator==(const ARef<Scene>& lhs, const ARef<Scene>& rhs)
  {
    return lhs->getUUID() == rhs->getUUID();
  }
  
bool operator!=(const ARef<Scene>& lhs, const ARef<Scene>& rhs)
  {
    return lhs->getUUID() != rhs->getUUID();
  }

}
