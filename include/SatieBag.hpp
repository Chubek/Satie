#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include "Common.hpp"

namespace satie::bag
{

/// SMT Theory Solver module for Bag.
class BagSolver
{
public:
  BagSolver () = default;

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "Bag";
  }

  SolveResult check () const
  {
    throw std::runtime_error ("SMT theory module 'Bag' solver is not yet fully implemented.");
  }
};

} // namespace satie::bag
