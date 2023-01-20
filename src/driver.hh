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

#include "operator.hh"

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

// Classe base dell'intera gerarchia di classi che rappresentano
// gli elementi del programma
class RootAST
{
  public:
    virtual ~RootAST(){};
    virtual void visit(){};
    virtual llvm::Value* codegen(driver& drv) { return nullptr; };
};

// Classe che rappresenta la sequenza di statement
class SeqAST : public RootAST
{
  private:
    RootAST* first;
    RootAST* continuation;

  public:
    SeqAST(RootAST* first, RootAST* continuation);
    void visit() override;
    llvm::Value* codegen(driver& drv) override;
};

/// ExprAST - Classe base per tutti i nodi espressione
class ExprAST : public RootAST
{
  protected:
    bool top;

  public:
    virtual ~ExprAST(){};
    void toggle();
    bool gettop();
};

/// NumberExprAST - Classe per la rappresentazione di costanti numeriche
class NumberExprAST : public ExprAST
{
  private:
    double Val;

  public:
    NumberExprAST(double Val);
    void visit() override;
    llvm::Value* codegen(driver& drv) override;
};

/// VariableExprAST - Classe per la rappresentazione di riferimenti a variabili
class VariableExprAST : public ExprAST
{
  private:
    std::string Name;

  public:
    VariableExprAST(std::string& Name);
    const std::string& getName() const;
    void visit() override;
    llvm::Value* codegen(driver& drv) override;
};

/// BinaryExprAST - Classe per la rappresentazione di operatori binary
class BinaryExprAST : public ExprAST
{
  private:
    Operator Op;
    ExprAST* LHS;
    ExprAST* RHS;

  public:
    BinaryExprAST(Operator Op, ExprAST* LHS, ExprAST* RHS);
    void visit() override;
    llvm::Value* codegen(driver& drv) override;
};

class UnaryExprAST : public ExprAST
{
  private:
    Operator op;
    ExprAST* operand;

  public:
    UnaryExprAST(const Operator&, ExprAST*);

    llvm::Value* codegen(driver&) override;
};

/// CallExprAST - Classe per la rappresentazione di chiamate di funzione
class CallExprAST : public ExprAST
{
  private:
    std::string Callee;
    std::vector<ExprAST*> Args; // ASTs per la valutazione degli argomenti

  public:
    CallExprAST(std::string Callee, std::vector<ExprAST*> Args);
    void visit() override;
    llvm::Value* codegen(driver& drv) override;
};

/// PrototypeAST - Classe per la rappresentazione dei prototipi di funzione
/// (nome, numero e nome dei parametri; in questo caso il tipo è implicito
/// perché unico)
class PrototypeAST : public RootAST
{
  private:
    std::string Name;
    std::vector<std::string> Args;
    bool emit;

  public:
    PrototypeAST(std::string Name, std::vector<std::string> Args);
    const std::string& getName() const;
    const std::vector<std::string>& getArgs() const;
    void visit() override;
    llvm::Function* codegen(driver& drv) override;
    void noemit();
    bool emitp();
};

/// FunctionAST - Classe che rappresenta la definizione di una funzione
class FunctionAST : public RootAST
{
  private:
    PrototypeAST* Proto;
    ExprAST* Body;
    bool external;

  public:
    FunctionAST(PrototypeAST* Proto, ExprAST* Body);
    void visit() override;
    llvm::Function* codegen(driver& drv) override;
};

class IfExprNode : public ExprAST
{
  private:
    ExprAST* condition;
    ExprAST* thenExpr;
    ExprAST* elseExpr;

  public:
    IfExprNode(ExprAST* condition, ExprAST* thenExpr, ExprAST* elseExpr);

    void visit() override;

    llvm::Value* codegen(driver& drv) override;
};

class ForExprAST : public ExprAST
{
  private:
    std::string varName;
    ExprAST* start;
    ExprAST* end;
    ExprAST* step;
    ExprAST* body;

  public:
    ForExprAST(const std::string&, ExprAST*, ExprAST*, ExprAST*, ExprAST*);
    llvm::Value* codegen(driver&) override;
};

class WhileExprAST : public ExprAST
{
  private:
    ExprAST* condition;
    ExprAST* body;

  public:
    WhileExprAST(ExprAST*, ExprAST*);
    llvm::Value* codegen(driver&) override;
};

class VarExprAST : public ExprAST
{
  private:
    std::vector<std::pair<std::string, ExprAST*>> varNames;
    ExprAST* body;

  public:
    VarExprAST(std::vector<std::pair<std::string, ExprAST*>>, ExprAST*);
    llvm::Value* codegen(driver&) override;
};

// void InitializeModule();

#endif // ! DRIVER_HH