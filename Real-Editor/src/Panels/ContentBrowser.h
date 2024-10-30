#ifndef CONTENT_BROWSER_TEST_H
#define CONTENT_BROWSER_TEST_H

#include "AssetManagerGui.h"
#include "Real-Engine/Core/AssetManager.h"
#include "Real-Engine/Render/API/Texture.h"
#include <filesystem>


namespace Real 
{
  class ContentBrowser
  {
  public:
    ContentBrowser(const std::string& res_path);//Should Take Project
    bool onImGuiUpdate();
  private:
    enum class FileType 
    {
      Real, Png, Jpeg, Jpg, Yaml, Xml, Cpp, H, Lua, Glsl, Dir, Other, 
      Image = Png || Jpeg || Jpg, Script = Lua, Scene = Real
    };
  private:
    UUID loadOrGet(const std::filesystem::path& path, FileType type);
    void draw(const std::filesystem::path& path, UUID handle);
    FileType getFileType(const std::filesystem::directory_entry& entry);
    AssetManager::Asset::Type getAssetManagerType(FileType);
    UUID getDefaultIcon(FileType ft);
    bool isAsset(FileType ft);
    void dragAndDropTexture(UUID, const Vec2&);
    void dragAndDropScene(UUID, const Vec2&);
    void dragAndDropScript(UUID, const Vec2&);
  private:
    std::filesystem::path m_res_dir;
    std::filesystem::path m_current_dir;
    std::map<UUID, ARef<Texture2D>> m_default_icons;
    std::map<std::string, UUID>     m_names;
    AssetManagerModal m_asset_manager_modal;
  };
}


#endif //CONTENT_BROWSER_TEST_H
