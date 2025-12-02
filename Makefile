all: compile run

compile:
	g++ solve.cpp -o solve.o

run: compile
	cat $(filePath)
	./solve.o $(filePath)

debug:
	g++ -g solve.cpp -o solve.o
	gdb -tui --args solve.o $(filePath)
