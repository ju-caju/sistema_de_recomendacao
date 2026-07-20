#include "similaridade.h"
#include <iostream>

std::vector<std::vector<int> > construirMatrizCompras(const ListaCompras &lista) {
    int n_clientes = lista.codigos_clientes.size();
    int n_produtos = lista.nomes_produtos.size();
    
    std::vector<std::vector<int> > A(n_clientes, std::vector<int>(n_produtos, 0));
    
    for (int i = 0; i < n_clientes; ++i) {
        for (std::list<int>::const_iterator it = lista.compras_clientes[i].begin(); 
             it != lista.compras_clientes[i].end(); ++it) {
            A[i][*it] = 1;
        }
    }
    return A;
}

std::vector<std::vector<int> > transporMatriz(const std::vector<std::vector<int> > &A) {
    if (A.empty()) return std::vector<std::vector<int> >();
    
    int m = A.size();
    int n = A[0].size();
    std::vector<std::vector<int> > T(n, std::vector<int>(m, 0));
    
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            T[j][i] = A[i][j];
        }
    }
    return T;
}

std::vector<std::vector<int> > multiplicarMatrizes(const std::vector<std::vector<int> > &A, const std::vector<std::vector<int> > &B) {
    int m = A.size();
    int n = A[0].size();
    int p = B[0].size();
    
    std::vector<std::vector<int> > C(m, std::vector<int>(p, 0));
    
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < p; ++j) {
            for (int k = 0; k < n; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return C;
}

std::vector<std::vector<double> > calcularMatrizSimilaridade(const std::vector<std::vector<int> > &intersecao, const ListaCompras &lista) {
    int n_clientes = intersecao.size();
    std::vector<std::vector<double> > S(n_clientes, std::vector<double>(n_clientes, 0.0));
    
    for (int i = 0; i < n_clientes; ++i) {
        double tamanho_Pi = lista.compras_clientes[i].size(); 
        
        for (int j = 0; j < n_clientes; ++j) {
            if (tamanho_Pi > 0) {
                S[i][j] = 1.0 - ((double)intersecao[i][j] / tamanho_Pi);
            } else {
            }
        }
    }
    return S;
}

int encontrarSimilaridade(int cliente_idx, const std::vector<std::vector<double> > &S) {
    int n_clientes = S.size();
    double menor_distancia = 2.0; 
    int vizinho_mais_similar = -1;
    
    for (int j = 0; j < n_clientes; ++j) {
        if (j == cliente_idx) continue; 
        
        if (S[cliente_idx][j] < menor_distancia) {
            menor_distancia = S[cliente_idx][j];
            vizinho_mais_similar = j;
        }
    }
    return vizinho_mais_similar;
}