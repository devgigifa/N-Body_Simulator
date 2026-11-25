#ifndef BODY_H
#define BODY_H

/*
 * body.h — Definição da estrutura Body e funções de alocação/liberação.
 * Cada corpo celeste possui: massa, posição (x,y), velocidade (vx,vy),
 * aceleração (ax,ay) e um nome de identificação.
 */

#include <stddef.h>  /* size_t */

/* -----------------------------------------------------------------------
 * Struct principal: representa um único corpo celeste no espaço 2D.
 * Todos os valores usam double para precisão numérica adequada.
 * ----------------------------------------------------------------------- */
typedef struct {
    char   name[32];   /* nome do corpo (ex: "Earth", "Sun")           */
    double mass;       /* massa em kg                                   */
    double x, y;       /* posição em metros                             */
    double vx, vy;     /* velocidade em m/s                             */
    double ax, ay;     /* aceleração em m/s² (calculada a cada passo)  */
    double trail_x[1024]; /* histórico de posições X para visualização */
    double trail_y[1024]; /* histórico de posições Y para visualização */
    int    trail_len;     /* número de pontos gravados no trail         */
} Body;

/* -----------------------------------------------------------------------
 * Aloca um array de n corpos na heap com calloc (zerando a memória).
 * Retorna NULL em caso de falha de alocação.
 * ----------------------------------------------------------------------- */
Body *body_alloc(size_t n);

/* -----------------------------------------------------------------------
 * Libera o array de corpos alocado por body_alloc.
 * ----------------------------------------------------------------------- */
void body_free(Body *bodies);

/* -----------------------------------------------------------------------
 * Registra a posição atual do corpo no seu trail (histórico de trajetória).
 * Circular: quando cheio, o trail para de gravar (evita overflow).
 * ----------------------------------------------------------------------- */
void body_record_trail(Body *b);

/* -----------------------------------------------------------------------
 * Zera a aceleração do corpo antes de recalcular as forças do passo atual.
 * ----------------------------------------------------------------------- */
void body_reset_acceleration(Body *b);

/* -----------------------------------------------------------------------
 * Imprime informações do corpo no stdout (para debug).
 * ----------------------------------------------------------------------- */
void body_print(const Body *b);

#endif /* BODY_H */
