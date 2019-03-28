#pragma once

#include <iostream>
#include <cstdio>
#include <cstring>
#include <typeinfo>

class Stack{
    int* stackPointer;
    int* stackEnd;

  public:
    enum Error{
        OK = 0,
        GET_TOP_FROM_EMPTY = 1,
        POP_FROM_EMPTY = 2
    };
    
    Stack() : stackPointer(nullptr), stackEnd(nullptr) {}

    void SetPointers(int* ptr) {
        stackPointer = ptr;
        stackEnd = ptr;
    }
    
    Error GetTop(int* res) const {
        if (stackPointer == stackEnd) {
            return Error(GET_TOP_FROM_EMPTY);
        }
        
        *res = *(stackPointer + 1);
        
        return Error(OK);
    }
    
    Error Push(int arg) {
        *stackPointer = arg;
        --stackPointer;
        
        return Error(OK);
    }
    
    Error Pop() {
        if (stackPointer == stackEnd) {
            return Error(POP_FROM_EMPTY);
        }
        
        ++stackPointer;
        
        return Error(OK);
    }
    
    int GetSize() const {
        return stackEnd - stackPointer;
    }
    
    Error Clear() {
        stackPointer = stackEnd;
        
        return Error(OK);
    }
};

