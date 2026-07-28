#ifndef RECOMENDACAO_H
#define RECOMENDACAO_H

#include "lista_compras.h"
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

#endif
