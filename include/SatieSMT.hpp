#pragma once

#include <string>
#include <utility>
#include "SatieCDCL.hpp"

namespace satie::smt
{

enum class Logic
{
  QF_UF,
  QF_LIA,
  QF_BV,
  QF_FP,
  QF_NRA,
  ALL
};

/// Generic SMT façade for Booleanized constraints.
class SMTSolver
{
public:
  SMTSolver () = default;
  explicit SMTSolver (Logic logic) : logic_ (logic) {}
  explicit SMTSolver (CNF cnf, Logic logic = Logic::ALL)
      : logic_ (logic), cnf_ (std::move (cnf))
  {
  }

  void set_logic (Logic logic) noexcept { logic_ = logic; }
  Logic logic () const noexcept { return logic_; }
  void load (CNF cnf) { cnf_ = std::move (cnf); }
  const CNF &problem () const noexcept { return cnf_; }
  SolveResult check () const { return solve_cdcl (cnf_); }

private:
  Logic logic_ = Logic::ALL;
  CNF cnf_{};
};

} // namespace satie::smt
