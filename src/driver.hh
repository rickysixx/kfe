#ifndef DRIVER_HH
#define DRIVER_HH
/************ Header file per la generazione del codice oggetto *************/
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
/***************************************************************************/
#include "parser.hh"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <llvm/IR/Instructions.h>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "ast_node.hh"

// Dichiarazione del prototipo yylex per Flex
// Flex va proprio a cercare YY_DECL perché
// deve espanderla (usando M4) nel punto appropriato
#define YY_DECL yy::parser::symbol_type yylex(driver& drv)
// Per il parser è sufficiente una forward declaration
YY_DECL;

// Classe che organizza e gestisce il processo di compilazione
class driver
{
  public:
    driver();
    llvm::LLVMContext* context;
    llvm::Module* module;
    llvm::IRBuilder<>* builder;
    std::map<std::string, llvm::AllocaInst*> NamedValues;
    static inline int Cnt = 0; // Contatore incrementale, per identificare registri SSA
    RootAST* root;             // A fine parsing "punta" alla radice dell'AST
    int parse(const std::string& f);
    std::string file;
    bool trace_parsing;    // Abilita le tracce di debug el parser
    void scan_begin();     // Implementata nello scanner
    void scan_end();       // Implementata nello scanner
    bool trace_scanning;   // Abilita le tracce di debug nello scanner
    yy::location location; // Utillizata dallo scannar per localizzare i token
    bool ast_print;
    void codegen();
};

// void InitializeModule();

#endif // ! DRIVER_HH
