//    Copyright 2019 Andreu Carminati
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.



#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/ErrorOr.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include "Parser.h"
#include "Lexer.h"
#include "AST.h"
#include "LLVMIRGen.h"
#include "Optimizer.h"
#include "Executor.h"

namespace cl = llvm::cl;
using namespace std;


static cl::opt<std::string> inputFilename(cl::Positional,
        cl::desc("<input source file>"),
        cl::init("-"),
        cl::value_desc("filename"));

namespace {

    enum IrType {
        LLVMIR, MLIR
    };
}
static cl::opt<enum IrType> irType(
        "ir", cl::init(LLVMIR), cl::desc("Decided the kind of output desired"),
        cl::values(clEnumValN(LLVMIR, "llvmir", "process input using LLVM IR gen.")),
        cl::values(clEnumValN(MLIR, "mlir", "process input using MLIR IR gen.")));

namespace {

    enum Action {
        None, DumpAST, DumpIR
    };
}
static cl::opt<enum Action> emitAction(
        "emit", cl::desc("Select the kind of output desired"),
        cl::values(clEnumValN(DumpAST, "ast", "output the AST dump")),
        cl::values(clEnumValN(DumpIR, "dumpir", "output the LLVM IR dump")));

/// Returns a Toy AST resulting from parsing the file or a nullptr on error.

template<typename T>
std::unique_ptr<Parser<T>> parseInputFile(llvm::StringRef filename) {
    llvm::ErrorOr<std::unique_ptr < llvm::MemoryBuffer>> fileOrErr =
            llvm::MemoryBuffer::getFileOrSTDIN(filename);
    if (std::error_code ec = fileOrErr.getError()) {
        llvm::errs() << "Could not open input file: " << ec.message() << "\n";
        return nullptr;
    }
    auto buffer = fileOrErr.get()->getBuffer();
    // LexerBuffer lexer(buffer.begin(), buffer.end(), );
    auto lexer = std::make_unique<Lexer>(buffer.begin(), buffer.end(), std::string(filename));
    auto parser = std::make_unique<Parser < T >> (std::move(lexer));
    return std::move(parser);
    //exit(0);
}

int dumpMLIR() {

    return 0;
}

int dumpAST() {
    return 0;
}

static llvm::LLVMContext TheContext;
int LLVMIRDriver() {

    auto parser = parseInputFile<llvm::Value*>(inputFilename);

    auto generator = std::make_unique<LLVMIRGen>(LLVMIRGen(&TheContext));

    while (true) {
        auto exp = parser->nextConstruct();
        if (parser->hasFail()) {
            llvm::errs() << "Aborting compilation\n";
            return -1;
        }
        if (!exp) {
            break;
        }
        generator->GenFromAST(std::move(exp));
    }

    auto optimizer = std::make_unique<Optimizer>(Optimizer(&TheContext, generator->getModule()));
    optimizer->optimizeCode();

    auto executor = std::make_unique<Executor>(Executor(&TheContext, optimizer->getModule()));
    executor->execute();

    return 0;
}

int main(int argc, char **argv) {
    cl::ParseCommandLineOptions(argc, argv, "jit compiler\n");

    switch (irType) {
        case IrType::LLVMIR:
            return LLVMIRDriver();
        case IrType::MLIR:
            //return dumpMLIR();
            break;
        default:
            llvm::errs() << "No action specified (parsing only?), use -emit=<action>\n";
    }

    return 0;
}
