#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H


#include "Real-Engine/Core/UUID.h"
#include "Real-Engine/Render/API/Texture.h"
#include "Real-Engine/Scene/Scene.h"
#include "Real-Engine/Scripting/Lua.h"
#include "Real-Engine/Scripting/Script.h"
#include <string>


namespace Real
{
  class AssetManager
  {
  public:
    struct Asset
    {
      Asset() = default;
      Asset(bool is_valid) : is_valid(is_valid) {}
      enum class Type { None = 0, Texture2D, Scene, Script };
      template<typename T>
      static Type getTypeEnum();
      struct Meta
      {
        std::string path;
        Type type;
      };
      Meta meta;
      std::function<void(void)> onReload;
      static const ARef<Asset> invalid;
      bool is_valid = true;
      operator bool() { return is_valid; }
    };

    struct Texture2DAsset : public Asset
    {
      ARef<Texture2D> texture;
    };

    struct SceneAsset : public Asset
    {
      ARef<Scene> scene;
    };

    struct ScriptAsset : public Asset
    {
      //std::string script_path;
      Script* script = nullptr;
      ScriptError err;
    };

    struct AssetLoader
    {
      template<typename T>
      static bool load(const T&);
      static bool loadTexture2D(const ARef<Texture2DAsset>& asset);
      static bool loadScene(const ARef<SceneAsset>& asset);
      static bool loadScript(const ARef<ScriptAsset>& asset);
    };

  public:
    AssetManager() = default;
    AssetManager(const std::filesystem::path& config_path);
    static void shutDown();
    static bool loadConfig(const std::filesystem::path& path);
    static bool saveConfig(const std::filesystem::path& path);
    static inline ARef<Asset> get(UUID id)
    {
      if(s_instance->m_assets.find(id) != s_instance->m_assets.end())
        return s_instance->m_assets.at(id);
      return Asset::invalid;
    }

    template<typename T>
    static inline ARef<T> get(UUID id)
    {
      if(s_instance->m_assets.find(id) != s_instance->m_assets.end())
      {
        if(s_instance->m_assets.at(id)->meta.type == Asset::getTypeEnum<T>())
          return std::static_pointer_cast<T>(s_instance->m_assets.at(id));
      }
      return std::static_pointer_cast<T>(Asset::invalid);
    }

    template<typename T>
    static inline ARef<T> get(const std::string& path)
    {
      return get<T>(getUUID(path));
    }

    static inline UUID getUUID(const std::string& path)
    {
      if(s_instance->m_paths.find(path) != s_instance->m_paths.end())
        return s_instance->m_paths.at(path);
      REAL_CORE_ERROR("(AssetManager): Can't find Asset: {0}", path);
      return UUID::invalid;
    }

    static inline const std::string& getPath(UUID id)
    {
      auto asset = get(id);
      if(asset) return asset->meta.path;
      REAL_CORE_ERROR("(AssetManager): Can't find Asset: {0}", id);
      return s_instance->error_string;
    }

    static inline const std::unordered_map<UUID, ARef<Asset>>& get()      { return s_instance->m_assets; }
    static inline const std::unordered_map<std::string, UUID>& getPaths() { return s_instance->m_paths; }

    template<typename T>
    static UUID load(const Asset::Meta& meta, UUID id = UUID())
    {
      auto asset = createARef<T>();
      asset->meta = meta;
      asset->meta.type = Asset::getTypeEnum<T>();

      if(!AssetLoader::load(asset))
      {
        REAL_CORE_ERROR("(AssetManager): Can't Load Asset: {0}", meta.path);
        return UUID::invalid;
      }
      id = add(std::static_pointer_cast<Asset>(asset), id, meta.path);
      if(id != UUID::invalid)
        return id;
      REAL_CORE_ERROR("Can't Load Asset: {0}", meta.path);
      return id;
    }

    static UUID load(const Asset::Meta& meta, UUID id = UUID())
    {
      switch(meta.type)
      {
        case Asset::Type::None:
        {
          REAL_CORE_ERROR("Can't Load Asset: {0}, With Asset::Type::None", meta.path);
          return UUID::invalid;
        }
        case Asset::Type::Texture2D:
        {
          return load<Texture2DAsset>(meta, id);
        }
        case Asset::Type::Scene:
        {
          return load<SceneAsset>(meta, id);
        }
        case Asset::Type::Script:
        {
          return load<ScriptAsset>(meta, id);
        }
      }
      REAL_CORE_ERROR("Invalid Asset::Type: {0}", (int)meta.type);
      return UUID::invalid;
    }

    static UUID loadOrGet(const Asset::Meta& meta)
    {
      auto id = getUUID(meta.path);
      if(id != UUID::invalid)
        return id;
      else
        return load(meta);
    }

    template<typename T>
    static inline bool reload(UUID id)
    {
      auto asset = get<T>(id);
      auto it = std::find_if(s_instance->m_paths.begin(), s_instance->m_paths.end(), [&id](auto& it)
          { return it.second == id; });
      s_instance->m_paths.erase(it);
      s_instance->m_paths[asset->meta.path] = id;
      return AssetLoader::load(asset);
    }

    static bool reload(UUID id)
    {
      auto asset = get(id);
      switch(asset->meta.type)
      {
        case Asset::Type::None:
        {
          REAL_CORE_ERROR("Can't Reload Asset: {0}, With Asset::Type::None", asset->meta.path);
          return false;
        }
        case Asset::Type::Texture2D:
        {
          return reload<Texture2DAsset>(id);
        }
        case Asset::Type::Scene:
        {
          return reload<SceneAsset>(id);
        }
        case Asset::Type::Script:
        {
          return reload<ScriptAsset>(id);
        }
      }
      REAL_CORE_ERROR("Invalid Asset::Type: {0}", (int)asset->meta.type);
      return false;
    }

    static bool reload()
    {
      for(auto iter : s_instance->m_assets)
      {
        bool is = reload(iter.first);
        if(!is) return false;
      }
      return true;
    }

    static inline void clear()
    {
      s_instance->m_assets.clear();
      s_instance->m_paths.clear();
    }

    static inline void remove(UUID id)
    {
      const auto& asset = get(id);
      s_instance->m_paths.erase(s_instance->m_paths.find(asset->meta.path));
      s_instance->m_assets.erase(s_instance->m_assets.find(id));
    }

  private:
    static UUID add(const ARef<Asset>& asset, UUID uuid, const std::string& path);
  private:
    static AssetManager* s_instance;
    std::unordered_map<UUID, ARef<Asset>> m_assets;
    std::unordered_map<std::string, UUID> m_paths;
    std::string error_string;
  };
}


#endif //ASSET_MANAGER_H_TEST
