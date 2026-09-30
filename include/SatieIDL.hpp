#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include "Common.hpp"

namespace satie::idl
{

/// SMT Theory Solver module for IDL.
class IDLSolver
{
public:
  IDLSolver () = default;

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "IDL";
  }

  SolveResult check () const
  {
    throw std::runtime_error ("SMT theory module 'IDL' solver is not yet fully implemented.");
  }
};

} // namespace satie::idl
