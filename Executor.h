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
#ifndef EXECUTOR_H
#define	EXECUTOR_H

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include "JIT.h"

using namespace llvm;
using namespace llvm::orc;

class Executor {
public:

    Executor(LLVMContext* TheContext, std::unique_ptr<Module>&& TheModule) :
    TheContext(TheContext), TheModule(std::move(TheModule)) {
    }

    void execute();
private:
    std::unique_ptr<Module> TheModule;
    LLVMContext* TheContext;
    std::unique_ptr<KaleidoscopeJIT> TheJIT;
};

#endif	/* EXECUTOR_H */

