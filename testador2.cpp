#include "lista_compras.h"
#include "similaridade.h"

#include <climits>
#include <cstdlib>
#include <cstring>

static bool converterInteiro(const char *texto, int &valor) {
    char *fim;
    long numero = std::strtol(texto, &fim, 10);

    if (texto[0] == '\0' || fim[0] != '\0' || numero < INT_MIN || numero > INT_MAX) {
        return false;
    }

    valor = (int)numero;
    return true;
}

static bool obterAlgoritmo(const char *nome, int &algoritmo) {
    if (std::strcmp(nome, "padrao") == 0) {
        algoritmo = ALGORITMO_PADRAO;
        return true;
    }

    if (std::strcmp(nome, "adaptado") == 0) {
        algoritmo = ALGORITMO_ADAPTADO;
        return true;
    }

    return false;
}

int main(int argc, char *argv[]) {
    int algoritmo = ALGORITMO_ADAPTADO;
    int idx_1;
    int idx_2;

    if (argc != 4 && argc != 5) {
        printf("Uso: %s arquivo.csv indice_cliente_1 indice_cliente_2 [padrao|adaptado]\n", argv[0]);
        return 1;
    }

    if (!converterInteiro(argv[2], idx_1) || !converterInteiro(argv[3], idx_2)) {
        printf("Os indices dos clientes devem ser numeros inteiros validos.\n");
        return 1;
    }

    if (argc == 5 && !obterAlgoritmo(argv[4], algoritmo)) {
        printf("Algoritmo invalido: %s. Use padrao ou adaptado.\n", argv[4]);
        return 1;
    }

    ListaCompras lista;
    if (!carregarListaCompras(lista, argv[1])) {
        printf("Nao foi possivel abrir o arquivo %s.\n", argv[1]);
        return 1;
    }

    if (lista.codigos_clientes.empty() || lista.nomes_produtos.empty()) {
        printf("Nao ha dados suficientes para construir a matriz de compras.\n");
        return 1;
    }

    if (idx_1 < 0 || (size_t)idx_1 >= lista.codigos_clientes.size()
        || idx_2 < 0 || (size_t)idx_2 >= lista.codigos_clientes.size()) {
        printf("Indice de cliente invalido. Use valores entre 0 e %d.\n",
               (int)lista.codigos_clientes.size() - 1);
        return 1;
    }

    std::vector<std::vector<int> > matriz_compras = construirMatrizCompras(lista);
    std::vector<std::vector<int> > intersecao;

    if (matriz_compras.empty()
        || !construirMatrizIntersecao(matriz_compras, algoritmo, intersecao)
        || intersecao.empty()) {
        printf("Falha ao construir a matriz de intersecao.\n");
        return 1;
    }

    std::vector<std::vector<double> > similaridade =
        calcularMatrizSimilaridade(intersecao, lista);

    int indices[2] = {idx_1, idx_2};

    for (int i = 0; i < 2; i++) {
        int indice = indices[i];
        int vizinho = encontrarSimilar(indice, similaridade);

        printf("\nPara o cliente de indice interno [%d] (Codigo original: %s):\n",
               indice, lista.codigos_clientes[indice].c_str());

        if (vizinho < 0) {
            printf("Nao foi encontrado outro cliente para comparacao.\n");
            continue;
        }

        printf("-> O cliente mais similar eh o indice [%d] (Codigo original: %s)"
               " com distancia Jaccard = %.2f\n",
               vizinho,
               lista.codigos_clientes[vizinho].c_str(),
               similaridade[indice][vizinho]);
    }

    return 0;
}
