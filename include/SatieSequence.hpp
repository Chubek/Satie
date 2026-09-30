#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include "Common.hpp"

namespace satie::sequence
{

/// SMT Theory Solver module for Sequence.
class SequenceSolver
{
public:
  SequenceSolver () = default;

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "Sequence";
  }

  SolveResult check () const
  {
    throw std::runtime_error ("SMT theory module 'Sequence' solver is not yet fully implemented.");
  }
};

} // namespace satie::sequence
