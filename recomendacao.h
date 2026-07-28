#ifndef RECOMENDACAO_H
#define RECOMENDACAO_H

#include "lista_compras.h"
#include "csr.h"
#include <vector>

typedef struct{
    int id_produto;
    double rank;
} ProdutoRank;

std::vector<int> recomendarProdutos(
    int cliente_idx,
    int k,
    const ListaCompras *lista,
    const std::vector<std::vector<double> > *matriz_similaridade
);
std::vector<int> recomendarProdutosCSR(
    int cliente_idx,
    int k,
    const ListaCompras *lista,
    const SimilaridadeCSR *similaridade
);

#endif
