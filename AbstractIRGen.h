/* 
 * File:   AbstractIRGen.h
 * Author: andreu
 *
 * Created on 19 de Março de 2020, 17:44
 */

#ifndef ABSTRACTIRGEN_H
#define	ABSTRACTIRGEN_H

#include <llvm/IR/Module.h>

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


template<typename T>
class AbstractIRGen {
public:
    virtual void GenFromAST(std::unique_ptr<PrimaryAST<T>> node) = 0;
    virtual void visit(PrototypeAST<T>* node) = 0;
    virtual void visit(FunctionAST<T>* node) = 0;
    virtual void visit(IfExprAST<T>* ifexp) = 0;
    virtual void visit(ReturnAST<T>* ifexp) = 0;
    virtual void visit(ForExprAST<T>* node) = 0;
    virtual void visit(WhileExprAST<T>* node) = 0;
    virtual T visit(VariableExprAST<T>* node) = 0;
    virtual T visit(RealNumberExprAST<T>* node) = 0;
    virtual T visit(IntegerNumberExprAST<T>* node) = 0;
    virtual T visit(BinaryExprAST<T>* node) = 0;
    virtual T visit(UnaryExprAST<T>* node) = 0;
    virtual T visit(CallExprAST<T>* node) = 0;
    virtual T visit(LocalVarDeclarationExprAST<T>* node) = 0;
    virtual std::unique_ptr<llvm::Module> getModule() = 0;

};

#endif	/* ABSTRACTIRGEN_H */

