#include <filesystem>
#include <pch.h>
#include "RecentProjects.h"
#include "Real-Engine/Core/Log.h"
#include "Real-Engine/Project/Project.h"


namespace Real
{
  ProjectsVector RecentProjects::s_recent_projects;
  const ProjectsVector& RecentProjects::get()
  {
    return s_recent_projects;
  }

  Project::Config RecentProjects::getProject(const std::filesystem::path& path)
  {
    for(const auto& proj : s_recent_projects)
    {
      if(std::filesystem::equivalent(proj.proj_dir, path))
        return proj;
    }
    REAL_CORE_ERROR("Didn't find Project: {0}", path);
    return {};
  }

  static bool nfind(const std::filesystem::path& path)
  {
    for(const auto& proj : RecentProjects::get())
    {
      if(std::filesystem::exists(path))
      {
        if(std::filesystem::equivalent(proj.proj_dir, path))
        {
          return false;
        }
      }
    }
    return true;
  }

  void RecentProjects::append(const std::filesystem::path& path)
  {
    if(nfind(path) && std::filesystem::exists(path))
    {
      if(Project::load(path / "project.reproj"))
      {
        s_recent_projects.push_back(Project::getConfig());
      }
      else 
      {
        REAL_CORE_ERROR("Error Loading Project: {0}", path);
      }
    }
    else REAL_CORE_INFO("Project: {0} already exist in RecentProjects", path);
  }
}
