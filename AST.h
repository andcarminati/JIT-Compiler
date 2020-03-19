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
template<typename T>
class BaseAST {
public:

    BaseAST(std::unique_ptr<DebugInfo>&& DI) : DI(std::move(DI)) {
    }

    std::unique_ptr<DebugInfo> getDebugInfo() {
        return std::move(DI);
    }

    virtual ~BaseAST() = default;

private:
    std::unique_ptr<DebugInfo> DI;
};

/// ExprAST - Base class for all expression nodes.

template<typename T>
class PrimaryAST : public BaseAST<T> {
public:

    PrimaryAST(std::unique_ptr<DebugInfo>&& DI) : BaseAST<T>(std::move(DI)) {
    }

    virtual ~PrimaryAST() {
    }

    virtual void acceptIRGenVisitor(IRGen* visitor) {
    }
};


/// ExprAST - Base class for all expression nodes.

template<typename T>
class ExprAST : public BaseAST<T> {
public:

    ExprAST(std::unique_ptr<DebugInfo>&& DI) : BaseAST<T>(std::move(DI)) {
    }

    virtual ~ExprAST() {
    }

    virtual T acceptIRGenVisitor(IRGen* visitor) {
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

template<typename T>
class RealNumberExprAST : public ExprAST<T> {
    double Val;

public:

    RealNumberExprAST(std::unique_ptr<DebugInfo>&& DI, double Val) : ExprAST<T>(std::move(DI)), Val(Val) {
    }

    T acceptIRGenVisitor(IRGen* visitor) {
        return visitor->visit(this);
    }

    double getVal() {
        return Val;
    }
};

/// NumberExprAST - Expression class for numeric literals like "1.0".

template<typename T>
class IntegerNumberExprAST : public ExprAST<T> {
    long long Val;

public:

    IntegerNumberExprAST(std::unique_ptr<DebugInfo>&& DI, long Val) : ExprAST<T>(std::move(DI)), Val(Val) {
    }

    T acceptIRGenVisitor(IRGen* visitor) {
        return visitor->visit(this);
    }

    long long getVal() {
        return Val;
    }
};

/// LocalVarDeclarationExprAST

template<typename T>
class LocalVarDeclarationExprAST : public ExprAST<T> {
    std::string Name;
    std::unique_ptr<ExprAST<T>> Initializer;
    VarType Type;
public:

    LocalVarDeclarationExprAST(std::unique_ptr<DebugInfo>&& DI,
            const std::string &Name,
            std::unique_ptr<ExprAST<T>>&& Initializer,
            VarType Type) : ExprAST<T>(std::move(DI)), Name(Name), Initializer(std::move(Initializer)), Type(Type) {
    }

    virtual ~LocalVarDeclarationExprAST() {
    }

    T acceptIRGenVisitor(IRGen* visitor) {
        return visitor->visit(this);
    }

    std::string& getName() {
        return Name;
    }

    std::unique_ptr<ExprAST<T>> getInitalizer() {
        return std::move(Initializer);
    }

    VarType getType() {
        return Type;
    }
};

/// VariableExprAST - Expression class for referencing a variable, like "a".

template<typename T>
class VariableExprAST : public ExprAST<T> {
    std::string Name;

public:

    VariableExprAST(std::unique_ptr<DebugInfo>&& DI, const std::string &Name) : ExprAST<T>(std::move(DI)), Name(Name) {
    }

    virtual ~VariableExprAST() {
    }

    T acceptIRGenVisitor(IRGen* visitor) {
        return visitor->visit(this);
    }

    std::string& getName() {
        return Name;
    }
};



/// BinaryExprAST - Expression class for a binary operator.

template<typename T>
class BinaryExprAST : public ExprAST<T> {
    Operation Op;
    std::unique_ptr<ExprAST<T>> LHS, RHS;

public:

    BinaryExprAST(std::unique_ptr<DebugInfo>&& DI, Operation op, std::unique_ptr<ExprAST<T>>&& LHS,
            std::unique_ptr<ExprAST<T>>&& RHS)
    : ExprAST<T>(std::move(DI)), Op(op), LHS(std::move(LHS)), RHS(std::move(RHS)) {
    }

    T acceptIRGenVisitor(IRGen* visitor) {
        return visitor->visit(this);
    }

    Operation getOp() {
        return Op;
    }

    std::unique_ptr<ExprAST<T>> getLHS() {
        return std::move(LHS);
    }

    std::unique_ptr<ExprAST<T>> getRHS() {
        return std::move(RHS);
    }
};

/// UnaryExprAST - Expression class for a binary operator.

template<typename T>
class UnaryExprAST : public ExprAST<T> {
    Operation Op;
    bool prefix;
    std::unique_ptr<ExprAST<T>> LRHS;

public:

    UnaryExprAST(std::unique_ptr<DebugInfo>&& DI, Operation op, std::unique_ptr<ExprAST<T>>&& LRHS, bool prefix)
    : ExprAST<T>(std::move(DI)), Op(op), LRHS(std::move(LRHS)), prefix(prefix) {
    }

    T acceptIRGenVisitor(IRGen* visitor) {
        return visitor->visit(this);
    }

    Operation getOp() {
        return Op;
    }

    std::unique_ptr<ExprAST<T>> getLRHS() {
        return std::move(LRHS);
    }

    bool isPrefix() {
        return prefix;
    }
};


/// AssignmentAST - Class for assignment nodes. (deprecated - must remove)

template<typename T>
class AssignmentAST : public ExprAST<T> {
    std::string VarName;
    std::unique_ptr<ExprAST<T>> RHS;
public:

    AssignmentAST(std::unique_ptr<DebugInfo>&& DI, const std::string &VarName,
            std::unique_ptr<ExprAST<T>>&& RHS)
    : ExprAST<T>(std::move(DI)), VarName(VarName), RHS(std::move(RHS)) {
    }
    virtual T acceptIRGenVisitor(IRGen* visitor);
};

/// ReturnAST - class for return nodes.

template<typename T>
class ReturnAST : public ExprAST<T> {
    std::unique_ptr<ExprAST<T>> RHS;
public:

    ReturnAST(std::unique_ptr<DebugInfo>&& DI, std::unique_ptr<ExprAST<T>>&& RHS) :
    ExprAST<T>(std::move(DI)), RHS(std::move(RHS)) {
    }

    T acceptIRGenVisitor(IRGen* visitor) {
        visitor->visit(this);
        return nullptr;
    }

    virtual bool isUncondTransfer() {
        return true;
    }

    std::unique_ptr<ExprAST<T>> getExpr() {
        return std::move(RHS);
    }
};



/// CallExprAST - Expression class for function calls.

template<typename T>
class CallExprAST : public ExprAST<T> {
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST<T>>> Args;

public:

    CallExprAST(std::unique_ptr<DebugInfo>&& DI, const std::string &Callee,
            std::vector<std::unique_ptr<ExprAST<T>>>&& Args)
    : ExprAST<T>(std::move(DI)), Callee(Callee), Args(std::move(Args)) {
    }

    T acceptIRGenVisitor(IRGen* visitor) {
        return visitor->visit(this);
    }

    std::string& getCalee() {
        return Callee;
    }

    std::vector<std::unique_ptr<ExprAST<T>>> getArgs() {
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

template<typename T>
class PrototypeAST : public PrimaryAST<T> {
    std::string Name;
    std::vector<Arg> Args;
    VarType returnType;
public:

    PrototypeAST(std::unique_ptr<DebugInfo>&& DI, VarType returnType, const std::string &name, std::vector<Arg>&& Args)
    : PrimaryAST<T>(std::move(DI)), returnType(returnType), Name(name), Args(std::move(Args)) {
    }

    void acceptIRGenVisitor(IRGen* visitor) {
        visitor->visit(this);
    }

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

template<typename T>
class ExprBlockAST : public ExprAST<T> {
    std::list<std::unique_ptr<ExprAST<T>>> Expressions;

public:

    ExprBlockAST(std::unique_ptr<DebugInfo>&& DI) : ExprAST<T>(std::move(DI)) {
    }

    virtual ~ExprBlockAST() {
    }

    void addExpression(std::unique_ptr<ExprAST<T>>&& exp) {
        Expressions.push_back(std::move(exp));
    }

    bool empty() {
        return Expressions.empty();
    }

    std::unique_ptr<ExprAST<T>> nextExp() {
        std::unique_ptr<ExprAST<T>> ret = std::move(Expressions.front());
        Expressions.pop_front();
        return ret;
    }

    bool hasReturn() {
        ReturnAST<T> *ret = dynamic_cast<ReturnAST<T> *> (Expressions.back().get());
        if (!ret) {
            return false;
        }
        return true;
    }
};

/// FunctionAST - This class represents a function definition itself.

template<typename T>
class FunctionAST : public PrimaryAST<T> {
    std::unique_ptr<PrototypeAST<T>> Proto;
    std::unique_ptr<ExprBlockAST<T>> Body;

public:

    FunctionAST(std::unique_ptr<DebugInfo>&& DI, std::unique_ptr<PrototypeAST<T>>&& Proto,
            std::unique_ptr<ExprBlockAST<T>>&& Body)
    : PrimaryAST<T>(std::move(DI)), Proto(std::move(Proto)), Body(std::move(Body)) {
    }

    void acceptIRGenVisitor(IRGen* visitor) {
        visitor->visit(this);
    }

    const std::string &getName() const {
        return Proto->getName();
    }

    std::unique_ptr<PrototypeAST<T>> getProto() {
        return std::move(Proto);
    }

    std::unique_ptr<ExprBlockAST<T>> getBody() {
        return std::move(Body);
    }
};

/// IfExprAST - Expression class for if/then/else.

template<typename T>
class IfExprAST : public ExprAST<T> {
    std::unique_ptr<ExprAST<T>> Cond;
    std::unique_ptr<ExprBlockAST<T>> Then, Else;

public:

    IfExprAST(std::unique_ptr<DebugInfo>&& DI, std::unique_ptr<ExprAST<T>>&& Cond, std::unique_ptr<ExprBlockAST<T>>&& Then,
            std::unique_ptr<ExprBlockAST<T>>&& Else)
    : ExprAST<T>(std::move(DI)), Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {
    }

    T acceptIRGenVisitor(IRGen* visitor) {
        visitor->visit(this);
        return nullptr;
    }

    virtual bool isSimple() {
        return false;
    }

    std::unique_ptr<ExprBlockAST<T>> getThen() {
        return std::move(Then);
    }

    std::unique_ptr<ExprBlockAST<T>> getElse() {
        return std::move(Else);
    }

    std::unique_ptr<ExprAST<T>> getCondition() {
        return std::move(Cond);
    }
};

/// ForExprAST - Expression class for for/in.

template<typename T>
class ForExprAST : public ExprAST<T> {
    std::unique_ptr<ExprAST<T>> Start, End, Cond;
    std::unique_ptr<ExprBlockAST<T>> Body;

public:

    ForExprAST(std::unique_ptr<DebugInfo>&& DI, std::unique_ptr<ExprAST<T>>&& Start,
            std::unique_ptr<ExprAST<T>>&& End, std::unique_ptr<ExprAST<T>>&& Cond,
            std::unique_ptr<ExprBlockAST<T>>&& Body)
    : ExprAST<T>(std::move(DI)), Start(std::move(Start)), End(std::move(End)),
    Cond(std::move(Cond)), Body(std::move(Body)) {
    }

    T acceptIRGenVisitor(IRGen* visitor) {
        visitor->visit(this);
        return nullptr;
    }

    virtual bool isSimple() {
        return false;
    }

    std::unique_ptr<ExprAST<T>> getStart() {
        return std::move(Start);
    }

    std::unique_ptr<ExprAST<T>> getEnd() {
        return std::move(End);
    }

    std::unique_ptr<ExprAST<T>> getCond() {
        return std::move(Cond);
    }

    std::unique_ptr<ExprBlockAST<T>> getBody() {
        return std::move(Body);
    }
};

/// WhileExprAST - Expression class for for/in.
template<typename T>
class WhileExprAST : public ExprAST<T> {
    std::unique_ptr<ExprAST<T>> Cond;
    std::unique_ptr<ExprBlockAST<T>> Body;

public:

    WhileExprAST(std::unique_ptr<DebugInfo>&& DI, std::unique_ptr<ExprAST<T>>&& Cond,
            std::unique_ptr<ExprBlockAST<T>>&& Body)
    : ExprAST<T>(std::move(DI)), Cond(std::move(Cond)), Body(std::move(Body)) {
    }

    T acceptIRGenVisitor(IRGen* visitor) {
        visitor->visit(this);
        return nullptr;
    }

    virtual bool isSimple() {
        return false;
    }

    std::unique_ptr<ExprAST<T>> getCond() {
        return std::move(Cond);
    }

    std::unique_ptr<ExprBlockAST<T>> getBody() {
        return std::move(Body);
    }
};

#endif	/* AST_H */

