#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include "Common.hpp"

namespace satie::adt
{

/// SMT Theory Solver module for ADT.
class ADTSolver
{
public:
  ADTSolver () = default;

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "ADT";
  }

  SolveResult check () const
  {
    throw std::runtime_error ("SMT theory module 'ADT' solver is not yet fully implemented.");
  }
};

} // namespace satie::adt
