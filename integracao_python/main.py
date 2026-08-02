import sys

import leitura_compras
import sistema_recomendacao


def _mostrar_uso(programa):
    print(f"Uso: python {programa} <arquivo.csv> <entrega> <algoritmo> <k>")
    print("ENTREGA: 1, 2, 3, 4 ou 5")
    print("ALGORITMO: 0 = padrao | 1 = adaptado | 2 = CSR")


def _indices_teste(quantidade):
    if quantidade <= 0:
        return []
    if quantidade == 1:
        return [0]
    if quantidade <= 4:
        return [0, quantidade - 1]
    return [1, 4]


def _clientes_recomendacao(dados):
    padrao = ["99DIQV01", "99KQAA01", "99FT8Z01"]
    codigos = []

    for codigo in padrao:
        if codigo in dados["mapa_clientes"]:
            codigos.append(codigo)

    indice = 0
    while len(codigos) < 3 and indice < len(dados["vetor_clientes"]):
        codigo = dados["vetor_clientes"][indice]
        if codigo not in codigos:
            codigos.append(codigo)
        indice += 1

    return codigos


def executar(dados, entrega, algoritmo, k):
    compras = dados["lista_compras"]
    quantidade_produtos = len(dados["vetor_produtos"])

    if entrega == 1:
        leitura_compras.testador_exibe_produtos(dados)
        return

    if entrega == 2:
        clientes = _indices_teste(len(compras))
        similares = sistema_recomendacao.encontrar_similares(
            compras, quantidade_produtos, clientes, algoritmo
        )
        for posicao in range(len(clientes)):
            cliente = clientes[posicao]
            similar = int(similares[posicao][0])
            if similar < 0:
                print(f"Cliente {cliente}: nenhum cliente similar encontrado.")
            else:
                print(
                    f"Cliente {cliente}: mais similar = {similar}; "
                    f"distancia = {similares[posicao][1]:.4f}"
                )
        return

    if entrega == 3:
        codigos = _clientes_recomendacao(dados)
        clientes = [dados["mapa_clientes"][codigo] for codigo in codigos]
        recomendacoes = sistema_recomendacao.recomendar_clientes(
            compras, quantidade_produtos, clientes, k, algoritmo
        )
        for posicao in range(len(codigos)):
            codigo = codigos[posicao]
            recomendados = recomendacoes[posicao]
            print(f"\nOs {len(recomendados)} produtos recomendados para {codigo}:")
            for produto in recomendados:
                print(f'- {dados["nomes_produtos"][produto]}')
        return

    if entrega == 4:
        tempos = sistema_recomendacao.comparar_tempos(compras, quantidade_produtos)
        if len(tempos) == 2:
            print(f"Algoritmo padrao: {tempos[0]:.6f} segundos")
            print(f"Algoritmo adaptado: {tempos[1]:.6f} segundos")
        return

    tempos = sistema_recomendacao.comparar_tempos(compras, quantidade_produtos)
    medidas = sistema_recomendacao.comparar_memoria_csr(
        compras, quantidade_produtos
    )
    if len(tempos) == 2:
        print(f"Algoritmo padrao: {tempos[0]:.6f} segundos")
        print(f"Algoritmo adaptado: {tempos[1]:.6f} segundos")
    if len(medidas) == 3:
        print(f"Tempo CSR: {medidas[0]:.6f} segundos")
        print(f"Memoria densa estimada: {int(medidas[1])} bytes")
        print(f"Memoria CSR estimada: {int(medidas[2])} bytes")
        if medidas[2] > 0:
            print(f"Economia estimada: {medidas[1] / medidas[2]:.2f}x")


def main():
    if len(sys.argv) != 5:
        _mostrar_uso(sys.argv[0])
        return 1

    try:
        entrega = int(sys.argv[2])
        algoritmo = int(sys.argv[3])
        k = int(sys.argv[4])
    except ValueError:
        _mostrar_uso(sys.argv[0])
        return 1

    if entrega < 1 or entrega > 5:
        print("Entrega invalida. Use 1, 2, 3, 4 ou 5.")
        return 1
    if algoritmo < 0 or algoritmo > 2:
        print("Algoritmo invalido. Use 0, 1 ou 2.")
        return 1
    if entrega == 3 and k <= 0:
        print("A quantidade k deve ser maior que zero.")
        return 1

    try:
        dados = leitura_compras.ler_arquivo(sys.argv[1])
    except (OSError, ValueError) as erro:
        print(erro)
        return 1

    if not dados["vetor_clientes"] or not dados["vetor_produtos"]:
        print("O arquivo CSV nao possui compras validas.")
        return 1

    executar(dados, entrega, algoritmo, k)
    return 0


if __name__ == "__main__":
    sys.exit(main())
