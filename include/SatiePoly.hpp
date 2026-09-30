#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include "Common.hpp"

namespace satie::poly
{

/// SMT Theory Solver module for Poly.
class PolySolver
{
public:
  PolySolver () = default;

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "Poly";
  }

  SolveResult check () const
  {
    throw std::runtime_error ("SMT theory module 'Poly' solver is not yet fully implemented.");
  }
};

} // namespace satie::poly
