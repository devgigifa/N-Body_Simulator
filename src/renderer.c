/*
 * renderer.c — Visualização de trajetórias via pipe para gnuplot.
 *
 * Conceitos de C aplicados:
 *   - popen/pclose para invocar processo externo   (Arquivos / I/O)
 *   - fprintf com especificadores %s, %d, %zu      (printf)
 *   - Loops for sobre array de corpos              (Repetição)
 *   - Ponteiros e structs (Body)                   (Ponteiros, Structs)
 *   - Strings e snprintf para montar comandos      (Strings)
 */

#include "renderer.h"

#include <stdio.h>    /* popen, pclose, fprintf, snprintf */
#include <string.h>   /* strlen */

/* Cores para cada corpo — ciclo se houver mais de 8 corpos */
static const char *COLORS[] = {
    "#FFD700", /* Sol:     dourado  */
    "#4FC3F7", /* Mercúrio: azul claro */
    "#FF8F00", /* Vênus:   laranja  */
    "#42A5F5", /* Terra:   azul     */
    "#EF5350", /* Marte:   vermelho */
    "#AB47BC", /* Júpiter: roxo     */
    "#26A69A", /* Saturno: verde-azul */
    "#78909C"  /* outros:  cinza    */
};
#define NUM_COLORS 8

/* -----------------------------------------------------------------------
 * renderer_open — abre pipe para gnuplot com popen.
 * "gnuplot -persistent" mantém a janela aberta após o programa encerrar.
 * ----------------------------------------------------------------------- */
FILE *renderer_open(void) {
    /* popen() cria pipe bidirecional para o processo gnuplot */
    FILE *gp = popen("gnuplot -persistent", "w");
    if (!gp) {
        fprintf(stderr, "[ERRO] renderer_open: gnuplot não encontrado no PATH.\n"
                        "       Instale via: sudo apt install gnuplot\n"
                        "                ou: brew install gnuplot\n");
    }
    return gp;
}

/* -----------------------------------------------------------------------
 * renderer_close — fecha o pipe e aguarda gnuplot terminar.
 * ----------------------------------------------------------------------- */
void renderer_close(FILE *gp) {
    if (gp) {
        pclose(gp);   /* equivalente a fclose() + waitpid() */
    }
}

/* -----------------------------------------------------------------------
 * renderer_setup — configura aparência do plot gnuplot.
 * Envia comandos de configuração pelo pipe.
 * ----------------------------------------------------------------------- */
void renderer_setup(FILE *gp, const char *title) {
    /* Fundo escuro para simular o espaço */
    fprintf(gp, "set terminal qt size 900,900 background '#0A0A1A'\n");
    fprintf(gp, "set title '%s' textcolor '#FFFFFF' font ',14'\n", title);

    /* Eixos e bordas */
    fprintf(gp, "set xlabel 'X (AU)' textcolor '#888888'\n");
    fprintf(gp, "set ylabel 'Y (AU)' textcolor '#888888'\n");
    fprintf(gp, "set border lc '#333333'\n");
    fprintf(gp, "set grid lc '#1A1A2E'\n");

    /* Escala em UA (Unidades Astronômicas = 1.496e11 metros) */
    fprintf(gp, "set xrange [-5e12:5e12]\n");
    fprintf(gp, "set yrange [-5e12:5e12]\n");
    fprintf(gp, "set size ratio 1\n");   /* aspecto 1:1 */
    fprintf(gp, "unset key\n");

    /* Fator de escala para converter metros → UA no label */
    fprintf(gp, "AU = 1.496e11\n");
}

/* -----------------------------------------------------------------------
 * renderer_plot_2d — plota trajetórias de todos os corpos.
 *
 * Usa o arquivo output.dat gerado pelo simulador.
 * Cada corpo é plotado como linha + ponto final.
 * ----------------------------------------------------------------------- */
void renderer_plot_2d(FILE *gp, const char *data_file,
                      const Body *bodies, size_t n) {
    size_t i;

    /* Monta comando 'plot' com um dataset por corpo */
    fprintf(gp, "plot ");

    for (i = 0; i < n; i++) {
        const char *color = COLORS[i % NUM_COLORS];

        /*
         * gnuplot 'index i' seleciona o i-ésimo bloco do arquivo
         * (blocos separados por linhas em branco no output.dat)
         */
        fprintf(gp,
            "'%s' index %zu using 2:3 with lines lc rgb '%s' lw 1.5",
            data_file, i, color);

        /* Adiciona ponto atual (última posição) com tamanho maior */
        fprintf(gp,
            ", '' index %zu every ::1::1 using 2:3"
            " with points pt 7 ps 1.5 lc rgb '%s'",
            i, color);

        /* Adiciona label com nome do corpo */
        fprintf(gp,
            ", '' index %zu every ::1::1 using 2:3:1"
            " with labels offset 0.5,0.5 tc rgb '%s' font ',9'",
            i, color);

        /* Vírgula separadora entre plots (exceto no último) */
        if (i < n - 1) {
            fprintf(gp, ", \\\n     ");
        }
    }

    fprintf(gp, "\n");
    fflush(gp);   /* garante que os comandos foram enviados ao processo */
}
