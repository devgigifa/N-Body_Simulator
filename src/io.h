#ifndef IO_H
#define IO_H

/*
 * io.h — Interface de leitura/escrita para o simulador N-body.
 *
 * Funções:
 *   - Carrega corpos de um arquivo JSON (via cJSON)
 *   - Grava trajetórias em output.dat para gnuplot
 *   - Utilitários de leitura de arquivo para string
 */

#include "body.h"
#include <stddef.h>   /* size_t */
#include <stdio.h>    /* FILE   */

/* -----------------------------------------------------------------------
 * Lê o conteúdo completo de um arquivo para uma string alocada na heap.
 * O chamador é responsável por chamar free() no resultado.
 * Retorna NULL em caso de erro.
 * ----------------------------------------------------------------------- */
char *io_read_file(const char *path);

/* -----------------------------------------------------------------------
 * Carrega corpos celestes de um arquivo JSON.
 *
 * Formato esperado:
 *   {
 *     "bodies": [
 *       { "name": "Sun", "mass": 1.989e30,
 *         "x": 0, "y": 0, "vx": 0, "vy": 0 },
 *       ...
 *     ]
 *   }
 *
 * Parâmetros de saída:
 *   bodies_out — ponteiro para o array alocado (chamar body_free ao fim)
 *   n_out      — número de corpos lidos
 *
 * Retorna 0 em sucesso, -1 em erro.
 * ----------------------------------------------------------------------- */
int io_load_json(const char *path, Body **bodies_out, size_t *n_out);

/* -----------------------------------------------------------------------
 * Abre (ou cria) o arquivo de saída de trajetórias.
 * Retorna o FILE* aberto; o chamador fecha com fclose().
 * ----------------------------------------------------------------------- */
FILE *io_open_output(const char *path);

/* -----------------------------------------------------------------------
 * Grava as posições atuais de todos os corpos no arquivo de saída.
 * Formato: nome x y  (uma linha por corpo, blocos separados por linhas em branco)
 * ----------------------------------------------------------------------- */
void io_write_positions(FILE *out, const Body *bodies, size_t n, double time);

/* -----------------------------------------------------------------------
 * Configuração global da simulação lida do JSON.
 * ----------------------------------------------------------------------- */
typedef struct {
    double dt;              /* passo de tempo em segundos               */
    double total_time;      /* tempo total de simulação em segundos      */
    char   integrator[16];  /* nome do integrador: euler/verlet/rk4      */
    int    plot_interval;   /* gravar posição a cada N passos             */
} SimConfig;

/* -----------------------------------------------------------------------
 * Lê a configuração da simulação do JSON.
 * Retorna 0 em sucesso, -1 em erro (usa defaults se campos faltarem).
 * ----------------------------------------------------------------------- */
int io_load_config(const char *path, SimConfig *cfg);

#endif /* IO_H */
