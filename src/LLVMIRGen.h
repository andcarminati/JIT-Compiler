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
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <map>
#include "AST.h"
#include "SymbolTable.h"
#include "LangDefs.h"
#include "AbstractIRGen.h"

class DebugInfo;

using LLVMValue = llvm::Value*;

using namespace llvm;

// consumes the AST generating LLVM IR
class LLVMIRGen : public AbstractIRGen<LLVMValue>{
public:
    LLVMIRGen(LLVMContext* TheContext);
    virtual void GenFromAST(std::unique_ptr<PrimaryAST<LLVMValue>> node) override;
    virtual void visit(PrototypeAST<LLVMValue>* node) override;
    virtual void visit(FunctionAST<LLVMValue>* node) override;
    virtual void visit(IfExprAST<LLVMValue>* ifexp) override;
    virtual void visit(ReturnAST<LLVMValue>* ifexp) override;
    virtual void visit(ForExprAST<LLVMValue>* node) override;
    virtual void visit(WhileExprAST<LLVMValue>* node) override;
    virtual llvm::Value* visit(VariableExprAST<LLVMValue>* node) override;
    virtual llvm::Value* visit(RealNumberExprAST<LLVMValue>* node) override;
    virtual llvm::Value* visit(IntegerNumberExprAST<LLVMValue>* node) override;
    virtual llvm::Value* visit(BinaryExprAST<LLVMValue>* node) override;
    virtual llvm::Value* visit(UnaryExprAST<LLVMValue>* node) override;
    virtual llvm::Value* visit(CallExprAST<LLVMValue>* node) override;
    virtual llvm::Value* visit(LocalVarDeclarationExprAST<LLVMValue>* node) override;
    virtual std::unique_ptr<Module> getModule() override;
    
private:
    SymbolTable symbolTable;
    LLVMContext* TheContext;
    llvm::BasicBlock* currentRetBB = nullptr;
    std::unique_ptr<legacy::FunctionPassManager> TheFPM;
    std::unique_ptr<IRBuilder<>> Builder;
    std::unique_ptr<Module> TheModule;
    std::map<std::string, llvm::Value *> NamedValues;
    llvm::Function* visitFunctionPrototypeImpl(PrototypeAST<LLVMValue>* node);
    llvm::Function* visitFunctionImpl(FunctionAST<LLVMValue>* node);
    llvm::BasicBlock* visitExpBlock(std::unique_ptr<ExprBlockAST<LLVMValue>> block, 
        std::string name, Function* function);
    
    void allocSpaceForParams(Function* function, BasicBlock* BB);
    llvm::Value*  allocLocalVar(Function* function, std::string& name, VarType type, DebugInfo* DI);
    
};

#endif	/* IRGEN_H */

