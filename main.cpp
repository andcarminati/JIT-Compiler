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



#include <llvm/IR/LLVMContext.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include "Parser.h"
#include "Lexer.h"
#include "AST.h"
#include "IRGen.h"
#include "Optimizer.h"
#include "Executor.h"

//using namespace llvm;
using namespace std;

static llvm::LLVMContext TheContext;

static void Driver(std::shared_ptr<Parser<llvm::Value*>> parser){
    
    auto generator = std::make_unique<IRGen>(IRGen(&TheContext));
    
    while(true){
        auto exp = parser->nextConstruct();
        if(parser->hasFail()){
            std::cout << "Aborting compilation" << std::endl;
            return;
        }
        if(!exp){
            break;
        }
        generator->GenFromAST(std::move(exp));
    }
    
    auto optimizer = std::make_unique<Optimizer>(Optimizer(&TheContext, generator->getModule()));
    optimizer->optimizeCode();
    
    auto executor = std::make_unique<Executor>(Executor(&TheContext, optimizer->getModule()));
    executor->execute();
}

/*
 * 
 */
// avoid reexecution of main.
static bool again = false;

int main(int argc, char** argv) {

    if(again){
        return -1;
    } else{
        again = true;
    } 
    
    //auto file = std::make_unique<std::ifstream>(std::ifstream("tests/testif.txt"));
    auto file = std::make_unique<std::ifstream>(std::ifstream("tests/test13.txt"));
    
    if(!file->good()){
        std::cout << "Cannot open the specified file!" << std::endl;
        exit(-1);
    }
    auto lexer = std::make_unique<Lexer>(Lexer(std::move(file), "tests/test13.txt"));
    auto parser = std::make_shared<Parser<llvm::Value*>>(Parser<llvm::Value*>(std::move(lexer)));
    
    Driver(parser);
    
    return 0;
}

