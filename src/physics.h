#ifndef PHYSICS_H
#define PHYSICS_H

/*
 * physics.h — Cálculo das forças gravitacionais entre N corpos.
 *
 * Lei de Newton: F = G * m1 * m2 / r²
 * A força é decomposta nos eixos X e Y via trigonometria vetorial.
 *
 * Complexidade: O(N²) — cada par único é calculado uma vez.
 */

#include "body.h"
#include <stddef.h>  /* size_t */

/* Constante gravitacional universal (m³ kg⁻¹ s⁻²) */
#define GRAVITY_G  6.674e-11

/*
 * Distância mínima permitida entre corpos (softening parameter).
 * Evita divisão por zero e singularidades numéricas quando corpos
 * se aproximam demais — técnica padrão em simulações N-body.
 */
#define SOFTENING  1.0e9

/* -----------------------------------------------------------------------
 * Calcula e acumula as acelerações de todos os N corpos devido à
 * atração gravitacional mútua (O(N²) pares).
 *
 * Parâmetros:
 *   bodies — ponteiro para o array de corpos (modificado in-place)
 *   n      — número de corpos
 * ----------------------------------------------------------------------- */
void physics_compute_forces(Body *bodies, size_t n);

/* -----------------------------------------------------------------------
 * Calcula a energia cinética total do sistema (½mv²).
 * Útil para verificar conservação de energia nos testes.
 * ----------------------------------------------------------------------- */
double physics_kinetic_energy(const Body *bodies, size_t n);

/* -----------------------------------------------------------------------
 * Calcula a energia potencial gravitacional total do sistema.
 * U = -G * m1 * m2 / r  (soma sobre todos os pares).
 * ----------------------------------------------------------------------- */
double physics_potential_energy(const Body *bodies, size_t n);

/* -----------------------------------------------------------------------
 * Retorna a energia mecânica total (cinética + potencial).
 * Em um sistema conservativo ideal, esse valor deve ser constante.
 * ----------------------------------------------------------------------- */
double physics_total_energy(const Body *bodies, size_t n);

#endif /* PHYSICS_H */
