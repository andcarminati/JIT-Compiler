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


#ifndef LEXER_H
#define	LEXER_H
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

#include "LangDefs.h"

// The lexer returns tokens [0-255] if it is an unknown character, otherwise one
// of these for known things.

enum Token {
    tok_eof = -1,

    // commands
    tok_function = -2,
    tok_extern = -3,

    // primary
    tok_identifier = -4,
    //tok_number = -5,

    // control
    tok_if = -6,
    tok_then = -7,
    tok_else = -8,

    // loop
    tok_for = -9,
    tok_while = -10,

    // return
    tok_return = -11,

    tok_operator = -12,
            
    //types
    tok_type_real = -13,
    tok_type_integer = -14,
    tok_type_string = -15,
    tok_type = -16,
    
    // numbers
    tok_real = -17,
    tok_integer = -18
};

class Lexer {
public:
    Lexer(std::unique_ptr<std::ifstream>, std::string);
    //Lexer(const Lexer& orig) { std::cout << "teste1";};
    //virtual ~Lexer(){}

    int getCurrentToken();
    int getNextToken();
    std::string getIdentifierStr();
    Operation getOperation();
    OperationType getOpType();
    double getNumValReal();
    double getNumValInteger();
    int GetTokPrecedence();
    int GetTokLine();
    std::string& getFileName();

private:
    int CurTok = ';';
    int LastChar = ' ';
    int line = 1;
    std::map<Operation, int> BinopPrecedence;
    std::unique_ptr<std::ifstream> file;
    std::string IdentifierStr; // Filled in if tok_identifier
    double NumValReal; // Filled in if tok_real
    double NumValInteger; // Filled in if tok_real
    Operation CurrOperation;
    OperationType CurrOpType;
    std::string FileName;
    int getTok();

};

#endif	/* LEXER_H */

