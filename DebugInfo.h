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

    DebugInfo(int line, int column, std::string file) : line(line), column(column), file(file) {
    }
    
    virtual ~DebugInfo() {}

    std::string getInfo() {
        std::string str = std::string(file);
        str.append(": line " + std::to_string(line) + " col " + std::to_string(column));
        return str;
    }

private:
    int line;
    int column;
    std::string file;
};


#endif	/* DEBUGINFO_H */

