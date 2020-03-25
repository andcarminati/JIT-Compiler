
#include "AST.h"


#include <stdlib.h>

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
#include <llvm/ADT/APSInt.h>
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
#include "LLVMIRGen.h"
#include "Lexer.h"
#include <iostream>
#include <vector>

using namespace llvm;

static void abort(const char *Str, std::string loc) {
    printf("Code generator fatal: %s -> %s\n", Str, loc.c_str());
    exit(-1);
}

static void abort(const char *Str, std::string msg, std::string loc) {
    printf("Code generator fatal: %s (%s) -> %s\n", Str, msg.c_str(), loc.c_str());
    exit(-1);
}

static void LogError(const char *Str, std::string loc) {
    fprintf(stderr, "Compiler error (code generator): %s -> %s\n", Str, loc.c_str());
}

static Type* convertType(VarType t, LLVMContext* context) {

    switch (t) {
        case REAL:
            return Type::getDoubleTy(*context);
            break;
        case INTEGER:
            return Type::getInt64Ty(*context);
            break;
        case NONE:
            return Type::getVoidTy(*context);
            break;
        default:
            return nullptr;
    }
    return nullptr;
}

LLVMIRGen::LLVMIRGen(llvm::LLVMContext* TheContext) : AbstractIRGen<LLVMValue>() {

    this->TheContext = TheContext;
    //Builder = std::make_unique<IRBuilder<>>(IRBuilder<>(*TheContext));
    Builder = std::unique_ptr<IRBuilder<>>(new llvm::IRBuilder<>(*TheContext));

    TheModule = std::make_unique<llvm::Module>("my new lang", *TheContext);
    //TheFPM = 
}

// Entry point of the IR Generator. Using a visitor design pattern.

void LLVMIRGen::GenFromAST(std::unique_ptr<PrimaryAST<LLVMValue>> node) {
    node->acceptIRGenVisitor(this);
}

// Prorotype overload.

void LLVMIRGen::visit(PrototypeAST<LLVMValue>* proto) {
    auto *TheFunction = visitFunctionPrototypeImpl(proto);
}
// FunctionAST overload.

void LLVMIRGen::visit(FunctionAST<LLVMValue>* node) {
    // calls the real implementation
    auto *TheFunction = visitFunctionImpl(node);
    if (TheFunction) {
        //TheFunction->print(errs());
    }
}

// Internal method for prototype function generation

Function* LLVMIRGen::visitFunctionPrototypeImpl(PrototypeAST<LLVMValue>* node) {

    std::vector<Arg>& Args = node->getArgs();
    const std::string& Name = node->getName();
    auto DI = node->getDebugInfo();
    // Make the function type:  double(double,double) etc.
    //std::vector<Type *> Doubles(Args.size(), Type::getDoubleTy(*TheContext));

    std::vector<Type *> ArgTypes;
    for (auto &arg : Args) {
        if (arg.getType() == REAL) {
            ArgTypes.push_back(Type::getDoubleTy(*TheContext));
        } else if (arg.getType() == INTEGER) {
            ArgTypes.push_back(Type::getInt64Ty(*TheContext));
        } else {
            std::cout << "unknown type in " << DI->getInfo() << std::endl;
        }
    }

    FunctionType *FT =
            FunctionType::get(convertType(node->getReturnType(), TheContext), ArgTypes, false);

    Function *F =
            Function::Create(FT, Function::ExternalLinkage, Name, TheModule.get());

    // Set names for all arguments.
    unsigned Idx = 0;
    for (auto &Arg : F->args()) {
        Arg.setName(Args[Idx++].getName());
    }

    return F;
}

// Private method for the real job. 

Function* LLVMIRGen::visitFunctionImpl(FunctionAST<LLVMValue>* node) {
    // First, check for an existing function from a previous 'extern' declaration.
    const std::string& Name = node->getName();
    Function *TheFunction = TheModule->getFunction(Name);
    auto DI = node->getDebugInfo();
    if (!TheFunction) {
        // generator must owns the prototype pointer  
        std::unique_ptr<PrototypeAST < LLVMValue>> proto = node->getProto();
        // gen code for prototype
        proto->acceptIRGenVisitor(this);
        TheFunction = TheModule->getFunction(Name);
    }
    if (!TheFunction) {
        return nullptr;
    }

    if (!TheFunction->empty()) {
        //rever
        abort("Function already defined", Name, DI->getInfo());
    }

    // Create a the first basic block and generate code.
    BasicBlock *BB = visitExpBlock(std::move(node->getBody()), "entry", TheFunction);

    //Check for return
    //BasicBlock* lastBB = &TheFunction->back();
    //lastBB->print(errs());
    //if(!lastBB->getTerminator()){
    //    abort("Last return statement is expected at the end function", Name, DI->getInfo());
    //}

    return TheFunction;
}


// Generate allocas for every function parameter plus the return value. We can optimize
// these allcoas with men2reg pass.

void LLVMIRGen::allocSpaceForParams(Function* function, BasicBlock* BB) {

    std::list<AllocaInst*> allocas;

    IRBuilder<> TmpB(&function->getEntryBlock(),
            function->getEntryBlock().begin());

    // create Allocas
    for (auto &Arg : function->args()) {
        AllocaInst* Alloca = TmpB.CreateAlloca(Arg.getType(), 0,
                Arg.getName());
        allocas.push_back(Alloca);
    }

    // create Alloca for return value if necessary;
    if (function->getReturnType() != Type::getVoidTy(*TheContext)) {
        AllocaInst* Alloca = TmpB.CreateAlloca(function->getReturnType(), 0,
                "retvalue");

        symbolTable.insertSymbol("retvalue", StorageType::LOCAL, Alloca);
    }

    // Store params in the allocas
    for (auto &Arg : function->args()) {
        AllocaInst* Alloca = allocas.front();
        allocas.pop_front();
        Builder->CreateStore(&Arg, Alloca);
        // populate the symbol table
        symbolTable.insertSymbol(Arg.getName().str(), StorageType::LOCAL, Alloca);
    }


}

// Generate alloca for specific var

Value* LLVMIRGen::allocLocalVar(Function* function, std::string& name, VarType type, DebugInfo* DI) {

    IRBuilder<> TmpB(&function->getEntryBlock(),
            function->getEntryBlock().begin());

    AllocaInst* Alloca = TmpB.CreateAlloca(convertType(type, TheContext), 0,
            name);

    if (symbolTable.contains(name)) {
        abort("Variable already declared", name, DI->getInfo());
    }

    symbolTable.insertSymbol(name, StorageType::LOCAL, Alloca);

    return Alloca;
}

// Private method for expression block code generation.

BasicBlock* LLVMIRGen::visitExpBlock(std::unique_ptr<ExprBlockAST<LLVMValue>> block,
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
        std::unique_ptr<ExprAST < LLVMValue>> expr = block->nextExp();
        expr->acceptIRGenVisitor(this);
    }

    if (mainBlock) {
        // create the final return statement.
        currentRetBB->moveAfter(Builder->GetInsertBlock());
        Builder->SetInsertPoint(currentRetBB);

        // only return a value if a function has a returnn type diferent from void
        if (function->getReturnType() == Type::getVoidTy(*TheContext)) {
            Builder->CreateRetVoid();
        } else {
            Symbol* retSymb = symbolTable.getSymbol("retvalue");
            Value* retV = retSymb->getMemRef();
            Value* loadRet = Builder->CreateLoad(retV);

            Builder->CreateRet(loadRet);
        }

        currentRetBB = nullptr;
    }

    symbolTable.pop_scope();

    return BB;
}

void LLVMIRGen::visit(IfExprAST<LLVMValue>* ifexp) {

    BasicBlock* parentBB = Builder->GetInsertBlock();
    BasicBlock* thenBB = nullptr;
    BasicBlock* elseBB = nullptr;
    BasicBlock* contBB = nullptr;
    Function* function = parentBB->getParent();
    std::unique_ptr<ExprBlockAST < LLVMValue>> ThenBlock = ifexp->getThen();
    std::unique_ptr<ExprBlockAST < LLVMValue>> ElseBlock = ifexp->getElse();
    std::unique_ptr<ExprAST < LLVMValue>> Condition = ifexp->getCondition();

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
    //CondValue = Builder->CreateFCmpONE(CondValue, ConstantFP::get(*TheContext,
    //       APFloat(0.0)), "ifcond");

    // if we have an else block/ jump to it
    if (elseBB) {
        Builder->CreateCondBr(CondValue, thenBB, elseBB);
        Builder->SetInsertPoint(elseBB);
        // ensure predecessor/sucessor relation
        Instruction* last = elseBB->getTerminator();

        if (!last) {
            Builder->CreateBr(contBB);
        }

    } else {
        Builder->CreateCondBr(CondValue, thenBB, contBB);
    }

    Builder->SetInsertPoint(contBB);

    // NOTE: phi nodes will appear here after mem2reg pass.
}

// VariableExprAST overload

llvm::Value* LLVMIRGen::visit(VariableExprAST<LLVMValue>* node) {

    auto DI = node->getDebugInfo();
    if (!symbolTable.contains(node->getName())) {
        abort("Variable not found", node->getName(), DI->getInfo());
        return nullptr;
    }

    Symbol* symbol = symbolTable.getSymbol(node->getName());

    // at current stage, we can deal only with local variables.
    if (symbol->getStorageType() == StorageType::LOCAL) {
        return Builder->CreateLoad(symbol->getMemRef());
    }
    abort("Not implemented ", DI->getInfo());

    return nullptr;
}

// RealNumberExprAST overload.

Value* LLVMIRGen::visit(RealNumberExprAST<LLVMValue>* node) {
    return ConstantFP::get(*TheContext, APFloat(node->getVal()));
}

// IntegerNumberExprAST overload.

Value* LLVMIRGen::visit(IntegerNumberExprAST<LLVMValue>* node) {
    return ConstantInt::get(*TheContext, APSInt::get(node->getVal()));
}


// BinaryExprAST overload.

Value* LLVMIRGen::visit(BinaryExprAST<LLVMValue>* node) {

    auto DI = node->getDebugInfo();
    Operation Op = node->getOp();
    std::unique_ptr<ExprAST < LLVMValue>> LHS = node->getLHS();
    std::unique_ptr<ExprAST < LLVMValue>> RHS = node->getRHS();

    if (Op == Operation::ASSIGN) {
        // Assignment requires the LHS to be an identifier.
        // This assume we're building without RTTI because LLVM builds that way by
        // default.  If you build LLVM with RTTI this can be changed to a
        // dynamic_cast for automatic error checking.
        VariableExprAST<LLVMValue> *LHSE = static_cast<VariableExprAST<LLVMValue> *> (LHS.get());
        if (!LHSE) {
            LogError("destination of '=' must be a variable", DI->getInfo());
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
            abort("Unknown variable name", LHSE->getName(), DI->getInfo());
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

    if (L->getType() != R->getType()) {
        abort("Type incompatibility between operands", DI->getInfo());
        return nullptr;
    }

    switch (Op) {
        case Operation::ADD:
            if (L->getType() == Type::getDoubleTy(*TheContext)) {
                return Builder->CreateFAdd(L, R, "addtmp");
            } else if (L->getType() == Type::getInt64Ty(*TheContext)) {
                return Builder->CreateAdd(L, R, "addtmp");
            } else {
                abort("Unimplemented operand type", DI->getInfo());
                return nullptr;
            }

        case Operation::SUB:
            if (L->getType() == Type::getDoubleTy(*TheContext)) {
                return Builder->CreateFSub(L, R, "subtmp");
            } else if (L->getType() == Type::getInt64Ty(*TheContext)) {
                return Builder->CreateSub(L, R, "subtmp");
            } else {
                abort("Unimplemented operand type", DI->getInfo());
                return nullptr;
            }

        case Operation::MUL:

            if (L->getType() == Type::getDoubleTy(*TheContext)) {
                return Builder->CreateFMul(L, R, "multmp");
            } else if (L->getType() == Type::getInt64Ty(*TheContext)) {
                return Builder->CreateMul(L, R, "multmp");
            } else {
                abort("Unimplemented operand type", DI->getInfo());
                return nullptr;
            }

        case Operation::EQ:
            if (L->getType() == Type::getDoubleTy(*TheContext)) {
                L = Builder->CreateFCmpUEQ(L, R, "eqtmp");
                // Convert bool 0/1 to double 0.0 or 1.0
                //return Builder->CreateUIToFP(L, Type::getDoubleTy(*TheContext), "booltmp");
                return L;
            } else if (L->getType() == Type::getInt64Ty(*TheContext)) {
                L = Builder->CreateICmpEQ(L, R, "eqtmp");
                return L;
            } else {
                abort("Unimplemented operand type", DI->getInfo());
                return nullptr;
            }


        case Operation::LT:

            if (L->getType() == Type::getDoubleTy(*TheContext)) {
                L = Builder->CreateFCmpULT(L, R, "cmptmp");
                // Convert bool 0/1 to double 0.0 or 1.0
                //return Builder->CreateUIToFP(L, Type::getDoubleTy(*TheContext), "booltmp");
                return L;
            } else if (L->getType() == Type::getInt64Ty(*TheContext)) {
                L = Builder->CreateICmpSLT(L, R, "eqtmp");
                return L;
            } else {
                abort("Unimplemented operand type", DI->getInfo());
                return nullptr;
            }

        default:
            abort("Unknown operand: ", std::string(1, Op));
            break;
    }


    return nullptr;
}

Value* LLVMIRGen::visit(UnaryExprAST<LLVMValue>* node) {

    auto DI = node->getDebugInfo();
    std::unique_ptr<ExprAST < LLVMValue>> LRHS = node->getLRHS();
    Operation op = node->getOp();
    bool isPrefixed = node->isPrefix();
    Value* result;

    VariableExprAST<LLVMValue> *LHSE = static_cast<VariableExprAST<LLVMValue> *> (LRHS.get());
    if (!LHSE) {
        abort("unary operand must be a variable", DI->getInfo());
        // return LogErrorV("destination of '=' must be a variable");
        return nullptr;
    }

    Value* var = LHSE->acceptIRGenVisitor(this);
    Symbol* sym = symbolTable.getSymbol(LHSE->getName());
    if (!sym) {
        abort("Unknown symbol: ", LHSE->getName());
        return nullptr;
    }

    switch (op) {
        case Operation::INC:
            if (var->getType() == Type::getDoubleTy(*TheContext)) {
                result = Builder->CreateFAdd(var, ConstantFP::get(*TheContext,
                        APFloat(1.0)), "inctmp");
            } else {
                result = Builder->CreateAdd(var, ConstantInt::get(*TheContext, APSInt::get(1)), "inctmp");
            }

            break;
        case Operation::DEC:
            if (var->getType() == Type::getDoubleTy(*TheContext)) {
                result = Builder->CreateFSub(var, ConstantFP::get(*TheContext,
                        APFloat(1.0)), "dectmp");
            } else {
                result = Builder->CreateSub(var, ConstantInt::get(*TheContext, APSInt::get(1)), "inctmp");
            }

            break;
        default:
            abort("Unimplemented unary operator", DI->getInfo());
            return nullptr;
    }
    Builder->CreateStore(result, sym->getMemRef());

    if (!isPrefixed) {
        result = var;
    }

    return result;
}


// ReturnAST overload

void LLVMIRGen::visit(ReturnAST<LLVMValue>* ifexp) {

    auto DI = ifexp->getDebugInfo();
    std::unique_ptr<ExprAST < LLVMValue>> RHS = ifexp->getExpr();
    // get the parent function for type verification
    Function* function = currentRetBB->getParent();

    /// we have a void return
    if (function->getReturnType() == Type::getVoidTy(*TheContext)) {
        if (RHS) {
            abort("Void functions cannot return a value", DI->getInfo());
        }
    } else {
        // for non void functions this method only saves the Value of the expression in the 
        // "retvalue" alloca and jumps to the return block.
        if (!RHS) {
            abort("A non void function must return a value", DI->getInfo());
        }

        Value* Expr = RHS->acceptIRGenVisitor(this);

        if (function->getReturnType() != Expr->getType()) {
            abort("Type incompatibility between returned expression and function's return type", DI->getInfo());
        }

        Symbol* retSymb = symbolTable.getSymbol("retvalue");
        Builder->CreateStore(Expr, retSymb->getMemRef());
    }

    // the usage of the "currentBB" is quite ugly, must be reworked in
    // the future.
    Builder->CreateBr(currentRetBB);
}

// CallExprAST overload

llvm::Value* LLVMIRGen::visit(CallExprAST<LLVMValue>* node) {
    // Look up the name in the global module table.
    auto DI = node->getDebugInfo();
    Function *CalleeF = TheModule->getFunction(node->getCalee());
    if (!CalleeF) {
        abort("Unknown function referenced", node->getCalee(), DI->getInfo());
        return nullptr;
    }
    std::vector<std::unique_ptr < ExprAST < LLVMValue>>> Args = node->getArgs();
    // If argument mismatch error.
    if (CalleeF->arg_size() != Args.size()) {
        abort("Incorrect # arguments passed", DI->getInfo());
        return nullptr;
    }
    //check parameter compatibility and construct the vector
    Function::arg_iterator I = CalleeF->arg_begin();
    std::vector<Value *> ArgsV;
    for (unsigned i = 0, e = Args.size(); i != e; ++i) {
        // real argument
        Argument& ActualArg = *I;
        Value* Arg = Args[i]->acceptIRGenVisitor(this);

        //check types
        if (ActualArg.getType() != Arg->getType()) {
            abort("Type incompatibility between provided and expected arguments", DI->getInfo());
            return nullptr;
        }
        I++;
        ArgsV.push_back(Arg);
        if (!ArgsV.back())
            return nullptr;
    }
    // if is a void function, do not save the return;
    if (CalleeF->getReturnType() == Type::getVoidTy(*TheContext)) {
        Builder->CreateCall(CalleeF, ArgsV);
        return nullptr;
    }
    return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

// LocalVarDeclarationExprAST overload

llvm::Value* LLVMIRGen::visit(LocalVarDeclarationExprAST<LLVMValue>* node) {

    auto DI = node->getDebugInfo();
    std::string name = node->getName();
    std::unique_ptr<ExprAST < LLVMValue>> Exp = node->getInitalizer();
    VarType type = node->getType();
    Value* allocated = allocLocalVar(Builder->GetInsertBlock()->getParent(), name, type, DI.get());

    Value* initializer = Exp->acceptIRGenVisitor(this);

    if (initializer) {
        if (convertType(type, TheContext) != initializer->getType()) {
            abort("Type incompatibility between variable and its initializer", DI->getInfo());
        }
        Builder->CreateStore(initializer, allocated);
    }


    return nullptr;
}

// ForExprAST overload

void LLVMIRGen::visit(ForExprAST<LLVMValue>* forExpr) {

    BasicBlock* parentBB = Builder->GetInsertBlock();
    BasicBlock* BodyBB = nullptr;
    BasicBlock* LastBodyBB = nullptr;
    BasicBlock* HeaderBB = nullptr;
    BasicBlock* ContBB = nullptr;

    Function* function = parentBB->getParent();
    std::unique_ptr<ExprBlockAST < LLVMValue>> Block = forExpr->getBody();
    std::unique_ptr<ExprAST < LLVMValue>> Start = forExpr->getStart();
    std::unique_ptr<ExprAST < LLVMValue>> Cond = forExpr->getCond();
    std::unique_ptr<ExprAST < LLVMValue>> End = forExpr->getEnd();

    // scope for declared variables
    symbolTable.push_scope();

    // for initialization
    Value* StartValue = Start->acceptIRGenVisitor(this);

    if (Block) {
        BodyBB = visitExpBlock(std::move(Block), "forBody", nullptr);
        // get and updated vertion of the basic block. If we have inserted more blocks
        // we need to continue from tha last block.
        LastBodyBB = &function->getBasicBlockList().back();
        // put end statement at the end of the body

        Builder->SetInsertPoint(LastBodyBB);
        // put end statement at the end of the body
        //check if the last block is empty, if yes we can gen End
        if (End && LastBodyBB->size() == 0) {
            Value* EndValue = End->acceptIRGenVisitor(this);
        } else {
            // this basic block is not empty, we can check the last instruction
            // dont't put a branch after a return statement
            Instruction& currInst = LastBodyBB->back();
            if (End && !currInst.isTerminator()) {
                Value* EndValue = End->acceptIRGenVisitor(this);
            }
        }
    } else {
        // empty BB
        BodyBB = BasicBlock::Create(*TheContext, "forBody");
        function->getBasicBlockList().push_back(BodyBB);
        Builder->SetInsertPoint(BodyBB);
        if (End) {
            Value* EndValue = End->acceptIRGenVisitor(this);
        }
    }

    // create for header
    HeaderBB = BasicBlock::Create(*TheContext, "forHeader");
    function->getBasicBlockList().push_back(HeaderBB);

    // branch from parent to header
    Builder->SetInsertPoint(parentBB);
    Builder->CreateBr(HeaderBB);

    // create for cont
    ContBB = BasicBlock::Create(*TheContext, "forCont");
    function->getBasicBlockList().push_back(ContBB);

    Builder->SetInsertPoint(HeaderBB);
    Value* cond;
    if (Cond) {
        // we have a condition
        cond = Cond->acceptIRGenVisitor(this);

    } else {
        //without a condiction we have an infinite loop
        // branch from header to body
        cond = ConstantInt::get(*TheContext, APInt(1, 1, false));
    }
    Builder->CreateCondBr(cond, BodyBB, ContBB);

    // branch from body to header
    Instruction& currInst = LastBodyBB->back();
    // dont't put a branch after a return statement
    if (!currInst.isTerminator()) {
        Builder->SetInsertPoint(LastBodyBB);
        Builder->CreateBr(HeaderBB);
    }

    Builder->SetInsertPoint(ContBB);
    symbolTable.pop_scope();

}


// WhileExprAST overload

void LLVMIRGen::visit(WhileExprAST<LLVMValue>* forExpr) {

    BasicBlock* parentBB = Builder->GetInsertBlock();
    BasicBlock* BodyBB = nullptr;
    BasicBlock* LastBodyBB = nullptr;
    BasicBlock* HeaderBB = nullptr;
    BasicBlock* ContBB = nullptr;

    Function* function = parentBB->getParent();
    std::unique_ptr<ExprBlockAST < LLVMValue>> Block = forExpr->getBody();
    std::unique_ptr<ExprAST < LLVMValue>> Cond = forExpr->getCond();

    if (Block) {
        BodyBB = visitExpBlock(std::move(Block), "whileBody", nullptr);
        LastBodyBB = &function->getBasicBlockList().back();
    } else {
        // empty BB
        BodyBB = BasicBlock::Create(*TheContext, "whileBody");
        function->getBasicBlockList().push_back(BodyBB);
        Builder->SetInsertPoint(BodyBB);
    }

    // create while header
    HeaderBB = BasicBlock::Create(*TheContext, "forHeader");
    function->getBasicBlockList().push_back(HeaderBB);

    // branch from parent to header
    Builder->SetInsertPoint(parentBB);
    Builder->CreateBr(HeaderBB);


    // create while cont
    ContBB = BasicBlock::Create(*TheContext, "whileCont");
    function->getBasicBlockList().push_back(ContBB);

    Builder->SetInsertPoint(HeaderBB);
    Value* cond = Cond->acceptIRGenVisitor(this);

    Builder->CreateCondBr(cond, BodyBB, ContBB);
    // branch from body to header
    Instruction& currInst = LastBodyBB->back();
    // dont't put a branch after a return statement
    if (!currInst.isTerminator()) {
        Builder->SetInsertPoint(LastBodyBB);
        Builder->CreateBr(HeaderBB);
    }

    Builder->SetInsertPoint(ContBB);
}


// Transfer the pointer out of this object.

std::unique_ptr<Module> LLVMIRGen::getModule() {
    return std::move(TheModule);
}
