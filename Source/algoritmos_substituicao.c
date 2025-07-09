#include "Headers/estruturas.h"
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

PaginaAcessos *paginas_acessos = NULL;
int total_paginas_global = 0;

void pre_processa(FILE *arquivo, int shift, int num_paginas) {
    rewind(arquivo);
    total_paginas_global = num_paginas;

    paginas_acessos = (PaginaAcessos *)calloc(num_paginas, sizeof(PaginaAcessos));
  
    unsigned int addr;
    char rw;
    int total_acessos = 0;

    while (fscanf(arquivo, "%x %c ", &addr, &rw) == 2) {
        int page = addr >> shift;
        if (page < num_paginas) {
            paginas_acessos[page].tamanho++;
            total_acessos++;
        }
    }

    rewind(arquivo);
    int *contador_acessos = (int *)calloc(num_paginas, sizeof(int));
    int tempo = 0;

    while (fscanf(arquivo, "%x %c ", &addr, &rw) == 2) {
        int page = addr >> shift;
        if (page < num_paginas) {
            if (paginas_acessos[page].acessos == NULL) {
                paginas_acessos[page].acessos = (int *)malloc(paginas_acessos[page].tamanho * sizeof(int));
            }

            paginas_acessos[page].acessos[contador_acessos[page]] = tempo;
            contador_acessos[page]++;
        }
        tempo++;
    }

    free(contador_acessos);
    rewind(arquivo);
}

int proximo_acesso(int pagina, int tempo_atual) {
    if (pagina < 0 || pagina >= total_paginas_global || 
        paginas_acessos[pagina].acessos == NULL) {
        return INT_MAX;
    }

    PaginaAcessos *pa = &paginas_acessos[pagina];

    while (pa->indice_atual < pa->tamanho && 
           pa->acessos[pa->indice_atual] <= tempo_atual) {
        pa->indice_atual++;
    }

    if (pa->indice_atual < pa->tamanho) {
        return pa->acessos[pa->indice_atual];
    }
    return INT_MAX;
}

int lru(Quadro *tq, int num_quadros) {
    int quadro = 0;
    unsigned int menor_tempo = tq[0].ultimo_acesso;

    for (int i = 1; i < num_quadros; i++) {
        if (tq[i].ultimo_acesso < menor_tempo) {
            menor_tempo = tq[i].ultimo_acesso;
            quadro = i;
        }
    }
    return quadro;
}

int seg_chance(Quadro *tq, int num_quadros, int *ponteiro) {
    int inicio = *ponteiro;
    int quadro_selecionado = -1;

    while (quadro_selecionado == -1) {
        if (tq[*ponteiro].R == 0) {
            quadro_selecionado = *ponteiro;
        } else {
            tq[*ponteiro].R = 0;
        }

        *ponteiro = (*ponteiro + 1) % num_quadros;

        if (*ponteiro == inicio && quadro_selecionado == -1) {
            quadro_selecionado = *ponteiro;
        }
    }

    return quadro_selecionado;
}

int clock(Quadro *tq, int num_quadros, int *ponteiro) {
    while (1) {
        if (tq[*ponteiro].R == 0) {
            int quadro_selecionado = *ponteiro;
            *ponteiro = (*ponteiro + 1) % num_quadros;
            return quadro_selecionado;
        } else {
            tq[*ponteiro].R = 0;
        }

        *ponteiro = (*ponteiro + 1) % num_quadros;
    }
}

int otimo(Pagina *tp, Quadro *tq, int num_quadros, int time) {
    int quadro_selecionado = -1;
    int max_futuro = -1;
    int encontrou_sem_futuro = 0;

    for (int i = 0; i < num_quadros; i++) {
        int pagina = tq[i].pagina;
        if (pagina == -1) continue;

        int prox = proximo_acesso(pagina, time);

        if (prox == INT_MAX) {
            quadro_selecionado = i;
            encontrou_sem_futuro = 1;
            break;
        }
      
        if (prox > max_futuro) {
            max_futuro = prox;
            quadro_selecionado = i;
        }
    }

    if (!encontrou_sem_futuro && quadro_selecionado == -1) {
        for (int i = 0; i < num_quadros; i++) {
            if (tq[i].pagina != -1) {
                quadro_selecionado = i;
                break;
            }
        }
    }

    return quadro_selecionado;
}

int substituicao(char algoritmo, Pagina *tp, Quadro *tq, int num_quadros, int np, int time) {
    static int ponteiro_segunda_chance = 0;
    static int ponteiro_clock = 0;
    int quadro;

    switch (algoritmo) {
        case 'L': // LRU
            quadro = lru(tq, num_quadros);
            break;

        case '2': // Segunda Chance
            quadro = seg_chance(tq, num_quadros, &ponteiro_segunda_chance);
            ponteiro_segunda_chance = (ponteiro_segunda_chance + 1) % num_quadros;
            break;

        case 'C': // Clock
            quadro = clock(tq, num_quadros, &ponteiro_clock);
            break;

        case 'O': // Ótimo
            quadro = otimo(tp, tq, num_quadros, time);
            break;
    }

    if (tq[quadro].pagina != -1) {
        tp[tq[quadro].pagina].presente = 0;
    }

    return quadro;
}
