#ifndef GRIDSTACK_H
#define GRIDSTACK_H
#include <list>

template <class Type>
class GridStack {
public:
    GridStack();
    void pushGrid(Type t);
    Type popGrid();

private:
    int capacity;
    std::list<Type> stack;
};

template <class Type>
GridStack<Type>::GridStack() :
    capacity(10){
}

template <class Type>
void GridStack<Type>::pushGrid(Type t) {
    stack.push_front(t);
    if (stack.size() > capacity) {
        delete stack.back();
        stack.pop_back();
    }
}

template <class Type>
Type GridStack<Type>::popGrid() {
    Type t = stack.front();
    stack.pop_front();
    return t; // transfers ownership of memory to caller
}


#endif // GRIDSTACK_H
