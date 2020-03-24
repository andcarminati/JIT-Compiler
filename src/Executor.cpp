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


#include "Executor.h"
#include <iostream>

static ExitOnError ExitOnErr;


void Executor::execute() {

    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();
    TheJIT = ExitOnErr(KaleidoscopeJIT::Create());
    TheModule->setDataLayout(TheJIT->getDataLayout());
    ExitOnErr(TheJIT->addModule(std::move(TheModule)));

    
    // Search the JIT for the __anon_expr symbol.
    auto ExprSymbol = ExitOnErr(TheJIT->lookup("main"));
    
    // necessary???
    if(!ExprSymbol){
        std::cout << "Main function not found" << std::endl;
        return;
    }

    // Get the symbol's address and cast it to the right type (takes no
    // arguments, returns a double) so we can call it as a native function.
    auto *FP = (double (*)())(intptr_t) ExprSymbol.getAddress();
    assert(FP && "Failed to codegen function");
    FP();
    // Delete the anonymous expression module from the JIT.
    //TheJIT->removeModule(H);
}
