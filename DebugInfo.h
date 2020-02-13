/* 
 * File:   DebugInfo.h
 * Author: andreu
 *
 * Created on 6 de Novembro de 2019, 17:34
 */

#ifndef DEBUGINFO_H
#define	DEBUGINFO_H

#include <string>

class DebugInfo {
public:

    DebugInfo(int line, int column, std::string file, std::string LineStr) : line(line), column(column), file(file), LineStr(LineStr) {
    }
    
    virtual ~DebugInfo() {}

    std::string getInfo() {
        std::string str = std::string(file);
        str.append(": line " + std::to_string(line) + " col " + std::to_string(column) + ": " + LineStr);
        return str;
    }

private:
    int line;
    int column;
    std::string file;
    std::string LineStr;
};


#endif	/* DEBUGINFO_H */

