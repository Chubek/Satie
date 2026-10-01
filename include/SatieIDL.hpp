#pragma once

#include <string>
#include <string_view>
#include "SatieCDCL.hpp"

namespace satie::idl
{

/// SMT Theory Solver module for IDL.
class IDLSolver
{
public:
  IDLSolver () = default;
  explicit IDLSolver (CNF cnf) : cnf_ (std::move (cnf)) {}

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "IDL";
  }

  void load (CNF cnf) { cnf_ = std::move (cnf); }
  const CNF &problem () const noexcept { return cnf_; }
  SolveResult check () const { return solve_cdcl (cnf_); }

private:
  CNF cnf_{};
};

} // namespace satie::idl
