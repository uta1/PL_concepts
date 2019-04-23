#include <map>
#include <set>
#include <string>
#include <iostream>
#include <functional>
#include <typeinfo>
#include <vector>

using namespace std;

class VitualClassParent{
  protected:
    bool deleted_already = false;
    vector<function<void(VitualClassParent*)>> destructors;
    unordered_map<string, function<void(VitualClassParent*)>> virtual_funcs_;
};

#define BASE(name)    \
    class name : public VitualClassParent { \
      public: \
        name() {

#define GET_FIELD_FROM_METHOD(field_name) (obj_ptr->field_name)
#define DECLARE_METHOD(class_name, method_name, realization)    \
    virtual_funcs_[#method_name] = \
        [this](VitualClassParent* arg) -> \
        void{   \
            class_name* obj_ptr = reinterpret_cast<class_name*>(arg);   \
            realization;    \
        };

#define GET_FIELD_FROM_CONSTRUCTOR(field_name) (field_name)
#define CONSTRUCTOR(name, realization)  \
        realization;    \
    

#define GET_FIELD_FROM_DESTRUCTOR(field_name) GET_FIELD_FROM_CONSTRUCTOR(field_name)
#define DESTRUCTOR(class_name, realization)   \
        auto current_destructor = [this](VitualClassParent* arg) -> \
            void{   \
                class_name* obj_ptr = reinterpret_cast<class_name*>(arg);   \
                realization;    \
            };  \
        destructors.push_back(current_destructor); \
    }   \
    ~class_name() { \
        if (!deleted_already) { \
            for (int i = destructors.size() - 1; i >= 0; --i) {  \
                destructors[i](this);   \
            }   \
            deleted_already = true; \
        }   \
    }

#define END(name)  \
      public: \
        void call_virtual_func(string method_name) { \
            virtual_funcs_[method_name](this); \
        } \
    };

#define DERIVE(name, parent) \
    class name: public parent { \
    public: \
        name() {

#define VIRTUAL_CALL(object_ptr, method_name) \
    try {   \
        (object_ptr)->call_virtual_func(#method_name);  \
    } catch (bad_function_call exception) {   \
        cout << "using of undefined function in line " << __LINE__ << endl; \
        throw exception; \
    }


