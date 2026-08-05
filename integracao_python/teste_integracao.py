import os
import leitura_compras
import sistema_recomendacao

diretorio_atual = os.path.dirname(os.path.abspath(__file__))
caminho_dados = os.path.join(diretorio_atual, "dados", "teste_regressao.csv")

dados = leitura_compras.ler_arquivo(caminho_dados)
compras = dados["lista_compras"]
quantidade_produtos = len(dados["vetor_produtos"])

assert len(dados["vetor_clientes"]) == 3
assert len(dados["vetor_produtos"]) == 4
assert len(compras[0]) == 2
assert dados["nomes_produtos"][0] == "Produto, Um"

caminho_ponto_virgula = os.path.join(diretorio_atual, "dados", "teste_regressao_ponto_virgula.csv")
dados_ponto_virgula = leitura_compras.ler_arquivo(caminho_ponto_virgula)
assert len(dados_ponto_virgula["vetor_clientes"]) == 2
assert len(dados_ponto_virgula["vetor_produtos"]) == 2
assert len(dados_ponto_virgula["lista_compras"][1]) == 1

padrao = sistema_recomendacao.calcular_similaridade(
    compras, quantidade_produtos, 0
)
adaptado = sistema_recomendacao.calcular_similaridade(
    compras, quantidade_produtos, 1
)
csr = sistema_recomendacao.calcular_similaridade(compras, quantidade_produtos, 2)

assert padrao == adaptado
assert padrao == csr
assert abs(padrao[0][1] - 0.5) < 0.000000001
assert abs(padrao[1][0] - (2.0 / 3.0)) < 0.000000001
assert padrao[0][1] != padrao[1][0]

compras_com_vazio = compras + [[]]
densa_com_vazio = sistema_recomendacao.calcular_similaridade(
    compras_com_vazio, quantidade_produtos, 1
)
csr_com_vazio = sistema_recomendacao.calcular_similaridade(
    compras_com_vazio, quantidade_produtos, 2
)
assert densa_com_vazio == csr_com_vazio
assert densa_com_vazio[3][3] == 0.0
assert densa_com_vazio[3][0] == 1.0

recomendacao_densa = sistema_recomendacao.recomendar_produtos(
    compras, quantidade_produtos, 0, 4, 1
)
recomendacao_csr = sistema_recomendacao.recomendar_produtos(
    compras, quantidade_produtos, 0, 4, 2
)

assert recomendacao_densa == recomendacao_csr
assert recomendacao_densa == [2, 3, 0, 1]
print("Todos os testes da integracao Python passaram.")