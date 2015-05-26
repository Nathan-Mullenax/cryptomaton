all: cryptomaton

cryptomaton: main.cpp
	c++ main.cpp -lncurses -lgmp -lgmpxx -ocryptomaton 
