#ifndef AST_NODE_HH
#define AST_NODE_HH

#include "operator.hh"
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>

class driver;

// Classe base dell'intera gerarchia di classi che rappresentano
// gli elementi del programma
class RootAST
{
  public:
    virtual ~RootAST() = default;
    virtual void visit(){};
    virtual llvm::Value* codegen(driver&) = 0; // pure virtual function, subclasses are forced to provide an implementation
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

    double getVal() const;

    void visit() override;
    llvm::Value* codegen(driver& drv) override;
};

/// VariableExprAST - Classe per la rappresentazione di riferimenti a variabili
class VariableExprAST : public ExprAST
{
  private:
    std::string varName;

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
    ExprAST* conditionExpr;
    ExprAST* thenExpr;
    ExprAST* elseExpr;

  public:
    IfExprNode(ExprAST*, ExprAST*, ExprAST*);

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

class ArrayInitExprAST : public ExprAST
{
  private:
    std::string name;
    unsigned int capacity;

  public:
    const std::string& getName() const;

    ArrayInitExprAST(const std::string&, unsigned int);
    llvm::AllocaInst* codegen(driver&) override;
};

class ArrayIndexingExprAST : public ExprAST
{
  private:
    std::string name;
    ExprAST* indexExpr;

  public:
    ArrayIndexingExprAST(const std::string&, ExprAST* indexExpr);
    llvm::Value* codegen(driver&) override;
};

#endif