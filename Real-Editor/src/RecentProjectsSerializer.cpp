#include "RecentProjectsSerializer.h"
#include "Real-Engine/Core/Log.h"
#include "RecentProjects.h"

#include "pugixml.hpp"


namespace Real
{
  bool RecentProjectsSerializer::serialize(const std::string& path)
  {
    pugi::xml_document doc;
    auto projects = doc.append_child("Projects");

    for(const auto& proj : RecentProjects::get())
    {
      auto project = projects.append_child("Project");
      project.append_child("Path").append_child(pugi::node_pcdata).set_value(proj.proj_dir.string().c_str());
    }

    return doc.save_file(path.c_str(), PUGIXML_TEXT("  "));
  }

  bool RecentProjectsSerializer::deserialize(const std::string& path)
  {
    RecentProjects::clear();
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(path.c_str(), pugi::parse_default|pugi::parse_declaration);
    if (!result)
    {
      REAL_CORE_ERROR("Parse Error at: {0}, offset: {1}",result.description(), result.offset);
      return false;
    }
    
    pugi::xml_node projects = doc.child("Projects");

    for(const auto& project : projects)
    {
      std::string path = project.child_value("Path");

      if(path.empty())
      {
        REAL_CORE_ERROR("No Path in Project");
        return false;
      }
      else 
      {
        RecentProjects::append(path);
      }
    }
    return true;
  }

}
