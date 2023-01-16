CXX = g++

.PHONY: clean all

all: kfe

kfe: driver.o parser.o scanner.o kfe.o
	$(CXX) -o kfe driver.o parser.o scanner.o kfe.o `llvm-config --cxxflags --ldflags --libs --libfiles --system-libs`

kfe.o:  kfe.cc driver.hh
	$(CXX) -c kfe.cc -I/usr/lib/llvm-14/include -std=c++17 -fno-exceptions -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -L/usr/lib/llvm-14/lib -lLLVM-14
	
parser.o: parser.cc
	$(CXX) -c parser.cc -I/usr/lib/llvm-14/include -std=c++17 -fno-exceptions -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
	
scanner.o: scanner.cc parser.hh
	$(CXX) -c scanner.cc -I/usr/lib/llvm-14/include -std=c++17 -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS 
	
driver.o: driver.cc parser.hh driver.hh
	$(CXX) -c driver.cc -I/usr/lib/llvm-14/include -std=c++17 -fno-exceptions -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS 

parser.cc, parser.hh: parser.yy 
	bison -o parser.cc parser.yy

scanner.cc: scanner.ll
	flex -o scanner.cc scanner.ll

clean:
	rm -f *~ driver.o scanner.o parser.o kfe.o kfe scanner.cc parser.cc parser.hh
