#ifndef FILE_DIALOGS_H
#define FILE_DIALOGS_H


namespace Real
{
  class REAL_API FileDialogs
  {
  public:
    enum class OpType {OpenFile, SaveFile, OpenDir};
  public:
    static std::string OpenFile(const char* filters = nullptr, const char* prev = nullptr);

    static std::string OpenDir();

    static std::string SaveFile(const char* filters = nullptr, const char* prev = nullptr);
  };
}

#endif //FILE_DIALOGS_H
