# Satie: Small, Header-Only, Multi-Algorithm SAT Solver

**This library has been created with the help of GPT 3.5 Codex, if you have moral quandries against using AI-generated libraries, this is your time to close this tab, and use another minimal library like PicoSAT or MiniSAT**.

Were Satie and DeBussy friends? Surely, Satie loved DeBussy. 

Satie is a small-footprint, minimal, header-only library that implements a naive SAT solver, DPPL adn CDCL algorithms. CND and DIMAC are both provided in `Common.hpp`, and all three files import it.

A manual has been provided, which is as detiled as you wish for an AI-generated manual to be. But it's enough to teach you how to encode boolean variables using either the API provided by the three algorithms, DIMAC or CNF.

The precision of this library remains untested. I made Satie for my own use. But you are free to use it as well. The library is licensed under MIT, but technically, since this is AI-generated code, only God owns it.

I did not slop out this library. I scaffoldered the filess, with placholder comments and boilerplatecode. This library is 100% halal for use, even if you are anti-AI. This library is unique, you cannot find one like it. PicoSAT and MiniSAT both use oblique heuristics. I use actual, established algorithms.

SAT is, as you know, NP-hard, and thusly, NP-Complete. So don't expect a tiny library like this to be end-all, be-all in finding satisfiability in boolean formulas. I highliy recommend *The Handbook of Satisfiability* if you wanna brush up on your SAT.

I plan on adding GPU kernels to this library. I am looking for a small, header-only library that uses OpenCL to create kernels. If you know such library, my contacts are in my [Github frontpage](https://github.com/Chubek) along with all my projects. I separate my AI-generated slopware from the software I toiled over, but it's becoming harder and harder.

~ Thanks, Chubak
