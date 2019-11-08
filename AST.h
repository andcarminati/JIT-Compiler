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


#ifndef AST_H
#define	AST_H
#include <memory>
#include <vector>
#include <deque>
#include <list>
#include <iostream>

#include "IRGen.h"
#include "LangDefs.h"
#include "DebugInfo.h"

class IRGen;

/// BaseAST - base for all other classes

class BaseAST {
public:

    BaseAST(std::unique_ptr<DebugInfo> DI) : DI(std::move(DI)) {
    }

    std::unique_ptr<DebugInfo> getDebugInfo() {
        return std::move(DI);
    }

    virtual ~BaseAST() {
    }

private:
    std::unique_ptr<DebugInfo> DI;
};

/// ExprAST - Base class for all expression nodes.

class PrimaryAST : public BaseAST {
public:

    PrimaryAST(std::unique_ptr<DebugInfo> DI) : BaseAST(std::move(DI)) {
    }

    virtual ~PrimaryAST() {
    }

    virtual void acceptIRGenVisitor(IRGen* visitor) {
    }
};


/// ExprAST - Base class for all expression nodes.

class ExprAST : public BaseAST {
public:

    ExprAST(std::unique_ptr<DebugInfo> DI) : BaseAST(std::move(DI)) {
    }

    virtual ~ExprAST() {
    }

    virtual llvm::Value* acceptIRGenVisitor(IRGen* visitor) {
        printf("Not implemented AST node\n");
        return nullptr;
    }

    virtual bool isSimple() {
        return true;
    }

    virtual bool isUncondTransfer() {
        return false;
    }
};


/// NumberExprAST - Expression class for numeric literals like "1.0".

class RealNumberExprAST : public ExprAST {
    double Val;

public:

    RealNumberExprAST(std::unique_ptr<DebugInfo> DI, double Val) : ExprAST(std::move(DI)), Val(Val) {
    }
    virtual llvm::Value* acceptIRGenVisitor(IRGen* visitor);

    double getVal() {
        return Val;
    }
};

/// NumberExprAST - Expression class for numeric literals like "1.0".

class IntegerNumberExprAST : public ExprAST {
    long long Val;

public:

    IntegerNumberExprAST(std::unique_ptr<DebugInfo> DI, long Val) : ExprAST(std::move(DI)), Val(Val) {
    }
    virtual llvm::Value* acceptIRGenVisitor(IRGen* visitor);

    long long getVal() {
        return Val;
    }
};

/// LocalVarDeclarationExprAST

class LocalVarDeclarationExprAST : public ExprAST {
    std::string Name;
    std::unique_ptr<ExprAST> Initializer;
    VarType Type;
public:

    LocalVarDeclarationExprAST(std::unique_ptr<DebugInfo> DI,
            const std::string &Name,
            std::unique_ptr<ExprAST> Initializer,
            VarType Type) : ExprAST(std::move(DI)), Name(Name), Initializer(std::move(Initializer)), Type(Type) {
    }

    virtual ~LocalVarDeclarationExprAST() {
    }

    virtual llvm::Value* acceptIRGenVisitor(IRGen* visitor);

    std::string& getName() {
        return Name;
    }
};

/// VariableExprAST - Expression class for referencing a variable, like "a".

class VariableExprAST : public ExprAST {
    std::string Name;

public:

    VariableExprAST(std::unique_ptr<DebugInfo> DI, const std::string &Name) : ExprAST(std::move(DI)), Name(Name) {
    }

    virtual ~VariableExprAST() {
    }

    virtual llvm::Value* acceptIRGenVisitor(IRGen* visitor);

    std::string& getName() {
        return Name;
    }
};



/// BinaryExprAST - Expression class for a binary operator.

class BinaryExprAST : public ExprAST {
    Operation Op;
    std::unique_ptr<ExprAST> LHS, RHS;

public:

    BinaryExprAST(std::unique_ptr<DebugInfo> DI, Operation op, std::unique_ptr<ExprAST> LHS,
            std::unique_ptr<ExprAST> RHS)
    : ExprAST(std::move(DI)), Op(op), LHS(std::move(LHS)), RHS(std::move(RHS)) {
    }
    virtual llvm::Value* acceptIRGenVisitor(IRGen* visitor);

    Operation getOp() {
        return Op;
    }

    std::unique_ptr<ExprAST> getLHS() {
        return std::move(LHS);
    }

    std::unique_ptr<ExprAST> getRHS() {
        return std::move(RHS);
    }
};

/// UnaryExprAST - Expression class for a binary operator.

class UnaryExprAST : public ExprAST {
    Operation Op;
    bool prefix;
    std::unique_ptr<ExprAST> LRHS;

public:

    UnaryExprAST(std::unique_ptr<DebugInfo> DI, Operation op, std::unique_ptr<ExprAST> LRHS, bool prefix)
    : ExprAST(std::move(DI)), Op(op), LRHS(std::move(LRHS)), prefix(prefix) {
    }
    virtual llvm::Value* acceptIRGenVisitor(IRGen* visitor);

    Operation getOp() {
        return Op;
    }

    std::unique_ptr<ExprAST> getLRHS() {
        return std::move(LRHS);
    }

    bool isPrefix() {
        return prefix;
    }
};


/// AssignmentAST - Class for assignment nodes. (deprecated - must remove)

class AssignmentAST : public ExprAST {
    std::string VarName;
    std::unique_ptr<ExprAST> RHS;
public:

    AssignmentAST(std::unique_ptr<DebugInfo> DI, const std::string &VarName,
            std::unique_ptr<ExprAST> RHS)
    : ExprAST(std::move(DI)), VarName(VarName), RHS(std::move(RHS)) {
    }
    virtual llvm::Value* acceptIRGenVisitor(IRGen* visitor);
};

/// ReturnAST - class for return nodes.

class ReturnAST : public ExprAST {
    std::unique_ptr<ExprAST> RHS;
public:

    ReturnAST(std::unique_ptr<DebugInfo> DI, std::unique_ptr<ExprAST> RHS) : ExprAST(std::move(DI)), RHS(std::move(RHS)) {
    }

    //virtual ~ReturnAST(){}

    virtual llvm::Value* acceptIRGenVisitor(IRGen* visitor);

    virtual bool isUncondTransfer() {
        return true;
    }

    std::unique_ptr<ExprAST> getExpr() {
        return std::move(RHS);
    }
};



/// CallExprAST - Expression class for function calls.

class CallExprAST : public ExprAST {
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;

public:

    CallExprAST(std::unique_ptr<DebugInfo> DI, const std::string &Callee,
            std::vector<std::unique_ptr<ExprAST>> Args)
    : ExprAST(std::move(DI)), Callee(Callee), Args(std::move(Args)) {
    }
    virtual llvm::Value* acceptIRGenVisitor(IRGen* visitor);

    std::string& getCalee() {
        return Callee;
    }

    std::vector<std::unique_ptr<ExprAST>> getArgs() {
        return std::move(Args);
    }
};

class Arg {
    std::string Name;
    VarType type;

public:

    Arg(std::string Name, VarType type) : Name(Name), type(type) {
    }

    std::string& getName() {
        return Name;
    }

    VarType getType() {
        return type;
    }
};


/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).

class PrototypeAST : public PrimaryAST {
    std::string Name;
    std::vector<Arg> Args;
    VarType returnType;
public:

    PrototypeAST(std::unique_ptr<DebugInfo> DI, VarType returnType, const std::string &name, std::vector<Arg> Args)
    : PrimaryAST(std::move(DI)), returnType(returnType), Name(name), Args(std::move(Args)) {
    }

    void acceptIRGenVisitor(IRGen* visitor);

    VarType getReturnType() {
        return returnType;
    }

    const std::string &getName() const {
        return Name;
    }

    std::vector<Arg> &getArgs() {
        return Args;
    }
};

/// ExprBlockAST - This class represents a sequence of Expressions (block))

class ExprBlockAST : public ExprAST {
    std::list<std::unique_ptr<ExprAST>> Expressions;

public:

    ExprBlockAST(std::unique_ptr<DebugInfo> DI) : ExprAST(std::move(DI)) {
    }

    virtual ~ExprBlockAST() {
    }

    void addExpression(std::unique_ptr<ExprAST> exp) {
        Expressions.push_back(std::move(exp));
    }

    bool empty() {
        return Expressions.empty();
    }

    std::unique_ptr<ExprAST> nextExp() {
        std::unique_ptr<ExprAST> ret = std::move(Expressions.front());
        Expressions.pop_front();
        return ret;
    }
};

/// FunctionAST - This class represents a function definition itself.

class FunctionAST : public PrimaryAST {
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<ExprBlockAST> Body;

public:

    FunctionAST(std::unique_ptr<DebugInfo> DI, std::unique_ptr<PrototypeAST> Proto,
            std::unique_ptr<ExprBlockAST> Body)
    : PrimaryAST(std::move(DI)), Proto(std::move(Proto)), Body(std::move(Body)) {
    }
    void acceptIRGenVisitor(IRGen* visitor);

    const std::string &getName() const {
        return Proto->getName();
    }

    std::unique_ptr<PrototypeAST> getProto() {
        return std::move(Proto);
    }

    std::unique_ptr<ExprBlockAST> getBody() {
        return std::move(Body);
    }
};

/// IfExprAST - Expression class for if/then/else.

class IfExprAST : public ExprAST {
    std::unique_ptr<ExprAST> Cond;
    std::unique_ptr<ExprBlockAST> Then, Else;

public:

    IfExprAST(std::unique_ptr<DebugInfo> DI, std::unique_ptr<ExprAST> Cond, std::unique_ptr<ExprBlockAST> Then,
            std::unique_ptr<ExprBlockAST> Else)
    : ExprAST(std::move(DI)), Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {
    }
    virtual llvm::Value* acceptIRGenVisitor(IRGen* visitor);

    virtual bool isSimple() {
        return false;
    }

    std::unique_ptr<ExprBlockAST> getThen() {
        return std::move(Then);
    }

    std::unique_ptr<ExprBlockAST> getElse() {
        return std::move(Else);
    }

    std::unique_ptr<ExprAST> getCondition() {
        return std::move(Cond);
    }
};


#endif	/* AST_H */

