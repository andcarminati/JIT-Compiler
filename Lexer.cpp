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


#include "Lexer.h"
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>

Lexer::Lexer(std::unique_ptr<std::ifstream> file, std::string FileName) {
    this->file = std::move(file);
    this->FileName = FileName;
    // Install standard binary operators.
    // 1 is lowest precedence.
    BinopPrecedence[Operation::ASSIGN] = 2;
    BinopPrecedence[Operation::LT] = 10;
    BinopPrecedence[Operation::EQ] = 12;
    BinopPrecedence[Operation::ADD] = 20;
    BinopPrecedence[Operation::SUB] = 20;
    BinopPrecedence[Operation::MUL] = 40; // highest.
    getNextToken();
}

int Lexer::getCurrentToken() {
    return CurTok;
}

int Lexer::getNextToken() {

    CurTok = getTok();
    return CurTok;
}

int Lexer::NextChar(){
    LastChar = file->get();
    LineStr += LastChar;
    return LastChar;
}

int Lexer::getTok() {// gettok - Return the next token from standard input.
    // Skip any whitespace.


    while (isspace(LastChar)) {
        if (LastChar == '\n') {
            line++;
            col = 1;
            LineStr = "";
        } else {
            col++;
        }
        NextChar();
    }
    tokCol = col;
    if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
        IdentifierStr = LastChar;
        while (isalnum(NextChar())) {
            col++;
            IdentifierStr += LastChar;
        }

        if (IdentifierStr == "function")
            return tok_function;
        if (IdentifierStr == "extern")
            return tok_extern;
        if (IdentifierStr == "return")
            return tok_return;
        if (IdentifierStr == "if")
            return tok_if;
        if (IdentifierStr == "else")
            return tok_else;
        if (IdentifierStr == "for")
            return tok_for;
        if (IdentifierStr == "while")
            return tok_while;
        if (IdentifierStr == "real") {
            return tok_type;
        }
        if (IdentifierStr == "integer") {
            return tok_type;
        }
        if (IdentifierStr == "string") {
            return tok_type;
        }
        if (IdentifierStr == "none") {
            return tok_type;
        }
        if (IdentifierStr == "let") {
            return tok_let;
        }
        if(IdentifierStr == "for") {
            return tok_for;
        }
        return tok_identifier;
    }

    if (isdigit(LastChar) || LastChar == '.') { // Number: [0-9.]+
        std::string NumStr;
        do {
            NumStr += LastChar;
            NextChar();
            col++;
        } while (isdigit(LastChar) || LastChar == '.');

        std::size_t found = NumStr.find('.');
        if (found != std::string::npos) {
            NumValReal = strtod(NumStr.c_str(), nullptr);
            return tok_real;
        } else {
            NumValInteger = strtoll(NumStr.c_str(), nullptr, 0);
            return tok_integer;
        }

    }

    if (LastChar == '#') {
        // Comment until end of line.
        do {
            NextChar();
        } while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');
        line++;
        col = 1;
        if (LastChar != EOF)
            return getTok();
    }

    // Check for end of file.  Don't eat the EOF.
    if (LastChar == EOF)
        return tok_eof;

    // Otherwise, just return the character as its ascii value.
    int ThisChar = LastChar;
    NextChar();
    col++;
    CurrOperation = Operation::UNKNOWN;
    CurrOpType = OperationType::BINARY;
    switch (ThisChar) {
        case '=':
        {
            if (LastChar == '=') {
                NextChar();
                col++;
                CurrOperation = Operation::EQ;
            } else {
                CurrOperation = Operation::ASSIGN;
            }
            break;
        }
        case '<':
        {
            CurrOperation = Operation::LT;
            break;
        }
        case '+':
        {
            if (LastChar == '+') {
                NextChar();
                col++;
                CurrOperation = Operation::INC;
                CurrOpType = OperationType::UNARY;
            } else {
                CurrOperation = Operation::ADD;
            }
            break;
        }
        case '-':
        {
            if (LastChar == '-') {
                NextChar();
                col++;
                CurrOperation = Operation::DEC;
                CurrOpType = OperationType::UNARY;
            } else {
                CurrOperation = Operation::SUB;
            }
            break;
        }
        case '*':
        {
            CurrOperation = Operation::MUL;
            break;
        }
        case '/':
        {
            CurrOperation = Operation::DIV;
            break;
        }
        default:
            return ThisChar;
    }

    return tok_operator;
}

std::string Lexer::getIdentifierStr() {
    return IdentifierStr;
}

double Lexer::getNumValReal() {
    return NumValReal;
}

double Lexer::getNumValInteger() {
    return NumValInteger;
}

Operation Lexer::getOperation() {
    return CurrOperation;
}

OperationType Lexer::getOpType() {
    return CurrOpType;
}

/// GetTokPrecedence - Get the precedence of the pending binary operator token.

int Lexer::GetTokPrecedence() {
    if (CurTok != tok_operator)
        return -1;

    // Make sure it's a declared binop.
    int TokPrec = BinopPrecedence[CurrOperation];
    if (TokPrec <= 0) return -1;
    return TokPrec;
}

int Lexer::GetTokLine() {
    return line;
}

int Lexer::GetTokCol(){
    return tokCol;
}

std::string& Lexer::getFileName() {
    return FileName;
}

std::string Lexer::getLineStr(){
    return LineStr;
}