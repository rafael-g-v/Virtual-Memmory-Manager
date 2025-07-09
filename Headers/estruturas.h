#include <stdio.h>

typedef struct {
  int pagina;
  int R;
  int M;
  unsigned int ultimo_acesso;
  int segunda_chance;
} Quadro;

typedef struct {
  int quadro;
  int presente;
  int R;
  int M; 
} Pagina;

typedef struct {
    int *acessos;
    int tamanho;
    int indice_atual;
} PaginaAcessos;

extern PaginaAcessos *paginas_acessos;
extern int total_paginas_global;

int substituicao(char algoritmo, Pagina *tp, Quadro *tq, int num_quadros, int np, int time);
void pre_processa(FILE *arquivo, int shift, int num_paginas);
int proximo_acesso(int pagina, int tempo_atual);
