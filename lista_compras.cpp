#include "lista_compras.h"

#include <algorithm>
#include <cstring>
#include <cstdio>

static std::string removerEspacos(const std::string &texto) {
    size_t inicio = texto.find_first_not_of(" \t\r\n");
    size_t fim = texto.find_last_not_of(" \t\r\n");

    if (inicio == std::string::npos) {
        return "";
    }

    return texto.substr(inicio, fim - inicio + 1);
}

static std::vector<std::string> separarCampos(const std::string &linha, char separador) {
    std::vector<std::string> campos;
    std::string campo;
    bool entre_aspas = false;

    for (size_t indice = 0; indice < linha.size(); indice++) {
        char caractere = linha[indice];

        if (caractere == '"') {
            if (entre_aspas && indice + 1 < linha.size() && linha[indice + 1] == '"') {
                campo += '"';
                indice++;
            } else {
                entre_aspas = !entre_aspas;
            }
        } else if (caractere == separador && !entre_aspas) {
            campos.push_back(removerEspacos(campo));
            campo.clear();
        } else {
            campo += caractere;
        }
    }

    campos.push_back(removerEspacos(campo));
    return campos;
}

static int obterOuCriarCliente(ListaCompras &lista, const std::string &codigo_cliente) {
    std::map<std::string, int>::iterator encontrado = lista.indices_clientes.find(codigo_cliente);

    if (encontrado != lista.indices_clientes.end()) {
        return encontrado->second;
    }

    int indice_cliente = (int)lista.codigos_clientes.size();
    lista.codigos_clientes.push_back(codigo_cliente);
    lista.indices_clientes[codigo_cliente] = indice_cliente;
    lista.compras_clientes.push_back(std::list<int>());
    return indice_cliente;
}

static int obterOuCriarProduto(ListaCompras &lista, const std::string &codigo_produto, const std::string &nome_produto) {
    std::map<std::string, int>::iterator encontrado = lista.indices_produtos.find(codigo_produto);

    if (encontrado != lista.indices_produtos.end()) {
        return encontrado->second;
    }

    int indice_produto = (int)lista.nomes_produtos.size();
    lista.nomes_produtos.push_back(nome_produto);
    lista.indices_produtos[codigo_produto] = indice_produto;
    return indice_produto;
}

bool carregarListaCompras(ListaCompras &lista, const char *caminho_arquivo) {
    FILE *arquivo = fopen(caminho_arquivo, "r");
    char linha[4096];

    if (arquivo == NULL) {
        return false;
    }

    if (fgets(linha, sizeof(linha), arquivo) == NULL) {
        fclose(arquivo);
        return false;
    }

    char separador = strchr(linha, ';') != NULL ? ';' : ',';

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        std::vector<std::string> campos = separarCampos(linha, separador);

        if (campos.size() < 4) {
            continue;
        }

        int indice_cliente = obterOuCriarCliente(lista, campos[1]);
        int indice_produto = obterOuCriarProduto(lista, campos[2], campos[3]);
        std::list<int> &compras = lista.compras_clientes[indice_cliente];

        if (std::find(compras.begin(), compras.end(), indice_produto) == compras.end()) {
            compras.push_back(indice_produto);
        }
    }

    fclose(arquivo);
    return true;
}

bool obterIndiceCliente(const ListaCompras &lista, const std::string &codigo_cliente, int &indice_cliente) {
    std::map<std::string, int>::const_iterator encontrado = lista.indices_clientes.find(codigo_cliente);

    if (encontrado == lista.indices_clientes.end()) {
        return false;
    }

    indice_cliente = encontrado->second;
    return true;
}

void exibirComprasCliente(const ListaCompras &lista, const std::string &codigo_cliente) {
    int indice_cliente;

    if (!obterIndiceCliente(lista, codigo_cliente, indice_cliente)) {
        printf("Cliente %s nao encontrado.\n", codigo_cliente.c_str());
        return;
    }

    printf("Cliente %s:\n", codigo_cliente.c_str());

    for (std::list<int>::const_iterator produto = lista.compras_clientes[indice_cliente].begin();
         produto != lista.compras_clientes[indice_cliente].end();
         ++produto) {
        printf("- %s\n", lista.nomes_produtos[*produto].c_str());
    }
}
