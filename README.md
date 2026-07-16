# Sistema de Recomendacao

Atividade 1 implementada: modulo `ListaCompras` e programa testador.

## Compilacao

```bash
make
```

## Execucao

```bash
./testador dados/dados_venda_cluster_1.csv 99DIQV01 67903101 9OBKC801
```

O primeiro argumento e o arquivo CSV. Os tres argumentos seguintes sao os codigos originais dos clientes que terao seus produtos exibidos.

O modulo aceita arquivos CSV separados por virgula ou ponto e virgula e preserva codigos como texto, incluindo zeros iniciais.
