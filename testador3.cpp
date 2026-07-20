#include "lista_compras.h"
#include "similaridade.h"
#include "recomendacao.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char *argv[]) {
    if (argc != 6) {
        printf("Uso: %s arquivo.csv quantidade_k codigo_cliente_1 codigo_cliente_2 codigo_cliente_3\n", argv[0]);
        return 1;
    }

    ListaCompras lista;
    if (!carregarListaCompras(lista, argv[1])) {
        printf("Nao foi possivel abrir o arquivo %s.\n", argv[1]);
        return 1;
    }

    int k = std::atoi(argv[2]);

    std::vector<std::vector<int> > A = construirMatrizCompras(lista);
    std::vector<std::vector<int> > AT = transporMatriz(A);
    std::vector<std::vector<int> > Intersecao = multiplicarMatrizes(A, AT);
    std::vector<std::vector<double> > S = calcularMatrizSimilaridade(Intersecao, lista);

    for (int i = 3; i < 6; ++i) {
        std::string codigo_original = argv[i];
        int indice_interno;

        if (obterIndiceCliente(lista, codigo_original, indice_interno)) {
            
            std::vector<int> recomendados = recomendarProdutos(indice_interno, k, lista, S);

            printf("=== Recomendacoes para o cliente %s ===\n", codigo_original.c_str());
            
            if (recomendados.empty()) {
                printf("  Nenhum produto para recomendar (cliente nao tem vizinhos).\n");
            } else {
                for (size_t j = 0; j < recomendados.size(); ++j) {
                    int id_produto = recomendados[j];
                    printf("  %lu. %s\n", j + 1, lista.nomes_produtos[id_produto].c_str());
                }
            }
        } else {
            printf("Cliente de codigo %s nao encontrado no banco de dados.\n\n", codigo_original.c_str());
        }
    }
    return 0;
}