#pragma once

#include <string>
#include <string_view>
#include "SatieCDCL.hpp"

namespace satie::ode
{

/// SMT Theory Solver module for ODE.
class ODESolver
{
public:
  ODESolver () = default;
  explicit ODESolver (CNF cnf) : cnf_ (std::move (cnf)) {}

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "ODE";
  }

  void load (CNF cnf) { cnf_ = std::move (cnf); }
  const CNF &problem () const noexcept { return cnf_; }
  SolveResult check () const { return solve_cdcl (cnf_); }

private:
  CNF cnf_{};
};

} // namespace satie::ode
