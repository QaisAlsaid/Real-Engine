#ifndef PROJECT_H
#define PROJECT_H

#include "Real-Engine/Core/Core.h"
#include <Real-Engine/Core/App.h>
#include <filesystem>

namespace Real 
{
  class REAL_API Project 
  {
  public:
    struct Config 
    {
      std::string name = "Untitled";

      std::filesystem::path proj_dir;
      std::filesystem::path asset_dir;
      std::filesystem::path main_scene;
      bool new_dir = true;
    };
  public:
    static bool load(const std::string& path);
    static bool save(const std::string& path);
    static const Config& getConfig() { return s_instance->m_config; }
    static void setConfig(const Config& config) { s_instance->m_config = config; }
  private:
    Config m_config;
    static ARef<Project> s_instance;
  };

}
#endif //PROJECT_H
