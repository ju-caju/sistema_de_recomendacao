#include "recomendacao.h"
#include <algorithm>

static bool compararPorRank(const ProdutoRank &a, const ProdutoRank &b) {
    return a.rank < b.rank;
}

std::vector<int> recomendarProdutos(int cliente_idx, int k, const ListaCompras &lista, const std::vector<std::vector<double> > &matriz_similaridade) {
    int n_clientes = lista.codigos_clientes.size();
    int n_produtos = lista.nomes_produtos.size();

    std::vector<int> vizinhos;
    for (int j = 0; j < n_clientes; ++j) {
        if (j != cliente_idx && matriz_similaridade[cliente_idx][j] < 1.0) {
            vizinhos.push_back(j);
        }
    }
    std::vector<ProdutoRank> R(n_produtos);
    for (int p = 0; p < n_produtos; ++p) {
        R[p].id_produto = p;
        R[p].rank = 1.0;
    }
    std::vector<bool> cliente_ja_comprou(n_produtos, false);
    for (std::list<int>::const_iterator it = lista.compras_clientes[cliente_idx].begin(); 
         it != lista.compras_clientes[cliente_idx].end(); ++it) {
        cliente_ja_comprou[*it] = true;
    }

    for (size_t i = 0; i < vizinhos.size(); ++i) {
        int s = vizinhos[i]; // Índice do vizinho
        double distancia_c_s = matriz_similaridade[cliente_idx][s];

        for (std::list<int>::const_iterator it = lista.compras_clientes[s].begin(); 
             it != lista.compras_clientes[s].end(); ++it) {
            int p = *it;
            if (!cliente_ja_comprou[p]) {
                R[p].rank = R[p].rank * distancia_c_s;
            }
        }
    }
    std::vector<ProdutoRank> produtos_recomendaveis;
    for (int p = 0; p < n_produtos; ++p) {
        if (!cliente_ja_comprou[p] && R[p].rank < 1.0) {
            produtos_recomendaveis.push_back(R[p]);
        }
    }
    std::sort(produtos_recomendaveis.begin(), produtos_recomendaveis.end(), compararPorRank);

    std::vector<int> top_k;
    for (size_t i = 0; i < produtos_recomendaveis.size() && i < (size_t)k; ++i) {
        top_k.push_back(produtos_recomendaveis[i].id_produto);
    }

    return top_k;
}