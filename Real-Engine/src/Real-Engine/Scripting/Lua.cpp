#include <pch.h>
#include <sol/forward.hpp>
#include <sol/object.hpp>
#include <sol/overload.hpp>
#include <sol/raii.hpp>
#include <sol/resolve.hpp>
#include <sol/types.hpp>

#include "Lua.h"
#include "Real-Engine/Core/ButtonsAndKeyCodes.h"
#include "Real-Engine/Core/Input.h"
#include "Real-Engine/Core/Log.h"
#include "Real-Engine/Core/Core.h"
#include "Real-Engine/Core/Math/math.h"
#include "Real-Engine/Core/UUID.h"
#include "Real-Engine/Core/Window.h"
#include "Real-Engine/Render/Renderer2D/Renderer2D.h"
#include "Real-Engine/Scene/Components.h"
#include "Real-Engine/Scene/Entity.h"
#include "Real-Engine/Scene/SceneCamera.h"
#include "Real-Engine/Core/App.h"
#include "Script.h"


#define ADD_COMPONENT_FUNCTIONS(component_prefex) \
    auto add_##component_prefex        = &::Real::Entity::addComponent<component_prefex##Component>; \
    auto insert_##component_prefex     = &::Real::Entity::insertComponent<component_prefex##Component>; \
    auto get_##component_prefex        = &::Real::Entity::getComponent<component_prefex##Component>; \
    auto try_get_##component_prefex    = &::Real::Entity::tryGetComponent<component_prefex##Component>; \
    auto remove_##component_prefex     = &::Real::Entity::removeComponent<component_prefex##Component>; \
    auto try_remove_##component_prefex = &::Real::Entity::tryRemoveComponent<component_prefex##Component>; \

#define REGESTER_COMPONENT_FUNCTIONS(component_prefex, usertype_name) \
  {\
    usertype_name[STRINGIFY(add##component_prefex)]       = add_##component_prefex; \
    usertype_name[STRINGIFY(insert##component_prefex)]    = insert_##component_prefex; \
    usertype_name[STRINGIFY(get##component_prefex)]       = get_##component_prefex; \
    usertype_name[STRINGIFY(tryGet##component_prefex)]    = try_get_##component_prefex; \
    usertype_name[STRINGIFY(remove##component_prefex)]    = remove_##component_prefex; \
    usertype_name[STRINGIFY(tryRemove##component_prefex)] = try_remove_##component_prefex; \
  }

namespace Real
{
  sol::state Lua::lua = sol::state{};
  sol::table Lua::real = sol::table{};
  
  std::string ScriptError::ECtoString(ScriptError::ErrorCode ec)
  {
    return sol::to_string(sol::call_status(ec));
  }

  std::string ScriptError::loadECtoString(ScriptError::LoadErrorCode lec)
  {
    return sol::to_string(sol::load_status(lec));
  }

  ScriptError::ErrorCode ScriptError::toEC(sol::call_status cs)
  {
    switch(cs)
    {
      case sol::call_status::ok      : return ErrorCode::ok;
      case sol::call_status::gc      : return ErrorCode::gc;
      case sol::call_status::file    : return ErrorCode::file;
      case sol::call_status::memory  : return ErrorCode::memory;
      case sol::call_status::syntax  : return ErrorCode::syntax;
      case sol::call_status::handler : return ErrorCode::handler;
      case sol::call_status::runtime : return ErrorCode::runtime;
      case sol::call_status::yielded : return ErrorCode::yielded;
    }
    return ErrorCode::ok;
  }

  ScriptError::LoadErrorCode ScriptError::toLEC(sol::load_status ls)
  {
    switch(ls)
    {
      case sol::load_status::ok      : return LoadErrorCode::ok;
      case sol::load_status::gc      : return LoadErrorCode::gc;
      case sol::load_status::file    : return LoadErrorCode::file;
      case sol::load_status::memory  : return LoadErrorCode::memory;
      case sol::load_status::syntax  : return LoadErrorCode::syntax;

    }
    return LoadErrorCode::ok;
  }

  void Lua::init()
  {
    lua.open_libraries();
    
    real = Lua::get()["Real"].get_or_create<sol::table>();
    REAL_CORE_WARN("Created Real table");
    
    REAL_CORE_WARN("initCore called");
    initCore();
    REAL_CORE_WARN("initCore exited");

    REAL_CORE_WARN("initMath exited");
    initMath();
    REAL_CORE_WARN("initMath exited");
    REAL_CORE_WARN("initRenderer called");
    initRenderer();
    REAL_CORE_WARN("initRenderer exited");
    initScene();

    initScripting();

#if 0
    real.new_usertype<Scene>("Scene", "addEntity", sol::resolve<Entity(const std::string&)>(&Scene::addEntity), 
        "clear", &Scene::clear, "removeEntity", &Scene::removeEntity);

    real.new_usertype<TransformComponent>("TransformComponent",
        "position", &TransformComponent::position, "rotation", &TransformComponent::rotation, 
        "scale", &TransformComponent::scale);
    real.new_usertype<SpriteComponent>("SpriteComponent", 
        "color", &SpriteComponent::color, "handle", &SpriteComponent::texture_handel);
    
/*    real.new_usertype<RigidBody2DComponent>("RigidBody2DComponent", "fixed_rotation", &RigidBody2DComponent::fixed_rotation, "body_type", &RigidBody2DComponent::type, 
        //Bodyype Enum
        "static",  RigidBody2DComponent::BodyType::Static,
        "dynamic", RigidBody2DComponent::BodyType::Dynamic,
        "kinematic", RigidBody2DComponent::BodyType::Kinematic);
  */  
    auto transform_func = &Entity::insertComponent<TransformComponent>;
    auto sprite_func = &Entity::insertComponent<SpriteComponent>;

    auto get_transform = &Entity::getComponent<TransformComponent>;
    auto get_sprite = &Entity::getComponent<SpriteComponent>;


    real.new_usertype<Entity>("Entity", "insertTransform", transform_func, "insertSprite", sprite_func, "getTransform", get_transform, "getSprite", get_sprite, "getId", &Entity::operator unsigned int, 
        "getUUID", &Entity::operator unsigned long);
    //real.set_function<void(const Mat4&, const Vec4&)>("drawQuad", &Renderer2D::drawQuad);

    //real.set_function("getScene", [&](const char* name){return App::get()->assetManager().getScene(name);});
#endif
        /*
    lua.script(R"(
    app = App.get();
    app:print();
    )");


	lua.safe_script(R"(
  script = Script.new()
  function on_create() 
    print("created which idk how");
    end
  script.onCreate = on_create();
  function GetObject()
    return script
  end
)");
  //auto res = lua["script"];
  auto res = lua["GetObject"];
  if(res.valid())
  {
    sol::function fun = res;
    Script s = fun();
    std::cout << "Calling from c++\n";
    //s.onCreate();
    std::cout << "Called from c++\n";
  }
  else 
  {
    sol::error e = res;
    std::cerr << e.what();
  }
      //sol::function fun = m_state["d2.run"];
//fun(20);
*/
  }

  ScriptError Lua::runScript(const std::string& path)
  {
    ScriptError e;
    sol::protected_function_result result
		     = lua.script_file(path, sol::script_pass_on_error);
		if(!result.valid()) 
    {
			sol::error err = result;
			sol::call_status status = result.status();
			e.status = ScriptError::toEC(status);
			e.error_message = err.what();
      e.load_status = ScriptError::LoadErrorCode::ok;
      e.load_error_message = "none";
		} return e; 

  
    /*
    ScriptError sc_err;

    sol::load_result loaded_chunk = lua.load_file(path);
		//SOL_ASSERT(!loaded_chunk.valid());
		if(!loaded_chunk.valid()) 
    {
			sol::error err = loaded_chunk;
			sol::load_status status = loaded_chunk.status();
		  sc_err.load_status = ScriptError::toLEC(status);
      sc_err.load_error_message = err.what();
    }
		else 
    {
			// Because the syntax is bad, this will never be
			// reached
			//SOL_ASSERT(false);
			// If there is a runtime error (lua GC memory
			// error, nil access, etc.) it will be caught here
			sol::protected_function script_func = loaded_chunk.get<sol::protected_function>();
			sol::protected_function_result result = script_func();
			if(!result.valid()) 
      {
				sol::error err = result;
				sol::call_status status = result.status();
			  sc_err.status = ScriptError::toEC(status);
        sc_err.error_message = err.what();
      }
		}
    return sc_err;
  */
  }

  void Lua::initCore()
  {
    //App:
    {
      //usertype
      sol::usertype<App> app = real.new_usertype<App>("App");
      app["print"] = [](){ App::get()->print(); };
      //app["getWindow"] = [](){ return App::get()->getWindow(); };
    }

    using k = Keyboard;
    //Keyboard & Mouse Codes
    {
      real.new_enum("Keyboard", "Space", k::Space, "Apostrophe", k::Apostrophe, "Comma", k::Comma, 
      "Minus", k::Minus, "Period", k::Period, "Slash", k::Slash, "Zero", k::Zero, "One", k::One, 
      "Two", k::Two, "Three", k::Three, "Four", k::Four, "Five", k::Five, "Six", k::Six, 
      "Seven", k::Seven, "Eight", k::Eight, "Nine", k::Nine, "Semicolon", k::Semicolon,
      "Equal", k::Equal, "A", k::A, "B", k::B, "C", k::C, "D", k::D, "E", k::E, "F", k::F, "G", k::G,
      "H", k::H, "I", k::I, "J", k::J, "K", k::K, "L", k::L, "M", k::M, "N", k::N, "O", k::O, "P", k::P,
      "Q", k::Q, "R", k::R, "S", k::S, "T", k::T, "U", k::U, "V", k::V, "W", k::W, "X", k::X, "Y", k::Y,
      "Z", k::Z, "LeftBracket", k::LeftBracket, "BackSlash", k::BackSlash, "RightBracket", k::RightBracket, 
      "Accent", k::Accent, "World1", k::World1, "World2", k::World2, "Escape", k::Escape, "Enter", k::Enter,
      "Tab", k::Tab, "BackSpace", k::BackSpace, "Insert", k::Insert, "Delete", k::Delete, "Right", k::Right, 
      "Left", k::Left, "Down", k::Down, "Up", k::Up, "PageUp", k::PageUp, "PageDown", k::PageDown, 
      "Home", k::Home, "End", k::End, "CapsLock", k::CapsLock, "ScrollLock", k::ScrollLock,
      "NumLock", k::NumLock, "F1", k::F1, "F2", k::F2, "F3", k::F3, "F4", k::F4, "F5", k::F5, "F6", k::F6,
      "F7", k::F7, "F8", k::F8, "F9", k::F9, "F10", k::F10, "F11", k::F11, "F12", k::F12, "F13", k::F13,
      "F14", k::F14, "F15", k::F15, "F16", k::F16, "F17", k::F17, "F18", k::F18, "F19", k::F19, "F20", k::F20, 
      "F21", k::F21, "F22", k::F22, "F23", k::F23, "F24", k::F24, "F25", k::F25, "NumPad0", k::NumPad0, 
      "NumPad1", k::NumPad1, "NumPad2", k::NumPad2, "NumPad3", k::NumPad3, "NumPad4", k::NumPad4, 
      "NumPad5", k::NumPad5, "NumPad6", k::NumPad6, "NumPad7", k::NumPad7, "NumPad8", k::NumPad8, 
      "NumPad9", k::NumPad9, "NumPadDecimal", k::NumPadDecimal, "NumPadDivide", k::NumPadDivide,
      "NumPadMultiply", k::NumPadMultiply, "NumPadSubtract", k::NumPadSubtract, "NumPadAdd", k::NumPadAdd,
      "NumPadEnter", k::NumPadEnter, "NumPadEqual", k::NumPadEqual, "LeftShift", k::LeftShift,
      "LeftControl", k::LeftControl, "LeftAlt", k::LeftAlt, "LeftSuper", k::LeftSuper,
      "RightShift", k::RightShift, "RightControl", k::RightControl, "RightAlt", k::RightAlt,
      "RightSuper", k::RightSuper
      );

      real.new_enum("Mouse", 
      "ButtonLeft", Mouse::ButtonLeft, "ButtonRight", Mouse::ButtonRight, "ButtonMiddel", Mouse::ButtonMiddle,
      "Button4", Mouse::Button4, "Button5", Mouse::Button5, "Button6", Mouse::ButtonLeft, 
      "Button7", Mouse::Button7, "Button8", Mouse::Button8
          );

    }

    //Input
    {
      sol::usertype<Input> input = real.new_usertype<Input>("Input");
      input["isKeyPressed"]          = [](int k) { return Input::isKeyPressed((Keyboard)k);       };
      input["isKeyReleased"]         = [](int k) { return Input::isKeyReleased((Keyboard)k);      };
      input["isMouseButtonPressed"]  = [](int b) { return Input::isMouseButtonPressed((Mouse)b);  };
      input["isMouseButtonReleased"] = [](int b) { return Input::isMouseButtonReleased((Mouse)b); };
      input["getMousePosition"]      = &Input::getMousePos;
    }

    //Timestep:
    {
      //usertype
      sol::usertype<Timestep> ts = real.new_usertype<Timestep>("Timestep");
      ts["getTime"]               = &Timestep::getTime;
      ts["getTimeAsMilliSeconds"] = &Timestep::getTimeAsMilliSeconds;
    }

    //UUID
    {
      sol::usertype<UUID> uuid = real.new_usertype<UUID>("UUID", 
          sol::constructors<UUID(), UUID(const UUID&), UUID(uint64_t)>()
          );
      uuid["asNumber"] = &UUID::operator unsigned long;
    }

    //AssetManager:
    {
      sol::usertype<AssetManager> asset_manager = real.new_usertype<AssetManager>("AssetManager");
      asset_manager["get"] = sol::resolve<ARef<AssetManager::Asset>(UUID)>(&AssetManager::get);
      asset_manager["getTexture2D"] = sol::resolve<ARef<AssetManager::Texture2DAsset>(UUID)>(&AssetManager::get<AssetManager::Texture2DAsset>);
      asset_manager["getScene"] = sol::resolve<ARef<AssetManager::SceneAsset>(UUID)>(&AssetManager::get<AssetManager::SceneAsset>);
      asset_manager["getScript"] = sol::resolve<ARef<AssetManager::ScriptAsset>(UUID)>(&AssetManager::get<AssetManager::ScriptAsset>);
      asset_manager["getUUID"] = &AssetManager::getUUID;
      asset_manager["loadConfig"] = &AssetManager::loadConfig;
      asset_manager["load"] = sol::overload(
          [](const AssetManager::Asset::Meta& meta, UUID id) { return AssetManager::load(meta, id); }, 
          [](const AssetManager::Asset::Meta& meta) { return AssetManager::load(meta); });
      asset_manager["reload"]  = sol::resolve<bool(UUID)>(AssetManager::reload);
      asset_manager["clear"]   = &AssetManager::clear;
      asset_manager["remove"]  = &AssetManager::remove;
      asset_manager["getPath"] = &AssetManager::getPath;

      //Asset
      sol::usertype<AssetManager::Asset> asset = real.new_usertype<AssetManager::Asset>("Asset");
      asset["meta"]     = &AssetManager::Asset::meta;
      asset["onReload"] = &AssetManager::Asset::onReload;
      asset["is_valid"] = &AssetManager::Asset::is_valid;
      
      //Meta
      sol::usertype<AssetManager::Asset::Meta> meta = real.new_usertype<AssetManager::Asset::Meta>("Meta");
      meta["path"] = &AssetManager::Asset::Meta::path;
      meta["type"] = &AssetManager::Asset::Meta::type;
      
      //Type
      real.new_enum("AssetType", "None", AssetManager::Asset::Type::None,
          "Texture2D", AssetManager::Asset::Type::Texture2D, "Scene", AssetManager::Asset::Type::Scene, 
          "Script", AssetManager::Asset::Type::Script);
      
      //Texture2DAsset
      sol::usertype<AssetManager::Texture2DAsset> t2d_asset = 
        real.new_usertype<AssetManager::Texture2DAsset>("Texture2DAsset", 
            sol::base_classes, sol::bases<AssetManager::Asset>());
      t2d_asset["texture"] = &AssetManager::Texture2DAsset::texture;
      
      //SceneAsset
      sol::usertype<AssetManager::SceneAsset> scene_asset = 
        real.new_usertype<AssetManager::SceneAsset>("SceneAsset", 
            sol::base_classes, sol::bases<AssetManager::Asset>());
      scene_asset["scene"] = &AssetManager::SceneAsset::scene;
      
      //ScriptAsset
      sol::usertype<AssetManager::ScriptAsset> script_asset = 
        real.new_usertype<AssetManager::ScriptAsset>("ScriptAsset",
            sol::base_classes, sol::bases<AssetManager::Asset>());
      script_asset["script"] = &AssetManager::ScriptAsset::script;
    }

    //Window
    {
      sol::usertype<Window> window = real.new_usertype<Window>("Window");
      window["getWidth"]  = &Window::getWidth;
      window["getHieght"] = &Window::getHeight;
      window["isVsync"]   = &Window::isVsync;
      window["setVsync"]  = &Window::setVsync;
    }

  }
  //stpq
  void Lua::initMath()
  {
    //primitives
    {
      //Float
      sol::usertype<Float> f = real.new_usertype<Float>("Float", sol::constructors<Float(float)>());
      f["float"] = &Float::_float;

      //Int32
      sol::usertype<Int> i = real.new_usertype<Int>("Int", sol::constructors<Int(int32_t)>());
      i["int"] = &Int::_int;

      //String 
      sol::usertype<String> s = real.new_usertype<String>("String", sol::constructors<String(const char*)>());
      s["string"] = &String::_string;

    }

    //Vec4
    {
      //usertype
      sol::usertype<Vec4> v4 = real.new_usertype<Vec4>("Vec4", 
          sol::constructors<Vec4(), Vec4(float), Vec4(float, float, float, float)>());

      v4["x"] = &Vec4::x; v4["r"] = &Vec4::r; v4["s"] = &Vec4::s;
      v4["y"] = &Vec4::y; v4["g"] = &Vec4::g; v4["t"] = &Vec4::t;
      v4["z"] = &Vec4::z; v4["b"] = &Vec4::b; v4["p"] = &Vec4::p;
      v4["w"] = &Vec4::w; v4["a"] = &Vec4::a; v4["q"] = &Vec4::q;
      
      v4["len"] = &Vec4::length;

      v4["add"] = sol::resolve<Vec4(const Vec4&, const Vec4&)>(&glm::operator+);
      v4["sub"] = sol::resolve<Vec4(const Vec4&, const Vec4&)>(glm::operator-);
      v4["mul"] = sol::overload(
          sol::resolve<Vec4(const Vec4&, const Vec4&)>(glm::operator*), 
          sol::resolve<Vec4(const Vec4&, float)>(glm::operator*)
          );
      v4["dev"] = sol::overload(
          sol::resolve<Vec4(const Vec4&, const Vec4&)>(glm::operator/),
          sol::resolve<Vec4(const Vec4&, float)>(glm::operator/)
          );

    }

    //Vec3
    {
      //usertype
      sol::usertype<Vec3> v3 = real.new_usertype<Vec3>("Vec3", 
          sol::constructors<Vec3(), Vec3(float), Vec3(float, float, float)>());
      
      v3["x"] = &Vec3::x; v3["r"] = &Vec3::r; v3["s"] = &Vec3::s;
      v3["y"] = &Vec3::y; v3["g"] = &Vec3::g; v3["t"] = &Vec3::t;
      v3["z"] = &Vec3::z; v3["b"] = &Vec3::b; v3["p"] = &Vec3::p;
      
      v3["len"] = &Vec3::length;

      v3["add"] = sol::resolve<Vec3(const Vec3&, const Vec3&)>(&glm::operator+);
      v3["sub"] = sol::resolve<Vec3(const Vec3&, const Vec3&)>(glm::operator-);
      v3["mul"] = sol::overload(
          sol::resolve<Vec3(const Vec3&, const Vec3&)>(glm::operator*), 
          sol::resolve<Vec3(const Vec3&, float)>(glm::operator*)
          );
      v3["dev"] = sol::overload(
          sol::resolve<Vec3(const Vec3&, const Vec3&)>(glm::operator/),
          sol::resolve<Vec3(const Vec3&, float)>(glm::operator/)
          );
    }

    //Vec2
    {
      //usertype
      sol::usertype<Vec2> v2 = real.new_usertype<Vec2>("Vec2", 
      sol::constructors<Vec2(), Vec2(float), Vec2(float, float)>());

      v2["x"] = &Vec2::x; v2["r"] = &Vec2::r; v2["s"] = &Vec2::s;
      v2["y"] = &Vec2::y; v2["g"] = &Vec2::g; v2["t"] = &Vec2::t;
      
      v2["len"] = &Vec2::length;

      v2["add"] = sol::resolve<Vec2(const Vec2&, const Vec2&)>(&glm::operator+);
      v2["sub"] = sol::resolve<Vec2(const Vec2&, const Vec2&)>(glm::operator-);
      v2["mul"] = sol::overload(
          sol::resolve<Vec2(const Vec2&, const Vec2&)>(glm::operator*), 
          sol::resolve<Vec2(const Vec2&, float)>(glm::operator*)
          );
      v2["dev"] = sol::overload(
          sol::resolve<Vec2(const Vec2&, const Vec2&)>(glm::operator/),
          sol::resolve<Vec2(const Vec2&, float)>(glm::operator/)
          );
    }

    //Mat4
    {
      sol::usertype<Mat4> m4 = real.new_usertype<Mat4>("Mat4", sol::constructors<Mat4(), Mat4(float)>());

      m4["add"] = sol::resolve<Mat4(const Mat4&, const Mat4&)>(&glm::operator+);
      m4["sub"] = sol::resolve<Mat4(const Mat4&, const Mat4&)>(&glm::operator-);
      m4["mul"] = sol::overload(
          sol::resolve<Mat4(const Mat4&, const Mat4&)>(&glm::operator*),
          sol::resolve<Vec4(const Mat4&, const Vec4&)>(&glm::operator*)
          );
      m4["div"] = sol::overload(
          sol::resolve<Mat4(const Mat4&, const Mat4&)>(&glm::operator/),
          sol::resolve<Vec4(const Mat4&, const Vec4&)>(&glm::operator/)
          );
    
      m4["transpose"] = [](Mat4* _this){ *_this = glm::transpose(*_this); };
      m4["inverse"]   = [](Mat4* _this){ *_this = glm::inverse(*_this); };

      m4["transposed"] = &glm::transpose<4, 4, glm::f32, glm::defaultp>;
      m4["inversed"]   = &glm::inverse<4, 4, glm::f32, glm::defaultp>;
      
      m4["translate"] = [](Mat4* _this, const Vec3& vec) {
        *_this = glm::translate(*_this, vec); 
        return _this;
      };
      m4["rotate"]    = [](Mat4* _this, float angle, const Vec3& vec) {
        *_this = glm::rotate(*_this, angle, vec); 
        return _this;
      };
      m4["scale"]     = [](Mat4* _this, const Vec3& vec) { 
        *_this = glm::translate(*_this, vec); 
        return _this;
      };
      m4["transform"] = [](Mat4* _this, float angle, const Vec3& translation, 
          const Vec3& scale, const Vec3& rotation) {
        *_this = glm::translate(*_this, translation);
        *_this = glm::rotate(*_this, angle, rotation);
        *_this = glm::scale(*_this, scale);
        return _this;
      };
      
      m4["decompose"] = &Real::decompose;
    }

    //Mat3
    {
      sol::usertype<Mat3> m3 = real.new_usertype<Mat3>("Mat3", sol::constructors<Mat3(), Mat3(float)>());

      m3["add"] = sol::resolve<Mat3(const Mat3&, const Mat3&)>(&glm::operator+);
      m3["sub"] = sol::resolve<Mat3(const Mat3&, const Mat3&)>(&glm::operator-);
      m3["mul"] = sol::overload(
          sol::resolve<Mat3(const Mat3&, const Mat3&)>(&glm::operator*),
          sol::resolve<Vec3(const Mat3&, const Vec3&)>(&glm::operator*)
          );
      m3["div"] = sol::overload(
          sol::resolve<Mat3(const Mat3&, const Mat3&)>(&glm::operator/),
          sol::resolve<Vec3(const Mat3&, const Vec3&)>(&glm::operator/)
          );
    
      m3["transpose"] = [](Mat3* _this){ *_this = glm::transpose(*_this); return _this; };
      m3["inverse"]   = [](Mat3* _this){ *_this = glm::inverse(*_this); return _this ;};

      m3["transposed"] = &glm::transpose<3, 3, glm::f32, glm::defaultp>;
      m3["inversed"]   = &glm::inverse<3, 3, glm::f32, glm::defaultp>;
    }

    //Mat2
    {
      sol::usertype<Mat2> m2 = real.new_usertype<Mat2>("Mat2", sol::constructors<Mat2(), Mat2(float)>());

      m2["add"] = sol::resolve<Mat2(const Mat2&, const Mat2&)>(&glm::operator+);
      m2["sub"] = sol::resolve<Mat2(const Mat2&, const Mat2&)>(&glm::operator-);
      m2["mul"] = sol::overload(
          sol::resolve<Mat2(const Mat2&, const Mat2&)>(&glm::operator*),
          sol::resolve<Vec2(const Mat2&, const Vec2&)>(&glm::operator*)
          );
      m2["div"] = sol::overload(
          sol::resolve<Mat2(const Mat2&, const Mat2&)>(&glm::operator/),
          sol::resolve<Vec2(const Mat2&, const Vec2&)>(&glm::operator/)
          );
      
      m2["transpose"] = [](Mat2* _this){ *_this = glm::transpose(*_this); return _this; };
      m2["inverse"]   = [](Mat2* _this){ *_this = glm::inverse(*_this); return _this; };

      m2["transposed"] = &glm::transpose<2, 2, glm::f32, glm::defaultp>;
      m2["inversed"]   = &glm::inverse<2, 2, glm::f32, glm::defaultp>;
    }

    //glm style functions
    {
      //arethmatic
      real.set_function("add", sol::overload(
            sol::resolve<Vec4(const Vec4&, const Vec4&)>(&glm::operator+), 
            sol::resolve<Vec4(const Vec4&, float)>(&glm::operator+), 
            sol::resolve<Vec4(float, const Vec4&)>(&glm::operator+), 
            
            sol::resolve<Vec3(const Vec3&, const Vec3&)>(&glm::operator+), 
            sol::resolve<Vec3(const Vec3&, float)>(&glm::operator+), 
            sol::resolve<Vec3(float, const Vec3&)>(&glm::operator+), 
            
            sol::resolve<Vec2(const Vec2&, const Vec2&)>(&glm::operator+), 
            sol::resolve<Vec2(const Vec2&, float)>(&glm::operator+), 
            sol::resolve<Vec2(float, const Vec2&)>(&glm::operator+), 
            
            sol::resolve<Mat4(const Mat4&, const Mat4&)>(&glm::operator+), 
            sol::resolve<Mat4(const Mat4&, float)>(&glm::operator+), 
            sol::resolve<Mat4(float, const Mat4&)>(&glm::operator+), 
            
            sol::resolve<Mat3(const Mat3&, const Mat3&)>(&glm::operator+), 
            sol::resolve<Mat3(const Mat3&, float)>(&glm::operator+), 
            sol::resolve<Mat3(float, const Mat3&)>(&glm::operator+), 
            
            sol::resolve<Mat2(const Mat2&, const Mat2&)>(&glm::operator+), 
            sol::resolve<Mat2(const Mat2&, float)>(&glm::operator+), 
            sol::resolve<Mat2(float, const Mat2&)>(&glm::operator+) 
            ));
      real.set_function("subtract", sol::overload(
            sol::resolve<Vec4(const Vec4&, const Vec4&)>(&glm::operator-), 
            sol::resolve<Vec4(const Vec4&, float)>(&glm::operator-), 
            sol::resolve<Vec4(float, const Vec4&)>(&glm::operator-), 
            
            sol::resolve<Vec3(const Vec3&, const Vec3&)>(&glm::operator-), 
            sol::resolve<Vec3(const Vec3&, float)>(&glm::operator-), 
            sol::resolve<Vec3(float, const Vec3&)>(&glm::operator-), 
            
            sol::resolve<Vec2(const Vec2&, const Vec2&)>(&glm::operator-), 
            sol::resolve<Vec2(const Vec2&, float)>(&glm::operator-), 
            sol::resolve<Vec2(float, const Vec2&)>(&glm::operator-), 
            
            sol::resolve<Mat4(const Mat4&, const Mat4&)>(&glm::operator-), 
            sol::resolve<Mat4(const Mat4&, float)>(&glm::operator-), 
            sol::resolve<Mat4(float, const Mat4&)>(&glm::operator-), 
            
            sol::resolve<Mat3(const Mat3&, const Mat3&)>(&glm::operator-), 
            sol::resolve<Mat3(const Mat3&, float)>(&glm::operator-), 
            sol::resolve<Mat3(float, const Mat3&)>(&glm::operator-), 
            
            sol::resolve<Mat2(const Mat2&, const Mat2&)>(&glm::operator-), 
            sol::resolve<Mat2(const Mat2&, float)>(&glm::operator-), 
            sol::resolve<Mat2(float, const Mat2&)>(&glm::operator-) 
            ));
      real.set_function("multiply", sol::overload(
            sol::resolve<Vec4(const Vec4&, const Vec4&)>(&glm::operator*), 
            sol::resolve<Vec4(const Vec4&, float)>(&glm::operator*), 
            sol::resolve<Vec4(float, const Vec4&)>(&glm::operator*), 
            
            sol::resolve<Vec3(const Vec3&, const Vec3&)>(&glm::operator*), 
            sol::resolve<Vec3(const Vec3&, float)>(&glm::operator*), 
            sol::resolve<Vec3(float, const Vec3&)>(&glm::operator*), 
            
            sol::resolve<Vec2(const Vec2&, const Vec2&)>(&glm::operator*), 
            sol::resolve<Vec2(const Vec2&, float)>(&glm::operator*), 
            sol::resolve<Vec2(float, const Vec2&)>(&glm::operator*), 
            
            sol::resolve<Mat4(const Mat4&, const Mat4&)>(&glm::operator*), 
            sol::resolve<Mat4(const Mat4&, float)>(&glm::operator*), 
            sol::resolve<Mat4(float, const Mat4&)>(&glm::operator*), 
            
            sol::resolve<Mat3(const Mat3&, const Mat3&)>(&glm::operator*), 
            sol::resolve<Mat3(const Mat3&, float)>(&glm::operator*), 
            sol::resolve<Mat3(float, const Mat3&)>(&glm::operator*), 
            
            sol::resolve<Mat2(const Mat2&, const Mat2&)>(&glm::operator*), 
            sol::resolve<Mat2(const Mat2&, float)>(&glm::operator*), 
            sol::resolve<Mat2(float, const Mat2&)>(&glm::operator*),  
          
            sol::resolve<Vec4(const Mat4&, const Vec4&)>(&glm::operator*), 
            sol::resolve<Vec3(const Vec3&, const Mat3&)>(&glm::operator*), 
            sol::resolve<Vec2(const Mat2&, const Vec2&)>(&glm::operator*)
            ));
      real.set_function("devide", sol::overload(
            sol::resolve<Vec4(const Vec4&, const Vec4&)>(&glm::operator/), 
            sol::resolve<Vec4(const Vec4&, float)>(&glm::operator/), 
            sol::resolve<Vec4(float, const Vec4&)>(&glm::operator/), 
            
            sol::resolve<Vec3(const Vec3&, const Vec3&)>(&glm::operator/), 
            sol::resolve<Vec3(const Vec3&, float)>(&glm::operator/), 
            sol::resolve<Vec3(float, const Vec3&)>(&glm::operator/), 
            
            sol::resolve<Vec2(const Vec2&, const Vec2&)>(&glm::operator/), 
            sol::resolve<Vec2(const Vec2&, float)>(&glm::operator/), 
            sol::resolve<Vec2(float, const Vec2&)>(&glm::operator/), 
            
            sol::resolve<Mat4(const Mat4&, const Mat4&)>(&glm::operator/), 
            sol::resolve<Mat4(const Mat4&, float)>(&glm::operator/), 
            sol::resolve<Mat4(float, const Mat4&)>(&glm::operator/), 
            
            sol::resolve<Mat3(const Mat3&, const Mat3&)>(&glm::operator/), 
            sol::resolve<Mat3(const Mat3&, float)>(&glm::operator/), 
            sol::resolve<Mat3(float, const Mat3&)>(&glm::operator/), 
            
            sol::resolve<Mat2(const Mat2&, const Mat2&)>(&glm::operator/), 
            sol::resolve<Mat2(const Mat2&, float)>(&glm::operator/), 
            sol::resolve<Mat2(float, const Mat2&)>(&glm::operator/),  
          
            sol::resolve<Vec4(const Mat4&, const Vec4&)>(&glm::operator/), 
            sol::resolve<Vec3(const Vec3&, const Mat3&)>(&glm::operator/), 
            sol::resolve<Vec2(const Mat2&, const Vec2&)>(&glm::operator/)
            ));
      //vector
      real.set_function("dot", sol::overload(
            sol::resolve<float(const Vec4&, const Vec4&)>(&glm::dot), 
            sol::resolve<float(const Vec3&, const Vec3&)>(&glm::dot), 
            sol::resolve<float(const Vec2&, const Vec2&)>(&glm::dot) 
            ));
      real.set_function("cross", sol::overload(
            sol::resolve<Vec3(const Vec3&, const Vec3&)>(&glm::cross)
            ));
      real.set_function("length", sol::overload(
            sol::resolve<float(const Vec4&)>(&glm::length), 
            sol::resolve<float(const Vec3&)>(&glm::length), 
            sol::resolve<float(const Vec2&)>(&glm::length) 
            ));
      real.set_function("normalize", sol::overload(
            sol::resolve<Vec4(const Vec4&)>(&glm::normalize), 
            sol::resolve<Vec3(const Vec3&)>(&glm::normalize), 
            sol::resolve<Vec2(const Vec2&)>(&glm::normalize) 
            ));
      //matrix 
      real.set_function("inverse", sol::overload(
            sol::resolve<Mat4(const Mat4&)>(&glm::inverse), 
            sol::resolve<Mat3(const Mat3&)>(&glm::inverse), 
            sol::resolve<Mat2(const Mat2&)>(&glm::inverse) 
            ));
      real.set_function("transpose", sol::overload(
            sol::resolve<Mat4(const Mat4&)>(&glm::transpose), 
            sol::resolve<Mat3(const Mat3&)>(&glm::transpose), 
            sol::resolve<Mat2(const Mat2&)>(&glm::transpose) 
            ));
      real.set_function("translate", sol::resolve<Mat4(const Mat4&, const Vec3&)>(&glm::translate));
      real.set_function("rotate", sol::resolve<Mat4(const Mat4&, float, const Vec3&)>(&glm::rotate));
      real.set_function("scale", sol::resolve<Mat4(const Mat4&, const Vec3&)>(&glm::scale));
      real.set_function("transform", [](const Mat4& m, float a, const Vec3& t, const Vec3& s, const Vec3& r){
          Mat4 ret = m;
          ret = glm::translate(ret, t);
          ret = glm::rotate(ret, a, r);
          ret = glm::scale(ret, s);
          return ret;
          });
      real.set_function("decompose", [](const Mat4& m){
          Vec3 t(0), r(0), s(0);
          bool sucsses = decompose(m, t, r, s);
          if(!sucsses) REAL_CORE_WARN("Failed to decompose matrix: {0}, returnning empty Vectors", m);
          return std::make_tuple(t, r, s);
          });
      //trig
      real.set_function("sin", sol::overload(
            sol::resolve<float(float)>(&glm::sin), 
            sol::resolve<Vec4(const Vec4&)>(&glm::sin), 
            sol::resolve<Vec3(const Vec3&)>(&glm::sin), 
            sol::resolve<Vec2(const Vec2&)>(&glm::sin)
            ));
     real.set_function("cos", sol::overload(
            sol::resolve<float(float)>(&glm::cos), 
            sol::resolve<Vec4(const Vec4&)>(&glm::cos), 
            sol::resolve<Vec3(const Vec3&)>(&glm::cos), 
            sol::resolve<Vec2(const Vec2&)>(&glm::cos)
            ));
     real.set_function("tan", sol::overload(
            sol::resolve<float(float)>(&glm::tan), 
            sol::resolve<Vec4(const Vec4&)>(&glm::tan), 
            sol::resolve<Vec3(const Vec3&)>(&glm::tan), 
            sol::resolve<Vec2(const Vec2&)>(&glm::tan) 
            ));
    real.set_function("asin", sol::overload(
           sol::resolve<float(float)>(&glm::asin), 
           sol::resolve<Vec4(const Vec4&)>(&glm::asin), 
           sol::resolve<Vec3(const Vec3&)>(&glm::asin), 
           sol::resolve<Vec2(const Vec2&)>(&glm::asin) 
           ));
    real.set_function("acos", sol::overload(
           sol::resolve<float(float)>(&glm::acos), 
           sol::resolve<Vec4(const Vec4&)>(&glm::acos), 
           sol::resolve<Vec3(const Vec3&)>(&glm::acos), 
           sol::resolve<Vec2(const Vec2&)>(&glm::acos) 
           ));
    real.set_function("atan", sol::overload(
           sol::resolve<float(float)>(&glm::atan), 
           sol::resolve<Vec4(const Vec4&)>(&glm::atan), 
           sol::resolve<Vec3(const Vec3&)>(&glm::atan), 
           sol::resolve<Vec2(const Vec2&)>(&glm::atan) 
           ));
    //others
     real.set_function("abs", sol::overload(
           sol::resolve<float(float)>(&glm::abs), 
           sol::resolve<Vec4(const Vec4&)>(&glm::abs), 
           sol::resolve<Vec3(const Vec3&)>(&glm::abs), 
           sol::resolve<Vec2(const Vec2&)>(&glm::abs), 
           sol::resolve<Mat4(const Mat4&)>(&glm::abs), 
           sol::resolve<Mat3(const Mat3&)>(&glm::abs), 
           sol::resolve<Mat2(const Mat2&)>(&glm::abs)
           ));
    }
  }

  void Lua::initScene()
  {
    //components
    {
      sol::table comps = real["Components"].get_or_create<sol::table>();
      
      //IDComponent
      {
        sol::usertype<IDComponent> id = comps.new_usertype<IDComponent>("UUID");
        id["getUUID"] = &IDComponent::ID;
      }

      //TagComponent
      {
        sol::usertype<TagComponent> tag = comps.new_usertype<TagComponent>("Tag", 
          sol::constructors<TagComponent(), TagComponent(const std::string&)>());
        tag["tag"] = &TagComponent::name;
      }

      //TransformComponent
      {
        sol::usertype<TransformComponent> trans = comps.new_usertype<TransformComponent>("Transform", 
            sol::constructors<TransformComponent(),
            TransformComponent(const Vec3&, const Vec3&, const Vec3&)>()
            );
        trans["position"] = &TransformComponent::position;
        trans["scale"]    = &TransformComponent::scale;
        trans["rotation"] = &TransformComponent::rotation;
        trans["asMat4"]   = &TransformComponent::getTransformationMatrix;
      }

      //SpriteComponent
      {
        sol::usertype<SpriteComponent> sprite = comps.new_usertype<SpriteComponent>("Sprite", 
            sol::constructors<SpriteComponent(const Vec4& ,UUID)>());
        sprite["color"]   = &SpriteComponent::color;
        sprite["handle"] = &SpriteComponent::texture_handle;
      }

      //CircleComponent
      {
        sol::usertype<CircleComponent> circle = comps.new_usertype<CircleComponent>("Circle");
        circle["color"]     = &CircleComponent::color;
        circle["thickness"] = &CircleComponent::thickness;
        circle["blur"]      = &CircleComponent::blur;
      }

      //ScriptComponent
      {
        sol::usertype<ScriptComponent> script = comps.new_usertype<ScriptComponent>("Script", 
            sol::constructors<ScriptComponent()>()
            );
        script["handle"] = &ScriptComponent::script_handle;
      }

      //CameraComponent
      {
        sol::usertype<CameraComponent> cam = comps.new_usertype<CameraComponent>("Camera");
        cam["camera"]             = &CameraComponent::camera;
        cam["fixed_aspect_ratio"] = &CameraComponent::is_fixed_aspect_ratio;
        cam["primary"]            = &CameraComponent::is_primary;
      }

      //RigidBody2DComponent
      {
        sol::usertype<RigidBody2DComponent> rb2d = comps.new_usertype<RigidBody2DComponent>("RigidBody2D");
        rb2d["type"]           = &RigidBody2DComponent::type;
        rb2d["fixed_rotation"] = &RigidBody2DComponent::fixed_rotation;
        rb2d["gravity_scale"] = &RigidBody2DComponent::gravity_scale;
      }

      //MovmentComponent
      {
        sol::usertype<MovmentComponent> mc = comps.new_usertype<MovmentComponent>("Movment");
        mc["new_transform"]  = &MovmentComponent::new_transform;
        mc["new_angle"] = &MovmentComponent::new_angle;
      }

      //LinearImpulseComponent
      {
        sol::usertype<LinearImpulseComponent> lic = comps.new_usertype<LinearImpulseComponent>("LinearImpulse");
        lic["magnitude"] = &LinearImpulseComponent::magnitude;
        lic["point"] = &LinearImpulseComponent::point;
      }

      //ForceComponent
      {
        sol::usertype<ForceComponent> fc = comps.new_usertype<ForceComponent>("Force");
        fc["magnitude"] = &ForceComponent::magnitude;
        fc["point"] = &ForceComponent::point;
      }

      //AngularImpulseComponent
      {
        sol::usertype<AngularImpulseComponent> aic = comps.new_usertype<AngularImpulseComponent>("AngularImpulse");
        aic["magnitude"] = &AngularImpulseComponent::magnitude;
      }
      
      //TorqueComponent
      {
        sol::usertype<TorqueComponent> aic = comps.new_usertype<TorqueComponent>("Torque");
        aic["magnitude"] = &TorqueComponent::magnitude;
      }

      //BoxColliderComponent
      {
        sol::usertype<BoxColliderComponent> bc = comps.new_usertype<BoxColliderComponent>("BoxCollider");
        bc["offset"]                = &BoxColliderComponent::offset;
        bc["size"]                  = &BoxColliderComponent::size;
        bc["density"]               = &BoxColliderComponent::density;
        bc["friction"]              = &BoxColliderComponent::friction;
        bc["restitution"]           = &BoxColliderComponent::restitution;
        bc["restitution_threshold"] = &BoxColliderComponent::restitution_threshold;
      }

      //CircleColliderComponent
      {
        sol::usertype<CircleColliderComponent> bc = comps.new_usertype<CircleColliderComponent>("CircleCollider");
        bc["offset"]                = &CircleColliderComponent::offset;
        bc["radius"]                = &CircleColliderComponent::radius;
        bc["density"]               = &CircleColliderComponent::density;
        bc["friction"]              = &CircleColliderComponent::friction;
        bc["restitution"]           = &CircleColliderComponent::restitution;
        bc["restitution_threshold"] = &CircleColliderComponent::restitution_threshold;
      }
    }

    //Scene Camera
    {
      sol::usertype<SceneCamera> scene_cam = real.new_usertype<SceneCamera>("SceneCamera");
      scene_cam["getProjection"] = &SceneCamera::getProjection;
      
      scene_cam["setType"] = &SceneCamera::setType;
      scene_cam["getType"] = &SceneCamera::getType;
      
      scene_cam["setPerspectiveData"] = sol::overload(
          sol::resolve<void(const SceneCamera::PerspectiveData&)>(&SceneCamera::setPerspectiveData), 
          sol::resolve<float, float, float>(&SceneCamera::setPerspectiveData)
          );
      scene_cam["getPerspectiveData"] = &SceneCamera::getPerspectiveData;
      scene_cam["setOrthographicData"] = sol::overload(
          sol::resolve<void(const SceneCamera::OrthographicData&)>(&SceneCamera::setOrthographicData), 
          sol::resolve<void(float, float, float)>(&SceneCamera::setOrthographicData)
          );
      scene_cam["getOrthographicData"] = &SceneCamera::getOrthographicData;  
    }

    //Scene
    {
      sol::usertype<Scene> scene_obj = real.new_usertype<Scene>("Scene");
      scene_obj["getName"]      = &Scene::getName;
      scene_obj["addEntity"]    = sol::resolve<Entity(const std::string&)>(&Scene::addEntity); 
      scene_obj["copyEntity"]   = &Scene::copyEntity;
      scene_obj["removeEntity"] = &Scene::removeEntity;
      scene_obj["clear"]        = &Scene::clear;
      scene_obj["getEntity"]    = sol::overload(
          sol::resolve<Entity(const std::string&)>(&Scene::getEntity), 
          sol::resolve<Entity(UUID)>(&Scene::getEntity));
    }
   
    //Entity
    {
      //Regestring functions for all components i'm sure there is much better way
      ADD_COMPONENT_FUNCTIONS(ID);
      ADD_COMPONENT_FUNCTIONS(Tag);
      ADD_COMPONENT_FUNCTIONS(Transform);
      ADD_COMPONENT_FUNCTIONS(Sprite);
      ADD_COMPONENT_FUNCTIONS(Circle);
      ADD_COMPONENT_FUNCTIONS(Script);
      ADD_COMPONENT_FUNCTIONS(Camera);
      ADD_COMPONENT_FUNCTIONS(RigidBody2D);
      ADD_COMPONENT_FUNCTIONS(Movment);
      ADD_COMPONENT_FUNCTIONS(BoxCollider);
      ADD_COMPONENT_FUNCTIONS(CircleCollider);
      ADD_COMPONENT_FUNCTIONS(LinearImpulse);
      ADD_COMPONENT_FUNCTIONS(Force);
      ADD_COMPONENT_FUNCTIONS(AngularImpulse);
      ADD_COMPONENT_FUNCTIONS(Torque);
      
      sol::usertype<Entity> entity = real.new_usertype<Entity>("Entity", 
          sol::constructors<Entity(uint64_t, Scene*)>()
          );
      entity["getId"] = &Entity::operator unsigned int;//Debug
      entity["valid"] = &Entity::operator bool;
      entity["same"]  = &Entity::operator==;

      REGESTER_COMPONENT_FUNCTIONS(ID, entity);
      REGESTER_COMPONENT_FUNCTIONS(Tag, entity);
      REGESTER_COMPONENT_FUNCTIONS(Transform, entity);
      REGESTER_COMPONENT_FUNCTIONS(Sprite, entity);
      REGESTER_COMPONENT_FUNCTIONS(Circle, entity);
      REGESTER_COMPONENT_FUNCTIONS(Script, entity);
      REGESTER_COMPONENT_FUNCTIONS(Camera, entity);
      REGESTER_COMPONENT_FUNCTIONS(RigidBody2D, entity);
      REGESTER_COMPONENT_FUNCTIONS(Movment, entity);
      REGESTER_COMPONENT_FUNCTIONS(BoxCollider, entity);
      REGESTER_COMPONENT_FUNCTIONS(CircleCollider, entity);
      REGESTER_COMPONENT_FUNCTIONS(LinearImpulse, entity);
      REGESTER_COMPONENT_FUNCTIONS(Force, entity);
      REGESTER_COMPONENT_FUNCTIONS(AngularImpulse, entity);
      REGESTER_COMPONENT_FUNCTIONS(Torque, entity);
    }

    REAL_CORE_WARN("We exited initScene");
  }

  void Lua::initRenderer()
  {
    //TODO: API

    //Camera 
    {
      sol::usertype<Camera> cam = real.new_usertype<Camera>("Camera");
      cam["getProjection"] = &Camera::getProjection;
    }
    //Renderer2D
    {
      sol::table r2d = real["Renderer2D"].get_or_create<sol::table>();
      r2d.set_function("beginScene", sol::overload(
            sol::resolve<void(const Mat4&, const Mat4&)>(&Renderer2D::beginScene)
            ));
      r2d.set_function("endScene", &Renderer2D::endScene);
      
      r2d.set_function("setClearColor", &Renderer2D::setClearColor);
      r2d.set_function("clear", sol::overload(
            sol::resolve<void(const Vec4&)>(&Renderer2D::clear),
            sol::resolve<void(void)>(&Renderer2D::clear)
            ));

      r2d.set_function("drawQuad", sol::overload(
            sol::resolve<void(const Mat4&, const Vec4&, int)>(&Renderer2D::drawQuad),
            sol::resolve<void(const Mat4&, const ARef<Texture2D>&, const Vec4&, int)>(&Renderer2D::drawQuad),
           //TODO: Camera overload but first make it take view insted of transform

            sol::resolve<void(const Vec3&, const Vec2&, const Vec4&)>(&Renderer2D::drawQuad),
            sol::resolve<void(const Vec2&, const Vec2&, const Vec4&)>(&Renderer2D::drawQuad),
            
            sol::resolve<void(const Vec3&, const Vec2&, float, const Vec4&)>(&Renderer2D::drawQuad),
            sol::resolve<void(const Vec2&, const Vec2&, float, const Vec4&)>(&Renderer2D::drawQuad),
            
            sol::resolve<void(const Vec3&, const Vec2&, float, const ARef<Texture2D>&, const Vec4&)>(&Renderer2D::drawQuad),
            sol::resolve<void(const Vec2&, const Vec2&, float, const ARef<Texture2D>&, const Vec4&)>(&Renderer2D::drawQuad),
            
            sol::resolve<void(const Vec3&, const Vec2&, const ARef<Texture2D>&, const Vec4&)>(&Renderer2D::drawQuad),
            sol::resolve<void(const Vec2&, const Vec2&, const ARef<Texture2D>&, const Vec4&)>(&Renderer2D::drawQuad)
            
            ));
    }
  }


  void Lua::initScripting()
  {
    sol::usertype<Script> script = real.new_usertype<Script>("Script");
    script["onCreate"] = &Script::onCreate;
    script["onUpdate"] = &Script::onUpdate;
    script["onDestroy"] = &Script::onDestroy;
    script["setExport"] = &Script::setExport;
    script["entity"] = &Script::entity;
    script["getTimestep"] = &Script::getTimestep;
    
    script["export"] = sol::overload(
            [](Script* _this, const char* as, Vec4* v4){
              Export::pushExportVariable(as, { ExportType::Type::Vec4, v4 },
                  _this->entity.getComponent<IDComponent>().ID);
              }, 
            [](Script* _this, const char* as, Vec3* v3){
              Export::pushExportVariable(as, { ExportType::Type::Vec3, v3 }, 
                  _this->entity.getComponent<IDComponent>().ID);
              }, 
            [](Script* _this, const char* as, Vec2* v2){
              Export::pushExportVariable(as, { ExportType::Type::Vec2, v2 }, 
                  _this->entity.getComponent<IDComponent>().ID);
              },
              [](Script* _this, const char* as, Vec4* v4, bool dummy){ 
              Export::pushExportVariable(as, { ExportType::Type::RGBA_Color, v4 },
                  _this->entity.getComponent<IDComponent>().ID);
              },  
            [](Script* _this, const char* as, Float* f){
              Export::pushExportVariable(as, { ExportType::Type::Float, f }, 
                  _this->entity.getComponent<IDComponent>().ID);
             },
            [](Script* _this, const char* as, String* s){
              Export::pushExportVariable(as, { ExportType::Type::String, s }, 
                  _this->entity.getComponent<IDComponent>().ID);  
              }, 
            [](Script* _this, const char* as, sol::lua_nil_t* n){
              Export::pushExportVariable(as, { ExportType::Type::None, n },
                  _this->entity.getComponent<IDComponent>().ID);
            }
          );

    script["exportEntity"] = [](Script* _this, const char* as, UUID* id)
    {
      Export::pushExportVariable(as, { ExportType::Type::Entity, id }, 
          _this->entity.getComponent<IDComponent>().ID);
    }; 

    script["exportScene"] = [](Script* _this, const char* as, UUID* id)
    {
      Export::pushExportVariable(as, { ExportType::Type::Scene, id }, 
          _this->entity.getComponent<IDComponent>().ID);
    }; 
    script["exportScript"] = [](Script* _this, const char* as, UUID* id)
    {
      Export::pushExportVariable(as, { ExportType::Type::Script, id }, 
          _this->entity.getComponent<IDComponent>().ID);
    };
    script["exportTexture"] = [](Script* _this, const char* as, UUID* id)
    {
      Export::pushExportVariable(as, { ExportType::Type::Texture, id }, 
          _this->entity.getComponent<IDComponent>().ID);
    };
    script["exportAsset"] = [](Script* _this, const char* as, UUID* id)
    {
      Export::pushExportVariable(as, { ExportType::Type::Asset, id }, 
          _this->entity.getComponent<IDComponent>().ID);
    }; 
  }
}

