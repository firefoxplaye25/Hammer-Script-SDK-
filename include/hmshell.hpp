#ifndef HAMMER_HMSHELL_HPP
#define HAMMER_HMSHELL_HPP

#include "environment.hpp"
#include <memory>

namespace Hammer {

void registerHmShell(std::shared_ptr<Environment> env);

} // namespace Hammer

#endif // HAMMER_HMSHELL_HPP
