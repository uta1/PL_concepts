#include <cstdio>
#include <sstream>
#include <iostream>
#include <string>
#include <set>
#include <map>
#include <vector>
#include "command.hpp"
#include "data.hpp"
#include "stack.hpp"
#include "file_operations.hpp"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::set;
using std::map;

void writeCommandToFile(FILE* fout, CommandType commandType, int arg, unsigned char* binCommand) {
    Command(commandType, arg).Compile(binCommand);
    for (int i = 0; i < sizeof(Command); ++i) {
        fprintf(fout, "%c", binCommand[i]);
    }
}

bool processLabel(std::istringstream &stream, map<int, set<int> >* labels, int commandIndex, int* name) {
    int arg = 0;
    stream >> arg;
    
    *name = arg;
    
    if ((*labels)[arg].size() == 1 && *((*labels)[arg].begin()) != commandIndex) {
        fprintf(stderr, "\033[31mCompilation error:\033[0m redefine of label %i\n", arg);
        return false;
    }
    
    (*labels)[arg].insert(commandIndex);
    
    return true;
}

void writeUserCommand(FILE* fout, std::istringstream &stream, CommandType commandType, unsigned char* binCommand, bool withArg) {
    int arg = 0;
    if (withArg) {
        stream >> arg;
    }

    writeCommandToFile(fout, commandType, arg, binCommand);
}

void writeJumpCommand(FILE* fout, std::istringstream &stream, map<int, set<int> >* labels, CommandType commandType, unsigned char* binCommand) {
    int arg = 0;
    stream >> arg;

    if (labels->count(arg) > 0) {
        arg = *((*labels)[arg].begin());
        writeCommandToFile(fout, commandType, arg, binCommand);
    }
}

bool processStrData(std::istringstream &stream, map<int, set<string> >* data) {
    int name = 0;
    stream >> name;
    
    string strData;
    char curChar = '\0';
    
    stream >> curChar;
    if (curChar != '\"') {
        fprintf(stderr, "\033[31mCompilation error:\033[0m unquoted string in data section\n");
        return false;
    }
    getline(stream, strData, '\"');
    
    if ((*data)[name].size() == 1 && *((*data)[name].begin()) != strData) {
        fprintf(stderr, "\033[31mCompilation error:\033[0m redefine of data %i\n", name);
        return false;
    }
    
    (*data)[name].insert(strData);
    
    return true;
}

bool processIntData(std::istringstream &stream, map<int, set<int> >* data) {
    int name = 0;
    stream >> name;
    
    int intData = 0;
    
    stream >> intData;
    
    if ((*data)[name].size() == 1 && *((*data)[name].begin()) != intData) {
        fprintf(stderr, "\033[31mCompilation error:\033[0m redefine of data %i\n", name);
        return false;
    }
    
    (*data)[name].insert(intData);
    
    return true;
}

int writeStrData(FILE* fout, map<int, set<string> > *strData, map<int, int> *nameToDataDisplacement, int initDisplacement) {
    int bytesUsed = initDisplacement;
    for (map<int, set<string> >::iterator it = (*strData).begin(); it != (*strData).end(); ++it) {
        const char* curStrArray = (it->second).begin()->c_str();
        fprintf(fout, "%c%s%c", DataType::STR, curStrArray, '\0');
        (*nameToDataDisplacement)[it->first] = bytesUsed + 1;
        bytesUsed += 1 + strlen(curStrArray) + 1;
    }
    return bytesUsed;
}

void writeSeparator(FILE* fout, unsigned char* binCommand) {
    writeCommandToFile(fout, CommandType::SEP, 0, binCommand);
}

int writeIntData(FILE* fout, map<int, set<int> > *intData, map<int, int> *nameToDataDisplacement, int initDisplacement) {
    int bytesUsed = initDisplacement;
    for (map<int, set<int> >::iterator it = (*intData).begin(); it != (*intData).end(); ++it) {
        set<int>::iterator dataBegin = (it->second).begin();
        const int* curDataPtrInt = &(*dataBegin);
        const unsigned char* curDataPtrChar = reinterpret_cast<const unsigned char*>(curDataPtrInt);
        fprintf(fout, "%c%c%c%c%c", DataType::INT, *curDataPtrChar, *(curDataPtrChar + 1), *(curDataPtrChar + 2), *(curDataPtrChar + 3));
        (*nameToDataDisplacement)[it->first] = bytesUsed + 1;
        bytesUsed += 1 + sizeof(int);
    }
    return bytesUsed;
}

void writePrintCommand(FILE* fout, std::istringstream &stream, map<int, int> *nameToDataDisplacement, unsigned char* binCommand) {
    int arg = 0;
    stream >> arg;
    
    int argToWrite = 0;
    if (nameToDataDisplacement->count(arg) > 0) {
        argToWrite = (*nameToDataDisplacement)[arg];
    }
    
    writeCommandToFile(fout, CommandType::PRINT, argToWrite, binCommand);
}

bool compileCycle(
  char const* pathIn, 
  char const* pathOut, 
  map<int, set<int> > *labels, 
  map<int, set<string> > *strData,
  map<int, set<int> > *intData,
  map<int, int> *nameToDataDisplacement
) {
    int finSize = getFileSize(pathIn);
    if (!isValidFile(pathIn)) {
        fprintf(stderr, "\033[Error:\033[0m invalid path to source\n");
        return false;
    }

    char* source = readFile(pathIn, finSize);
    
    std::istringstream stream;
    stream.str(source);
    
    unsigned char* binCommand = (unsigned char*)calloc(sizeof(Command), sizeof(unsigned char));
    char* type = (char*)calloc(10, sizeof(char));
    char* sysArg = (char*)calloc(10, sizeof(char)); 
    
    FILE* fout = fopen(pathOut, "wb");

    int commandIndex = -1;
    
    while (stream >> type) {
        ++commandIndex;

        if (strcmp(type, ".") == 0) {
            --commandIndex;
            stream >> sysArg;
            if (strcmp(sysArg, "str") == 0) {    
                if (!processStrData(stream, strData)) {
                    return false;
                }
            } else {
                if (!processIntData(stream, intData)) {
                    return false;
                }
            }
            continue;
        }

        if (strcmp(type, ":") == 0) {
            int name = 0;
            if (!processLabel(stream, labels, commandIndex, &name)) {
                return false;
            }
            writeCommandToFile(fout, CommandType::LABEL, name, binCommand);  
            continue;
        }
        
        if (strcmp(type, "PRINT") == 0) {
            writePrintCommand(fout, stream, nameToDataDisplacement, binCommand);
            continue;
        }
        
        if (strcmp(type, "GOTO") == 0) {
            writeJumpCommand(fout, stream, labels, CommandType::GOTO, binCommand);
            continue;
        }
        
        if (strcmp(type, "PUSH") == 0) {
            stream >> sysArg;
                    
            if (strcmp(sysArg, "S") == 0) {
                writeUserCommand(fout, stream, CommandType::PUSH, binCommand, true);
            } else 
             
            if (strcmp(sysArg, "RAX") == 0) {
                writeUserCommand(fout, stream, CommandType::PUSHRAX, binCommand, false);
            } else
            if (strcmp(sysArg, "RBX") == 0) {
                writeUserCommand(fout, stream, CommandType::PUSHRBX, binCommand, false);
            } else
            if (strcmp(sysArg, "RCX") == 0) {
                writeUserCommand(fout, stream, CommandType::PUSHRCX, binCommand, false);
            } else 
            if (strcmp(sysArg, "RDX") == 0) {
                writeUserCommand(fout, stream, CommandType::PUSHRDX, binCommand, false);
            } else 

            if (strcmp(sysArg, "RAM") == 0) {
                writeUserCommand(fout, stream, CommandType::PUSHRAM, binCommand, true);
            }  
            continue;
        }
        
        if (strcmp(type, "POP") == 0) {
            stream >> sysArg;
            
            if (strcmp(sysArg, "S") == 0) {
                writeUserCommand(fout, stream, CommandType::POP, binCommand, false);
            } else 
            
            if (strcmp(sysArg, "RAX") == 0) {
                writeUserCommand(fout, stream, CommandType::POPRAX, binCommand, false);
            } else
            if (strcmp(sysArg, "RBX") == 0) {
                writeUserCommand(fout, stream, CommandType::POPRBX, binCommand, false);
            } else
            if (strcmp(sysArg, "RCX") == 0) {
                writeUserCommand(fout, stream, CommandType::POPRCX, binCommand, false);
            } else 
            if (strcmp(sysArg, "RDX") == 0) {
                writeUserCommand(fout, stream, CommandType::POPRDX, binCommand, false);
            } else 

            if (strcmp(sysArg, "RAM") == 0) {
                writeUserCommand(fout, stream, CommandType::POPRAM, binCommand, true);
            }
            continue;
        }
        
        if (strcmp(type, "CMP") == 0) {
            stream >> sysArg;
            
            if (strcmp(sysArg, "S") == 0) {
                writeUserCommand(fout, stream, CommandType::CMP, binCommand, true);
            } else 
            
            if (strcmp(sysArg, "RAX") == 0) {
                writeUserCommand(fout, stream, CommandType::CMPRAX, binCommand, true);
            } else
            if (strcmp(sysArg, "RBX") == 0) {
                writeUserCommand(fout, stream, CommandType::CMPRBX, binCommand, true);
            } else
            if (strcmp(sysArg, "RCX") == 0) {
                writeUserCommand(fout, stream, CommandType::CMPRCX, binCommand, true);
            } else 
            if (strcmp(sysArg, "RDX") == 0) {
                writeUserCommand(fout, stream, CommandType::CMPRDX, binCommand, true);
            }
            continue;
        }
        
        if (strcmp(type, "JE") == 0) {
            writeJumpCommand(fout, stream, labels, CommandType::JE, binCommand);
            continue;
        }
        
        if (strcmp(type, "JE") == 0) {
            writeJumpCommand(fout, stream, labels, CommandType::JE, binCommand);
            continue;
        }
        
        if (strcmp(type, "JL") == 0) {
            writeJumpCommand(fout, stream, labels, CommandType::JL, binCommand);
            continue;
        }
        
        if (strcmp(type, "CALL") == 0) {
            writeJumpCommand(fout, stream, labels, CommandType::CALL, binCommand);
            continue;
        }
        
        if (strcmp(type, "ADD") == 0) {
            writeUserCommand(fout, stream, CommandType::ADD, binCommand, false);
            continue;
        }
        
        if (strcmp(type, "MUL") == 0) {
            writeUserCommand(fout, stream, CommandType::MUL, binCommand, false);
            continue;
        }
        
        if (strcmp(type, "IN") == 0) {
            writeUserCommand(fout, stream, CommandType::IN, binCommand, false);
            continue;
        }
        
        if (strcmp(type, "OUT") == 0) {
            writeUserCommand(fout, stream, CommandType::OUT, binCommand, false);
            continue;
        }
        
        if (strcmp(type, "AND") == 0) {
            writeUserCommand(fout, stream, CommandType::AND, binCommand, false);
            continue;
        }
        
        if (strcmp(type, "RET") == 0) {
            writeUserCommand(fout, stream, CommandType::RET, binCommand, false);
            continue;
        }
        
        fprintf(stderr, "\033[31mCompilation error:\033[0m unknown command type \"%s\"\n", type);
        
        free(type);
        free(binCommand);
        
        return false;
    }
    
    writeSeparator(fout, binCommand);
    ++commandIndex;
    
    int bytesUsed = (commandIndex + 1) * sizeof(Command);
    bytesUsed = writeStrData(fout, strData, nameToDataDisplacement, bytesUsed);
    bytesUsed = writeIntData(fout, intData, nameToDataDisplacement, bytesUsed);
    
    fclose(fout);
    free(type);
    free(sysArg);
    free(binCommand);
    free(source);
    
    return true;
}

int main(int argc, char* argv[]) {
    if (argc <= 2 || !isValidFile(argv[1])) {
        printf("\033[31mError:\033[0m invalid argument: first must be path to source and second - path to bin.\n");
        return 1;
    }
    
    map<int, set<int> > labels;
    map<int, set<string> > strData;
    map<int, set<int> > intData;
    map<int, int> nameToDataDisplacement;
    
    if (!compileCycle(argv[1], argv[2], &labels, &strData, &intData, &nameToDataDisplacement)) {
        return 1;
    }
    
    return !compileCycle(argv[1], argv[2], &labels, &strData, &intData, &nameToDataDisplacement);
}
