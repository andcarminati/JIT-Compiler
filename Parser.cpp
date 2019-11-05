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


#include "Parser.h"
/// LogError* - These are little helper functions for error handling.

static void abort(std::string str, int line) {
    std::cout << str << " at line: " << line << std::endl;
    exit(-1);
}

template <class T>
std::unique_ptr<T> LogError(const char *Str, int line) {
    fprintf(stderr, "Compiler error: %s - line %d\n", Str, line);
    return nullptr;
}

void LogWarn(const char *Str, int line) {
    fprintf(stderr, "Compiler warning: %s -line %d\n", Str, line);
}

/*
std::unique_ptr<ExprAST> LogError(const char *Str, int line) {
    fprintf(stderr, "LogError: %s - line %d\n", Str, line);
    return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char *Str, int line) {
    LogError(Str, line);
    return nullptr;
}

std::unique_ptr<FunctionAST> LogErrorF(const char *Str, int line) {
    LogError(Str, line);
    return nullptr;
}


std::unique_ptr<ExprBlockAST> LogErrorB(const char *Str, int line) {
    LogError(Str, line);
    return nullptr;
}
 */
void Parser::fail() {
    failed = true;
}

bool Parser::hasFail() {
    return failed;
}

std::unique_ptr<PrimaryAST> Parser::nextConstruct() {
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

/// prototype
///   ::= id '(' id* ')'

std::unique_ptr<PrototypeAST> Parser::ParsePrototype(bool pure) {

    VarType t;

    if (lexer->getCurrentToken() != tok_type)
        return LogError<PrototypeAST>("Expected return type in prototype",
            lexer->GetTokLine());

    std::string type = lexer->getIdentifierStr();
    t = convertType(type);
    lexer->getNextToken();

    if (lexer->getCurrentToken() != tok_identifier)
        return LogError<PrototypeAST>("Expected function name in prototype",
            lexer->GetTokLine());

    std::string FnName = lexer->getIdentifierStr();
    //std::cout << FnName << std::endl;
    lexer->getNextToken();

    //std::cout << lexer->getCurrentToken() << std::endl;
    if (lexer->getCurrentToken() != '(')
        return LogError<PrototypeAST>("Expected '(' in prototype",
            lexer->GetTokLine());

    // Read the list of argument names.
    std::vector<Arg> ArgNames;
    lexer->getNextToken();
    while (lexer->getCurrentToken() == tok_type) {
        std::string type = lexer->getIdentifierStr();
        // eat type
        lexer->getNextToken();
        if (lexer->getCurrentToken() != tok_identifier) {
            return LogError<PrototypeAST>("Expected parameter name after type in prototype",
                    lexer->GetTokLine());
        }
        std::string name = lexer->getIdentifierStr();
        ArgNames.push_back(createArg(type, name));
        lexer->getNextToken();
        if (lexer->getCurrentToken() == ',') {
            lexer->getNextToken();
            if (lexer->getCurrentToken() != tok_type) {
                return LogError<PrototypeAST>("Untermined list of params in prototype",
                        lexer->GetTokLine());
            }
        } else {
            break;
        }
    }
    if (lexer->getCurrentToken() != ')') {
        return LogError<PrototypeAST>("Expected ')' in prototype",
                lexer->GetTokLine());
    }
    // success.
    lexer->getNextToken(); // eat ')'.

    // pure prototype for trully extern functions
    if (pure) {
        if (lexer->getCurrentToken() != ';') {
            LogError<PrototypeAST>("Expected ';' in prototype",
                    lexer->GetTokLine());
        }
        lexer->getNextToken();
    }

    //std::cout << "Func name " << FnName << std::endl;
    //std::cout << "Func args " << ArgNames.size() << std::endl;
    return std::make_unique<PrototypeAST>(t, FnName, std::move(ArgNames));
}

/// block of expressions {}

std::unique_ptr<ExprBlockAST> Parser::ParseExprBlock() {
    std::unique_ptr<ExprBlockAST> block = std::make_unique<ExprBlockAST>(ExprBlockAST());

    if (lexer->getCurrentToken() != '{') {
        fail();
        return LogError<ExprBlockAST>("Expected { in function body start",
                lexer->GetTokLine());

    }
    //eat {
    lexer->getNextToken();
    bool unreachable = false;
    while (auto E = ParseExpression()) {
        if (unreachable) {
            LogWarn("ureachable code", lexer->GetTokLine());
        }
        if (E->isSimple()) {
            if (lexer->getCurrentToken() != ';') {
                fail();
                return LogError<ExprBlockAST>("Expected ; in expression",
                        lexer->GetTokLine());
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
        return LogError<ExprBlockAST>("Expected } in function body termination",
                lexer->GetTokLine());
    }

    lexer->getNextToken();
    return block;
}


/// definition ::= 'def' prototype expression

std::unique_ptr<FunctionAST> Parser::ParseDefinition() {
    lexer->getNextToken(); // eat def.
    auto Proto = ParsePrototype(false);
    if (!Proto) return nullptr;

    if (lexer->getCurrentToken() != '{') {
        fail();
        return LogError<FunctionAST>("Expected { in function body start",
                lexer->GetTokLine());
    }
    //eat {
    //lexer->getNextToken();

    //if (auto E = ParseExpression()) {
    //   if (lexer->getCurrentToken() != '}') {
    //       printf("%c\n", lexer->getCurrentToken());
    //         return LogErrorF("Expected } in function body termination", lexer->GetTokLine());
    //     }
    //eat }
    //      lexer->getNextToken();
    //     return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    // }
    std::unique_ptr<ExprBlockAST> block = ParseExprBlock();

    if (!block) {
        fail();
        return LogError<FunctionAST>("Problem in function parsing", lexer->GetTokLine());
    }

    if (block->empty()) {
        fail();
        return LogError<FunctionAST>("An empty function body is not allowed",
                lexer->GetTokLine());
    }

    return std::make_unique<FunctionAST>(std::move(Proto), std::move(block));
}

/// expression
///   ::= primary binoprhs
///

std::unique_ptr<ExprAST> Parser::ParseExpression() {

    auto LHS = ParsePrimary();
    if (!LHS) {
        return nullptr;
    }

    std::unique_ptr<ExprAST> exp = ParseBinOpRHS(0, std::move(LHS));

    return exp;
}

/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr

std::unique_ptr<ExprAST> Parser::ParsePrimary() {
    std::unique_ptr<ExprAST> exp;
    switch (lexer->getCurrentToken()) {
        default:
            fail();
            return LogError<ExprAST>("unknown token when expecting an expression",
                    lexer->GetTokLine());
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
            //exp = 
    }
    return exp;
}

std::unique_ptr<ExprAST> Parser::ParseUnaryExpr() {
    Operation op = lexer->getOperation();
    std::unique_ptr<ExprAST> exp;

    if (lexer->getOpType() != OperationType::UNARY) {
        fail();
        return LogError<ExprAST>("Invalid unary operator",
                lexer->GetTokLine());
    }
    lexer->getNextToken(); //eat unary op;
    exp = ParseIdentifierExpr();


    return std::make_unique<UnaryExprAST>(op, std::move(exp), true);
}


/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'

std::unique_ptr<ExprAST> Parser::ParseIdentifierExpr() {
    std::string IdName = lexer->getIdentifierStr();
    std::unique_ptr<ExprAST> exp;
    lexer->getNextToken(); // eat identifier.
    if (lexer->getCurrentToken() != '(') { // Simple variable ref.
        auto expTemp = std::make_unique<VariableExprAST>(IdName);
        // we can have an unary expression i.e. ++, --.
        if (lexer->getCurrentToken() == Token::tok_operator &&
                lexer->getOpType() == OperationType::UNARY) {
            exp = std::make_unique<UnaryExprAST>(lexer->getOperation(), std::move(expTemp), false);
            lexer->getNextToken(); // eat operator
        } else {
            // or a simple variable expression
            exp = std::move(expTemp);
        }
        // Call.
    } else if (lexer->getCurrentToken() == '(') {
        lexer->getNextToken(); // eat (
        std::vector<std::unique_ptr < ExprAST>> Args;
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
                    return LogError<ExprAST>("Expected ')' or ',' in argument list",
                            lexer->GetTokLine());
                }
                lexer->getNextToken();
            }
        }
        // Eat the ')'.
        lexer->getNextToken();
        exp = std::make_unique<CallExprAST>(IdName, std::move(Args));
        // assignment expression   

    }

    return exp;
}

/// binoprhs
///   ::= ('+' primary)*

std::unique_ptr<ExprAST> Parser::ParseBinOpRHS(int ExprPrec,
        std::unique_ptr<ExprAST> LHS) {
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
        LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS),
                std::move(RHS));
    }
}

std::unique_ptr<ExprAST> Parser::ParseRealNumberExpr() {
    auto Result = std::make_unique<RealNumberExprAST>(lexer->getNumValReal());
    lexer->getNextToken(); // consume the number
    return std::move(Result);
}

std::unique_ptr<ExprAST> Parser::ParseIntegerNumberExpr() {
    auto Result = std::make_unique<IntegerNumberExprAST>(lexer->getNumValInteger());
    lexer->getNextToken(); // consume the number
    return std::move(Result);
}

/// parenexpr ::= '(' expression ')'

std::unique_ptr<ExprAST> Parser::ParseParenExpr() {
    lexer->getNextToken(); // eat (.
    auto V = ParseExpression();
    if (!V) {
        return nullptr;
    }
    if (lexer->getCurrentToken() != ')') {
        fail();
        return LogError<ExprAST>("expected ')'", lexer->GetTokLine());
    }
    lexer->getNextToken(); // eat ).
    return V;
}

std::unique_ptr<ExprAST> Parser::ParseReturnExpr() {
    lexer->getNextToken(); // eat 'return'
    auto V = ParseExpression();
    if (!V) {
        return nullptr;
    }
    return std::make_unique<ReturnAST>(ReturnAST(std::move(V)));
}


/// ifexpr ::= 'if' expression 'then' expression 'else' expression

std::unique_ptr<ExprAST> Parser::ParseIfExpr() {
    lexer->getNextToken(); // eat the if.
    if (lexer->getCurrentToken() != '(') {
        fail();
        return LogError<ExprAST>("expected ( in if condition", lexer->GetTokLine());
    }
    lexer->getNextToken();

    // condition.
    auto Cond = ParseExpression();
    if (!Cond)
        return nullptr;

    if (lexer->getCurrentToken() != ')') {
        fail();
        return LogError<ExprAST>("expected ) in the end of if condition", lexer->GetTokLine());
    }
    lexer->getNextToken();

    //if (lexer->getCurrentToken() != '{')
    //  return LogError<ExprAST>("expected then", lexer->GetTokLine());
    //lexer->getNextToken();  // eat the then

    auto Then = ParseExprBlock();
    if (!Then) {
        return nullptr;
    }

    std::unique_ptr<ExprBlockAST> Else = nullptr;

    if (lexer->getCurrentToken() == tok_else) {
        lexer->getNextToken();
        Else = ParseExprBlock();
        if (!Else)
            return nullptr;
    }
    //lexer->getNextToken();
    return std::make_unique<IfExprAST>(std::move(Cond), std::move(Then),
            std::move(Else));
}