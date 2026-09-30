#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include "Common.hpp"

namespace satie::euf
{

/// SMT Theory Solver module for EUF.
class EUFSolver
{
public:
  EUFSolver () = default;

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "EUF";
  }

  SolveResult check () const
  {
    throw std::runtime_error ("SMT theory module 'EUF' solver is not yet fully implemented.");
  }
};

} // namespace satie::euf
