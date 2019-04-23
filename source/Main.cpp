#include "oop.hpp"

#include <iostream>

using std::cout;
using std::endl;

class Creation;

void printInfo(Creation* creation);

BASE(Creation);
    DECLARE_METHOD(Creation, describe, {
        cout << "Creation::describe" << endl;
    });
    DECLARE_METHOD(Creation, printAge, {
        cout << "Creation::printAge " << GET_FIELD_FROM_METHOD(age) << endl;
    });
    DECLARE_METHOD(Creation, celebrateBirthday, {
        cout << "Creation::celebrateBirthday" << endl;
        cout << "  age after celebrating - " << ++GET_FIELD_FROM_METHOD(age) << endl;
        printInfo(this);
    });
    CONSTRUCTOR(Creation, {
        cout << "Creation born" << endl;
        age = 0;
    });    
    DESTRUCTOR(Creation, {
        cout << "Creation death" << endl;
    });
    int age;
END(b);

void printInfo(Creation* creation) {
    VIRTUAL_CALL(creation, describe);
    VIRTUAL_CALL(creation, printAge);
}

DERIVE(Animal, Creation);
    DECLARE_METHOD(Animal, describe, {
        cout << "Animal::describe" << endl;
        cout << "  heterotrophy, greater mobility" << endl;
    });
    CONSTRUCTOR(Animal, {
        cout << "Animal born" << endl;
    });
    DESTRUCTOR(Animal, {
        cout << "Animal death" << endl;
    })
END(Mammal);

DERIVE(Human, Animal);
    DECLARE_METHOD(Human, describe, {
        cout << "Human::describe" << endl;
        cout << "  mind" << endl;
    });
    DECLARE_METHOD(Human, celebrateBirthday, {
        cout << "Human::celebrateBirthday" << endl;
        cout << "  party hard with " << GET_FIELD_FROM_METHOD(friendsNumber) << " friends!" << endl;
        cout << "  age after celebrating - " << ++GET_FIELD_FROM_METHOD(age) << endl;
        printInfo(this);
    });
    DECLARE_METHOD(Human, think, {
        cout << "Human::think" << endl;
    });
    CONSTRUCTOR(Human, {
        cout << "Human born" << endl;
        friendsNumber = rand() % 100;
    });
    DESTRUCTOR(Human, {
        cout << "Human death" << endl;
    });
    int friendsNumber;
END(Human);

BASE(WithoutMethods);
    CONSTRUCTOR(WithoutMethods, {
        cout << "WithoutMethods constr" << endl;
        GET_FIELD_FROM_DESTRUCTOR(humanLike) = new Human;
    });
    DESTRUCTOR(WithoutMethods, {
        cout << "WithoutMethods destr" << endl;
        delete GET_FIELD_FROM_DESTRUCTOR(humanLike);
    });
    Creation* humanLike;
    Human human;
END(WithoutMethods);

int main() {
    cout << "GENERAL TEST: " << endl;
    Creation* ptr = nullptr;
    
    Creation* creation = new Creation;
    cout << endl;
    ptr = creation;
    VIRTUAL_CALL(ptr, celebrateBirthday)
    VIRTUAL_CALL(ptr, celebrateBirthday)
    
    cout << endl << endl;
    
    Animal* animal = new Animal;
    cout << endl;
    ptr = animal;
    VIRTUAL_CALL(ptr, celebrateBirthday)
    VIRTUAL_CALL(ptr, celebrateBirthday)
    
    cout << endl << endl;
    
    Human* human = new Human;
    cout << endl;
    ptr = human;
    VIRTUAL_CALL(ptr, celebrateBirthday)
    VIRTUAL_CALL(ptr, celebrateBirthday)
    
    cout << endl << endl << endl << endl;
    
    cout << "UNDEFINED METHODS: " << endl;
    
    try {
        VIRTUAL_CALL(animal, think);
    } catch (bad_function_call) {
    
    }
    
    cout << endl << endl << endl << endl;
    
    cout << "DELETING TEST: " << endl; 
        
    ptr = creation;
    delete ptr;
    cout << endl;
    
    ptr = animal;
    delete ptr;
    cout << endl;
    
    ptr = human;
    delete ptr;
    
    cout << endl << endl << endl << endl;
    
    cout << "NESTED STRUCTURES CONSTUCTORS: " << endl;
    
    WithoutMethods withoutMethods;
    cout << endl;
    WithoutMethods* withoutMethodsPtr = new WithoutMethods;
    
    cout << endl << endl << endl << endl;
    
    cout << "NESTED STRUCTURES DESTRUCTORS: " << endl;
    delete withoutMethodsPtr;
    
    cout << endl << endl << endl << endl;
    
    cout << "STATIC DELETING: " << endl;
    return 0;
}
