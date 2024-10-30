#include <pch.h>
#include "AssetManager.h"
#include "Real-Engine/Core/App.h"
#include "Real-Engine/Core/Core.h"
#include "Real-Engine/Core/Events/Events.h"
#include "Real-Engine/Core/Log.h"
#include "Real-Engine/Scene/Components.h"
#include "Real-Engine/Scene/Scene.h"
#include "Real-Engine/Scene/SceneSerializer.h"
#include "Real-Engine/Scripting/Lua.h"
#include "Real-Engine/Scripting/Script.h"
#include "Real-Engine/Project/Project.h"
#include "AssetManagerSerializer.h"

#include <pugixml.hpp>


namespace Real
{
  const ARef<AssetManager::Asset> AssetManager::Asset::invalid = createARef<AssetManager::Asset>(false);
  AssetManager* AssetManager::s_instance = new AssetManager;

  bool AssetManager::loadConfig(const std::filesystem::path& path)
  {
    return AssetManagerSerializer::deserialize(path);
  }

  bool AssetManager::saveConfig(const std::filesystem::path& path)
  {
    return AssetManagerSerializer::serialize(path);
  }

  bool AssetManager::AssetLoader::loadTexture2D(const ARef<Texture2DAsset>& asset)
  {
    const auto& abs_asset_dir = Project::getConfig().proj_dir/Project::getConfig().asset_dir;
    REAL_CORE_ERROR("final path: {0}", abs_asset_dir/asset->meta.path);
    asset->texture = Texture2D::create((abs_asset_dir/asset->meta.path).string());
    if(asset->onReload)
      asset->onReload();
    
    REAL_CORE_ERROR("done loading it");
    return true;
  }

  bool AssetManager::AssetLoader::loadScene(const ARef<SceneAsset>& asset)
  {
    const auto& abs_asset_dir = Project::getConfig().proj_dir/Project::getConfig().asset_dir;
    asset->scene = createARef<Scene>();
    SceneSerializer ss(asset->scene);
    if(!ss.deSerializeText((abs_asset_dir/asset->meta.path).string().c_str()))
      return false;
    if(asset->onReload)
      asset->onReload();
    return true;
  }

  bool AssetManager::AssetLoader::loadScript(const ARef<ScriptAsset>& asset)
  {
    const auto& abs_asset_dir = Project::getConfig().proj_dir/Project::getConfig().asset_dir;
    auto& lua = Lua::get();
    //lua.safe_script_file(asset->meta.path);
    auto err = Lua::runScript((abs_asset_dir/asset->meta.path).string());
    asset->err = err;
    auto aerr = createARef<ScriptError>(err);
    ScriptErrorEvent e(aerr);
    EventDispatcher dp(e);
    dp.dispatch<ScriptErrorEvent>([&](auto& e){ App::get()->onEvent(e) ; return false;} );
    auto is_error = false;
    if(err.load_status != ScriptError::LoadErrorCode::ok)
    {
      REAL_CORE_ERROR("Error loading Script: path: {0}, Error Code: {1}, Error Message: {2}", 
          asset->meta.path, err.loadECtoString(err.load_status), err.load_error_message);
      is_error = true;
    }
    if(err.status != ScriptError::ErrorCode::ok)
    {
      REAL_CORE_ERROR("Error in Script: path: {0}, Error Code: {1}, Error Message: {2}", 
          asset->meta.path, err.ECtoString(err.status), err.error_message);
      is_error = true;
    }
    auto res = lua["GetObject"];
    if(res.valid())
    {
      sol::function fun = res;
      asset->script = fun();
    }
    else 
    {
      sol::error e = res;
      REAL_CORE_ERROR("{0}", e.what());
      is_error = true;
    }
    asset->is_valid = !is_error;
    if(asset->onReload)
      asset->onReload();
    return true;
  }

  UUID AssetManager::add(const ARef<Asset>& asset, UUID uuid, const std::string& path)
  {
    UUID id = uuid;
    if(s_instance->m_assets.find(uuid) != s_instance->m_assets.end())
    {
      id = UUID();
      add(asset, id, path);
    }
    if(s_instance->m_paths.find(path) != s_instance->m_paths.end())
    {
      REAL_CORE_ERROR("(AssetManager): Asset With Path: {0} already exist", path);
      return UUID::invalid;
    }
    s_instance->m_assets[id] = asset;
    s_instance->m_paths[path] = id;
    return id;
  }

  void AssetManager::shutDown()
  {
    delete s_instance;
    s_instance = nullptr;
  }

  template<typename T>
  bool AssetManager::AssetLoader::load(const T&)
  {
    return false;
  }

  template<>
  bool AssetManager::AssetLoader::load(const ARef<Texture2DAsset>& t)
  {
    return AssetLoader::loadTexture2D(t);
  }

  template<>
  bool AssetManager::AssetLoader::load(const ARef<SceneAsset>& t)
  {
    return AssetLoader::loadScene(t);
  }

 template<>
  bool AssetManager::AssetLoader::load(const ARef<ScriptAsset>& t)
  {
    return AssetLoader::loadScript(t);
  }

 template<typename T>
 AssetManager::Asset::Type AssetManager::Asset::getTypeEnum()
 {
   return Type::None;
 }

 template<>
 AssetManager::Asset::Type AssetManager::Asset::getTypeEnum<AssetManager::Texture2DAsset>()
 {
   return Type::Texture2D;
 }
 
 template<>
 AssetManager::Asset::Type AssetManager::Asset::getTypeEnum<AssetManager::SceneAsset>()
 {
   return Type::Scene;
 }
 
 template<>
 AssetManager::Asset::Type AssetManager::Asset::getTypeEnum<AssetManager::ScriptAsset>()
 {
   return Type::Script;
 }

}
