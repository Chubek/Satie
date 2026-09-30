#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include "Common.hpp"

namespace satie::ode
{

/// SMT Theory Solver module for ODE.
class ODESolver
{
public:
  ODESolver () = default;

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "ODE";
  }

  SolveResult check () const
  {
    throw std::runtime_error ("SMT theory module 'ODE' solver is not yet fully implemented.");
  }
};

} // namespace satie::ode
