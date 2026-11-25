/*
 * integrator.c — Implementação dos integradores numéricos Euler, Verlet e RK4.
 *
 * Conceitos de C aplicados:
 *   - Enum para seleção de algoritmo          (Tipos de Dados)
 *   - switch-case para despacho               (Estruturas de Decisão)
 *   - Alocação dinâmica de arrays temporários (Alocação Dinâmica)
 *   - Ponteiros e passagem por referência     (Ponteiros, Funções)
 *   - Loops for com variável de controle      (Estruturas de Repetição)
 *   - strcmp para comparar strings            (Strings, string.h)
 */

#include "integrator.h"
#include "physics.h"

#include <stdlib.h>   /* malloc, free  */
#include <string.h>   /* strcmp        */
#include <stdio.h>    /* fprintf       */

/* -----------------------------------------------------------------------
 * integrator_euler — integração de 1ª ordem (Euler explícito).
 *
 * Mais rápido por passo, mas sem conservação de energia.
 * Adequado apenas para visualizações rápidas / testes.
 * ----------------------------------------------------------------------- */
void integrator_euler(Body *bodies, size_t n, double dt) {
    size_t i;

    /* Passo 1: recalcula as forças com as posições atuais */
    physics_compute_forces(bodies, n);

    /* Passo 2: integra posição e velocidade com as acelerações */
    for (i = 0; i < n; i++) {
        /* Atualiza velocidade: v += a * dt */
        bodies[i].vx += bodies[i].ax * dt;
        bodies[i].vy += bodies[i].ay * dt;

        /* Atualiza posição: x += v * dt */
        bodies[i].x  += bodies[i].vx * dt;
        bodies[i].y  += bodies[i].vy * dt;

        /* Registra a nova posição no trail de trajetória */
        body_record_trail(&bodies[i]);
    }
}

/* -----------------------------------------------------------------------
 * integrator_verlet — Velocity Verlet de 2ª ordem.
 *
 * Algoritmo:
 *   1. x(t+dt) = x(t) + v(t)·dt + ½·a(t)·dt²
 *   2. Calcula a(t+dt) com as novas posições
 *   3. v(t+dt) = v(t) + ½·[a(t) + a(t+dt)]·dt
 *
 * Propriedade simplética: conserva o volume do espaço de fase,
 * resultando em melhor conservação de energia a longo prazo.
 * ----------------------------------------------------------------------- */
void integrator_verlet(Body *bodies, size_t n, double dt) {
    size_t i;
    double half_dt = 0.5 * dt;
    double half_dt2 = half_dt * dt;   /* ½·dt² */

    /* Passo 1: calcula acelerações no tempo atual t */
    physics_compute_forces(bodies, n);

    /*
     * Passo 2: atualiza posições e aplica meio passo de velocidade.
     * x(t+dt) = x + v·dt + ½·a·dt²
     * v*(t)   = v + ½·a·dt   (velocidade intermediária)
     */
    for (i = 0; i < n; i++) {
        bodies[i].x  += bodies[i].vx * dt + bodies[i].ax * half_dt2;
        bodies[i].y  += bodies[i].vy * dt + bodies[i].ay * half_dt2;
        bodies[i].vx += bodies[i].ax * half_dt;
        bodies[i].vy += bodies[i].ay * half_dt;
    }

    /* Passo 3: recalcula forças com as novas posições t+dt */
    physics_compute_forces(bodies, n);

    /* Passo 4: completa a atualização de velocidade com a nova aceleração */
    for (i = 0; i < n; i++) {
        bodies[i].vx += bodies[i].ax * half_dt;
        bodies[i].vy += bodies[i].ay * half_dt;
        body_record_trail(&bodies[i]);
    }
}

/* -----------------------------------------------------------------------
 * integrator_rk4 — Runge-Kutta de 4ª ordem.
 *
 * Requer 4 avaliações de força por passo (k1, k2, k3, k4).
 * Aloca arrays temporários na heap para os estados intermediários.
 * Erro de truncamento: O(dt⁴) por passo.
 * ----------------------------------------------------------------------- */
void integrator_rk4(Body *bodies, size_t n, double dt) {
    size_t i;
    double half_dt = 0.5 * dt;
    double sixth_dt = dt / 6.0;

    /*
     * Aloca arrays temporários para os 4 estados intermediários do RK4.
     * Cada ki armazena (dx, dy, dvx, dvy) para todos os n corpos.
     */
    double *k1x  = (double *)malloc(n * sizeof(double));
    double *k1y  = (double *)malloc(n * sizeof(double));
    double *k1vx = (double *)malloc(n * sizeof(double));
    double *k1vy = (double *)malloc(n * sizeof(double));

    double *k2x  = (double *)malloc(n * sizeof(double));
    double *k2y  = (double *)malloc(n * sizeof(double));
    double *k2vx = (double *)malloc(n * sizeof(double));
    double *k2vy = (double *)malloc(n * sizeof(double));

    double *k3x  = (double *)malloc(n * sizeof(double));
    double *k3y  = (double *)malloc(n * sizeof(double));
    double *k3vx = (double *)malloc(n * sizeof(double));
    double *k3vy = (double *)malloc(n * sizeof(double));

    double *k4x  = (double *)malloc(n * sizeof(double));
    double *k4y  = (double *)malloc(n * sizeof(double));
    double *k4vx = (double *)malloc(n * sizeof(double));
    double *k4vy = (double *)malloc(n * sizeof(double));

    /* Array temporário para estado intermediário */
    Body *temp = body_alloc(n);

    /* Verifica falhas de alocação — necessário em código robusto */
    if (!k1x || !k1y || !k1vx || !k1vy ||
        !k2x || !k2y || !k2vx || !k2vy ||
        !k3x || !k3y || !k3vx || !k3vy ||
        !k4x || !k4y || !k4vx || !k4vy || !temp) {
        fprintf(stderr, "[ERRO] integrator_rk4: falha de alocação.\n");
        goto cleanup;
    }

    /* ---- k1: derivadas no ponto atual (t) ----------------------------- */
    physics_compute_forces(bodies, n);
    for (i = 0; i < n; i++) {
        k1x[i]  = bodies[i].vx;
        k1y[i]  = bodies[i].vy;
        k1vx[i] = bodies[i].ax;
        k1vy[i] = bodies[i].ay;
    }

    /* ---- k2: derivadas em t + dt/2 com k1 ----------------------------- */
    for (i = 0; i < n; i++) {
        temp[i]      = bodies[i];   /* copia struct por valor */
        temp[i].x   += k1x[i]  * half_dt;
        temp[i].y   += k1y[i]  * half_dt;
        temp[i].vx  += k1vx[i] * half_dt;
        temp[i].vy  += k1vy[i] * half_dt;
    }
    physics_compute_forces(temp, n);
    for (i = 0; i < n; i++) {
        k2x[i]  = temp[i].vx;
        k2y[i]  = temp[i].vy;
        k2vx[i] = temp[i].ax;
        k2vy[i] = temp[i].ay;
    }

    /* ---- k3: derivadas em t + dt/2 com k2 ----------------------------- */
    for (i = 0; i < n; i++) {
        temp[i]      = bodies[i];
        temp[i].x   += k2x[i]  * half_dt;
        temp[i].y   += k2y[i]  * half_dt;
        temp[i].vx  += k2vx[i] * half_dt;
        temp[i].vy  += k2vy[i] * half_dt;
    }
    physics_compute_forces(temp, n);
    for (i = 0; i < n; i++) {
        k3x[i]  = temp[i].vx;
        k3y[i]  = temp[i].vy;
        k3vx[i] = temp[i].ax;
        k3vy[i] = temp[i].ay;
    }

    /* ---- k4: derivadas em t + dt com k3 ------------------------------- */
    for (i = 0; i < n; i++) {
        temp[i]      = bodies[i];
        temp[i].x   += k3x[i]  * dt;
        temp[i].y   += k3y[i]  * dt;
        temp[i].vx  += k3vx[i] * dt;
        temp[i].vy  += k3vy[i] * dt;
    }
    physics_compute_forces(temp, n);
    for (i = 0; i < n; i++) {
        k4x[i]  = temp[i].vx;
        k4y[i]  = temp[i].vy;
        k4vx[i] = temp[i].ax;
        k4vy[i] = temp[i].ay;
    }

    /* ---- Combina: y(t+dt) = y(t) + (dt/6)·(k1 + 2k2 + 2k3 + k4) ----- */
    for (i = 0; i < n; i++) {
        bodies[i].x  += sixth_dt * (k1x[i]  + 2*k2x[i]  + 2*k3x[i]  + k4x[i]);
        bodies[i].y  += sixth_dt * (k1y[i]  + 2*k2y[i]  + 2*k3y[i]  + k4y[i]);
        bodies[i].vx += sixth_dt * (k1vx[i] + 2*k2vx[i] + 2*k3vx[i] + k4vx[i]);
        bodies[i].vy += sixth_dt * (k1vy[i] + 2*k2vy[i] + 2*k3vy[i] + k4vy[i]);
        body_record_trail(&bodies[i]);
    }

cleanup:
    /* Libera todos os arrays temporários — evita vazamento de memória */
    free(k1x);  free(k1y);  free(k1vx);  free(k1vy);
    free(k2x);  free(k2y);  free(k2vx);  free(k2vy);
    free(k3x);  free(k3y);  free(k3vx);  free(k3vy);
    free(k4x);  free(k4y);  free(k4vx);  free(k4vy);
    body_free(temp);
}

/* -----------------------------------------------------------------------
 * integrator_step — despacha para o integrador correto via switch-case.
 * ----------------------------------------------------------------------- */
void integrator_step(Body *bodies, size_t n, double dt, IntegratorType type) {
    switch (type) {
        case INTEGRATOR_EULER:
            integrator_euler(bodies, n, dt);
            break;
        case INTEGRATOR_VERLET:
            integrator_verlet(bodies, n, dt);
            break;
        case INTEGRATOR_RK4:
            integrator_rk4(bodies, n, dt);
            break;
        default:
            /* Fallback seguro: usa Verlet se tipo for desconhecido */
            fprintf(stderr, "[AVISO] Integrador desconhecido; usando Verlet.\n");
            integrator_verlet(bodies, n, dt);
            break;
    }
}

/* -----------------------------------------------------------------------
 * integrator_from_string — converte string para enum IntegratorType.
 * Usa strcmp da string.h para comparação de strings.
 * ----------------------------------------------------------------------- */
IntegratorType integrator_from_string(const char *name) {
    if (strcmp(name, "euler")  == 0) return INTEGRATOR_EULER;
    if (strcmp(name, "verlet") == 0) return INTEGRATOR_VERLET;
    if (strcmp(name, "rk4")    == 0) return INTEGRATOR_RK4;

    /* Padrão: Verlet (melhor equilíbrio custo/precisão) */
    fprintf(stderr, "[AVISO] Integrador '%s' desconhecido; usando 'verlet'.\n", name);
    return INTEGRATOR_VERLET;
}
