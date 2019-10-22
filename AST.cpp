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


#include "AST.h"
#include "IRGen.h"
#include <iostream>

void FunctionAST::acceptIRGenVisitor(IRGen* visitor) {
    visitor->visit(this);
}

void PrototypeAST::acceptIRGenVisitor(IRGen* visitor) {
    visitor->visit(this);
}
llvm::Value* IfExprAST::acceptIRGenVisitor(IRGen* visitor){
    visitor->visit(this);
    return nullptr;
}

llvm::Value* ReturnAST::acceptIRGenVisitor(IRGen* visitor) {
    visitor->visit(this);
    return nullptr;
}

llvm::Value* DoubleNumberExprAST::acceptIRGenVisitor(IRGen* visitor) {
    return visitor->visit(this);
}

llvm::Value* VariableExprAST::acceptIRGenVisitor(IRGen* visitor) {
    return visitor->visit(this);
}

llvm::Value* BinaryExprAST::acceptIRGenVisitor(IRGen* visitor){
    return visitor->visit(this);
}

llvm::Value* CallExprAST::acceptIRGenVisitor(IRGen* visitor){
    
    return visitor->visit(this);
}

