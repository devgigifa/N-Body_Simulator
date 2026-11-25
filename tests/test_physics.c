/*
 * test_physics.c — Testes unitários para physics.c usando Unity framework.
 *
 * Conceitos de C verificados nos testes:
 *   - Operadores aritméticos e relacionais    (Operadores)
 *   - Structs e ponteiros                     (Structs, Ponteiros)
 *   - Alocação e liberação de memória         (Alocação Dinâmica)
 *   - Funções com retorno double              (Funções)
 *   - Constantes (#define GRAVITY_G)          (Tipos de Dados)
 */

#include "unity.h"
#include "../src/body.h"
#include "../src/physics.h"

#include <math.h>    /* fabs */

/* Tolerância para comparação de doubles (1 ppm) */
#define TOLERANCE 1e-6

/* -----------------------------------------------------------------------
 * setUp / tearDown — chamados automaticamente pelo Unity antes/após cada teste.
 * ----------------------------------------------------------------------- */
void setUp(void) {}
void tearDown(void) {}

/* -----------------------------------------------------------------------
 * test_two_body_force_symmetry — verifica 3ª lei de Newton:
 * A força que A exerce sobre B deve ser igual e oposta à que B exerce sobre A.
 *
 * Critério: ax[0] * m[0] == -ax[1] * m[1]  (F = m*a)
 * ----------------------------------------------------------------------- */
void test_two_body_force_symmetry(void) {
    /* Aloca 2 corpos na heap */
    Body *b = body_alloc(2);
    TEST_ASSERT_NOT_NULL(b);

    /* Corpo 0: Sol — massa ~1.989e30 kg na origem */
    b[0].mass = 1.989e30;
    b[0].x    = 0.0;
    b[0].y    = 0.0;

    /* Corpo 1: Terra — massa ~5.972e24 kg a 1 UA de distância */
    b[1].mass = 5.972e24;
    b[1].x    = 1.496e11;   /* 1 UA em metros */
    b[1].y    = 0.0;

    /* Calcula forças */
    physics_compute_forces(b, 2);

    /* 3ª lei: F01 = -F10, logo m0*ax0 = -m1*ax1 */
    double F0 = b[0].mass * b[0].ax;
    double F1 = b[1].mass * b[1].ax;

    /*
     * |F0 + F1| / |F0| deve ser menor que TOLERANCE.
     * (ação e reação iguais e opostas)
     */
    TEST_ASSERT_DOUBLE_WITHIN(TOLERANCE * fabs(F0), -F0, F1);

    body_free(b);
}

/* -----------------------------------------------------------------------
 * test_gravity_magnitude — verifica o valor absoluto da força gravitacional
 * entre Sol e Terra contra o valor analítico.
 *
 * F = G * M * m / r²
 * ----------------------------------------------------------------------- */
void test_gravity_magnitude(void) {
    Body *b = body_alloc(2);
    TEST_ASSERT_NOT_NULL(b);

    double M = 1.989e30;   /* massa do Sol  */
    double m = 5.972e24;   /* massa da Terra */
    double r = 1.496e11;   /* distância (1 UA) */

    b[0].mass = M;  b[0].x = 0.0;  b[0].y = 0.0;
    b[1].mass = m;  b[1].x = r;    b[1].y = 0.0;

    physics_compute_forces(b, 2);

    /* Aceleração analítica da Terra em direção ao Sol: a = G*M/r² */
    double a_expected = GRAVITY_G * M / (r * r);

    /* A aceleração de b[1] em x deve ser negativa (aponta para o Sol) */
    double a_got = -b[1].ax;   /* módulo (invertemos o sinal) */

    /* Tolerância de 1% — o softening introduz erro pequeno a esta distância */
    TEST_ASSERT_DOUBLE_WITHIN(a_expected * 0.01, a_expected, a_got);

    body_free(b);
}

/* -----------------------------------------------------------------------
 * test_zero_force_same_position — dois corpos na mesma posição (softening).
 * Com softening, a força deve ser finita (não infinita).
 * ----------------------------------------------------------------------- */
void test_zero_force_finite_softening(void) {
    Body *b = body_alloc(2);
    TEST_ASSERT_NOT_NULL(b);

    b[0].mass = 1.0e30;  b[0].x = 0.0;  b[0].y = 0.0;
    b[1].mass = 1.0e30;  b[1].x = 0.0;  b[1].y = 0.0;

    physics_compute_forces(b, 2);

    /* Com softening, ax deve ser finito — não NaN nem Inf */
    TEST_ASSERT_FALSE(isnan(b[0].ax));
    TEST_ASSERT_FALSE(isinf(b[0].ax));
    TEST_ASSERT_FALSE(isnan(b[1].ax));
    TEST_ASSERT_FALSE(isinf(b[1].ax));

    body_free(b);
}

/* -----------------------------------------------------------------------
 * test_kinetic_energy — verifica E_k = ½mv² para um corpo simples.
 * ----------------------------------------------------------------------- */
void test_kinetic_energy(void) {
    Body *b = body_alloc(1);
    TEST_ASSERT_NOT_NULL(b);

    b[0].mass = 1000.0;   /* 1 tonelada */
    b[0].vx   = 100.0;    /* 100 m/s em x */
    b[0].vy   = 0.0;

    double ek = physics_kinetic_energy(b, 1);

    /* E_k = ½ * 1000 * 100² = 5.000.000 J */
    TEST_ASSERT_DOUBLE_WITHIN(1.0, 5000000.0, ek);

    body_free(b);
}

/* -----------------------------------------------------------------------
 * test_energy_conservation_verlet — simula 100 passos com Verlet e verifica
 * que a energia total se mantém estável (desvio < 0.1%).
 * ----------------------------------------------------------------------- */
void test_energy_conservation_verlet(void) {
    /* Importa integrador e verifica conservação */
    #include "../src/integrator.h"

    Body *b = body_alloc(2);
    TEST_ASSERT_NOT_NULL(b);

    /* Sistema Sol-Terra simplificado */
    b[0].mass = 1.989e30;  b[0].x = 0.0;       b[0].y = 0.0;
    b[0].vx   = 0.0;       b[0].vy = 0.0;

    b[1].mass = 5.972e24;  b[1].x = 1.496e11;  b[1].y = 0.0;
    b[1].vx   = 0.0;       b[1].vy = 29780.0;  /* velocidade orbital Terra ~29.8 km/s */

    double e0 = physics_total_energy(b, 2);

    /* Simula 1000 passos de 1 hora cada */
    int step;
    for (step = 0; step < 1000; step++) {
        integrator_verlet(b, 2, 3600.0);
    }

    double e1 = physics_total_energy(b, 2);

    /* Desvio relativo de energia deve ser < 0.1% (1e-3) */
    double drift = fabs((e1 - e0) / e0);
    TEST_ASSERT_LESS_THAN_DOUBLE(1e-3, drift);

    body_free(b);
}

/* -----------------------------------------------------------------------
 * main — ponto de entrada do Unity test runner.
 * ----------------------------------------------------------------------- */
int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_two_body_force_symmetry);
    RUN_TEST(test_gravity_magnitude);
    RUN_TEST(test_zero_force_finite_softening);
    RUN_TEST(test_kinetic_energy);
    RUN_TEST(test_energy_conservation_verlet);

    return UNITY_END();
}
