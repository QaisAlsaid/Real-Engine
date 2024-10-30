#ifndef PROJECT_SERIALIZER_H
#define PROJECT_SERIALIZER_H

#include "Project.h"

namespace Real
{
  class REAL_API ProjectSerializer 
  {
  public:
    ProjectSerializer(const ARef<Project>& proj)
      : m_context(proj) {}
    
    bool serialize(const std::string& path);
    bool deserialize(const std::string& path);
  private:
    ARef<Project> m_context;
  };
}

#endif //PROJECT_SERIALIZER_H
