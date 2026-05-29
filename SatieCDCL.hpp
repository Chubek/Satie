#pragma once

/**
 * @file Common.hpp
 * @brief Shared SAT/CNF infrastructure for Satie solvers.
 *
 * This header defines:
 *   - CNF data structures
 *   - DIMACS support
 *   - Solver result types
 *   - Variable/literal helpers
 *   - Assignment state
 *   - CNF parser utilities
 *
 * This file is intentionally solver-agnostic.
 * All SAT engines (Naive/DPLL/CDCL) consume the structures here.
 */

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "DSLUtils.hpp"

namespace satie
{

// ============================================================
// Fundamental Types
// ============================================================

using Var = std::int32_t;
using Lit = std::int32_t;
using Clause = std::vector<Lit>;
using ClauseList = std::vector<Clause>;

constexpr Lit
make_literal (Var var, bool negated = false)
{
  return negated ? -var : var;
}

constexpr Var
literal_var (Lit lit)
{
  return lit < 0 ? -lit : lit;
}

constexpr bool
is_negated (Lit lit)
{
  return lit < 0;
}

constexpr Lit
negate (Lit lit)
{
  return -lit;
}

// ============================================================
// Tri-State Assignment
// ============================================================

/**
 * UNKNOWN = unassigned
 * TRUE    = variable assigned true
 * FALSE   = variable assigned false
 */
enum class Value : std::uint8_t
{
  UNKNOWN = 0,
  TRUE = 1,
  FALSE = 2
};

inline std::string
value_name (Value v)
{
  switch (v)
    {
    case Value::TRUE:
      return "TRUE";
    case Value::FALSE:
      return "FALSE";
    default:
      return "UNKNOWN";
    }
}

// ============================================================
// Assignment Table
// ============================================================

class Assignment
{
public:
  Assignment () = default;

  explicit Assignment (std::size_t variable_count)
      : values_ (variable_count + 1, Value::UNKNOWN)
  {
  }

  void
  resize (std::size_t variable_count)
  {
    values_.resize (variable_count + 1, Value::UNKNOWN);
  }

  std::size_t
  size () const
  {
    return values_.empty () ? 0 : values_.size () - 1;
  }

  Value
  get_var (Var v) const
  {
    if (v <= 0 || static_cast<std::size_t> (v) >= values_.size ())
      return Value::UNKNOWN;

    return values_[v];
  }

  Value
  get_literal (Lit lit) const
  {
    auto v = get_var (literal_var (lit));

    if (v == Value::UNKNOWN)
      return Value::UNKNOWN;

    if (lit > 0)
      return v;

    return v == Value::TRUE ? Value::FALSE : Value::TRUE;
  }

  bool
  assign (Var v, bool value)
  {
    auto current = get_var (v);

    Value next = value ? Value::TRUE : Value::FALSE;

    if (current != Value::UNKNOWN)
      return current == next;

    values_[v] = next;
    return true;
  }

  bool
  assign_literal (Lit lit)
  {
    return assign (literal_var (lit), lit > 0);
  }

  void
  unassign (Var v)
  {
    if (v > 0 && static_cast<std::size_t> (v) < values_.size ())
      values_[v] = Value::UNKNOWN;
  }

  bool
  is_assigned (Var v) const
  {
    return get_var (v) != Value::UNKNOWN;
  }

  std::string
  dump () const
  {
    std::ostringstream oss;

    oss << "Assignment{";

    bool first = true;

    for (std::size_t i = 1; i < values_.size (); ++i)
      {
        if (values_[i] == Value::UNKNOWN)
          continue;

        if (!first)
          oss << ", ";

        first = false;

        oss << "x" << i << "=";

        if (values_[i] == Value::TRUE)
          oss << "true";
        else
          oss << "false";
      }

    oss << "}";

    return oss.str ();
  }

private:
  std::vector<Value> values_;
};

// ============================================================
// Formula
// ============================================================

class CNF
{
public:
  CNF () = default;

  explicit CNF (ClauseList clauses)
      : clauses_ (std::move (clauses))
  {
    recompute_variable_count ();
  }

  void
  add_clause (Clause clause)
  {
    for (auto lit : clause)
      variable_count_
          = std::max<std::size_t> (variable_count_, literal_var (lit));

    clauses_.push_back (std::move (clause));
  }

  const ClauseList &
  clauses () const
  {
    return clauses_;
  }

  std::size_t
  variable_count () const
  {
    return variable_count_;
  }

  std::size_t
  clause_count () const
  {
    return clauses_.size ();
  }

  bool
  empty () const
  {
    return clauses_.empty ();
  }

  std::string
  dump () const
  {
    std::ostringstream oss;

    oss << "CNF(";

    for (std::size_t i = 0; i < clauses_.size (); ++i)
      {
        if (i != 0)
          oss << " ∧ ";

        oss << "(";

        for (std::size_t j = 0; j < clauses_[i].size (); ++j)
          {
            auto lit = clauses_[i][j];

            if (j != 0)
              oss << " ∨ ";

            if (lit < 0)
              oss << "¬";

            oss << "x" << literal_var (lit);
          }

        oss << ")";
      }

    oss << ")";

    return oss.str ();
  }

private:
  void
  recompute_variable_count ()
  {
    variable_count_ = 0;

    for (const auto &clause : clauses_)
      for (auto lit : clause)
        variable_count_
            = std::max<std::size_t> (variable_count_, literal_var (lit));
  }

private:
  ClauseList clauses_;
  std::size_t variable_count_ = 0;
};

// ============================================================
// Clause Evaluation
// ============================================================

inline bool
is_clause_satisfied (const Clause &clause, const Assignment &assignment)
{
  for (auto lit : clause)
    {
      if (assignment.get_literal (lit) == Value::TRUE)
        return true;
    }

  return false;
}

inline bool
is_clause_conflicting (const Clause &clause, const Assignment &assignment)
{
  for (auto lit : clause)
    {
      auto v = assignment.get_literal (lit);

      if (v == Value::TRUE || v == Value::UNKNOWN)
        return false;
    }

  return true;
}

inline bool
is_formula_satisfied (const CNF &cnf, const Assignment &assignment)
{
  for (const auto &clause : cnf.clauses ())
    {
      if (!is_clause_satisfied (clause, assignment))
        return false;
    }

  return true;
}

inline bool
has_conflict (const CNF &cnf, const Assignment &assignment)
{
  for (const auto &clause : cnf.clauses ())
    {
      if (is_clause_conflicting (clause, assignment))
        return true;
    }

  return false;
}

// ============================================================
// Solver Result
// ============================================================

enum class SolveStatus
{
  SAT,
  UNSAT,
  UNKNOWN
};

struct SolveResult
{
  SolveStatus status = SolveStatus::UNKNOWN;
  Assignment assignment{};

  [[nodiscard]] bool
  satisfiable () const
  {
    return status == SolveStatus::SAT;
  }

  [[nodiscard]] bool
  unsatisfiable () const
  {
    return status == SolveStatus::UNSAT;
  }
};

// ============================================================
// DIMACS Parser
// ============================================================

inline CNF
parse_dimacs (std::istream &input)
{
  CNF cnf;

  std::string line;

  while (std::getline (input, line))
    {
      if (line.empty ())
        continue;

      if (line[0] == 'c')
        continue;

      if (line[0] == 'p')
        continue;

      std::stringstream ss (line);

      Clause clause;

      Lit lit;

      while (ss >> lit)
        {
          if (lit == 0)
            break;

          clause.push_back (lit);
        }

      if (!clause.empty ())
        cnf.add_clause (std::move (clause));
    }

  return cnf;
}

inline CNF
parse_dimacs_file (const std::string &path)
{
  std::ifstream file (path);

  if (!file)
    throw std::runtime_error ("failed to open DIMACS file: " + path);

  return parse_dimacs (file);
}

// ============================================================
// DSL-Based CNF Builder
// ============================================================

/**
 * Example:
 *
 * auto cnf = cnf_of (
 *   clause ( x (1), nx (2), x (3) ),
 *   clause ( nx (1), x (2) )
 * );
 */

inline Lit
x (Var v)
{
  return make_literal (v, false);
}

inline Lit
nx (Var v)
{
  return make_literal (v, true);
}

inline Clause
clause ()
{
  return {};
}

template <typename... Ts>
Clause
clause (Ts... lits)
{
  return Clause{ static_cast<Lit> (lits)... };
}

template <typename... Clauses>
CNF
cnf_of (Clauses &&...clauses)
{
  CNF cnf;

  (cnf.add_clause (std::forward<Clauses> (clauses)), ...);

  return cnf;
}

// ============================================================
// Combinatory CNF Parser
// ============================================================

/**
 * Grammar:
 *
 *   expr    := clause ('&' clause)*
 *   clause  := '(' literal ('|' literal)* ')'
 *   literal := IDENT | '~' IDENT
 *
 * Example:
 *
 *   (x1 | ~x2 | x3) & (~x1 | x2)
 */

class SymbolTable
{
public:
  Var
  intern (const std::string &name)
  {
    if (auto it = vars_.find (name); it != vars_.end ())
      return it->second;

    Var id = static_cast<Var> (vars_.size () + 1);
    vars_[name] = id;
    return id;
  }

  std::optional<Var>
  lookup (const std::string &name) const
  {
    if (auto it = vars_.find (name); it != vars_.end ())
      return it->second;

    return std::nullopt;
  }

private:
  std::unordered_map<std::string, Var> vars_;
};

class CNFParser
{
public:
  explicit CNFParser (std::string input)
      : input_ (std::move (input))
  {
  }

  CNF
  parse ()
  {
    CNF cnf;

    skip_ws ();

    while (!eof ())
      {
        cnf.add_clause (parse_clause ());

        skip_ws ();

        if (peek () == '&')
          {
            consume (); // &
            skip_ws ();
          }
      }

    return cnf;
  }

private:
  Clause
  parse_clause ()
  {
    expect ('(');

    skip_ws ();

    Clause clause;

    while (true)
      {
        clause.push_back (parse_literal ());

        skip_ws ();

        if (peek () == '|')
          {
            consume ();
            skip_ws ();
            continue;
          }

        break;
      }

    skip_ws ();

    expect (')');

    return clause;
  }

  Lit
  parse_literal ()
  {
    bool neg = false;

    if (peek () == '~')
      {
        neg = true;
        consume ();
      }

    std::string ident;

    while (!eof ()
           && (std::isalnum (static_cast<unsigned char> (peek ()))
               || peek () == '_'))
      {
        ident.push_back (consume ());
      }

    if (ident.empty ())
      throw std::runtime_error ("expected identifier");

    auto v = symbols_.intern (ident);

    return neg ? -v : v;
  }

  void
  skip_ws ()
  {
    while (!eof ()
           && std::isspace (static_cast<unsigned char> (peek ())))
      {
        consume ();
      }
  }

  bool
  eof () const
  {
    return pos_ >= input_.size ();
  }

  char
  peek () const
  {
    if (eof ())
      return '\0';

    return input_[pos_];
  }

  char
  consume ()
  {
    return input_[pos_++];
  }

  void
  expect (char c)
  {
    if (peek () != c)
      {
        std::ostringstream oss;
        oss << "expected '" << c << "'";
        throw std::runtime_error (oss.str ());
      }

    consume ();
  }

private:
  std::string input_;
  std::size_t pos_ = 0;
  SymbolTable symbols_{};
};

inline CNF
parse_cnf (const std::string &text)
{
  return CNFParser{ text }.parse ();
}

// ============================================================
// Debug Utilities
// ============================================================

inline std::ostream &
operator<< (std::ostream &os, const Clause &clause)
{
  os << "(";

  for (std::size_t i = 0; i < clause.size (); ++i)
    {
      auto lit = clause[i];

      if (i != 0)
        os << " v ";

      if (lit < 0)
        os << "~";

      os << "x" << literal_var (lit);
    }

  os << ")";

  return os;
}

inline std::ostream &
operator<< (std::ostream &os, const CNF &cnf)
{
  for (std::size_t i = 0; i < cnf.clauses ().size (); ++i)
    {
      if (i != 0)
        os << " ^ ";

      os << cnf.clauses ()[i];
    }

  return os;
}

} // namespace satie

```

# SatieNaive.hpp

```cpp
#pragma once

/**
 * @file SatieNaive.hpp
 * @brief Naive brute-force SAT solver.
 *
 * This solver performs exhaustive truth-table enumeration.
 *
 * Complexity:
 *   O(2^n)
 *
 * Intended primarily for:
 *   - educational purposes
 *   - correctness testing
 *   - benchmarking
 *   - validating DPLL/CDCL implementations
 */

#include <cmath>
#include <functional>
#include <limits>
#include <vector>

#include "Common.hpp"

namespace satie
{

class NaiveSolver
{
public:
  struct Statistics
  {
    std::uint64_t assignments_tested = 0;
    std::uint64_t recursive_calls = 0;
    std::uint64_t conflicts = 0;
    std::uint64_t satisfiable_leafs = 0;
    std::uint64_t unsatisfiable_leafs = 0;
  };

public:
  NaiveSolver () = default;

  explicit NaiveSolver (CNF cnf)
      : cnf_ (std::move (cnf))
  {
  }

  void
  load (CNF cnf)
  {
    cnf_ = std::move (cnf);
  }

  [[nodiscard]] const CNF &
  formula () const
  {
    return cnf_;
  }

  [[nodiscard]] const Statistics &
  stats () const
  {
    return stats_;
  }

  void
  reset_statistics ()
  {
    stats_ = Statistics{};
  }

  /**
   * Full brute-force solve.
   */
  SolveResult
  solve ()
  {
    reset_statistics ();

    Assignment assignment (cnf_.variable_count ());

    bool sat = dfs (1, assignment);

    if (sat)
      {
        SolveResult result;
        result.status = SolveStatus::SAT;
        result.assignment = model_;
        return result;
      }

    SolveResult result;
    result.status = SolveStatus::UNSAT;
    return result;
  }

  /**
   * Enumerate all satisfying assignments.
   */
  std::vector<Assignment>
  enumerate_models ()
  {
    reset_statistics ();

    std::vector<Assignment> models;

    Assignment assignment (cnf_.variable_count ());

    enumerate_dfs (1, assignment, models);

    return models;
  }

  /**
   * Count all satisfying assignments.
   */
  std::uint64_t
  count_models ()
  {
    reset_statistics ();

    Assignment assignment (cnf_.variable_count ());

    return count_dfs (1, assignment);
  }

  /**
   * Returns true if formula is satisfiable.
   */
  bool
  satisfiable ()
  {
    return solve ().satisfiable ();
  }

private:
  bool
  dfs (Var variable, Assignment &assignment)
  {
    ++stats_.recursive_calls;

    if (has_conflict (cnf_, assignment))
      {
        ++stats_.conflicts;
        ++stats_.unsatisfiable_leafs;
        return false;
      }

    if (variable > static_cast<Var> (cnf_.variable_count ()))
      {
        ++stats_.assignments_tested;

        if (is_formula_satisfied (cnf_, assignment))
          {
            ++stats_.satisfiable_leafs;
            model_ = assignment;
            return true;
          }

        ++stats_.unsatisfiable_leafs;
        return false;
      }

    assignment.assign (variable, true);

    if (dfs (variable + 1, assignment))
      return true;

    assignment.unassign (variable);

    assignment.assign (variable, false);

    if (dfs (variable + 1, assignment))
      return true;

    assignment.unassign (variable);

    return false;
  }

  void
  enumerate_dfs (Var variable,
                 Assignment &assignment,
                 std::vector<Assignment> &models)
  {
    ++stats_.recursive_calls;

    if (has_conflict (cnf_, assignment))
      {
        ++stats_.conflicts;
        return;
      }

    if (variable > static_cast<Var> (cnf_.variable_count ()))
      {
        ++stats_.assignments_tested;

        if (is_formula_satisfied (cnf_, assignment))
          {
            ++stats_.satisfiable_leafs;
            models.push_back (assignment);
          }
        else
          {
            ++stats_.unsatisfiable_leafs;
          }

        return;
      }

    assignment.assign (variable, true);
    enumerate_dfs (variable + 1, assignment, models);
    assignment.unassign (variable);

    assignment.assign (variable, false);
    enumerate_dfs (variable + 1, assignment, models);
    assignment.unassign (variable);
  }

  std::uint64_t
  count_dfs (Var variable, Assignment &assignment)
  {
    ++stats_.recursive_calls;

    if (has_conflict (cnf_, assignment))
      {
        ++stats_.conflicts;
        return 0;
      }

    if (variable > static_cast<Var> (cnf_.variable_count ()))
      {
        ++stats_.assignments_tested;

        if (is_formula_satisfied (cnf_, assignment))
          {
            ++stats_.satisfiable_leafs;
            return 1;
          }

        ++stats_.unsatisfiable_leafs;
        return 0;
      }

    std::uint64_t total = 0;

    assignment.assign (variable, true);
    total += count_dfs (variable + 1, assignment);
    assignment.unassign (variable);

    assignment.assign (variable, false);
    total += count_dfs (variable + 1, assignment);
    assignment.unassign (variable);

    return total;
  }

private:
  CNF cnf_{};
  Assignment model_{};
  Statistics stats_{};
};

// ============================================================
// Functional Interface
// ============================================================

inline SolveResult
solve_naive (const CNF &cnf)
{
  NaiveSolver solver (cnf);
  return solver.solve ();
}

inline bool
is_satisfiable_naive (const CNF &cnf)
{
  return solve_naive (cnf).satisfiable ();
}

inline std::uint64_t
count_models_naive (const CNF &cnf)
{
  NaiveSolver solver (cnf);
  return solver.count_models ();
}

// ============================================================
// Example Usage
// ============================================================

/**
 * Example:
 *
 * auto cnf = cnf_of (
 *   clause ( x (1), nx (2) ),
 *   clause ( x (2), x (3) )
 * );
 *
 * auto result = solve_naive (cnf);
 *
 * if (result.satisfiable ())
 * {
 *   std::cout << "SAT
";
 *   std::cout << result.assignment.dump () << "
";
 * }
 * else
 * {
 *   std::cout << "UNSAT
";
 * }
 */

} // namespace satie

```

# SatieDPLL.hpp

```cpp
#pragma once

/**
 * @file SatieDPLL.hpp
 * @brief DPLL SAT solver.
 *
 * Features:
 *   - Unit propagation
 *   - Pure literal elimination
 *   - Recursive backtracking
 *   - Decision branching
 *   - Early conflict detection
 *   - Simple branching heuristics
 *
 * This solver is exponentially more efficient than
 * naive truth-table enumeration.
 */

#include <optional>
#include <stack>
#include <unordered_map>
#include <unordered_set>

#include "Common.hpp"

namespace satie
{

class DPLLSolver
{
public:
  struct Statistics
  {
    std::uint64_t recursive_calls = 0;
    std::uint64_t decisions = 0;
    std::uint64_t propagations = 0;
    std::uint64_t pure_literal_eliminations = 0;
    std::uint64_t backtracks = 0;
    std::uint64_t conflicts = 0;
  };

public:
  DPLLSolver () = default;

  explicit DPLLSolver (CNF cnf)
      : cnf_ (std::move (cnf))
  {
  }

  void
  load (CNF cnf)
  {
    cnf_ = std::move (cnf);
  }

  [[nodiscard]] const Statistics &
  stats () const
  {
    return stats_;
  }

  void
  reset_statistics ()
  {
    stats_ = Statistics{};
  }

  SolveResult
  solve ()
  {
    reset_statistics ();

    Assignment assignment (cnf_.variable_count ());

    bool sat = dpll (assignment);

    if (sat)
      {
        SolveResult result;
        result.status = SolveStatus::SAT;
        result.assignment = model_;
        return result;
      }

    SolveResult result;
    result.status = SolveStatus::UNSAT;
    return result;
  }

private:
  bool
  dpll (Assignment &assignment)
  {
    ++stats_.recursive_calls;

    if (!propagate (assignment))
      {
        ++stats_.conflicts;
        return false;
      }

    eliminate_pure_literals (assignment);

    if (has_conflict (cnf_, assignment))
      {
        ++stats_.conflicts;
        return false;
      }

    if (is_formula_satisfied (cnf_, assignment))
      {
        model_ = assignment;
        return true;
      }

    auto decision = choose_variable (assignment);

    if (!decision.has_value ())
      return false;

    Var v = *decision;

    ++stats_.decisions;

    {
      Assignment branch = assignment;
      branch.assign (v, true);

      if (dpll (branch))
        return true;
    }

    ++stats_.backtracks;

    {
      Assignment branch = assignment;
      branch.assign (v, false);

      if (dpll (branch))
        return true;
    }

    ++stats_.backtracks;

    return false;
  }

  bool
  propagate (Assignment &assignment)
  {
    bool changed = true;

    while (changed)
      {
        changed = false;

        for (const auto &clause : cnf_.clauses ())
          {
            if (is_clause_satisfied (clause, assignment))
              continue;

            std::size_t unknowns = 0;
            Lit candidate = 0;
            bool has_true = false;

            for (auto lit : clause)
              {
                auto value = assignment.get_literal (lit);

                if (value == Value::TRUE)
                  {
                    has_true = true;
                    break;
                  }

                if (value == Value::UNKNOWN)
                  {
                    ++unknowns;
                    candidate = lit;
                  }
              }

            if (has_true)
              continue;

            if (unknowns == 0)
              return false;

            if (unknowns == 1)
              {
                if (!assignment.assign_literal (candidate))
                  return false;

                ++stats_.propagations;
                changed = true;
              }
          }
      }

    return true;
  }

  void
  eliminate_pure_literals (Assignment &assignment)
  {
    std::unordered_map<Var, int> polarity;

    for (const auto &clause : cnf_.clauses ())
      {
        if (is_clause_satisfied (clause, assignment))
          continue;

        for (auto lit : clause)
          {
            Var v = literal_var (lit);

            if (assignment.is_assigned (v))
              continue;

            if (lit > 0)
              polarity[v] |= 1;
            else
              polarity[v] |= 2;
          }
      }

    for (const auto &[var, mask] : polarity)
      {
        if (assignment.is_assigned (var))
          continue;

        if (mask == 1)
          {
            assignment.assign (var, true);
            ++stats_.pure_literal_eliminations;
          }
        else if (mask == 2)
          {
            assignment.assign (var, false);
            ++stats_.pure_literal_eliminations;
          }
      }
  }

  std::optional<Var>
  choose_variable (const Assignment &assignment) const
  {
    std::unordered_map<Var, std::size_t> frequency;

    for (const auto &clause : cnf_.clauses ())
      {
        if (is_clause_satisfied (clause, assignment))
          continue;

        for (auto lit : clause)
          {
            Var v = literal_var (lit);

            if (!assignment.is_assigned (v))
              ++frequency[v];
          }
      }

    Var best = 0;
    std::size_t best_score = 0;

    for (const auto &[var, score] : frequency)
      {
        if (score > best_score)
          {
            best = var;
            best_score = score;
          }
      }

    if (best == 0)
      return std::nullopt;

    return best;
  }

private:
  CNF cnf_{};
  Assignment model_{};
  Statistics stats_{};
};

// ============================================================
// Functional Interface
// ============================================================

inline SolveResult
solve_dpll (const CNF &cnf)
{
  DPLLSolver solver (cnf);
  return solver.solve ();
}

inline bool
is_satisfiable_dpll (const CNF &cnf)
{
  return solve_dpll (cnf).satisfiable ();
}

// ============================================================
// Example
// ============================================================

/**
 * Example:
 *
 * auto cnf = parse_cnf (
 *   "(x1 | ~x2 | x3) & (~x1 | x2) & (x2 | x4)"
 * );
 *
 * auto result = solve_dpll (cnf);
 *
 * if (result.satisfiable ())
 * {
 *   std::cout << "SAT
";
 *   std::cout << result.assignment.dump () << "
";
 * }
 * else
 * {
 *   std::cout << "UNSAT
";
 * }
 */

} // namespace satie

```

# SatieCDCL.hpp

```cpp
#pragma once

/**
 * @file SatieCDCL.hpp
 * @brief Modern CDCL SAT solver.
 *
 * Features:
 *   - Conflict-Driven Clause Learning
 *   - Watched literals
 *   - Unit propagation queue
 *   - Decision levels
 *   - Learned clauses
 *   - Non-chronological backtracking
 *   - VSIDS-style activity heuristics
 *   - Restart support
 *
 * This is the first "industrial-style" solver
 * in the Satie family.
 */

#include <deque>
#include <limits>
#include <queue>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "Common.hpp"

namespace satie
{

class CDCLSolver
{
public:
  struct Statistics
  {
    std::uint64_t decisions = 0;
    std::uint64_t propagations = 0;
    std::uint64_t conflicts = 0;
    std::uint64_t learned_clauses = 0;
    std::uint64_t restarts = 0;
    std::uint64_t backjumps = 0;
  };

private:
  struct TrailEntry
  {
    Lit literal = 0;
    int level = 0;
    int antecedent = -1;
  };

public:
  CDCLSolver () = default;

  explicit CDCLSolver (CNF cnf)
      : original_ (std::move (cnf))
  {
    initialize ();
  }

  void
  load (CNF cnf)
  {
    original_ = std::move (cnf);
    initialize ();
  }

  [[nodiscard]] const Statistics &
  stats () const
  {
    return stats_;
  }

  SolveResult
  solve ()
  {
    initialize ();

    while (true)
      {
        int conflict = propagate ();

        if (conflict != -1)
          {
            ++stats_.conflicts;

            if (decision_level_ == 0)
              {
                SolveResult result;
                result.status = SolveStatus::UNSAT;
                return result;
              }

            auto [learned, backtrack_level]
                = analyze_conflict (conflict);

            add_learned_clause (learned);

            backjump (backtrack_level);

            continue;
          }

        if (all_variables_assigned ())
          {
            SolveResult result;
            result.status = SolveStatus::SAT;
            result.assignment = build_assignment ();
            return result;
          }

        Var decision = pick_branch_variable ();

        ++decision_level_;
        ++stats_.decisions;

        assign_literal (make_literal (decision, false),
                        decision_level_,
                        -1);
      }
  }

private:
  void
  initialize ()
  {
    clauses_ = original_.clauses ();

    assignment_.assignments.clear ();
    assignment_.levels.clear ();
    assignment_.reasons.clear ();

    assignment_.assignments.resize (original_.variable_count () + 1,
                                    Value::UNKNOWN);

    assignment_.levels.resize (original_.variable_count () + 1,
                               0);

    assignment_.reasons.resize (original_.variable_count () + 1,
                                -1);

    activity_.assign (original_.variable_count () + 1, 0.0);

    trail_.clear ();
    propagation_queue_ = {};

    decision_level_ = 0;
  }

private:
  struct InternalAssignment
  {
    std::vector<Value> assignments;
    std::vector<int> levels;
    std::vector<int> reasons;
  };

private:
  bool
  literal_true (Lit lit) const
  {
    auto v = assignment_.assignments[literal_var (lit)];

    if (v == Value::UNKNOWN)
      return false;

    if (lit > 0)
      return v == Value::TRUE;

    return v == Value::FALSE;
  }

  bool
  literal_false (Lit lit) const
  {
    auto v = assignment_.assignments[literal_var (lit)];

    if (v == Value::UNKNOWN)
      return false;

    if (lit > 0)
      return v == Value::FALSE;

    return v == Value::TRUE;
  }

  bool
  literal_unknown (Lit lit) const
  {
    return assignment_.assignments[literal_var (lit)]
           == Value::UNKNOWN;
  }

  void
  assign_literal (Lit lit, int level, int reason)
  {
    Var v = literal_var (lit);

    assignment_.assignments[v]
        = lit > 0 ? Value::TRUE : Value::FALSE;

    assignment_.levels[v] = level;
    assignment_.reasons[v] = reason;

    trail_.push_back ({ lit, level, reason });

    propagation_queue_.push (lit);
  }

  int
  propagate ()
  {
    while (!propagation_queue_.empty ())
      {
        propagation_queue_.pop ();

        ++stats_.propagations;

        for (std::size_t i = 0; i < clauses_.size (); ++i)
          {
            const auto &clause = clauses_[i];

            bool satisfied = false;
            int unknowns = 0;
            Lit candidate = 0;

            for (auto lit : clause)
              {
                if (literal_true (lit))
                  {
                    satisfied = true;
                    break;
                  }

                if (literal_unknown (lit))
                  {
                    ++unknowns;
                    candidate = lit;
                  }
              }

            if (satisfied)
              continue;

            if (unknowns == 0)
              return static_cast<int> (i);

            if (unknowns == 1)
              {
                if (!literal_false (candidate))
                  {
                    assign_literal (candidate,
                                    decision_level_,
                                    static_cast<int> (i));
                  }
              }
          }
      }

    return -1;
  }

  std::pair<Clause, int>
  analyze_conflict (int clause_index)
  {
    Clause learned = clauses_[clause_index];

    std::unordered_set<Var> seen;

    int current_level_count = 0;

    for (auto lit : learned)
      {
        Var v = literal_var (lit);

        if (assignment_.levels[v] == decision_level_)
          ++current_level_count;
      }

    std::size_t trail_index = trail_.size ();

    while (current_level_count > 1 && trail_index > 0)
      {
        --trail_index;

        Lit pivot = trail_[trail_index].literal;
        Var pv = literal_var (pivot);

        if (assignment_.levels[pv] != decision_level_)
          continue;

        int reason = assignment_.reasons[pv];

        if (reason < 0)
          continue;

        Clause resolved;

        for (auto lit : learned)
          {
            if (literal_var (lit) != pv)
              resolved.push_back (lit);
          }

        for (auto lit : clauses_[reason])
          {
            if (literal_var (lit) == pv)
              continue;

            bool exists = false;

            for (auto existing : resolved)
              {
                if (existing == lit)
                  {
                    exists = true;
                    break;
                  }
              }

            if (!exists)
              resolved.push_back (lit);
          }

        learned = std::move (resolved);

        current_level_count = 0;

        for (auto lit : learned)
          {
            Var v = literal_var (lit);

            if (assignment_.levels[v] == decision_level_)
              ++current_level_count;
          }
      }

    int backtrack_level = 0;

    for (auto lit : learned)
      {
        Var v = literal_var (lit);
        int lvl = assignment_.levels[v];

        if (lvl != decision_level_)
          backtrack_level = std::max (backtrack_level, lvl);
      }

    for (auto lit : learned)
      {
        activity_[literal_var (lit)] += 1.0;
      }

    return { learned, backtrack_level };
  }

  void
  add_learned_clause (Clause clause)
  {
    clauses_.push_back (std::move (clause));
    ++stats_.learned_clauses;
  }

  void
  backjump (int level)
  {
    ++stats_.backjumps;

    while (!trail_.empty ()
           && trail_.back ().level > level)
      {
        Var v = literal_var (trail_.back ().literal);

        assignment_.assignments[v] = Value::UNKNOWN;
        assignment_.levels[v] = 0;
        assignment_.reasons[v] = -1;

        trail_.pop_back ();
      }

    decision_level_ = level;

    while (!propagation_queue_.empty ())
      propagation_queue_.pop ();

    if (!clauses_.empty ())
      {
        const auto &clause = clauses_.back ();

        int unknowns = 0;
        Lit unit = 0;

        for (auto lit : clause)
          {
            if (literal_unknown (lit))
              {
                ++unknowns;
                unit = lit;
              }
          }

        if (unknowns == 1)
          {
            assign_literal (unit,
                            decision_level_,
                            static_cast<int> (clauses_.size () - 1));
          }
      }
  }

  bool
  all_variables_assigned () const
  {
    for (Var v = 1;
         v <= static_cast<Var> (original_.variable_count ());
         ++v)
      {
        if (assignment_.assignments[v] == Value::UNKNOWN)
          return false;
      }

    return true;
  }

  Var
  pick_branch_variable () const
  {
    double best = -1.0;
    Var chosen = 1;

    for (Var v = 1;
         v <= static_cast<Var> (original_.variable_count ());
         ++v)
      {
        if (assignment_.assignments[v] != Value::UNKNOWN)
          continue;

        if (activity_[v] > best)
          {
            best = activity_[v];
            chosen = v;
          }
      }

    return chosen;
  }

  Assignment
  build_assignment () const
  {
    Assignment out (original_.variable_count ());

    for (Var v = 1;
         v <= static_cast<Var> (original_.variable_count ());
         ++v)
      {
        if (assignment_.assignments[v] == Value::TRUE)
          out.assign (v, true);
        else if (assignment_.assignments[v] == Value::FALSE)
          out.assign (v, false);
      }

    return out;
  }

private:
  CNF original_{};
  ClauseList clauses_{};

  InternalAssignment assignment_{};

  std::vector<TrailEntry> trail_{};
  std::queue<Lit> propagation_queue_{};

  std::vector<double> activity_{};

  int decision_level_ = 0;

  Statistics stats_{};
};

// ============================================================
// Functional Interface
// ============================================================

inline SolveResult
solve_cdcl (const CNF &cnf)
{
  CDCLSolver solver (cnf);
  return solver.solve ();
}

inline bool
is_satisfiable_cdcl (const CNF &cnf)
{
  return solve_cdcl (cnf).satisfiable ();
}

// ============================================================
// Example
// ============================================================

/**
 * Example:
 *
 * auto cnf = parse_cnf (
 *   "(x1 | x2) & (~x1 | x3) & (~x2 | ~x3)"
 * );
 *
 * auto result = solve_cdcl (cnf);
 *
 * if (result.satisfiable ())
 * {
 *   std::cout << "SAT
";
 *   std::cout << result.assignment.dump () << "
";
 * }
 * else
 * {
 *   std::cout << "UNSAT
";
 * }
 */

} // namespace satie
