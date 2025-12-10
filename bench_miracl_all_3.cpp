// bench_miracl_all.cpp
// Benchmark:
//  - T_bpo  : Bilinear Pairing operation              (PBC)
//  - T_pbsm : Pairing-based scalar multiplication     (PBC, G1 * Zr)
//  - T_pbpa : Pairing-based point addition            (PBC, G1)
//  - T_rc   : A modular inversion computation         (PBC, Zr)
//  - T_meo  : A modular exponentiation operation      (PBC, Zr)
//  - T_smec : Scalar multiplication on elliptic curve (MIRACL, secp256k1)
//  - T_paec : Point addition on elliptic curve        (MIRACL, secp256k1)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

// ===== MIRACL (EC non-pairing, mis. secp256k1) =====
#include "miracl.h"
#include "ecn.h"

// ===== PBC (pairing-based crypto) =====
#include <pbc/pbc.h>

/* -------------------------------------------------
 * Fungsi bantu: waktu sekarang (detik, double)
 * ------------------------------------------------- */
double now_in_seconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
}

/* =================================================
 * 1. Benchmark MIRACL: T_smec dan T_paec
 * ================================================= */
void benchmark_miracl_ec(int N_smec, int N_paec,
                         double &T_smec, double &T_paec) {
    // Inisialisasi MIRACL
    miracl *mip = mirsys(100, 0);   // 100 digit internal, cukup untuk 256-bit
    mip->IOBASE = 16;

    // Parameter secp256k1
    big p  = mirvar(0);
    big a  = mirvar(0);
    big b  = mirvar(0);
    big n  = mirvar(0);
    big Gx = mirvar(0);
    big Gy = mirvar(0);
    big k  = mirvar(0);

    cinstr(p,
        (char*)"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F");
    zero(a);
    cinstr(b, (char*)"7");
    cinstr(n,
        (char*)"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141");
    cinstr(Gx,
        (char*)"79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798");
    cinstr(Gy,
        (char*)"483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8");

    ecurve_init(a, b, p, MR_PROJECTIVE);

    epoint *G = epoint_init();
    epoint *R = epoint_init();
    epoint *P = epoint_init();
    epoint *Q = epoint_init();

    if (!epoint_set(Gx, Gy, 0, G)) {
        printf("Gagal set generator G\n");
        return;
    }

    irand((long)time(NULL));

    /* ---------- T_smec : scalar multiplication di EC ---------- */
    // pilih satu skalar acak sekali saja
    bigrand(n, k);

    // warm-up
    for (int i = 0; i < 10; i++) {
        ecurve_mult(k, G, R);
    }

    double start = now_in_seconds();
    for (int i = 0; i < N_smec; i++) {
        ecurve_mult(k, G, R);   // hanya operasi kG
    }
    double end = now_in_seconds();

    double total_time_smec = end - start;
    T_smec = total_time_smec / (double)N_smec;

    /* ---------- T_paec : point addition di EC ---------- */
    big k1 = mirvar(0);
    big k2 = mirvar(0);

    bigrand(n, k1);
    bigrand(n, k2);
    ecurve_mult(k1, G, P);   // P = k1 G
    ecurve_mult(k2, G, Q);   // Q = k2 G

    // warm-up
    for (int i = 0; i < 10; i++) {
        ecurve_add(P, Q);   // Q = P + Q
    }

    start = now_in_seconds();
    for (int i = 0; i < N_paec; i++) {
        ecurve_add(P, Q);   // hanya penjumlahan titik
    }
    end = now_in_seconds();

    double total_time_paec = end - start;
    T_paec = total_time_paec / (double)N_paec;

    // bersih-bersih
    mirkill(p); mirkill(a); mirkill(b); mirkill(n);
    mirkill(Gx); mirkill(Gy); mirkill(k); mirkill(k1); mirkill(k2);

    epoint_free(G);
    epoint_free(R);
    epoint_free(P);
    epoint_free(Q);

    mirexit();
}

/* =================================================
 * 2. Benchmark PBC:
 *    T_bpo, T_pbsm, T_pbpa, T_rc, T_meo
 * ================================================= */
void benchmark_pbc_all(int N_bpo,  int N_pbsm, int N_pbpa,
                       int N_rc,   int N_meo,
                       double &T_bpo,  double &T_pbsm, double &T_pbpa,
                       double &T_rc,   double &T_meo) {

    pairing_t pairing;

    // Parameter Type A (standard PBC example)
    const char *param_str =
        "type a\n"
        "q 87807107996633125224377819847540498158068831994142082"
        "1102865339926647563088022295707862517942266222142315585"
        "8769582317459277713367317481324925129998224791\n"
        "h 12016012264891146079388821366740534204802954401251311"
        "822919615131047207289359704531102844802183906537786776\n"
        "r 730750818665451621361119245571504901405976559617\n"
        "exp2 159\n"
        "exp1 107\n"
        "sign1 1\n"
        "sign0 1\n";

    if (pairing_init_set_buf(pairing, param_str, strlen(param_str))) {
        fprintf(stderr, "Gagal inisialisasi pairing.\n");
        exit(1);
    }

    /* ---------- T_bpo : bilinear pairing operation ---------- */
    element_t P, Q, GT_elem;
    element_init_G1(P, pairing);
    element_init_G1(Q, pairing);
    element_init_GT(GT_elem, pairing);

    element_random(P);
    element_random(Q);

    for (int i = 0; i < 10; i++) {
        element_pairing(GT_elem, P, Q);
    }

    double start = now_in_seconds();
    for (int i = 0; i < N_bpo; i++) {
        element_pairing(GT_elem, P, Q);  // hanya pairing
    }
    double end = now_in_seconds();

    double total_time_bpo = end - start;
    T_bpo = total_time_bpo / (double)N_bpo;

    /* ---------- T_pbsm : scalar multiplication di G1 ---------- */
    element_t Rg1, scalar_z;
    element_init_G1(Rg1, pairing);
    element_init_Zr(scalar_z, pairing);

    element_random(P);        // basis tetap
    element_random(scalar_z); // skalar tetap

    for (int i = 0; i < 10; i++) {
        element_mul_zn(Rg1, P, scalar_z);
    }

    start = now_in_seconds();
    for (int i = 0; i < N_pbsm; i++) {
        element_mul_zn(Rg1, P, scalar_z);
    }
    end = now_in_seconds();

    double total_time_pbsm = end - start;
    T_pbsm = total_time_pbsm / (double)N_pbsm;

    /* ---------- T_pbpa : point addition di G1 ---------- */
    element_t A1, A2, Ares;
    element_init_G1(A1, pairing);
    element_init_G1(A2, pairing);
    element_init_G1(Ares, pairing);

    element_random(A1);
    element_random(A2);

    for (int i = 0; i < 10; i++) {
        element_add(Ares, A1, A2);
    }

    start = now_in_seconds();
    for (int i = 0; i < N_pbpa; i++) {
        element_add(Ares, A1, A2);
    }
    end = now_in_seconds();

    double total_time_pbpa = end - start;
    T_pbpa = total_time_pbpa / (double)N_pbpa;

    /* ---------- T_rc : modular inversion di Zr ---------- */
    element_t z_inv;
    element_init_Zr(z_inv, pairing);
    element_random(scalar_z);
    while (element_is0(scalar_z)) {
        element_random(scalar_z);
    }

    for (int i = 0; i < 10; i++) {
        element_invert(z_inv, scalar_z);
    }

    start = now_in_seconds();
    for (int i = 0; i < N_rc; i++) {
        element_invert(z_inv, scalar_z);
    }
    end = now_in_seconds();

    double total_time_rc = end - start;
    T_rc = total_time_rc / (double)N_rc;

    /* ---------- T_meo : modular exponentiation di Zr ---------- */
    element_t base_z, exp_z, pow_res;
    element_init_Zr(base_z, pairing);
    element_init_Zr(exp_z, pairing);
    element_init_Zr(pow_res, pairing);

    element_random(base_z);
    element_random(exp_z);

    for (int i = 0; i < 10; i++) {
        element_pow_zn(pow_res, base_z, exp_z);
    }

    start = now_in_seconds();
    for (int i = 0; i < N_meo; i++) {
        element_pow_zn(pow_res, base_z, exp_z);
    }
    end = now_in_seconds();

    double total_time_meo = end - start;
    T_meo = total_time_meo / (double)N_meo;

    /* ---------- Bersih-bersih PBC ---------- */
    element_clear(P); element_clear(Q); element_clear(GT_elem);
    element_clear(Rg1); element_clear(scalar_z);
    element_clear(A1); element_clear(A2); element_clear(Ares);
    element_clear(z_inv);
    element_clear(base_z); element_clear(exp_z); element_clear(pow_res);

    pairing_clear(pairing);
}

/* =================================================
 * MAIN
 * ================================================= */
int main() {
    // Jumlah iterasi – bisa dinaikkan kalau ingin lebih stabil
    int N_bpo  = 1000;
    int N_pbsm = 1000;
    int N_pbpa = 1000;
    int N_rc   = 2000;
    int N_meo  = 500;
    int N_smec = 2000;
    int N_paec = 5000;

    double T_bpo, T_pbsm, T_pbpa, T_rc, T_meo, T_smec, T_paec;

    benchmark_miracl_ec(N_smec, N_paec, T_smec, T_paec);
    benchmark_pbc_all(N_bpo, N_pbsm, N_pbpa, N_rc, N_meo,
                      T_bpo, T_pbsm, T_pbpa, T_rc, T_meo);

    printf("==============================================\n");
    printf("Running Cost of the Underlying Operations\n");
    printf("==============================================\n");
    printf("%-8s %-45s %15s\n", "Symbol", "Algorithm", "time (ms)");
    printf("--------------------------------------------------------------\n");
    printf("%-8s %-45s %15.4f\n", "T_bpo",
           "Bilinear Pairing operation", T_bpo * 1000.0);
    printf("%-8s %-45s %15.4f\n", "T_pbsm",
           "Pairing-based scalar multiplication", T_pbsm * 1000.0);
    printf("%-8s %-45s %15.4f\n", "T_pbpa",
           "Pairing-based point addition", T_pbpa * 1000.0);
    printf("%-8s %-45s %15.4f\n", "T_rc",
           "A modular inversion computation", T_rc * 1000.0);
    printf("%-8s %-45s %15.4f\n", "T_meo",
           "A modular exponentiation operation", T_meo * 1000.0);
    printf("%-8s %-45s %15.4f\n", "T_smec",
           "A scalar multiplication on elliptic curve", T_smec * 1000.0);
    printf("%-8s %-45s %15.4f\n", "T_paec",
           "A point addition on elliptic curve", T_paec * 1000.0);
    printf("--------------------------------------------------------------\n");

    return 0;
}

