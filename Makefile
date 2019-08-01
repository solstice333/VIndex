all: a

a: main.cpp vindex.h
	g++ main.cpp -o $@

.PHONY: clean

clean:
	rm -f *.exe* *.out
