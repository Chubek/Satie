#pragma once

#include <string>
#include <string_view>
#include "SatieCDCL.hpp"

namespace satie::bag
{

/// SMT Theory Solver module for Bag.
class BagSolver
{
public:
  BagSolver () = default;
  explicit BagSolver (CNF cnf) : cnf_ (std::move (cnf)) {}

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "Bag";
  }

  void load (CNF cnf) { cnf_ = std::move (cnf); }
  const CNF &problem () const noexcept { return cnf_; }
  SolveResult check () const { return solve_cdcl (cnf_); }

private:
  CNF cnf_{};
};

} // namespace satie::bag
