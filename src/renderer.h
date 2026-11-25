#ifndef RENDERER_H
#define RENDERER_H

/*
 * renderer.h — Visualização de trajetórias via pipe para gnuplot.
 *
 * gnuplot é invocado como processo filho via popen().
 * Comandos são enviados pelo pipe com fprintf() como se fosse um arquivo.
 * Não requer bibliotecas gráficas — apenas gnuplot instalado no PATH.
 */

#include "body.h"
#include <stddef.h>   /* size_t */
#include <stdio.h>    /* FILE   */

/* -----------------------------------------------------------------------
 * Abre pipe para gnuplot.
 * Retorna FILE* do pipe; NULL em caso de erro (gnuplot não encontrado).
 * ----------------------------------------------------------------------- */
FILE *renderer_open(void);

/* -----------------------------------------------------------------------
 * Fecha o pipe do gnuplot e aguarda encerramento do processo.
 * ----------------------------------------------------------------------- */
void renderer_close(FILE *gp);

/* -----------------------------------------------------------------------
 * Plota as trajetórias 2D de todos os corpos a partir dos dados em output.dat.
 * Envia comandos gnuplot pelo pipe.
 * ----------------------------------------------------------------------- */
void renderer_plot_2d(FILE *gp, const char *data_file,
                      const Body *bodies, size_t n);

/* -----------------------------------------------------------------------
 * Configura estilo do plot (título, eixos, cores) no gnuplot.
 * ----------------------------------------------------------------------- */
void renderer_setup(FILE *gp, const char *title);

#endif /* RENDERER_H */
