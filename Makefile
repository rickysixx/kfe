CXX = g++
CXXFLAGS = -g -Wall -Werror -pedantic-errors -std=c++17
SRCDIR = src
OBJDIR = obj
BINDIR = bin

.PHONY: clean all

all: kfe

kfe: driver.o parser.o scanner.o kfe.o operator.o
	$(CXX) -o $@ $(CXXFLAGS) `llvm-config --cxxflags --ldflags --libs --libfiles --system-libs` $^

kfe.o: kfe.cc driver.hh
	$(CXX) -c kfe.cc $(CXXFLAGS) -I/usr/lib/llvm-14/include -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -L/usr/lib/llvm-14/lib -lLLVM-14
	
parser.o: parser.cc
	$(CXX) -c parser.cc $(CXXFLAGS) -I/usr/lib/llvm-14/include -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
	
scanner.o: scanner.cc parser.hh
	$(CXX) -c scanner.cc $(CXXFLAGS) -I/usr/lib/llvm-14/include -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS 
	
driver.o: driver.cc parser.hh driver.hh operator.cc
	$(CXX) -c driver.cc $(CXXFLAGS) -I/usr/lib/llvm-14/include -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS 

operator.o: operator.cc
	$(CXX) -c $^ $(CXXFLAGS)

parser.cc, parser.hh: parser.yy
	bison -o parser.cc -Wempty-rule $^

scanner.cc: scanner.ll
	flex -o $@ $^

clean:
	rm -f *.o kfe scanner.cc parser.cc parser.hh
