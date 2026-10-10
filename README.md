# code_MIRACLE_PBS

Unit-cost microbenchmark for the elliptic-curve and pairing-based primitives
used in **Table 4** of the CBAS-DSH manuscript (JICT submission). It measures
the seven per-operation constants only. It does **not** contain end-to-end
implementations of Hou et al. (2021), Verma et al. (2020), Qiao et al. (2024),
or CBAS-DSH.

## What this benchmarks

`bench_miracl_all_3.cpp` runs a 10-iteration warm-up followed by N timed
iterations per operation (N is set in `main()`), and prints the mean time per
operation:

| Symbol (code) | Symbol (Table 4) | Operation | Library |
|---|---|---|---|
| `T_bpo`  | Tbpo  | Bilinear pairing | PBC (Type A) |
| `T_pbsm` | Tbpsm | Scalar multiplication in G1 | PBC |
| `T_pbpa` | Tbppa | Point addition in G1 | PBC |
| `T_rc`   | Trc   | Modular inversion in Zr | PBC |
| `T_meo`  | Tmeo  | Modular exponentiation in Zr | PBC |
| `T_smec` | Tsmec | Scalar multiplication, secp256k1 | MIRACL |
| `T_paec` | Tpaec | Point addition, secp256k1 | MIRACL |

Each printed value is the mean of one run. The manuscript reports the mean over
1,500 runs on a MacBook Air M2; the hardware and software details are in its
Performance Analysis subsection.

## Relation to Table 5 and Figures 3-5

Table 5 and Figures 3-5 are **not** produced by running this program per
scheme. Their Sign / Verify / AggSign / AggVerify values are computed
analytically: each scheme's operation count is multiplied by the constants
measured here (the same cost model is used for Table 6).

Worked example with Tsmec = 0.1809 ms and Tpaec = 0.0009 ms:

```
CBAS-DSH Verify       = 3*Tsmec + 3*Tpaec                 = 0.5454 ms
Qiao et al. Verify    = 4*Tsmec + 3*Tpaec                 = 0.7263 ms
CBAS-DSH AggVerify    = (3n+2)*Tsmec + 3n*Tpaec, n=100    = 54.9018 ms
Qiao et al. AggVerify = 4n*Tsmec + (3n+3)*Tpaec, n=100    = 72.6327 ms
```

On other hardware the constants will differ; recompute the formulas with your
own measured values.

## Build

Requires the MIRACL C library (`miracl.h`, `mirsys`, `ecurve_*` API) and PBC.

```bash
# MIRACL (64-bit build): build miracl.a following the MIRACL repository's
# lib/linux64 instructions, then
# PBC
git clone https://github.com/blynn/pbc.git
cd pbc && autoreconf -f -i && ./configure && make && sudo make install

# benchmark
g++ -O2 -I<miracl>/source -I/usr/local/include \
    -o bench_run bench_miracl_all_3.cpp <miracl>/source/miracl.a \
    -L/usr/local/lib -lpbc -lgmp -Wl,-rpath,/usr/local/lib
./bench_run
```

If the compiler reports conflicts between `miracl.h` and `ecn.h`, remove the
`#include "ecn.h"` line: the C API used here does not need it.

## Notes

- Absolute times depend on the hardware; the relative comparison between
  schemes and the linear scaling in n do not.
- Only the summary constants are printed; no per-trial CSV is included. To log
  each timed iteration, modify `main()`.

## Related repositories

| Repository | Contains |
|---|---|
| `code_MIRACLE_PBS` (this repo) | Unit-cost benchmark for Table 4 |
| `Web` | End-to-end Spring-MVC/JPBC simulation (Tables 6-7 context) |
| `code_RELIC` | Second-library benchmark of the pairing-based constants |
| `ProVerif` | Formal protocol model (`Simulation.pv`) |
