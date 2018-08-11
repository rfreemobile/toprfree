
#include "pfp-local/pfp_memory.hpp"

namespace stdplus {


const char* ptr_only_init_error_init_again::what() const noexcept {
 return "Object of ptr_only_init was again initialized (set to some address) - which is not allowed.";
}

void ptr_only_init_error_init_again::throw_self() { throw ptr_only_init_error_init_again(); }

} // namespace


