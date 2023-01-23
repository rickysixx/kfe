#include "driver.hh"
#include "operator.hh"
#include "parser.hh"
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Operator.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Use.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>

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
