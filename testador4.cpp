#include "lista_compras.h"
#include "similaridade.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

#define REPETICOES_BENCHMARK 1
#define TOLERANCIA_SIMILARIDADE 0.000000001

static bool converterInteiroPositivo(const char *texto, int *valor) {
    char *fim;
    long numero = strtol(texto, &fim, 10);

    if (texto[0] == '\0' || fim[0] != '\0'
        || numero <= 0 || numero > INT_MAX) {
        return false;
    }

    *valor = (int)numero;
    return true;
}

static bool matrizesInteirasIguais(
    const std::vector<std::vector<int> > *matriz_a,
    const std::vector<std::vector<int> > *matriz_b
) {
    if (matriz_a->size() != matriz_b->size()) {
        return false;
    }

    for (size_t i = 0; i < matriz_a->size(); i++) {
        if ((*matriz_a)[i].size() != (*matriz_b)[i].size()) {
            return false;
        }

        for (size_t j = 0; j < (*matriz_a)[i].size(); j++) {
            if ((*matriz_a)[i][j] != (*matriz_b)[i][j]) {
                return false;
            }
        }
    }

    return true;
}

static bool matrizesReaisIguais(
    const std::vector<std::vector<double> > *matriz_a,
    const std::vector<std::vector<double> > *matriz_b
) {
    if (matriz_a->size() != matriz_b->size()) {
        return false;
    }

    for (size_t i = 0; i < matriz_a->size(); i++) {
        if ((*matriz_a)[i].size() != (*matriz_b)[i].size()) {
            return false;
        }

        for (size_t j = 0; j < (*matriz_a)[i].size(); j++) {
            double diferenca =
                fabs((*matriz_a)[i][j] - (*matriz_b)[i][j]);

            if (diferenca > TOLERANCIA_SIMILARIDADE) {
                return false;
            }
        }
    }

    return true;
}

static bool testarMatrizConhecida() {
    int valores[3][3] = {
        {1, 0, 1},
        {0, 1, 0},
        {1, 1, 0}
    };
    int valores_esperados[3][3] = {
        {2, 0, 1},
        {0, 1, 1},
        {1, 1, 2}
    };
    std::vector<std::vector<int> > matriz(
        3,
        std::vector<int>(3, 0)
    );
    std::vector<std::vector<int> > esperado(
        3,
        std::vector<int>(3, 0)
    );
    std::vector<std::vector<int> > resultado_padrao;
    std::vector<std::vector<int> > resultado_adaptado;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            matriz[i][j] = valores[i][j];
            esperado[i][j] = valores_esperados[i][j];
        }
    }

    if (!construirMatrizIntersecao(
            &matriz,
            ALGORITMO_PADRAO,
            &resultado_padrao
        )) {
        return false;
    }

    if (!construirMatrizIntersecao(
            &matriz,
            ALGORITMO_ADAPTADO,
            &resultado_adaptado
        )) {
        return false;
    }

    return matrizesInteirasIguais(&resultado_padrao, &esperado)
        && matrizesInteirasIguais(&resultado_adaptado, &esperado);
}

static bool executarBenchmark(
    const ListaCompras *lista,
    int quantidade_solicitada
) {
    int total_clientes = (int)lista->codigos_clientes.size();
    int quantidade_clientes = quantidade_solicitada;

    if (quantidade_clientes > total_clientes) {
        quantidade_clientes = total_clientes;
    }

    std::vector<std::vector<int> > matriz_compras =
        construirMatrizComprasLimitada(lista, quantidade_clientes);

    if (matriz_compras.empty()) {
        printf("Falha ao construir a matriz de compras para %d clientes.\n",
               quantidade_clientes);
        return false;
    }

    int quantidade_produtos = (int)matriz_compras[0].size();
    std::vector<std::vector<int> > intersecao_padrao;
    std::vector<std::vector<int> > intersecao_adaptada;
    std::vector<std::vector<double> > similaridade_padrao;
    std::vector<std::vector<double> > similaridade_adaptada;
    double soma_tempo_padrao = 0.0;
    double soma_tempo_adaptado = 0.0;

    for (int repeticao = 0; repeticao < REPETICOES_BENCHMARK; repeticao++) {
        intersecao_padrao.clear();
        similaridade_padrao.clear();

        clock_t inicio_padrao = clock();
        bool sucesso_padrao = construirMatrizIntersecao(
            &matriz_compras,
            ALGORITMO_PADRAO,
            &intersecao_padrao
        );

        if (!sucesso_padrao) {
            printf("Falha na execucao do algoritmo padrao.\n");
            return false;
        }

        similaridade_padrao =
            calcularMatrizSimilaridade(&intersecao_padrao, lista);
        clock_t fim_padrao = clock();
        soma_tempo_padrao +=
            (double)(fim_padrao - inicio_padrao) / CLOCKS_PER_SEC;

        intersecao_adaptada.clear();
        similaridade_adaptada.clear();

        clock_t inicio_adaptado = clock();
        bool sucesso_adaptado = construirMatrizIntersecao(
            &matriz_compras,
            ALGORITMO_ADAPTADO,
            &intersecao_adaptada
        );

        if (!sucesso_adaptado) {
            printf("Falha na execucao do algoritmo adaptado.\n");
            return false;
        }

        similaridade_adaptada =
            calcularMatrizSimilaridade(&intersecao_adaptada, lista);
        clock_t fim_adaptado = clock();
        soma_tempo_adaptado +=
            (double)(fim_adaptado - inicio_adaptado) / CLOCKS_PER_SEC;
    }

    double tempo_padrao = soma_tempo_padrao / REPETICOES_BENCHMARK;
    double tempo_adaptado = soma_tempo_adaptado / REPETICOES_BENCHMARK;
    bool intersecoes_iguais =
        matrizesInteirasIguais(&intersecao_padrao, &intersecao_adaptada);
    bool similaridades_iguais =
        matrizesReaisIguais(&similaridade_padrao, &similaridade_adaptada);

    printf("%8d | %8d | %9.6f | %11.6f | ",
           quantidade_clientes,
           quantidade_produtos,
           tempo_padrao,
           tempo_adaptado);

    if (tempo_adaptado > 0.0) {
        printf("%9.2fx | ", tempo_padrao / tempo_adaptado);
    } else {
        printf("%10s | ", "N/D");
    }

    printf("%16s | %18s\n",
           intersecoes_iguais ? "SIM" : "NAO",
           similaridades_iguais ? "SIM" : "NAO");

    std::vector<std::vector<int> >().swap(matriz_compras);
    std::vector<std::vector<int> >().swap(intersecao_padrao);
    std::vector<std::vector<int> >().swap(intersecao_adaptada);
    std::vector<std::vector<double> >().swap(similaridade_padrao);
    std::vector<std::vector<double> >().swap(similaridade_adaptada);

    return intersecoes_iguais && similaridades_iguais;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s arquivo.csv quantidade_clientes"
               " [quantidade_clientes ...]\n", argv[0]);
        return 1;
    }

    if (!testarMatrizConhecida()) {
        printf("Teste da matriz conhecida: FALHOU\n");
        return 1;
    }

    ListaCompras lista;

    if (!carregarListaCompras(&lista, argv[1])) {
        printf("Nao foi possivel abrir o arquivo %s.\n", argv[1]);
        return 1;
    }

    if (lista.codigos_clientes.empty() || lista.nomes_produtos.empty()) {
        printf("O arquivo CSV nao possui clientes e produtos validos.\n");
        return 1;
    }

    for (int i = 2; i < argc; i++) {
        int quantidade;

        if (!converterInteiroPositivo(argv[i], &quantidade)) {
            printf("Quantidade de clientes invalida: %s.\n", argv[i]);
            return 1;
        }
    }

    printf("Teste da matriz conhecida: OK\n");
    printf("Comparacao dos algoritmos da Atividade 4\n");
    printf("Media de %d repeticoes; tempos incluem intersecao e similaridade.\n",
           REPETICOES_BENCHMARK);
    printf("Clientes | Produtos | Padrao(s) | Adaptado(s) | Aceleracao"
           " | Intersecao igual | Similaridade igual\n");

    bool todos_iguais = true;

    for (int i = 2; i < argc; i++) {
        int quantidade;
        converterInteiroPositivo(argv[i], &quantidade);

        if (!executarBenchmark(&lista, quantidade)) {
            todos_iguais = false;
        }
    }

    return todos_iguais ? 0 : 1;
}
