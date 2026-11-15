#include <stdio.h>

int op_add(int a, int b) { return a + b; }
int op_sub(int a, int b) { return a - b; }
int op_xor(int a, int b) { return a ^ b; }
int op_and(int a, int b) { return a & b; }
int op_or(int a, int b)  { return a | b; }

int main() {
    int x, y;
    printf("Enter two numbers: ");
    scanf("%d %d", &x, &y);
    printf("Add: %d | Sub: %d | Xor: %d | And: %d | Or: %d\n",
           op_add(x, y), op_sub(x, y), op_xor(x, y), op_and(x, y), op_or(x, y));
    return 0;
}
