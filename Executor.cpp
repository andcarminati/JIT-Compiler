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

void Executor::execute() {

    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();
    TheJIT = std::make_unique<KaleidoscopeJIT>();
    TheModule->setDataLayout(TheJIT->getTargetMachine().createDataLayout());
    TheJIT->addModule(std::move(TheModule));

    
    // Search the JIT for the __anon_expr symbol.
    auto ExprSymbol = TheJIT->findSymbol("main");
    if(!ExprSymbol){
        std::cout << "Main function not found" << std::endl;
        return;
    }

    // Get the symbol's address and cast it to the right type (takes no
    // arguments, returns a double) so we can call it as a native function.
    double (*FP)() = (double (*)())(intptr_t) cantFail(ExprSymbol.getAddress());
    //fprintf(stderr, "Return of the main function %f\n", FP());
    FP();
    // Delete the anonymous expression module from the JIT.
    //TheJIT->removeModule(H);
}
