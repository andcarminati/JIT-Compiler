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



#include "Optimizer.h"
#include <llvm/IR/Verifier.h>
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils.h"

void Optimizer::InitializePassManager() {
    //TheModule->setDataLayout(TheJIT->getTargetMachine().createDataLayout());
    TheFPM = std::make_unique<legacy::FunctionPassManager>(TheModule.get());
    // Create a new pass manager attached to it.

    // Promote allocas to registers.
    TheFPM->add(createPromoteMemoryToRegisterPass());
    // Do simple "peephole" optimizations and bit-twiddling optzns.
    TheFPM->add(createInstructionCombiningPass());
    // Reassociate expressions.
    TheFPM->add(createReassociatePass());
    // Eliminate Common SubExpressions.
    TheFPM->add(createGVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    TheFPM->add(createCFGSimplificationPass());

    TheFPM->doInitialization();
}


void Optimizer::optimizeCode() {

    for (auto &Func : TheModule->getFunctionList()) {
        //Func.print(errs());
       
        verifyFunction(Func, &errs());
        //TheFPM->run(Func);
        //Func.print(errs());
        
    }
}

std::unique_ptr<Module> Optimizer::getModule(){
    
    return std::move(TheModule);
}
