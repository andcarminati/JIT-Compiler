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


#include <llvm/IR/Value.h>
#include <string>
#include <memory>
#include <list>


#ifndef SYMBOLTABLE_H
#define	SYMBOLTABLE_H

enum StorageType{
    LOCAL = 0,
    GLOBAL
};    

class Symbol{
    std::string name;
    llvm::Value* memRef;
    StorageType storageType;
    int scope;
public:
    Symbol(std::string name, llvm::Value* memRef, StorageType storageType, int scope) :
        name(name), memRef(memRef), storageType(storageType), scope(scope) {}
    std::string& getName(){return name;}    
    llvm::Value* getMemRef(){return memRef;}
    int getScopeLevel(){return scope;}  
    StorageType getStorageType(){return storageType;};
};

class SymbolTable {
  
std::list<Symbol> symbols;
int scopeDeep = 0;

public:
    SymbolTable();
    void push_scope();
    void pop_scope();
    bool contains (std::string name);
    StorageType getStorageType(std::string name);
    Symbol* getSymbol(std::string name);
    void insertSymbol(std::string name, StorageType storageType, llvm::Value* menRef);
private:

};

#endif	/* SYMBOLTABLE_H */

