/*
 * test_integrator.c — Testes unitários para integrator.c usando Unity.
 *
 * Verifica:
 *   - Coerência numérica de Euler, Verlet e RK4
 *   - integrator_from_string retorna enum correto
 *   - RK4 é mais preciso que Euler para a mesma dt
 */

#include "unity.h"
#include "../src/body.h"
#include "../src/physics.h"
#include "../src/integrator.h"

#include <math.h>    /* fabs, sqrt */
#include <string.h>  /* strcmp     */

void setUp(void) {}
void tearDown(void) {}

/* -----------------------------------------------------------------------
 * test_integrator_from_string — verifica conversão de nome para enum.
 * ----------------------------------------------------------------------- */
void test_integrator_from_string(void) {
    TEST_ASSERT_EQUAL_INT(INTEGRATOR_EULER,  integrator_from_string("euler"));
    TEST_ASSERT_EQUAL_INT(INTEGRATOR_VERLET, integrator_from_string("verlet"));
    TEST_ASSERT_EQUAL_INT(INTEGRATOR_RK4,    integrator_from_string("rk4"));
    /* Desconhecido retorna Verlet */
    TEST_ASSERT_EQUAL_INT(INTEGRATOR_VERLET, integrator_from_string("xyz_desconhecido"));
}

/* -----------------------------------------------------------------------
 * test_euler_moves_body — Euler deve mover um corpo com velocidade constante.
 * Sem forças (massa zero no outro corpo), posição deve avançar v*dt.
 * ----------------------------------------------------------------------- */
void test_euler_moves_body(void) {
    Body *b = body_alloc(1);
    TEST_ASSERT_NOT_NULL(b);

    b[0].mass = 1.0;
    b[0].x    = 0.0;
    b[0].y    = 0.0;
    b[0].vx   = 10.0;   /* 10 m/s em x */
    b[0].vy   = 0.0;
    b[0].ax   = 0.0;    /* sem força */
    b[0].ay   = 0.0;

    double dt = 1.0;    /* 1 segundo */

    /*
     * Euler sem forças (body_alloc zera ax/ay, e physics_compute_forces
     * com 1 corpo não calcula nada): x deve avançar v*dt = 10 m.
     * Usamos integrator_euler diretamente para testar o método.
     */
    integrator_euler(b, 1, dt);

    /* x(t+dt) = x(t) + vx*dt = 0 + 10*1 = 10 */
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 10.0, b[0].x);

    body_free(b);
}

/* -----------------------------------------------------------------------
 * test_rk4_more_accurate_than_euler — para a mesma dt, RK4 deve conservar
 * energia melhor que Euler num sistema de 2 corpos.
 *
 * Estratégia: simula 500 passos com cada método e compara o desvio de energia.
 * ----------------------------------------------------------------------- */
void test_rk4_more_accurate_than_euler(void) {
    /* Inicializa dois sistemas idênticos */
    Body *b_euler = body_alloc(2);
    Body *b_rk4   = body_alloc(2);
    TEST_ASSERT_NOT_NULL(b_euler);
    TEST_ASSERT_NOT_NULL(b_rk4);

    /* Condições iniciais: sistema Sol-Terra simplificado */
    int k;
    for (k = 0; k < 2; k++) {
        Body *b = (k == 0) ? b_euler : b_rk4;

        b[0].mass = 1.989e30;  b[0].x = 0.0;      b[0].y = 0.0;
        b[0].vx   = 0.0;       b[0].vy = 0.0;

        b[1].mass = 5.972e24;  b[1].x = 1.496e11; b[1].y = 0.0;
        b[1].vx   = 0.0;       b[1].vy = 29780.0;
    }

    double e0_euler = physics_total_energy(b_euler, 2);
    double e0_rk4   = physics_total_energy(b_rk4,   2);

    double dt = 86400.0;  /* 1 dia */
    int steps = 100;

    int i;
    for (i = 0; i < steps; i++) {
        integrator_euler(b_euler, 2, dt);
        integrator_rk4(b_rk4, 2, dt);
    }

    double drift_euler = fabs((physics_total_energy(b_euler, 2) - e0_euler) / e0_euler);
    double drift_rk4   = fabs((physics_total_energy(b_rk4,   2) - e0_rk4)  / e0_rk4);

    /* RK4 deve ter menor desvio de energia que Euler */
    TEST_ASSERT_LESS_THAN_DOUBLE(drift_euler, drift_rk4);

    body_free(b_euler);
    body_free(b_rk4);
}

/* -----------------------------------------------------------------------
 * test_verlet_reversibility — Verlet é (quase) reversível no tempo.
 * Simula N passos para frente e N passos para trás com dt negativo.
 * A posição final deve ser próxima da inicial.
 * ----------------------------------------------------------------------- */
void test_verlet_reversibility(void) {
    Body *b = body_alloc(2);
    TEST_ASSERT_NOT_NULL(b);

    b[0].mass = 1.989e30;  b[0].x = 0.0;      b[0].y = 0.0;
    b[0].vx   = 0.0;       b[0].vy = 0.0;

    b[1].mass = 5.972e24;  b[1].x = 1.496e11; b[1].y = 0.0;
    b[1].vx   = 0.0;       b[1].vy = 29780.0;

    double x0 = b[1].x;
    double y0 = b[1].y;

    double dt = 3600.0;   /* 1 hora */
    int    n  = 100;      /* 100 passos */
    int    i;

    /* Avança n passos para frente */
    for (i = 0; i < n; i++) integrator_verlet(b, 2, dt);

    /* Inverte velocidades */
    b[0].vx = -b[0].vx;  b[0].vy = -b[0].vy;
    b[1].vx = -b[1].vx;  b[1].vy = -b[1].vy;

    /* Avança n passos (que é voltar no tempo) */
    for (i = 0; i < n; i++) integrator_verlet(b, 2, dt);

    /* Inverte velocidades de volta ao estado original */
    b[0].vx = -b[0].vx;  b[0].vy = -b[0].vy;
    b[1].vx = -b[1].vx;  b[1].vy = -b[1].vy;

    /* Desvio de posição deve ser < 0.01% da distância inicial */
    double tol = x0 * 1e-4;
    TEST_ASSERT_DOUBLE_WITHIN(tol, x0, b[1].x);
    TEST_ASSERT_DOUBLE_WITHIN(tol, y0, b[1].y);

    body_free(b);
}

/* -----------------------------------------------------------------------
 * main — ponto de entrada do Unity test runner.
 * ----------------------------------------------------------------------- */
int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_integrator_from_string);
    RUN_TEST(test_euler_moves_body);
    RUN_TEST(test_rk4_more_accurate_than_euler);
    RUN_TEST(test_verlet_reversibility);

    return UNITY_END();
}
