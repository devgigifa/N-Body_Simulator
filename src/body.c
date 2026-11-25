/*
 * body.c — Implementação das funções de gerenciamento de corpos celestes.
 *
 * Conceitos de C aplicados:
 *   - Alocação dinâmica com calloc / free  (Alocação Dinâmica)
 *   - Ponteiros e structs                  (Ponteiros, Structs)
 *   - Passagem por ponteiro                (Funções)
 *   - Strings com snprintf                 (Strings)
 */

#include "body.h"

#include <stdlib.h>   /* calloc, free  */
#include <stdio.h>    /* printf, fprintf */
#include <string.h>   /* strncpy        */

/* -----------------------------------------------------------------------
 * body_alloc — aloca n corpos e inicializa tudo com zero (calloc).
 * Usar calloc em vez de malloc garante que mass/vel/acc comecem em 0.
 * ----------------------------------------------------------------------- */
Body *body_alloc(size_t n) {
    /* calloc(n, tamanho) zera a memória — evita lixo nos campos numéricos */
    Body *bodies = (Body *)calloc(n, sizeof(Body));

    /* Verifica falha de alocação — boa prática obrigatória em C */
    if (bodies == NULL) {
        fprintf(stderr, "[ERRO] body_alloc: falha ao alocar %zu corpos.\n", n);
        return NULL;
    }

    return bodies;
}

/* -----------------------------------------------------------------------
 * body_free — libera memória alocada por body_alloc.
 * Sempre chamar ao encerrar o programa para evitar vazamento.
 * ----------------------------------------------------------------------- */
void body_free(Body *bodies) {
    free(bodies);  /* free(NULL) é seguro por definição (C99 §7.20.3.2) */
}

/* -----------------------------------------------------------------------
 * body_record_trail — adiciona a posição atual ao histórico de trajetória.
 * O trail tem tamanho fixo (1024 pontos); quando cheio, não grava mais.
 * ----------------------------------------------------------------------- */
void body_record_trail(Body *b) {
    /* Guarda somente se ainda houver espaço no array de trail */
    if (b->trail_len < 1024) {
        b->trail_x[b->trail_len] = b->x;
        b->trail_y[b->trail_len] = b->y;
        b->trail_len++;
    }
}

/* -----------------------------------------------------------------------
 * body_reset_acceleration — zera ax e ay antes do novo cálculo de forças.
 * É essencial resetar antes de acumular as contribuições dos N-1 corpos.
 * ----------------------------------------------------------------------- */
void body_reset_acceleration(Body *b) {
    b->ax = 0.0;
    b->ay = 0.0;
}

/* -----------------------------------------------------------------------
 * body_print — exibe estado atual do corpo (debug / logging).
 * ----------------------------------------------------------------------- */
void body_print(const Body *b) {
    printf("Corpo: %-12s | massa=%.3e kg | pos=(%.3e, %.3e) m | "
           "vel=(%.3e, %.3e) m/s\n",
           b->name, b->mass, b->x, b->y, b->vx, b->vy);
}
