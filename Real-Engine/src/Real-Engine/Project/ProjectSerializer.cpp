#include "ProjectSerializer.h"
#include "Real-Engine/Core/Log.h"
#include <pugixml.hpp>
#include <string>


namespace Real
{
  bool ProjectSerializer::serialize(const std::string& path)
  {
    const auto& config = m_context->getConfig();
    pugi::xml_document doc;
    auto proj = doc.append_child("Project");

    pugi::xml_node name = proj.append_child("Name");
    name.append_child(pugi::node_pcdata).set_value(config.name.c_str());

    auto proj_dir = proj.append_child("ProjectDirectory");
    proj_dir.append_child(pugi::node_pcdata).set_value(config.proj_dir.string().c_str());

    auto asset_dir = proj.append_child("AssetDirectory");
    asset_dir.append_child(pugi::node_pcdata).set_value(config.asset_dir.string().c_str());

    auto main_scene = proj.append_child("MainScene");
    main_scene.append_child(pugi::node_pcdata).set_value(config.main_scene.c_str());

    auto new_dir = proj.append_child("NewDirectory");
    new_dir.append_child(pugi::node_pcdata).set_value(config.new_dir == false ? "False" : "True");
  
    return doc.save_file(path.c_str(), PUGIXML_TEXT("  "));
  }

  bool ProjectSerializer::deserialize(const std::string& path)
  {
    bool ret_invalid = false;
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(path.c_str(),
    pugi::parse_default | pugi::parse_declaration);
    if (!result)
    {
      REAL_CORE_ERROR("Parse Error at: {0}, offset: {1}", result.description(), result.offset);
      return false;
    }

    auto proj = doc.child("Project");

    std::string name = proj.child_value("Name");
    if(name.empty()) 
    {
      REAL_CORE_WARN("Project Name is Empty");
    }
    std::string proj_dir = proj.child_value("ProjectDirectory");
    if(proj_dir.empty())
    {
      REAL_CORE_ERROR("ProjectDirectory is Empty Project: {0} won't be created", name);
      ret_invalid = true;
    }
    std::string asset_dir = proj.child_value("AssetDirectory");
    if(asset_dir.empty())
    {
      REAL_CORE_ERROR("AssetDirectory is Empty Project: {0} won't be created", name);
      ret_invalid = true;
    }
    std::string main_scene = proj.child_value("MainScene");
    if(main_scene.empty())
    {
      REAL_CORE_WARN("MainScene is Empty Project: {0}", name);
    }
    std::string new_dir_str = proj.child_value("NewDirectory");
    if(new_dir_str.empty())
    {
      REAL_CORE_WARN("NewDirectory is Empty creating a new directory by default");
    }

    if(ret_invalid) return false;

    Project::Config config;
    config.name = name.empty() ? "Untitled" : name;
    config.proj_dir = proj_dir; //absolute path
    config.asset_dir = asset_dir;//relative to proj_dir
    config.main_scene = main_scene;//relative to asset_dir
    config.new_dir = new_dir_str == "False" ? false : true; 
    m_context->setConfig(config);
    return true;
  }

}
