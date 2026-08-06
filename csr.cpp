#include "csr.h"

#include <algorithm>
#include <limits.h>

static void limparMatrizCSR(MatrizCSR *matriz) {
    matriz->quantidade_linhas = 0;
    matriz->quantidade_colunas = 0;
    matriz->values.clear();
    matriz->col_index.clear();
    matriz->row_ptr.clear();
}

static int produtoInternoLinhas(
    const MatrizCSR *matriz,
    int linha_a,
    int linha_b
) {
    int indice_a = matriz->row_ptr[linha_a];
    int fim_a = matriz->row_ptr[linha_a + 1];
    int indice_b = matriz->row_ptr[linha_b];
    int fim_b = matriz->row_ptr[linha_b + 1];
    int soma = 0;

    while (indice_a < fim_a && indice_b < fim_b) {
        int coluna_a = matriz->col_index[indice_a];
        int coluna_b = matriz->col_index[indice_b];

        if (coluna_a < coluna_b) {
            indice_a++;
        } else if (coluna_b < coluna_a) {
            indice_b++;
        } else {
            soma += matriz->values[indice_a] * matriz->values[indice_b];
            indice_a++;
            indice_b++;
        }
    }

    return soma;
}

bool construirMatrizComprasCSR(
    const ListaCompras *lista,
    int limite_clientes,
    MatrizCSR *matriz
) {
    if (lista == NULL || matriz == NULL || limite_clientes <= 0) {
        return false;
    }

    int total_clientes = (int)lista->codigos_clientes.size();
    int quantidade_clientes = limite_clientes;

    if (quantidade_clientes > total_clientes) {
        quantidade_clientes = total_clientes;
    }

    limparMatrizCSR(matriz);
    matriz->quantidade_linhas = quantidade_clientes;
    matriz->quantidade_colunas = (int)lista->nomes_produtos.size();
    matriz->row_ptr.push_back(0);

    for (int cliente = 0; cliente < quantidade_clientes; cliente++) {
        std::list<int> compras = lista->compras_clientes[cliente];
        std::vector<int> produtos;

        while (!compras.empty()) {
            produtos.push_back(compras.front());
            compras.pop_front();
        }

        if (!produtos.empty()) {
            std::sort(&produtos[0], &produtos[0] + produtos.size());
        }

        for (size_t indice = 0; indice < produtos.size(); indice++) {
            matriz->values.push_back(1);
            matriz->col_index.push_back(produtos[indice]);
        }

        matriz->row_ptr.push_back((int)matriz->values.size());
    }

    return quantidade_clientes > 0 && matriz->quantidade_colunas > 0;
}

bool multiplicarCSRPorTransposta(
    const MatrizCSR *matriz,
    MatrizCSR *resultado
) {
    if (matriz == NULL || resultado == NULL ||
        matriz->quantidade_linhas < 0 || matriz->quantidade_colunas < 0 ||
        matriz->row_ptr.size() != (size_t)matriz->quantidade_linhas + 1 ||
        matriz->values.size() != matriz->col_index.size()) {
        return false;
    }

    limparMatrizCSR(resultado);
    resultado->quantidade_linhas = matriz->quantidade_linhas;
    resultado->quantidade_colunas = matriz->quantidade_linhas;
    resultado->row_ptr.push_back(0);

    for (int linha = 0; linha < matriz->quantidade_linhas; linha++) {
        for (int coluna = 0; coluna < matriz->quantidade_linhas; coluna++) {
            int soma = produtoInternoLinhas(matriz, linha, coluna);

            if (soma != 0) {
                resultado->values.push_back(soma);
                resultado->col_index.push_back(coluna);
            }
        }

        resultado->row_ptr.push_back((int)resultado->values.size());
    }

    return true;
}

bool construirSimilaridadeCSR(
    const ListaCompras *lista,
    int limite_clientes,
    SimilaridadeCSR *similaridade
) {
    MatrizCSR compras;

    if (similaridade == NULL ||
        !construirMatrizComprasCSR(lista, limite_clientes, &compras)) {
        return false;
    }

    if (!multiplicarCSRPorTransposta(&compras, &similaridade->intersecoes)) {
        return false;
    }

    similaridade->quantidade_compras.clear();

    for (int linha = 0; linha < compras.quantidade_linhas; linha++) {
        similaridade->quantidade_compras.push_back(
            compras.row_ptr[linha + 1] - compras.row_ptr[linha]
        );
    }

    return true;
}

bool obterValorCSR(
    const MatrizCSR *matriz,
    int linha,
    int coluna,
    int *valor
) {
    if (matriz == NULL || valor == NULL || linha < 0 || coluna < 0 ||
        linha >= matriz->quantidade_linhas ||
        coluna >= matriz->quantidade_colunas ||
        matriz->row_ptr.size() != (size_t)matriz->quantidade_linhas + 1) {
        return false;
    }

    int inicio = matriz->row_ptr[linha];
    int fim = matriz->row_ptr[linha + 1];

    while (inicio < fim) {
        int meio = inicio + (fim - inicio) / 2;

        if (matriz->col_index[meio] < coluna) {
            inicio = meio + 1;
        } else {
            fim = meio;
        }
    }

    if (inicio < matriz->row_ptr[linha + 1] &&
        matriz->col_index[inicio] == coluna) {
        *valor = matriz->values[inicio];
    } else {
        *valor = 0;
    }

    return true;
}

bool obterSimilaridadeCSR(
    const SimilaridadeCSR *similaridade,
    int cliente_1,
    int cliente_2,
    double *valor
) {
    int intersecao;

    if (similaridade == NULL || valor == NULL ||
        cliente_1 < 0 ||
        cliente_1 >= (int)similaridade->quantidade_compras.size() ||
        cliente_2 < 0 ||
        cliente_2 >= (int)similaridade->quantidade_compras.size() ||
        !obterValorCSR(
            &similaridade->intersecoes,
            cliente_1,
            cliente_2,
            &intersecao
        )) {
        return false;
    }

    int quantidade = similaridade->quantidade_compras[cliente_1];

    if (quantidade == 0) {
        *valor = cliente_1 == cliente_2 ? 0.0 : 1.0;
    } else {
        *valor = 1.0 - (double)intersecao / quantidade;
    }

    return true;
}

int encontrarSimilarCSR(
    int cliente_idx,
    const SimilaridadeCSR *similaridade
) {
    if (similaridade == NULL || cliente_idx < 0 ||
        cliente_idx >= similaridade->intersecoes.quantidade_linhas) {
        return -1;
    }

    int inicio = similaridade->intersecoes.row_ptr[cliente_idx];
    int fim = similaridade->intersecoes.row_ptr[cliente_idx + 1];
    int melhor_cliente = -1;
    int maior_intersecao = -1;

    for (int indice = inicio; indice < fim; indice++) {
        int candidato = similaridade->intersecoes.col_index[indice];
        int intersecao = similaridade->intersecoes.values[indice];

        if (candidato != cliente_idx && intersecao > maior_intersecao) {
            melhor_cliente = candidato;
            maior_intersecao = intersecao;
        }
    }

    if (melhor_cliente < 0 &&
        similaridade->intersecoes.quantidade_linhas > 1) {
        melhor_cliente = cliente_idx == 0 ? 1 : 0;
    }

    return melhor_cliente;
}

unsigned long long estimarMemoriaCSR(const MatrizCSR *matriz) {
    if (matriz == NULL) {
        return 0;
    }

    return (unsigned long long)matriz->values.size() * sizeof(int) +
           (unsigned long long)matriz->col_index.size() * sizeof(int) +
           (unsigned long long)matriz->row_ptr.size() * sizeof(int);
}

unsigned long long estimarMemoriaDensaInt(int linhas, int colunas) {
    if (linhas < 0 || colunas < 0) {
        return 0;
    }

    return (unsigned long long)linhas *
           (unsigned long long)colunas * sizeof(int);
}

unsigned long long estimarMemoriaDensaDouble(int linhas, int colunas) {
    if (linhas < 0 || colunas < 0) {
        return 0;
    }

    return (unsigned long long)linhas *
           (unsigned long long)colunas * sizeof(double);
}