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

  explicit CNF (ClauseList clauses) : clauses_ (std::move (clauses))
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
  explicit CNFParser (std::string input) : input_ (std::move (input)) {}

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
    while (!eof () && std::isspace (static_cast<unsigned char> (peek ())))
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
