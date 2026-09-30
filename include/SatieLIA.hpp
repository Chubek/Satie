#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include "Common.hpp"

namespace satie::lia
{

/// SMT Theory Solver module for LIA.
class LIASolver
{
public:
  LIASolver () = default;

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "LIA";
  }

  SolveResult check () const
  {
    throw std::runtime_error ("SMT theory module 'LIA' solver is not yet fully implemented.");
  }
};

} // namespace satie::lia
