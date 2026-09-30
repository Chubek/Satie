#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include "Common.hpp"

namespace satie::quant
{

/// SMT Theory Solver module for Quant.
class QuantSolver
{
public:
  QuantSolver () = default;

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "Quant";
  }

  SolveResult check () const
  {
    throw std::runtime_error ("SMT theory module 'Quant' solver is not yet fully implemented.");
  }
};

} // namespace satie::quant
