#ifndef RECENT_PROJECTS_SERIALIZER
#define RECENT_PROJECTS_SERIALIZER

#include <Real-Engine/Real-Engine.h>

namespace Real
{
  class RecentProjectsSerializer 
  {
  public:
    static bool serialize(const std::string& path);

    static bool deserialize(const std::string& path);
  };
}


#endif //RECENT_PROJECTS_SERIALIZER
