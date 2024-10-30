#ifndef RE_ERROR_H
#define RE_ERROR_H

#include "Real-Engine/Core/Core.h"
#include <string>


namespace Real
{
  struct REAL_API Error
  {
    Error() = default;
    Error(const std::string& message, bool n, bool h, bool f)
      :message(message), need_handling(n), handled(h), fatal(f) {};

    std::string message;
    bool need_handling = false;
    bool handled = true;
    bool fatal = false;
  };
}

#endif //RE_ERROR_H
