#ifndef CSR_H
#define CSR_H

#include "lista_compras.h"

#include <vector>

typedef struct {
    int quantidade_linhas;
    int quantidade_colunas;
    std::vector<int> values;
    std::vector<int> col_index;
    std::vector<int> row_ptr;
} MatrizCSR;

typedef struct {
    MatrizCSR intersecoes;
    std::vector<int> quantidade_compras;
} SimilaridadeCSR;

bool construirMatrizComprasCSR(
    const ListaCompras *lista,
    int limite_clientes,
    MatrizCSR *matriz
);
bool multiplicarCSRPorTransposta(
    const MatrizCSR *matriz,
    MatrizCSR *resultado
);
bool construirSimilaridadeCSR(
    const ListaCompras *lista,
    int limite_clientes,
    SimilaridadeCSR *similaridade
);
bool obterValorCSR(
    const MatrizCSR *matriz,
    int linha,
    int coluna,
    int *valor
);
bool obterSimilaridadeCSR(
    const SimilaridadeCSR *similaridade,
    int cliente_1,
    int cliente_2,
    double *valor
);
int encontrarSimilarCSR(
    int cliente_idx,
    const SimilaridadeCSR *similaridade
);
unsigned long long estimarMemoriaCSR(const MatrizCSR *matriz);
unsigned long long estimarMemoriaDensaInt(int linhas, int colunas);
unsigned long long estimarMemoriaDensaDouble(int linhas, int colunas);

#endif
