#include "csr.h"
#include "lista_compras.h"
#include "recomendacao.h"
#include "similaridade.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

#define TOLERANCIA 0.000000001

static bool converterInteiroPositivo(const char *texto, int *valor) {
    char *fim;
    long numero = strtol(texto, &fim, 10);

    if (texto[0] == '\0' || fim[0] != '\0' ||
        numero <= 0 || numero > INT_MAX) {
        return false;
    }

    *valor = (int)numero;
    return true;
}

static double paraMiB(unsigned long long bytes) {
    return (double)bytes / (1024.0 * 1024.0);
}

static void criarAmostra(
    const ListaCompras *lista,
    int quantidade,
    ListaCompras *amostra
) {
    amostra->codigos_clientes.clear();
    amostra->indices_clientes.clear();
    amostra->nomes_produtos = lista->nomes_produtos;
    amostra->indices_produtos = lista->indices_produtos;
    amostra->compras_clientes.clear();

    for (int cliente = 0; cliente < quantidade; cliente++) {
        amostra->codigos_clientes.push_back(
            lista->codigos_clientes[cliente]
        );
        amostra->indices_clientes[
            lista->codigos_clientes[cliente]
        ] = cliente;
        amostra->compras_clientes.push_back(
            lista->compras_clientes[cliente]
        );
    }
}

static bool validarCSR(const MatrizCSR *matriz) {
    if (matriz->row_ptr.size() !=
            (size_t)matriz->quantidade_linhas + 1 ||
        matriz->values.size() != matriz->col_index.size() ||
        matriz->row_ptr.empty() || matriz->row_ptr[0] != 0 ||
        matriz->row_ptr[matriz->quantidade_linhas] !=
            (int)matriz->values.size()) {
        return false;
    }

    for (int linha = 0; linha < matriz->quantidade_linhas; linha++) {
        if (matriz->row_ptr[linha] > matriz->row_ptr[linha + 1]) {
            return false;
        }

        for (int indice = matriz->row_ptr[linha];
             indice < matriz->row_ptr[linha + 1];
             indice++) {
            if (matriz->col_index[indice] < 0 ||
                matriz->col_index[indice] >= matriz->quantidade_colunas ||
                matriz->values[indice] == 0 ||
                (indice > matriz->row_ptr[linha] &&
                 matriz->col_index[indice - 1] >=
                    matriz->col_index[indice])) {
                return false;
            }
        }
    }

    return true;
}

static bool testarExemploConhecido() {
    MatrizCSR matriz;
    MatrizCSR resultado;
    int esperado[3][3] = {
        {2, 0, 1},
        {0, 1, 1},
        {1, 1, 2}
    };

    matriz.quantidade_linhas = 3;
    matriz.quantidade_colunas = 3;
    matriz.values.push_back(1);
    matriz.values.push_back(1);
    matriz.values.push_back(1);
    matriz.values.push_back(1);
    matriz.values.push_back(1);
    matriz.col_index.push_back(0);
    matriz.col_index.push_back(2);
    matriz.col_index.push_back(1);
    matriz.col_index.push_back(0);
    matriz.col_index.push_back(1);
    matriz.row_ptr.push_back(0);
    matriz.row_ptr.push_back(2);
    matriz.row_ptr.push_back(3);
    matriz.row_ptr.push_back(5);

    if (!multiplicarCSRPorTransposta(&matriz, &resultado) ||
        !validarCSR(&resultado)) {
        return false;
    }

    for (int linha = 0; linha < 3; linha++) {
        for (int coluna = 0; coluna < 3; coluna++) {
            int valor;

            if (!obterValorCSR(&resultado, linha, coluna, &valor) ||
                valor != esperado[linha][coluna]) {
                return false;
            }
        }
    }

    return true;
}

static bool compararComDensa(
    const ListaCompras *lista,
    int quantidade_solicitada
) {
    int quantidade = quantidade_solicitada;
    int total = (int)lista->codigos_clientes.size();

    if (quantidade > total) {
        quantidade = total;
    }

    clock_t inicio_densa = clock();
    std::vector<std::vector<int> > compras_densa =
        construirMatrizComprasLimitada(lista, quantidade);
    std::vector<std::vector<int> > intersecao_densa;

    if (!construirMatrizIntersecao(
            &compras_densa,
            ALGORITMO_ADAPTADO,
            &intersecao_densa
        )) {
        return false;
    }

    std::vector<std::vector<double> > similaridade_densa =
        calcularMatrizSimilaridade(&intersecao_densa, lista);
    clock_t fim_densa = clock();

    clock_t inicio_csr = clock();
    SimilaridadeCSR similaridade_csr;

    if (!construirSimilaridadeCSR(
            lista,
            quantidade,
            &similaridade_csr
        )) {
        return false;
    }

    clock_t fim_csr = clock();
    MatrizCSR compras_csr;

    if (!construirMatrizComprasCSR(lista, quantidade, &compras_csr) ||
        !validarCSR(&compras_csr) ||
        !validarCSR(&similaridade_csr.intersecoes)) {
        return false;
    }

    bool iguais = true;

    for (int linha = 0; linha < quantidade && iguais; linha++) {
        for (int coluna = 0; coluna < quantidade; coluna++) {
            int valor_csr;
            double distancia_csr;

            if (!obterValorCSR(
                    &similaridade_csr.intersecoes,
                    linha,
                    coluna,
                    &valor_csr
                ) ||
                !obterSimilaridadeCSR(
                    &similaridade_csr,
                    linha,
                    coluna,
                    &distancia_csr
                ) ||
                valor_csr != intersecao_densa[linha][coluna] ||
                fabs(distancia_csr -
                     similaridade_densa[linha][coluna]) > TOLERANCIA) {
                iguais = false;
                break;
            }
        }
    }

    int quantidade_testes_recomendacao = quantidade;
    ListaCompras amostra;
    criarAmostra(lista, quantidade, &amostra);

    if (quantidade_testes_recomendacao > 20) {
        quantidade_testes_recomendacao = 20;
    }

    for (int cliente = 0;
         cliente < quantidade_testes_recomendacao && iguais;
         cliente++) {
        std::vector<int> recomendacoes_densas = recomendarProdutos(
            cliente,
            5,
            &amostra,
            &similaridade_densa
        );
        std::vector<int> recomendacoes_csr = recomendarProdutosCSR(
            cliente,
            5,
            &amostra,
            &similaridade_csr
        );

        if (recomendacoes_densas.size() != recomendacoes_csr.size()) {
            iguais = false;
        }

        for (size_t indice = 0;
             indice < recomendacoes_densas.size() && iguais;
             indice++) {
            if (recomendacoes_densas[indice] != recomendacoes_csr[indice]) {
                iguais = false;
            }
        }
    }

    unsigned long long memoria_densa =
        estimarMemoriaDensaInt(
            quantidade,
            (int)lista->nomes_produtos.size()
        ) +
        estimarMemoriaDensaInt(quantidade, quantidade) +
        estimarMemoriaDensaDouble(quantidade, quantidade);
    unsigned long long memoria_csr =
        estimarMemoriaCSR(&compras_csr) +
        estimarMemoriaCSR(&similaridade_csr.intersecoes) +
        (unsigned long long)similaridade_csr.quantidade_compras.size() *
            sizeof(int);
    double tempo_densa =
        (double)(fim_densa - inicio_densa) / CLOCKS_PER_SEC;
    double tempo_csr =
        (double)(fim_csr - inicio_csr) / CLOCKS_PER_SEC;

    printf("%8d | %10.3f | %8.3f | %8.3f | %6.2fx | %s\n",
           quantidade,
           tempo_densa,
           tempo_csr,
           paraMiB(memoria_csr),
           memoria_csr > 0 ? (double)memoria_densa / memoria_csr : 0.0,
           iguais ? "SIM" : "NAO");

    return iguais;
}

int main(int argc, char *argv[]) {
    int tamanhos_padrao[3] = {100, 250, 500};

    if (argc < 2) {
        printf("Uso: %s arquivo.csv [clientes ...]\n", argv[0]);
        return 1;
    }

    if (!testarExemploConhecido()) {
        printf("Teste conhecido CSR: FALHOU\n");
        return 1;
    }

    ListaCompras lista;

    if (!carregarListaCompras(&lista, argv[1]) ||
        lista.codigos_clientes.empty() ||
        lista.nomes_produtos.empty()) {
        printf("Nao foi possivel carregar uma base valida de %s.\n", argv[1]);
        return 1;
    }

    for (int indice = 2; indice < argc; indice++) {
        int quantidade;

        if (!converterInteiroPositivo(argv[indice], &quantidade)) {
            printf("Quantidade de clientes invalida: %s.\n", argv[indice]);
            return 1;
        }
    }

    MatrizCSR compras_completa;

    if (!construirMatrizComprasCSR(
            &lista,
            (int)lista.codigos_clientes.size(),
            &compras_completa
        ) ||
        !validarCSR(&compras_completa)) {
        printf("Falha ao construir a matriz de compras CSR.\n");
        return 1;
    }

    unsigned long long densa_completa =
        estimarMemoriaDensaInt(
            compras_completa.quantidade_linhas,
            compras_completa.quantidade_colunas
        );
    unsigned long long csr_completa = estimarMemoriaCSR(&compras_completa);

    printf("Teste conhecido CSR: OK\n");
    printf("Base: %d clientes, %d produtos, %d compras distintas\n",
           compras_completa.quantidade_linhas,
           compras_completa.quantidade_colunas,
           (int)compras_completa.values.size());
    printf("Matriz de compras: densa %.3f MiB; CSR %.3f MiB; "
           "economia %.2fx\n\n",
           paraMiB(densa_completa),
           paraMiB(csr_completa),
           csr_completa > 0 ? (double)densa_completa / csr_completa : 0.0);
    printf("Clientes | Densa (s) | CSR (s)  | CSR MiB  | Economia | "
           "Matrizes/recomendacoes iguais\n");

    bool sucesso = true;

    if (argc == 2) {
        for (int indice = 0; indice < 3; indice++) {
            if (!compararComDensa(&lista, tamanhos_padrao[indice])) {
                sucesso = false;
            }
        }
    } else {
        for (int indice = 2; indice < argc; indice++) {
            int quantidade;
            converterInteiroPositivo(argv[indice], &quantidade);

            if (!compararComDensa(&lista, quantidade)) {
                sucesso = false;
            }
        }
    }

    return sucesso ? 0 : 1;
}
