//    Copyright 2020 Andreu Carminati
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


#ifndef MLIRGEN_H
#define	MLIRGEN_H

#include "mlir/Analysis/Verifier.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/Function.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Module.h"
#include "mlir/IR/StandardTypes.h"

#include "llvm/IR/Module.h"
#include "AST.h"
#include "SymbolTable.h"
#include "LangDefs.h"
#include "AbstractIRGen.h"

using namespace mlir;

using MLIRValue = mlir::Value;

// consumes the AST generating MLIR IR
class MLIRGen  : public AbstractIRGen<MLIRValue>{
public:
virtual void GenFromAST(std::unique_ptr<PrimaryAST<MLIRValue>> node) override;
    virtual void visit(PrototypeAST<MLIRValue>* node) override;
    virtual void visit(FunctionAST<MLIRValue>* node) override;
    virtual void visit(IfExprAST<MLIRValue>* ifexp) override;
    virtual void visit(ReturnAST<MLIRValue>* ifexp) override;
    virtual void visit(ForExprAST<MLIRValue>* node) override;
    virtual void visit(WhileExprAST<MLIRValue>* node) override;
    virtual MLIRValue visit(VariableExprAST<MLIRValue>* node) override;
    virtual MLIRValue visit(RealNumberExprAST<MLIRValue>* node) override;
    virtual MLIRValue visit(IntegerNumberExprAST<MLIRValue>* node) override;
    virtual MLIRValue visit(BinaryExprAST<MLIRValue>* node) override;
    virtual MLIRValue visit(UnaryExprAST<MLIRValue>* node) override;
    virtual MLIRValue visit(CallExprAST<MLIRValue>* node) override;
    virtual MLIRValue visit(LocalVarDeclarationExprAST<MLIRValue>* node) override;
    virtual std::unique_ptr<llvm::Module> getModule() override;
private:
  mlir::ModuleOp theModule;

  mlir::OpBuilder builder;
};

#endif	/* MLIRGEN_H */

