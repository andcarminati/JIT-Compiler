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



#include "SymbolTable.h"
#include <iostream>
#include <iterator>

SymbolTable::SymbolTable() {
}

void SymbolTable::push_scope(){
    scopeDeep++;
}

void SymbolTable::pop_scope(){
    std::list<Symbol>::iterator it=symbols.begin();
    
    while(it != symbols.end()){
        Symbol* symb = &*it;
        if(symb->getScopeLevel() == scopeDeep){
            it = symbols.erase(it);
        } else {
            // job done;
            break;
        }
    }
    scopeDeep--;
}

bool SymbolTable::contains(std::string name){
    
    for (std::list<Symbol>::iterator it=symbols.begin(); it != symbols.end(); ++it){
        Symbol* symb = &(*it);
        if(symb->getName() == name){
            return true;
        }
    }
    return false;
}
StorageType SymbolTable::getStorageType(std::string name){
    return StorageType::LOCAL;
}

void SymbolTable::insertSymbol(std::string name, StorageType storageType, llvm::Value* menRef){
    
    if(!contains(name)){
        symbols.push_front(Symbol(name, menRef, storageType, scopeDeep));
    }
}

Symbol* SymbolTable::getSymbol(std::string name){
    
    for (std::list<Symbol>::iterator it=symbols.begin(); it != symbols.end(); ++it){
        Symbol* symb = &(*it);
        if(symb->getName() == name){
            return symb;
        }
    }
    return nullptr;    
}
