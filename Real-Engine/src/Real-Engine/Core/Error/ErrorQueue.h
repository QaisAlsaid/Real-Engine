#ifndef ERROR_QUEUE_H
#define ERROR_QUEUE_H

#include "Error.h"

#include <queue>


namespace Real
{
  class REAL_API ErrorQueue
  {
  public:
    static inline Error& getFirstError() { return s_error_queue.size() ? s_error_queue.front() : s_no_error; }
    static inline Error& getLastError() { return s_error_queue.size() ? s_error_queue.back() : s_no_error; }
    static inline void pushError(const Error& e) { s_error_queue.push(e); }
    static inline void emplaceError(const std::string& m, bool n, bool h, bool f) { s_error_queue.emplace(m, n, h, f); }
    static inline void popError() { if(s_error_queue.size()) s_error_queue.pop(); }
  private:
    static std::queue<Error> s_error_queue;
    static Error s_no_error;
  };
}

#endif //ERROR_QUEUE_H
