#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "csr.h"
#include "lista_compras.h"
#include "recomendacao.h"
#include "similaridade.h"

#include <list>
#include <time.h>
#include <vector>

static bool montarListaCompras(
    const std::vector<std::vector<int> > &compras,
    int quantidade_produtos,
    ListaCompras *lista
) {
    if (lista == NULL || quantidade_produtos <= 0 || compras.empty()) {
        return false;
    }

    lista->codigos_clientes.clear();
    lista->indices_clientes.clear();
    lista->nomes_produtos.clear();
    lista->indices_produtos.clear();
    lista->compras_clientes.clear();

    lista->codigos_clientes.resize(compras.size());
    lista->nomes_produtos.resize(quantidade_produtos);
    lista->compras_clientes.resize(compras.size());

    for (size_t cliente = 0; cliente < compras.size(); cliente++) {
        for (size_t indice = 0; indice < compras[cliente].size(); indice++) {
            int produto = compras[cliente][indice];
            if (produto >= 0 && produto < quantidade_produtos) {
                lista->compras_clientes[cliente].push_back(produto);
            }
        }
    }

    return true;
}

static std::vector<std::vector<double> > calcularSimilaridadePython(
    std::vector<std::vector<int> > compras,
    int quantidade_produtos,
    int algoritmo
) {
    ListaCompras lista;
    std::vector<std::vector<double> > vazio;

    if (!montarListaCompras(compras, quantidade_produtos, &lista)) {
        return vazio;
    }

    if (algoritmo == 2) {
        SimilaridadeCSR csr;

        if (!construirSimilaridadeCSR(
                &lista,
                (int)lista.codigos_clientes.size(),
                &csr
            )) {
            return vazio;
        }

        int quantidade_clientes = (int)lista.codigos_clientes.size();
        std::vector<std::vector<double> > resultado(
            quantidade_clientes,
            std::vector<double>(quantidade_clientes, 0.0)
        );

        for (int linha = 0; linha < quantidade_clientes; linha++) {
            for (int coluna = 0; coluna < quantidade_clientes; coluna++) {
                if (!obterSimilaridadeCSR(
                        &csr,
                        linha,
                        coluna,
                        &resultado[linha][coluna]
                    )) {
                    return vazio;
                }
            }
        }

        return resultado;
    }

    std::vector<std::vector<int> > matriz = construirMatrizCompras(&lista);
    std::vector<std::vector<int> > intersecao;

    if (!construirMatrizIntersecao(&matriz, algoritmo, &intersecao)) {
        return vazio;
    }

    return calcularMatrizSimilaridade(&intersecao, &lista);
}

static int encontrarSimilarPython(
    std::vector<std::vector<int> > compras,
    int quantidade_produtos,
    int cliente,
    int algoritmo
) {
    ListaCompras lista;

    if (!montarListaCompras(compras, quantidade_produtos, &lista) ||
        cliente < 0 || cliente >= (int)compras.size()) {
        return -1;
    }

    if (algoritmo == 2) {
        SimilaridadeCSR csr;

        if (!construirSimilaridadeCSR(
                &lista,
                (int)lista.codigos_clientes.size(),
                &csr
            )) {
            return -1;
        }

        return encontrarSimilarCSR(cliente, &csr);
    }

    std::vector<std::vector<double> > similaridade =
        calcularSimilaridadePython(compras, quantidade_produtos, algoritmo);

    if (similaridade.empty()) {
        return -1;
    }

    return encontrarSimilar(cliente, &similaridade);
}

static std::vector<std::vector<double> > encontrarSimilaresPython(
    std::vector<std::vector<int> > compras,
    int quantidade_produtos,
    std::vector<int> clientes,
    int algoritmo
) {
    ListaCompras lista;
    std::vector<std::vector<double> > resultado;

    if (!montarListaCompras(compras, quantidade_produtos, &lista)) {
        return resultado;
    }

    if (algoritmo == 2) {
        SimilaridadeCSR csr;

        if (!construirSimilaridadeCSR(
                &lista,
                (int)lista.codigos_clientes.size(),
                &csr
            )) {
            return resultado;
        }

        for (size_t indice = 0; indice < clientes.size(); indice++) {
            int cliente = clientes[indice];
            std::vector<double> item;

            if (cliente < 0 || cliente >= (int)compras.size()) {
                item.push_back(-1.0);
                item.push_back(1.0);
            } else {
                int similar = encontrarSimilarCSR(cliente, &csr);
                double distancia = 1.0;

                if (similar >= 0) {
                    obterSimilaridadeCSR(
                        &csr,
                        cliente,
                        similar,
                        &distancia
                    );
                }

                item.push_back((double)similar);
                item.push_back(distancia);
            }
            resultado.push_back(item);
        }

        return resultado;
    }

    std::vector<std::vector<double> > similaridade =
        calcularSimilaridadePython(compras, quantidade_produtos, algoritmo);

    if (similaridade.empty()) {
        return resultado;
    }

    for (size_t indice = 0; indice < clientes.size(); indice++) {
        int cliente = clientes[indice];
        std::vector<double> item;

        if (cliente < 0 || cliente >= (int)compras.size()) {
            item.push_back(-1.0);
            item.push_back(1.0);
        } else {
            int similar = encontrarSimilar(cliente, &similaridade);
            item.push_back((double)similar);
            item.push_back(
                similar >= 0 ? similaridade[cliente][similar] : 1.0
            );
        }
        resultado.push_back(item);
    }

    return resultado;
}

static std::vector<int> recomendarProdutosPython(
    std::vector<std::vector<int> > compras,
    int quantidade_produtos,
    int cliente,
    int k,
    int algoritmo
) {
    ListaCompras lista;
    std::vector<int> vazio;

    if (!montarListaCompras(compras, quantidade_produtos, &lista) ||
        cliente < 0 || cliente >= (int)compras.size() || k <= 0) {
        return vazio;
    }

    if (algoritmo == 2) {
        SimilaridadeCSR csr;

        if (!construirSimilaridadeCSR(
                &lista,
                (int)lista.codigos_clientes.size(),
                &csr
            )) {
            return vazio;
        }

        return recomendarProdutosCSR(cliente, k, &lista, &csr);
    }

    std::vector<std::vector<double> > similaridade =
        calcularSimilaridadePython(compras, quantidade_produtos, algoritmo);

    if (similaridade.empty()) {
        return vazio;
    }

    return recomendarProdutos(cliente, k, &lista, &similaridade);
}

static std::vector<std::vector<int> > recomendarClientesPython(
    std::vector<std::vector<int> > compras,
    int quantidade_produtos,
    std::vector<int> clientes,
    int k,
    int algoritmo
) {
    ListaCompras lista;
    std::vector<std::vector<int> > resultado;

    if (!montarListaCompras(compras, quantidade_produtos, &lista) || k <= 0) {
        return resultado;
    }

    if (algoritmo == 2) {
        SimilaridadeCSR csr;

        if (!construirSimilaridadeCSR(
                &lista,
                (int)lista.codigos_clientes.size(),
                &csr
            )) {
            return resultado;
        }

        for (size_t indice = 0; indice < clientes.size(); indice++) {
            int cliente = clientes[indice];

            if (cliente < 0 || cliente >= (int)compras.size()) {
                resultado.push_back(std::vector<int>());
            } else {
                resultado.push_back(
                    recomendarProdutosCSR(cliente, k, &lista, &csr)
                );
            }
        }

        return resultado;
    }

    std::vector<std::vector<double> > similaridade =
        calcularSimilaridadePython(compras, quantidade_produtos, algoritmo);

    if (similaridade.empty()) {
        return resultado;
    }

    for (size_t indice = 0; indice < clientes.size(); indice++) {
        int cliente = clientes[indice];

        if (cliente < 0 || cliente >= (int)compras.size()) {
            resultado.push_back(std::vector<int>());
        } else {
            resultado.push_back(
                recomendarProdutos(cliente, k, &lista, &similaridade)
            );
        }
    }

    return resultado;
}

static std::vector<double> compararTemposPython(
    std::vector<std::vector<int> > compras,
    int quantidade_produtos
) {
    ListaCompras lista;
    std::vector<double> resultado;

    if (!montarListaCompras(compras, quantidade_produtos, &lista)) {
        return resultado;
    }

    std::vector<std::vector<int> > matriz = construirMatrizCompras(&lista);
    std::vector<std::vector<int> > intersecao;
    clock_t inicio = clock();
    construirMatrizIntersecao(&matriz, ALGORITMO_PADRAO, &intersecao);
    calcularMatrizSimilaridade(&intersecao, &lista);
    clock_t fim = clock();
    resultado.push_back((double)(fim - inicio) / CLOCKS_PER_SEC);

    inicio = clock();
    construirMatrizIntersecao(&matriz, ALGORITMO_ADAPTADO, &intersecao);
    calcularMatrizSimilaridade(&intersecao, &lista);
    fim = clock();
    resultado.push_back((double)(fim - inicio) / CLOCKS_PER_SEC);
    return resultado;
}

static std::vector<double> compararMemoriaCSRPython(
    std::vector<std::vector<int> > compras,
    int quantidade_produtos
) {
    ListaCompras lista;
    std::vector<double> resultado;

    if (!montarListaCompras(compras, quantidade_produtos, &lista)) {
        return resultado;
    }

    int quantidade_clientes = (int)lista.codigos_clientes.size();
    MatrizCSR matriz_compras;
    SimilaridadeCSR similaridade;

    if (!construirMatrizComprasCSR(
            &lista,
            quantidade_clientes,
            &matriz_compras
        )) {
        return resultado;
    }

    clock_t inicio = clock();

    if (!construirSimilaridadeCSR(
            &lista,
            quantidade_clientes,
            &similaridade
        )) {
        return resultado;
    }

    clock_t fim = clock();
    unsigned long long memoria_densa =
        estimarMemoriaDensaInt(quantidade_clientes, quantidade_produtos) +
        estimarMemoriaDensaInt(quantidade_clientes, quantidade_clientes) +
        estimarMemoriaDensaDouble(quantidade_clientes, quantidade_clientes);
    unsigned long long memoria_csr =
        estimarMemoriaCSR(&matriz_compras) +
        estimarMemoriaCSR(&similaridade.intersecoes) +
        (unsigned long long)similaridade.quantidade_compras.size() * sizeof(int);

    resultado.push_back((double)(fim - inicio) / CLOCKS_PER_SEC);
    resultado.push_back((double)memoria_densa);
    resultado.push_back((double)memoria_csr);
    return resultado;
}

PYBIND11_MODULE(sistema_recomendacao, modulo) {
    modulo.doc() = "API procedural do sistema de recomendacao";
    modulo.def("calcular_similaridade", &calcularSimilaridadePython);
    modulo.def("encontrar_similar", &encontrarSimilarPython);
    modulo.def("encontrar_similares", &encontrarSimilaresPython);
    modulo.def("recomendar_produtos", &recomendarProdutosPython);
    modulo.def("recomendar_clientes", &recomendarClientesPython);
    modulo.def("comparar_tempos", &compararTemposPython);
    modulo.def("comparar_memoria_csr", &compararMemoriaCSRPython);
}