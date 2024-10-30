#ifndef ASSET_MANAGER_SERIALIZER_H
#define ASSET_MANAGER_SERIALIZER_H

#include "AssetManager.h"


namespace Real
{
  class AssetManagerSerializer 
  {
  public:
    static bool serialize(const std::filesystem::path& path);
    static bool deserialize(const std::filesystem::path& path);
  };
}


#endif //ASSET_MANAGER_SERIALIZER_H
