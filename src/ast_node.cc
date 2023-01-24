#include "ast_node.hh"
#include "driver.hh"
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/IRBuilderFolder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/Alignment.h>
#include <stdexcept>
#include <vector>

static llvm::AllocaInst* CreateEntryBlockAlloca(const driver& drv, llvm::Function* function, const std::string& varName)
{
    llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());

    return tmpBuilder.CreateAlloca(llvm::Type::getDoubleTy(*drv.context), 0, varName.c_str());
}

llvm::Value* LogErrorV(const std::string Str)
{
    std::cerr << Str << std::endl;
    return nullptr;
}

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
    if (first != nullptr)
    {
        first->visit();
    }
    else
    {
        if (continuation == nullptr)
        {
            return;
        };
    };
    std::cout << ";";
    continuation->visit();
};

llvm::Value* SeqAST::codegen(driver& drv)
{
    if (first != nullptr)
    {
        first->codegen(drv);
    }
    else
    {
        if (continuation == nullptr)
            return nullptr;
    }
    continuation->codegen(drv);
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
    llvm::AllocaInst* A = drv.NamedValues[Name];

    if (!A)
    {
        return LogErrorV("Variabile [" + Name + "] non dichiarata.");
    }

    return drv.builder->CreateLoad(A->getAllocatedType(), A, Name.c_str());
}

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
    if (Op == Operator::EQ)
    {
        llvm::Value* rhsValue = nullptr;
        llvm::Value* lhsAddress = nullptr;

        if (ArrayIndexingExprAST* arrayExpr = dynamic_cast<ArrayIndexingExprAST*>(this->RHS))
        {
            llvm::Value* elementAddress = arrayExpr->codegen(drv);

            rhsValue = drv.builder->CreateLoad(llvm::Type::getDoubleTy(*drv.context), elementAddress);
        }
        else
        {
            rhsValue = this->RHS->codegen(drv);
        }

        if (VariableExprAST* variableExpr = dynamic_cast<VariableExprAST*>(this->LHS))
        {
            lhsAddress = drv.NamedValues[variableExpr->getName()];
        }
        else if (ArrayIndexingExprAST* arrayExpr = dynamic_cast<ArrayIndexingExprAST*>(this->LHS))
        {
            lhsAddress = arrayExpr->codegen(drv);
        }

        if (!rhsValue)
        {
            throw std::runtime_error("Errore nel calcolo del right value.");
        }

        if (!lhsAddress)
        {
            throw std::runtime_error("Errore nel calcolo dell'indirizzo del left value.");
        }

        drv.builder->CreateStore(rhsValue, lhsAddress);

        return rhsValue;
    }

    if (gettop())
    {
        return TopExpression(this, drv);
    }
    else
    {
        llvm::Value* L = LHS->codegen(drv);
        llvm::Value* R = nullptr;

        if (ArrayIndexingExprAST* rhsArray = dynamic_cast<ArrayIndexingExprAST*>(RHS))
        {
            auto* gep = rhsArray->codegen(drv);
            R = drv.builder->CreateLoad(llvm::Type::getDoubleTy(*drv.context), gep);
        }
        else
        {
            R = RHS->codegen(drv);
        }

        if (!L || !R)
            return nullptr;
        switch (Op)
        {
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

llvm::Value* UnaryExprAST::codegen(driver& drv)
{
    llvm::Value* exprV = operand->codegen(drv);

    switch (op)
    {
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
    for (ExprAST* arg : Args)
    {
        arg->visit();
    };
    std::cout << ')';
};

llvm::Value* CallExprAST::codegen(driver& drv)
{
    if (gettop())
    {
        return TopExpression(this, drv);
    }
    else
    {
        // Cerchiamo la funzione nell'ambiente globale
        llvm::Function* CalleeF = drv.module->getFunction(Callee);
        if (!CalleeF)
            return LogErrorV("Funzione non definita");
        // Controlliamo che gli argomenti coincidano in numero coi parametri
        if (CalleeF->arg_size() != Args.size())
            return LogErrorV("Numero di argomenti non corretto");
        std::vector<llvm::Value*> ArgsV;
        for (auto arg : Args)
        {
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
    for (auto it = getArgs().begin(); it != getArgs().end(); ++it)
    {
        std::cout << *it << ' ';
    };
    std::cout << ')';
};

void PrototypeAST::noemit() { emit = false; };

bool PrototypeAST::emitp() { return emit; };

llvm::Function* PrototypeAST::codegen(driver& drv)
{
    // Costruisce una struttura double(double,...,double) che descrive
    // tipo di ritorno e tipo dei parametri (in Kaleidoscope solo double)
    std::vector<llvm::Type*> Doubles(Args.size(), llvm::Type::getDoubleTy(*drv.context));
    llvm::FunctionType* FT =
        llvm::FunctionType::get(llvm::Type::getDoubleTy(*drv.context), Doubles, false);
    llvm::Function* F =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Name, *drv.module);

    // Attribuiamo agli argomenti il nome dei parametri formali specificati dal
    // programmatore
    unsigned Idx = 0;
    for (auto& Arg : F->args())
        Arg.setName(Args[Idx++]);

    if (emitp())
    { // emitp() restituisce true se e solo se il prototipo è
      // definito extern
        F->print(llvm::errs());
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
    for (auto it = Proto->getArgs().begin(); it != Proto->getArgs().end(); ++it)
    {
        std::cout << *it << ' ';
    };
    std::cout << ')';
    Body->visit();
};

llvm::Function* FunctionAST::codegen(driver& drv)
{
    // Verifica che non esiste già, nel contesto, una funzione con lo stesso nome
    std::string name = Proto->getName();
    llvm::Function* TheFunction = drv.module->getFunction(name);
    // E se non esiste prova a definirla
    if (TheFunction)
    {
        LogErrorV("Funzione " + name + " già definita");
        return nullptr;
    }
    if (!TheFunction)
        TheFunction = Proto->codegen(drv);
    if (!TheFunction)
        return nullptr; // Se la definizione "fallisce" restituisce nullptr

    // Crea un blocco di base in cui iniziare a inserire il codice
    llvm::BasicBlock* BB = llvm::BasicBlock::Create(*drv.context, "entry", TheFunction);
    drv.builder->SetInsertPoint(BB);

    // Registra gli argomenti nella symbol table
    drv.NamedValues.clear();
    for (auto& Arg : TheFunction->args())
    {
        llvm::AllocaInst* Alloca = CreateEntryBlockAlloca(drv, TheFunction, std::string(Arg.getName()));

        drv.builder->CreateStore(&Arg, Alloca);

        drv.NamedValues[std::string(Arg.getName())] = Alloca;
    }

    if (llvm::Value* RetVal = Body->codegen(drv))
    {
        // Termina la creazione del codice corrispondente alla funzione
        drv.builder->CreateRet(RetVal);

        // Effettua la validazione del codice e un controllo di consistenza
        verifyFunction(*TheFunction);

        TheFunction->print(llvm::errs());
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

llvm::Value* IfExprNode::codegen(driver& drv)
{
    llvm::Value* conditionRegistry = condition->codegen(drv);

    conditionRegistry = drv.builder->CreateFCmpONE(conditionRegistry, llvm::ConstantFP::get(*drv.context, llvm::APFloat(0.0)), "iftest");

    llvm::Function* function = drv.builder->GetInsertBlock()->getParent();
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(*drv.context, "then", function);
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(*drv.context, "else");
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*drv.context, "merge");

    drv.builder->CreateCondBr(conditionRegistry, thenBB, elseBB);

    drv.builder->SetInsertPoint(thenBB);

    llvm::Value* thenV = thenExpr->codegen(drv);
    drv.builder->CreateBr(mergeBB);

    thenBB = drv.builder->GetInsertBlock();
    function->getBasicBlockList().push_back(elseBB);
    drv.builder->SetInsertPoint(elseBB);

    llvm::Value* elseV = elseExpr->codegen(drv);
    drv.builder->CreateBr(mergeBB);

    elseBB = drv.builder->GetInsertBlock();
    function->getBasicBlockList().push_back(mergeBB);
    drv.builder->SetInsertPoint(mergeBB);

    llvm::PHINode* IFRES = drv.builder->CreatePHI(llvm::Type::getDoubleTy(*drv.context), 2, "ifres");

    IFRES->addIncoming(thenV, thenBB);
    IFRES->addIncoming(elseV, elseBB);

    return IFRES;
}

ForExprAST::ForExprAST(const std::string& varName, ExprAST* start, ExprAST* end, ExprAST* step, ExprAST* body) :
    varName(varName), start(start), end(end), step(step), body(body) {}

llvm::Value* ForExprAST::codegen(driver& drv)
{
    llvm::Function* f = drv.builder->GetInsertBlock()->getParent();
    llvm::AllocaInst* alloca = CreateEntryBlockAlloca(drv, f, varName);
    llvm::Value* startValue = start->codegen(drv);

    drv.builder->CreateStore(startValue, alloca);

    llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(*drv.context, "loop", f);

    drv.builder->CreateBr(loopBB);

    drv.builder->SetInsertPoint(loopBB);

    llvm::AllocaInst* oldVal = drv.NamedValues[varName];
    drv.NamedValues[varName] = alloca;

    body->codegen(drv);

    llvm::Value* stepVal = nullptr;

    if (step != nullptr)
    {
        stepVal = step->codegen(drv);
    }
    else
    {
        stepVal = llvm::ConstantFP::get(*drv.context, llvm::APFloat(1.0));
    }

    llvm::Value* currentVar = drv.builder->CreateLoad(alloca->getAllocatedType(), alloca, varName);

    llvm::Value* nextVar = drv.builder->CreateFAdd(currentVar, stepVal, "nextvar");

    drv.builder->CreateStore(nextVar, alloca);

    llvm::Value* endCond = end->codegen(drv);

    endCond = drv.builder->CreateFCmpONE(endCond, llvm::ConstantFP::get(*drv.context, llvm::APFloat(0.0)), "loopcond");

    llvm::BasicBlock* afterBB = llvm::BasicBlock::Create(*drv.context, "afterloop", f);

    drv.builder->CreateCondBr(endCond, loopBB, afterBB);

    drv.builder->SetInsertPoint(afterBB);

    if (oldVal != nullptr)
    {
        drv.NamedValues[varName] = oldVal;
    }
    else
    {
        drv.NamedValues.erase(varName);
    }

    return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*drv.context));
}

WhileExprAST::WhileExprAST(ExprAST* condition, ExprAST* body) :
    condition(condition), body(body) {}

llvm::Value* WhileExprAST::codegen(driver& drv)
{
    llvm::Function* currentFunction = drv.builder->GetInsertBlock()->getParent();
    llvm::BasicBlock* loopBlock = llvm::BasicBlock::Create(*drv.context, "whileloop", currentFunction);

    drv.builder->CreateBr(loopBlock);
    drv.builder->SetInsertPoint(loopBlock);

    body->codegen(drv);

    llvm::Value* endCondition = condition->codegen(drv);

    endCondition = drv.builder->CreateFCmpONE(endCondition, llvm::ConstantFP::get(*drv.context, llvm::APFloat(0.0)), "whileloopcond");

    llvm::BasicBlock* afterLoopBlock = llvm::BasicBlock::Create(*drv.context, "afterwhileloop", currentFunction);

    drv.builder->CreateCondBr(endCondition, loopBlock, afterLoopBlock);
    drv.builder->SetInsertPoint(afterLoopBlock);

    return llvm::ConstantFP::getNullValue(llvm::Type::getDoubleTy(*drv.context));
}

VarExprAST::VarExprAST(std::vector<std::pair<std::string, ExprAST*>> varNames, ExprAST* body) :
    varNames(varNames), body(body) {}

llvm::Value* VarExprAST::codegen(driver& drv)
{
    std::vector<llvm::AllocaInst*> oldBindings;
    llvm::Function* function = drv.builder->GetInsertBlock()->getParent();

    for (unsigned int i = 0, e = varNames.size(); i != e; i++)
    {
        const std::string& varName = varNames[i].first;
        ExprAST* varInitialValueExpr = varNames[i].second;
        llvm::Value* initialValue = nullptr;
        llvm::AllocaInst* allocaInstr = nullptr;

        if (varInitialValueExpr == nullptr)
        {
            initialValue = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*drv.context), 0.0);
        }
        else if (ArrayInitExprAST* arrayInitExpr = dynamic_cast<ArrayInitExprAST*>(varInitialValueExpr))
        {
            allocaInstr = arrayInitExpr->codegen(drv);

            oldBindings.push_back(drv.NamedValues[varName]);

            drv.NamedValues[varName] = allocaInstr;
        }
        else
        {
            initialValue = varInitialValueExpr->codegen(drv);
            allocaInstr = CreateEntryBlockAlloca(drv, function, varName);

            drv.builder->CreateStore(initialValue, allocaInstr);
        }

        auto* oldValue = drv.NamedValues[varName];

        if (oldValue)
        {
            oldBindings.push_back(oldValue);
        }

        drv.NamedValues[varName] = allocaInstr;
    }

    llvm::Value* bodyVal = nullptr;

    if (ArrayIndexingExprAST* arrayIndexingExpr = dynamic_cast<ArrayIndexingExprAST*>(body))
    {
        // bodyVal is a GEP
        auto* gep = arrayIndexingExpr->codegen(drv);
        bodyVal = drv.builder->CreateLoad(llvm::Type::getDoubleTy(*drv.context), gep);
    }
    else
    {
        bodyVal = body->codegen(drv);
    }

    for (unsigned int i = 0, e = varNames.size(); i != e; i++)
    {
        drv.NamedValues[varNames[i].first] = oldBindings[i];
    }

    return bodyVal;
}

double NumberExprAST::getVal() const
{
    return this->Val;
}

ArrayInitExprAST::ArrayInitExprAST(const std::string& name, unsigned int capacity) :
    name(name), capacity(capacity) {}

llvm::AllocaInst* ArrayInitExprAST::codegen(driver& drv)
{
    auto* arrayType = llvm::ArrayType::get(llvm::Type::getDoubleTy(*drv.context), this->capacity);
    // auto* currentFunction = drv.builder->GetInsertBlock()->getParent();
    //  llvm::IRBuilder<> tmpBuilder(&currentFunction->getEntryBlock(), currentFunction->getEntryBlock().begin());
    auto* allocaInstr = drv.builder->CreateAlloca(arrayType, nullptr, this->name); // passing nullptr as array size because it is already in arrayType

    // allocaInstr->setAlignment(llvm::Align(16)); // TODO sta roba serve a qualcosa?

    // initialize array with all zero
    drv.builder->CreateMemSet(allocaInstr, llvm::ConstantInt::get(llvm::Type::getInt8Ty(*drv.context), 0), this->capacity * sizeof(double), allocaInstr->getAlign());

    return allocaInstr;
}

ArrayIndexingExprAST::ArrayIndexingExprAST(const std::string& name, ExprAST* indexExpr) :
    name(name), indexExpr(indexExpr) {}

llvm::Value* ArrayIndexingExprAST::codegen(driver& drv)
{
    auto* allocaInstr = drv.NamedValues.at(this->name);

    if (!allocaInstr)
    {
        return LogErrorV("Accesso ad un array non dichiarato");
    }

    llvm::Value* indexExprResultAsDouble = indexExpr->codegen(drv);
    llvm::Value* indexExprResultAsUInt = drv.builder->CreateFPToUI(indexExprResultAsDouble, llvm::Type::getInt32Ty(*drv.context));
    llvm::Value* indexExprAs64Bit = drv.builder->CreateZExt(indexExprResultAsUInt, llvm::Type::getInt64Ty(*drv.context));

    std::vector<llvm::Value*> indexes = {
        llvm::ConstantInt::get(llvm::Type::getInt64Ty(*drv.context), 0),
        indexExprAs64Bit};

    auto* gep = drv.builder->CreateInBoundsGEP(allocaInstr->getAllocatedType(), allocaInstr, indexes);

    return gep;
}