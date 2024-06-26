#include "driver.hh"
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetOptions.h>
#include <optional>

using namespace llvm;

int main(int argc, char* argv[])
{
    int exitCode = 0;
    driver drv;
    /***********************************************************************/
    /********* Inizializzazione del target (local machine) *****************/
    /***********************************************************************/
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    drv.module->setTargetTriple(TargetTriple);
    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);
    if (!Target)
    {
        llvm::errs() << Error;
        return 1;
    }
    /************************** Set-up macchina target ********************/
    auto CPU = "generic";
    auto Features = "";
    TargetOptions opt;
    auto RM = std::optional<llvm::Reloc::Model>();
    auto TheTargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);
    /************************* Configurazione del modulo *****************/
    drv.module->setDataLayout(TheTargetMachine->createDataLayout());
    drv.module->setTargetTriple(TargetTriple);
    /***********************************************************************/
    /************* Fine set-up per creazione codice oggetto ****************/
    /***********************************************************************/
    int i = 1;
    std::string Filename = ""; // Il default è che il codice oggetto non viene generato

    while (i < argc)
    {
        if (argv[i] == std::string("-p"))
        {
            drv.trace_parsing = true; // Abilita tracce debug nel parser
        }
        else if (argv[i] == std::string("-s"))
        {
            drv.trace_scanning = true; // Abilita tracce debug nello scanner
        }
        else if (argv[i] == std::string("-v"))
        {
            drv.ast_print = true; // Stampa una rapp. esterna dell'AST
        }
        else if (argv[i] == std::string("-o"))
        {
            Filename = argv[++i] + (std::string) ".o"; // Crea codice oggetto nel file indicato
        }
        else if (!drv.parse(argv[i]))
        { // Parsing e creazione dell'AST
            drv.codegen(); // Visita AST e generazione dell'IR (su stdout)
            if (Filename != "")
            {
                /*****************************************************************/
                /******************** Generazione codice oggetto *****************/
                /*****************************************************************/
                std::error_code EC;
                raw_fd_ostream dest(Filename, EC, sys::fs::OF_None);
                if (EC)
                {
                    errs() << "Could not open file: " << EC.message();
                    return 1;
                }
                legacy::PassManager pass;
                auto FileType = CGFT_ObjectFile;
                if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType))
                {
                    errs() << "TheTargetMachine can't emit a file of this type";
                    return 1;
                }
                pass.run(*drv.module); // Compilazione dell'IR prodotto dal frontend
                dest.flush();
                outs() << "Wrote " << Filename << "\n";
                return 0;
            }
        }
        else
        {
            exitCode = 1;
        }

        i++;
    }

    return exitCode;
}
