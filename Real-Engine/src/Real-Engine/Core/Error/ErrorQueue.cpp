#include "ErrorQueue.h"


namespace Real
{
  std::queue<Error> ErrorQueue::s_error_queue;
  Error ErrorQueue::s_no_error;
}
