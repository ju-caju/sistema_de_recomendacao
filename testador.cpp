#include "lista_compras.h"

#include <cstdio>

int main(int quantidade_argumentos, char *argumentos[]) {
    ListaCompras lista;

    if (quantidade_argumentos != 5) {
        printf("Uso: %s arquivo.csv codigo_cliente_1 codigo_cliente_2 codigo_cliente_3\n", argumentos[0]);
        return 1;
    }

    if (!carregarListaCompras(lista, argumentos[1])) {
        printf("Nao foi possivel abrir o arquivo %s.\n", argumentos[1]);
        return 1;
    }

    printf("Clientes: %d\nProdutos: %d\n\n", (int)lista.codigos_clientes.size(), (int)lista.nomes_produtos.size());
    exibirComprasCliente(lista, argumentos[2]);
    printf("\n");
    exibirComprasCliente(lista, argumentos[3]);
    printf("\n");
    exibirComprasCliente(lista, argumentos[4]);
    return 0;
}
