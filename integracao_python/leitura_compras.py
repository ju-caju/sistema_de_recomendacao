import csv
from pathlib import Path


def _detectar_separador(caminho):
    with caminho.open("r", encoding="utf-8-sig", newline="") as arquivo:
        cabecalho = arquivo.readline()

    if ";" in cabecalho:
        return ";"
    return ","


def _linhas_csv(caminho, separador):
    with caminho.open("r", encoding="utf-8-sig", newline="") as arquivo:
        leitor = csv.reader(arquivo, delimiter=separador)
        cabecalho = next(leitor, None)
        if cabecalho is None or len(cabecalho) < 4:
            raise ValueError("O CSV deve possuir pelo menos quatro colunas.")

        for campos in leitor:
            if len(campos) >= 4:
                yield [campo.strip() for campo in campos]


def ler_arquivo(caminho_arquivo):
    caminho = Path(caminho_arquivo)
    if not caminho.is_file():
        raise FileNotFoundError(f"Arquivo CSV nao encontrado: {caminho}")

    separador = _detectar_separador(caminho)
    dados = {
        "vetor_clientes": [],
        "mapa_clientes": {},
        "vetor_produtos": [],
        "mapa_produtos": {},
        "nomes_produtos": [],
        "lista_compras": [],
    }

    # Primeira fase: cria os indices internos.
    for campos in _linhas_csv(caminho, separador):
        codigo_cliente = campos[1]
        codigo_produto = campos[2]
        nome_produto = campos[3]

        if codigo_cliente not in dados["mapa_clientes"]:
            indice_cliente = len(dados["vetor_clientes"])
            dados["mapa_clientes"][codigo_cliente] = indice_cliente
            dados["vetor_clientes"].append(codigo_cliente)
            dados["lista_compras"].append([])

        if codigo_produto not in dados["mapa_produtos"]:
            indice_produto = len(dados["vetor_produtos"])
            dados["mapa_produtos"][codigo_produto] = indice_produto
            dados["vetor_produtos"].append(codigo_produto)
            dados["nomes_produtos"].append(nome_produto)

    # Segunda fase: preenche as compras sem duplicatas.
    for campos in _linhas_csv(caminho, separador):
        codigo_cliente = campos[1]
        codigo_produto = campos[2]
        indice_cliente = dados["mapa_clientes"][codigo_cliente]
        indice_produto = dados["mapa_produtos"][codigo_produto]

        if indice_produto not in dados["lista_compras"][indice_cliente]:
            dados["lista_compras"][indice_cliente].append(indice_produto)

    return dados


def mostrar_produtos_cliente(dados, codigo_cliente):
    if codigo_cliente not in dados["mapa_clientes"]:
        print(f"Cliente {codigo_cliente} nao esta registrado.")
        return

    indice_cliente = dados["mapa_clientes"][codigo_cliente]
    print(f"\nProdutos que o cliente {codigo_cliente} comprou:")
    for indice_produto in dados["lista_compras"][indice_cliente]:
        print(f'- {dados["nomes_produtos"][indice_produto]}')


def testador_exibe_produtos(dados):
    codigos_padrao = ["99DIQV01", "99KQAA01", "99FT8Z01"]
    codigos_encontrados = []

    for codigo in codigos_padrao:
        if codigo in dados["mapa_clientes"]:
            codigos_encontrados.append(codigo)

    indice = 0
    while len(codigos_encontrados) < 3 and indice < len(dados["vetor_clientes"]):
        codigo = dados["vetor_clientes"][indice]
        if codigo not in codigos_encontrados:
            codigos_encontrados.append(codigo)
        indice += 1

    for codigo in codigos_encontrados:
        mostrar_produtos_cliente(dados, codigo)
