#include "catch_shim.hpp"
#include "Satie.hpp"
#include "SatieSAT.hpp"

#include <cstdint>
#include <sstream>
#include <string>

using namespace satie;

static void check_model (const CNF &cnf, const SolveResult &result)
{
  if (result.satisfiable ())
    REQUIRE (is_formula_satisfied (cnf, result.assignment));
}

TEST_CASE ("SAT engines handle empty, empty clause, units, and contradictions")
{
  for (Engine engine : { Engine::Native, Engine::DPLL, Engine::CDCL })
    {
      for (const CNF &cnf : { CNF{}, CNF ({{ 1 }}), CNF ({{ 1, 1, -1 }, { 2 }}) })
        {
          SolveResult result = solve (cnf, engine);
          REQUIRE (result.satisfiable ());
          check_model (cnf, result);
        }
      for (const CNF &cnf : { CNF ({{}}), CNF ({{ 1 }, { -1 }}) })
        REQUIRE (solve (cnf, engine).unsatisfiable ());
    }
}

TEST_CASE ("SAT engines branch and agree on 128 reproducible formulas")
{
  std::uint32_t seed = 71;
  auto next = [&seed] () { return seed = seed * 1664525u + 1013904223u; };
  for (int instance = 0; instance < 128; ++instance)
    {
      CNF cnf;
      int clause_count = 3 + static_cast<int> (next () % 11);
      for (int i = 0; i < clause_count; ++i)
        {
          Clause clause;
          int length = 1 + static_cast<int> (next () % 3);
          for (int j = 0; j < length; ++j)
            {
              Lit var = 1 + static_cast<Lit> (next () % 4);
              clause.push_back ((next () & 1u) ? var : -var);
            }
          cnf.add_clause (std::move (clause));
        }
      SolveResult expected = solve (cnf, Engine::Native);
      check_model (cnf, expected);
      for (Engine engine : { Engine::DPLL, Engine::CDCL })
        {
          SolveResult actual = solve (cnf, engine);
          REQUIRE (actual.status == expected.status);
          check_model (cnf, actual);
        }
    }
}

TEST_CASE ("CDCL counts root conflicts and resets statistics on each solve")
{
  CDCLSolver solver (CNF ({{ 1 }, { -1 }}));
  REQUIRE (solver.solve ().unsatisfiable ());
  REQUIRE (solver.statistics ().conflicts == 1);
  REQUIRE (solver.solve ().unsatisfiable ());
  REQUIRE (solver.statistics ().conflicts == 1);

  solver.load (CNF ({{ 1 }}));
  SolveResult sat = solver.solve ();
  REQUIRE (sat.satisfiable ());
  REQUIRE (sat.assignment.get_var (1) == Value::TRUE);
  REQUIRE (solver.statistics ().conflicts == 0);
}

TEST_CASE ("CDCL learns and backjumps from branching conflicts")
{
  CNF cnf ({{ 1, 2 }, { 1, -2 }, { -1, 2 }, { -1, -2 }});
  CDCLSolver solver (cnf);
  REQUIRE (solver.solve ().unsatisfiable ());
  REQUIRE (solver.statistics ().decisions > 0);
  REQUIRE (solver.statistics ().learned_clauses > 0);
  REQUIRE (solver.statistics ().backjumps > 0);
  REQUIRE (solver.solve ().unsatisfiable ());
  REQUIRE (solver.statistics ().learned_clauses > 0);
}

TEST_CASE ("DPLL resets state and models across solve and load")
{
  DPLLSolver solver (CNF ({{ 1, 2 }, { -1, 2 }, { 1, -2 }}));
  CNF cnf ({{ 1, 2 }, { -1, 2 }, { 1, -2 }});
  REQUIRE (is_formula_satisfied (cnf, solver.solve ().assignment));
  auto decisions = solver.statistics ().decisions;
  REQUIRE (is_formula_satisfied (cnf, solver.solve ().assignment));
  REQUIRE (solver.statistics ().decisions == decisions);
  solver.load (CNF ({{}}));
  REQUIRE (solver.solve ().unsatisfiable ());
  solver.load (CNF{});
  REQUIRE (solver.solve ().satisfiable ());
}

TEST_CASE ("DIMACS comments, terminators, and empty clauses flow through public API")
{
  CNF sat = parse ("c comment\np cnf 2 2\n1 -2 0\n2 0\n", ParseFormat::DIMACS);
  REQUIRE (sat.clause_count () == 2);
  SolveResult result = solve_sat (sat);
  REQUIRE (result.satisfiable ());
  check_model (sat, result);
  CNF unsat = parse ("p cnf 1 1\n0\n", ParseFormat::DIMACS);
  REQUIRE (solve_sat (unsat).unsatisfiable ());
  REQUIRE_THROWS_AS (parse ("p cnf 1 1\n1\n", ParseFormat::DIMACS), ParseError);
}

#include "test_theory.hpp"

SATIE_RUN_MAIN
