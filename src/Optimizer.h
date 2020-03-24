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



#ifndef JIT_H
#define	JIT_H

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/LegacyPassManager.h>
#include <memory>

using namespace llvm;

class Optimizer {
public:

    Optimizer(LLVMContext* TheContext, std::unique_ptr<Module>&& TheModule) :
    TheContext(TheContext), TheModule(std::move(TheModule)) {
        InitializePassManager();
    }

    void optimizeCode();
    std::unique_ptr<Module> getModule();
private:
    std::unique_ptr<legacy::FunctionPassManager> TheFPM;
    std::unique_ptr<Module> TheModule;
    LLVMContext* TheContext;

    void InitializePassManager();
};

#endif	/* JIT_H */

