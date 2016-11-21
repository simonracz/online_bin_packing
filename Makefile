CC=clang
CXX=clang++
RM=rm -f

CXXFLAGS=-O3 -std=c++11 -stdlib=libc++ -Wall

PRGS=generate annotate evaluate

all: $(PRGS)

generate: generate.cpp
	$(CXX) -o generate $(CXXFLAGS) generate.cpp

annotate: annotate.cpp
	$(CXX) -o annotate $(CXXFLAGS) annotate.cpp AutoAnnotator.cpp

evaluate: evaluate.cpp
	$(CXX) -o evaluate $(CXXFLAGS) evaluate.cpp

.PHONY: clean

clean:
	$(RM) $(PRGS)

