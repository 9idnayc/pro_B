#include <iostream>


void add1(int* x){
    (*x)++;
}

int main(){
    int a = 1;
    int* b = &a;
    printf("%d ", a);
    add1(b);
    printf("%d ", *b);
    return 0;
}