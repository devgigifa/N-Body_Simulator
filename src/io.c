/*
 * io.c — Implementação de entrada/saída: parser JSON e escrita de trajetórias.
 *
 * Conceitos de C aplicados:
 *   - Manipulação de arquivos: fopen/fclose/fprintf/fscanf  (Arquivos)
 *   - Alocação dinâmica: malloc/realloc/free                (Alocação Dinâmica)
 *   - Strings: strlen, strncpy, snprintf                    (Strings)
 *   - Ponteiros para ponteiros (double indirection)         (Ponteiros)
 *   - Estruturas (SimConfig, Body)                          (Structs)
 *   - Especificadores de formato %s, %lf, %d               (printf/scanf)
 */

#include "io.h"
#include "cJSON.h"

#include <stdio.h>    /* fopen, fclose, fprintf, fseek, ftell, fread */
#include <stdlib.h>   /* malloc, free  */
#include <string.h>   /* strncpy       */

/* -----------------------------------------------------------------------
 * io_read_file — lê arquivo inteiro para buffer alocado na heap.
 * Usa fseek/ftell para determinar tamanho sem segunda passagem.
 * ----------------------------------------------------------------------- */
char *io_read_file(const char *path) {
    /* Abre o arquivo em modo leitura texto */
    FILE *fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "[ERRO] io_read_file: não foi possível abrir '%s'.\n", path);
        return NULL;
    }

    /* Determina o tamanho do arquivo */
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);    /* equivalente a fseek(fp, 0, SEEK_SET) */

    /* Aloca buffer com espaço para '\0' terminador */
    char *buf = (char *)malloc((size_t)size + 1);
    if (!buf) {
        fprintf(stderr, "[ERRO] io_read_file: falha de alocação.\n");
        fclose(fp);
        return NULL;
    }

    /* Lê o conteúdo completo de uma vez */
    size_t read = fread(buf, 1, (size_t)size, fp);
    buf[read] = '\0';   /* garante terminação da string */

    fclose(fp);
    return buf;
}

/* -----------------------------------------------------------------------
 * io_load_json — parseia JSON e aloca array de Body structs.
 *
 * Usa cJSON (single-header library) para navegar a estrutura JSON.
 * ----------------------------------------------------------------------- */
int io_load_json(const char *path, Body **bodies_out, size_t *n_out) {
    /* Lê arquivo para string */
    char *text = io_read_file(path);
    if (!text) return -1;

    /* Parseia JSON com cJSON */
    cJSON *root = cJSON_Parse(text);
    free(text);   /* libera buffer de texto — não precisa mais */

    if (!root) {
        fprintf(stderr, "[ERRO] io_load_json: JSON inválido em '%s'.\n", path);
        return -1;
    }

    /* Navega até o array "bodies" */
    cJSON *bodies_json = cJSON_GetObjectItem(root, "bodies");
    if (!cJSON_IsArray(bodies_json)) {
        fprintf(stderr, "[ERRO] io_load_json: campo 'bodies' não encontrado.\n");
        cJSON_Delete(root);
        return -1;
    }

    /* Conta quantos corpos existem no JSON */
    int count = cJSON_GetArraySize(bodies_json);
    if (count <= 0) {
        fprintf(stderr, "[ERRO] io_load_json: nenhum corpo no JSON.\n");
        cJSON_Delete(root);
        return -1;
    }

    /* Aloca array de Body structs */
    Body *bodies = body_alloc((size_t)count);
    if (!bodies) {
        cJSON_Delete(root);
        return -1;
    }

    /* Itera sobre cada corpo no JSON e preenche a struct */
    int i;
    for (i = 0; i < count; i++) {
        cJSON *b = cJSON_GetArrayItem(bodies_json, i);

        /* Lê o nome */
        cJSON *name = cJSON_GetObjectItem(b, "name");
        if (cJSON_IsString(name)) {
            strncpy(bodies[i].name, name->valuestring, 31);
            bodies[i].name[31] = '\0';   /* garante terminação */
        }

        /* Lê campos numéricos (double) */
        cJSON *mass = cJSON_GetObjectItem(b, "mass");
        cJSON *x    = cJSON_GetObjectItem(b, "x");
        cJSON *y    = cJSON_GetObjectItem(b, "y");
        cJSON *vx   = cJSON_GetObjectItem(b, "vx");
        cJSON *vy   = cJSON_GetObjectItem(b, "vy");

        if (cJSON_IsNumber(mass)) bodies[i].mass = mass->valuedouble;
        if (cJSON_IsNumber(x))    bodies[i].x    = x->valuedouble;
        if (cJSON_IsNumber(y))    bodies[i].y    = y->valuedouble;
        if (cJSON_IsNumber(vx))   bodies[i].vx   = vx->valuedouble;
        if (cJSON_IsNumber(vy))   bodies[i].vy   = vy->valuedouble;

        /* Registra posição inicial no trail */
        body_record_trail(&bodies[i]);

        fprintf(stdout, "  [+] Corpo carregado: %s (massa=%.3e kg)\n",
                bodies[i].name, bodies[i].mass);
    }

    cJSON_Delete(root);

    /* Retorna via ponteiro para ponteiro (double indirection) */
    *bodies_out = bodies;
    *n_out      = (size_t)count;

    return 0;   /* sucesso */
}

/* -----------------------------------------------------------------------
 * io_load_config — lê parâmetros da simulação do JSON.
 * Usa defaults seguros se campos estiverem ausentes.
 * ----------------------------------------------------------------------- */
int io_load_config(const char *path, SimConfig *cfg) {
    /* Defaults para caso o JSON não tenha todos os campos */
    cfg->dt            = 3600.0;      /* 1 hora em segundos            */
    cfg->total_time    = 3.154e7;     /* 1 ano em segundos             */
    cfg->plot_interval = 24;          /* grava a cada 24 horas         */
    strncpy(cfg->integrator, "verlet", 15);

    char *text = io_read_file(path);
    if (!text) return -1;

    cJSON *root = cJSON_Parse(text);
    free(text);
    if (!root) return -1;

    cJSON *sim = cJSON_GetObjectItem(root, "simulation");
    if (sim) {
        cJSON *dt   = cJSON_GetObjectItem(sim, "dt");
        cJSON *tmax = cJSON_GetObjectItem(sim, "total_time");
        cJSON *intg = cJSON_GetObjectItem(sim, "integrator");
        cJSON *pi   = cJSON_GetObjectItem(sim, "plot_interval");

        if (cJSON_IsNumber(dt))   cfg->dt            = dt->valuedouble;
        if (cJSON_IsNumber(tmax)) cfg->total_time    = tmax->valuedouble;
        if (cJSON_IsNumber(pi))   cfg->plot_interval = (int)pi->valuedouble;
        if (cJSON_IsString(intg)) {
            strncpy(cfg->integrator, intg->valuestring, 15);
            cfg->integrator[15] = '\0';
        }
    }

    cJSON_Delete(root);
    return 0;
}

/* -----------------------------------------------------------------------
 * io_open_output — abre arquivo de saída para escrita.
 * Modo "w" cria ou sobrescreve o arquivo.
 * ----------------------------------------------------------------------- */
FILE *io_open_output(const char *path) {
    FILE *fp = fopen(path, "w");
    if (!fp) {
        fprintf(stderr, "[ERRO] io_open_output: não foi possível criar '%s'.\n", path);
    }
    return fp;
}

/* -----------------------------------------------------------------------
 * io_write_positions — grava posições no formato gnuplot.
 *
 * Formato de saída:
 *   # t=<tempo_s>
 *   <nome> <x> <y>
 *   ...
 *   <linha em branco>  ← necessário para separar blocos no gnuplot
 * ----------------------------------------------------------------------- */
void io_write_positions(FILE *out, const Body *bodies, size_t n, double time) {
    size_t i;

    /* Cabeçalho do bloco com timestamp */
    fprintf(out, "# t=%.6e s\n", time);

    /* Uma linha por corpo: nome X Y */
    for (i = 0; i < n; i++) {
        fprintf(out, "%s %.6e %.6e\n",
                bodies[i].name, bodies[i].x, bodies[i].y);
    }

    /* Linha em branco separa blocos — necessário para gnuplot */
    fprintf(out, "\n");
}
