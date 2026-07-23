#ifndef SIMILARIDADE_H
#define SIMILARIDADE_H

#include "lista_compras.h"
#include <vector>

std::vector<std::vector<int> > construirMatrizCompras(const ListaCompras &lista);
std::vector<std::vector<int> > construirMatrizComprasLimitada(
    const ListaCompras &lista,
    int limite_clientes
);
std::vector<std::vector<int> > transporMatriz(const std::vector<std::vector<int> > &matriz);
std::vector<std::vector<int> > multiplicarMatrizes(const std::vector<std::vector<int> > &A, const std::vector<std::vector<int> > &B);
std::vector<std::vector<int> > multiplicarPorTranspostaAdaptado(
    const std::vector<std::vector<int> > &matriz
);
std::vector<std::vector<double> > calcularMatrizSimilaridade(const std::vector<std::vector<int> > &intersecao, const ListaCompras &lista);

int encontrarSimilar(int cliente_idx, const std::vector<std::vector<double> > &matriz_similaridade);

#endif
