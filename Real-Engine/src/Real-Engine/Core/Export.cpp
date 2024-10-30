#include "Export.h"


namespace Real
{
  ExportedMap Export::s_export_vars;
  
  void Export::pushExportVariable(const char *as, const ExportType &et, UUID e_id)
  {
    s_export_vars[e_id][as] = et; 
  }

  ExportedVector* Export::getExportVariablesFor(UUID id)
  {
    if(s_export_vars.find(id) != s_export_vars.end())
    {
      return &s_export_vars.at(id);
    }
    return nullptr;
  }
}
