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



#ifndef PARSER_H
#define	PARSER_H
#include <memory>
#include "Lexer.h"
#include "AST.h"

static void abort(std::string str, int line) {
    std::cout << str << " at line: " << line << std::endl;
    exit(-1);
}

template <class T>
static std::unique_ptr<T> LogError(const char *Str, std::string loc) {
    fprintf(stderr, "Compiler error (parser): %s -> %s\n", Str, loc.c_str());
    return nullptr;
}

static void LogWarn(const char *Str, std::string loc) {
    fprintf(stderr, "Compiler warning (parser): %s -> %s\n", Str, loc.c_str());
}

static VarType convertType(std::string& type) {

    VarType t;

    if (type == "real") {
        t = REAL;
    } else if (type == "integer") {
        t = INTEGER;
    } else if (type == "none") {
        t = NONE;
    } else {
        t = UNIMPLEMENTED;
    }
    return t;
}

static Arg createArg(std::string& type, std::string& name) {

    VarType t = convertType(type);

    if (t == UNIMPLEMENTED) {
        std::cout << "uninplemented type\n";
        t = REAL;
    }

    return Arg(name, t);

}

template<typename T>
class Parser {
public:

    Parser(std::unique_ptr<Lexer>&& lexer) : lexer(std::move(lexer)) {
    }
    //Parser(const Parser& orig) { std::cout << "teste";};
    //virtual ~Parser() {};

    std::unique_ptr<PrimaryAST<T>> nextConstruct() {
        //while (lexer->getCurrentToken() == ';') {
        //lexer->getNextToken();
        //}

        switch (lexer->getCurrentToken()) {
            case tok_eof:
                return nullptr;
                break;
            case tok_function:
                return ParseDefinition();
                break;
            case tok_extern:
                lexer->getNextToken();
                return ParsePrototype(true);
                break;
            default:
                //HandleTopLevelExpression();
                abort("Unknown token: " + lexer->getIdentifierStr(), 0);
                break;
        }
        return nullptr;
    }

    bool hasFail() {
        return failed;
    }
private:
    std::unique_ptr<Lexer> lexer;

    std::unique_ptr<DebugInfo> genDebugInfo() {
        return std::make_unique<DebugInfo>(DebugInfo(lexer->GetTokLine(), lexer->GetTokCol(), lexer->getFileName(), lexer->getLineStr()));

    }

    /// definition ::= 'def' prototype expression

    std::unique_ptr<FunctionAST<T>> ParseDefinition() {
        lexer->getNextToken(); // eat def.
        auto DI = genDebugInfo();
        auto Proto = ParsePrototype(false);
        if (!Proto) return nullptr;

        if (lexer->getCurrentToken() != '{') {
            fail();
            return LogError<FunctionAST < T >> ("Expected { in function body start",
                    DI->getInfo());
        }

        std::unique_ptr<ExprBlockAST < T>> block = ParseExprBlock();

        if (!block) {
            fail();
            return LogError<FunctionAST < T >> ("Problem in function parsing", DI->getInfo());
        }

        if (block->empty()) {
            fail();
            return LogError<FunctionAST < T >> ("An empty function body is not allowed",
                    DI->getInfo());
        }

        /// verify return
        if (!block->hasReturn()) {

            if (Proto->getReturnType() == VarType::NONE) {
                // force an empty return if we don't hava one
                auto DII = genDebugInfo();
                block->addExpression(std::make_unique<ReturnAST < T >> (std::move(DII), nullptr));
            } else {
                fail();
                return LogError<FunctionAST < T >> ("Function without return as last sentence",
                        DI->getInfo());
            }
        }

        return std::make_unique<FunctionAST < T >> (std::move(DI), std::move(Proto), std::move(block));
    }


    /// prototype
    ///   ::= id '(' id* ')'

    std::unique_ptr<PrototypeAST<T>> ParsePrototype(bool pure) {

        VarType t;
        auto DI = genDebugInfo();

        if (lexer->getCurrentToken() != tok_type) {
            fail();
            return LogError<PrototypeAST < T >> ("Expected return type in prototype",
                    DI->getInfo());
        }

        std::string type = lexer->getIdentifierStr();
        t = convertType(type);
        lexer->getNextToken();

        if (lexer->getCurrentToken() != tok_identifier) {
            fail();
            return LogError<PrototypeAST < T >> ("Expected function name in prototype",
                    DI->getInfo());
        }

        std::string FnName = lexer->getIdentifierStr();
        //std::cout << FnName << std::endl;
        lexer->getNextToken();

        //std::cout << lexer->getCurrentToken() << std::endl;
        if (lexer->getCurrentToken() != '(') {
            fail();
            return LogError<PrototypeAST < T >> ("Expected '(' in prototype",
                    DI->getInfo());
        }

        // Read the list of argument names.
        std::vector<Arg> ArgNames;
        lexer->getNextToken();
        while (lexer->getCurrentToken() == tok_type) {
            std::string type = lexer->getIdentifierStr();
            // eat type
            lexer->getNextToken();
            if (lexer->getCurrentToken() != tok_identifier) {
                fail();
                return LogError<PrototypeAST < T >> ("Expected parameter name after type in prototype",
                        DI->getInfo());
            }
            std::string name = lexer->getIdentifierStr();
            ArgNames.push_back(createArg(type, name));
            lexer->getNextToken();
            if (lexer->getCurrentToken() == ',') {
                lexer->getNextToken();
                if (lexer->getCurrentToken() != tok_type) {
                    fail();
                    return LogError<PrototypeAST < T >> ("Untermined list of params in prototype",
                            DI->getInfo());
                }
            } else {
                break;
            }
        }
        if (lexer->getCurrentToken() != ')') {
            fail();
            return LogError<PrototypeAST < T >> ("Expected ')' in prototype",
                    DI->getInfo());
        }
        // success.
        lexer->getNextToken(); // eat ')'.

        // pure prototype for trully extern functions
        if (pure) {
            if (lexer->getCurrentToken() != ';') {
                fail();
                LogError<PrototypeAST < T >> ("Expected ';' in prototype",
                        DI->getInfo());
            }
            lexer->getNextToken();
        }

        return std::make_unique<PrototypeAST < T >> (std::move(DI), t, FnName, std::move(ArgNames));
    }

    /// expression
    ///   ::= primary binoprhs
    ///

    std::unique_ptr<ExprAST<T>> ParseExpression() {

        auto LHS = ParsePrimary();
        if (!LHS) {
            return nullptr;
        }

        std::unique_ptr<ExprAST < T>> exp = ParseBinOpRHS(0, std::move(LHS));

        return exp;
    }

    /// primary
    ///   ::= identifierexpr
    ///   ::= numberexpr
    ///   ::= parenexpr

    std::unique_ptr<ExprAST<T>> ParsePrimary() {
        auto DI = genDebugInfo();
        std::unique_ptr<ExprAST < T>> exp;
        switch (lexer->getCurrentToken()) {
            default:
                fail();
                std::cout << lexer->getIdentifierStr() << "\n";
                printf("-> %d\n", lexer->getCurrentToken());
                return LogError<ExprAST < T >> ("unknown token when expecting an expression",
                        DI->getInfo());
            case tok_identifier:
                exp = ParseIdentifierExpr();
                break;
            case tok_real:
                exp = ParseRealNumberExpr();
                break;
            case tok_integer:
                exp = ParseIntegerNumberExpr();
                break;
            case '(':
                exp = ParseParenExpr();
                break;
            case tok_return:
                exp = ParseReturnExpr();
                break;
            case tok_if:
                exp = ParseIfExpr();
                break;
            case tok_operator:
                exp = ParseUnaryExpr();
                break;
            case tok_let:
                exp = ParseLocalDeclarationExpr();
                break;
            case tok_for:
                exp = ParseForExpr();
                break;
            case tok_while:
                exp = ParseWhileExpr();
                break;
                //exp = 
        }
        return exp;
    }


    /// identifierexpr
    ///   ::= identifier
    ///   ::= identifier '(' expression* ')'

    std::unique_ptr<ExprAST<T>> ParseIdentifierExpr() {

        auto DI = genDebugInfo();
        std::string IdName = lexer->getIdentifierStr();
        std::unique_ptr<ExprAST < T>> exp;
        lexer->getNextToken(); // eat identifier.
        if (lexer->getCurrentToken() != '(') { // Simple variable ref.
            auto expTemp = std::make_unique<VariableExprAST < T >> (std::move(DI), IdName);
            // we can have an unary expression i.e. ++, --.
            if (lexer->getCurrentToken() == Token::tok_operator &&
                    lexer->getOpType() == OperationType::UNARY) {
                exp = std::make_unique<UnaryExprAST < T >> (std::move(DI), lexer->getOperation(), std::move(expTemp), false);
                lexer->getNextToken(); // eat operator
            } else {
                // or a simple variable expression
                exp = std::move(expTemp);
            }
            // Call.
        } else if (lexer->getCurrentToken() == '(') {
            lexer->getNextToken(); // eat (
            std::vector<std::unique_ptr < ExprAST < T>>> Args;
            if (lexer->getCurrentToken() != ')') {
                while (true) {
                    if (auto Arg = ParseExpression())
                        Args.push_back(std::move(Arg));
                    else
                        return nullptr;

                    if (lexer->getCurrentToken() == ')')
                        break;

                    if (lexer->getCurrentToken() != ',') {
                        fail();
                        return LogError<ExprAST < T >> ("Expected ')' or ',' in argument list",
                                DI->getInfo());
                    }
                    lexer->getNextToken();
                }
            }
            // Eat the ')'.
            lexer->getNextToken();
            exp = std::make_unique<CallExprAST < T >> (std::move(DI), IdName, std::move(Args));
            // assignment expression   

        }

        return exp;
    }



    /// binoprhs
    ///   ::= ('+' primary)*

    std::unique_ptr<ExprAST<T>> ParseBinOpRHS(int ExprPrec,
            std::unique_ptr<ExprAST<T>> LHS) {
        auto DI = genDebugInfo();
        // If this is a binop, find its precedence.
        while (true) {
            int TokPrec = lexer->GetTokPrecedence();

            // If this is a binop that binds at least as tightly as the current binop,
            // consume it, otherwise we are done.
            if (TokPrec < ExprPrec)
                return LHS;

            // Okay, we know this is a binop.
            Operation BinOp = lexer->getOperation();
            lexer->getNextToken(); // eat binop

            // Parse the primary expression after the binary operator.
            auto RHS = ParsePrimary();
            if (!RHS)
                return nullptr;

            // If BinOp binds less tightly with RHS than the operator after RHS, let
            // the pending operator take RHS as its LHS.
            int NextPrec = lexer->GetTokPrecedence();
            if (TokPrec < NextPrec) {
                RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
                if (!RHS)
                    return nullptr;
            }

            // Merge LHS/RHS.
            LHS = std::make_unique<BinaryExprAST < T >> (std::move(DI), BinOp, std::move(LHS),
                    std::move(RHS));
        }
    }

    std::unique_ptr<ExprAST<T>> ParseRealNumberExpr() {
        auto DI = genDebugInfo();
        auto Result = std::make_unique<RealNumberExprAST < T >> (std::move(DI), lexer->getNumValReal());
        lexer->getNextToken(); // consume the number
        return std::move(Result);
    }

    std::unique_ptr<ExprAST<T>> ParseIntegerNumberExpr() {
        auto DI = genDebugInfo();
        auto Result = std::make_unique<IntegerNumberExprAST < T >> (std::move(DI), lexer->getNumValInteger());
        lexer->getNextToken(); // consume the number
        return std::move(Result);
    }

    /// parenexpr ::= '(' expression ')'

    std::unique_ptr<ExprAST<T>> ParseParenExpr() {
        lexer->getNextToken(); // eat (.
        auto V = ParseExpression();
        auto DI = genDebugInfo();
        if (!V) {
            return nullptr;
        }
        if (lexer->getCurrentToken() != ')') {
            fail();
            return LogError<ExprAST < T >> ("expected ')'", DI->getInfo());
        }
        lexer->getNextToken(); // eat ).
        return V;
    }

    std::unique_ptr<ExprAST<T>> ParseReturnExpr() {

        auto DI = genDebugInfo();
        std::unique_ptr<ExprAST < T>> V;
        lexer->getNextToken(); // eat 'return'
        if (lexer->getCurrentToken() != ';') {
            V = ParseExpression();
            if (!V) {
                return nullptr;
            }
            //return std::make_unique<ReturnAST>(std::move(DI), std::move(V));
        } else {
            V = nullptr;
        }
        return std::make_unique<ReturnAST < T >> (std::move(DI), std::move(V));
    }


    /// block of expressions {}

    std::unique_ptr<ExprBlockAST<T>> ParseExprBlock() {

        auto DI = genDebugInfo();
        std::unique_ptr<ExprBlockAST < T>> block = std::make_unique<ExprBlockAST < T >> (std::move(DI));

        if (lexer->getCurrentToken() != '{') {
            fail();
            return LogError<ExprBlockAST < T >> ("Expected { in function body start",
                    block->getDebugInfo()->getInfo());

        }
        //eat {
        lexer->getNextToken();

        // Empty block
        if (lexer->getCurrentToken() == '}') {
            //eat }
            lexer->getNextToken();
            return block;
        }

        bool unreachable = false;
        while (auto E = ParseExpression()) {
            if (unreachable) {
                LogWarn("ureachable code", block->getDebugInfo()->getInfo());
            }
            if (E->isSimple()) {
                if (lexer->getCurrentToken() != ';') {
                    fail();
                    return LogError<ExprBlockAST < T >> ("Expected ; in expression",
                            E->getDebugInfo()->getInfo());
                }
                lexer->getNextToken();
            }
            if (E->isUncondTransfer()) {
                unreachable = true;
            }
            // test if we get a return
            block->addExpression(std::move(E));


            if (lexer->getCurrentToken() == '}') {
                break;
            }
        }
        if (lexer->getCurrentToken() != '}') {
            fail();
            return LogError<ExprBlockAST < T >> ("Expected } in function body termination",
                    genDebugInfo()->getInfo());
        }

        lexer->getNextToken();
        return block;
    }

    /// ifexpr ::= 'if' expression 'then' expression 'else' expression

    std::unique_ptr<ExprAST<T>> ParseIfExpr() {
        auto DI = genDebugInfo();
        lexer->getNextToken(); // eat the if.
        if (lexer->getCurrentToken() != '(') {
            fail();
            return LogError<ExprAST < T >> ("expected ( in if condition", DI->getInfo());
        }
        lexer->getNextToken();

        // condition.
        auto Cond = ParseExpression();
        if (!Cond)
            return nullptr;

        if (lexer->getCurrentToken() != ')') {
            fail();
            return LogError<ExprAST < T >> ("expected ) in the end of if condition", DI->getInfo());
        }
        lexer->getNextToken();

        //if (lexer->getCurrentToken() != '{')
        //  return LogError<ExprAST>("expected then", lexer->GetTokLine());
        //lexer->getNextToken();  // eat the then

        auto Then = ParseExprBlock();
        if (!Then) {
            return nullptr;
        }

        std::unique_ptr<ExprBlockAST < T>> Else = nullptr;

        if (lexer->getCurrentToken() == tok_else) {
            lexer->getNextToken();
            Else = ParseExprBlock();
            if (!Else)
                return nullptr;
        }
        //lexer->getNextToken();
        return std::make_unique<IfExprAST < T >> (std::move(DI), std::move(Cond), std::move(Then),
                std::move(Else));
    }



    /// forexpr ::= ...

    std::unique_ptr<ExprAST<T>> ParseForExpr() {

        auto DI = genDebugInfo();
        std::unique_ptr<ExprAST < T>> Start = nullptr;
        std::unique_ptr<ExprAST < T>> Cond = nullptr;
        std::unique_ptr<ExprAST < T>> End = nullptr;
        lexer->getNextToken(); // eat the for.
        if (lexer->getCurrentToken() != '(') {
            fail();
            return LogError<ExprAST < T >> ("expected ( in for construct", DI->getInfo());
        }

        // Est ')'
        lexer->getNextToken();

        // parse for header

        // we have a start?
        if (lexer->getCurrentToken() != ';') {
            Start = ParseExpression();
        }

        if (lexer->getCurrentToken() != ';') {
            fail();
            return LogError<ExprAST < T >> ("expected ; in for construct", DI->getInfo());
        }
        lexer->getNextToken();

        // we have a step?
        if (lexer->getCurrentToken() != ';') {
            Cond = ParseExpression();
        }

        if (lexer->getCurrentToken() != ';') {
            fail();
            return LogError<ExprAST < T >> ("expected ; in for construct", DI->getInfo());
        }
        lexer->getNextToken();

        // we have an end?
        if (lexer->getCurrentToken() != ')') {
            End = ParseExpression();
        }

        if (lexer->getCurrentToken() != ')') {
            fail();
            return LogError<ExprAST < T >> ("expected ) in for construct", DI->getInfo());
        }
        // Est ')'
        lexer->getNextToken();

        auto Block = ParseExprBlock();

        return std::make_unique<ForExprAST < T >> (std::move(DI), std::move(Start), std::move(End),
                std::move(Cond), std::move(Block));
    }


    /// whileexpr ::= ...

    std::unique_ptr<ExprAST<T>> ParseWhileExpr() {

        auto DI = genDebugInfo();
        std::unique_ptr<ExprAST < T>> Cond = nullptr;
        lexer->getNextToken(); // eat the while.

        if (lexer->getCurrentToken() != '(') {
            fail();
            return LogError<ExprAST < T >> ("expected ( in for construct", DI->getInfo());
        }
        // Est ')'
        lexer->getNextToken();

        // parse condition
        Cond = ParseExpression();

        if (lexer->getCurrentToken() != ')') {
            fail();
            return LogError<ExprAST < T >> ("expected ) in for construct", DI->getInfo());
        }
        // Est ')'
        lexer->getNextToken();

        auto Block = ParseExprBlock();

        return std::make_unique<WhileExprAST < T >> (std::move(DI), std::move(Cond), std::move(Block));

    }

    std::unique_ptr<ExprAST<T>> ParseUnaryExpr() {
        auto DI = genDebugInfo();
        Operation op = lexer->getOperation();
        std::unique_ptr<ExprAST < T>> exp;

        if (lexer->getOpType() != OperationType::UNARY) {
            fail();
            return LogError<ExprAST < T >> ("Invalid unary operator",
                    DI->getInfo());
        }
        lexer->getNextToken(); //eat unary op;
        exp = ParseIdentifierExpr();


        return std::make_unique<UnaryExprAST < T >> (std::move(DI), op, std::move(exp), true);
    }

    std::unique_ptr<ExprAST<T>> ParseLocalDeclarationExpr() {
        auto DI = genDebugInfo();
        std::unique_ptr<ExprAST < T>> exp;
        lexer->getNextToken(); // eat the let.

        if (lexer->getCurrentToken() != tok_type) {
            fail();
            return LogError<LocalVarDeclarationExprAST < T >> ("Expected type in let sentence",
                    DI->getInfo());
        }

        std::string typeStr = lexer->getIdentifierStr();
        VarType type = convertType(typeStr);

        lexer->getNextToken(); // eat type
        if (lexer->getCurrentToken() != tok_identifier) {
            fail();
            return LogError<LocalVarDeclarationExprAST < T >> ("Expected variable name in let sentence",
                    DI->getInfo());
        }

        std::string Name = lexer->getIdentifierStr();
        lexer->getNextToken(); // eat identifier

        if (lexer->getCurrentToken() == tok_operator && lexer->getOperation() == Operation::ASSIGN) {
            lexer->getNextToken(); // eat =
            exp = ParseExpression();
        }

        return std::make_unique<LocalVarDeclarationExprAST < T >> (std::move(DI), Name, std::move(exp), type);
    }

    bool failed = false;

    void fail() {
        failed = true;
    }

};

#endif	/* PARSER_H */
