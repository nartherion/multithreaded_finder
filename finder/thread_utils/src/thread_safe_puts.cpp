#include "thread_safe_puts.hpp"

namespace finder::thread_utils
{

std::recursive_mutex io_mx;

}