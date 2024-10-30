#ifndef RECENT_PROJECTS
#define RECENT_PROJECTS

#include "Real-Engine/Project/Project.h"
#include <Real-Engine/Real-Engine.h>
#include <vector>


namespace Real
{
  using ProjectsVector = std::vector<Project::Config>;
  class RecentProjects 
  {
  public:
    static const ProjectsVector& get();
    static Project::Config getProject(const std::filesystem::path& path);
    static void clear() { s_recent_projects.clear(); }
    static void append(const std::filesystem::path&);
  private:
    static ProjectsVector s_recent_projects;
  };
}

#endif //RECENT_PROJECTS
