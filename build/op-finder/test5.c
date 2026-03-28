#include <stdio.h>

int add(int a, int b) {
    return a + b;
}

int main() {
    int num1 = 5;
    int num2 = 10;
    int sum, sum1;

    sum = add(num1, num2);
    sum1 = add(num1, num2);
    printf("sum = %d", sum);
    printf("sum = %d", sum1);
    
    return 0;
}
