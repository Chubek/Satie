#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include "Common.hpp"

namespace satie::nra
{

/// SMT Theory Solver module for NRA.
class NRASolver
{
public:
  NRASolver () = default;

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "NRA";
  }

  SolveResult check () const
  {
    throw std::runtime_error ("SMT theory module 'NRA' solver is not yet fully implemented.");
  }
};

} // namespace satie::nra
