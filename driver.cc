#include "driver.hh"
#include "operator.hh"
#include "parser.hh"
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Operator.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Use.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>

using llvm::APFloat;
using llvm::BasicBlock;
using llvm::ConstantFP;
using llvm::errs;
using llvm::Function;
using llvm::FunctionType;
using llvm::PHINode;
using llvm::Type;
using llvm::Value;

Value* LogErrorV(const std::string Str)
{
    std::cerr << Str << std::endl;
    return nullptr;
}

/*************************** Driver class *************************/
driver::driver() :
    trace_parsing(false), trace_scanning(false), ast_print(false)
{
    context = new llvm::LLVMContext;
    module = new llvm::Module("Kaleidoscope", *context);
    builder = new llvm::IRBuilder<>(*context);
};

int driver::parse(const std::string& f)
{
    file = f;
    location.initialize(&file);
    scan_begin();
    yy::parser parser(*this);
    parser.set_debug_level(trace_parsing);
    int res = parser.parse();
    scan_end();
    return res;
}

void driver::codegen()
{
    if (ast_print)
        root->visit();
    std::cout << std::endl;
    root->codegen(*this);
};

/********************** Handle Top Expressions ********************/
llvm::Value* TopExpression(ExprAST* E, driver& drv)
{
    // Crea una funzione anonima anonima il cui body è un'espressione top-level
    // viene "racchiusa" un'espressione top-level
    E->toggle(); // Evita la doppia emissione del prototipo
    PrototypeAST* Proto = new PrototypeAST(
        "__espr_anonima" + std::to_string(++drv.Cnt), std::vector<std::string>());
    Proto->noemit();
    FunctionAST* F = new FunctionAST(std::move(Proto), E);
    auto* FnIR = F->codegen(drv);
    FnIR->eraseFromParent();
    return nullptr;
};

/************************ Expression tree *************************/
// Inverte il flag che definisce le TopLevelExpression
// ando viene chiamata
void ExprAST::toggle() { top = top ? false : true; };

bool ExprAST::gettop() { return top; };

/************************* Sequence tree **************************/
SeqAST::SeqAST(RootAST* first, RootAST* continuation) :
    first(first), continuation(continuation){};

void SeqAST::visit()
{
    if (first != nullptr) {
        first->visit();
    } else {
        if (continuation == nullptr) {
            return;
        };
    };
    std::cout << ";";
    continuation->visit();
};

llvm::Value* SeqAST::codegen(driver& drv)
{
    if (first != nullptr) {
        llvm::Value* f = first->codegen(drv);
    } else {
        if (continuation == nullptr)
            return nullptr;
    }
    llvm::Value* c = continuation->codegen(drv);
    return nullptr;
};

/********************* Number Expression Tree *********************/
NumberExprAST::NumberExprAST(double Val) :
    Val(Val) { top = false; };
void NumberExprAST::visit() { std::cout << Val << " "; };

llvm::Value* NumberExprAST::codegen(driver& drv)
{
    if (gettop())
        return TopExpression(this, drv);
    else
        return llvm::ConstantFP::get(*drv.context, llvm::APFloat(Val));
};

/****************** Variable Expression TreeAST *******************/
VariableExprAST::VariableExprAST(std::string& Name) :
    Name(Name)
{
    top = false;
};

const std::string& VariableExprAST::getName() const { return Name; };

void VariableExprAST::visit() { std::cout << getName() << " "; };

llvm::Value* VariableExprAST::codegen(driver& drv)
{
    if (gettop()) {
        return TopExpression(this, drv);
    } else {
        llvm::Value* V = drv.NamedValues[Name];
        if (!V)
            LogErrorV("Variabile non definita");
        return V;
    }
};

/******************** Binary Expression Tree **********************/
BinaryExprAST::BinaryExprAST(Operator Op, ExprAST* LHS, ExprAST* RHS) :
    Op(Op), LHS(LHS), RHS(RHS)
{
    top = false;
}

void BinaryExprAST::visit()
{
    std::cout << "(" << Op << " ";
    LHS->visit();
    if (RHS != nullptr)
        RHS->visit();
    std::cout << ")";
}

llvm::Value* BinaryExprAST::codegen(driver& drv)
{
    if (gettop()) {
        return TopExpression(this, drv);
    } else {
        llvm::Value* L = LHS->codegen(drv);
        llvm::Value* R = RHS->codegen(drv);
        if (!L || !R)
            return nullptr;
        switch (Op) {
            case Operator::PLUS:
                return drv.builder->CreateFAdd(L, R, "addregister");
            case Operator::MINUS:
                return drv.builder->CreateFSub(L, R, "subregister");
            case Operator::STAR:
                return drv.builder->CreateFMul(L, R, "mulregister");
            case Operator::SLASH:
                return drv.builder->CreateFDiv(L, R, "addregister");
            case Operator::LESS_THAN:
                L = drv.builder->CreateFCmpULT(L, R, "cmptmp");
                return drv.builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*drv.context), "booltmp");
            case Operator::LESS_EQUAL:
                L = drv.builder->CreateFCmpULE(L, R, "cmptemp");
                return drv.builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*drv.context), "booltmp");
            case Operator::GREATER_THAN:
                L = drv.builder->CreateFCmpUGT(L, R, "cmptmp");
                return drv.builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*drv.context), "booltmp");
            case Operator::GREATER_EQUAL:
                L = drv.builder->CreateFCmpUGE(L, R, "cmptmp");
                return drv.builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*drv.context), "booltmp");
            case Operator::EQUAL:
                L = drv.builder->CreateFCmpUEQ(L, R, "cmptmp");
                return drv.builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*drv.context), "booltmp");
            case Operator::NOT_EQUAL:
                L = drv.builder->CreateFCmpUNE(L, R, "cmptmp");
                return drv.builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*drv.context), "booltmp");
            case Operator::COMPOUND:
                return R;
            default:
                return LogErrorV("Operatore binario non supportato");
        }
    }
}

UnaryExprAST::UnaryExprAST(const Operator& op, ExprAST* operand) :
    op(op), operand(operand) {}

Value* UnaryExprAST::codegen(driver& drv)
{
    Value* exprV = operand->codegen(drv);

    switch (op) {
        case Operator::MINUS:
            return drv.builder->CreateFNeg(exprV, "negreg");
        default:
            return LogErrorV("Operatore unario non supportato");
    }
}

/********************* Call Expression Tree ***********************/
CallExprAST::CallExprAST(std::string Callee, std::vector<ExprAST*> Args) :
    Callee(Callee), Args(std::move(Args))
{
    top = false;
};
void CallExprAST::visit()
{
    std::cout << Callee << "( ";
    for (ExprAST* arg : Args) {
        arg->visit();
    };
    std::cout << ')';
};

Value* CallExprAST::codegen(driver& drv)
{
    if (gettop()) {
        return TopExpression(this, drv);
    } else {
        // Cerchiamo la funzione nell'ambiente globale
        Function* CalleeF = drv.module->getFunction(Callee);
        if (!CalleeF)
            return LogErrorV("Funzione non definita");
        // Controlliamo che gli argomenti coincidano in numero coi parametri
        if (CalleeF->arg_size() != Args.size())
            return LogErrorV("Numero di argomenti non corretto");
        std::vector<Value*> ArgsV;
        for (auto arg : Args) {
            ArgsV.push_back(arg->codegen(drv));
            if (!ArgsV.back())
                return nullptr;
        }
        return drv.builder->CreateCall(CalleeF, ArgsV, "calltmp");
    }
}

/************************* Prototype Tree *************************/
PrototypeAST::PrototypeAST(std::string Name, std::vector<std::string> Args) :
    Name(Name), Args(std::move(Args))
{
    emit = true;
};

const std::string& PrototypeAST::getName() const { return Name; };
const std::vector<std::string>& PrototypeAST::getArgs() const { return Args; };

void PrototypeAST::visit()
{
    std::cout << "extern " << getName() << "( ";
    for (auto it = getArgs().begin(); it != getArgs().end(); ++it) {
        std::cout << *it << ' ';
    };
    std::cout << ')';
};

void PrototypeAST::noemit() { emit = false; };

bool PrototypeAST::emitp() { return emit; };

Function* PrototypeAST::codegen(driver& drv)
{
    // Costruisce una struttura double(double,...,double) che descrive
    // tipo di ritorno e tipo dei parametri (in Kaleidoscope solo double)
    std::vector<Type*> Doubles(Args.size(), Type::getDoubleTy(*drv.context));
    FunctionType* FT =
        FunctionType::get(Type::getDoubleTy(*drv.context), Doubles, false);
    Function* F =
        Function::Create(FT, Function::ExternalLinkage, Name, *drv.module);

    // Attribuiamo agli argomenti il nome dei parametri formali specificati dal
    // programmatore
    unsigned Idx = 0;
    for (auto& Arg : F->args())
        Arg.setName(Args[Idx++]);

    if (emitp()) { // emitp() restituisce true se e solo se il prototipo è
                   // definito extern
        F->print(errs());
        fprintf(stderr, "\n");
    };

    return F;
}

/************************* Function Tree **************************/
FunctionAST::FunctionAST(PrototypeAST* Proto, ExprAST* Body) :
    Proto(Proto), Body(Body)
{
    if (Body == nullptr)
        external = true;
    else
        external = false;
};

void FunctionAST::visit()
{
    std::cout << Proto->getName() << "( ";
    for (auto it = Proto->getArgs().begin(); it != Proto->getArgs().end(); ++it) {
        std::cout << *it << ' ';
    };
    std::cout << ')';
    Body->visit();
};

Function* FunctionAST::codegen(driver& drv)
{
    // Verifica che non esiste già, nel contesto, una funzione con lo stesso nome
    std::string name = Proto->getName();
    Function* TheFunction = drv.module->getFunction(name);
    // E se non esiste prova a definirla
    if (TheFunction) {
        LogErrorV("Funzione " + name + " già definita");
        return nullptr;
    }
    if (!TheFunction)
        TheFunction = Proto->codegen(drv);
    if (!TheFunction)
        return nullptr; // Se la definizione "fallisce" restituisce nullptr

    // Crea un blocco di base in cui iniziare a inserire il codice
    BasicBlock* BB = BasicBlock::Create(*drv.context, "entry", TheFunction);
    drv.builder->SetInsertPoint(BB);

    // Registra gli argomenti nella symbol table
    drv.NamedValues.clear();
    for (auto& Arg : TheFunction->args())
        drv.NamedValues[std::string(Arg.getName())] = &Arg;

    if (Value* RetVal = Body->codegen(drv)) {
        // Termina la creazione del codice corrispondente alla funzione
        drv.builder->CreateRet(RetVal);

        // Effettua la validazione del codice e un controllo di consistenza
        verifyFunction(*TheFunction);

        TheFunction->print(errs());
        fprintf(stderr, "\n");
        return TheFunction;
    }

    // Errore nella definizione. La funzione viene rimossa
    TheFunction->eraseFromParent();
    return nullptr;
};

IfExprNode::IfExprNode(ExprAST* condition, ExprAST* thenExpr, ExprAST* elseExpr)
{
    this->condition = condition;
    this->thenExpr = thenExpr;
    this->elseExpr = elseExpr;
}

void IfExprNode::visit()
{
    std::cout << "(";
    condition->visit();

    std::cout << " then ";

    thenExpr->visit();

    std::cout << " else ";
    elseExpr->visit();

    std::cout << ")";
}

Value* IfExprNode::codegen(driver& drv)
{
    Value* conditionRegistry = condition->codegen(drv);

    conditionRegistry = drv.builder->CreateFCmpONE(conditionRegistry, ConstantFP::get(*drv.context, APFloat(0.0)), "iftest");

    Function* function = drv.builder->GetInsertBlock()->getParent();
    BasicBlock* thenBB = BasicBlock::Create(*drv.context, "then", function);
    BasicBlock* elseBB = BasicBlock::Create(*drv.context, "else");
    BasicBlock* mergeBB = BasicBlock::Create(*drv.context, "merge");

    drv.builder->CreateCondBr(conditionRegistry, thenBB, elseBB);

    drv.builder->SetInsertPoint(thenBB);

    Value* thenV = thenExpr->codegen(drv);
    drv.builder->CreateBr(mergeBB);

    thenBB = drv.builder->GetInsertBlock();
    function->getBasicBlockList().push_back(elseBB);
    drv.builder->SetInsertPoint(elseBB);

    Value* elseV = elseExpr->codegen(drv);
    drv.builder->CreateBr(mergeBB);

    elseBB = drv.builder->GetInsertBlock();
    function->getBasicBlockList().push_back(mergeBB);
    drv.builder->SetInsertPoint(mergeBB);

    PHINode* IFRES = drv.builder->CreatePHI(Type::getDoubleTy(*drv.context), 2, "ifres");

    IFRES->addIncoming(thenV, thenBB);
    IFRES->addIncoming(elseV, elseBB);

    return IFRES;
}
