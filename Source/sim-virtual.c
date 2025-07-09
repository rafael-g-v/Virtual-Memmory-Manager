#include "Headers/estruturas.h"
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if(argc < 5){
        printf("Uso: sim-virtual <algoritmo> <arquivo> <tam. quadro/pagina> <tam. memoria>\n");
        exit(1);
    }

    char algoritmo = argv[1][0];
    FILE *arquivo = fopen(argv[2], "rt");
    int tam_pagina = atoi(argv[3]), tam_memoria = atoi(argv[4]);

    if((tam_pagina != 8) && (tam_pagina != 32)) {
        printf("Tamanhos de página permitidos: 8 e 32KB\n");
        exit(3);
    }
    if((tam_memoria != 1) && (tam_memoria != 2)) {
        printf("Tamanhos de memória permitidos: 1 e 2MB\n");
        exit(3);
    }

    printf("Executando o simulador...\n");
    printf("Arquivo de entrada: %s\n", argv[2]);
    printf("Tamanho da memória física: %d MB\n", tam_memoria);
    printf("Tamanho das páginas: %d KB\n", tam_pagina);
    printf("Algoritmo de substituição: ");
    switch (algoritmo) {
        case 'L': printf("LRU\n"); break;
        case '2': printf("Segunda Chance\n"); break;
        case 'C': printf("Clock\n"); break;
        case 'O': printf("Ótimo\n"); break;
        default: printf("Desconhecido\n"); exit(4);
    }

    int shift = (tam_pagina == 8) ? 13 : 15;
    
    int num_paginas = pow(2, (32 - shift));
    Pagina *tp = (Pagina*)calloc(num_paginas, sizeof(Pagina));
    if(!tp){
        printf("Não foi possível alocar a tabela de páginas.\n");
        exit(2);
    }

    int num_quadros = (tam_memoria * 1024) / tam_pagina;
    Quadro *tq = (Quadro *)calloc(num_quadros, sizeof(Quadro));
    if(!tq){
        printf("Não foi possível alocar a tabela de quadros.\n");
        exit(2);
    }

    if (algoritmo == 'O') {
        pre_processa(arquivo, shift, num_paginas);
    }
    rewind(arquivo);

    int pf = 0, escrita_ps = 0, time = 0;
    unsigned int addr;
    char rw;
    int np, quadro;

    while(fscanf(arquivo, "%x %c ", &addr, &rw) == 2) {
        time++;
        np = addr >> shift;

        if(tp[np].presente) {
            tp[np].R = 1;
            if (rw == 'W') tp[np].M = 1;

            quadro = tp[np].quadro;
            tq[quadro].R = 1;
            if(rw == 'W') tq[quadro].M = 1;
            tq[quadro].ultimo_acesso = time;

            if (algoritmo == 'O') {
                proximo_acesso(np, time);
            }
        }
        else {
            pf++;

            if (algoritmo == 'O') {
                proximo_acesso(np, time);
            }

            quadro = substituicao(algoritmo, tp, tq, num_quadros, np, time);

            if (tq[quadro].M == 1) {
                escrita_ps++;
            }

            tq[quadro].pagina = np;
            tq[quadro].R = 1;
            tq[quadro].M = (rw == 'W') ? 1 : 0;
            tq[quadro].ultimo_acesso = time;

            tp[np].quadro = quadro;
            tp[np].presente = 1;
            tp[np].R = 1;
            tp[np].M = (rw == 'W') ? 1 : 0;
        }
    }

    if (algoritmo == 'O') {
        for (int i = 0; i < total_paginas_global; i++) {
            if (paginas_acessos[i].acessos != NULL) {
                free(paginas_acessos[i].acessos);
            }
        }
        free(paginas_acessos);
    }

    free(tp);
    free(tq);
    fclose(arquivo);

    printf("Número de Faltas de Páginas: %d\n", pf);
    printf("Número de Páginas Escritas: %d\n\n", escrita_ps);

    return 0;
}
