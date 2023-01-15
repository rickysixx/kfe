.PHONY: clean all

all: kfe

kfe: driver.o parser.o scanner.o kfe.o
	g++ -o kfe driver.o parser.o scanner.o kfe.o `llvm-config --cxxflags --ldflags --libs --libfiles --system-libs`

kfe.o:  kfe.cc driver.hh
	g++ -c kfe.cc -I/usr/lib/llvm-14/include -std=c++17 -fno-exceptions -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -L/usr/lib/llvm-14/lib -lLLVM-14
	
parser.o: parser.cc
	g++ -c parser.cc -I/usr/lib/llvm-14/include -std=c++17 -fno-exceptions -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
	
scanner.o: scanner.cc parser.hh
	g++ -c scanner.cc -I/usr/lib/llvm-14/include -std=c++17 -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS 
	
driver.o: driver.cc parser.hh driver.hh
	g++ -c driver.cc -I/usr/lib/llvm-14/include -std=c++17 -fno-exceptions -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS 

parser.cc, parser.hh: parser.yy 
	bison -o parser.cc parser.yy

scanner.cc: scanner.ll
	flex -o scanner.cc scanner.ll

clean:
	rm -f *~ driver.o scanner.o parser.o kfe.o kfe scanner.cc parser.cc parser.hh
