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


#include <iterator>
#include <string>
#include <memory>
#include <list>


#ifndef SYMBOLTABLE_H
#define	SYMBOLTABLE_H

enum StorageType {
    LOCAL = 0,
    GLOBAL
};

template<typename SSAType>
class Symbol {
    std::string name;
    SSAType memRef;
    StorageType storageType;
    int scope;
public:

    Symbol(std::string name, SSAType memRef, StorageType storageType, int scope) :
    name(name), memRef(memRef), storageType(storageType), scope(scope) {
    }

    std::string& getName() {
        return name;
    }

    SSAType getMemRef() {
        return memRef;
    }

    int getScopeLevel() {
        return scope;
    }

    StorageType getStorageType() {
        return storageType;
    };
};

template<typename SSAType>
class ListSymbolTable {
public:
    ListSymbolTable() = default;

    void push_scope() {
        scopeDeep++;
    }

    void pop_scope() {
        typename std::list<Symbol<SSAType>>::iterator it = symbols.begin();

        while (it != symbols.end()) {
            Symbol<SSAType>* symb = &*it;
            if (symb->getScopeLevel() == scopeDeep) {
                it = symbols.erase(it);
            } else {
                // job done;
                break;
            }
        }
        scopeDeep--;
    }

    bool contains(std::string name) {

        for (typename std::list<Symbol<SSAType>>::iterator it = symbols.begin(); it != symbols.end(); ++it) {
            Symbol<SSAType>* symb = &(*it);
            if (symb->getName() == name) {
                return true;
            }
        }
        return false;
    }

    StorageType getStorageType(std::string name) {
        return StorageType::LOCAL;
    }

    Symbol<SSAType>* getSymbol(std::string name) {

        for (typename std::list<Symbol<SSAType>>::iterator it = symbols.begin(); it != symbols.end(); ++it) {
            Symbol<SSAType>* symb = &(*it);
            if (symb->getName() == name) {
                return symb;
            }
        }
        return nullptr;
    }

    void insertSymbol(std::string name, StorageType storageType, SSAType menRef) {

        if (!contains(name)) {
            symbols.push_front(Symbol<SSAType>(name, menRef, storageType, scopeDeep));
        }
    }
private:
    std::list<Symbol<SSAType>> symbols;
    int scopeDeep = 0;
};

#endif	/* SYMBOLTABLE_H */

