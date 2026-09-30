#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include "Common.hpp"

namespace satie::isl
{

/// SMT Theory Solver module for ISL.
class ISLSolver
{
public:
  ISLSolver () = default;

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "ISL";
  }

  SolveResult check () const
  {
    throw std::runtime_error ("SMT theory module 'ISL' solver is not yet fully implemented.");
  }
};

} // namespace satie::isl
