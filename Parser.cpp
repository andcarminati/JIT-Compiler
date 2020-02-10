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
static std::unique_ptr<T> LogError(const char *Str, std::string loc) {
    fprintf(stderr, "Compiler error (parser): %s -> %s\n", Str, loc.c_str());
    return nullptr;
}

void LogWarn(const char *Str, std::string loc) {
    fprintf(stderr, "Compiler warning (parser): %s -> %s\n", Str, loc.c_str());
}

std::unique_ptr<DebugInfo> Parser::genDebugInfo() {

    return std::make_unique<DebugInfo>(DebugInfo(lexer->GetTokLine(), lexer->GetTokCol(), lexer->getFileName()));
}

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
    auto DI = genDebugInfo();

    if (lexer->getCurrentToken() != tok_type) {
        fail();
        return LogError<PrototypeAST>("Expected return type in prototype",
                DI->getInfo());
    }

    std::string type = lexer->getIdentifierStr();
    t = convertType(type);
    lexer->getNextToken();

    if (lexer->getCurrentToken() != tok_identifier) {
        fail();
        return LogError<PrototypeAST>("Expected function name in prototype",
                DI->getInfo());
    }

    std::string FnName = lexer->getIdentifierStr();
    //std::cout << FnName << std::endl;
    lexer->getNextToken();

    //std::cout << lexer->getCurrentToken() << std::endl;
    if (lexer->getCurrentToken() != '(') {
        fail();
        return LogError<PrototypeAST>("Expected '(' in prototype",
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
            return LogError<PrototypeAST>("Expected parameter name after type in prototype",
                    DI->getInfo());
        }
        std::string name = lexer->getIdentifierStr();
        ArgNames.push_back(createArg(type, name));
        lexer->getNextToken();
        if (lexer->getCurrentToken() == ',') {
            lexer->getNextToken();
            if (lexer->getCurrentToken() != tok_type) {
                fail();
                return LogError<PrototypeAST>("Untermined list of params in prototype",
                        DI->getInfo());
            }
        } else {
            break;
        }
    }
    if (lexer->getCurrentToken() != ')') {
        fail();
        return LogError<PrototypeAST>("Expected ')' in prototype",
                DI->getInfo());
    }
    // success.
    lexer->getNextToken(); // eat ')'.

    // pure prototype for trully extern functions
    if (pure) {
        if (lexer->getCurrentToken() != ';') {
            fail();
            LogError<PrototypeAST>("Expected ';' in prototype",
                    DI->getInfo());
        }
        lexer->getNextToken();
    }

    return std::make_unique<PrototypeAST>(std::move(DI), t, FnName, std::move(ArgNames));
}

/// block of expressions {}

std::unique_ptr<ExprBlockAST> Parser::ParseExprBlock() {

    auto DI = genDebugInfo();
    std::unique_ptr<ExprBlockAST> block = std::make_unique<ExprBlockAST>(std::move(DI));

    if (lexer->getCurrentToken() != '{') {
        fail();
        return LogError<ExprBlockAST>("Expected { in function body start",
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
                return LogError<ExprBlockAST>("Expected ; in expression",
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
        return LogError<ExprBlockAST>("Expected } in function body termination",
                block->getDebugInfo()->getInfo());
    }

    lexer->getNextToken();
    return block;
}


/// definition ::= 'def' prototype expression

std::unique_ptr<FunctionAST> Parser::ParseDefinition() {
    lexer->getNextToken(); // eat def.
    auto DI = genDebugInfo();
    auto Proto = ParsePrototype(false);
    if (!Proto) return nullptr;

    if (lexer->getCurrentToken() != '{') {
        fail();
        return LogError<FunctionAST>("Expected { in function body start",
                DI->getInfo());
    }

    std::unique_ptr<ExprBlockAST> block = ParseExprBlock();

    if (!block) {
        fail();
        return LogError<FunctionAST>("Problem in function parsing", DI->getInfo());
    }

    if (block->empty()) {
        fail();
        return LogError<FunctionAST>("An empty function body is not allowed",
                DI->getInfo());
    }

    /// verify return
    if (!block->hasReturn()) {

        if (Proto->getReturnType() == VarType::NONE) {
            // force an empty return if we don't hava one
            auto DII = genDebugInfo();
            block->addExpression(std::make_unique<ReturnAST>(std::move(DII), nullptr));
        } else {
            fail();
            return LogError<FunctionAST>("Function without return as last sentence",
                    DI->getInfo());
        }
    }

    return std::make_unique<FunctionAST>(std::move(DI), std::move(Proto), std::move(block));
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
    auto DI = genDebugInfo();
    std::unique_ptr<ExprAST> exp;
    switch (lexer->getCurrentToken()) {
        default:
            fail();
            std::cout << lexer->getIdentifierStr() << "\n";
            printf("-> %d\n", lexer->getCurrentToken());
            return LogError<ExprAST>("unknown token when expecting an expression",
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
            //exp = 
    }
    return exp;
}

std::unique_ptr<ExprAST> Parser::ParseUnaryExpr() {
    auto DI = genDebugInfo();
    Operation op = lexer->getOperation();
    std::unique_ptr<ExprAST> exp;

    if (lexer->getOpType() != OperationType::UNARY) {
        fail();
        return LogError<ExprAST>("Invalid unary operator",
                DI->getInfo());
    }
    lexer->getNextToken(); //eat unary op;
    exp = ParseIdentifierExpr();


    return std::make_unique<UnaryExprAST>(std::move(DI), op, std::move(exp), true);
}


/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'

std::unique_ptr<ExprAST> Parser::ParseIdentifierExpr() {

    auto DI = genDebugInfo();
    std::string IdName = lexer->getIdentifierStr();
    std::unique_ptr<ExprAST> exp;
    lexer->getNextToken(); // eat identifier.
    if (lexer->getCurrentToken() != '(') { // Simple variable ref.
        auto expTemp = std::make_unique<VariableExprAST>(std::move(DI), IdName);
        // we can have an unary expression i.e. ++, --.
        if (lexer->getCurrentToken() == Token::tok_operator &&
                lexer->getOpType() == OperationType::UNARY) {
            exp = std::make_unique<UnaryExprAST>(std::move(DI), lexer->getOperation(), std::move(expTemp), false);
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
                            DI->getInfo());
                }
                lexer->getNextToken();
            }
        }
        // Eat the ')'.
        lexer->getNextToken();
        exp = std::make_unique<CallExprAST>(std::move(DI), IdName, std::move(Args));
        // assignment expression   

    }

    return exp;
}

/// binoprhs
///   ::= ('+' primary)*

std::unique_ptr<ExprAST> Parser::ParseBinOpRHS(int ExprPrec,
        std::unique_ptr<ExprAST> LHS) {
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
        LHS = std::make_unique<BinaryExprAST>(std::move(DI), BinOp, std::move(LHS),
                std::move(RHS));
    }
}

std::unique_ptr<ExprAST> Parser::ParseRealNumberExpr() {
    auto DI = genDebugInfo();
    auto Result = std::make_unique<RealNumberExprAST>(std::move(DI), lexer->getNumValReal());
    lexer->getNextToken(); // consume the number
    return std::move(Result);
}

std::unique_ptr<ExprAST> Parser::ParseIntegerNumberExpr() {
    auto DI = genDebugInfo();
    auto Result = std::make_unique<IntegerNumberExprAST>(std::move(DI), lexer->getNumValInteger());
    lexer->getNextToken(); // consume the number
    return std::move(Result);
}

/// parenexpr ::= '(' expression ')'

std::unique_ptr<ExprAST> Parser::ParseParenExpr() {
    lexer->getNextToken(); // eat (.
    auto V = ParseExpression();
    auto DI = genDebugInfo();
    if (!V) {
        return nullptr;
    }
    if (lexer->getCurrentToken() != ')') {
        fail();
        return LogError<ExprAST>("expected ')'", DI->getInfo());
    }
    lexer->getNextToken(); // eat ).
    return V;
}

std::unique_ptr<ExprAST> Parser::ParseReturnExpr() {

    auto DI = genDebugInfo();
    std::unique_ptr<ExprAST> V;
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
    return std::make_unique<ReturnAST>(std::move(DI), std::move(V));
}


/// ifexpr ::= 'if' expression 'then' expression 'else' expression

std::unique_ptr<ExprAST> Parser::ParseIfExpr() {
    auto DI = genDebugInfo();
    lexer->getNextToken(); // eat the if.
    if (lexer->getCurrentToken() != '(') {
        fail();
        return LogError<ExprAST>("expected ( in if condition", DI->getInfo());
    }
    lexer->getNextToken();

    // condition.
    auto Cond = ParseExpression();
    if (!Cond)
        return nullptr;

    if (lexer->getCurrentToken() != ')') {
        fail();
        return LogError<ExprAST>("expected ) in the end of if condition", DI->getInfo());
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
    return std::make_unique<IfExprAST>(std::move(DI), std::move(Cond), std::move(Then),
            std::move(Else));
}

/// forexpr ::= ...

std::unique_ptr<ExprAST> Parser::ParseForExpr() {

    auto DI = genDebugInfo();
    std::unique_ptr<ExprAST> Start = nullptr;
    std::unique_ptr<ExprAST> Cond = nullptr;
    std::unique_ptr<ExprAST> End = nullptr;
    lexer->getNextToken(); // eat the if.
    if (lexer->getCurrentToken() != '(') {
        fail();
        return LogError<ExprAST>("expected ( in for construct", DI->getInfo());
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
        return LogError<ExprAST>("expected ; in for construct", DI->getInfo());
    }
    lexer->getNextToken();

    // we have a step?
    if (lexer->getCurrentToken() != ';') {
        Cond = ParseExpression();
    }

    if (lexer->getCurrentToken() != ';') {
        fail();
        return LogError<ExprAST>("expected ; in for construct", DI->getInfo());
    }
    lexer->getNextToken();

    // we have an end?
    if (lexer->getCurrentToken() != ')') {
        End = ParseExpression();
    }

    if (lexer->getCurrentToken() != ')') {
        fail();
        return LogError<ExprAST>("expected ) in for construct", DI->getInfo());
    }
    // Est '('
    lexer->getNextToken();

    auto Block = ParseExprBlock();

    return std::make_unique<ForExprAST>(std::move(DI), std::move(Start), std::move(End),
            std::move(Cond), std::move(Block));
}

std::unique_ptr<ExprAST> Parser::ParseLocalDeclarationExpr() {

    auto DI = genDebugInfo();
    std::unique_ptr<ExprAST> exp;
    lexer->getNextToken(); // eat the let.

    if (lexer->getCurrentToken() != tok_type) {
        fail();
        return LogError<LocalVarDeclarationExprAST>("Expected type in let sentence",
                DI->getInfo());
    }

    std::string typeStr = lexer->getIdentifierStr();
    VarType type = convertType(typeStr);

    lexer->getNextToken(); // eat type
    if (lexer->getCurrentToken() != tok_identifier) {
        fail();
        return LogError<LocalVarDeclarationExprAST>("Expected variable name in let sentence",
                DI->getInfo());
    }

    std::string Name = lexer->getIdentifierStr();
    lexer->getNextToken(); // eat identifier

    if (lexer->getCurrentToken() == tok_operator && lexer->getOperation() == Operation::ASSIGN) {
        lexer->getNextToken(); // eat =
        exp = ParseExpression();
    }

    return std::make_unique<LocalVarDeclarationExprAST>(std::move(DI), Name, std::move(exp), type);
}