CC=clang
CXX=clang++
RM=rm -f

CXXFLAGS=-std=c++11 -stdlib=libc++ -Wall

PRGS=generate annotate

all: $(PRGS)

generate: generate.cpp
	$(CXX) -o generate $(CXXFLAGS) generate.cpp

annotate: annotate.cpp
	$(CXX) -o annotate $(CXXFLAGS) annotate.cpp

.PHONY: clean

clean:
	$(RM) $(PRGS)

