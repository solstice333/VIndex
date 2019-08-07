ifeq '$(CPPSTD)' '11'
	OPT=--std=c++11
endif

all: a

a: main.cpp vindex.h
	g++ $(OPT) main.cpp -o $@

.PHONY: clean

clean:
	rm -f *.exe* *.out a
