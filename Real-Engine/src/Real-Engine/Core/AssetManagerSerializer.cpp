#include <pch.h>
#include "AssetManagerSerializer.h"

#include <pugixml.hpp>
#include <string>

namespace Real
{
  bool AssetManagerSerializer::serialize(const std::filesystem::path& path)
  {
    pugi::xml_document doc;
    auto assets = doc.append_child("Assets");
    std::string type_str;
    for(const auto& asset : AssetManager::get())
    {
      const auto& meta = asset.second->meta;
      auto asset_n = assets.append_child("Asset");
      asset_n.append_child("UUID").append_child(pugi::node_pcdata).set_value(std::to_string(asset.first).c_str());
      auto meta_n = asset_n.append_child("Meta");
      meta_n.append_child("Path").append_child(pugi::node_pcdata).set_value(meta.path.c_str());
      switch(meta.type)
      {
        case AssetManager::Asset::Type::Scene: type_str = "Scene"; break;
        case AssetManager::Asset::Type::Script: type_str = "Script"; break;
        case AssetManager::Asset::Type::Texture2D: type_str = "Texture2D"; break;
        default: type_str = "None";
      }
      meta_n.append_child("Type").append_child(pugi::node_pcdata).set_value(type_str.c_str());
    }
    return doc.save_file(path.c_str(), PUGIXML_TEXT("  "));
  }


  bool AssetManagerSerializer::deserialize(const std::filesystem::path& path)
  {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(path.c_str(),
    pugi::parse_default | pugi::parse_declaration);
    if (!result)
    {
      REAL_CORE_ERROR("Parse Error at: {0}, offset: {1}",result.description(), result.offset);
      return false;
    }
    
    auto assets = doc.child("Assets");
    for(auto asset : assets)
    {
      std::string uuid_str = asset.child_value("UUID");
      if(uuid_str.empty())
      {
        REAL_CORE_WARN("(AssetManager): Trying to Load Asset with no UUID");
        continue;
      }
      auto meta_n = asset.child("Meta");
      std::string path = meta_n.child_value("Path");
      if(path.empty())
      {
        REAL_CORE_WARN("(AssetManager): Trying to Load Asset: {0} with no Meta::Path", uuid_str);
        continue;
      }
      std::string type_str = meta_n.child_value("Type");
      if(type_str.empty())
      {
        REAL_CORE_WARN("(AssetManager): Trying to Load Asset: {0} with no Meta::Type", uuid_str);
        continue;
      }
      
      UUID uuid = std::stoll(uuid_str);
      if(uuid == UUID::invalid) uuid = UUID();
      AssetManager::Asset::Type type = AssetManager::Asset::Type::None;
      if(type_str == "Texture2D") type = AssetManager::Asset::Type::Texture2D; 
      else if(type_str == "Scene") type = AssetManager::Asset::Type::Scene;
      else if(type_str == "Script") type = AssetManager::Asset::Type::Script;
      else { REAL_CORE_WARN("(AssetManager): invalid Asset::Type from Asset: {0}", uuid_str); continue; }

      AssetManager::Asset::Meta meta;
      meta.path = path;
      meta.type = type;
      REAL_CORE_WARN("START LOADING ASSET: {0}", meta.path);
      AssetManager::load(meta, uuid);
      REAL_CORE_WARN("DONE");
    }
    return true;

  }
}
