#include "pch.h"
#ifdef REAL_PLATFORM_LINUX
#include "Real-Engine/Core/Utils/FileDialogs.h"


namespace Real
{
  //temp TODO:remove this once you have a proper file/asset manager 
  //future: why? :-)
  static std::string processFilters(const char* kr_filters, FileDialogs::OpType type);
  static void split(const std::string& str, std::vector<std::string>& cont, char sep) 
  {
    REAL_PROFILE_FUNCTION();
    std::stringstream ss(str);
    std::string token;
    while(ss >> token)
    {
      token.erase(std::remove(token.begin(), token.end(), sep), token.end());
      cont.push_back(token);
    }
  }

  std::string FileDialogs::OpenFile(const char* filters, const char* prev)
  {
    return processFilters(filters, OpType::OpenFile);
  }

  std::string FileDialogs::SaveFile(const char* filters, const char* prev)
  {
    return processFilters(filters, OpType::SaveFile);
  }

  std::string FileDialogs::OpenDir()
  {
    bool stats = true;
    char filename[BUFSIZ];
    auto *f = popen("zenity --file-selection --directory", "r");
    if(fgets(filename, sizeof(filename), f) == nullptr)
      stats = false;
    int res = pclose(f);
    if(!stats && res < 0)
    {
      REAL_CORE_ERROR("Can't Open File Dialog");
      return std::string();
    }
    std::string str_bsn(filename);
    return str_bsn.substr(0, str_bsn.size() - 1);
  }


  std::string processFilters(const char* kr_filters, FileDialogs::OpType type)
  {
    bool stats = true;
    const char* base = "zenity --file-selection";
    const char* filter_base = "--file-filter=";
    std::stringstream ss;
    std::vector<std::string> fils;
    
    if(kr_filters)
    {
      fils.reserve(strlen(kr_filters)/3);
      split(kr_filters, fils, ',');
    }  
    ss << base;
    switch (type)
    {
      case FileDialogs::OpType::OpenFile:
      {
        if(kr_filters)
        {
          ss << " " << filter_base;
          ss << "\" ";
          for(const auto& fil : fils)
          {
            ss << "\"*." << fil << "\" ";
          }
          ss << " \"";
        }
        char filename[BUFSIZ];
        auto *f = popen(ss.str().c_str(), "r");
        if(fgets(filename, sizeof(filename), f) == nullptr)
          stats = false;
        int res = pclose(f);
        if(!stats && res < 0)
        {
          REAL_CORE_ERROR("Can't Open File Dialog");
          return std::string();
        }
        std::string str_bsn(filename);
        return str_bsn.substr(0, str_bsn.size() - 1);
      }
      case FileDialogs::OpType::SaveFile:
      {
        ss << " --save ";
        if(kr_filters)
        {
          ss << " " << filter_base;
          ss << "\" ";
          for(const auto& fil : fils)
          {
            ss << "\"*." << fil << "\" ";
          }
          ss << " \"";
        }
        char filename[BUFSIZ];
        auto *f = popen(ss.str().c_str(), "r");
        if(fgets(filename, sizeof(filename), f) == nullptr)
          stats = false;
        int res = pclose(f);
        if(!stats && res < 0)
        {
          REAL_CORE_ERROR("Can't Open File Dialog");
          return std::string();
        }
        std::string str_bsn(filename);
        return str_bsn.substr(0, str_bsn.size() - 1);
      }
      default: return "";
    }
    REAL_CORE_ERROR("Invalid OpType {0}", (int)type);
    return std::string();
  }
}

#endif //REAL_PLATFORM_LINUX
