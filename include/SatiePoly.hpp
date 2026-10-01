#pragma once

#include <string>
#include <string_view>
#include "SatieCDCL.hpp"

namespace satie::poly
{

/// SMT Theory Solver module for Poly.
class PolySolver
{
public:
  PolySolver () = default;
  explicit PolySolver (CNF cnf) : cnf_ (std::move (cnf)) {}

  [[nodiscard]] static constexpr std::string_view theory_name () noexcept
  {
    return "Poly";
  }

  void load (CNF cnf) { cnf_ = std::move (cnf); }
  const CNF &problem () const noexcept { return cnf_; }
  SolveResult check () const { return solve_cdcl (cnf_); }

private:
  CNF cnf_{};
};

} // namespace satie::poly
