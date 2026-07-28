#include "lista_compras.h"
#include "similaridade.h"
#include "recomendacao.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

static bool converterInteiro(const char *texto, int *valor) {
    char *fim;
    long numero = strtol(texto, &fim, 10);

    if (texto[0] == '\0' || fim[0] != '\0' || numero < INT_MIN || numero > INT_MAX) {
        return false;
    }

    *valor = (int)numero;
    return true;
}

static bool obterAlgoritmo(const char *nome, int *algoritmo) {
    if (strcmp(nome, "padrao") == 0) {
        *algoritmo = ALGORITMO_PADRAO;
        return true;
    }

    if (strcmp(nome, "adaptado") == 0) {
        *algoritmo = ALGORITMO_ADAPTADO;
        return true;
    }

    return false;
}

int main(int argc, char *argv[]) {
    int algoritmo = ALGORITMO_ADAPTADO;
    int k;

    if (argc != 6 && argc != 7) {
        printf("Uso: %s arquivo.csv quantidade_k codigo_cliente_1 codigo_cliente_2"
               " codigo_cliente_3 [padrao|adaptado]\n", argv[0]);
        return 1;
    }

    if (!converterInteiro(argv[2], &k) || k <= 0) {
        printf("A quantidade k deve ser um numero inteiro maior que zero.\n");
        return 1;
    }

    if (argc == 7 && !obterAlgoritmo(argv[6], &algoritmo)) {
        printf("Algoritmo invalido: %s. Use padrao ou adaptado.\n", argv[6]);
        return 1;
    }

    ListaCompras lista;
    if (!carregarListaCompras(&lista, argv[1])) {
        printf("Nao foi possivel abrir o arquivo %s.\n", argv[1]);
        return 1;
    }

    if (lista.codigos_clientes.empty() || lista.nomes_produtos.empty()) {
        printf("Nao ha dados suficientes para construir a matriz de compras.\n");
        return 1;
    }

    std::vector<std::vector<int> > matriz_compras =
        construirMatrizCompras(&lista);
    std::vector<std::vector<int> > intersecao;

    if (matriz_compras.empty()
        || !construirMatrizIntersecao(
            &matriz_compras,
            algoritmo,
            &intersecao
        )
        || intersecao.empty()) {
        printf("Falha ao construir a matriz de intersecao.\n");
        return 1;
    }

    std::vector<std::vector<double> > similaridade =
        calcularMatrizSimilaridade(&intersecao, &lista);

    for (int i = 3; i < 6; i++) {
        std::string codigo_original = argv[i];
        int indice_interno;

        if (obterIndiceCliente(
                &lista,
                &codigo_original,
                &indice_interno
            )) {
            std::vector<int> recomendados =
                recomendarProdutos(
                    indice_interno,
                    k,
                    &lista,
                    &similaridade
                );

            printf("=== Recomendacoes para o cliente %s ===\n", codigo_original.c_str());
            
            if (recomendados.empty()) {
                printf("  Nenhum produto para recomendar (cliente nao tem vizinhos).\n");
            } else {
                for (size_t j = 0; j < recomendados.size(); j++) {
                    int id_produto = recomendados[j];
                    printf(
                        "  %lu. %s\n",
                        (unsigned long)j + 1,
                        lista.nomes_produtos[id_produto].c_str()
                    );
                }
            }
        } else {
            printf("Cliente de codigo %s nao encontrado no banco de dados.\n\n", codigo_original.c_str());
        }
    }
    return 0;
}
