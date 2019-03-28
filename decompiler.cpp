#include <unistd.h>
#include <cstdio>
#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include "command.hpp"
#include "data.hpp"
#include "stack.hpp"
#include "file_operations.hpp"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::map;

void detectLabels(unsigned char const* binSource, int finSize, map<int, int> *labels) {
    for (int i = 0; i < finSize; i += sizeof(Command)) {
        int type = binSource[i + 0] + (binSource[i + 1] << 8) + (binSource[i + 2] << 16) + (binSource[i + 3] << 24);
        int arg =  binSource[i + 4] + (binSource[i + 5] << 8) + (binSource[i + 6] << 16) + (binSource[i + 7] << 24);
 
        if (type == CommandType::LABEL) {
            (*labels)[i / sizeof(Command)] = arg;
        }
    }
}

void decompileData(FILE* fout, unsigned char* binSource, int finSize, int dataStartPos) {
    int i = dataStartPos;
    while (i < finSize) {
        DataType curDataType = DataType(binSource[i]);
        i++;
        
        if (curDataType == DataType::STR) {
            fprintf(fout, ". str %i \"%s\"\n", i, binSource + i);
            i += strlen(reinterpret_cast<const char*>(binSource + i)) + 1;
        } else {
            fprintf(fout, ". int %i %i\n", i, *(reinterpret_cast<const int*>(binSource + i)));
            i += sizeof(int);
        }
    }
}

bool decompile(char const* pathIn, char const* pathOut) {
    int finSize = getFileSize(pathIn);
    if (!isValidFile(pathIn)) {
        fprintf(stderr, "Invalid path to bin-file\n");
        return false;
    }

    FILE* fin = fopen(pathIn, "rb");
    FILE* fout = fopen(pathOut, "w");

    unsigned char* binSource = (unsigned char*)calloc(finSize + 1, sizeof(unsigned char));
    
    read(fileno(fin), binSource, finSize);
    
    map<int, int> labels;
    detectLabels(binSource, finSize, &labels);
    
    bool toContinue = finSize != 0;
    for (int i = 0; toContinue; i += sizeof(Command)) {
        int type = binSource[i + 0] + (binSource[i + 1] << 8) + (binSource[i + 2]  << 16) + (binSource[i + 3]  << 24);
        int arg  = binSource[i + 4] + (binSource[i + 5] << 8) + (binSource[i + 6]  << 16) + (binSource[i + 7]  << 24);
        
        switch (type) {
          case CommandType::PUSH:
            fprintf(fout, "PUSH S %i\n", arg);
            break;
            
          case CommandType::PUSHRAX:
            fprintf(fout, "PUSH RAX\n");
            break;
              
          case CommandType::PUSHRBX:
            fprintf(fout, "PUSH RBX\n");
            break;

          case CommandType::PUSHRCX:
            fprintf(fout, "PUSH RCX\n");
            break;

          case CommandType::PUSHRDX:
            fprintf(fout, "PUSH RDX\n");
            break;

          case CommandType::PUSHRAM:
            fprintf(fout, "PUSH RAM %i\n", arg);
            break;

          case CommandType::POP:
            fprintf(fout, "POP S\n");
            break;
          
          case CommandType::POPRAX:
            fprintf(fout, "POP RAX\n");
            break;
          
          case CommandType::POPRBX:
            fprintf(fout, "POP RBX\n");
            break;
          
          case CommandType::POPRCX:
            fprintf(fout, "POP RCX\n");
            break;
          
          case CommandType::POPRDX:
            fprintf(fout, "POP RDX\n");
            break;
            
          case CommandType::POPRAM:
            fprintf(fout, "POP RAM %i\n", arg);
            break;

          case CommandType::ADD:
            fprintf(fout, "ADD\n");
            break;
            
          case CommandType::MUL:
            fprintf(fout, "MUL\n");
            break;
            
          case CommandType::IN:
            fprintf(fout, "IN\n");
            break;
            
          case CommandType::OUT:
            fprintf(fout, "OUT\n");
            break;
          
          case CommandType::PRINT:
            fprintf(fout, "PRINT %i\n", arg);
            break;

          case CommandType::AND:
            fprintf(fout, "AND\n");
            break;

          case CommandType::LABEL:
            fprintf(fout, ": %i\n", arg);
            break;
          
          case CommandType::GOTO:
            fprintf(fout, "GOTO %i\n", labels[arg]);
            break;

          case CommandType::CMP:
            fprintf(fout, "CMP S %i\n", arg);
            break;
          
          case CommandType::CMPRAX:
            fprintf(fout, "CMP RAX %i\n", arg);
            break;
          
          case CommandType::CMPRBX:
            fprintf(fout, "CMP RBX %i\n", arg);
            break;
          
          case CommandType::CMPRCX:
            fprintf(fout, "CMP RCX %i\n", arg);
            break;
          
          case CommandType::CMPRDX:
            fprintf(fout, "CMP RDX %i\n", arg);
            break;

          case CommandType::JE:
            fprintf(fout, "JE %i\n", labels[arg]);
            break;

          case CommandType::JL:
            fprintf(fout, "JL %i\n", labels[arg]);
            break;
            
          case CommandType::CALL:
            fprintf(fout, "CALL %i\n", labels[arg]);
            break;
            
          case CommandType::RET:
            fprintf(fout, "RET\n");
            break;
            
          case CommandType::SEP:
            decompileData(fout, binSource, finSize, i + 8);
            toContinue = false;
            break;
            
          default:
            fprintf(stderr, "Broken bin-file: unknown command:\n command type: %i\n command arg: %i\n", type, arg);
        }
    }
    
    fclose(fin);
    fclose(fout);
    free(binSource);
}


int main(int argc, char* argv[]) {
    if (argc <= 2 || !isValidFile(argv[1])) {
        printf("Invalid argument: first must be path to bin and second - path to source.\n");
        return 1;
    }
    
    return !decompile(argv[1], argv[2]);
}
