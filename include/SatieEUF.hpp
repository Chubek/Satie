#pragma once

#include <string>
#include <string_view>
#include "SatieCDCL.hpp"

namespace satie::euf
{

/// SMT Theory Solver module for EUF.
class EUFSolver
{
public:
  EUFSolver () = default;
  explicit EUFSolver (CNF cnf) : cnf_ (std::move (cnf)) {}

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "EUF";
  }

  void load (CNF cnf) { cnf_ = std::move (cnf); }
  const CNF &problem () const noexcept { return cnf_; }
  SolveResult check () const { return solve_cdcl (cnf_); }

private:
  CNF cnf_{};
};

} // namespace satie::euf
