/*
 * physics.c — Implementação do cálculo de forças gravitacionais N-body.
 *
 * Conceitos de C aplicados:
 *   - Operadores aritméticos e relacionais  (Operadores)
 *   - Ponteiros para structs                (Ponteiros, Structs)
 *   - Loops com variável de controle        (Estruturas de Repetição)
 *   - Funções com retorno double            (Funções)
 *   - math.h: sqrt, fabs                   (Bibliotecas padrão)
 */

#include "physics.h"

#include <math.h>    /* sqrt, fabs  */
#include <stddef.h>  /* size_t      */

/* -----------------------------------------------------------------------
 * physics_compute_forces — acumula aceleração de cada corpo.
 *
 * Algoritmo:
 *   Para cada par (i, j) com i < j:
 *     1. Calcula distância com softening para evitar singularidade
 *     2. Calcula módulo da força: F = G*m1*m2 / (r² + ε²)
 *     3. Decompõe F nos eixos X e Y
 *     4. Aplica 3ª lei de Newton: ação e reação iguais e opostas
 *
 * Ref.: Numerical Recipes in C, Cap. 17 (integração de ODEs)
 * ----------------------------------------------------------------------- */
void physics_compute_forces(Body *bodies, size_t n) {
    size_t i, j;

    /* Zera acelerações antes de acumular — cada passo recomeça do zero */
    for (i = 0; i < n; i++) {
        body_reset_acceleration(&bodies[i]);
    }

    /* Loop O(N²): itera sobre todos os pares únicos (i, j) com j > i */
    for (i = 0; i < n; i++) {
        for (j = i + 1; j < n; j++) {

            /* Vetor de deslocamento de i até j */
            double dx = bodies[j].x - bodies[i].x;
            double dy = bodies[j].y - bodies[i].y;

            /*
             * Distância com softening:
             *   r² = dx² + dy² + ε²
             * O softening ε² evita r→0 quando dois corpos se sobrepõem,
             * prevenindo acelerações infinitas (instabilidade numérica).
             */
            double r2  = dx*dx + dy*dy + SOFTENING*SOFTENING;
            double r   = sqrt(r2);        /* distância real (metros)    */
            double r3  = r2 * r;          /* r³ para a fórmula vetorial */

            /*
             * Aceleração de i causada por j: a = G*mj / r³ * (rj - ri)
             * (dividir por mi cancela — a força é G*mi*mj/r², mas
             *  a aceleração é F/mi = G*mj/r²; aqui usamos a forma vetorial)
             */
            double factor_i = GRAVITY_G * bodies[j].mass / r3;
            bodies[i].ax += factor_i * dx;
            bodies[i].ay += factor_i * dy;

            /* 3ª lei: aceleração de j causada por i tem sinal oposto */
            double factor_j = GRAVITY_G * bodies[i].mass / r3;
            bodies[j].ax -= factor_j * dx;
            bodies[j].ay -= factor_j * dy;
        }
    }
}

/* -----------------------------------------------------------------------
 * physics_kinetic_energy — Ek = Σ ½·m·v²
 * ----------------------------------------------------------------------- */
double physics_kinetic_energy(const Body *bodies, size_t n) {
    double ek = 0.0;
    size_t i;

    for (i = 0; i < n; i++) {
        double v2 = bodies[i].vx * bodies[i].vx
                  + bodies[i].vy * bodies[i].vy;
        ek += 0.5 * bodies[i].mass * v2;
    }

    return ek;
}

/* -----------------------------------------------------------------------
 * physics_potential_energy — Ep = Σ -G·m1·m2 / r  (pares únicos)
 * ----------------------------------------------------------------------- */
double physics_potential_energy(const Body *bodies, size_t n) {
    double ep = 0.0;
    size_t i, j;

    for (i = 0; i < n; i++) {
        for (j = i + 1; j < n; j++) {
            double dx = bodies[j].x - bodies[i].x;
            double dy = bodies[j].y - bodies[i].y;
            double r  = sqrt(dx*dx + dy*dy + SOFTENING*SOFTENING);

            ep -= GRAVITY_G * bodies[i].mass * bodies[j].mass / r;
        }
    }

    return ep;
}

/* -----------------------------------------------------------------------
 * physics_total_energy — energia mecânica total (conservada idealmente)
 * ----------------------------------------------------------------------- */
double physics_total_energy(const Body *bodies, size_t n) {
    return physics_kinetic_energy(bodies, n)
         + physics_potential_energy(bodies, n);
}
