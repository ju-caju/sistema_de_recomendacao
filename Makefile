CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic

testador: lista_compras.cpp testador.cpp lista_compras.h
	$(CXX) $(CXXFLAGS) lista_compras.cpp testador.cpp -o testador

clean:
	rm -f testador
