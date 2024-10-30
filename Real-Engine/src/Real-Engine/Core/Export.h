#ifndef EXPORT_H
#define EXPORT_H

#include "Core.h"
#include "Real-Engine/Core/Math/math.h"
#include "Real-Engine/Core/UUID.h"

namespace Real
{
  class REAL_API ExportType 
  {
  public:
    enum class Type 
    {
      None = 0, Float, Int, String, Vec4, Vec3, Vec2, RGBA_Color,
      Scene, Script, Texture, Asset, Entity
    };
    ExportType() = default;
    ExportType(Type T, void* data) 
      :m_data_type(T), m_raw_data(data) {} ;

    inline Type getType()     const { return m_data_type; }
    inline void* getRawData() const { return m_raw_data;  }

    template<typename T>
    T* getAs() const { return static_cast<T*>(m_raw_data); }
  private:
    Type  m_data_type = Type::None;
    void* m_raw_data = nullptr;
  };

  typedef std::pair<const char*, ExportType> ExportedPair;
  typedef std::unordered_map<const char*, ExportType> ExportedVector;
  typedef std::unordered_map<uint64_t, ExportedVector> ExportedMap; 
  
  class REAL_API Export 
  { 
  public:
    static void pushExportVariable(const char* as, const ExportType& et, UUID entity_id);
    static ExportedMap& getExportedVariables() { return s_export_vars; }
    static ExportedVector*    getExportVariablesFor(UUID id);
  private: 
    static ExportedMap s_export_vars;
  };
}


#endif //EXPORT_H
