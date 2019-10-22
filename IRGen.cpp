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

//This software contains code derived from the Kaleidoscope LLVM Toturial, including various
//modifications.


#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include "IRGen.h"
#include "Lexer.h"
#include <iostream>
#include <vector>

using namespace llvm;

static void abort(std::string str, std::string str2) {
    std::cout << "Code generator fatal: " << str << str2 << std::endl;
    exit(-1);
}

IRGen::IRGen(llvm::LLVMContext* TheContext) {
    this->TheContext = TheContext;
    Builder = std::make_unique<llvm::IRBuilder<>>(llvm::IRBuilder<>(*TheContext));
    TheModule = std::make_unique<llvm::Module>("my new lang", *TheContext);
    //TheFPM = 
}

// Entry point of the IR Generator. Using a visitor design pattern.
void IRGen::GenFromAST(std::unique_ptr<PrimaryAST> node) {
    node->acceptIRGenVisitor(this);
}

// Prorotype overload.
void IRGen::visit(PrototypeAST* proto) {
    auto *TheFunction = visitFunctionPrototypeImpl(proto);
}
// FunctionAST overload.
void IRGen::visit(FunctionAST* node) {
    // calls the real implementation
    auto *TheFunction = visitFunctionImpl(node);
    if (TheFunction) {
        //TheFunction->print(errs());
    }
}

// Internal method for prototype function generation
Function* IRGen::visitFunctionPrototypeImpl(PrototypeAST* node) {
    std::vector<std::string>& Args = node->getArgs();
    const std::string& Name = node->getName();
    // Make the function type:  double(double,double) etc.
    std::vector<Type *> Doubles(Args.size(), Type::getDoubleTy(*TheContext));
    FunctionType *FT =
            FunctionType::get(Type::getDoubleTy(*TheContext), Doubles, false);

    Function *F =
            Function::Create(FT, Function::ExternalLinkage, Name, TheModule.get());

    // Set names for all arguments.
    unsigned Idx = 0;
    for (auto &Arg : F->args())
        Arg.setName(Args[Idx++]);

    return F;
}

// Private method for the real job. 
Function* IRGen::visitFunctionImpl(FunctionAST* node) {
    // First, check for an existing function from a previous 'extern' declaration.
    const std::string& Name = node->getName();
    Function *TheFunction = TheModule->getFunction(Name);

    if (!TheFunction) {
        // generator must owns the prototype pointer  
        std::unique_ptr<PrototypeAST> proto = node->getProto();
        // gen code for prototype
        proto->acceptIRGenVisitor(this);
        TheFunction = TheModule->getFunction(Name);
    }
    if (!TheFunction) {
        return nullptr;
    }

    if (!TheFunction->empty()) {
        //rever
        std::cout << "Function already defined!\n";
        return nullptr;
    }

    // Create a the first basic block and generate code.
    BasicBlock *BB = visitExpBlock(std::move(node->getBody()), "entry", TheFunction);

    return TheFunction;
}

// Generate allocas for every function parameter plus the return value. We can optimize
// these allcoas with men2reg pass.
void IRGen::allocSpaceForParams(Function* function, BasicBlock* BB) {

    std::list<AllocaInst*> allocas;

    IRBuilder<> TmpB(&function->getEntryBlock(),
            function->getEntryBlock().begin());

    // create Allocas
    for (auto &Arg : function->args()) {
        AllocaInst* Alloca = TmpB.CreateAlloca(Type::getDoubleTy(*TheContext), 0,
                Arg.getName());
        allocas.push_back(Alloca);
    }

    // create Alloca for return value;
    AllocaInst* Alloca = TmpB.CreateAlloca(Type::getDoubleTy(*TheContext), 0,
            "retvalue");

    symbolTable.insertSymbol("retvalue", StorageType::LOCAL, Alloca);

    // Store params in the allocas
    for (auto &Arg : function->args()) {
        AllocaInst* Alloca = allocas.front();
        allocas.pop_front();
        Builder->CreateStore(&Arg, Alloca);
        // populate the symbol table
        symbolTable.insertSymbol(Arg.getName().str(), StorageType::LOCAL, Alloca);
    }


}

// Private method for expression block code generation.
BasicBlock* IRGen::visitExpBlock(std::unique_ptr<ExprBlockAST> block,
        std::string name, Function* function) {

    symbolTable.push_scope();
    bool mainBlock = false;

    BasicBlock *BB;
    // we are dealing with the first block of a function
    if (function != nullptr) {
        BB = BasicBlock::Create(*TheContext, name, function);
        Builder->SetInsertPoint(BB);
        // we must allocate local memory for parameters.
        allocSpaceForParams(function, BB);
        currentRetBB = BasicBlock::Create(*TheContext, "retblock");
        function->getBasicBlockList().push_back(currentRetBB);
        // set flag to true to remember to generate the return block
        mainBlock = true;
    } else {
        BasicBlock* parent = Builder->GetInsertBlock();
        function = parent->getParent();
        BB = BasicBlock::Create(*TheContext, name);
        function->getBasicBlockList().push_back(BB);
        Builder->SetInsertPoint(BB);
    }

    // generate IR for all expressions
    while (!block->empty()) {
        std::unique_ptr<ExprAST> expr = block->nextExp();
        expr->acceptIRGenVisitor(this);
    }

    if (mainBlock) {
        // create the final return statement.
        currentRetBB->moveAfter(Builder->GetInsertBlock());
        Builder->SetInsertPoint(currentRetBB);
        Symbol* retSymb = symbolTable.getSymbol("retvalue");
        Value* retV = retSymb->getMemRef();
        Value* loadRet = Builder->CreateLoad(retV);
        Builder->CreateRet(loadRet);
        currentRetBB = nullptr;
    }

    symbolTable.pop_scope();

    return BB;
}

void IRGen::visit(IfExprAST* ifexp) {

    BasicBlock* parentBB = Builder->GetInsertBlock();
    BasicBlock* thenBB = nullptr;
    BasicBlock* elseBB = nullptr;
    BasicBlock* contBB = nullptr;
    Function* function = parentBB->getParent();
    std::unique_ptr<ExprBlockAST> ThenBlock = ifexp->getThen();
    std::unique_ptr<ExprBlockAST> ElseBlock = ifexp->getElse();
    std::unique_ptr<ExprAST> Condition = ifexp->getCondition();

    // then and merge blocks
    contBB = BasicBlock::Create(*TheContext, "cont");
    thenBB = visitExpBlock(std::move(ThenBlock), "then", nullptr);

    // we deal with else blocks as optional
    if (ElseBlock) {
        elseBB = visitExpBlock(std::move(ElseBlock), "else", nullptr);

    }

    // branch from then to cont
    function->getBasicBlockList().push_back(contBB);
    Builder->SetInsertPoint(thenBB);
    Instruction& currInst = thenBB->back();
    // dont't put a branch after a return statement.
    if (!currInst.isTerminator()) {
        Builder->CreateBr(contBB);
    }


    // conditional transfer from parent fo else/cont
    Builder->SetInsertPoint(parentBB);

    Value *CondValue = Condition->acceptIRGenVisitor(this);
    CondValue = Builder->CreateFCmpONE(CondValue, ConstantFP::get(*TheContext,
            APFloat(0.0)), "ifcond");

    // if we have an else block/ jump to it
    if (elseBB) {
        Builder->CreateCondBr(CondValue, thenBB, elseBB);
        Builder->SetInsertPoint(elseBB);
        // ensure predecessor/sucessor relation
        Builder->CreateBr(contBB);
    } else {
        Builder->CreateCondBr(CondValue, thenBB, contBB);
    }
    Builder->SetInsertPoint(contBB);
    // NOTE: phi nodes will appear here after mem2reg pass.
}

// VariableExprAST overload
llvm::Value* IRGen::visit(VariableExprAST* node) {

    if (!symbolTable.contains(node->getName())) {
        abort("Variable not found: ", node->getName());
        return nullptr;
    }

    Symbol* symbol = symbolTable.getSymbol(node->getName());

    // at current stage, we can deal only with local variables.
    if (symbol->getStorageType() == StorageType::LOCAL) {
        return Builder->CreateLoad(symbol->getMemRef());
    }
    abort("Not implemented ", "");

    return nullptr;
}

// DoubleNumberExprAST overload.
Value* IRGen::visit(DoubleNumberExprAST* node) {
    return ConstantFP::get(*TheContext, APFloat(node->getVal()));
}

// BinaryExprAST overload.
Value* IRGen::visit(BinaryExprAST* node) {

    Operation Op = node->getOp();
    std::unique_ptr<ExprAST> LHS = node->getLHS();
    std::unique_ptr<ExprAST> RHS = node->getRHS();

    if (Op == Operation::ASSIGN) {
        // Assignment requires the LHS to be an identifier.
        // This assume we're building without RTTI because LLVM builds that way by
        // default.  If you build LLVM with RTTI this can be changed to a
        // dynamic_cast for automatic error checking.
        VariableExprAST *LHSE = static_cast<VariableExprAST *> (LHS.get());
        if (!LHSE) {
            std::cout << "destination of '=' must be a variable\n";
            // return LogErrorV("destination of '=' must be a variable");
            return nullptr;
        }
        // Codegen the RHS.
        Value *Val = RHS->acceptIRGenVisitor(this);
        if (!Val)
            return nullptr;

        // Look up the name.
        Symbol* symb = symbolTable.getSymbol(LHSE->getName());

        if (!symb) {
            abort("Unknown variable name: ", LHSE->getName());
            // return LogErrorV("destination of '=' must be a variable");
            return nullptr;
        }
        Value *Variable = symb->getMemRef();

        Builder->CreateStore(Val, Variable);
        return Val;
    }

    Value *L = LHS->acceptIRGenVisitor(this);
    Value *R = RHS->acceptIRGenVisitor(this);
    if (!L || !R)
        return nullptr;

    switch (Op) {
        case Operation::ADD:
            return Builder->CreateFAdd(L, R, "addtmp");
        case Operation::SUB:
            return Builder->CreateFSub(L, R, "subtmp");
        case Operation::MUL:
            return Builder->CreateFMul(L, R, "multmp");
        case Operation::EQ:
            L = Builder->CreateFCmpUEQ(L, R, "eqtmp");
            // Convert bool 0/1 to double 0.0 or 1.0
            return Builder->CreateUIToFP(L, Type::getDoubleTy(*TheContext), "booltmp"); 
        case Operation::LT:
            L = Builder->CreateFCmpULT(L, R, "cmptmp");
            // Convert bool 0/1 to double 0.0 or 1.0
            return Builder->CreateUIToFP(L, Type::getDoubleTy(*TheContext), "booltmp");
        default:
            abort("Unknown operand: ", std::string(1, Op));
            break;
    }


    return nullptr;
}

// ReturnAST overload
void IRGen::visit(ReturnAST* ifexp) {

    // this methos only saves the Value of the expression in the 
    // "retvalue" alloca and jumps to the return block.
    std::unique_ptr<ExprAST> RHS = ifexp->getExpr();
    Value* Expr = RHS->acceptIRGenVisitor(this);

    Symbol* retSymb = symbolTable.getSymbol("retvalue");

    Builder->CreateStore(Expr, retSymb->getMemRef());
    // the usage of the "currentBB" is quite ugly, must be reworked in
    // the future.
    Builder->CreateBr(currentRetBB);
}

// CallExprAST overload
llvm::Value* IRGen::visit(CallExprAST* node) {
    // Look up the name in the global module table.
    Function *CalleeF = TheModule->getFunction(node->getCalee());
    if (!CalleeF) {
        abort("Unknown function referenced", "");
        return nullptr;
    }
    std::vector<std::unique_ptr < ExprAST>> Args = node->getArgs();
    // If argument mismatch error.
    if (CalleeF->arg_size() != Args.size()) {
        abort("Incorrect # arguments passed", "");
        return nullptr;
    }
    std::vector<Value *> ArgsV;
    for (unsigned i = 0, e = Args.size(); i != e; ++i) {
        ArgsV.push_back(Args[i]->acceptIRGenVisitor(this));
        if (!ArgsV.back())
            return nullptr;
    }

    return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

// Transfer the pointer out of this object.
std::unique_ptr<Module> IRGen::getModule() {
    return std::move(TheModule);
}
