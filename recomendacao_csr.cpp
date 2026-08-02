#include "recomendacao.h"

#include <algorithm>

static bool compararProdutoCSR(ProdutoRank a, ProdutoRank b) {
    if (a.rank == b.rank) {
        return a.id_produto < b.id_produto;
    }

    return a.rank < b.rank;
}

std::vector<int> recomendarProdutosCSR(
    int cliente_idx,
    int k,
    const ListaCompras *lista,
    const SimilaridadeCSR *similaridade
) {
    std::vector<int> vazio;

    if (lista == NULL || similaridade == NULL || cliente_idx < 0 ||
        cliente_idx >= similaridade->intersecoes.quantidade_linhas || k <= 0) {
        return vazio;
    }

    int n_produtos = (int)lista->nomes_produtos.size();
    std::vector<ProdutoRank> ranking(n_produtos);
    std::vector<char> cliente_ja_comprou(n_produtos, 0);
    std::list<int> compras_cliente = lista->compras_clientes[cliente_idx];

    for (int produto = 0; produto < n_produtos; produto++) {
        ranking[produto].id_produto = produto;
        ranking[produto].rank = 1.0;
    }

    while (!compras_cliente.empty()) {
        cliente_ja_comprou[compras_cliente.front()] = 1;
        compras_cliente.pop_front();
    }

    int inicio = similaridade->intersecoes.row_ptr[cliente_idx];
    int fim = similaridade->intersecoes.row_ptr[cliente_idx + 1];

    for (int indice = inicio; indice < fim; indice++) {
        int vizinho = similaridade->intersecoes.col_index[indice];

        if (vizinho != cliente_idx) {
            double distancia;

            if (obterSimilaridadeCSR(
                    similaridade,
                    cliente_idx,
                    vizinho,
                    &distancia
                ) && distancia < 1.0) {
                std::list<int> compras_vizinho =
                    lista->compras_clientes[vizinho];

                while (!compras_vizinho.empty()) {
                    int produto = compras_vizinho.front();

                    if (!cliente_ja_comprou[produto]) {
                        ranking[produto].rank *= distancia;
                    }

                    compras_vizinho.pop_front();
                }
            }
        }
    }

    if (!ranking.empty()) {
        std::sort(
            &ranking[0],
            &ranking[0] + ranking.size(),
            compararProdutoCSR
        );
    }

    std::vector<int> top_k;

    for (size_t indice = 0;
         indice < ranking.size() && indice < (size_t)k;
         indice++) {
        top_k.push_back(ranking[indice].id_produto);
    }

    return top_k;
}
