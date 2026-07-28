# Sistema de Recomendacao

Projeto procedural em C++11 que organiza o historico de compras, calcula a
distancia entre clientes e recomenda produtos. A Atividade 4 acrescenta uma
multiplicacao eficiente da matriz de compras por sua transposta e um benchmark
comparativo com o algoritmo padrao.

O codigo segue o estilo procedural da linguagem C. O compilador C++ e usado
somente porque a especificacao permite as estruturas `vector`, `string`, `map`
e `list`, alem dos algoritmos `find` e `sort` da STL. A implementacao nao usa
classes proprias, referencias, iteradores, lacos baseados em intervalo ou
outros recursos exclusivos de C++ fora dessas excecoes.

## Compilacao

O projeto usa `g++` com as opcoes `-std=c++11 -Wall -Wextra -pedantic`.

```bash
make clean
make all
```

Os executaveis sao criados em `bin/`. Tambem e possivel compilar uma atividade
isoladamente:

```bash
make atividade1
make atividade2
make atividade3
make atividade4
```

## Atividade 1 - Lista de compras

O modulo `ListaCompras` aceita arquivos CSV separados por virgula ou ponto e
virgula, preserva codigos como texto, incluindo zeros iniciais, e remove compras
duplicadas de um mesmo produto pelo mesmo cliente.

```bash
./bin/atividade1 dados/dados_venda_cluster_1.csv 99DIQV01 67903101 9OBKC801
```

Os tres codigos desse exemplo existem no arquivo informado.

## Atividade 2 - Similaridade

O ultimo argumento e opcional e aceita `padrao` ou `adaptado`. Quando omitido,
o algoritmo adaptado e usado.

```bash
./bin/atividade2 dados/dados_venda_cluster_1.csv 0 1 padrao
./bin/atividade2 dados/dados_venda_cluster_1.csv 0 1 adaptado
./bin/atividade2 dados/dados_venda_cluster_1.csv 0 1
```

## Atividade 3 - Recomendacao

O parametro de selecao do algoritmo tambem e opcional no testador da Atividade
3 e deve ser colocado depois dos tres codigos de cliente.

```bash
./bin/atividade3 dados/dados_venda_cluster_1.csv 3 99DIQV01 67903101 9OBKC801 padrao
./bin/atividade3 dados/dados_venda_cluster_1.csv 3 99DIQV01 67903101 9OBKC801 adaptado
./bin/atividade3 dados/dados_venda_cluster_1.csv 3 99DIQV01 67903101 9OBKC801
```

## Atividade 4 - Multiplicacao eficiente

A matriz binaria de compras `A` possui uma linha por cliente e uma coluna por
produto. O algoritmo padrao constroi `A^T` e usa a multiplicacao geral de
matrizes. O algoritmo adaptado calcula diretamente cada produto interno entre
duas linhas de `A`:

```text
C[i][j] = soma(A[i][produto] * A[j][produto])
```

Como `C = A * A^T` e simetrica, o algoritmo adaptado calcula somente as
posicoes em que `j >= i` e copia cada resultado para `C[j][i]`. A transposta
nao e construida nesse caminho.

A simetria vale para a matriz de intersecao `C`, mas nao para a matriz final de
distancias `S`. Cada linha de `S` usa a quantidade de produtos do cliente da
linha como denominador:

```text
S[i][j] = 1 - C[i][j] / |Pi|
```

Em geral, `|Pi|` e diferente de `|Pj|`; portanto, `S[i][j]` pode ser diferente
de `S[j][i]`. O modulo calcula cada celula de `S` com o seu proprio denominador
e nao espelha essa matriz.

As constantes `ALGORITMO_PADRAO` e `ALGORITMO_ADAPTADO` selecionam o caminho
usado por `construirMatrizIntersecao()`. A funcao
`construirMatrizComprasLimitada()` cria a matriz densa somente para os
primeiros N clientes, mantendo todas as colunas de produtos.

### Benchmark

O arquivo `dados_vendas.csv` possui 321.741 registros, 236.244 clientes e 715
produtos. Ele e carregado uma unica vez, mas a matriz densa e construida
somente para recortes controlados. Nao se deve construir `A` para os 236.244
clientes nem a matriz `C` de dimensao 236.244 por 236.244, pois o consumo de
memoria seria excessivo.

Coloque `dados_vendas.csv` na raiz do projeto ou informe o caminho onde ele
esta armazenado e execute:

```bash
./bin/atividade4 dados_vendas.csv 100 250 500
```

O testador executa tres repeticoes de cada algoritmo com `clock()` e mostra a
media em segundos. A leitura do CSV, a construcao de `A` e a impressao ficam
fora do trecho cronometrado. O tempo apresentado inclui a construcao de `C` e
a conversao para `S`. As intersecoes sao comparadas elemento por elemento e as
matrizes `double` usam tolerancia de `0.000000001`.

O testador tambem executa automaticamente o caso conhecido:

```text
A = 1 0 1       A * A^T = 2 0 1
    0 1 0                   0 1 1
    1 1 0                   1 1 2
```

### Resultados medidos

Resultados reais obtidos em 23/07/2026 com o `Makefile` deste repositorio,
media de tres repeticoes:

| Clientes | Produtos | Padrao (s) | Adaptado (s) | Aceleracao | Intersecao igual | Similaridade igual |
|---------:|---------:|-----------:|-------------:|-----------:|:----------------:|:------------------:|
| 100 | 715 | 0.174884 | 0.061898 | 2.83x | SIM | SIM |
| 250 | 715 | 1.116509 | 0.364323 | 3.06x | SIM | SIM |
| 500 | 715 | 4.878307 | 1.630710 | 2.99x | SIM | SIM |

Esses valores descrevem esta execucao e podem variar conforme a maquina e a
carga do sistema. Em todos os recortes testados, os dois algoritmos produziram
exatamente a mesma matriz de intersecao e matrizes de similaridade iguais
dentro da tolerancia.

## Atividade 5 - Matrizes esparsas CSR

O modulo `csr` representa matrizes esparsas com os tres vetores do formato
Compressed Sparse Row:

- `values`: valores diferentes de zero;
- `col_index`: coluna correspondente a cada valor;
- `row_ptr`: inicio de cada linha em `values`.

As colunas de cada linha sao ordenadas. Assim, o produto interno de duas linhas
usa dois indices e ignora todos os zeros. A funcao
`multiplicarCSRPorTransposta()` constroi `A * A^T` diretamente em CSR, sem
materializar `A^T`.

A matriz densa de distancias tambem nao e criada. `SimilaridadeCSR` armazena a
matriz CSR de intersecoes e a quantidade de compras de cada cliente.
`obterSimilaridadeCSR()` calcula uma celula sob demanda; uma intersecao ausente
representa distancia 1.

Compile e execute o testador:

```bash
make atividade5
./bin/atividade5 dados/dados_venda_cluster_1.csv 100 250 500
```

O testador valida um exemplo conhecido, compara todas as celulas e tambem as
recomendacoes produzidas pelas abordagens densa e CSR, e mostra o tempo e a
memoria estimada. Ele constroi a matriz de compras CSR para toda a base, mas
limita a multiplicacao comparativa aos tamanhos informados para evitar a
alocacao da matriz densa completa.

O codigo continua procedural: usa recursos da linguagem C e apenas `vector`,
`string`, `list`, `map`, `find` e `sort` da STL, conforme permitido pela
atividade.
