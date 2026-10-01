#pragma once

#include <string>
#include <string_view>
#include "SatieCDCL.hpp"

namespace satie::milp
{

/// SMT Theory Solver module for MILP.
class MILPSolver
{
public:
  MILPSolver () = default;
  explicit MILPSolver (CNF cnf) : cnf_ (std::move (cnf)) {}

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "MILP";
  }

  void load (CNF cnf) { cnf_ = std::move (cnf); }
  const CNF &problem () const noexcept { return cnf_; }
  SolveResult check () const { return solve_cdcl (cnf_); }

private:
  CNF cnf_{};
};

} // namespace satie::milp
