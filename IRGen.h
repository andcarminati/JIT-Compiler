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
#include "LangDefs.h"

class DebugInfo;

using LLMVValue = llvm::Value*;

template<typename T>
class PrimaryAST;
template<typename T>
class PrototypeAST;
template<typename T>
class FunctionAST;
template<typename T>
class ExprBlockAST;
template<typename T>
class IfExprAST;
template<typename T>
class VariableExprAST;
template<typename T>
class RealNumberExprAST;
template<typename T>
class IntegerNumberExprAST;
template<typename T>
class BinaryExprAST;
template<typename T>
class ReturnAST;
template<typename T>
class CallExprAST;
template<typename T>
class UnaryExprAST;
template<typename T>
class LocalVarDeclarationExprAST;
template<typename T>
class ForExprAST;
template<typename T>
class WhileExprAST;

using namespace llvm;

// consumes the AST generating LLVM IR
class IRGen {
public:
    IRGen(LLVMContext* TheContext);
    void GenFromAST(std::unique_ptr<PrimaryAST<LLMVValue>> node);
    void visit(PrototypeAST<LLMVValue>* node);
    void visit(FunctionAST<LLMVValue>* node);
    void visit(IfExprAST<LLMVValue>* ifexp);
    void visit(ReturnAST<LLMVValue>* ifexp);
    void visit(ForExprAST<LLMVValue>* node);
    void visit(WhileExprAST<LLMVValue>* node);
    llvm::Value* visit(VariableExprAST<LLMVValue>* node);
    llvm::Value* visit(RealNumberExprAST<LLMVValue>* node);
    llvm::Value* visit(IntegerNumberExprAST<LLMVValue>* node);
    llvm::Value* visit(BinaryExprAST<LLMVValue>* node);
    llvm::Value* visit(UnaryExprAST<LLMVValue>* node);
    llvm::Value* visit(CallExprAST<LLMVValue>* node);
    llvm::Value* visit(LocalVarDeclarationExprAST<LLMVValue>* node);
    std::unique_ptr<Module> getModule();
    
private:
    SymbolTable symbolTable;
    LLVMContext* TheContext;
    llvm::BasicBlock* currentRetBB = nullptr;
    std::unique_ptr<legacy::FunctionPassManager> TheFPM;
    std::unique_ptr<IRBuilder<>> Builder;
    std::unique_ptr<Module> TheModule;
    std::map<std::string, llvm::Value *> NamedValues;
    llvm::Function* visitFunctionPrototypeImpl(PrototypeAST<LLMVValue>* node);
    llvm::Function* visitFunctionImpl(FunctionAST<LLMVValue>* node);
    llvm::BasicBlock* visitExpBlock(std::unique_ptr<ExprBlockAST<LLMVValue>> block, 
        std::string name, Function* function);
    
    void allocSpaceForParams(Function* function, BasicBlock* BB);
    llvm::Value*  allocLocalVar(Function* function, std::string& name, VarType type, DebugInfo* DI);
    
};

#endif	/* IRGEN_H */

