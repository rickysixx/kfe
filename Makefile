CXX = g++
CXXFLAGS = -g -Wall -Werror -pedantic-errors -std=c++17
SRCDIR = src
OBJDIR = obj
BINDIR = bin

.PHONY: clean all

all: $(BINDIR)/kfe

$(BINDIR)/kfe: $(OBJDIR)/driver.o $(OBJDIR)/parser.o $(OBJDIR)/scanner.o $(OBJDIR)/kfe.o $(OBJDIR)/operator.o
	$(CXX) -o $@ $(CXXFLAGS) `llvm-config --cxxflags --ldflags --libs --libfiles --system-libs` $^

$(OBJDIR)/kfe.o: $(SRCDIR)/kfe.cc $(SRCDIR)/driver.hh
	$(CXX) -c $(SRCDIR)/kfe.cc -o $@ $(CXXFLAGS) -I/usr/lib/llvm-14/include -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -L/usr/lib/llvm-14/lib -lLLVM-14
	
$(OBJDIR)/parser.o: $(SRCDIR)/parser.cc
	$(CXX) -c $^ $(CXXFLAGS) -o $@ -I/usr/lib/llvm-14/include -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
	
$(OBJDIR)/scanner.o: $(SRCDIR)/scanner.cc $(SRCDIR)/parser.hh
	$(CXX) -c $(SRCDIR)/scanner.cc -o $@ $(CXXFLAGS) -I/usr/lib/llvm-14/include -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS 
	
$(OBJDIR)/driver.o: $(SRCDIR)/driver.cc $(SRCDIR)/parser.hh $(SRCDIR)/driver.hh $(SRCDIR)/operator.cc
	$(CXX) -c $(SRCDIR)/driver.cc -o $@ $(CXXFLAGS) -I/usr/lib/llvm-14/include -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS 

$(OBJDIR)/operator.o: $(SRCDIR)/operator.hh $(SRCDIR)/operator.cc
	$(CXX) -c $(SRCDIR)/operator.cc -o $@ $(CXXFLAGS)

$(SRCDIR)/parser.cc, $(SRCDIR)/parser.hh: $(SRCDIR)/parser.yy
	bison -o $(SRCDIR)/parser.cc -Wall -Werror $^

$(SRCDIR)/scanner.cc: $(SRCDIR)/scanner.ll
	flex -o $@ $^

clean:
	rm -f $(OBJDIR)/* $(BINDIR)/* $(SRCDIR)/parser.cc $(SRCDIR)/parser.hh $(SRCDIR)/scanner.cc