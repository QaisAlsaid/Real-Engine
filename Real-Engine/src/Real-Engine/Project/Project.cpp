#include "Project.h"
#include "ProjectSerializer.h"
#include "Real-Engine/Core/Core.h"
#include "Real-Engine/Core/Log.h"

namespace Real 
{
  ARef<Project> Project::s_instance = createARef<Project>();

  bool Project::load(const std::string& path)
  {
    ProjectSerializer ps(s_instance);
    return ps.deserialize(path);
  }

  bool Project::save(const std::string& path)
  {
    ProjectSerializer ps(s_instance);
    return ps.serialize(path);
  }
}
