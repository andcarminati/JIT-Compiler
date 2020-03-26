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

#include "mlir/IR/Module.h"
#include "MLIRGen.h"

MLIRGen::MLIRGen(mlir::MLIRContext* context) : builder(context) {
    theModule = mlir::ModuleOp::create(builder.getUnknownLoc());
}

void MLIRGen::GenFromAST(std::unique_ptr<PrimaryAST<MLIRValue>> node) {

}

void MLIRGen::visit(PrototypeAST<MLIRValue>* node) {
    
}

void MLIRGen::visit(FunctionAST<MLIRValue>* node) {
    
}

void MLIRGen::visit(IfExprAST<MLIRValue>* ifexp){
    
}

void MLIRGen::visit(ReturnAST<MLIRValue>* ifexp){
    
}

void MLIRGen::visit(ForExprAST<MLIRValue>* node){
    
}

void MLIRGen::visit(WhileExprAST<MLIRValue>* node){
    
}

MLIRValue MLIRGen::visit(VariableExprAST<MLIRValue>* node){
    return nullptr;
}

MLIRValue MLIRGen::visit(RealNumberExprAST<MLIRValue>* node){
    return nullptr;
}

MLIRValue MLIRGen::visit(IntegerNumberExprAST<MLIRValue>* node){
    return nullptr;
}

MLIRValue MLIRGen::visit(BinaryExprAST<MLIRValue>* node){
    return nullptr;
}

MLIRValue MLIRGen::visit(UnaryExprAST<MLIRValue>* node){
    return nullptr;
}

MLIRValue MLIRGen::visit(CallExprAST<MLIRValue>* node){
    return nullptr;
}

MLIRValue MLIRGen::visit(LocalVarDeclarationExprAST<MLIRValue>* node){
    return nullptr;
}
    
std::unique_ptr<llvm::Module> MLIRGen::getModule(){
    return nullptr;
}