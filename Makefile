CXX = g++
CXXFLAGS = -g -Wall -Wextra -Werror -pedantic-errors -std=c++17
LLVM_LDFLAGS = $(shell llvm-config --ldflags)
LLVM_LIBS = $(shell llvm-config --libs)
SRCDIR = src
OBJDIR = obj
BINDIR = bin

.PHONY: clean all

all: makedirs $(BINDIR)/kfe

$(BINDIR)/kfe: $(OBJDIR)/driver.o $(OBJDIR)/parser.o $(OBJDIR)/scanner.o $(OBJDIR)/kfe.o $(OBJDIR)/operator.o $(OBJDIR)/ast_node.o
	$(CXX) -o $@ $(LLVM_LDFLAGS) $(LLVM_LIBS) $^

$(OBJDIR)/kfe.o: $(SRCDIR)/kfe.cc $(SRCDIR)/driver.hh
	$(CXX) -c $(SRCDIR)/kfe.cc -o $@ $(CXXFLAGS)

$(OBJDIR)/parser.o: $(SRCDIR)/parser.cc
	$(CXX) -c $^ $(CXXFLAGS) -o $@
	
$(OBJDIR)/scanner.o: $(SRCDIR)/scanner.cc $(SRCDIR)/parser.hh
	$(CXX) -c $(SRCDIR)/scanner.cc -o $@ $(CXXFLAGS)
	
$(OBJDIR)/driver.o: $(SRCDIR)/driver.cc $(SRCDIR)/parser.hh $(SRCDIR)/driver.hh $(SRCDIR)/operator.cc
	$(CXX) -c $(SRCDIR)/driver.cc -o $@ $(CXXFLAGS)

$(OBJDIR)/ast_node.o: $(SRCDIR)/ast_node.cc
	$(CXX) -c $^ -o $@ $(CXXFLAGS)

$(OBJDIR)/operator.o: $(SRCDIR)/operator.hh $(SRCDIR)/operator.cc
	$(CXX) -c $(SRCDIR)/operator.cc -o $@ $(CXXFLAGS)

$(SRCDIR)/parser.cc, $(SRCDIR)/parser.hh: $(SRCDIR)/parser.yy
	bison -o $(SRCDIR)/parser.cc -Wall -Werror -Wcounterexamples $^

$(SRCDIR)/scanner.cc: $(SRCDIR)/scanner.ll
	flex -o $@ $^

makedirs:
	mkdir -p $(OBJDIR) $(BINDIR)

clean:
	rm -f $(OBJDIR)/* $(BINDIR)/* $(SRCDIR)/parser.cc $(SRCDIR)/parser.hh $(SRCDIR)/scanner.cc