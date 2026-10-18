#ifndef HAMMER_HAMSYS_HPP
#define HAMMER_HAMSYS_HPP

#include "environment.hpp"
#include <memory>

namespace Hammer {

void registerHamSys(std::shared_ptr<Environment> env);

} // namespace Hammer

#endif // HAMMER_HAMSYS_HPP
