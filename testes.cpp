#include "csr.h"
#include "lista_compras.h"
#include "recomendacao.h"
#include "similaridade.h"

#include <math.h>
#include <stdio.h>
#include <vector>

#define TOLERANCIA_TESTE 0.000000001

static bool verificar(bool condicao, const char *mensagem) {
    if (!condicao) {
        printf("FALHOU: %s\n", mensagem);
        return false;
    }

    return true;
}

static bool quaseIgual(double valor, double esperado) {
    return fabs(valor - esperado) <= TOLERANCIA_TESTE;
}

static bool testarLeitura(ListaCompras *lista) {
    if (!verificar(
            carregarListaCompras(lista, "dados/teste_regressao.csv"),
            "leitura do CSV de regressao"
        )) {
        return false;
    }

    return verificar(lista->codigos_clientes.size() == 3, "quantidade de clientes") &&
        verificar(lista->nomes_produtos.size() == 4, "quantidade de produtos") &&
        verificar(lista->compras_clientes[0].size() == 2, "remocao de duplicatas") &&
        verificar(lista->nomes_produtos[0] == "Produto, Um", "campo CSV entre aspas");
}

static bool testarLeituraPontoVirgula() {
    ListaCompras lista;

    if (!verificar(
            carregarListaCompras(
                &lista,
                "dados/teste_regressao_ponto_virgula.csv"
            ),
            "leitura do CSV separado por ponto e virgula"
        )) {
        return false;
    }

    return verificar(lista.codigos_clientes.size() == 2, "clientes com ponto e virgula") &&
        verificar(lista.nomes_produtos.size() == 2, "produtos com ponto e virgula") &&
        verificar(lista.compras_clientes[1].size() == 1, "duplicata com ponto e virgula");
}

static bool compararIntersecoes(
    const std::vector<std::vector<int> > *matriz_a,
    const std::vector<std::vector<int> > *matriz_b
) {
    if (matriz_a->size() != matriz_b->size()) {
        return false;
    }

    for (size_t linha = 0; linha < matriz_a->size(); linha++) {
        if ((*matriz_a)[linha].size() != (*matriz_b)[linha].size()) {
            return false;
        }

        for (size_t coluna = 0;
             coluna < (*matriz_a)[linha].size();
             coluna++) {
            if ((*matriz_a)[linha][coluna] != (*matriz_b)[linha][coluna]) {
                return false;
            }
        }
    }

    return true;
}

static bool testarSimilaridade(ListaCompras *lista) {
    std::vector<std::vector<int> > compras = construirMatrizCompras(lista);
    std::vector<std::vector<int> > intersecao_padrao;
    std::vector<std::vector<int> > intersecao_adaptada;

    if (!verificar(
            construirMatrizIntersecao(
                &compras,
                ALGORITMO_PADRAO,
                &intersecao_padrao
            ),
            "algoritmo padrao"
        ) ||
        !verificar(
            construirMatrizIntersecao(
                &compras,
                ALGORITMO_ADAPTADO,
                &intersecao_adaptada
            ),
            "algoritmo adaptado"
        ) ||
        !verificar(
            compararIntersecoes(&intersecao_padrao, &intersecao_adaptada),
            "intersecoes padrao e adaptada"
        )) {
        return false;
    }

    std::vector<std::vector<double> > distancias =
        calcularMatrizSimilaridade(&intersecao_padrao, lista);

    return verificar(quaseIgual(distancias[0][1], 0.5), "distancia C1 para C2") &&
        verificar(
            quaseIgual(distancias[1][0], 2.0 / 3.0),
            "distancia C2 para C1"
        ) &&
        verificar(
            !quaseIgual(distancias[0][1], distancias[1][0]),
            "assimetria exigida"
        );
}

static bool testarCSR(ListaCompras *lista) {
    lista->codigos_clientes.push_back("VAZIO");
    lista->indices_clientes["VAZIO"] = 3;
    lista->compras_clientes.push_back(std::list<int>());

    std::vector<std::vector<int> > compras = construirMatrizCompras(lista);
    std::vector<std::vector<int> > intersecao;
    construirMatrizIntersecao(&compras, ALGORITMO_ADAPTADO, &intersecao);
    std::vector<std::vector<double> > densa =
        calcularMatrizSimilaridade(&intersecao, lista);
    SimilaridadeCSR csr;

    if (!verificar(
            construirSimilaridadeCSR(
                lista,
                (int)lista->codigos_clientes.size(),
                &csr
            ),
            "construcao CSR"
        )) {
        return false;
    }

    for (int linha = 0; linha < (int)densa.size(); linha++) {
        for (int coluna = 0; coluna < (int)densa.size(); coluna++) {
            double distancia;

            if (!obterSimilaridadeCSR(&csr, linha, coluna, &distancia) ||
                !quaseIgual(distancia, densa[linha][coluna])) {
                return verificar(false, "igualdade entre distancias densa e CSR");
            }
        }
    }

    return verificar(quaseIgual(densa[3][3], 0.0), "cliente vazio consigo") &&
        verificar(quaseIgual(densa[3][0], 1.0), "cliente vazio com outro") &&
        verificar(encontrarSimilarCSR(3, &csr) == 0, "vizinho do cliente vazio");
}

static bool testarRecomendacao(ListaCompras *lista) {
    std::vector<std::vector<int> > compras = construirMatrizCompras(lista);
    std::vector<std::vector<int> > intersecao;
    construirMatrizIntersecao(&compras, ALGORITMO_ADAPTADO, &intersecao);
    std::vector<std::vector<double> > densa =
        calcularMatrizSimilaridade(&intersecao, lista);
    SimilaridadeCSR csr;
    construirSimilaridadeCSR(
        lista,
        (int)lista->codigos_clientes.size(),
        &csr
    );
    std::vector<int> recomendacao_densa =
        recomendarProdutos(0, 4, lista, &densa);
    std::vector<int> recomendacao_csr =
        recomendarProdutosCSR(0, 4, lista, &csr);

    if (!verificar(recomendacao_densa.size() == 4, "retorno exato de k produtos") ||
        !verificar(
            recomendacao_densa.size() == recomendacao_csr.size(),
            "quantidade de recomendacoes CSR"
        )) {
        return false;
    }

    for (size_t indice = 0; indice < recomendacao_densa.size(); indice++) {
        if (recomendacao_densa[indice] != recomendacao_csr[indice]) {
            return verificar(false, "recomendacoes densa e CSR");
        }
    }

    return verificar(recomendacao_densa[0] == 2, "primeiro produto recomendado") &&
        verificar(recomendacao_densa[1] == 3, "segundo produto recomendado");
}

int main() {
    ListaCompras lista;

    if (!testarLeitura(&lista) ||
        !testarLeituraPontoVirgula() ||
        !testarSimilaridade(&lista) ||
        !testarRecomendacao(&lista) ||
        !testarCSR(&lista)) {
        return 1;
    }

    printf("Todos os testes de regressao passaram.\n");
    return 0;
}
