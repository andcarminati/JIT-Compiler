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

class Parser {
public:
    Parser(std::unique_ptr<Lexer> lexer): lexer(std::move(lexer)){}
    //Parser(const Parser& orig) { std::cout << "teste";};
    //virtual ~Parser() {};
    
    std::unique_ptr<PrimaryAST> nextConstruct();
private:
    std::unique_ptr<Lexer> lexer;
    
    std::unique_ptr<FunctionAST> ParseDefinition();
    std::unique_ptr<PrototypeAST> ParsePrototype(bool pure);
    std::unique_ptr<ExprAST> ParseExpression();
    std::unique_ptr<ExprAST> ParsePrimary(); 
    std::unique_ptr<ExprAST> ParseIdentifierExpr();
    std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
                                              std::unique_ptr<ExprAST> LHS);
    std::unique_ptr<ExprAST> ParseNumberExpr();
    std::unique_ptr<ExprAST> ParseParenExpr();
    std::unique_ptr<ExprAST> ParseReturnExpr();
    std::unique_ptr<ExprBlockAST> ParseExprBlock();
    std::unique_ptr<ExprAST> ParseIfExpr();
    
};

#endif	/* PARSER_H */

