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

#include <llvm/Support/ErrorHandling.h>
#include "mlir/IR/Module.h"
#include "MLIRGen.h"

MLIRGen::MLIRGen(mlir::MLIRContext* context) : builder(context) {
    theModule = mlir::ModuleOp::create(builder.getUnknownLoc());
}

void MLIRGen::GenFromAST(std::unique_ptr<PrimaryAST<MLIRValue>> node) {
    llvm_unreachable("Unimplemented MLIRGen member function");
}

void MLIRGen::visit(PrototypeAST<MLIRValue>* node) {
    llvm_unreachable("Unimplemented MLIRGen member function");
}

void MLIRGen::visit(FunctionAST<MLIRValue>* node) {
    llvm_unreachable("Unimplemented MLIRGen member function");
}

void MLIRGen::visit(IfExprAST<MLIRValue>* ifexp){
    llvm_unreachable("Unimplemented MLIRGen member function");
}

void MLIRGen::visit(ReturnAST<MLIRValue>* ifexp){
    llvm_unreachable("Unimplemented MLIRGen member function");
}

void MLIRGen::visit(ForExprAST<MLIRValue>* node){
    llvm_unreachable("Unimplemented MLIRGen member function");
}

void MLIRGen::visit(WhileExprAST<MLIRValue>* node){
    llvm_unreachable("Unimplemented MLIRGen member function");
}

MLIRValue MLIRGen::visit(VariableExprAST<MLIRValue>* node){
    llvm_unreachable("Unimplemented MLIRGen member function");
    return nullptr;
}

MLIRValue MLIRGen::visit(RealNumberExprAST<MLIRValue>* node){
    llvm_unreachable("Unimplemented MLIRGen member function");
    return nullptr;
}

MLIRValue MLIRGen::visit(IntegerNumberExprAST<MLIRValue>* node){
    llvm_unreachable("Unimplemented MLIRGen member function");
    return nullptr;
}

MLIRValue MLIRGen::visit(BinaryExprAST<MLIRValue>* node){
    llvm_unreachable("Unimplemented MLIRGen member function");
    return nullptr;
}

MLIRValue MLIRGen::visit(UnaryExprAST<MLIRValue>* node){
    llvm_unreachable("Unimplemented MLIRGen member function");
    return nullptr;
}

MLIRValue MLIRGen::visit(CallExprAST<MLIRValue>* node){
    llvm_unreachable("Unimplemented MLIRGen member function");
    return nullptr;
}

MLIRValue MLIRGen::visit(LocalVarDeclarationExprAST<MLIRValue>* node){
    llvm_unreachable("Unimplemented MLIRGen member function");
    return nullptr;
}
    
std::unique_ptr<llvm::Module> MLIRGen::getModule(){
    llvm_unreachable("Unimplemented MLIRGen member function");
    return nullptr;
}