#include <iostream>
#include <unistd.h>
#include <stack>
#include "file_operations.hpp"
#include "command.hpp"
#include "data.hpp"
#include "stack.hpp"

using std::cout;
using std::endl;
using std::stack;

class Processor {
  public:
    enum Error{
        OK = 0,
        INVALID_COMMAND = 1,
        WRANG_ENTERED_NUMBER = 3,
        SOURCE_ERROR = 4,
        UNSUPPORTED_SOFTWARE = 5
    };

  private:
    static const int RAM_SIZE = 1024 * 1024;
    int programStartIndex = 0;
    static const int INTS_PER_COMMAND = sizeof(Command) / sizeof(int);
    
    int ram[RAM_SIZE]{0};
    int rax = 0;
    int rbx = 0;
    int rcx = 0;
    int rdx = 0;
    
    Stack s;
    
    unsigned char* GetProgramStartPointer(int binSize) {
        return reinterpret_cast<unsigned char*>(ram + RAM_SIZE - binSize / sizeof(int) - 1);
    }
    
    Error CheckStackFullness(int minSize) const {
        if (s.GetSize() < minSize) {
            fprintf(stderr, "Source error:\n getting item from empty stack\n");
            return Error(SOURCE_ERROR);
        }
        
        return Error(OK);
    }
    
    Error CheckPointerValidity(int pointer) const {
        if (pointer >= RAM_SIZE) {
            fprintf(stderr, "Unsupported software:\n not enough RAM (no address %i)\n", pointer);
            return Error(UNSUPPORTED_SOFTWARE);
        }
        
        return Error(OK);
    }
    
    Error PushReg(int* reg) {
        Error error = CheckStackFullness(1);
        if (error != Error::OK) {
            return error;
        }
        
        s.GetTop(reg);
        
        return Error(OK);
    }
    
    Error CmpReg(int reg, int comparingValue) {
        s.Push(reg);
        s.Push(comparingValue);
    }
    
    Error PushRam(int pointer) {
        Error error = CheckPointerValidity(pointer);
        if (error != Error::OK) {
            return error;
        }
        
        s.GetTop(ram + pointer);
        
        return Error(OK);
    }
    
    Error PopRam(int pointer) {
        Error error = CheckPointerValidity(pointer);
        if (error != Error::OK) {
            return error;
        }
        
        s.Push(ram[pointer]);
        
        return Error(OK);
    }    
    
    Error PrintTop() const {
        Error error = CheckStackFullness(1);
        if (error != Error::OK) {
            return error;
        }

        int result = 0;
        s.GetTop(&result);
        cout << result << endl;
        
        return Error(OK);
    }
    
    Error ProcessArithmetic(Command command) {
        Error error = CheckStackFullness(2);
        if (error != Error::OK) {
            return error;
        }
        
        int processArg1 = 0;
        int processArg2 = 0;
        
        s.GetTop(&processArg1);
        s.Pop();
        s.GetTop(&processArg2);
        s.Pop();
        
        switch (command.GetType()) {
          case CommandType::ADD:
            s.Push(processArg1 + processArg2);
            break;
            
          case CommandType::MUL:
            s.Push(processArg1 * processArg2);
            break;
        }
    }
    
    Error ProcessCompareAndJump(Command command, int* newLine) {
        Error error = CheckStackFullness(2);
        if (error != Error::OK) {
            return error;
        }
    
        int processArg1 = 0;
        int processArg2 = 0;
        
        s.GetTop(&processArg2);
        s.Pop();
        s.GetTop(&processArg1);
        s.Pop();
        
        switch (command.GetType()) {
          case CommandType::JE:
            if (processArg1 == processArg2) {
                *newLine = command.GetArg();
            }
            break;
          
          case CommandType::JL:
            if (processArg1 < processArg2) {
                *newLine = command.GetArg();
            }
            break;
        }
    }
    
    const unsigned char* dataPtr;
    DataType type;
    Error Process(Command command, int currentLine, int* newLine) {
        if (!command.IsValid()) {
            fprintf(stderr, "Invalid command:\n type: %i\n arg: %i\n", (int)command.GetType(), command.GetArg());
            return Error(INVALID_COMMAND);
        }
        
        int processArg1 = 0;
        int processArg2 = 0;
        int enteredNumber = 0;
        
        switch (command.GetType()) {            
          case CommandType::PUSH:            
            s.Push(command.GetArg());
            break;
            
          case CommandType::PUSHRAX:
            PushReg(&rax);
            break;
            
          case CommandType::PUSHRBX:
            PushReg(&rbx);
            break;
            
          case CommandType::PUSHRCX:
            PushReg(&rcx);
            break;
            
          case CommandType::PUSHRDX:
            PushReg(&rdx);
            break;
            
          case CommandType::PUSHRAM:
            PushRam(command.GetArg());
            break;  
            
          case CommandType::POP:
            s.Pop();
            break;
            
          case CommandType::POPRAX:
            s.Push(rax);
            break;

          case CommandType::POPRBX:
            s.Push(rbx);
            break;

          case CommandType::POPRCX:
            s.Push(rcx);
            break;

          case CommandType::POPRDX:
            s.Push(rdx);
            break;

          case CommandType::POPRAM:
            PopRam(command.GetArg());
            break;  

          case CommandType::ADD:
            ProcessArithmetic(command);
            break;
            
          case CommandType::MUL:
            ProcessArithmetic(command);
            break;
            
          case CommandType::IN:
            if (scanf("%i", &enteredNumber) != 1) {
                fprintf(stderr, "Wrang number entered\n");
                return Error(WRANG_ENTERED_NUMBER);
            }
            s.Push(enteredNumber);
            break;
            
          case CommandType::OUT:            
            PrintTop();
            break;
            
          case CommandType::PRINT:
            dataPtr = reinterpret_cast<const unsigned char*>(ram + programStartIndex) + command.GetArg() - 1;
            
            type = (DataType)(*dataPtr);
            if (type == DataType::STR) {
                printf("%s", dataPtr + 1);
            } else {
                printf("%i\n", *(reinterpret_cast<const int*>(dataPtr + 1)));
            }
            break;
            
          case CommandType::LABEL:
            break;
            
          case CommandType::GOTO:
            *newLine = command.GetArg();
            break;

          case CommandType::CMP:
            s.GetTop(&processArg1);
            s.Push(processArg1);
            s.Push(command.GetArg());
            break;
          
          case CommandType::CMPRAX:
            CmpReg(rax, command.GetArg());
            break;

          case CommandType::CMPRBX:
            CmpReg(rbx, command.GetArg());
            break;

          case CommandType::CMPRCX:
            CmpReg(rcx, command.GetArg());
            break;

          case CommandType::CMPRDX:
            CmpReg(rdx, command.GetArg());
            break;
            
          case CommandType::JE:
            ProcessCompareAndJump(command, newLine);
            break;
            
          case CommandType::JL:
            ProcessCompareAndJump(command, newLine);
            break;
        
          case CommandType::CALL:
            s.Push(currentLine);
            *newLine = command.GetArg();
            break;
            
          case CommandType::RET:
            s.GetTop(newLine);
            (*newLine)++;
            s.Pop();
            break;
        }
        
        return Error(OK);
    }
    
  public:
    Processor() {}

    bool Execute(unsigned char const* dataCommandArray) {
        for (int currentLine = 0;; ) {
            Command current(dataCommandArray + currentLine * sizeof(Command));
            if (current.GetType() == CommandType::AND) {
                s.Clear();
                return true;
            }
            
            int newLine = currentLine + 1;
            Error error = Process(current, currentLine, &newLine);
            if (error != Error::OK) {
                return false;
            }
            
            currentLine = newLine;
        }
        
        s.Clear();
    }
    
    bool Execute(char const* pathToBin) {
        if (!isValidFile(pathToBin)) {
            fprintf(stderr, "Invalid path to bin-file\n");
            return false;
        }
        
        int finSize = getFileSize(pathToBin);

        FILE* fin = fopen(pathToBin, "rb");
        
        programStartIndex = RAM_SIZE - finSize / sizeof(int) - 1;
        unsigned char* programStartPointer = GetProgramStartPointer(finSize);
        
        s.SetPointers(reinterpret_cast<int*>(programStartPointer));

        read(fileno(fin), programStartPointer, finSize);
        Execute(programStartPointer);
        
        fclose(fin);
        
        return true;
    }
};

int main(int argc, char* argv[]) {
    if (argc <= 1 || !isValidFile(argv[1])) {
        printf("Invalid argument: it must be path to source\n");
        return 1;
    }
    
    Processor processor;
    
    return !processor.Execute(argv[1]);
}

