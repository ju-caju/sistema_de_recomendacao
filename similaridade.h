#ifndef SIMILARIDADE_H
#define SIMILARIDADE_H

#include "lista_compras.h"
#include <vector>

#define ALGORITMO_PADRAO 0
#define ALGORITMO_ADAPTADO 1

std::vector<std::vector<int> > construirMatrizCompras(
    const ListaCompras *lista
);
std::vector<std::vector<int> > construirMatrizComprasLimitada(
    const ListaCompras *lista,
    int limite_clientes
);
std::vector<std::vector<int> > transporMatriz(
    const std::vector<std::vector<int> > *matriz
);
std::vector<std::vector<int> > multiplicarMatrizes(
    const std::vector<std::vector<int> > *matriz_a,
    const std::vector<std::vector<int> > *matriz_b
);
std::vector<std::vector<int> > multiplicarPorTranspostaAdaptado(
    const std::vector<std::vector<int> > *matriz
);
bool construirMatrizIntersecao(
    const std::vector<std::vector<int> > *matriz_compras,
    int algoritmo,
    std::vector<std::vector<int> > *intersecao
);
std::vector<std::vector<double> > calcularMatrizSimilaridade(
    const std::vector<std::vector<int> > *intersecao,
    const ListaCompras *lista
);

int encontrarSimilar(
    int cliente_idx,
    const std::vector<std::vector<double> > *matriz_similaridade
);

#endif
