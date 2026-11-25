#ifndef INTEGRATOR_H
#define INTEGRATOR_H

/*
 * integrator.h — Métodos de integração numérica para o sistema N-body.
 *
 * Três métodos implementados em ordem crescente de precisão:
 *
 *   1. Euler         — O(dt):  simples, barato, pouco preciso
 *   2. Verlet        — O(dt²): conserva energia melhor, sem custo extra
 *   3. RK4           — O(dt⁴): mais preciso, 4× mais cálculos de força
 *
 * Ref.: Press et al., Numerical Recipes in C, Cap. 16–17.
 */

#include "body.h"
#include <stddef.h>  /* size_t */

/* -----------------------------------------------------------------------
 * Tipos de integrador disponíveis.
 * Usado como enum para seleção em tempo de execução.
 * ----------------------------------------------------------------------- */
typedef enum {
    INTEGRATOR_EULER  = 0,  /* Euler explícito (1ª ordem)         */
    INTEGRATOR_VERLET = 1,  /* Velocity Verlet (2ª ordem)         */
    INTEGRATOR_RK4    = 2   /* Runge-Kutta 4ª ordem               */
} IntegratorType;

/* -----------------------------------------------------------------------
 * Avança todos os corpos em um passo de tempo dt usando Euler explícito.
 *
 *   v(t+dt) = v(t) + a(t) * dt
 *   x(t+dt) = x(t) + v(t) * dt
 *
 * Mais simples, mas acumula erro de energia ao longo do tempo.
 * ----------------------------------------------------------------------- */
void integrator_euler(Body *bodies, size_t n, double dt);

/* -----------------------------------------------------------------------
 * Avança todos os corpos em dt usando o algoritmo Velocity Verlet.
 *
 *   x(t+dt) = x(t) + v(t)*dt + ½·a(t)·dt²
 *   a(t+dt) = compute_forces(x(t+dt))
 *   v(t+dt) = v(t) + ½·[a(t) + a(t+dt)]·dt
 *
 * Preserva o simplectismo (conservação aproximada de energia),
 * recomendado para simulações longas.
 * ----------------------------------------------------------------------- */
void integrator_verlet(Body *bodies, size_t n, double dt);

/* -----------------------------------------------------------------------
 * Avança todos os corpos em dt usando Runge-Kutta de 4ª ordem.
 *
 * Usa 4 avaliações intermediárias da força por passo. Mais preciso,
 * mas requer alocação temporária de arrays de corpo.
 * ----------------------------------------------------------------------- */
void integrator_rk4(Body *bodies, size_t n, double dt);

/* -----------------------------------------------------------------------
 * Função de despacho: seleciona e chama o integrador pelo tipo.
 * Permite trocar o método em tempo de execução via parâmetro.
 * ----------------------------------------------------------------------- */
void integrator_step(Body *bodies, size_t n, double dt, IntegratorType type);

/* -----------------------------------------------------------------------
 * Converte um nome ("euler", "verlet", "rk4") para IntegratorType.
 * Retorna INTEGRATOR_VERLET como padrão se o nome for desconhecido.
 * ----------------------------------------------------------------------- */
IntegratorType integrator_from_string(const char *name);

#endif /* INTEGRATOR_H */
