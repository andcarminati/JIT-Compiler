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



#ifndef IRGEN_H
#define	IRGEN_H
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <map>
#include "AST.h"
#include "SymbolTable.h"

class PrimaryAST;
class PrototypeAST;
class FunctionAST;
class ExprBlockAST;
class IfExprAST;
class VariableExprAST;
class DoubleNumberExprAST;
class BinaryExprAST;
class ReturnAST;
class CallExprAST;

using namespace llvm;

// consumes the AST generating LLVM IR
class IRGen {
public:
    IRGen(LLVMContext* TheContext);
    void GenFromAST(std::unique_ptr<PrimaryAST> node);
    void visit(PrototypeAST* node);
    void visit(FunctionAST* node);
    void visit(IfExprAST* ifexp);
    void visit(ReturnAST* ifexp);
    llvm::Value* visit(VariableExprAST* node);
    llvm::Value* visit(DoubleNumberExprAST* node);
    llvm::Value* visit(BinaryExprAST* node);
    llvm::Value* visit(CallExprAST* node);
    std::unique_ptr<Module> getModule();
    
private:
    SymbolTable symbolTable;
    LLVMContext* TheContext;
    llvm::BasicBlock* currentRetBB = nullptr;
    std::unique_ptr<legacy::FunctionPassManager> TheFPM;
    std::unique_ptr<IRBuilder<>> Builder;
    std::unique_ptr<Module> TheModule;
    std::map<std::string, llvm::Value *> NamedValues;
    llvm::Function* visitFunctionPrototypeImpl(PrototypeAST* node);
    llvm::Function* visitFunctionImpl(FunctionAST* node);
    llvm::BasicBlock* visitExpBlock(std::unique_ptr<ExprBlockAST> block, 
        std::string name, Function* function);
    
    void allocSpaceForParams(Function* function, BasicBlock* BB);
    
    
};

#endif	/* IRGEN_H */

