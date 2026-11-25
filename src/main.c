/*
 * main.c — Ponto de entrada do simulador gravitacional N-body.
 *
 * Fluxo principal:
 *   1. Lê arquivo JSON com condições iniciais dos corpos
 *   2. Loop de simulação: calcula forças → integra → grava output.dat
 *   3. Exibe energia total a cada intervalo para verificar conservação
 *   4. Ao final, abre gnuplot e plota as trajetórias
 *
 * Conceitos de C aplicados:
 *   - argc/argv para argumentos de linha de comando       (I/O, Funções)
 *   - Loop while com variável de controle de tempo        (Repetição)
 *   - Estruturas de decisão if/else                       (Decisão)
 *   - Ponteiros e alocação dinâmica                       (Ponteiros)
 *   - Manipulação de arquivos (output.dat)                (Arquivos)
 *   - printf com especificadores de formato               (I/O formatado)
 *   - Pré-processador: #include, #define                  (Pré-processador)
 */

#include <stdio.h>    /* printf, fprintf, stderr */
#include <stdlib.h>   /* exit, EXIT_FAILURE      */
#include <string.h>   /* strncpy                 */

#include "body.h"
#include "physics.h"
#include "integrator.h"
#include "io.h"
#include "renderer.h"

/* -----------------------------------------------------------------------
 * Versão do programa — definida via pré-processador.
 * Em vez de hard-code na string, usa macro para facilitar atualização.
 * ----------------------------------------------------------------------- */
#define SIMULATOR_VERSION "1.0.0"

/* -----------------------------------------------------------------------
 * Exibe uso correto do programa e encerra.
 * ----------------------------------------------------------------------- */
static void print_usage(const char *prog_name) {
    fprintf(stderr,
        "N-Gravity Simulator v%s\n\n"
        "Uso: %s <config.json> [--no-plot]\n\n"
        "Exemplo:\n"
        "  %s config/solar_system.json\n"
        "  %s config/solar_system.json --no-plot\n\n"
        "Opções:\n"
        "  --no-plot   Executa simulação sem abrir gnuplot\n",
        SIMULATOR_VERSION, prog_name, prog_name, prog_name);
}

/* -----------------------------------------------------------------------
 * Exibe barra de progresso simples na mesma linha do terminal.
 * Usa '\r' (carriage return) para sobrescrever sem nova linha.
 * ----------------------------------------------------------------------- */
static void print_progress(double current, double total, double energy) {
    int pct = (int)(100.0 * current / total);
    printf("\r  Progresso: [%3d%%] | t=%.3e s | E=%.6e J   ",
           pct, current, energy);
    fflush(stdout);   /* força exibição imediata sem \n */
}

/* -----------------------------------------------------------------------
 * main — ponto de entrada do programa.
 * ----------------------------------------------------------------------- */
int main(int argc, char *argv[]) {
    /* ---- Parsing de argumentos de linha de comando -------------------- */
    if (argc < 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    const char *config_path = argv[1];

    /* Verifica se --no-plot foi passado (varre args com for) */
    int enable_plot = 1;
    int i;
    for (i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--no-plot") == 0) {
            enable_plot = 0;
        }
    }

    /* ---- Carrega configuração da simulação ---------------------------- */
    SimConfig cfg;
    if (io_load_config(config_path, &cfg) != 0) {
        fprintf(stderr, "[ERRO] Falha ao carregar configuração de '%s'.\n",
                config_path);
        return EXIT_FAILURE;
    }

    printf("=== N-Gravity Simulator v%s ===\n", SIMULATOR_VERSION);
    printf("  Config:     %s\n", config_path);
    printf("  Integrador: %s\n", cfg.integrator);
    printf("  dt:         %.2e s\n", cfg.dt);
    printf("  Tempo total:%.2e s\n\n", cfg.total_time);

    /* ---- Carrega corpos do JSON --------------------------------------- */
    Body  *bodies = NULL;
    size_t n_bodies = 0;

    printf("Carregando corpos celestes...\n");
    if (io_load_json(config_path, &bodies, &n_bodies) != 0) {
        fprintf(stderr, "[ERRO] Falha ao carregar corpos de '%s'.\n",
                config_path);
        return EXIT_FAILURE;
    }
    printf("  %zu corpo(s) carregado(s).\n\n", n_bodies);

    /* Converte nome do integrador para enum */
    IntegratorType intg_type = integrator_from_string(cfg.integrator);

    /* ---- Abre arquivo de saída para trajetórias ----------------------- */
    FILE *output = io_open_output("data/output.dat");
    if (!output) {
        body_free(bodies);
        return EXIT_FAILURE;
    }

    /* ---- Loop principal de simulação ---------------------------------- */
    double t       = 0.0;
    long   step    = 0;
    double e_init  = physics_total_energy(bodies, n_bodies);

    printf("Iniciando simulação...\n");

    while (t < cfg.total_time) {

        /* Avança um passo de tempo com o integrador selecionado */
        integrator_step(bodies, n_bodies, cfg.dt, intg_type);

        t    += cfg.dt;
        step += 1;

        /* Grava posições no arquivo a cada plot_interval passos */
        if (step % cfg.plot_interval == 0) {
            io_write_positions(output, bodies, n_bodies, t);

            /* Exibe energia para monitorar conservação */
            double e_now = physics_total_energy(bodies, n_bodies);
            print_progress(t, cfg.total_time, e_now);
        }
    }

    printf("\n\n  Simulação concluída. Total de passos: %ld\n", step);

    /* ---- Relatório final de conservação de energia -------------------- */
    double e_final = physics_total_energy(bodies, n_bodies);
    double e_drift = (e_final - e_init) / e_init * 100.0;
    printf("  Energia inicial: %.6e J\n", e_init);
    printf("  Energia final:   %.6e J\n", e_final);
    printf("  Desvio relativo: %.4f%%\n\n", e_drift);

    /* Fecha arquivo de trajetórias */
    fclose(output);
    printf("  Trajetórias gravadas em: data/output.dat\n");

    /* ---- Visualização com gnuplot ------------------------------------- */
    if (enable_plot) {
        printf("  Abrindo gnuplot...\n");

        FILE *gp = renderer_open();
        if (gp) {
            renderer_setup(gp, "N-Body Gravitational Simulator");
            renderer_plot_2d(gp, "data/output.dat", bodies, n_bodies);
            renderer_close(gp);
        }
    } else {
        printf("  Plot desativado (--no-plot).\n");
    }

    /* ---- Libera memória alocada --------------------------------------- */
    body_free(bodies);   /* evita vazamento de memória */
    printf("  Memória liberada. Encerrando.\n");

    return 0;   /* EXIT_SUCCESS */
}
