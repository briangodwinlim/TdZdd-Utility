OPT = -O3 -DB_64 -I. -ISAPPOROBDD/include -ITdZdd/include

all: run

run: run.cpp SAPPOROBDD/lib/BDD64.a WeightedIterator.hpp RandomPruning.hpp MomentsEval.hpp
	g++ run.cpp SAPPOROBDD/lib/BDD64.a -o run $(OPT)

clean:
	rm -f run run.exe *.o
