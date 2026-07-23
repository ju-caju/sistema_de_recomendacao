#include "similaridade.h"

std::vector<std::vector<int> > construirMatrizCompras(const ListaCompras &lista) {
    return construirMatrizComprasLimitada(lista, (int)lista.codigos_clientes.size());
}

std::vector<std::vector<int> > construirMatrizComprasLimitada(
    const ListaCompras &lista,
    int limite_clientes
) {
    int total_clientes = (int)lista.codigos_clientes.size();
    int quantidade_clientes;
    int quantidade_produtos = (int)lista.nomes_produtos.size();

    if (limite_clientes <= 0) {
        return std::vector<std::vector<int> >();
    }

    quantidade_clientes = limite_clientes;
    if (quantidade_clientes > total_clientes) {
        quantidade_clientes = total_clientes;
    }

    std::vector<std::vector<int> > matriz(
        quantidade_clientes,
        std::vector<int>(quantidade_produtos, 0)
    );

    for (int i = 0; i < quantidade_clientes; i++) {
        for (std::list<int>::const_iterator it = lista.compras_clientes[i].begin(); 
             it != lista.compras_clientes[i].end(); ++it) {
            matriz[i][*it] = 1;
        }
    }

    return matriz;
}

std::vector<std::vector<int> > transporMatriz(
    const std::vector<std::vector<int> > &matriz
) {
    if (matriz.empty()) {
        return std::vector<std::vector<int> >();
    }

    int quantidade_linhas = (int)matriz.size();
    int quantidade_colunas = (int)matriz[0].size();
    std::vector<std::vector<int> > transposta(
        quantidade_colunas,
        std::vector<int>(quantidade_linhas, 0)
    );

    for (int i = 0; i < quantidade_linhas; i++) {
        for (int j = 0; j < quantidade_colunas; j++) {
            transposta[j][i] = matriz[i][j];
        }
    }

    return transposta;
}

std::vector<std::vector<int> > multiplicarMatrizes(
    const std::vector<std::vector<int> > &matriz_a,
    const std::vector<std::vector<int> > &matriz_b
) {
    if (matriz_a.empty() || matriz_b.empty()) {
        return std::vector<std::vector<int> >();
    }

    int linhas_a = (int)matriz_a.size();
    int colunas_a = (int)matriz_a[0].size();
    int linhas_b = (int)matriz_b.size();
    int colunas_b = (int)matriz_b[0].size();

    if (colunas_a != linhas_b) {
        return std::vector<std::vector<int> >();
    }

    std::vector<std::vector<int> > resultado(
        linhas_a,
        std::vector<int>(colunas_b, 0)
    );

    for (int i = 0; i < linhas_a; i++) {
        for (int j = 0; j < colunas_b; j++) {
            for (int k = 0; k < colunas_a; k++) {
                resultado[i][j] += matriz_a[i][k] * matriz_b[k][j];
            }
        }
    }

    return resultado;
}

std::vector<std::vector<int> > multiplicarPorTranspostaAdaptado(
    const std::vector<std::vector<int> > &matriz
) {
    if (matriz.empty()) {
        return std::vector<std::vector<int> >();
    }

    int quantidade_linhas = (int)matriz.size();
    int quantidade_colunas = (int)matriz[0].size();
    std::vector<std::vector<int> > resultado(
        quantidade_linhas,
        std::vector<int>(quantidade_linhas, 0)
    );

    for (int i = 0; i < quantidade_linhas; i++) {
        for (int j = i; j < quantidade_linhas; j++) {
            int soma = 0;

            for (int produto = 0; produto < quantidade_colunas; produto++) {
                soma += matriz[i][produto] * matriz[j][produto];
            }

            resultado[i][j] = soma;
            resultado[j][i] = soma;
        }
    }

    return resultado;
}

std::vector<std::vector<double> > calcularMatrizSimilaridade(const std::vector<std::vector<int> > &intersecao, const ListaCompras &lista) {
    int n_clientes = intersecao.size();
    std::vector<std::vector<double> > S(n_clientes, std::vector<double>(n_clientes, 0.0));
    
    for (int i = 0; i < n_clientes; ++i) {
        double tamanho_Pi = lista.compras_clientes[i].size(); 
        
        for (int j = 0; j < n_clientes; ++j) {
            if (tamanho_Pi > 0) {
                S[i][j] = 1.0 - ((double)intersecao[i][j] / tamanho_Pi);
            } else {
            }
        }
    }
    return S;
}

int encontrarSimilar(int cliente_idx, const std::vector<std::vector<double> > &S) {
    int n_clientes = S.size();
    double menor_distancia = 2.0;
    int vizinho_mais_similar = -1;
    
    for (int j = 0; j < n_clientes; ++j) {
        if (j == cliente_idx) continue; 
        
        if (S[cliente_idx][j] < menor_distancia) {
            menor_distancia = S[cliente_idx][j];
            vizinho_mais_similar = j;
        }
    }
    return vizinho_mais_similar;
}
