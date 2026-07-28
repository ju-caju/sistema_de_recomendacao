#ifndef LISTA_COMPRAS_H
#define LISTA_COMPRAS_H

#include <list>
#include <map>
#include <string>
#include <vector>

typedef struct {
    std::vector<std::string> codigos_clientes;
    std::map<std::string, int> indices_clientes;
    std::vector<std::string> nomes_produtos;
    std::map<std::string, int> indices_produtos;
    std::vector<std::list<int> > compras_clientes;
} ListaCompras;

bool carregarListaCompras(ListaCompras *lista, const char *caminho_arquivo);
bool obterIndiceCliente(
    const ListaCompras *lista,
    const std::string *codigo_cliente,
    int *indice_cliente
);
void exibirComprasCliente(
    const ListaCompras *lista,
    const std::string *codigo_cliente
);

#endif
