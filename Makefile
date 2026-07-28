CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic

BIN_DIR = bin

.PHONY: all atividade1 atividade2 atividade3 atividade4 atividade5 clean

all: atividade1 atividade2 atividade3 atividade4 atividade5

atividade1: $(BIN_DIR)/atividade1

atividade2: $(BIN_DIR)/atividade2

atividade3: $(BIN_DIR)/atividade3

atividade4: $(BIN_DIR)/atividade4

atividade5: $(BIN_DIR)/atividade5

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/atividade1: lista_compras.cpp lista_compras.h testador.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) lista_compras.cpp testador.cpp -o $@

$(BIN_DIR)/atividade2: lista_compras.cpp lista_compras.h similaridade.cpp similaridade.h testador2.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) lista_compras.cpp similaridade.cpp testador2.cpp -o $@

$(BIN_DIR)/atividade3: lista_compras.cpp lista_compras.h similaridade.cpp similaridade.h recomendacao.cpp recomendacao.h testador3.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) lista_compras.cpp similaridade.cpp recomendacao.cpp testador3.cpp -o $@

$(BIN_DIR)/atividade4: lista_compras.cpp lista_compras.h similaridade.cpp similaridade.h testador4.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) lista_compras.cpp similaridade.cpp testador4.cpp -o $@

$(BIN_DIR)/atividade5: lista_compras.cpp lista_compras.h similaridade.cpp similaridade.h csr.cpp csr.h recomendacao.cpp recomendacao_csr.cpp recomendacao.h testador5.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) lista_compras.cpp similaridade.cpp csr.cpp recomendacao.cpp recomendacao_csr.cpp testador5.cpp -o $@

clean:
	rm -f $(BIN_DIR)/atividade1 $(BIN_DIR)/atividade2 $(BIN_DIR)/atividade3 $(BIN_DIR)/atividade4 $(BIN_DIR)/atividade5
