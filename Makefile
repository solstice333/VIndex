all: a.exe

a.exe: main.cpp vindex.h
	g++ main.cpp

.PHONY: clean
	
clean:
	rm -f *.exe* *.out
