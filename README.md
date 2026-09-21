# code_MIRACLE_PBS

Unit-cost microbenchmark for the elliptic-curve and pairing-based
cryptographic primitives underlying **Table 4** of the CBAS-DSH manuscript
(JICT submission). This repository measures the seven per-operation
constants directly; it does **not** contain separate end-to-end
implementations of Hou et al. (2021), Verma et al. (2020), Qiao et al.
(2024), or CBAS-DSH.

## What this benchmarks

`bench_miracl_all_3.cpp` measures, with a 10-iteration warm-up followed by
timed repetition (N iterations per operation, see the `main()` constants):

| Symbol (code) | Symbol (Table 4) | Operation | Library |
|---|---|---|---|
| `T_bpo`  | Tbpo  | Bilinear pairing operation | PBC (Type A curve) |
| `T_pbsm` | Tbpsm | Pairing-based scalar multiplication (G1) | PBC |
| `T_pbpa` | Tbppa | Pairing-based point addition (G1) | PBC |
| `T_rc`   | Trc   | Modular inversion in Zr | PBC |
| `T_meo`  | Tmeo  | Modular exponentiation in Zr | PBC |
| `T_smec` | Tsmec | Scalar multiplication on secp256k1 | MIRACL |
| `T_paec` | Tpaec | Point addition on secp256k1 | MIRACL |

Each printed value is one run's arithmetic mean; the manuscript reports the
mean over 1,500 independent trials collected this way on a MacBook Air M2
(see the Performance Analysis subsection for the full hardware/software
specification).

## How Table 5 and Figures 3-5 relate to this code (read this before assuming a bug)

**Table 5 is not produced by running this code once per scheme.** Table 5's
Sign / Verify / AggSign / AggVerify entries for CBAS-DSH, Hou et al., Verma
et al., and Qiao et al. are computed **analytically**: each scheme's
published specification is used to count how many scalar multiplications,
point additions, and pairing operations its Sign/Verify/AggSign/AggVerify
algorithms require, and that count is multiplied by the constants this
repository measures. This is the same cost-model methodology the manuscript
already used, and explicitly labelled, for the multi-batch-size projection
in Table 6.

Worked example (reproducible by hand from this repo's own output):

```
Tsmec = 0.1809 ms, Tpaec = 0.0009 ms   (measured by this benchmark)

CBAS-DSH Verify     = 3*Tsmec + 3*Tpaec              = 0.5454 ms  (Table 5)
Qiao et al. Verify   = 4*Tsmec + 3*Tpaec              = 0.7263 ms  (Table 5)
CBAS-DSH AggVerify   = (3n+2)*Tsmec + 3n*Tpaec, n=100 = 54.9018 ms (Table 5)
Qiao et al. AggVerify = 4n*Tsmec + (3n+3)*Tpaec, n=100 = 72.6327 ms (Table 5)
```

Both reproduce Table 5 exactly given this repository's measured constants.
If you re-run this benchmark on different hardware, your `T_smec`/`T_paec`
will differ from 0.1809/0.0009, and recomputing the formulas above with
your own constants is the correct way to sanity-check Table 5/6 on your
machine -- not expecting this program to print Table 5's numbers directly,
since it never computes them.

## Build instructions

Requires the MIRACL Core library and PBC (Pairing-Based Crypto) library.

```bash
# MIRACL (64-bit build)
git clone https://github.com/miracl/MIRACL.git
cd MIRACL/source
cp ../include/mirdef.h64 mirdef.h
cp ../include/{miracl.h,ecn.h,big.h} .
# compile the core .c files and ecn.cpp/big.cpp per lib/linux64 in the MIRACL repo,
# producing miracl.a

# PBC
git clone https://github.com/blynn/pbc.git
cd pbc && autoreconf -f -i && ./configure && make && sudo make install

# This benchmark
g++ -O2 -I<miracl>/source -I/usr/local/include \
    -o bench_run bench_miracl_all_3.cpp <miracl>/source/miracl.a \
    -L/usr/local/lib -lpbc -lgmp -Wl,-rpath,/usr/local/lib
./bench_run
```

`ecn.h` cannot be included from C++ alongside `miracl.h` without wrapping
`miracl.h` in `extern "C" { ... }` (the C API used here does not need
`ecn.h`'s C++ `ECn` class at all, so it can simply be omitted from the
include list).

## Reproducibility notes

- Absolute millisecond values are hardware-dependent; only the *relative*
  comparison between schemes and the *linear scaling in n* are
  hardware-independent properties this benchmark supports.
- This repository was independently compiled and executed as part of peer
  review, on hardware unrelated to the authors' own machine, confirming the
  code compiles cleanly and the relative CBAS-DSH-vs-Qiao improvement
  (~24% at n=100) reproduces closely (19.5%-24.8% across n=10..500) despite
  very different absolute timings. See the manuscript's Performance
  Analysis subsection for the full account.
- No raw per-trial CSV is currently included in this repository (only the
  summary constants are printed). If you need per-trial data for
  statistical re-analysis, modify `main()` to log each timed iteration to a
  file, or contact the corresponding author.

## Relationship to the other repositories

| Repository | Contains |
|---|---|
| `code_MIRACLE_PBS` (this repo) | Unit-cost benchmark -> Table 4 |
| `Web` | End-to-end Spring-MVC/JPBC simulation -> Table 6/7; also holds an archived snapshot that bundled this MIRACL benchmark at submission time |
| `code_RELIC` | Independent second-library cross-check of the pairing-based constants (T_bpo, T_pbsm, T_pbpa) |
| `ProVerif` | Formal protocol verification model (`Simulation.pv`) |
