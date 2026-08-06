#include "lista_compras.h"

#include <stdio.h>

static std::string removerEspacos(const std::string *texto) {
    if (texto->empty()) {
        return "";
    }

    size_t inicio = 0;
    
    while (inicio < texto->size() &&
          ((*texto)[inicio] == ' ' || (*texto)[inicio] == '\t' ||
           (*texto)[inicio] == '\r' || (*texto)[inicio] == '\n')) {
        inicio++;
    }

    if (inicio == texto->size()) {
        return "";
    }

    size_t fim = texto->size() - 1;

    while (fim > inicio &&
          ((*texto)[fim] == ' ' || (*texto)[fim] == '\t' ||
           (*texto)[fim] == '\r' || (*texto)[fim] == '\n')) {
        fim--;
    }

    return texto->substr(inicio, fim - inicio + 1);
}

static std::vector<std::string> separarCampos(
    const std::string *linha,
    char separador
) {
    std::vector<std::string> campos;
    std::string campo;
    bool entre_aspas = false;

    for (size_t indice = 0; indice < linha->size(); indice++) {
        char caractere = (*linha)[indice];

        if (caractere == '"') {
            if (entre_aspas && indice + 1 < linha->size()
                && (*linha)[indice + 1] == '"') {
                campo += '"';
                indice++;
            } else {
                entre_aspas = !entre_aspas;
            }
        } else if (caractere == separador && !entre_aspas) {
            campos.push_back(removerEspacos(&campo));
            campo.clear();
        } else {
            campo += caractere;
        }
    }

    campos.push_back(removerEspacos(&campo));
    return campos;
}

static int obterOuCriarCliente(
    ListaCompras *lista,
    const std::string *codigo_cliente
) {
    if (lista->indices_clientes.count(*codigo_cliente) > 0) {
        return lista->indices_clientes[*codigo_cliente];
    }

    int indice_cliente = (int)lista->codigos_clientes.size();
    lista->codigos_clientes.push_back(*codigo_cliente);
    lista->indices_clientes[*codigo_cliente] = indice_cliente;
    lista->compras_clientes.push_back(std::list<int>());
    return indice_cliente;
}

static int obterOuCriarProduto(
    ListaCompras *lista,
    const std::string *codigo_produto,
    const std::string *nome_produto
) {
    if (lista->indices_produtos.count(*codigo_produto) > 0) {
        return lista->indices_produtos[*codigo_produto];
    }

    int indice_produto = (int)lista->nomes_produtos.size();
    lista->nomes_produtos.push_back(*nome_produto);
    lista->indices_produtos[*codigo_produto] = indice_produto;
    return indice_produto;
}

static void limparListaCompras(ListaCompras *lista) {
    lista->codigos_clientes.clear();
    lista->indices_clientes.clear();
    lista->nomes_produtos.clear();
    lista->indices_produtos.clear();
    lista->compras_clientes.clear();
}

static char detectarSeparador(const char *linha) {
    for (int indice = 0; linha[indice] != '\0'; indice++) {
        if (linha[indice] == ';') {
            return ';';
        }
    }

    return ',';
}

bool carregarListaCompras(ListaCompras *lista, const char *caminho_arquivo) {
    if (lista == NULL || caminho_arquivo == NULL) {
        return false;
    }

    FILE *arquivo = fopen(caminho_arquivo, "r");
    char linha[4096];

    if (arquivo == NULL) {
        return false;
    }

    if (fgets(linha, sizeof(linha), arquivo) == NULL) {
        fclose(arquivo);
        return false;
    }

    limparListaCompras(lista);
    char separador = detectarSeparador(linha);

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        std::string texto_linha = linha;
        std::vector<std::string> campos = separarCampos(&texto_linha, separador);

        if (campos.size() < 4) {
            continue;
        }

        obterOuCriarCliente(lista, &campos[1]);
        obterOuCriarProduto(lista, &campos[2], &campos[3]);
    }

    rewind(arquivo);

    if (fgets(linha, sizeof(linha), arquivo) == NULL) {
        fclose(arquivo);
        limparListaCompras(lista);
        return false;
    }

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        std::string texto_linha = linha;
        std::vector<std::string> campos = separarCampos(&texto_linha, separador);

        if (campos.size() < 4 ||
            lista->indices_clientes.count(campos[1]) == 0 ||
            lista->indices_produtos.count(campos[2]) == 0) {
            continue;
        }

        int indice_cliente = lista->indices_clientes.at(campos[1]);
        int indice_produto = lista->indices_produtos.at(campos[2]);
        std::list<int> compras =
            lista->compras_clientes[indice_cliente];
        bool produto_encontrado = false;

        while (!compras.empty()) {
            if (compras.front() == indice_produto) {
                produto_encontrado = true;
                break;
            }
            compras.pop_front();
        }

        if (!produto_encontrado) {
            lista->compras_clientes[indice_cliente].push_back(indice_produto);
        }
    }

    fclose(arquivo);
    return true;
}

bool obterIndiceCliente(
    const ListaCompras *lista,
    const std::string *codigo_cliente,
    int *indice_cliente
) {
    if (lista->indices_clientes.count(*codigo_cliente) == 0) {
        return false;
    }

    *indice_cliente = lista->indices_clientes.at(*codigo_cliente);
    return true;
}

void exibirComprasCliente(
    const ListaCompras *lista,
    const std::string *codigo_cliente
) {
    int indice_cliente;

    if (!obterIndiceCliente(lista, codigo_cliente, &indice_cliente)) {
        printf("Cliente %s nao encontrado.\n", codigo_cliente->c_str());
        return;
    }

    printf("Cliente %s:\n", codigo_cliente->c_str());
    std::list<int> compras = lista->compras_clientes[indice_cliente];

    while (!compras.empty()) {
        int produto = compras.front();
        printf("- %s\n", lista->nomes_produtos[produto].c_str());
        compras.pop_front();
    }
}
