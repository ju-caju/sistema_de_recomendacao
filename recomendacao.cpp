#include "recomendacao.h"
#include <algorithm>

static bool compararPorRank(ProdutoRank a, ProdutoRank b) {
    if (a.rank == b.rank) {
        return a.id_produto < b.id_produto;
    }

    return a.rank < b.rank;
}

std::vector<int> recomendarProdutos(
    int cliente_idx,
    int k,
    const ListaCompras *lista,
    const std::vector<std::vector<double> > *matriz_similaridade
) {
    std::vector<int> vazio;

    if (lista == NULL || matriz_similaridade == NULL || cliente_idx < 0 ||
        cliente_idx >= (int)lista->codigos_clientes.size() || k <= 0) {
        return vazio;
    }

    int n_clientes = (int)lista->codigos_clientes.size();
    int n_produtos = (int)lista->nomes_produtos.size();

    std::vector<int> vizinhos;
    for (int j = 0; j < n_clientes; ++j) {
        if (j != cliente_idx
            && (*matriz_similaridade)[cliente_idx][j] < 1.0) {
            vizinhos.push_back(j);
        }
    }
    std::vector<ProdutoRank> R(n_produtos);
    for (int p = 0; p < n_produtos; ++p) {
        R[p].id_produto = p;
        R[p].rank = 1.0;
    }
    std::vector<bool> cliente_ja_comprou(n_produtos, false);

    std::list<int> compras_cliente =
        lista->compras_clientes[cliente_idx];
    while (!compras_cliente.empty()) {
        int p = compras_cliente.front();
        cliente_ja_comprou[p] = true;
        compras_cliente.pop_front();
    }

    for (size_t i = 0; i < vizinhos.size(); i++) {
        int s = vizinhos[i];
        double distancia_c_s = (*matriz_similaridade)[cliente_idx][s];
        std::list<int> compras_vizinho = lista->compras_clientes[s];

        while (!compras_vizinho.empty()) {
            int p = compras_vizinho.front();
            if (!cliente_ja_comprou[p]) {
                R[p].rank = R[p].rank * distancia_c_s;
            }
            compras_vizinho.pop_front();
        }
    }
    if (!R.empty()) {
        ProdutoRank *inicio = &R[0];
        ProdutoRank *fim = inicio + R.size();
        std::sort(inicio, fim, compararPorRank);
    }

    std::vector<int> top_k;
    for (size_t i = 0; i < R.size() && i < (size_t)k; ++i) {
        top_k.push_back(R[i].id_produto);
    }
    return top_k;
}
