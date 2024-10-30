#ifndef KR_LUA_H
#define KR_LUA_H


#include <sol/types.hpp>
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

namespace Real
{
  struct REAL_API ScriptError
  {
    enum class LoadErrorCode
    {
      ok, syntax, memory, gc, file
    };
    
    enum class ErrorCode 
    {
      ok, yielded, runtime, memory, handler, gc, syntax, file
    };

    LoadErrorCode load_status = LoadErrorCode::ok;
    ErrorCode status = ErrorCode::ok;

    std::string load_error_message;
    std::string error_message;
    
    std::string loadECtoString(LoadErrorCode lec);
    std::string ECtoString(ErrorCode ec);
  private:
    static ErrorCode toEC(sol::call_status);
    static LoadErrorCode toLEC(sol::load_status);
    
    friend class Lua;
  };

  class REAL_API Lua 
  {
  public:
    static void init();
    inline static sol::state& get() { return lua; }
    static ScriptError runScript(const std::string& path);
  private:
    static void initCore();
    static void initMath();
    static void initRenderer();
    static void initScripting();
    static void initScene();
  private:
    static sol::state lua;
    static sol::table real;
  };

  struct Float 
  {
    Float(float f) : _float(f) {};

    float _float = 0.0f;
  };

  struct Int 
  {
    Int(int32_t i) : _int(i) {}

    int32_t _int = 0; 
  };

  //for now its here maybe later will make own string class 
  struct String
  {
    String(const char* s) : _string(s) {}

    std::string _string;
  };
}

#endif //KR_LUA_H
