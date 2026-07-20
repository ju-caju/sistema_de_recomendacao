#include "lista_compras.h"
#include "similaridade.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("\nUso: %s arquivo.csv indice_cliente_1 indice_cliente_2\n", argv[0]);
        return 1;
    }

    ListaCompras lista;
    if (!carregarListaCompras(lista, argv[1])) {
        printf("\nNao foi possivel abrir o arquivo %s.\n", argv[1]);
        return 1;
    }
    
    std::vector<std::vector<int> > A = construirMatrizCompras(lista);
    std::vector<std::vector<int> > AT = transporMatriz(A);
    std::vector<std::vector<int> > Intersecao = multiplicarMatrizes(A, AT);
    std::vector<std::vector<double> > S = calcularMatrizSimilaridade(Intersecao, lista);

    int idx_1 = std::atoi(argv[2]);
    int idx_2 = std::atoi(argv[3]);

    if (idx_1 >= 0 && (size_t)idx_1 < lista.codigos_clientes.size()) {
        int vizinho_1 = encontrarSimilar(idx_1, S);
        printf("\n\nPara o cliente de indice interno [%d] (Codigo original: %s):\n", idx_1, lista.codigos_clientes[idx_1].c_str());
        printf("-> O cliente mais similar eh o indice [%d] (Codigo original: %s) com distancia Jaccard = %.2f\n\n", 
               vizinho_1, lista.codigos_clientes[vizinho_1].c_str(), S[idx_1][vizinho_1]);
    } else {
        printf("Indice %d invalido!\n", idx_1);
    }

    if (idx_2 >= 0 && (size_t)idx_2 < lista.codigos_clientes.size()) {
        int vizinho_2 = encontrarSimilar(idx_2, S);
        printf("\nPara o cliente de indice interno [%d] (Codigo original: %s):\n", idx_2, lista.codigos_clientes[idx_2].c_str());
        printf(" -> O cliente mais similar eh o indice [%d] (Codigo original: %s) com distancia Jaccard = %.2f\n\n", 
               vizinho_2, lista.codigos_clientes[vizinho_2].c_str(), S[idx_2][vizinho_2]);
    } else {
        printf("Indice %d invalido!\n", idx_2);
    }

    return 0;
}