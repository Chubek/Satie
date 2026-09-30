#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include "Common.hpp"

namespace satie::smt
{{

enum class Logic
{{
  QF_UF,
  QF_LIA,
  QF_BV,
  QF_FP,
  QF_NRA,
  ALL
}};

class SMTSolver
{{
public:
  SMTSolver () = default;
  explicit SMTSolver (Logic logic) : logic_ (logic) {}

  void set_logic (Logic logic) { logic_ = logic; }
  Logic logic () const { return logic_; }

  SolveResult check ()
  {
    throw std::runtime_error ("General SMT solving baseline: non-SAT theory integration unsupported.");
  }

private:
  Logic logic_ = Logic::ALL;
}};

}} // namespace satie::smt
